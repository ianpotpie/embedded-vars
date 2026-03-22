#include "../include/cli_server.h"
#include "../include/var_registry.h"
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define CONNECTION_READ_BUFFER_SIZE 1024
#define CONNECTION_WRITE_BUFFER_SIZE 4096
#define COMMAND_MAX_ARGS 16

// TODO: define a path_buffer_size used for registry_variables and here

#define UNUSED(x) (void)(x)

/**
 * @brief Creates a TCP listener socket.
 *
 * Creates a TCP listener socket, binds it to the specified address, and starts
 * listening for incoming connections.
 *
 * @param addr Pointer to a sockaddr structure containing the address to bind
 * to.
 * @param max_pending_connections The maximum number of pending connections
 * allowed in the queue.
 * @return The file descriptor of the created TCP listener socket on success.
 *         Exits the program on failure.
 */
int create_tcp_listener(struct sockaddr *addr, int max_pending_connections) {
  int res;
  int listener_fd;

  // Step 1: Create a TCP listener socket
  listener_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (listener_fd == -1) {
    perror("Error while creating TCP listener socket");
    exit(EXIT_FAILURE);
  } else {
    printf("Successfully created TCP listener socket\n");
  }

  // Step 2: Bind the socket to a port
  res = bind(listener_fd, addr, sizeof(struct sockaddr));

  if (res == -1) {
    perror("Error while binding TCP listener socket");
    exit(EXIT_FAILURE);
  } else {
    printf("Successfully bound TCP listener socket\n");
  }

  // Step 3: Listen for incoming connections
  res = listen(listener_fd, max_pending_connections);

  if (res == -1) {
    perror("Error while beginning to listen on TCP listener socket");
    exit(EXIT_FAILURE);
  } else {
    printf("Successfully started listening on TCP listener socket\n");
  }

  return listener_fd;
}

typedef void (*command_func_t)(connection_context_t *context, int argc,
                               char **argv);

typedef struct {
  const char *name;
  const char *desc;
  command_func_t func;
} command_t;

void cmd_hello(connection_context_t *context, int argc, char **argv) {
  UNUSED(argc);
  UNUSED(argv);

  char write_buffer[CONNECTION_WRITE_BUFFER_SIZE];
  snprintf(write_buffer, sizeof(write_buffer), "Hello, client on FD %d!\n",
           context->connection_fd);
  send(context->connection_fd, write_buffer, strlen(write_buffer), 0);
}

void cmd_exit(connection_context_t *context, int argc, char **argv) {
  UNUSED(argc);
  UNUSED(argv);

  send(context->connection_fd, "Goodbye!\n", 9, 0);

  printf("[Server] Client on FD %d exited.\n", context->connection_fd);
  close(context->connection_fd);

  pthread_mutex_lock(context->connection_count_mutex);
  (*context->connection_count)--;
  pthread_cond_signal(context->connection_available_cond);
  pthread_mutex_unlock(context->connection_count_mutex);

  pthread_exit(NULL);
}

void cmd_help(connection_context_t *context, int argc,
              char **argv); // Forward declaration since cmd_help is used in the
                            // registry before its definition

void cmd_get(connection_context_t *context, int argc, char **argv) {
  UNUSED(argc);
  char write_buffer[CONNECTION_WRITE_BUFFER_SIZE];

  var_t *var = get_registry_entry_by_name(context->registry, argv[1]);
  if (var == NULL) {
    snprintf(write_buffer, sizeof(write_buffer), "Variable not found: %s\n",
             argv[1]);
    send(context->connection_fd, write_buffer, strlen(write_buffer), 0);
    return;
  } else {
    char value_buf[256];
    var_value_as_string(var, value_buf, sizeof(value_buf));
    snprintf(write_buffer, sizeof(write_buffer), "%s: %s\n", var->name,
             value_buf);
    send(context->connection_fd, write_buffer, strlen(write_buffer), 0);
  }
}

void cmd_set(connection_context_t *context, int argc, char **argv) {
  UNUSED(context);
  UNUSED(argc);
  UNUSED(argv);

  // TODO: Implement setting variable values
}

void cmd_list(connection_context_t *context, int argc, char **argv) {
  UNUSED(argc);
  UNUSED(argv);

  // TODO: implement
  // for now, just list all variables in the registry
  var_t *entries = context->registry->entries;
  size_t count = context->registry->count;
  char write_buffer[CONNECTION_WRITE_BUFFER_SIZE];

  snprintf(write_buffer, sizeof(write_buffer), "Variables in registry:\n");
  for (size_t i = 0; i < count; i++) {
    char value_buf[256];
    var_value_as_string(&entries[i], value_buf, sizeof(value_buf));
    strncat(write_buffer, entries[i].name,
            sizeof(write_buffer) - strlen(write_buffer) - 1);
    strncat(write_buffer, ": ",
            sizeof(write_buffer) - strlen(write_buffer) - 1);
    strncat(write_buffer, value_buf,
            sizeof(write_buffer) - strlen(write_buffer) - 1);
    strncat(write_buffer, "\n",
            sizeof(write_buffer) - strlen(write_buffer) - 1);
  }

  send(context->connection_fd, write_buffer, strlen(write_buffer), 0);
}

void cmd_cd(connection_context_t *context, int argc, char **argv) {
  if (argc < 2 || argv[1] == NULL)
    return;

  char new_cwd[256];
  char *path_ptr =
      argv[1]; // Use a pointer so we don't lose the start of argv[1]

  // 1. Initialize the starting point
  if (path_ptr[0] == '/') {
    snprintf(new_cwd, sizeof(new_cwd), "/");
  } else {
    snprintf(new_cwd, sizeof(new_cwd), "%s", context->cwd);
  }

  // 2. Tokenize and step through the path
  char *token;
  while ((token = strsep(&path_ptr, "/")) != NULL) {
    // Skip empty tokens (from //) or "." (current dir)
    if (*token == '\0' || strcmp(token, ".") == 0) {
      continue;
    }

    if (strcmp(token, "..") == 0) {
      // Find the last slash to move up
      char *last_slash = strrchr(new_cwd, '/');
      if (last_slash != NULL) {
        if (last_slash == new_cwd) {
          // We are at root, keep the leading slash
          new_cwd[1] = '\0';
        } else {
          // Erase the last component
          *last_slash = '\0';
        }
      }
    } else {
      // Append a slash unless we are at root "/"
      size_t len = strlen(new_cwd);
      if (len > 1 || (len == 1 && new_cwd[0] != '/')) {
        strncat(new_cwd, "/", sizeof(new_cwd) - strlen(new_cwd) - 1);
      } else if (len == 0) {
        // Should not happen with current logic, but safe-guard
        new_cwd[0] = '/';
        new_cwd[1] = '\0';
      }

      strncat(new_cwd, token, sizeof(new_cwd) - strlen(new_cwd) - 1);
    }
  }

  // 3. Update the context
  strncpy(context->cwd, new_cwd, 256);
  context->cwd[255] = '\0'; // Ensure null-termination
}

// 3. The Actual Registry (Array)
command_t registry[] = {
    {"hello", "Greets the user", cmd_hello},
    {"help", "Shows this list", cmd_help},
    {"exit", "Quits the REPL", cmd_exit},
    {"get", "Gets the value of a variable", cmd_get},
    {"set", "Sets the value of a variable", cmd_set},
    {"ls", "List the contents of the current directory", cmd_list},
    {"cd", "Change the current directory", cmd_cd},
};

#define CMD_COUNT (sizeof(registry) / sizeof(command_t))

/**
 * @brief Command handler for the "help" command.
 *
 * Sends a list of available commands and their descriptions to the client.
 * @param context Pointer to the connection context containing the client
 * information and registry.
 * @param argc The number of arguments passed to the command (not used).
 * @param argv The array of argument strings passed to the command (not used).
 *
 * Note: This function uses the global `registry` array to access the list of
 * commands and their descriptions.
 */
void cmd_help(connection_context_t *context, int argc, char **argv) {
  UNUSED(argc);
  UNUSED(argv);
  char write_buffer[CONNECTION_WRITE_BUFFER_SIZE];

  snprintf(write_buffer, sizeof(write_buffer), "Available commands:\n");
  for (size_t i = 0; i < CMD_COUNT; i++) {
    strncat(write_buffer, registry[i].name,
            sizeof(write_buffer) - strlen(write_buffer) - 1);
    strncat(write_buffer, ": ",
            sizeof(write_buffer) - strlen(write_buffer) - 1);
    strncat(write_buffer, registry[i].desc,
            sizeof(write_buffer) - strlen(write_buffer) - 1);
    strncat(write_buffer, "\n",
            sizeof(write_buffer) - strlen(write_buffer) - 1);
  }

  send(context->connection_fd, write_buffer, strlen(write_buffer), 0);
}

void execute_command(connection_context_t *context, char *line) {
  // Basic string tokenizer (splits by space)
  char write_buffer[CONNECTION_WRITE_BUFFER_SIZE];
  char *argv[COMMAND_MAX_ARGS];
  int argc = 0;
  char *token = strtok(line, " \n");

  while (token != NULL && argc < COMMAND_MAX_ARGS) {
    argv[argc++] = token;
    token = strtok(NULL, " \n");
  }

  if (argc == 0)
    return;

  // Search registry
  for (int i = 0; i < CMD_COUNT; i++) {
    if (strcmp(argv[0], registry[i].name) == 0) {
      registry[i].func(context, argc, argv);
      return;
    }
  }

  snprintf(write_buffer, sizeof(write_buffer),
           "Unknown command: %s. Type 'help' for info.\n", argv[0]);
  send(context->connection_fd, write_buffer, strlen(write_buffer), 0);
}

void *handle_connection(void *arg) {
  connection_context_t context = *((connection_context_t *)arg);
  free(arg);

  int fd = context.connection_fd;
  char read_buffer[CONNECTION_READ_BUFFER_SIZE];
  char cwd_buffer[256] = "/";
  context.cwd = cwd_buffer;
  ssize_t bytes_read;

  printf("[Server] Handling new connection on FD %d\n", fd);

  send(fd, cwd_buffer, strlen(cwd_buffer), 0);
  send(fd, "> ", 2, 0); // Send prompt after each command execution

  // Loop until the client closes the connection or an error occurs
  while ((bytes_read = recv(fd, read_buffer, sizeof(read_buffer) - 1, 0)) > 0) {

    // 1. Null-terminate the received data to treat it as a string
    read_buffer[bytes_read] = '\0';
    printf("[Client %d]: %s", fd, read_buffer);

    execute_command(&context, read_buffer);
    send(fd, cwd_buffer, strlen(cwd_buffer), 0);
    send(fd, "> ", 2, 0); // Send prompt after each command execution
  }

  if (bytes_read == 0) {
    printf("[Server] Client on FD %d disconnected.\n", fd);
  } else if (bytes_read == -1) {
    perror("recv failed");
  }

  close(context.connection_fd);

  pthread_mutex_lock(context.connection_count_mutex);
  (*context.connection_count)--;
  pthread_cond_signal(context.connection_available_cond);
  pthread_mutex_unlock(context.connection_count_mutex);

  return NULL;
}

void *handle_listener(void *arg) {
  listener_context_t context = *((listener_context_t *)arg);
  free(arg);

  int connection_count = 0;
  pthread_mutex_t connection_count_mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t connection_available_cond = PTHREAD_COND_INITIALIZER;

  while (true) {
    pthread_mutex_lock(&connection_count_mutex);
    while (connection_count >= context.max_connections) {
      printf("Maximum number of connections reached. Waiting for a connection "
             "to close...\n");
      pthread_cond_wait(&connection_available_cond, &connection_count_mutex);
    }

    struct sockaddr client_addr;
    socklen_t client_addrlen = sizeof(client_addr);
    int connection_fd =
        accept(context.listener_fd, &client_addr, &client_addrlen);

    connection_count++;
    pthread_mutex_unlock(&connection_count_mutex);

    if (connection_fd == -1) {
      char *err_template = "Error: %s (while accepting incoming connection).\n";
      fprintf(stderr, err_template, strerror(errno));
      exit(EXIT_FAILURE);
    } else {
      printf("Successfully accepted incoming connection\n");
    }

    connection_context_t *connection_context =
        malloc(sizeof(connection_context_t));
    connection_context->connection_fd = connection_fd;
    connection_context->client_addr = client_addr;
    connection_context->client_addrlen = client_addrlen;
    connection_context->connection_count = &connection_count;
    connection_context->connection_count_mutex = &connection_count_mutex;
    connection_context->connection_available_cond = &connection_available_cond;
    connection_context->registry = context.registry;

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, handle_connection, connection_context);
    pthread_detach(thread_id);
  }
}

int start_cli_server(var_registry_t *registry, pthread_t *thread_id,
                     uint32_t ip, uint16_t port, unsigned int max_connections,
                     unsigned int max_pending_connections) {

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = htonl(ip);

  int listener_fd = create_tcp_listener((struct sockaddr *)&server_addr,
                                        max_pending_connections);

  listener_context_t *context = malloc(sizeof(listener_context_t));
  context->listener_fd = listener_fd;
  context->max_connections = max_connections;
  context->registry = registry;

  pthread_create(thread_id, NULL, handle_listener, context);

  return 0;
}

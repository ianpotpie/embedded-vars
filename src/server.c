#include "../include/server.h"
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

static int connection_count = 0;
static pthread_mutex_t connection_count_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t connection_slots_available = PTHREAD_COND_INITIALIZER;

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

typedef void (*command_func_t)(connection_contex_t *context, int argc,
                               char **argv);

typedef struct {
  const char *name;
  const char *desc;
  command_func_t func;
} command_t;

void cmd_hello(connection_contex_t *context, int argc, char **argv) {
  char buffer[256];
  snprintf(buffer, sizeof(buffer), "Hello, client on FD %d!\n",
           context->connection_fd);
  send(context->connection_fd, buffer, strlen(buffer), 0);
}

void cmd_exit(connection_contex_t *context, int argc, char **argv) {
  send(context->connection_fd, "Goodbye!\n", 9, 0);
  context->exit_triggered = true;
}

void cmd_help(connection_contex_t *context, int argc,
              char **argv); // Forward declaration

// 3. The Actual Registry (Array)
command_t registry[] = {{"hello", "Greets the user", cmd_hello},
                        {"help", "Shows this list", cmd_help},
                        {"exit", "Quits the REPL", cmd_exit}};

#define CMD_COUNT (sizeof(registry) / sizeof(command_t))

void cmd_help(connection_contex_t *context, int argc, char **argv) {
  char buffer[512];

  snprintf(buffer, sizeof(buffer), "Available commands:\n");
  for (size_t i = 0; i < CMD_COUNT; i++) {
    strncat(buffer, registry[i].name, sizeof(buffer) - strlen(buffer) - 1);
    strncat(buffer, ": ", sizeof(buffer) - strlen(buffer) - 1);
    strncat(buffer, registry[i].desc, sizeof(buffer) - strlen(buffer) - 1);
    strncat(buffer, "\n", sizeof(buffer) - strlen(buffer) - 1);
  }

  send(context->connection_fd, buffer, strlen(buffer), 0);
}

void execute_command(connection_contex_t *context, char *line) {
  // Basic string tokenizer (splits by space)
  char *argv[10];
  int argc = 0;
  char *token = strtok(line, " \n");

  while (token != NULL && argc < 10) {
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

  printf("Unknown command: %s. Type 'help' for info.\n", argv[0]);
}

void *handle_client_connection(void *arg) {
  connection_contex_t connection_context = *((connection_contex_t *)arg);
  free(arg);

  int fd = connection_context.connection_fd;
  char buffer[1024];
  ssize_t bytes_read;

  printf("[Thread] Handling new connection on FD %d\n", fd);

  // Loop until the client closes the connection or an error occurs
  while ((bytes_read = recv(fd, buffer, sizeof(buffer) - 1, 0)) > 0) {

    // 1. Null-terminate the received data to treat it as a string
    buffer[bytes_read] = '\0';
    printf("[Client %d]: %s", fd, buffer);

    execute_command(&connection_context, buffer);

    if (connection_context.exit_triggered) {
      break;
    }
  }

  if (bytes_read == 0 || connection_context.exit_triggered) {
    printf("[Thread] Client on FD %d disconnected.\n", fd);
  } else if (bytes_read == -1) {
    perror("recv failed");
  }

  close(connection_context.connection_fd);

  pthread_mutex_lock(&connection_count_mutex);
  connection_count--;
  pthread_cond_signal(&connection_slots_available);
  pthread_mutex_unlock(&connection_count_mutex);

  return NULL;
}

int start_server(uint32_t ip, uint16_t port, unsigned int max_connections,
                 unsigned int max_pending_connections) {
  struct sockaddr_in server_addr;
  int listener_fd;
  int new_connection_fd;

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = htonl(ip);

  listener_fd = create_tcp_listener((struct sockaddr *)&server_addr,
                                    max_pending_connections);

  struct sockaddr client_addr;
  socklen_t client_addrlen = sizeof(client_addr);
  while (true) {
    pthread_mutex_lock(&connection_count_mutex);
    while (connection_count >= max_connections) {
      printf("Maximum number of connections reached. Waiting for a connection "
             "to close...\n");
      pthread_cond_wait(&connection_slots_available, &connection_count_mutex);
    }

    new_connection_fd = accept(listener_fd, &client_addr, &client_addrlen);

    connection_count++;
    pthread_mutex_unlock(&connection_count_mutex);

    if (new_connection_fd == -1) {
      char *err_template = "Error: %s (while accepting incoming connection).\n";
      fprintf(stderr, err_template, strerror(errno));
      exit(EXIT_FAILURE);
    } else {
      printf("Successfully accepted incoming connection\n");
    }

    connection_contex_t *client_info = malloc(sizeof(connection_contex_t));
    client_info->connection_fd = new_connection_fd;
    client_info->client_addr = client_addr;
    client_info->client_addrlen = client_addrlen;

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, handle_client_connection, client_info);
    pthread_detach(thread_id);
  }
}

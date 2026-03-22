#include "../include/cli_server.h"
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
  char write_buffer[CONNECTION_WRITE_BUFFER_SIZE];
  snprintf(write_buffer, sizeof(write_buffer), "Hello, client on FD %d!\n",
           context->connection_fd);
  send(context->connection_fd, write_buffer, strlen(write_buffer), 0);
}

void cmd_exit(connection_context_t *context, int argc, char **argv) {
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

// 3. The Actual Registry (Array)
command_t registry[] = {{"hello", "Greets the user", cmd_hello},
                        {"help", "Shows this list", cmd_help},
                        {"exit", "Quits the REPL", cmd_exit}};

#define CMD_COUNT (sizeof(registry) / sizeof(command_t))

void cmd_help(connection_context_t *context, int argc, char **argv) {
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
  ssize_t bytes_read;

  printf("[Server] Handling new connection on FD %d\n", fd);

  // Loop until the client closes the connection or an error occurs
  while ((bytes_read = recv(fd, read_buffer, sizeof(read_buffer) - 1, 0)) > 0) {

    // 1. Null-terminate the received data to treat it as a string
    read_buffer[bytes_read] = '\0';
    printf("[Client %d]: %s", fd, read_buffer);

    execute_command(&context, read_buffer);
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
  listener_context_t args = *((listener_context_t *)arg);
  free(arg);

  int connection_count = 0;
  pthread_mutex_t connection_count_mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t connection_available_cond = PTHREAD_COND_INITIALIZER;

  while (true) {
    pthread_mutex_lock(&connection_count_mutex);
    while (connection_count >= args.max_connections) {
      printf("Maximum number of connections reached. Waiting for a connection "
             "to close...\n");
      pthread_cond_wait(&connection_available_cond, &connection_count_mutex);
    }

    struct sockaddr client_addr;
    socklen_t client_addrlen = sizeof(client_addr);
    int connection_fd = accept(args.listener_fd, &client_addr, &client_addrlen);

    connection_count++;
    pthread_mutex_unlock(&connection_count_mutex);

    if (connection_fd == -1) {
      char *err_template = "Error: %s (while accepting incoming connection).\n";
      fprintf(stderr, err_template, strerror(errno));
      exit(EXIT_FAILURE);
    } else {
      printf("Successfully accepted incoming connection\n");
    }

    connection_context_t *context = malloc(sizeof(connection_context_t));
    context->connection_fd = connection_fd;
    context->client_addr = client_addr;
    context->client_addrlen = client_addrlen;
    context->connection_count = &connection_count;
    context->connection_count_mutex = &connection_count_mutex;
    context->connection_available_cond = &connection_available_cond;

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, handle_connection, context);
    pthread_detach(thread_id);
  }
}

int start_cli_server(pthread_t *thread_id, uint32_t ip, uint16_t port,
                     unsigned int max_connections,
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

  pthread_create(thread_id, NULL, handle_listener, context);

  return 0;
}

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

int connection_count = 0;
pthread_mutex_t connection_count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t connection_slots_available = PTHREAD_COND_INITIALIZER;

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

void *handle_client_connection(void *arg) {
  client_info_t client_info = *((client_info_t *)arg);
  free(arg);

  int fd = client_info.connection_fd;
  char buffer[1024];
  ssize_t bytes_read;

  printf("[Thread] Handling new connection on FD %d\n", fd);

  // Loop until the client closes the connection or an error occurs
  while ((bytes_read = recv(fd, buffer, sizeof(buffer) - 1, 0)) > 0) {

    // 1. Null-terminate the received data to treat it as a string
    buffer[bytes_read] = '\0';

    // 2. Echo it back to the client
    // We use bytes_read to ensure we send exactly what we got
    send(fd, buffer, bytes_read, 0);

    // 3. (Optional) Log to server console
    printf("[Client %d]: %s", fd, buffer);
  }

  if (bytes_read == 0) {
    printf("[Thread] Client on FD %d disconnected.\n", fd);
  } else if (bytes_read == -1) {
    perror("recv failed");
  }

  close(client_info.connection_fd);

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

    client_info_t *client_info = malloc(sizeof(client_info_t));
    client_info->connection_fd = new_connection_fd;
    client_info->client_addr = client_addr;
    client_info->client_addrlen = client_addrlen;

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, handle_client_connection, client_info);
    pthread_detach(thread_id);
  }
}

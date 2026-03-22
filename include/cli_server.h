#ifndef CLI_SERVER_H
#define CLI_SERVER_H

#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/socket.h>

int start_cli_server(pthread_t *thread_id, uint32_t ip, uint16_t port,
                     unsigned int max_connections,
                     unsigned int max_pending_connections);

typedef struct {
  int connection_fd;
  struct sockaddr client_addr;
  socklen_t client_addrlen;
  int *connection_count;
  pthread_mutex_t *connection_count_mutex;
  pthread_cond_t *connection_available_cond;
} connection_context_t;

typedef struct {
  int listener_fd;
  int max_connections;
} listener_context_t;

#endif // CLI_SERVER_H

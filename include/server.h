#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <stdint.h>
#include <sys/socket.h>

int start_server(uint32_t ip, uint16_t port, unsigned int max_connections,
                 unsigned int max_pending_connections);

typedef struct {
  int connection_fd;
  struct sockaddr client_addr;
  socklen_t client_addrlen;
} client_info_t;

#endif // SERVER_H

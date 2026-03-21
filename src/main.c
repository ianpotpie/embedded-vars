#include "../include/server.h"
#include <errno.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

int main(int argc, char *argv[]) {
  const uint32_t ip = INADDR_ANY; // Listens on all interfaces (any IP)
  const uint16_t port = 8080;
  const unsigned int max_connections = 5;
  const unsigned int max_pending_connections = 2;

  start_server(ip, port, max_connections, max_pending_connections);

  return EXIT_SUCCESS;
}

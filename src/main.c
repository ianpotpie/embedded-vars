#include "../include/cli_server.h"
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>

int main(int argc, char *argv[]) {
  const uint32_t ip = INADDR_ANY; // Listens on all interfaces (any IP)
  const uint16_t port = 8080;
  const unsigned int max_connections = 5;
  const unsigned int max_pending_connections = 2;

  pthread_t cli_thread_id;
  start_cli_server(&cli_thread_id, ip, port, max_connections,
                   max_pending_connections);
  pthread_join(cli_thread_id, NULL);

  return EXIT_SUCCESS;
}

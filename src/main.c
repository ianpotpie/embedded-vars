#include "../include/cli_server.h"
#include "../include/var_registry.h"
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <time.h>

int main(int argc, char *argv[]) {
  const uint32_t ip = INADDR_ANY; // Listens on all interfaces (any IP)
  const uint16_t port = 8080;
  const unsigned int max_connections = 5;
  const unsigned int max_pending_connections = 2;

  var_registry_t var_registry;
  var_t registry_entries[100];
  init_registry(&var_registry, "./save_file.txt", registry_entries,
                sizeof(registry_entries));

  float stable_float = 3.14;
  uint16_t stable_int = 5;
  bool my_bool = true;
  int64_t my_time;
  int32_t rand_num;
  time(&my_time);

  register_bool(&var_registry, "/stable/my_bool", true, &my_bool);
  register_float(&var_registry, "/stable/stable_float", true, &stable_float);
  register_uint16(&var_registry, "/stable/stable_int", true, &stable_int);
  register_int64(&var_registry, "/time/my_time", false, &my_time);
  register_int32(&var_registry, "/rand/rand_num", false, &rand_num);

  pthread_t cli_thread_id;
  start_cli_server(&var_registry, &cli_thread_id, ip, port, max_connections,
                   max_pending_connections);

  while (true) {
    time(&my_time);
    rand_num = rand();
  }

  pthread_join(cli_thread_id, NULL);

  return EXIT_SUCCESS;
}

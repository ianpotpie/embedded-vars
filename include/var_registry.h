#ifndef VAR_REGISTRY_H
#define VAR_REGISTRY_H

#include "var.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
  char *save_path;
  var_t *entries;
  size_t count;
  size_t capacity;
} var_registry_t;

void init_registry(var_registry_t *registry, char *save_path, var_t *entries,
                   size_t capacity);
void save_registry(var_registry_t *registry);
void load_persistents(var_registry_t *registry);

// TODO: these should populate a pointer instead of returning
var_t *get_registry_entry_by_name(var_registry_t *registry, char *name);
var_t *get_registry_entry_by_index(var_registry_t *registry, size_t index);

// Regitration functions
int register_bool(var_registry_t *registry, char *name, bool is_persistent,
                  bool *val);
int register_char(var_registry_t *registry, char *name, bool is_persistent,
                  char *val);
int register_int8(var_registry_t *registry, char *name, bool is_persistent,
                  int8_t *val);
int register_int16(var_registry_t *registry, char *name, bool is_persistent,
                   int16_t *val);
int register_int32(var_registry_t *registry, char *name, bool is_persistent,
                   int32_t *val);
int register_int64(var_registry_t *registry, char *name, bool is_persistent,
                   int64_t *val);
int register_uint8(var_registry_t *registry, char *name, bool is_persistent,
                   uint8_t *val);
int register_uint16(var_registry_t *registry, char *name, bool is_persistent,
                    uint16_t *val);
int register_uint32(var_registry_t *registry, char *name, bool is_persistent,
                    uint32_t *val);
int register_uint64(var_registry_t *registry, char *name, bool is_persistent,
                    uint64_t *val);
int register_float(var_registry_t *registry, char *name, bool is_persistent,
                   float *val);
int register_double(var_registry_t *registry, char *name, bool is_persistent,
                    double *val);
int register_bool_arr(var_registry_t *registry, char *name, bool is_persistent,
                      bool **arr, size_t length);
int register_char_arr(var_registry_t *registry, char *name, bool is_persistent,
                      char **arr, size_t length);
int register_int8_arr(var_registry_t *registry, char *name, bool is_persistent,
                      int8_t **arr, size_t length);
int register_int16_arr(var_registry_t *registry, char *name, bool is_persistent,
                       int16_t **arr, size_t length);
int register_int32_arr(var_registry_t *registry, char *name, bool is_persistent,
                       int32_t **arr, size_t length);
int register_int64_arr(var_registry_t *registry, char *name, bool is_persistent,
                       int64_t **arr, size_t length);
int register_uint8_arr(var_registry_t *registry, char *name, bool is_persistent,
                       uint8_t **arr, size_t length);
int register_uint16_arr(var_registry_t *registry, char *name,
                        bool is_persistent, uint16_t **arr, size_t length);
int register_uint32_arr(var_registry_t *registry, char *name,
                        bool is_persistent, uint32_t **arr, size_t length);
int register_uint64_arr(var_registry_t *registry, char *name,
                        bool is_persistent, uint64_t **arr, size_t length);
int register_float_arr(var_registry_t *registry, char *name, bool is_persistent,
                       float **arr, size_t length);
int register_double_arr(var_registry_t *registry, char *name,
                        bool is_persistent, double **arr, size_t length);

#endif // VAR_REGISTRY_H

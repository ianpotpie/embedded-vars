#include "../include/var_registry.h"
#include <pthread.h>
#include <string.h>

void init_registry(var_registry_t *registry, char *save_path, var_t *entries,
                   size_t capacity) {
  registry->entries = entries;
  registry->count = 0;
  registry->capacity = capacity;
  registry->save_path = save_path;
}

void load_persistents(var_registry_t *registry) {
  // TODO: Implement loading from file
}

void save_registry(var_registry_t *registry) {
  // TODO: Implement saving to file
}

int register_var(var_registry_t *registry, var_t var) {
  if (registry->count >= registry->capacity) {
    return -1;
  }

  // Binary search to find the correct insertion point
  // Then shift elements to the right and insert the new item
  size_t left = 0;
  size_t right = registry->count;

  while (left < right) {
    size_t mid = left + (right - left) / 2;
    const char *mid_name = registry->entries[mid].name;

    int cmp = strcmp(mid_name, var.name);
    if (cmp < 0) {
      left = mid + 1;
    } else {
      right = mid;
    }
  }

  // Shift elements to the right to make space for the new item
  for (size_t i = registry->count; i > left; i--) {
    registry->entries[i] = registry->entries[i - 1];
  }

  registry->entries[left] = var;
  registry->count++;

  return 0;
}

var_t create_var(var_type type, char *name, bool is_persistent) {
  var_t var = {.type = type,
               .name = name,
               .is_persistent = is_persistent,
               .mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER,
               .arr_length = 0};
  return var;
}

int register_bool(var_registry_t *registry, char *name, bool is_persistent,
                  bool *val) {
  var_t var = create_var(VAR_TYPE_BOOL, name, is_persistent);
  var.value.bool_val = val;
  return register_var(registry, var);
}

int register_char(var_registry_t *registry, char *name, bool is_persistent,
                  char *val) {
  var_t var = create_var(VAR_TYPE_CHAR, name, is_persistent);
  var.value.char_val = val;
  return register_var(registry, var);
}

int register_int8(var_registry_t *registry, char *name, bool is_persistent,
                  int8_t *val) {
  var_t var = create_var(VAR_TYPE_INT8, name, is_persistent);
  var.value.int8_val = val;
  return register_var(registry, var);
}

int register_int16(var_registry_t *registry, char *name, bool is_persistent,
                   int16_t *val) {
  var_t var = create_var(VAR_TYPE_INT16, name, is_persistent);
  var.value.int16_val = val;
  return register_var(registry, var);
}

int register_int32(var_registry_t *registry, char *name, bool is_persistent,
                   int32_t *val) {
  var_t var = create_var(VAR_TYPE_INT32, name, is_persistent);
  var.value.int32_val = val;
  return register_var(registry, var);
}

int register_int64(var_registry_t *registry, char *name, bool is_persistent,
                   int64_t *val) {
  var_t var = create_var(VAR_TYPE_INT64, name, is_persistent);
  var.value.int64_val = val;
  return register_var(registry, var);
}

int register_uint8(var_registry_t *registry, char *name, bool is_persistent,
                   uint8_t *val) {
  var_t var = create_var(VAR_TYPE_UINT8, name, is_persistent);
  var.value.uint8_val = val;
  return register_var(registry, var);
}

int register_uint16(var_registry_t *registry, char *name, bool is_persistent,
                    uint16_t *val) {
  var_t var = create_var(VAR_TYPE_UINT16, name, is_persistent);
  var.value.uint16_val = val;
  return register_var(registry, var);
}

int register_uint32(var_registry_t *registry, char *name, bool is_persistent,
                    uint32_t *val) {
  var_t var = create_var(VAR_TYPE_UINT32, name, is_persistent);
  var.value.uint32_val = val;
  return register_var(registry, var);
}

int register_uint64(var_registry_t *registry, char *name, bool is_persistent,
                    uint64_t *val) {
  var_t var = create_var(VAR_TYPE_UINT64, name, is_persistent);
  var.value.uint64_val = val;
  return register_var(registry, var);
}

int register_float(var_registry_t *registry, char *name, bool is_persistent,
                   float *val) {
  var_t var = create_var(VAR_TYPE_FLOAT, name, is_persistent);
  var.value.float_val = val;
  return register_var(registry, var);
}

int register_double(var_registry_t *registry, char *name, bool is_persistent,
                    double *val) {
  var_t var = create_var(VAR_TYPE_DOUBLE, name, is_persistent);
  var.value.double_val = val;
  return register_var(registry, var);
}

int register_bool_arr(var_registry_t *registry, char *name, bool is_persistent,
                      bool **arr, size_t arr_length) {
  var_t var = create_var(VAR_TYPE_BOOL_ARR, name, is_persistent);
  var.value.bool_arr = arr;
  var.arr_length = arr_length;
  return register_var(registry, var);
}

int register_char_arr(var_registry_t *registry, char *name, bool is_persistent,
                      char **arr, size_t arr_length) {
  var_t var = create_var(VAR_TYPE_CHAR_ARR, name, is_persistent);
  var.value.char_arr = arr;
  var.arr_length = arr_length;
  return register_var(registry, var);
}

int register_int8_arr(var_registry_t *registry, char *name, bool is_persistent,
                      int8_t **arr, size_t arr_length) {
  var_t var = create_var(VAR_TYPE_INT8_ARR, name, is_persistent);
  var.value.int8_arr = arr;
  var.arr_length = arr_length;
  return register_var(registry, var);
}

int register_int16_arr(var_registry_t *registry, char *name, bool is_persistent,
                       int16_t **arr, size_t arr_length) {
  var_t var = create_var(VAR_TYPE_INT16_ARR, name, is_persistent);
  var.value.int16_arr = arr;
  var.arr_length = arr_length;
  return register_var(registry, var);
}

int register_int32_arr(var_registry_t *registry, char *name, bool is_persistent,
                       int32_t **arr, size_t arr_length) {
  var_t var = create_var(VAR_TYPE_INT32_ARR, name, is_persistent);
  var.value.int32_arr = arr;
  var.arr_length = arr_length;
  return register_var(registry, var);
}

int register_int64_arr(var_registry_t *registry, char *name, bool is_persistent,
                       int64_t **arr, size_t arr_length) {
  var_t var = create_var(VAR_TYPE_INT64_ARR, name, is_persistent);
  var.value.int64_arr = arr;
  var.arr_length = arr_length;
  return register_var(registry, var);
}

int register_uint8_arr(var_registry_t *registry, char *name, bool is_persistent,
                       uint8_t **arr, size_t arr_length) {
  var_t var = create_var(VAR_TYPE_UINT8_ARR, name, is_persistent);
  var.value.uint8_arr = arr;
  var.arr_length = arr_length;
  return register_var(registry, var);
}

int register_uint16_arr(var_registry_t *registry, char *name,
                        bool is_persistent, uint16_t **arr, size_t arr_length) {
  var_t var = create_var(VAR_TYPE_UINT16_ARR, name, is_persistent);
  var.value.uint16_arr = arr;
  var.arr_length = arr_length;
  return register_var(registry, var);
}

int register_uint32_arr(var_registry_t *registry, char *name,
                        bool is_persistent, uint32_t **arr, size_t arr_length) {
  var_t var = create_var(VAR_TYPE_UINT32_ARR, name, is_persistent);
  var.value.uint32_arr = arr;
  var.arr_length = arr_length;
  return register_var(registry, var);
}

int register_uint64_arr(var_registry_t *registry, char *name,
                        bool is_persistent, uint64_t **arr, size_t arr_length) {
  var_t var = create_var(VAR_TYPE_UINT64_ARR, name, is_persistent);
  var.value.uint64_arr = arr;
  var.arr_length = arr_length;
  return register_var(registry, var);
}

int register_float_arr(var_registry_t *registry, char *name, bool is_persistent,
                       float **arr, size_t arr_length) {
  var_t var = create_var(VAR_TYPE_FLOAT_ARR, name, is_persistent);
  var.value.float_arr = arr;
  var.arr_length = arr_length;
  return register_var(registry, var);
}

int register_double_arr(char *name, bool is_persistent, double **arr,
                        size_t arr_length) {
  var_t var = create_var(VAR_TYPE_DOUBLE_ARR, name, is_persistent);
  var.value.double_arr = arr;
  var.arr_length = arr_length;
  return register_var(registry, var);
}

var_t *get_storage_item_by_path(char *name) {
  if (global_registry->count == 0) {
    return NULL; // No items in the registry
  }

  // Perform a binary search since the items are sorted by path
  size_t left = 0;
  size_t right = global_registry->count;

  while (left <= right) {

    size_t mid = left + (right - left) / 2;
    const char *mid_path = global_registry->items[mid].path;

    int cmp = strcmp(mid_path, path);
    if (cmp == 0) {
      return &global_registry->items[mid]; // Found the item
    } else if (cmp < 0) {
      left = mid + 1; // Search in the right half
    } else {
      right = mid - 1; // Search in the left half
    }
  }

  return NULL; // Item not found
}

var_t *get_storage_item_by_index(size_t index) {
  if (index >= global_registry->count) {
    return NULL; // Index out of bounds
  }
  return &global_registry->items[index];
}

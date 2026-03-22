#ifndef VAR_H
#define VAR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
  VAR_TYPE_BOOL,
  VAR_TYPE_CHAR,
  VAR_TYPE_INT8,
  VAR_TYPE_INT16,
  VAR_TYPE_INT32,
  VAR_TYPE_INT64,
  VAR_TYPE_UINT8,
  VAR_TYPE_UINT16,
  VAR_TYPE_UINT32,
  VAR_TYPE_UINT64,
  VAR_TYPE_FLOAT,
  VAR_TYPE_DOUBLE,
  VAR_TYPE_BOOL_ARR,
  VAR_TYPE_CHAR_ARR,
  VAR_TYPE_INT8_ARR,
  VAR_TYPE_INT16_ARR,
  VAR_TYPE_INT32_ARR,
  VAR_TYPE_INT64_ARR,
  VAR_TYPE_UINT8_ARR,
  VAR_TYPE_UINT16_ARR,
  VAR_TYPE_UINT32_ARR,
  VAR_TYPE_UINT64_ARR,
  VAR_TYPE_FLOAT_ARR,
  VAR_TYPE_DOUBLE_ARR,
  VAR_TYPE_COUNT
} var_type;

// TODO: should have a mutex to synchronize access eventually
typedef struct {
  var_type type;
  const char *name;
  bool is_persistent;
  size_t arr_length; // For arrays, 0 for single variables
  union {
    bool *bool_val;
    char *char_val;
    uint8_t *uint8_val;
    uint16_t *uint16_val;
    uint32_t *uint32_val;
    uint64_t *uint64_val;
    int8_t *int8_val;
    int16_t *int16_val;
    int32_t *int32_val;
    int64_t *int64_val;
    float *float_val;
    double *double_val;

    bool **bool_arr;
    char **char_arr;
    uint8_t **uint8_arr;
    uint16_t **uint16_arr;
    uint32_t **uint32_arr;
    uint64_t **uint64_arr;
    int8_t **int8_arr;
    int16_t **int16_arr;
    int32_t **int32_arr;
    int64_t **int64_arr;
    float **float_arr;
    double **double_arr;
  } value;
} var_t;

char *var_type_to_string(var_type type);
int var_value_as_string(var_t *var, char *value_buf, size_t buf_size);
int var_to_json(var_t *var, char *json_buf, size_t json_size, char *val_buf,
                size_t val_size);

#endif // VAR_H

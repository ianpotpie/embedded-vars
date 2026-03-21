#include "../include/var.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>

char *var_type_to_string(var_type type) {
  switch (type) {
  case VAR_TYPE_BOOL:
    return "bool";
  case VAR_TYPE_CHAR:
    return "char";
  case VAR_TYPE_INT8:
    return "int8";
  case VAR_TYPE_INT16:
    return "int16";
  case VAR_TYPE_INT32:
    return "int32";
  case VAR_TYPE_INT64:
    return "int64";
  case VAR_TYPE_UINT8:
    return "uint8";
  case VAR_TYPE_UINT16:
    return "uint16";
  case VAR_TYPE_UINT32:
    return "uint32";
  case VAR_TYPE_UINT64:
    return "uint64";
  case VAR_TYPE_FLOAT:
    return "float";
  case VAR_TYPE_DOUBLE:
    return "double";
  case VAR_TYPE_BOOL_ARR:
    return "bool[]";
  case VAR_TYPE_CHAR_ARR:
    return "char[]";
  case VAR_TYPE_INT8_ARR:
    return "int8[]";
  case VAR_TYPE_INT16_ARR:
    return "int16[]";
  case VAR_TYPE_INT32_ARR:
    return "int32[]";
  case VAR_TYPE_INT64_ARR:
    return "int64[]";
  case VAR_TYPE_UINT8_ARR:
    return "uint8[]";
  case VAR_TYPE_UINT16_ARR:
    return "uint16[]";
  case VAR_TYPE_UINT32_ARR:
    return "uint32[]";
  case VAR_TYPE_UINT64_ARR:
    return "uint64[]";
  case VAR_TYPE_FLOAT_ARR:
    return "float[]";
  case VAR_TYPE_DOUBLE_ARR:
    return "double[]";
  default:
    return "unknown";
  }
}

var_type string_to_var_type(const char *type_str) {
  for (int type = 0; type < VAR_TYPE_COUNT; type++) {
    if (strcmp(type_str, var_type_to_string(type)) == 0) {
      return type;
    }
  }
  return -1; // Invalid type string
}

// returns the number of character attempted to write, excluding the null
// terminator. If the output was truncated, then return will be bigger than
// buf_size
int var_value_as_string(var_t *var, char *value_buf, size_t buf_size) {
  // TODO: Return value is not as described for array types
  switch (var->type) {
  case VAR_TYPE_BOOL:
    return snprintf(value_buf, buf_size, "%s",
                    *(var->value.bool_val) ? "true" : "false");
  case VAR_TYPE_CHAR:
    return snprintf(value_buf, buf_size, "%c", *(var->value.char_val));
  case VAR_TYPE_INT8:
    return snprintf(value_buf, buf_size, "%d", *(var->value.int8_val));
  case VAR_TYPE_INT16:
    return snprintf(value_buf, buf_size, "%d", *(var->value.int16_val));
  case VAR_TYPE_INT32:
    return snprintf(value_buf, buf_size, "%d", *(var->value.int32_val));
  case VAR_TYPE_INT64:
    return snprintf(value_buf, buf_size, "%lld",
                    (long long)*(var->value.int64_val));
  case VAR_TYPE_UINT8:
    return snprintf(value_buf, buf_size, "%u", *(var->value.uint8_val));
  case VAR_TYPE_UINT16:
    return snprintf(value_buf, buf_size, "%u", *(var->value.uint16_val));
  case VAR_TYPE_UINT32:
    return snprintf(value_buf, buf_size, "%u", *(var->value.uint32_val));
  case VAR_TYPE_UINT64:
    return snprintf(value_buf, buf_size, "%llu",
                    (unsigned long long)*(var->value.uint64_val));
  case VAR_TYPE_FLOAT:
    return snprintf(value_buf, buf_size, "%f", *(var->value.float_val));
  case VAR_TYPE_DOUBLE:
    return snprintf(value_buf, buf_size, "%lf", *(var->value.double_val));
  case VAR_TYPE_BOOL_ARR:
    snprintf(value_buf, buf_size, "[");
    for (size_t i = 0; i < var->arr_length; i++) {
      snprintf(value_buf + strlen(value_buf), buf_size - strlen(value_buf),
               "%s%s", *(var->value.bool_arr[i]) ? "true" : "false",
               (i < var->arr_length - 1) ? ", " : "");
    }
    snprintf(value_buf + strlen(value_buf), buf_size - strlen(value_buf), "]");
  case VAR_TYPE_CHAR_ARR:
    snprintf(value_buf, buf_size, "[");
    for (size_t i = 0; i < var->arr_length; i++) {
      snprintf(value_buf + strlen(value_buf), buf_size - strlen(value_buf),
               "\'%c\'%s", *(var->value.char_arr[i]),
               (i < var->arr_length - 1) ? ", " : "");
    }
    snprintf(value_buf + strlen(value_buf), buf_size - strlen(value_buf), "]");
    return 0;
  case VAR_TYPE_INT8_ARR:
    snprintf(value_buf, buf_size, "[");
    for (size_t i = 0; i < var->arr_length; i++) {
      snprintf(value_buf + strlen(value_buf), buf_size - strlen(value_buf),
               "%d%s", *(var->value.int8_arr[i]),
               (i < var->arr_length - 1) ? ", " : "");
    }
    snprintf(value_buf + strlen(value_buf), buf_size - strlen(value_buf), "]");
    return 0;
  case VAR_TYPE_INT16_ARR:
    snprintf(value_buf, buf_size, "[");
    for (size_t i = 0; i < var->arr_length; i++) {
      snprintf(value_buf + strlen(value_buf), buf_size - strlen(value_buf),
               "%d%s", *(var->value.int16_arr[i]),
               (i < var->arr_length - 1) ? ", " : "");
    }
    snprintf(value_buf + strlen(value_buf), buf_size - strlen(value_buf), "]");
    return 0;
  case VAR_TYPE_INT32_ARR:
    snprintf(value_buf, buf_size, "[");
    for (size_t i = 0; i < var->arr_length; i++) {
      snprintf(value_buf + strlen(value_buf), buf_size - strlen(value_buf),
               "%d%s", *(var->value.int32_arr[i]),
               (i < var->arr_length - 1) ? ", " : "");
    }
    snprintf(value_buf + strlen(value_buf), buf_size - strlen(value_buf), "]");
    return 0;
  case VAR_TYPE_INT64_ARR:
    snprintf(value_buf, buf_size, "[");
    for (size_t i = 0; i < var->arr_length; i++) {
      snprintf(value_buf + strlen(value_buf), buf_size - strlen(value_buf),
               "%lld%s", (long long)*(var->value.int64_arr[i]),
               (i < var->arr_length - 1) ? ", " : "");
    }
    snprintf(value_buf + strlen(value_buf), buf_size - strlen(value_buf), "]");
    return 0;
  case VAR_TYPE_UINT8_ARR:
    snprintf(value_buf, buf_size, "[");
    for (size_t i = 0; i < var->arr_length; i++) {
      snprintf(value_buf + strlen(value_buf), buf_size - strlen(value_buf),
               "%u%s", *(var->value.uint8_arr[i]),
               (i < var->arr_length - 1) ? ", " : "");
    }
    snprintf(value_buf + strlen(value_buf), buf_size - strlen(value_buf), "]");
    return 0;
  case VAR_TYPE_UINT16_ARR:
    snprintf(value_buf, buf_size, "[");
    for (size_t i = 0; i < var->arr_length; i++) {
      snprintf(value_buf + strlen(value_buf), buf_size - strlen(value_buf),
               "%u%s", *(var->value.uint16_arr[i]),
               (i < var->arr_length - 1) ? ", " : "");
    }
    snprintf(value_buf + strlen(value_buf), buf_size - strlen(value_buf), "]");
    return 0;
  case VAR_TYPE_UINT32_ARR:
    snprintf(value_buf, buf_size, "[");
    for (size_t i = 0; i < var->arr_length; i++) {
      snprintf(value_buf + strlen(value_buf), buf_size - strlen(value_buf),
               "%u%s", *(var->value.uint32_arr[i]),
               (i < var->arr_length - 1) ? ", " : "");
    }
    snprintf(value_buf + strlen(value_buf), buf_size - strlen(value_buf), "]");
    return 0;
  case VAR_TYPE_UINT64_ARR:
    snprintf(value_buf, buf_size, "[");
    for (size_t i = 0; i < var->arr_length; i++) {
      snprintf(value_buf + strlen(value_buf), buf_size - strlen(value_buf),
               "%llu%s", (unsigned long long)*(var->value.uint64_arr[i]),
               (i < var->arr_length - 1) ? ", " : "");
    }
    snprintf(value_buf + strlen(value_buf), buf_size - strlen(value_buf), "]");
    return 0;
  case VAR_TYPE_FLOAT_ARR:
    snprintf(value_buf, buf_size, "[");
    for (size_t i = 0; i < var->arr_length; i++) {
      snprintf(value_buf + strlen(value_buf), buf_size - strlen(value_buf),
               "%f%s", *(var->value.float_arr[i]),
               (i < var->arr_length - 1) ? ", " : "");
    }
    snprintf(value_buf + strlen(value_buf), buf_size - strlen(value_buf), "]");
    return 0;
  case VAR_TYPE_DOUBLE_ARR:
    snprintf(value_buf, buf_size, "[");
    for (size_t i = 0; i < var->arr_length; i++) {
      snprintf(value_buf + strlen(value_buf), buf_size - strlen(value_buf),
               "%lf%s", *(var->value.double_arr[i]),
               (i < var->arr_length - 1) ? ", " : "");
    }
    snprintf(value_buf + strlen(value_buf), buf_size - strlen(value_buf), "]");
    return 0;
  default:
    return -1; // Unsupported type for string conversion
  }
}

int var_to_json(var_t *var, char *json_buf, size_t json_size, char *value_buf,
                size_t value_size) {

  char *template = "{\"name\": \"%s\", \"type\": \"%s\", \"is_persistent\": "
                   "%s, \"value\": %s}";

  var_value_as_string(var, value_buf, value_size);

  snprintf(json_buf, json_size, template, var->name,
           var_type_to_string(var->type), var->is_persistent ? "true" : "false",
           value_buf);

  return -1; // Not implemented
}

#pragma once

#include <stdio.h>
#include <haversine_types.h>
#include <stdarg.h>

#define JSON_PREFIX(name) "{\"" name "\":["
#define JSON_SUFFIX "\n]}\n"

typedef int (*formatter)(char*, size_t str_len, const void* data);

struct JsonBuffer{
  char* data;
  FILE* o_file;
  formatter fmt_f;
  size_t entry_len;
  int num_entry;
  size_t cap;
};

s8 preallocated_json_buffer(
    JsonBuffer* self,
    const char* json_prefix,
    const char* file_name,
    formatter fmt_f,
    size_t num_ele,
    size_t ele_size);

s8 push_entry_at(JsonBuffer* json, size_t i, void* data);
void end_json(JsonBuffer* json);

char* get_entry_json(JsonBuffer* json, u64 i);

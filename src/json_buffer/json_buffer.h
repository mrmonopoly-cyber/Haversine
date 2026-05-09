#pragma once

#include <haversine_types.h>

#define JSON_PREFIX(name) "{\"" name "\":["
#define JSON_SUFFIX "\n]}\n"

struct JsonBuffer{
  char* data;
  char* start_entries;
  char* start_suffix;
  const char* fmt;
  int entry_len;
  size_t cap;
  size_t len;
};

s8 preallocated_json_buffer(
    const char* json_prefix,
    const char* fmt,
    size_t num_ele,
    size_t ele_size,
    JsonBuffer* out
    );
void push_point_entry(JsonBuffer* json, ...);
void end_json(JsonBuffer* json);

char* get_entry_json(JsonBuffer* json, size_t i);

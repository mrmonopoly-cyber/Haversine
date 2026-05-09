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
  int num_entry;
  size_t cap;
};

s8 preallocated_json_buffer(
    const char* json_prefix,
    const char* fmt,
    size_t num_ele,
    size_t ele_size,
    JsonBuffer* out
    );

s8 push_entry_at(JsonBuffer* json, size_t i, ...);
void end_json(JsonBuffer* json);

char* get_entry_json(JsonBuffer* json, u64 i);

#pragma once

#include <haversine_types.h>

#define FMT_DOUBLE "%24.16lf"

#define JSON_PREFIX(name) "{\"" name "\":["
#define FMT_PAIRS \
  "\n{\"x0\":" FMT_DOUBLE", \"y0\":" FMT_DOUBLE ", \"x1\":" FMT_DOUBLE ", \"y1\":" FMT_DOUBLE"}"
#define FMT_SOL "\n" FMT_DOUBLE","
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

struct Point{
  f64 x;
  f64 y;
};

s8 preallocated_json_buffer(
    const char* json_prefix,
    const char* fmt,
    size_t num_ele,
    size_t ele_size,
    JsonBuffer* out
    );
void push_point_in_json(JsonBuffer* json, Point* p1, Point* p2);
void push_double(JsonBuffer* json, f64 d);
void end_json(JsonBuffer* json);

char* get_entry_json(JsonBuffer* json, size_t i);

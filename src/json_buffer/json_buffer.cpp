#include "json_buffer.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

s8 preallocated_json_buffer(
    const char* json_prefix,
    const char* fmt,
    size_t num_ele,
    size_t ele_size,
    JsonBuffer* out)
{
  static char failed_preallocation;
  s8 res =0;
  char* cursor;
  size_t json_len;

  json_len = 
    strlen(json_prefix) + 
    ((ele_size + 1)* num_ele) +
    strlen(JSON_SUFFIX);

  out->data = (char*) calloc(1, json_len);

  if (out->data == nullptr) {
    res = -1;
    out->data =&failed_preallocation;
    out->start_entries = &failed_preallocation;
    out->fmt = "",
    out->cap= 0;
    out->len = 0;
    out->entry_len = 0;
    goto end;
  }

  out->cap = json_len;
  cursor = out->data;
  cursor += snprintf(cursor, &out->data[out->cap-1] - cursor, "%s", json_prefix);
  out->start_entries = cursor;
  out->len = cursor - out->data;
  out->entry_len = ele_size;
  out->fmt = fmt;

end:
  return res;
}

void push_point_in_json(JsonBuffer* json, Point* p1, Point* p2)
{
  char* cursor = json->data + json->len;

  cursor += snprintf(cursor, json->cap - json->len, json->fmt, p1->x, p1->x, p2->x, p2->y);
  cursor += snprintf(cursor, json->cap - json->len, ",");
  json->len = cursor - json->data;
}

void push_double(JsonBuffer* json, f64 d)
{
  char* cursor = json->data + json->len;

  cursor += snprintf(cursor, &json->data[json->cap-1] - cursor, json->fmt, d);
  cursor += snprintf(cursor, &json->data[json->cap-1] - cursor, ",");
  json->len = cursor - json->data;
}

void end_json(JsonBuffer* json)
{
  char* cursor = json->data + json->len -1;

  json->start_suffix = cursor;

  cursor += snprintf(cursor, &json->data[json->cap-1] - cursor, JSON_SUFFIX);
  json->len = cursor - json->data;
}

char* get_entry_json(JsonBuffer* json, size_t i)
{
  static char invalid_entry = '\0';
  char *entry = json->start_entries + ((json->entry_len + 1) * i);

  if(entry < json->data + json->len && entry < json->start_suffix)
  {
    return entry;
  }

  return &invalid_entry;
}

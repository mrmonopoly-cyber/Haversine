#include "json_buffer.h"

#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char invalid_entry = '\0';

int _dummy_formatter(char* str, size_t str_len, const void* data)
{
  int res=0;
  (void)str_len;
  (void)data;
  if(str) *str = '\0';

  return res;
}

s8 preallocated_json_buffer(
    const char* json_prefix,
    formatter fmt_f,
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

  out->data = (char*) malloc(json_len);
  out->fmt_f= (fmt_f == nullptr) ? _dummy_formatter : fmt_f;

  if (out->data == nullptr) {
    res = -1;
    out->data =&failed_preallocation;
    out->start_entries = &failed_preallocation;
    out->cap= 0;
    out->entry_len = 0;
    goto end;
  }

  out->cap = json_len;
  cursor = out->data;
  cursor += snprintf(cursor, out->data + out->cap - cursor, "%s", json_prefix);
  out->start_entries = cursor;
  out->entry_len = ele_size;

end:
  return res;
}

s8 push_entry_at(JsonBuffer* json, size_t i, void* data) 
{
  size_t res=0;
  char* cursor = get_entry_json(json, i);

  if(cursor == &invalid_entry || data == nullptr)
  {
    res=-1;
    goto end;
  }

  res = json->fmt_f(cursor, json->entry_len + 1, data);

  while(res<json->entry_len) cursor[res++] = ' ';

  if(res > json->entry_len){
    res = -1;
    goto end;
  };

  cursor[json->entry_len] = ',';
  res++;

end:
  return res;
}

void end_json(JsonBuffer* json)
{

  char* cursor = json->data + json->cap - strlen(JSON_SUFFIX) - 1;
  json->start_suffix = cursor;

  cursor += snprintf(cursor, &json->data[json->cap-1] - cursor + 1, JSON_SUFFIX);
}

char* get_entry_json(JsonBuffer* json, u64 i)
{
  char *entry = json->start_entries + ((json->entry_len + 1) * i);

  if(entry < json->data + json->cap && (entry < json->start_suffix || !json->start_suffix))
  {
    return entry;
  }

  return &invalid_entry;
}

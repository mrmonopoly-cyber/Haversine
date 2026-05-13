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
    JsonBuffer* self,
    const char* json_prefix,
    const char* file_name,
    formatter fmt_f,
    size_t num_ele,
    size_t ele_size)
{
  static char failed_preallocation;
  static FILE* failed_open_file = stdout;
  s8 res =0;
  size_t json_len;

  json_len =  ((ele_size + 1)* num_ele);

  self->data = (char*) malloc(json_len);
  self->fmt_f= (fmt_f == nullptr) ? _dummy_formatter : fmt_f;

  if (self->data == nullptr) {
    res = -1;
    goto bad;
  }

  self->o_file = fopen(file_name, "w");
  if (self->o_file == nullptr)
  {
    res=-2;
    goto bad;
  }

  self->cap = json_len;
  fprintf(self->o_file, "%s", json_prefix);
  self->entry_len = ele_size;

  return res;

bad:
    self->data =&failed_preallocation;
    self->o_file = failed_open_file;
    self->cap= 0;
    self->entry_len = 0;
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
  static char end_buffer = '\0';

  char* cursor = json->data + json->cap - strlen(JSON_SUFFIX) - 1;
  snprintf(cursor, &json->data[json->cap-1] - cursor + 1, JSON_SUFFIX);


  fwrite(json->data, 1, json->cap, json->o_file);

  fclose(json->o_file);
  free(json->data);
  memset(json, 0, sizeof(*json));
  json->o_file = stdout;
  json->data = &end_buffer;
  json->fmt_f = _dummy_formatter;
}

char* get_entry_json(JsonBuffer* json, u64 i)
{
  char *entry = json->data + ((json->entry_len + 1) * i);

  if(entry < json->data + json->cap)
  {
    return entry;
  }

  return &invalid_entry;
}

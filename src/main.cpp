#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <haversine_types.h>

#include "cli/cli.h"
#include "haversine.h"

#define FMT_DOUBLE "%24.16lf"

#define JSON_PREFIX(name) "{\"" name "\":["
#define FMT_PAIRS \
  "\n{\"x0\":" FMT_DOUBLE", \"y0\":" FMT_DOUBLE ", \"x1\":" FMT_DOUBLE ", \"y1\":" FMT_DOUBLE"},"
#define FMT_SOL "\n" FMT_DOUBLE","
#define JSON_SUFFIX "\n]}\n"

struct JsonBuffer{
  char* data;
  size_t cap;
  size_t len;
};
  
struct Point{
  f64 x;
  f64 y;
};

static inline f64 _new_rand_coordinate(f64 max)
{
  return ((f64) rand() / (f64)RAND_MAX) * max;
}

static inline void _new_point(Point* p, f64 max_coordinate)
{
  p->x = _new_rand_coordinate(max_coordinate);
  p->y = _new_rand_coordinate(max_coordinate);
}

static s8 _create_solution_file(Input* input, FILE** out)
{
  s8 res=0;
  const char sol_prefix[] = "solution_";
  char* sol_path_file = nullptr;
  char* cursor;
  size_t sol_len = 0;

  sol_len = strlen(input->o_file_path) + strlen(sol_prefix);
  sol_path_file = (char*) calloc(sol_len + 1, 1);

  if(sol_path_file == nullptr)
  {
    res= -99;
    printf("error allocating solution path name\n");
    goto bad;
  }
  cursor = strncat(sol_path_file, sol_prefix, sol_len);
  strncat(cursor, input->o_file_path, sol_path_file[sol_len] - (size_t) cursor);
  *out = fopen(sol_path_file, "wa");
  if (*out == nullptr)
  {
    res = -2;
    printf("error opening o_file %s: %s\n", input->o_file_path, strerror(errno));
    goto bad;
  }

  printf("solution_file: %s\n", sol_path_file);
  free(sol_path_file);
  return res;

bad:
  if(sol_path_file)free(sol_path_file);
  *out = stdout;
  return res;
}

static s8 _preallocated_json_buffer(const char* json_prefix, size_t num_ele, size_t ele_size, JsonBuffer* out)
{
  static char failed_preallocation;
  s8 res =0;
  char* cursor;
  size_t json_len;

  json_len = 
    strlen(json_prefix) + 
    (ele_size* num_ele) +
    strlen(JSON_SUFFIX);

  out->data = (char*) calloc(1, json_len);

  if (out->data == nullptr) {
    res = -1;
    out->data =&failed_preallocation;
    out->cap= 0;
    out->len = 0;
    goto end;
  }

  out->cap = json_len;
  cursor = out->data;
  cursor += snprintf(cursor, &out->data[out->cap-1] - cursor, "%s", json_prefix);
  out->len = cursor - out->data;

end:
  return res;
}

static void _push_point_in_json(JsonBuffer* json, Point* p1, Point* p2)
{
  char* cursor = json->data + json->len;

  const u64 max_amount = json->cap - json->len;

  if (max_amount >= json->cap)
  {
    printf("cap: %lu, am: %lu\n", json->cap, max_amount);
    assert(max_amount < json->cap);
  }

  cursor += snprintf(cursor, max_amount, FMT_PAIRS, p1->x, p1->x, p2->x, p2->y);
  json->len = cursor - json->data;
}

static void _push_double(JsonBuffer* json, f64 d)
{
  char* cursor = json->data + json->len;

  cursor += snprintf(cursor, &json->data[json->cap-1] - cursor, FMT_SOL, d);
  json->len = cursor - json->data;
}

static void _end_json(JsonBuffer* json)
{
  char* cursor = json->data + json->len -1;

  cursor += snprintf(cursor, &json->data[json->cap-1] - cursor, JSON_SUFFIX);
  json->len = cursor - json->data;
}

int main(int argc, char *argv[])
{
  s8 res=0;
  Input input;
  Point p1={}, p2={};
  FILE* o_file = stdout;
  FILE* o_file_sol = stdout;
  f64 acc=0;
  f64 temp;
  JsonBuffer json_buffer_out;
  JsonBuffer json_buffer_sol;

  char dummy_pair[128] = {};
  size_t pair_len=0;
  size_t double_len=0;

  sprintf(dummy_pair, FMT_PAIRS, p1.x, p1.y, p2.x, p2.y);
  pair_len = strlen(dummy_pair);

  sprintf(dummy_pair, FMT_SOL, p1.x);
  double_len = strlen(dummy_pair);

  if((res = _parse_args(argc, argv, &input))<0){
    printf("error parsing args: %d\n", res);
    goto end;
  }

  if((res=_preallocated_json_buffer(JSON_PREFIX("pairs"), input.num_points, pair_len, &json_buffer_out))<0){
    printf("failed preallocating memory for json: %d\n", res);
    goto end;
  }

  if((res=_preallocated_json_buffer(JSON_PREFIX("solutions"), input.num_points, double_len, &json_buffer_sol))<0){
    printf("failed preallocating memory for json: %d\n", res);
    goto end;
  }

  if((res =_create_solution_file(&input, &o_file_sol))<0){
    printf("error creating solution file: %d\n", res);
    goto end;
  }

  o_file = fopen(input.o_file_path, "wa");
  if (o_file == nullptr)
  {
    res = -2;
    printf("error opening o_file %s: %s\n", input.o_file_path, strerror(errno));
    goto end;
  }


  for(size_t i=0; i<input.num_points; i++)
  {
    _new_point(&p1, input.num_points);
    _new_point(&p2, input.num_points);
    temp = ReferenceHaversine(p1.x, p1.y, p2.x, p2.y);
    acc+=temp;
    _push_point_in_json(&json_buffer_out, &p1, &p2);
    _push_double(&json_buffer_sol, temp);
  }
  _end_json(&json_buffer_out);
  _end_json(&json_buffer_sol);


  fprintf(o_file,"%s", json_buffer_out.data);
  fprintf(o_file_sol,"%s", json_buffer_sol.data);

  fclose(o_file);
  fclose(o_file_sol);

  printf("expected sum: " FMT_DOUBLE"\n", acc);

end:
  if(json_buffer_out.cap >0) free(json_buffer_out.data);
  if(json_buffer_sol.cap >0) free(json_buffer_sol.data);
  return res;
}

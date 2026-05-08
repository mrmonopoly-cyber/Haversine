#include <cstdlib>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <haversine_types.h>

#include "cli/cli.h"
#include "haversine.h"

#define FMT_DOUBLE "%.16lf"

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

int main(int argc, char *argv[])
{
  s8 res=0;
  Input input;
  Point points[2];
  FILE* o_file = stdout;
  FILE* o_file_sol = stdout;
  f64 acc=0;
  f64 temp;

  if((res = _parse_args(argc, argv, &input))<0){
    printf("error parsing args: %d\n", res);
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


  fprintf(o_file,"{\"pairs\":[");
  fprintf(o_file_sol,"{\"solutions\":[");
  for(size_t i=0; i<input.num_points; i++)
  {
    _new_point(&points[0], input.num_points);
    _new_point(&points[1], input.num_points);
    temp = ReferenceHaversine(points[0].x, points[0].y, points[1].x, points[1].y);
    acc+=temp;

    if(i!=0){
      fprintf(o_file,",");
      fprintf(o_file_sol,",");
    }

    fprintf(o_file_sol, "\n\t" FMT_DOUBLE, temp);

    fprintf(o_file, "\n\t{\"x0\": " FMT_DOUBLE", \"y0\": " FMT_DOUBLE ", \"x1\": " FMT_DOUBLE ", \"y1\": " FMT_DOUBLE"}",
        points[0].x, points[0].y, points[1].x, points[1].y);

  }
  fprintf(o_file, "\n]}\n");
  fprintf(o_file_sol, "\n]}\n");

  fclose(o_file);
  fclose(o_file_sol);

  printf("expected sum: %lf\n", acc);
  
end:
  return res;
}

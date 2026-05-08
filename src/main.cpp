#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <haversine_types.h>

#include "cli/cli.h"

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

int main(int argc, char *argv[])
{
  s8 res=0;
  Input input;
  Point points[2];
  FILE* o_file = stdout;


  if((res = _parse_args(argc, argv, &input))<0){
    goto end;
  }

  srand(!(input.seed) && time(nullptr));

  o_file = fopen(input.o_file_path, "wa");
  if (o_file == nullptr)
  {
    res = -2;
    printf("error opening o_file %s: %s\n", input.o_file_path, strerror(errno));
    goto end;
  }

  fprintf(o_file,"{\"pairs\":[");
  for(size_t i=0; i<input.num_points; i++)
  {
    _new_point(&points[0], input.num_points);
    _new_point(&points[1], input.num_points);

    if(i!=0)fprintf(o_file,",");
    fprintf(o_file, "\n\t{\"x0\": " FMT_DOUBLE", \"y0\": " FMT_DOUBLE ", \"x1\": " FMT_DOUBLE ", \"y1\": " FMT_DOUBLE"}",
        points[0].x, points[0].y, points[1].x, points[1].y);

  }
  fprintf(o_file, "\n]}\n");
  
end:
  if(o_file) fclose(o_file);
  return res;
}

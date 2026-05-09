#include <assert.h>
#include <cstdlib>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#include <haversine_types.h>

#include "cli/cli.h"
#include "json_buffer/json_buffer.h"
#include "point/point.h"
#include "haversine.h"

#define FMT_DOUBLE "%24.16lf"

#define FMT_PAIRS \
  "\n{\"x0\":" FMT_DOUBLE", \"y0\":" FMT_DOUBLE ", \"x1\":" FMT_DOUBLE ", \"y1\":" FMT_DOUBLE"}"
#define FMT_SOL "\n" FMT_DOUBLE

struct JsonFillerWorkerArg{
  JsonBuffer* json_buffer_out;
  JsonBuffer* json_buffer_sol;
  size_t starting_index;
  size_t num_indexes;
  size_t num_points;
  size_t id;
  u64 seed;
};


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
  cursor = sol_path_file;
  cursor += snprintf(cursor, sol_len, "%s", sol_prefix);
  snprintf(cursor, &sol_path_file[sol_len] - cursor + 1, "%s", input->o_file_path);
  *out = fopen(sol_path_file, "w");
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

void* json_filler_worker(void* arg)
{
  s8 res;
  JsonFillerWorkerArg params = *(JsonFillerWorkerArg*)arg;
  Point p1, p2;
  f64 temp;
  union{
    f64 data;
    void* ptr;
  }acc={0};

  printf("worker %ld, %ld -> %ld\n",
      params.id, params.starting_index, params.starting_index + params.num_indexes -1);

  for(size_t i=params.starting_index; i< params.starting_index + params.num_indexes; i++)
  {
    _new_point(&p1, params.seed, i, 0, params.num_points);
    _new_point(&p2, params.seed, i, 1, params.num_points);
    temp = ReferenceHaversine(p1.x, p1.y, p2.x, p2.y);

    acc.data+=temp;

    if((res=push_entry_at(params.json_buffer_out, i, p1.x, p1.y, p2.x, p2.y))<0){
      printf("failed writing pair at index: %ld\n", i);
    }
    if((res=push_entry_at(params.json_buffer_sol, i, temp))<0){
      printf("failed writing at solution index: %ld\n", i);
    }
  }

  return acc.ptr;
}

int main(int argc, char *argv[])
{
  s8 res=0;
  Input input;
  Point p1={}, p2={};
  FILE* o_file = stdout;
  FILE* o_file_sol = stdout;
  f64 acc=0;
  JsonBuffer json_buffer_out;
  JsonBuffer json_buffer_sol;
  pthread_t workers[128];
  JsonFillerWorkerArg workers_args[128];
  size_t section_size;
  f64 res_acc_data = 12;
  union{
    void* ptr;
    f64 data;
  }res_acc ={
    &res_acc_data,
  };

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


  section_size = input.num_points / input.nproc;

  if((res=preallocated_json_buffer(
          JSON_PREFIX("pairs"),
          FMT_PAIRS,
          input.num_points,
          pair_len,
          &json_buffer_out))<0){
    printf("failed preallocating memory for json: %d\n", res);
    goto end;
  }

  if((res=preallocated_json_buffer(
          JSON_PREFIX("solutions"),
          FMT_SOL,
          input.num_points,
          double_len,
          &json_buffer_sol))<0){
    printf("failed preallocating memory for json: %d\n", res);
    goto end;
  }

  if((res =_create_solution_file(&input, &o_file_sol))<0){
    printf("error creating solution file: %d\n", res);
    goto end;
  }

  o_file = fopen(input.o_file_path, "w");
  if (o_file == nullptr)
  {
    res = -2;
    printf("error opening o_file %s: %s\n", input.o_file_path, strerror(errno));
    goto end;
  }

  for(size_t proc = 0; proc<input.nproc; proc++)
  {
    workers_args[proc] = {
      &json_buffer_out,
      &json_buffer_sol,
      proc * section_size,
      ((proc!=input.nproc-1) * section_size) +
        ((proc==input.nproc-1) * (input.num_points - (proc * section_size))),
      input.num_points,
      proc,
      input.seed,
    };
    pthread_create(&workers[proc], NULL, json_filler_worker, &workers_args[proc]);
  }


  for(size_t proc = 0; proc<input.nproc; proc++)
  {
    pthread_join(workers[proc], &res_acc.ptr);
    acc += res_acc.data;
  }

  end_json(&json_buffer_out);
  end_json(&json_buffer_sol);


  fprintf(o_file,"%s", json_buffer_out.data);
  fprintf(o_file_sol,"%s", json_buffer_sol.data);

  fclose(o_file);
  o_file = nullptr;
  fclose(o_file_sol);
  o_file_sol = nullptr;

  printf("expected sum: " FMT_DOUBLE"\n", acc);

end:
  if(json_buffer_out.cap >0) free(json_buffer_out.data);
  if(json_buffer_sol.cap >0) free(json_buffer_sol.data);
  if(o_file) fclose(o_file);
  if(o_file_sol) fclose(o_file_sol);
  return res;
}

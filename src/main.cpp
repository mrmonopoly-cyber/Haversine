#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
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

#include <ryu/ryu.h>

#define FMT_DOUBLE_PRECISION 16
#define FMT_DOUBLE "%21.16lf"

#define ENTRY_NEW_LINE "\n"
#define PAIR_PREFIX ENTRY_NEW_LINE"{"
#define PAIR_ELE(l,n) "\""#l""#n"\":"
#define PAIR_SPACE ", "
#define PAIR_SUFFIX "}"

#define FMT_PAIRS \
  PAIR_PREFIX PAIR_ELE(x,0)FMT_DOUBLE PAIR_SPACE PAIR_ELE(y,0)FMT_DOUBLE PAIR_SPACE PAIR_ELE(x,1)FMT_DOUBLE PAIR_SPACE PAIR_ELE(y,1)FMT_DOUBLE PAIR_SUFFIX

#define FMT_SOL ENTRY_NEW_LINE FMT_DOUBLE

struct JsonFillerWorkerArg{
  JsonBuffer* json_buffer_out;
  JsonBuffer* json_buffer_sol;
  size_t starting_index;
  size_t num_indexes;
  size_t num_points;
  size_t id;
  size_t cluster_size;
  u64 seed;
  bool verbose;

  f64 o_partial_sum;
};

union Ptrf64{
  f64 _f64;
  void* ptr;
};

struct FmtPairInput{
  Point* p1;
  Point* p2;
};

#define TRY_COPY(STR)                             \
  do{                                             \
    size_t len = sizeof(STR) -1;                  \
    if(len <= str_len - (size_t)(cur - str)){     \
      memcpy(cur, STR, len);                      \
      cur+=len;                                   \
    }else{                                        \
      res=-1;                                     \
      goto bad;                                   \
    }                                             \
  }while(0)

#define TRY_D2_FIXED(DATA)                                          \
  do{                                                               \
    written =d2fixed_buffered_n((DATA), FMT_DOUBLE_PRECISION, cur); \
    if(cur - str + written > str_len){                              \
      res=-1;                                                       \
      goto bad;                                                     \
    }                                                               \
    cur+=written;                                                   \
  }while(0);

static int formatter_pair(char* str, size_t str_len, const void* data)
{
  const FmtPairInput* in = (const FmtPairInput*) data;
  int res=0;
  char *cur = str;
  size_t written;


  TRY_COPY(PAIR_PREFIX);

  TRY_COPY(PAIR_ELE(x, 0));
  TRY_D2_FIXED(in->p1->x);
  TRY_COPY(PAIR_SPACE);
  TRY_COPY(PAIR_ELE(y, 0));
  TRY_D2_FIXED(in->p1->y);

  TRY_COPY(PAIR_SPACE);

  TRY_COPY(PAIR_ELE(x, 1));
  TRY_D2_FIXED(in->p2->x);
  TRY_COPY(PAIR_SPACE);
  TRY_COPY(PAIR_ELE(y, 1));
  TRY_D2_FIXED(in->p2->y);

  TRY_COPY(PAIR_SUFFIX);

  res = cur - str;
  return res;

bad:
  res=-1;
  return res;
}

static int formatter_sol(char* str,  size_t str_len, const void* data)
{
  const f64 in = *(const f64*) data;
  int res=0;
  char *cur = str;
  size_t written;

  TRY_COPY(ENTRY_NEW_LINE);
  TRY_D2_FIXED(in);

  res = cur - str;
  return res;

bad:
  res=-1;
  return res;
}

#undef TRY_COPY
#undef TRY_D2_FIXED

void* json_filler_worker(void* arg)
{
  JsonFillerWorkerArg* params = (JsonFillerWorkerArg*)arg;
  Point p1, p2;
  f64 temp = 0.0;
  f64 sum = 0.0;
  size_t cluster=1;
  FmtPairInput in_pair;

  if(params->verbose)
  {
    printf("worker %zu, from %zu to %zu\n",
      params->id, params->starting_index, params->starting_index + params->num_indexes -1);
  }

  for(size_t i=params->starting_index; i< params->starting_index + params->num_indexes; i++)
  {
    cluster = (i / params->cluster_size)+1;
    if(params->verbose) printf("worker: %zu, index: %zu, cluster: %zu\n", params->id, i, cluster);
    _new_point(&p1, cluster * params->seed, i, 0);
    _new_point(&p2, cluster * params->seed, i, 1);

    temp = ReferenceHaversine(p1.x, p1.y, p2.x, p2.y);
    in_pair.p1 = &p1;
    in_pair.p2 = &p2;

    push_entry_at(params->json_buffer_out, i, &in_pair);
    push_entry_at(params->json_buffer_sol, i, &temp);


    sum+=temp;
  }

  params->o_partial_sum=sum;

  return nullptr;
}

int main(int argc, char *argv[])
{
  s8 res=0;
  Input input;
  Point p1={}, p2={};
  f64 acc=0;
  JsonBuffer json_buffer_out;
  JsonBuffer json_buffer_sol;
  pthread_t workers[128];
  JsonFillerWorkerArg workers_args[128];
  char dummy_pair[128] = {};
  size_t section_size;
  size_t cluster_size;
  size_t pair_len;
  size_t double_len;
  size_t sol_len = 0;
  char* sol_path_file = nullptr;
  char* cur;
  const char sol_prefix[] = "solution_";

  sol_len = strlen(input.o_file_path) + strlen(sol_prefix);
  sol_path_file = (char*) calloc(sol_len + 1, 1);
  cur = sol_path_file;
  cur += snprintf(cur, sol_len, "%s", sol_prefix);
  snprintf(cur, &sol_path_file[sol_len] - cur + 1, "%s", input.o_file_path);

  if(sol_path_file == nullptr)
  {
    res= -99;
    printf("error allocating solution path name\n");
    goto end;
  }

  snprintf(dummy_pair, sizeof(dummy_pair), FMT_PAIRS, p1.x, p1.y, p2.x, p2.y);
  pair_len = strlen(dummy_pair);

  snprintf(dummy_pair, sizeof(dummy_pair), FMT_SOL, p1.x);
  double_len = strlen(dummy_pair);

  if((res = _parse_args(argc, argv, &input))<0){
    printf("error parsing args: %d\n", res);
    goto end;
  }


  if (input.nproc > sizeof(workers)/sizeof(workers[0])) {
    printf("reached max num of workers: %zu, capping it to %zu\n",
        input.nproc, sizeof(workers)/sizeof(workers[0]));
    input.nproc = sizeof(workers)/sizeof(workers[0]);
  }

  section_size = input.num_points / input.nproc;
  cluster_size = input.num_points / input.num_clusters;

  if((res=preallocated_json_buffer(
          &json_buffer_out,
          JSON_PREFIX("pairs"),
          input.o_file_path,
          formatter_pair,
          input.num_points,
          pair_len
          ))<0)
  {
    printf("failed preallocating memory for json: %d\n", res);
    goto end;
  }

  if((res=preallocated_json_buffer(
          &json_buffer_sol,
          JSON_PREFIX("solutions"),
          sol_path_file,
          formatter_sol,
          input.num_points,
          double_len
          ))<0)
  {
    printf("failed preallocating memory for json: %d\n", res);
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
      cluster_size,
      input.seed,
      input.verbose,

      0.0,
    };
    if((res=pthread_create(&workers[proc], NULL, json_filler_worker, &workers_args[proc]))){
      printf("%s.%d: error creating workers : %s", __FILE__, __LINE__, strerror(res));
      res =-3;
      goto end;
    }
  }


  for(size_t proc = 0; proc<input.nproc; proc++)
  {
    pthread_join(workers[proc], nullptr);
    acc += workers_args[proc].o_partial_sum;
  }

  end_json(&json_buffer_out);
  end_json(&json_buffer_sol);

  printf("\nexpected sum: " FMT_DOUBLE"\n", acc);

end:
  if(sol_path_file) free(sol_path_file);
  return res;
}

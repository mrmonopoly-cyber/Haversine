#include "cli.h"

#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string_view>

#define STRINGIFY(X) #X

static char* _next_argv(int argc=0, char** argv=nullptr)
{
  static int num_elements = 0;
  static int i=0;
  static char** pool = nullptr;
  char* res=nullptr;

  if(argv){
    pool = argv;
  }

  if(argc > 0)
  {
    num_elements = argc;
    i=1;
    return res;
  }

  if(pool && i < num_elements)
  {
    res = pool[i++];
  }

  return res;
}

static inline void print_input(Input* input)
{
  printf("seed: %zu\n", input->seed);
  printf("num points: %lu\n", input->num_points);
  if (input->num_clusters == 1)
  {
    printf("rand mode : %s\n", STRINGIFY(Uniform));
  }
  else if(input->num_clusters > 0)
  {
    printf("rand mode : %s\n", STRINGIFY(Cluster));
  }
  else
  {
    printf("DEBUG: num_clusters: %zu\n", input->num_clusters);
    assert(0 && "unreachable");
  }
  printf("nproc : %lu\n", input->nproc);
  printf("verbose: %s\n", input->verbose ? "true": "false");

  printf("out file: ");
  if(input->o_file_path)
  {
    printf("%s\n", input->o_file_path);
  }
  else
  {
    printf("stdout\n");
  }
}

static inline void _help(void)
{
  printf("usage: <options> [num_points]\n");
  printf("options: \n");
  printf(TAB_ALIGN_1"-h" TAB_ALIGN_3"print help\n");
  printf(TAB_ALIGN_1"-s [seed]" TAB_ALIGN_2"specify the seed (default is 0)\n");
  printf(TAB_ALIGN_1"-o [path]" TAB_ALIGN_2"specify the output file (" DEFAULT_O_FILE ")\n");
  printf(TAB_ALIGN_1"-r [%s/%s]" TAB_ALIGN_1"specify the distribution pattern to use\n", STRINGIFY(Uniform), STRINGIFY(Cluster));
  printf(TAB_ALIGN_1"-j [nproc]" TAB_ALIGN_2"specify of threads (default all of the cores)\n");
  printf(TAB_ALIGN_1"-v" TAB_ALIGN_3"verbose\n");
}

s8 _parse_args(int argc, char** argv, Input* input)
{
  s8 res =0;
  ::std::string_view sw;
  char* arg;

  *input = {};

  _next_argv(argc, argv);

  while((arg = _next_argv()))
  {
    sw = arg;

#define IF_ARG(STR) else if(!sw.compare(#STR))
    if(!sw.compare("-h"))
    {
      _help();
      exit(0);
    }
    IF_ARG(-s)
    {
      arg = _next_argv();
      sscanf(arg, "%lu", &input->seed);
    }
    IF_ARG(-o)
    {
      input->o_file_path =  _next_argv();
    }
    IF_ARG(-j)
    {
      arg = _next_argv();
      sscanf(arg, "%lu", &input->nproc);
    }
    IF_ARG(-r)
    {
      sw = _next_argv();
      if(!sw.compare(STRINGIFY(Uniform)))
      {
        input->num_clusters = Uniform;
      }
      else if(!sw.compare(STRINGIFY(Cluster)))
      {
        input->num_clusters = Cluster;
      }
      else
      {
        input->num_clusters = Uniform;
        printf("invalid rand mode: %s, expected %s or %s using %s as fallback\n",
            sw.begin(), STRINGIFY(Uniform), STRINGIFY(Cluster), STRINGIFY(Uniform));
      }
    }
    IF_ARG(-v)
    {
      input->verbose = true;
    }
    else
    {
      sscanf(sw.begin(), "%lu", &input->num_points);
    }
#undef IF_ARG
  }

  if (input->nproc == 0)
  {
    input->nproc = sysconf(_SC_NPROCESSORS_ONLN);
    if (input->nproc > input->num_points) {
      input->nproc = input->num_points ? input->num_points : 1;
    }
  }

  if(!input->seed)
  {
    input->seed = (u64) time(nullptr);
  }

  print_input(input);

  return res;
}

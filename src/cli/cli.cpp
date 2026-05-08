#include "cli.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string_view>

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
  printf("seed: %lu\n", input->seed);
  printf("num points: %lu\n", input->num_points);
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

#define IF_ARG(STR) if(!sw.compare(#STR))
    IF_ARG(-h)
    {
      _help();
      res = -1;
      goto end;
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
    else
    {
      sscanf(sw.begin(), "%lu", &input->num_points);
    }
#undef IF_ARG
  }

  if(!input->seed)
  {
    input->seed = (u64) time(nullptr);
  }

  srand(input->seed);

  print_input(input);

end:
  return res;
}

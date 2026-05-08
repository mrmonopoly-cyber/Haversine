#include "cli.h"

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
}

static inline void _help(void)
{
  printf("usage: <options> [num_points > 0]\n");
  printf("options: \n");
  printf(TAB_ALIGN_1"-h" TAB_ALIGN_3"print help\n");
  printf(TAB_ALIGN_1"-s [seed]" TAB_ALIGN_2"specify the seed (default is 0)\n");
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
    else
    {
      sscanf(arg, "%lu", &input->num_points);
    }
#undef IF_ARG
  }
  
  if (input->num_points == 0)
  {
    res = -1;
    printf("invalid input: num points must be > 0\n");
    _help();
    goto end;
  }

  print_input(input);

end:
  return res;
}

#include <stdio.h>
#include <stdlib.h>
#include <string_view>

#include <haversine_types.h>

#define TAB_ALIGN_1 "\t"
#define TAB_ALIGN_2 "\t\t"
#define TAB_ALIGN_3 "\t\t\t"

struct Input{
};

void _help(void)
{
  printf("usage: \n");
  printf("options: \n");
  printf(TAB_ALIGN_1"-h" TAB_ALIGN_3"print help\n");
}

static inline char* _next_argv(int argc=0, char** argv=nullptr)
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


static s8 _parse_args(int argc, char** argv, Input* input)
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
      exit(0);
    }
#undef IF_ARG
  }
  

  return res;
}

int main(int argc, char *argv[])
{
  s8 res=0;
  Input input;

  if((res = _parse_args(argc, argv, &input))<0){
    goto end;
  }


end:
  return res;
}

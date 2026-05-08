#include <haversine_types.h>

#include "cli/cli.h"

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

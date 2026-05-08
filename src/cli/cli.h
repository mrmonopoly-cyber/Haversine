#pragma once

#include <stdio.h>
#include <haversine_types.h>

#define TAB_ALIGN_1 "\t"
#define TAB_ALIGN_2 "\t\t"
#define TAB_ALIGN_3 "\t\t\t"

#define DEFAULT_SEED (0)

struct Input{
  u64 seed = DEFAULT_SEED;
  u64 num_points = 0;
  char* o_file_path;
  FILE* o_file = stdout;
};

s8 _parse_args(int argc, char** argv, Input* input);

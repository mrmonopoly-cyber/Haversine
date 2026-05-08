#pragma once

#include <haversine_types.h>

#define TAB_ALIGN_1 "\t"
#define TAB_ALIGN_2 "\t\t"
#define TAB_ALIGN_3 "\t\t\t"

#define DEFAULT_SEED (0)
#define DEFAULT_O_FILE "out.json"

struct Input{
  u64 seed = DEFAULT_SEED;
  u64 num_points = 0;
  const char* o_file_path = DEFAULT_O_FILE;
};

s8 _parse_args(int argc, char** argv, Input* input);

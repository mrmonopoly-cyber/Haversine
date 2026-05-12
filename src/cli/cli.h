#pragma once

#include "../point/point.h"

#include <haversine_types.h>

#define TAB_ALIGN_1 "\t"
#define TAB_ALIGN_2 "\t\t"
#define TAB_ALIGN_3 "\t\t\t"

#define DEFAULT_SEED (0)
#define DEFAULT_O_FILE "out.json"


struct Input{
  u64 seed = DEFAULT_SEED;
  u64 num_points = 0;
  size_t nproc;
  const char* o_file_path = DEFAULT_O_FILE;
  RandMode rand_mode = Uniform;
};

s8 _parse_args(int argc, char** argv, Input* input);

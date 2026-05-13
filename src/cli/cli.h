#pragma once

#include <haversine_types.h>

enum RandMode : size_t{
  Uniform=1,
  Cluster=5,
};

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
  size_t num_clusters = Uniform;
  bool verbose = false;
};

s8 _parse_args(int argc, char** argv, Input* input);

#pragma once

#include <stdio.h>
#include <stdint.h>

#include <haversine_types.h>

enum RandMode{
  Uniform,
  Cluster,
};

struct Point{
  f64 x;
  f64 y;
};

static inline u64 _splitmix64(u64 x)
{
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

static inline f64 _deterministic_rand(
    u64 global_seed,
    u64 index,
    u64 component,
    f64 max)
{
    u64 v =
        _splitmix64(global_seed ^
                   (index * 4 + component));

    return ((f64)v / (f64)UINT64_MAX) * max;
}

static inline void _new_point(Point* p, u64 seed, u64 i, u64 comp, f64 max_coordinate, RandMode mode)
{
  switch (mode) {
    case Uniform:
      p->x = _deterministic_rand(seed, i, 1<<comp, max_coordinate);
      p->y = _deterministic_rand(seed, i, 1<<(comp+1), max_coordinate);
      break;
    case Cluster:
      perror("non implemented");
      break;
  }
}

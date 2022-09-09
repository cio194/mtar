#pragma once

#include <cstdio>
#include <cstdint>
#include <sys/stat.h>

#define COMPRESS_MAGIC "Ll6)v-LurUR%Prt="
#define COMPRESS_SUFFIX "mzip"

struct MStat {
  mode_t st_mode_;
  uid_t st_uid_;
  gid_t st_gid_;
  time_t st_atime_;
  time_t st_mtime_;
  time_t st_ctime_;
  uint16_t name_size_;
};

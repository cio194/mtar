#pragma once

#include <cstdio>
#include <cstdint>
#include <string>
#include <sys/stat.h>

#define COMPRESS_MAGIC "Ll6)v-LurUR%Prt="
#define COMPRESS_SUFFIX "mzip"

struct CStat {
  mode_t st_mode_;
  uid_t st_uid_;
  gid_t st_gid_;
  time_t st_atime_;
  time_t st_mtime_;
  uint16_t name_len_;
  std::string name_;

  void Set(const struct stat& sb, const std::string& name);
  int Write(FILE *dst);
  int Read(FILE *src);
};

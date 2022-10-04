#pragma once

#include <string>
#include <cstdint>
#include <cstdio>
#include <sys/stat.h>
#include <libgen.h>

#define PACK_SUFFIX "mtar"
#define PACK_MAGIC "nR8[pO9.sN4+dA1*"

struct PStat {
  mode_t st_mode_;
  uid_t st_uid_;
  gid_t st_gid_;
  time_t st_atime_;
  time_t st_mtime_;
  uint64_t num_;  // 目录表示entry_num，文件表示byte_num
  uint16_t name_len_;
  std::string name_;

  void Set(const struct stat &sb, const std::string &name);
  int Write(FILE *dst);
  int Read(FILE *src);
};

std::string GetBasename(const std::string &s);

void PPerror(const std::string &path, const std::string &msg);

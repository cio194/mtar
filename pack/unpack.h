#pragma once

#include "pack_utils.h"
#include <string>
#include <cstdio>
#include <unistd.h>
#include <utime.h>
#include <sys/stat.h>
#include <fcntl.h>

class Unpacker {
public:
  int Unpack(const std::string &src, const std::string &dst_dir);

private:
  int CheckMagic();
  int Unpack();
  int UnpackDir(const PStat &pstat);
  int UnpackReg(const PStat &pstat);
  int UnpackLnk(const PStat &pstat);

private:
  FILE *src_;
  std::string cur_;
};

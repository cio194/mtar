#pragma once

#include "pack_utils.h"
#include <string>
#include <vector>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

class Packer {
public:
  int Pack(const std::vector<std::string> &src_vec, const std::string &dst);
private:
  int Pack(const std::string &path, const std::string &name);
  int PackDir(const std::string &path, const std::string &name,
              const struct stat &sb);
  int PackReg(const std::string &path, const std::string &name,
              const struct stat &sb);
  int PackLnk(const std::string &path, const std::string &name,
              const struct stat &sb);
private:
  FILE *dst_;
};

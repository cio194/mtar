#ifndef MTAR_INTERACT_H
#define MTAR_INTERACT_H

#include "pack.h"
#include "compress.h"
#include "unpack.h"
#include "decompress.h"
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <unordered_set>
#include <string>
#include <random>
#include <sstream>

class Interactor {
public:
  int Interact(const std::vector<std::string> &argv);
  ~Interactor();
private:
  int ParseArg(const std::vector<std::string> &argv);
  // 打包参数检查
  int CheckPack();
  void PrintUsage();
  int GetAbsolutePath(std::string &path);
  int Mkdirp(const std::string &dir);
  std::string GetUUID();
  int GenerateRandomFilename(const std::string &dir, std::string &filename);
  int JudgeTarIncluded(const std::vector<std::string> &srcs,
                       const std::string &tar_dir);
  std::string GetBasename(const std::string &s);
  int GenerateTarFilename(const std::vector<std::string> &srcs,
                          const std::string &tar_dir, std::string &filename);
  // 解包参数检查
  int CheckUnpack();
  int JudgeCompressed(std::string &file);
private:
  // 打包相关参数
  bool m_pack_ = false;
  std::string pack_dir_;
  std::string pack_file_;
  std::string pack_path_;

  bool m_compress_ = false;
  std::string compress_dir_;
  std::string compress_file_;
  std::string compress_path_;

  bool m_target_ = false;
  std::string tar_dir_ = "./";
  std::string tar_file_;
  std::string tar_path_;

  std::vector<std::string> srcs_;

  // 解包相关参数
  std::string src_file_;
  std::string dst_dir_ = "./";

  bool m_decompress_ = false;
  std::string decompress_path_;
};

#endif //MTAR_INTERACT_H

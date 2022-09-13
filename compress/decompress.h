#pragma once

#include "compress_utils.h"
#include "huffman_tree.h"
#include <string>
#include <vector>
#include <cstdio>
#include <unistd.h>
#include <utime.h>

class Decompressor {
public:
  int Decompress(const std::string &src, const std::string &dst_dir);

private:
  int CheckMagic();
  int GetDstFile(const std::string &dst_dir, CStat &cstat);
  int ReadFreq(std::vector<Freq> &freq_vec);
  int DecompressData(const HuffmanTree& tree);

private:
  FILE* src_;
  FILE* dst_;
};

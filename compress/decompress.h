#pragma once

#include "compress_utils.h"
#include "huffman_tree.h"
#include <string>
#include <vector>

class Decompressor {
public:
  Decompressor() = default;
  ~Decompressor() = default;
  int Decompress(const std::string &src, const std::string &dst_dir);

private:
  int CheckMagic(FILE* src);
  FILE *SetDstFileAttribution(FILE* src, const std::string &dst_dir);
  int ReadFreq(FILE* src, std::vector<Freq> &freq_vec);
  int DecompressData(FILE* src, FILE *dst, const HuffmanTree& tree);
};

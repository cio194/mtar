#pragma once

#include "huffman_tree.h"
#include "compress_utils.h"
#include <string>
#include <cstdlib>
#include <unordered_map>
#include <vector>
#include <cstdio>
#include <libgen.h>

class Compressor {
public:
  int Compress(const std::string &src, const std::string &dst);

private:
  int WriteMeta(const std::string &src_str);
  int GetFreq(std::vector<Freq> &freq_vec);
  int WriteFreq(const std::vector<Freq> &freq_vec);
  int WriteData(const std::unordered_map<char, std::string> &coding_map);
private:
  FILE *src_;
  FILE *dst_;
private:
  struct BBuf {
    int buf_ = 0;
    int cur_ = 0;
    FILE *dst_;

    explicit BBuf(FILE *dst) : dst_(dst) {}

    int Write(char b) {
      if (b == '1') SetBit(cur_);
      cur_ = (cur_ + 1) % 8;
      if (cur_ == 0) {
        if (fputc(buf_, dst_) == EOF) {
          perror("fputc dst");
          return -1;
        }
        buf_ = 0;
      }
      return 0;
    }

  private:
    void SetBit(int i) {
      int mask = 1;
      for (int j = 0; j < i; ++j) mask <<= 1;
      buf_ |= mask;
    }
  };
};

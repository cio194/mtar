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
  Compressor() = default;
  ~Compressor() = default;
  int Compress(const std::string &src, const std::string &dst);

public: // test
//private:
  int GetFreq(FILE *src, std::vector<Freq> &freq_vec);
  int WriteMeta(FILE *dst, const std::string &src_str);
  int WriteFreq(FILE *dst, const std::vector<Freq> &freq_vec);
  int WriteData(FILE *src, FILE *dst,
                const std::unordered_map<char, std::string> &coding_map);

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

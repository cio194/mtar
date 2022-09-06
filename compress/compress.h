#pragma once

#include "huffman_tree.h"
#include <string>
#include <cstdlib>
#include <unordered_map>
#include <cstdint>
#include <utility>
#include <vector>
#include <cstdio>

#include <sys/stat.h>
#include <libgen.h>

#define COMPRESS_MAGIC "Ll6)v-LurUR%Prt="
#define COMPRESS_SUFFIX "mzip"

class Compressor {
public:
  Compressor() = default;
  ~Compressor() = default;
  int Compress(const std::string &src, const std::string &dst);

public: // test
//private:
  int GetFreq(std::vector<Freq> &freq_vec, FILE *src);
  int Write(FILE *src, FILE *dst, const std::vector<Freq> &freq_vec,
            const std::unordered_map<char, std::string> &coding_map,
            const std::string &src_str);
  int WriteMeta(FILE *dst, const std::string &src_str);
  int WriteFreq(FILE *dst, const std::vector<Freq> &freq_vec);
  int WriteData(FILE *src, FILE *dst,
                const std::unordered_map<char, std::string> &coding_map);

private:
  struct MStat {
    mode_t st_mode_;
    uid_t st_uid_;
    gid_t st_gid_;
    time_t st_atime_;
    time_t st_mtime_;
    time_t st_ctime_;
    uint16_t name_size_;
  };

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

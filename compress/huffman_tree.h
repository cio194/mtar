#pragma once

#include <unordered_map>
#include <cstdint>
#include <vector>
#include <string>
#include <cstdio>

struct Freq {
  char ch_;
  uint64_t freq_;
};

class HuffmanTree {
public:  // test
  struct HuffmanNode {
    HuffmanNode *p_;
    HuffmanNode *left_;
    HuffmanNode *right_;
    char ch_;
    uint64_t freq_;

    explicit HuffmanNode(char ch = 0, uint64_t freq = 0) : p_(nullptr),
                                                           left_(nullptr),
                                                           right_(nullptr),
                                                           ch_(ch),
                                                           freq_(freq) {}
  };

public:
  explicit HuffmanTree(const std::vector<Freq> &freq_vec);
  ~HuffmanTree();
  void Encode(std::unordered_map<char, std::string> &coding_map);
private:
  void Clear(HuffmanNode *t);
public:  // test
//private:
  HuffmanNode *root_;
  std::vector<HuffmanNode *> leaves_;

public:
  class HuffmanDecoder {
  public:
    HuffmanDecoder(const HuffmanTree &tree, FILE *dst)
            : tree_(tree), dst_(dst), cur_(tree.root_) {}

    int Decode(int c, int len) {
      for (int i = 0; i < len; ++i) {
        if (GetBit(c, i) == 0) {
          cur_ = cur_->left_;
        } else {
          cur_ = cur_->right_;
        }
        // 如果到达叶结点，则写入数据
        if (cur_->left_ == nullptr && cur_->right_ == nullptr) {
          int ch = cur_->ch_;
          if (fputc(ch, dst_) == EOF) {
            perror("fputc");
            return -1;
          }
          cur_ = tree_.root_;
        }
      }
      return 0;
    }

    bool IsRoot() {
      return cur_ == tree_.root_;
    }

  private:
    int GetBit(int c, int i) {
      int mask = 1;
      for (int j = 0; j < i; ++j) mask <<= 1;
      return c & mask;
    }

  private:
    const HuffmanTree &tree_;
    FILE *dst_;
    HuffmanNode *cur_;
  };
};

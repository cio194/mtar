#pragma once

#include <unordered_map>
#include <cstdint>
#include <vector>
#include <string>

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
  HuffmanTree();
  ~HuffmanTree();
  void Encode(const std::vector<Freq> &freq_vec,
              std::unordered_map<char, std::string> &coding_map);
private:
  void Clear(HuffmanNode *t);
public:  // test
//private:
  HuffmanNode *root_;
};
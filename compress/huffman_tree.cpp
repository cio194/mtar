#include "huffman_tree.h"
#include <cstdio>
#include <queue>
#include <algorithm>

HuffmanTree::HuffmanTree(const std::vector<Freq> &freq_vec) : root_(nullptr) {
  for (const auto &freq : freq_vec)
    leaves_.push_back(new HuffmanNode(freq.ch_, freq.freq_));

  // 小顶堆比较函数
  static auto comp = [](HuffmanNode *x, HuffmanNode *y) {
    return x->freq_ > y->freq_;
  };
  std::priority_queue<
          HuffmanNode *, std::vector<HuffmanNode *>, decltype(comp)
  > pq(leaves_.begin(), leaves_.end(), comp);
  for (int i = 1; i < freq_vec.size(); ++i) {
    auto x = new HuffmanNode();
    auto y = pq.top();
    pq.pop();
    auto z = pq.top();
    pq.pop();
    x->left_ = y;
    y->p_ = x;
    x->right_ = z;
    z->p_ = x;
    x->freq_ = y->freq_ + z->freq_;
    pq.push(x);
  }
  root_ = pq.top();
}

void HuffmanTree::Encode(std::unordered_map<char, std::string> &coding_map) {
  std::string coding;
  for (auto node : leaves_) {
    auto ch = node->ch_;
    while (node->p_ != nullptr) {
      coding.push_back(node == node->p_->left_ ? '0' : '1');
      node = node->p_;
    }
    std::reverse(coding.begin(), coding.end());
    coding_map[ch] = coding;
    coding.clear();
  }
}

HuffmanTree::~HuffmanTree() {
  Clear(root_);
}

void HuffmanTree::Clear(HuffmanNode *t) {
  if (t == nullptr) return;
  Clear(t->left_);
  Clear(t->right_);
  delete t;
}
#include "huffman_tree.h"
#include <cstdio>
#include <queue>
#include <algorithm>

HuffmanTree::HuffmanTree() : root_(nullptr) {}

void HuffmanTree::Encode(const std::vector<Freq> &freq_vec,
                        std::unordered_map<char, std::string> &coding_map) {
  std::vector<HuffmanNode*> freq_nodes;
  for (const auto &freq : freq_vec)
    freq_nodes.push_back(new HuffmanNode(freq.ch_, freq.freq_));

  // 小顶堆比较函数
  static auto comp = [](HuffmanNode *x, HuffmanNode *y) {
    return x->freq_ > y->freq_;
  };
  std::priority_queue<
          HuffmanNode *, std::vector<HuffmanNode *>, decltype(comp)
  > pq(freq_nodes.begin(), freq_nodes.end(), comp);
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

  std::string coding;
  for (auto node : freq_nodes) {
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
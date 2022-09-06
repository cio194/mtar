#include "huffman_tree.h"
#include "treeprinter.h"
#include "compress.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstdio>
#include <unordered_map>
#include <iostream>
#include <string>

#define READ_BUFF_SIZE 4096

using namespace std;

int testHuffmanTree() {
  string src = "static/test_huffman_tree.txt";
  FILE *src_file = fopen(src.c_str(), "r");
  if (nullptr == src_file) {
    perror("fopen src");
    return -1;
  }

  Compressor compressor;

  std::vector<Freq> freq_vec;
  if (compressor.GetFreq(freq_vec, src_file) < 0) {
    cout << "get freq failed" << endl;
    return -1;
  }

  std::unordered_map<char, std::string> coding_map;
  HuffmanTree tree;
  tree.Encode(freq_vec, coding_map);

  using TestNode = HuffmanTree::HuffmanNode;
  tp::TreePrinter<TestNode, tp::HuffmanTreeNodeAdapter<TestNode>>
          printer(tree.root_);
  printer.print();
  for (const auto &p : coding_map) {
    printf("%c: %s\n", p.first, p.second.c_str());
  }
}

void testCompress() {
//  string src = "static/[代码大全2中文版.pdf";
  string src = "static/test_compress.txt";
  string dst = "static/test_compress.mzip";
  Compressor compressor;
  compressor.Compress(src, dst);
}

int main() {
//  testHuffmanTree();
  testCompress();
}
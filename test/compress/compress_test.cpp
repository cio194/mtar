#include "treeprinter.h"
#include "compress.h"
#include "decompress.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstdio>
#include <unordered_map>
#include <iostream>
#include <string>

using namespace std;

int testHuffmanTree() {
  string src = "static/test_huffman_tree.txt";
  FILE *src_file = fopen(src.c_str(), "r");
  if (src_file == nullptr) {
    perror("fopen src");
    return -1;
  }

  Compressor compressor;

  std::vector<Freq> freq_vec;
  if (compressor.GetFreq(src_file, freq_vec) < 0) {
    cout << "get freq failed" << endl;
    return -1;
  }

  std::unordered_map<char, std::string> coding_map;
  HuffmanTree tree(freq_vec);
  tree.Encode(coding_map);

  using TestNode = HuffmanTree::HuffmanNode;
  tp::TreePrinter<TestNode, tp::HuffmanTreeNodeAdapter<TestNode>>
          printer(tree.root_);
  printer.print();
  for (const auto &p : coding_map) {
    printf("%c: %s\n", p.first, p.second.c_str());
  }
}

void testCompress() {
  string src = "static/[代码大全2中文版.pdf";
//  string src = "static/test_compress.txt";
  string dst = "static/test_compress.mzip";
  Compressor compressor;
  compressor.Compress(src, dst);
}

void testDecompress() {
  string src = "static/test_compress.mzip";
  string dst = "static/decompressed";
  Decompressor decompressor;
  decompressor.Decompress(src, dst);
}

int main() {
//  testHuffmanTree();
  testCompress();
  testDecompress();
}
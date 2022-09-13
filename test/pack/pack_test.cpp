#include "pack.h"
#include "unpack.h"
#include <string>
#include <vector>
#include <cstdio>

using namespace std;

void testPack() {
  vector<string> src_vec = {"static/a"};
  string dst = "static/static.mpk";
  Packer packer;
  if (packer.Pack(src_vec, dst) < 0) {
    printf("pack failed\n");
  }
}

void testUnpack() {
  string src = "static/static.mpk";
  string dst_dir = "static/unpacked/";
  Unpacker unpacker;
  if(unpacker.Unpack(src, dst_dir)<0) {
    printf("unpack failed\n");
  }
}

int main() {
  testPack();
  testUnpack();
  return 0;
}

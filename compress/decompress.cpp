#include "decompress.h"

int
Decompressor::Decompress(const std::string &src, const std::string &dst_dir,
                         std::string &decompressed) {
  // 打开文件
  src_ = fopen(src.c_str(), "r");
  if (src_ == nullptr) {
    perror("fopen src");
    return -1;
  }

  // 检查魔数
  if (CheckMagic() < 0) {
    printf("error CheckMagic\n");
    return -1;
  }

  // 读取源文件的元信息，创建目标文件
  CStat cstat;
  if (GetDstFile(dst_dir, cstat, decompressed) < 0) {
    printf("error GetDstFile\n");
    return -1;
  }

  // 读取词频表，以构造Huffman树
  std::vector<Freq> freq_vec;
  if (ReadFreq(freq_vec) < 0) {
    printf("error ReadFreq\n");
    return -1;
  }
  HuffmanTree tree(freq_vec);

  // 通过Huffman树，进行解压缩
  if (DecompressData(tree) < 0) {
    printf("error DecompressData\n");
    return -1;
  }

  // 设置目标文件元信息，并关闭文件
  lchown(decompressed.c_str(), cstat.st_uid_, cstat.st_gid_);
  struct utimbuf utb;
  utb.actime = cstat.st_atime_;
  utb.modtime = cstat.st_mtime_;
  utime(decompressed.c_str(), &utb);
  fclose(src_);
  fclose(dst_);
  return 0;
}

int Decompressor::CheckMagic() {
  static const std::string magic = COMPRESS_MAGIC;

  auto m_len = magic.size();
  std::string s(m_len, ' ');
  if (fread(const_cast<char *>(s.c_str()), 1, m_len, src_) != m_len) {
    perror("fread magic");
    return -1;
  }

  if (s != magic) return -1;
  return 0;
}

int Decompressor::ReadFreq(std::vector<Freq> &freq_vec) {
  int n;
  if (fread(&n, sizeof(n), 1, src_) != 1) {
    perror("fread");
    return -1;
  }

  Freq freq;
  int c;
  for (int i = 0; i < n; ++i) {
    if ((c = fgetc(src_)) == EOF) {
      perror("fgetc");
      return -1;
    }
    freq.ch_ = c;
    if (fread(&freq.freq_, sizeof(freq.freq_), 1, src_) != 1) {
      perror("fread");
      return -1;
    }
    freq_vec.push_back(freq);
  }
  return 0;
}

int
Decompressor::DecompressData(const HuffmanTree &tree) {
  // 读取末尾字节
  int tail_len, tail_b;
  if ((tail_len = fgetc(src_)) == EOF) {
    perror("fgetc");
    return -1;
  }
  if ((tail_b = fgetc(src_)) == EOF) {
    perror("fgetc");
    return -1;
  }
  // 数据解码
  int c;
  HuffmanTree::HuffmanDecoder decoder(tree, dst_);
  while ((c = fgetc(src_)) != EOF) {
    if (decoder.Decode(c, 8) < 0) return -1;
  }
  if (ferror(src_)) {
    perror("fgetc");
    return -1;
  }
  if (decoder.Decode(tail_b, tail_len) < 0) return -1;
  if (!decoder.IsRoot()) {
    printf("tail byte error\n");
    return -1;
  }
  return 0;
}

int Decompressor::GetDstFile(const std::string &dst_dir, CStat &cstat,
                             std::string &decompressed) {
  if (cstat.Read(src_) < 0) return -1;

  // todo 解压文件已存在的处理
  decompressed = dst_dir + cstat.name_;
  dst_ = fopen(decompressed.c_str(), "w");
  if (dst_ == nullptr) {
    perror("fopen dst");
    return -1;
  }
  return 0;
}

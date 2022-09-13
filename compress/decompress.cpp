#include "decompress.h"

int
Decompressor::Decompress(const std::string &src, const std::string &dst_dir) {
  // 打开文件
  FILE *src_file = fopen(src.c_str(), "r");
  if (nullptr == src_file) {
    perror("fopen src");
    return -1;
  }

  // 检查魔数
  if (CheckMagic(src_file) < 0) {
    printf("error CheckMagic\n");
    return -1;
  }

  // 读取源文件的元信息，创建目标文件，并设置其文件属性
  FILE *dst_file = SetDstFileAttribution(src_file, dst_dir);
  if (dst_file == nullptr) {
    printf("error SetDstFileAttribution\n");
    return -1;
  }

  // 读取词频表，以构造Huffman树
  std::vector<Freq> freq_vec;
  if (ReadFreq(src_file, freq_vec) < 0) {
    printf("error ReadFreq\n");
    return -1;
  }
  HuffmanTree tree(freq_vec);
  // TODO 针对空文件、单字符文件，做特殊处理

  // 通过Huffman树，进行解压缩
  if (DecompressData(src_file, dst_file, tree) < 0) {
    printf("error DecompressData\n");
    return -1;
  }

  // 关闭文件
  fclose(src_file);
  fclose(dst_file);
  return 0;
}

int Decompressor::CheckMagic(FILE *src) {
  static const std::string magic = COMPRESS_MAGIC;

  auto m_len = magic.size();
  std::string s(m_len, ' ');
  if (fread(const_cast<char *>(s.c_str()), 1, m_len, src) != m_len) {
    perror("fread magic");
    return -1;
  }

  if (s != magic) return -1;
  return 0;
}

int Decompressor::ReadFreq(FILE *src, std::vector<Freq> &freq_vec) {
  int n;
  if (fread(&n, sizeof(n), 1, src) != 1) {
    perror("fread");
    return -1;
  }

  Freq freq;
  int c;
  for (int i = 0; i < n; ++i) {
    if ((c = fgetc(src)) == EOF) {
      perror("fgetc");
      return -1;
    }
    freq.ch_ = c;
    if (fread(&freq.freq_, sizeof(freq.freq_), 1, src) != 1) {
      perror("fread");
      return -1;
    }
    freq_vec.push_back(freq);
  }
  return 0;
}

int
Decompressor::DecompressData(FILE *src, FILE *dst, const HuffmanTree &tree) {
  // 读取末尾字节
  int tail_len, tail_b;
  if ((tail_len = fgetc(src)) == EOF) {
    perror("fgetc");
    return -1;
  }
  if ((tail_b = fgetc(src)) == EOF) {
    perror("fgetc");
    return -1;
  }
  // 数据解码
  int c;
  HuffmanTree::HuffmanDecoder decoder(tree, dst);
  while ((c = fgetc(src)) != EOF) {
    if (decoder.Decode(c, 8) < 0) return -1;
  }
  if (ferror(src)) {
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

FILE *
Decompressor::SetDstFileAttribution(FILE *src, const std::string &dst_dir) {
  CStat mstat;
  if (fread(&mstat.st_mode_, sizeof(mstat.st_mode_), 1, src) != 1) {
    perror("fread");
    return nullptr;
  }
  if (fread(&mstat.st_uid_, sizeof(mstat.st_uid_), 1, src) != 1) {
    perror("fread");
    return nullptr;
  }
  if (fread(&mstat.st_gid_, sizeof(mstat.st_gid_), 1, src) != 1) {
    perror("fread");
    return nullptr;
  }
  if (fread(&mstat.st_atime_, sizeof(mstat.st_atime_), 1, src) != 1) {
    perror("fread");
    return nullptr;
  }
  if (fread(&mstat.st_mtime_, sizeof(mstat.st_mtime_), 1, src) != 1) {
    perror("fread");
    return nullptr;
  }
  if (fread(&mstat.st_ctime_, sizeof(mstat.st_ctime_), 1, src) != 1) {
    perror("fread");
    return nullptr;
  }
  if (fread(&mstat.name_len_, sizeof(mstat.name_len_), 1, src) != 1) {
    perror("fread");
    return nullptr;
  }
  int len = mstat.name_len_;
  std::string name(len, ' ');
  if (fread(const_cast<char *>(name.c_str()), 1, len, src) != len) {
    perror("fread");
    return nullptr;
  }

  // TODO 设置文件元信息

  std::string dst =
          dst_dir.back() == '/' ? dst_dir + name : dst_dir + "/" + name;
  auto dst_file = fopen(dst.c_str(), "w");
  if (dst_file == nullptr) {
    perror("fopen dst");
    return nullptr;
  }
  return dst_file;
}


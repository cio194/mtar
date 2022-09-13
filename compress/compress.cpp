#include "compress.h"

int Compressor::Compress(const std::string &src, const std::string &dst) {
  // 打开文件
  src_ = fopen(src.c_str(), "r");
  if (src_ == nullptr) {
    perror("fopen src");
    return -1;
  }
  dst_ = fopen(dst.c_str(), "w");
  if (dst_ == nullptr) {
    perror("fopen dst");
    return -1;
  }

  // 写入魔数
  if (fputs(COMPRESS_MAGIC, dst_) == EOF) {
    perror("fputs magic");
    return -1;
  }

  // 写入文件元信息
  if (WriteMeta(src) < 0) {
    printf("error WriteMeta\n");
    return -1;
  }

  // 词频统计，并写入词频表
  std::vector<Freq> freq_vec;
  if (GetFreq(freq_vec) < 0) {
    printf("error GetFreq\n");
    return -1;
  }
  if (WriteFreq(freq_vec) < 0) {
    printf("error WriteFreq\n");
    return -1;
  }
  // TODO 针对空文件、单字符文件，做特殊处理

  // 用词频表构造Huffman树，生成编码
  std::unordered_map<char, std::string> coding_map;
  HuffmanTree tree(freq_vec);
  tree.Encode(coding_map);

  // 依靠编码，写入压缩数据
  if (WriteData(coding_map) < 0) {
    printf("error WriteData\n");
    return -1;
  }

  // 关闭文件
  fclose(src_);
  fclose(dst_);
  return 0;
}

int Compressor::GetFreq(std::vector<Freq> &freq_vec) {
  int c;
  std::unordered_map<char, uint64_t> freq_map;
  while ((c = fgetc(src_)) != EOF) {
    if (freq_map.count(c)) {
      ++freq_map[c];
    } else {
      freq_map[c] = 1;
    }
  }
  if (ferror(src_)) {
    perror("error reading src");
    return -1;
  }
  for (const auto &p : freq_map) freq_vec.push_back({p.first, p.second});
  return 0;
}

static std::string GetBasename(const std::string &s) {
  std::string sc = s;
  return std::string(basename(const_cast<char *>(sc.c_str())));
}

int Compressor::WriteMeta(const std::string &src_str) {
  struct stat sb;
  if (lstat(src_str.c_str(), &sb) != 0) {
    perror("lstat src");
    return -1;
  }
  CStat cstat;
  cstat.Set(sb, GetBasename(src_str));
  if (cstat.Write(dst_) < 0) return -1;
  return 0;
}

int Compressor::WriteFreq(const std::vector<Freq> &freq_vec) {
  int n = freq_vec.size();
  if (fwrite(&n, sizeof(n), 1, dst_) != 1) {
    perror("fwrite dst");
    return -1;
  }
  for (int i = 0; i < n; ++i) {
    if (fputc(freq_vec[i].ch_, dst_) == EOF) {
      perror("fputc dst");
      return -1;
    }
    auto freq = freq_vec[i].freq_;
    if (fwrite(&freq, sizeof(freq), 1, dst_) != 1) {
      perror("fwrite dst");
      return -1;
    }
  }
  return 0;
}

int
Compressor::WriteData(const std::unordered_map<char, std::string> &coding_map) {
  if (fseek(src_, 0, SEEK_SET) != 0) {
    perror("fseek src_");
    return -1;
  }

  // 为末尾byte预留2字节空间
  auto pos = ftell(dst_);
  if (pos == -1L) {
    perror("ftell dst_");
    return -1;
  }
  if (fseek(dst_, 2, SEEK_CUR) != 0) {
    perror("fseek dst_");
    return -1;
  }

  int c;
  BBuf bbuf(dst_);
  while ((c = fgetc(src_)) != EOF) {
    const auto &coding = coding_map.at(c);
    for (int i = 0; i < coding.size(); ++i) {
      if (bbuf.Write(coding[i]) < 0) return -1;
    }
  }
  if (ferror(src_)) {
    perror("error reading src_");
    return -1;
  }

  // 末尾字节写入
  if (fseek(dst_, pos, SEEK_SET) != 0) {
    perror("fseek dst_");
    return -1;
  }
  if (fputc(bbuf.cur_, dst_) == EOF) {
    perror("fputc dst_");
    return -1;
  }
  if (fputc(bbuf.buf_, dst_) == EOF) {
    perror("fputc dst_");
    return -1;
  }

  return 0;
}

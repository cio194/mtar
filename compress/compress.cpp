#include "compress.h"
#include "huffman_tree.h"

int Compressor::Compress(const std::string &src, const std::string &dst) {
  // 打开文件
  FILE *src_file = fopen(src.c_str(), "r");
  if (nullptr == src_file) {
    perror("fopen src");
    return -1;
  }
  FILE *dst_file = fopen(dst.c_str(), "w");
  if (nullptr == dst_file) {
    perror("fopen dst");
    return -1;
  }

  // 词频统计
  std::vector<Freq> freq_vec;
  if (GetFreq(freq_vec, src_file) < 0) return -1;
  // TODO 针对空文件、单字符文件，做特殊处理

  // 构造Huffman树
  std::unordered_map<char, std::string> coding_map;
  HuffmanTree tree;
  tree.Encode(freq_vec, coding_map);

  // 写入目标文件
  if (Write(src_file, dst_file, freq_vec, coding_map, src) < 0) return -1;

  // 关闭文件
  fclose(src_file);
  fclose(dst_file);
  return 0;
}

int Compressor::GetFreq(std::vector<Freq> &freq_vec, FILE *src) {
  int c;
  std::unordered_map<char, uint64_t> freq_map;
  while ((c = fgetc(src)) != EOF) {
    if (freq_map.count(c)) {
      ++freq_map[c];
    } else {
      freq_map[c] = 1;
    }
  }
  if (ferror(src)) {
    perror("error reading src");
    return -1;
  }
  for (const auto &p : freq_map) freq_vec.push_back({p.first, p.second});
  return 0;
}

int Compressor::Write(FILE *src, FILE *dst, const std::vector<Freq> &freq_vec,
                      const std::unordered_map<char, std::string> &coding_map,
                      const std::string &src_str) {
  // 魔数 - 元信息 - 词频表 - 压缩数据
  if (fputs(COMPRESS_MAGIC, dst) == EOF) {
    perror("fputs dst");
    return -1;
  }
  if (WriteMeta(dst, src_str) < 0) return -1;
  if (WriteFreq(dst, freq_vec) < 0) return -1;
  if (WriteData(src, dst, coding_map) < 0) return -1;
  return 0;
}

static std::string GetBasename(const std::string &s) {
  std::string sc = s;
  return std::string(basename(const_cast<char *>(sc.c_str())));
}

int Compressor::WriteMeta(FILE *dst, const std::string &src_str) {
  MStat mstat;
  struct stat sb;
  if (lstat(src_str.c_str(), &sb) != 0) {
    perror("lstat src");
    return -1;
  }
  mstat.st_mode_ = sb.st_mode;
  mstat.st_uid_ = sb.st_uid;
  mstat.st_gid_ = sb.st_gid;
  mstat.st_atime_ = sb.st_atim.tv_sec;
  mstat.st_mtime_ = sb.st_mtim.tv_sec;
  mstat.st_ctime_ = sb.st_ctim.tv_sec;
  std::string mbasename = GetBasename(src_str);
  mstat.name_size_ = mbasename.size();
  // 结构体写入可移植性太差
  if (fwrite(&mstat.st_mode_, sizeof(mstat.st_mode_), 1, dst) != 1) {
    perror("fwrite dst");
    return -1;
  }
  if (fwrite(&mstat.st_uid_, sizeof(mstat.st_uid_), 1, dst) != 1) {
    perror("fwrite dst");
    return -1;
  }
  if (fwrite(&mstat.st_gid_, sizeof(mstat.st_gid_), 1, dst) != 1) {
    perror("fwrite dst");
    return -1;
  }
  if (fwrite(&mstat.st_atime_, sizeof(mstat.st_atime_), 1, dst) != 1) {
    perror("fwrite dst");
    return -1;
  }
  if (fwrite(&mstat.st_mtime_, sizeof(mstat.st_mtime_), 1, dst) != 1) {
    perror("fwrite dst");
    return -1;
  }
  if (fwrite(&mstat.st_ctime_, sizeof(mstat.st_ctime_), 1, dst) != 1) {
    perror("fwrite dst");
    return -1;
  }
  if (fwrite(&mstat.name_size_, sizeof(mstat.name_size_), 1, dst) != 1) {
    perror("fwrite dst");
    return -1;
  }
  if (fputs(mbasename.c_str(), dst) == EOF) {
    perror("fputs dst");
    return -1;
  }
  return 0;
}

int Compressor::WriteFreq(FILE *dst, const std::vector<Freq> &freq_vec) {
  int n = freq_vec.size();
  if (fputc(n, dst) == EOF) {
    perror("fputc dst");
    return -1;
  }
  for (int i = 0; i < n; ++i) {
    if (fputc(freq_vec[i].ch_, dst) == EOF) {
      perror("fputc dst");
      return -1;
    }
    auto freq = freq_vec[i].freq_;
    if (fwrite(&freq, sizeof(freq), 1, dst) != 1) {
      perror("fwrite dst");
      return -1;
    }
  }
  return 0;
}

int Compressor::WriteData(FILE *src, FILE *dst,
                          const std::unordered_map<char, std::string> &coding_map) {
  if (fseek(src, 0, SEEK_SET) != 0) {
    perror("fseek src");
    return -1;
  }

  // 为末尾byte预留2字节空间
  auto pos = ftell(dst);
  if (pos == -1L) {
    perror("ftell dst");
    return -1;
  }
  if (fseek(dst, 2, SEEK_CUR) != 0) {
    perror("fseek dst");
    return -1;
  }

  int c;
  BBuf bbuf(dst);
  while ((c = fgetc(src)) != EOF) {
    const auto &coding = coding_map.at(c);
    for (int i = 0; i < coding.size(); ++i) {
      if (bbuf.Write(coding[i]) < 0) return -1;
    }
  }
  if (ferror(src)) {
    perror("error reading src");
    return -1;
  }

  // 末尾字节写入
  if (fseek(dst, pos, SEEK_SET) != 0) {
    perror("fseek dst");
    return -1;
  }
  if (fputc(bbuf.cur_, dst) == EOF) {
    perror("fputc dst");
    return -1;
  }
  if (fputc(bbuf.buf_, dst) == EOF) {
    perror("fputc dst");
    return -1;
  }

  return 0;
}


#include "unpack.h"

int Unpacker::Unpack(const std::string &src, const std::string &dst_dir) {
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

  // 递归解包
  while (true) {
    cur_ = dst_dir;
    int ret = Unpack();
    if (ret < 0) {  // 错误发生
      return -1;
    } else if (ret == 0) {  // 解包完毕
      break;
    } else {  // 解包进行中，后面可能还有数据
      continue;
    }
  }

  // 关闭文件
  fclose(src_);
  return 0;
}

int Unpacker::Unpack() {
  // 读取PStat
  PStat pstat;
  if (pstat.Read(src_) < 0) {
    if (feof(src_)) {
      return 0;  // 到达文件结尾，解包完毕
    } else {
      printf("error read pstat\n");
      return -1;
    }
  }

  // 根据PStat类型分类处理
  if (S_ISDIR(pstat.st_mode_)) {
    if (UnpackDir(pstat) < 0) return -1;
  } else if (S_ISREG(pstat.st_mode_)) {
    if (UnpackReg(pstat) < 0) return -1;
  } else if (S_ISLNK(pstat.st_mode_)) {
    if (UnpackLnk(pstat) < 0) return -1;
  } else {
    printf("error file type\n");
    return -1;
  }

  return 1;  // 解包进行中，后面可能还有数据
}

int Unpacker::UnpackDir(const PStat &pstat) {
  // 创建文件夹
  cur_.append(pstat.name_);
  if (mkdir(cur_.c_str(), pstat.st_mode_) == -1) {
    PPerror(cur_, "mkdir");
    return -1;
  }

  // 递归
  cur_.push_back('/');
  for (int i = 0; i < pstat.num_; ++i) {
    if (Unpack() != 1) return -1;  // 必须为“解包进行中”
  }
  cur_.pop_back();

  // 回溯时，设置属主、时间
  lchown(cur_.c_str(), pstat.st_uid_, pstat.st_gid_);
  struct utimbuf utb;
  utb.actime = pstat.st_atime_;
  utb.modtime = pstat.st_mtime_;
  utime(cur_.c_str(), &utb);
  for (int i = 0; i < pstat.name_len_; ++i) cur_.pop_back();
  return 0;
}

int Unpacker::UnpackReg(const PStat &pstat) {
  // 创建文件
  cur_.append(pstat.name_);
  int fd = open(cur_.c_str(), O_WRONLY | O_CREAT | O_TRUNC, pstat.st_mode_);
  if (fd == -1) {
    PPerror(cur_, "open");
    return -1;
  }
  FILE *file = fdopen(fd, "w");
  if (file == nullptr) {
    PPerror(cur_, "fdopen");
    return -1;
  }

  // 写入文件数据
  int c;
  for (decltype(pstat.num_) i = 0; i < pstat.num_; ++i) {
    if ((c = fgetc(src_)) == EOF) {
      PPerror(cur_, "fgetc");
      return -1;
    }
    if (fputc(c, file) == EOF) {
      PPerror(cur_, "fputc");
      return -1;
    }
  }

  // 设置属主、时间
  lchown(cur_.c_str(), pstat.st_uid_, pstat.st_gid_);
  struct utimbuf utb;
  utb.actime = pstat.st_atime_;
  utb.modtime = pstat.st_mtime_;
  utime(cur_.c_str(), &utb);

  // 关闭文件
  fclose(file);
  for (int i = 0; i < pstat.name_len_; ++i) cur_.pop_back();
  return 0;
}

int Unpacker::UnpackLnk(const PStat &pstat) {
  cur_.append(pstat.name_);
  // 读取软链接目标
  int c;
  std::string t(pstat.num_, ' ');
  if (fread(const_cast<char *>(t.c_str()), sizeof(char), pstat.num_, src_)
      != pstat.num_) {
    PPerror(cur_, "fread");
    return -1;
  }

  // 创建软链接
  if (symlink(t.c_str(), cur_.c_str()) == -1) {
    PPerror(cur_, "symlink");
    return -1;
  }
  for (int i = 0; i < pstat.name_len_; ++i) cur_.pop_back();
  return 0;
}

int Unpacker::CheckMagic() {
  static const std::string magic = PACK_MAGIC;

  auto m_len = magic.size();
  std::string s(m_len, ' ');
  if (fread(const_cast<char *>(s.c_str()), 1, m_len, src_) != m_len) {
    perror("fread magic");
    return -1;
  }

  if (s != magic) return -1;
  return 0;
}

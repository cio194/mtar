#include "pack.h"

int
Packer::Pack(const std::vector<std::string> &src_vec, const std::string &dst) {
  // 打开文件
  dst_ = fopen(dst.c_str(), "w");
  if (dst_ == nullptr) {
    perror("fopen dst");
    return -1;
  }

  // 写入Pack魔数
  if (fputs(PACK_MAGIC, dst_) == EOF) {
    perror("fputs magic");
    return -1;
  }

  // 递归打包
  for (int i = 0; i < src_vec.size(); ++i) {
    std::string name = GetBasename(src_vec[i]);
    if (Pack(src_vec[i], name) < 0) return -1;
  }

  // 关闭文件
  fclose(dst_);
  return 0;
}

int Packer::Pack(const std::string &path, const std::string &name) {
  // 获取stat
  struct stat sb;
  if (lstat(path.c_str(), &sb) != 0) {
    printf("lstat failed: %s\n", path.c_str());
    return -1;
  }
  // 根据文件类型分类pack
  if (S_ISDIR(sb.st_mode)) {
    if (PackDir(path, name, sb) < 0) return -1;
  } else if (S_ISREG(sb.st_mode)) {
    if (PackReg(path, name, sb) < 0) return -1;
  } else if (S_ISLNK(sb.st_mode)) {
    if (PackLnk(path, name, sb) < 0) return -1;
  }

  return 0;
}

int Packer::PackDir(const std::string &path, const std::string &name,
                    const struct stat &sb) {
  // 打开目录
  int fd = open(path.c_str(), O_RDONLY | O_NOATIME);
  if (fd == -1) {
    PPerror(path, "open");
    return -1;
  }
  DIR *dirp = fdopendir(fd);
  if (dirp == nullptr) {
    PPerror(path, "fdopendir");
    return -1;
  }

  // 统计entry数量
  PStat pstat;
  pstat.num_ = 0;
  dirent *dp;
  while (true) {
    errno = 0;
    dp = readdir(dirp);
    if (dp == nullptr) break;
    if (strcmp(dp->d_name, ".") == 0 ||
        strcmp(dp->d_name, "..") == 0)
      continue;
    ++pstat.num_;
  }
  if (errno != 0) {
    PPerror(path, "readdir");
    return -1;
  }
  rewinddir(dirp);  // 目录流重置

  // 写入PStat
  pstat.Set(sb, name);
  if (pstat.Write(dst_) < 0) {
    printf("pstat write failed: %s\n", path.c_str());
    return -1;
  }

  // 对entry进行递归打包
  while (true) {
    errno = 0;
    dp = readdir(dirp);
    if (dp == nullptr) break;
    if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
      continue;
    std::string d_name = dp->d_name;
    if (Pack(path + "/" + d_name, d_name) < 0) return -1;
  }
  if (errno != 0) {
    PPerror(path, "readdir");
    return -1;
  }

  // 关闭目录
  closedir(dirp);
  return 0;
}

int Packer::PackReg(const std::string &path, const std::string &name,
                    const struct stat &sb) {
  // 写入PStat
  PStat pstat;
  pstat.Set(sb, name);
  pstat.num_ = sb.st_size;
  if (pstat.Write(dst_) < 0) {
    printf("pstat write failed: %s\n", path.c_str());
    return -1;
  }

  // 打开文件
  int fd = open(path.c_str(), O_RDONLY | O_NOATIME);
  if (fd == -1) {
    PPerror(path, "open");
    return -1;
  }
  FILE *src = fdopen(fd, "r");
  if (src == nullptr) {
    PPerror(path, "fdopen");
    return -1;
  }

  // 写入文件数据
  int c;
  while ((c = fgetc(src)) != EOF) {
    if (fputc(c, dst_) == EOF) {
      PPerror(path, "fputc");
      return -1;
    }
  }
  if (ferror(src)) {
    PPerror(path, "fgetc");
    return -1;
  }

  // 关闭文件
  fclose(src);
  return 0;
}

int Packer::PackLnk(const std::string &path, const std::string &name,
                    const struct stat &sb) {
  // 读取链接
  static char buf[8192];
  auto n = readlink(path.c_str(), buf, 8192);
  if (n == -1) {
    PPerror(path, "readlink");
    return -1;
  }

  // 写入PStat
  PStat pstat;
  pstat.Set(sb, name);
  pstat.num_ = n;
  if (pstat.Write(dst_) < 0) {
    printf("pstat write failed: %s\n", path.c_str());
    return -1;
  }

  // 写入链接内容
  if (fwrite(buf, 1, n, dst_) != n) {
    PPerror(path, "fwrite lnk");
    return -1;
  }
  return 0;
}

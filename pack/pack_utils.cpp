#include "pack_utils.h"

std::string GetBasename(const std::string &s) {
  std::string sc = s;
  return std::string(basename(const_cast<char *>(sc.c_str())));
}

void PPerror(const std::string &path, const std::string &msg) {
  perror(msg.c_str());
  printf("current: %s\n", path.c_str());
}

void PStat::Set(const struct stat &sb, const std::string &name) {
  st_mode_ = sb.st_mode;
  if (!S_ISLNK(st_mode_)) {
    st_uid_ = sb.st_uid;
    st_gid_ = sb.st_gid;
    st_atime_ = sb.st_atim.tv_sec;
    st_mtime_ = sb.st_mtim.tv_sec;
  }
  name_len_ = name.size();
  name_ = name;
}

int PStat::Write(FILE *dst) {
  if (fwrite(&st_mode_, sizeof(st_mode_), 1, dst) != 1) {
    perror("fwrite");
    return -1;
  }
  if (!S_ISLNK(st_mode_)) {  // 软链接某些属性不写入
    if (fwrite(&st_uid_, sizeof(st_uid_), 1, dst) != 1) {
      perror("fwrite");
      return -1;
    }
    if (fwrite(&st_gid_, sizeof(st_gid_), 1, dst) != 1) {
      perror("fwrite");
      return -1;
    }
    if (fwrite(&st_atime_, sizeof(st_atime_), 1, dst) != 1) {
      perror("fwrite");
      return -1;
    }
    if (fwrite(&st_mtime_, sizeof(st_mtime_), 1, dst) != 1) {
      perror("fwrite");
      return -1;
    }
  }
  if (fwrite(&num_, sizeof(num_), 1, dst) != 1) {
    perror("fwrite");
    return -1;
  }
  if (fwrite(&name_len_, sizeof(name_len_), 1, dst) != 1) {
    perror("fwrite");
    return -1;
  }
  if (fwrite(name_.c_str(), 1, name_len_, dst) != name_len_) {
    perror("fwrite");
    return -1;
  }
  return 0;
}

int PStat::Read(FILE *src) {
  if (fread(&st_mode_, sizeof(st_mode_), 1, src) != 1) {
    if (!feof(src)) perror("fread");
    return -1;
  }
  if (!S_ISLNK(st_mode_)) {
    if (fread(&st_uid_, sizeof(st_uid_), 1, src) != 1) {
      perror("fread");
      return -1;
    }
    if (fread(&st_gid_, sizeof(st_gid_), 1, src) != 1) {
      perror("fread");
      return -1;
    }
    if (fread(&st_atime_, sizeof(st_atime_), 1, src) != 1) {
      perror("fread");
      return -1;
    }
    if (fread(&st_mtime_, sizeof(st_mtime_), 1, src) != 1) {
      perror("fread");
      return -1;
    }
  }
  if (fread(&num_, sizeof(num_), 1, src) != 1) {
    perror("fread");
    return -1;
  }
  if (fread(&name_len_, sizeof(name_len_), 1, src) != 1) {
    perror("fread");
    return -1;
  }
  name_.resize(name_len_);
  if (fread(const_cast<char *>(name_.c_str()), sizeof(char), name_len_, src)
      != name_len_) {
    perror("fread");
    return -1;
  }
  return 0;
}

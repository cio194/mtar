#include "interact.h"

int Interactor::Interact(const std::vector<std::string> &argv) {
  if (ParseArg(argv) != 0) return -1;
  if (m_pack_) {
    if (CheckPack() != 0) return -1;
    // 打包
    Packer packer;
    if (packer.Pack(srcs_, pack_path_) != 0) {
      std::cout << "pack failed" << std::endl;
      return -1;
    }
    // 压缩
    if (m_compress_) {
      Compressor compressor;
      if (compressor.Compress(pack_path_, compress_path_) != 0) {
        std::cout << "compress failed" << std::endl;
        return -1;
      }
    }
    // 创建硬链接
    std::string tmp_src = m_compress_ ? compress_path_ : pack_path_;
    if (link(tmp_src.c_str(), tar_path_.c_str()) != 0) {
      perror("link tarfile");
      return -1;
    }
  } else {
    // 解包
    if (CheckUnpack() != 0) return -1;
    if (m_decompress_) {
      Decompressor decompressor;
      if (decompressor.Decompress(src_file_, dst_dir_, decompress_path_) != 0) {
        std::cout << "decompress failed" << std::endl;
        return -1;
      }
    }
    Unpacker unpacker;
    std::string unpack_path = m_decompress_ ? decompress_path_ : src_file_;
    if (unpacker.Unpack(unpack_path.c_str(), dst_dir_) != 0) {
      std::cout << "unpack failed" << std::endl;
      return -1;
    }
  }
  return 0;
}

int Interactor::ParseArg(const std::vector<std::string> &argv) {
  if (argv.size() < 2) {
    PrintUsage();
    return -1;
  }
  if (argv[1].size() >= 2 && argv[1][0] == '-' && argv[1][1] == 'p') {
    // 打包
    int cur = 1;
    m_pack_ = true;
    // 选项
    for (int i = 2; i < argv[cur].size(); ++i) {
      switch (argv[cur][i]) {
        case 't':
          m_target_ = true;
          break;
        case 'c':
          m_compress_ = true;
          break;
        default:
          std::cout << "invalid pack option: " << argv[cur][i] << std::endl;
          PrintUsage();
          return -1;
      }
    }
    ++cur;
    // 目标文件夹
    if (m_target_) {
      if (cur >= argv.size()) {
        PrintUsage();
        return -1;
      }
      tar_dir_ = argv[cur];
      ++cur;
    }
    // 需打包的源文件
    if (cur >= argv.size()) {
      PrintUsage();
      return -1;
    }
    for (int i = cur; i < argv.size(); ++i) srcs_.push_back(argv[i]);
  } else {  // 解包
    // 源文件
    src_file_ = argv[1];
    // 目的地文件夹
    if (argv.size() == 3) dst_dir_ = argv[2];
    if (argv.size() > 3) {
      PrintUsage();
      return -1;
    }
  }
  return 0;
}

void Interactor::PrintUsage() {
  std::cout << "pack: mtar -p[options] dir/file..." << std::endl;
  std::cout << "      -t dir -- set target dir" << std::endl;
  std::cout << "      -c     -- compress" << std::endl;
  std::cout << "       e.g. mtar -ptc ./target/ ./pack_file ./pack_dir"
            << std::endl;
  std::cout << "unpack: mtar src_file [dst_dir]" << std::endl;
}

int Interactor::CheckPack() {
  // 源文件/文件夹
  // 不能为根目录
  // 必须存在且有权限
  // 类型必须为 文件夹/普通文件/链接文件
  // 转为绝对路径
  // todo 无basename冲突
  for (int i = 0; i < srcs_.size(); ++i) {
    if (srcs_[i] == "/") {
      std::cout << "root dir cannot be packed" << std::endl;
      return -1;
    }
    if (access(srcs_[i].c_str(), F_OK | R_OK) != 0) {
      std::cout << srcs_[i] << " not exist or no permission" << std::endl;
      return -1;
    }
    struct stat sb;
    if (lstat(srcs_[i].c_str(), &sb) != 0) {
      std::string msg = "lstat error " + srcs_[i];
      perror(msg.c_str());
      return -1;
    }
    if (!S_ISDIR(sb.st_mode) && !S_ISREG(sb.st_mode) && !S_ISLNK(sb.st_mode)) {
      std::cout << srcs_[i] << " not dir, reg or lnk" << std::endl;
      return -1;
    }
    if (S_ISDIR(sb.st_mode) && access(srcs_[i].c_str(), X_OK) != 0) {
      std::cout << srcs_[i] << " not exist or no permission" << std::endl;
      return -1;
    }
    if (GetAbsolutePath(srcs_[i]) != 0) return -1;
  }

  // 目标文件夹
  // 必须存在，不存在则创建
  if (access(tar_dir_.c_str(), F_OK) != 0) {
    // 尝试递归创建文件夹
    if (Mkdirp(tar_dir_) != 0) return -1;
  }
  // 必须是文件夹
  struct stat sb;
  if (lstat(tar_dir_.c_str(), &sb) != 0) {
    std::string msg = "lstat error " + tar_dir_;
    perror(msg.c_str());
    return -1;
  }
  if (!S_ISDIR(sb.st_mode)) {
    std::cout << tar_dir_ << " not dir" << std::endl;
    return -1;
  }
  // 必须有权限
  if (access(tar_dir_.c_str(), R_OK | W_OK | X_OK) != 0) {
    std::cout << tar_dir_ << " no permission" << std::endl;
    return -1;
  }
  // 转换为绝对路径，不能被各源文件夹包含
  // 若为目录，绝对路径尾部需有'/'
  if (GetAbsolutePath(tar_dir_) != 0) return -1;
  if (JudgeTarIncluded(srcs_, tar_dir_) != 0) return -1;
  if (tar_dir_.back() != '/') tar_dir_.push_back('/');
  // 生成目标文件名称，需保证不存在，单文件取basename，多文件取tar_dir basename
  if (GenerateTarFilename(srcs_, tar_dir_, tar_file_) != 0) return -1;
  tar_path_ = tar_dir_ + tar_file_;
  // 统一为random file，完事后创建硬链接并删除
  pack_dir_ = tar_dir_;
  if (GenerateRandomFilename(tar_dir_, pack_file_) != 0) return -1;
  pack_path_ = pack_dir_ + pack_file_;
  if (m_compress_) {
    compress_dir_ = tar_dir_;
    if (GenerateRandomFilename(tar_dir_, compress_file_) != 0) return -1;
    compress_path_ = compress_dir_ + compress_file_;
  }
  return 0;
}

Interactor::~Interactor() {
  // 清理工作
  if (m_pack_) {
    unlink(pack_path_.c_str());
    if (m_compress_) unlink(compress_path_.c_str());
  } else {
    if (m_decompress_) unlink(decompress_path_.c_str());
  }
}

int Interactor::GetAbsolutePath(std::string &path) {
  auto rpath = realpath(path.c_str(), nullptr);
  if (rpath == nullptr) {
    perror("realpath");
    return -1;
  }
  path = rpath;
  free(rpath);
  return 0;
}

int Interactor::Mkdirp(const std::string &dir) {
  std::string rdir = dir;
  if (rdir.front() != '/') rdir.insert(0, "./");
  if (rdir.back() != '/') rdir.push_back('/');
  // path split
  std::vector<std::string> names;
  std::string name;
  for (int i = 0; i < rdir.size(); ++i) {
    if (rdir[i] == '/') {
      if (!name.empty()) {
        names.push_back(name);
        name.clear();
      }
      continue;
    }
    name.push_back(rdir[i]);
  }
  // 逐层创建目录
  std::string path;
  int idx = 0;
  if (names[0] == "." || names[0] == "..") {
    path += names[0];
    ++idx;
  }
  for (; idx < names.size(); ++idx) {
    path.push_back('/');
    path += names[idx];
    if (access(path.c_str(), F_OK) == 0) continue;
    if (mkdir(path.c_str(), S_IRUSR | S_IWUSR | S_IXUSR) != 0) {
      std::string msg = "mkdir " + path;
      perror(msg.c_str());
      return -1;
    }
  }
  return 0;
}

int Interactor::GenerateRandomFilename(const std::string &dir,
                                       std::string &filename) {
  // dir参数必须以'/'结尾
  static std::unordered_set<std::string> us;
  bool generated = false;
  std::string path;
  for (int i = 0; i < 1000; ++i) {  // 尝试1000次
    filename = "mtar-" + GetUUID();
    path = dir + filename;
    if (us.count(path)) continue;
    if (access(path.c_str(), F_OK) == 0) continue;
    generated = true;
    us.insert(path);
    break;
  }
  return generated ? 0 : -1;
}

std::string Interactor::GetUUID() {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<> dis(0, 15);
  static std::uniform_int_distribution<> dis2(8, 11);
  std::stringstream ss;
  int i;
  ss << std::hex;
  for (i = 0; i < 8; i++) {
    ss << dis(gen);
  }
  ss << "-";
  for (i = 0; i < 4; i++) {
    ss << dis(gen);
  }
  ss << "-4";
  for (i = 0; i < 3; i++) {
    ss << dis(gen);
  }
  ss << "-";
  ss << dis2(gen);
  for (i = 0; i < 3; i++) {
    ss << dis(gen);
  }
  ss << "-";
  for (i = 0; i < 12; i++) {
    ss << dis(gen);
  };
  return ss.str();
}

int Interactor::GenerateTarFilename(const std::vector<std::string> &srcs,
                                    const std::string &tar_dir,
                                    std::string &filename) {
  // srcs中均为绝对路径，且不为根目录
  // 单源文件取其basename，多源文件取tar_dir basename
  std::string path = tar_dir;
  if (srcs.size() == 1 || tar_dir == "/") path = srcs[0];
  filename = GetBasename(path);
  // 去除后缀
  int point_idx = -1;
  for (int i = filename.size() - 1; i > 0; --i) {
    if (filename[i] == '.') {
      point_idx = i;
      break;
    }
  }
  if (point_idx != -1)
    filename.erase(filename.begin() + point_idx, filename.end());
  // 生成目标文件名
  int suffix_idx = filename.size();
  filename.push_back('.');
  filename += PACK_SUFFIX;
  path = tar_dir + filename;
  bool generated = false;
  for (int i = 0; i < 1000; ++i) {  // 尝试1000次
    if (access(path.c_str(), F_OK) != 0) {  // 文件不存在
      generated = true;
      break;
    }
    // 文件已存在
    filename.erase(filename.begin() + suffix_idx, filename.end());
    filename += std::to_string(i);
    filename.push_back('.');
    filename += PACK_SUFFIX;
    path = tar_dir + filename;
  }
  return generated ? 0 : -1;
}

std::string Interactor::GetBasename(const std::string &s) {
  std::string sc = s;
  return std::string(basename(const_cast<char *>(sc.c_str())));
}

int Interactor::JudgeTarIncluded(const std::vector<std::string> &srcs,
                                 const std::string &tar_dir) {
  for (int i = 0; i < srcs.size(); ++i) {
    const auto &src = srcs[i];
    struct stat sb;
    if (lstat(src.c_str(), &sb) != 0) {
      std::string msg = "lstat error " + src;
      perror(msg.c_str());
      return -1;
    }
    if (!S_ISDIR(sb.st_mode)) continue;
    // tar_dir不能是src的子目录
    if (tar_dir.size() < src.size()) continue;
    bool included = true;
    for (int j = 0; j < src.size(); ++j) {
      if (tar_dir[j] != src[j]) {
        included = false;
        break;
      }
    }
    if (!included) continue;
    if (tar_dir.size() > src.size() && tar_dir[src.size()] != '/') continue;
    // 发现tar_dir是子目录
    std::cout << "tar_dir cannot be subdir of src" << std::endl;
    std::cout << "tar_dir: " << tar_dir << std::endl;
    std::cout << "src    : " << src << std::endl;
    return -1;
  }
  return 0;
}

int Interactor::CheckUnpack() {
  // 源文件
  // 必须存在且有权限
  // 必须为普通文件
  // 转为绝对路径
  if (access(src_file_.c_str(), F_OK | R_OK) != 0) {
    std::cout << src_file_ << " not exist or no permission" << std::endl;
    return -1;
  }
  struct stat sb;
  if (lstat(src_file_.c_str(), &sb) != 0) {
    std::string msg = "lstat error " + src_file_;
    perror(msg.c_str());
    return -1;
  }
  if (!S_ISREG(sb.st_mode)) {
    std::cout << src_file_ << " not reg" << std::endl;
    return -1;
  }
  if (GetAbsolutePath(src_file_) != 0) return -1;

  // 目标文件夹
  // 必须存在，不存在则创建
  if (access(dst_dir_.c_str(), F_OK) != 0) {
    // 尝试递归创建文件夹
    if (Mkdirp(dst_dir_) != 0) return -1;
  }
  // 必须是文件夹
  if (lstat(dst_dir_.c_str(), &sb) != 0) {
    std::string msg = "lstat error " + dst_dir_;
    perror(msg.c_str());
    return -1;
  }
  if (!S_ISDIR(sb.st_mode)) {
    std::cout << dst_dir_ << " not dir" << std::endl;
    return -1;
  }
  // 必须有权限
  if (access(dst_dir_.c_str(), R_OK | W_OK | X_OK) != 0) {
    std::cout << dst_dir_ << " no permission" << std::endl;
    return -1;
  }
  // 转换为绝对路径，尾部需有'/'
  if (GetAbsolutePath(dst_dir_) != 0) return -1;
  if (dst_dir_.back() != '/') dst_dir_.push_back('/');
  // 检测是否需要解压
  int ret = JudgeCompressed(src_file_);
  if (ret < 0) return -1;
  if (ret == 1) m_decompress_ = true;  // 需要解压
  return 0;
}

int Interactor::JudgeCompressed(std::string &file) {
  FILE *f = fopen(file.c_str(), "r");
  if (f == nullptr) {
    std::string msg = "fopen " + file;
    perror(msg.c_str());
    return -1;
  }

  static const std::string magic = COMPRESS_MAGIC;
  auto m_len = magic.size();
  std::string s(m_len, ' ');
  if (fread(const_cast<char *>(s.c_str()), 1, m_len, f) != m_len) {
    std::string msg = "fread magic " + file;
    perror(msg.c_str());
    return -1;
  }
  fclose(f);
  return s == magic ? 1 : 0;
}

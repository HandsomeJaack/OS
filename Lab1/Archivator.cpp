#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include <cstring>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

void arch(string dirpath, int& f) {
  DIR* dir = opendir(dirpath.c_str());
  if (!dir) {
    perror("diropen");
    exit(1);
  }

  struct dirent* entry = NULL;
  vector<char> buf(1024);

  while (true) {
    entry = readdir(dir);

    if (!entry) {
      break;
    }

    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }

    struct stat sb;
    string new_dir = (dirpath + "/" + entry->d_name);

    int i = stat(new_dir.c_str(), &sb);

    if (i == 0) {
      if (!S_ISREG(sb.st_mode) && !S_ISDIR(sb.st_mode)) {
        closedir(dir);
        throw runtime_error(string("reading failed: ") + strerror(errno));
      }

      if (S_ISDIR(sb.st_mode)) {
        char is_directory = '1';
        if (!write(f, &is_directory, sizeof is_directory)) {
          closedir(dir);
          throw runtime_error(string("writing of undefined object") +
                              strerror(errno));
        }

        short int len = new_dir.size();
        if (!write(f, &len, sizeof len)) {
          closedir(dir);
          throw runtime_error(string("writing directory name size error: ") +
                              strerror(errno));
        }

        if (!write(f, new_dir.c_str(), len)) {
          closedir(dir);
          throw runtime_error(string("writing directory name error: ") +
                              strerror(errno));
        }
        cout << new_dir << endl;
        arch(new_dir, f);
      }

      if (S_ISREG(sb.st_mode)) {
        char is_directory = '0';
        if (!write(f, &is_directory, sizeof is_directory)) {
          closedir(dir);
          throw runtime_error(string("writing of undefined object") +
                              strerror(errno));
        }

        short int len = new_dir.size();
        if (!write(f, &len, sizeof len)) {
          close(f);
          throw runtime_error(string("writing file size error: ") +
                              strerror(errno));
        }
        cout << new_dir << endl;

        if (!write(f, new_dir.c_str(), len)) {
          close(f);
          throw runtime_error(string("writing file name error: ") +
                              strerror(errno));
        }

        int local = open(new_dir.c_str(), O_RDONLY);
        if (!local) {
          close(f);
          throw runtime_error(string("opening new file error: ") +
                              strerror(errno));
        }

        int size = sb.st_size;
        if (!write(f, &size, sizeof size)) {
          close(f);
          close(local);
          throw runtime_error(string("writing file size error") +
                              strerror(errno));
        }

        while (size > 0) {
          int block_size = min(int(buf.size()), size);
          if (!read(local, buf.data(), block_size)) {
            close(local);
            close(f);
            throw runtime_error(string("reding file data error: ") +
                                strerror(errno));
          }
          write(f, buf.data(), block_size);
          size -= block_size;
        }
        close(local);
      }
    } else {
      closedir(dir);
      throw runtime_error(string("readfail: ") + strerror(errno));
    }
  }
  closedir(dir);
}

void rearch(string file) {
  int f = open(file.c_str(), O_RDONLY);

  string arch_dir;
  for (const auto& c : file) {
    if (c == '.') {
      break;
    }
    arch_dir += c;
  }
  cout << arch_dir << endl;

  if (f == -1) {
    throw runtime_error(string("not a file: ") + strerror(errno));
  }
  set<string> directories;

  while (true) {
    char is_directory;
    int t = read(f, &is_directory, sizeof is_directory);

    if (t < 0) {
      throw runtime_error(string("couldn't read from file: ") +
                          strerror(errno));
    }

    if (t == 0) {
      close(f);
      break;
    }

    if (t != sizeof is_directory) {
      close(f);
      throw runtime_error(
          string("size of object doesn't match with CHAR size: ") +
          strerror(errno));
    }

    if (is_directory == '1') {
      short int dir_size;
      if (read(f, &dir_size, sizeof dir_size) != sizeof dir_size) {
        close(f);
        throw runtime_error(
            string(
                "size of directory name doesn't match with SHORT_INT size: ") +
            strerror(errno));
      }

      string dir(dir_size, '.');
      if (read(f, dir.data(), dir_size) != dir_size) {
        close(f);
        throw runtime_error(
            string("size of directory doesn't match with DIR_SIZE size: ") +
            strerror(errno));
      }
      // cout << dir << endl;

      stringstream parse(dir);
      string buf_string;
      string full_path;
      while (getline(parse, buf_string, '/')) {
        if (buf_string == ".") {
          continue;
        }

        if (full_path.size() > 0) {
          full_path += "/";
        }
        full_path += buf_string;
        cout << full_path << endl;
        if (directories.insert(full_path).second) {
          if (mkdir(full_path.c_str(), S_IRWXG | S_IRWXU)) {
            close(f);
            throw runtime_error(string("failed to create directory: ") +
                                strerror(errno));
          }
        }
      }
      // string res_dir = arch_dir + dir;
    }

    else if (is_directory == '0') {
      short int file_name_size;
      if (read(f, &file_name_size, sizeof file_name_size) !=
          sizeof file_name_size) {
        close(f);
        throw runtime_error(
            string("size of file name doesn't match with SHORT_INT size: ") +
            strerror(errno));
      }

      string file_name(file_name_size, ' ');
      if (read(f, file_name.data(), file_name.size()) != file_name_size) {
        close(f);
        throw runtime_error(string("failed to read file name: ") +
                            strerror(errno));
      }

      int file_size;
      if (read(f, &file_size, sizeof file_size) != sizeof file_size) {
        close(f);
        throw runtime_error(
            string("size of file doesn't match with INT size: ") +
            strerror(errno));
      }

      string object(file_size, ' ');
      if (read(f, object.data(), object.size()) != ssize_t(object.size())) {
        close(f);
        throw runtime_error(string("failed to read contents of file: ") +
                            strerror(errno));
      }

      int o = open(file_name.c_str(), O_CREAT | O_RDWR, S_IRWXG | S_IRWXU);
      if (!o) {
        throw runtime_error(string("failed to create file: ") +
                            strerror(errno));
      }
      if (!write(o, object.c_str(), file_size)) {
        close(f);
        close(o);
        throw runtime_error(string("failed to write contents of file: ") +
                            strerror(errno));
      }
    } else {
      close(f);
      throw runtime_error(string("invalid object type: ") + strerror(errno));
    }
  }
  close(f);
}

int main(int argc, char* argv[]) {
  try {
    if (string(argv[1]) == "arch") {
      int f = open("../arch.swag", O_CREAT | O_WRONLY, S_IRWXG | S_IRWXU);
      arch("./", f);
      close(f);
    } else if (string(argv[1]) == "rearch") {
      rearch("arch.swag");
    } else {
      cout << "Invalid operation: " << argv[1]
           << ". 'arch' or 'rearch' were expected" << endl;
    }
  } catch (exception& e) {
    cout << e.what() << endl;
  }
  return 0;
}

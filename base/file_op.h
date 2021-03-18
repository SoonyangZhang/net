#include <string>
#include <string.h>
#include <dirent.h>
#include <vector>
#include <stdint.h>
#if defined(_WIN32)
#include <direct.h>   // _mkdir
#else
#include <sys/stat.h>
#endif
namespace basic{
    bool isDirExist(const std::string& path);
    bool makePath(const std::string& path);
    void getFiles(std::string &cate_dir,std::vector<std::string> &files);
    int get_file_size(std::string &path_and_name);
}
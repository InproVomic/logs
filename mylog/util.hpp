#ifndef __MY_UTIL__
#define __MY_UTIL__
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <ctime>

namespace mylog
{
    namespace util
    {
        class Date
        {
        public:
            static size_t getTime()
            {
                return (size_t)time(nullptr);
            }
        };
        class File
        {
        public:
            static bool exists(const std::string &pathname)
            {
                struct stat st;
                if (stat(pathname.c_str(), &st) < 0)
                {
                    return false;
                }
                return true;
            }
            static std::string path(const std::string &pathname)
            {
                size_t pos = pathname.find_last_of("/\\");
                if (pos == std::string::npos)
                    return pathname;
                return pathname.substr(0, pos + 1);
            }
            static void createDirectory(const std::string &pathname)
            {
                //   ./abc/ef/q
                size_t pos = 0, idx = 0;
                std::string path_dir;
                while (idx < pathname.size())
                {
                    pos = pathname.find_first_of("/\\", idx);
                    if (pos == std::string::npos)
                    {
                        mkdir(pathname.c_str(), 0777);
                        break;
                    }
                    std::string parent_path = pathname.substr(0, pos + 1);
                    if (exists(parent_path))
                    {
                        idx = pos + 1;
                        continue;
                    }
                    mkdir(parent_path.c_str(), 0777);
                    idx = pos + 1;
                }
            }
        };
    }
}

#endif

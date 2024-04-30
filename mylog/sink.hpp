#ifndef __LOG_SINK__
#define __LOG_SINK__
#include "util.hpp"
#include <cassert>
#include <memory>
#include <fstream>
#include <sstream>

namespace mylog
{
    class LogSink
    {
    public:
        using ptr = std::shared_ptr<LogSink>;
        LogSink(){};
        virtual ~LogSink(){};
        virtual void log(const char *data, const size_t &len) = 0;
    };

    // 落地方向：标准输出
    class StdoutSink : public LogSink
    {
    public:
        void log(const char *data, const size_t &len)
        {
            std::cout.write(data, len);
        }
    };

    // 落地方向：指定文件
    class FileSink : public LogSink
    {
    public:
        FileSink(const std::string &pathname) : _pathname(pathname)
        {
            // 创建文件所在目录
            util::File::createDirectory(util::File::path(pathname));
            // 创建并打开文件
            _ofs.open(pathname, std::ios::binary | std::ios::app);
            assert(_ofs.is_open());
        }

        void log(const char *data, const size_t &len)
        {
            _ofs.write(data, len);
            assert(_ofs.good());
        }

    private:
        std::ofstream _ofs;
        std::string _pathname;
    };

    // 落地方向：滚动文件（以大小进行滚动）
    class RollBySizeSink : public LogSink
    {
    public:
        RollBySizeSink(const std::string &basename, const size_t max_fsize)
            : _basename(basename), _max_fsize(max_fsize), _cur_fsize(0), _name_count(0)
        {
            std::string filename = createNewFile();
            util::File::createDirectory(util::File::path(filename)); // 创建文件所在的文件夹
            _ofs.open(filename, std::ios::binary | std::ios::app);   // 打开并创建文件
            assert(_ofs.is_open());
        }
        void log(const char *data, const size_t &len)
        {
            if (_cur_fsize >= _max_fsize)
            {
                _cur_fsize = 0;
                _ofs.close();
                std::string pathname = createNewFile();
                _ofs.open(pathname, std::ios::binary | std::ios::app);
                assert(_ofs.is_open());
            }
            _cur_fsize += len;
            _ofs.write(data, len);
            assert(_ofs.good());
        }

    private:
        std::string createNewFile() // 进行大小判读，超过指定大小就创建新文件
        {
            time_t t = util::Date::getTime();
            struct tm lt;
            localtime_r(&t, &lt);
            std::stringstream ss;
            ss << _basename;
            ss << lt.tm_year + 1900;
            ss << lt.tm_mon + 1;
            ss << lt.tm_mday;
            ss << lt.tm_hour;
            ss << lt.tm_min;
            ss << lt.tm_sec;
            ss << "-";
            ss << _name_count++;
            ss << ".log";
            return ss.str();
        }

    private:
        // 通过基础文件名 + 拓展文件名（以生成时间）组成当前输出文件名
        size_t _name_count;
        std::string _basename; // 例如   ./logs/base-20240330201530.log
        std::ofstream _ofs;
        size_t _max_fsize; // 记录最大大小，超过大小就切换文件
        size_t _cur_fsize; // 当前已经写入的文件的大小
    };

    class SinkFactory
    {
    public:
        template <typename SinkType, typename... Args>
        static LogSink::ptr create(Args &&...args)
        {
            return std::make_shared<SinkType>(std::forward<Args>(args)...);
        }
    };
}

#endif

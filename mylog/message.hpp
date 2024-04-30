#ifndef __MY_MESSAGE___
#define __MY_MESSAGE___
#include <iostream>
#include <thread>
#include <string>
#include <ctime>
#include "level.hpp"
#include "util.hpp"

namespace mylog
{
  struct logMsg
  {
    time_t _ctime;          // 时间戳
    LogLevel::value _level; // 日志等级
    size_t _line;           // 行号
    std::thread::id _tid;   // 线程id
    std::string _file;      // 源文件名
    std::string _logger;    // 日志器名
    std::string _payload;   // 有效消息数据
    logMsg(const LogLevel::value level, size_t line, const std::string file, const std::string logger, const std::string msg)
        : _ctime(util::Date::getTime()), _level(level), _line(line), _tid(std::this_thread::get_id()), _file(file), _logger(logger), _payload(msg)
    {
    }
  };
}

#endif

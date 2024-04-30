#ifndef __MY_LOG__
#define __MY_LOG__
#include "logger.hpp"

namespace mylog
{
    Logger::ptr getLogger(const std::string &name)
    {
        return LoggerManager::getInstance().getLogger(name);
    }

    Logger::ptr rootLogger()
    {
        return LoggerManager::getInstance().rootLogger();
    }

// 使用宏函数对接口进行代理
#define debug(fmt, ...) debug(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define info(fmt, ...) info(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define warn(fmt, ...) warn(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define error(fmt, ...) error(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define fatal(fmt, ...) fatal(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

// 提供宏函数通过默认日志器进行标准输出打印
#define DEBUG(fmt, ...) mylog::rootLogger()->debug(fmt, ##__VA_ARGS__)
#define INFO(fmt, ...) mylog::rootLogger()->info(fmt, ##__VA_ARGS__)
#define WARN(fmt, ...) mylog::rootLogger()->warn(fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...) mylog::rootLogger()->error(fmt, ##__VA_ARGS__)
#define FATAL(fmt, ...) mylog::rootLogger()->fatal(fmt, ##__VA_ARGS__)

}

#endif
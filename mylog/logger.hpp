#ifndef __MY_LOGGER
#define __MY_LOGGER
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "util.hpp"
#include "sink.hpp"
#include "level.hpp"
#include "format.hpp"
#include "looper.hpp"
#include <unordered_map>
#include <atomic>
#include <stdarg.h>
#include <mutex>

namespace mylog
{
    class Logger
    {
    public:
        using ptr = std::shared_ptr<Logger>;
        Logger(const LogLevel::value &level, const std::string &logger_name, Formatter::ptr &formatter, std::vector<LogSink::ptr> &sink)
            : _limit_level(level), _logger_name(logger_name), _formatter(formatter), _sink(sink.begin(), sink.end())
        {
        }
        const std::string &getLoggerName()
        {
            return _logger_name;
        }
        // 完成日志消息对象过程并进行格式化，得到格式化后的日志消息，随后进行落地输出
        void debug(const std::string &file, const size_t &line, const std::string &fmt, ...)
        {
            // 先判断当前日志是否达到输出等级n
            if (LogLevel::value::DEBUG < _limit_level)
                return;
            // 对fmt格式化字符串和不定参进行字符串组织，得到日志消息字符串
            va_list ap;
            va_start(ap, fmt);
            char *res;
            int ret = vasprintf(&res, fmt.c_str(), ap);
            if (ret == -1)
            {
                std::cout << "vasprintf failed!\n";
                return;
            }
            va_end(ap);
            serialize(LogLevel::value::DEBUG, file, line, res);
            free(res);
        }
        void info(const std::string &file, const size_t &line, const std::string &fmt, ...)
        {
            // 先判断当前日志是否达到输出等级
            if (LogLevel::value::INFO < _limit_level)
                return;
            // 对fmt格式化字符串和不定参进行字符串组织，得到日志消息字符串
            va_list ap;
            va_start(ap, fmt);
            char *res;
            int ret = vasprintf(&res, fmt.c_str(), ap);
            if (ret == -1)
            {
                std::cout << "vasprintf failed!\n";
                return;
            }
            va_end(ap);
            serialize(LogLevel::value::INFO, file, line, res);
            free(res);
        }
        void warn(const std::string &file, const size_t &line, const std::string &fmt, ...)
        {
            // 先判断当前日志是否达到输出等级
            if (LogLevel::value::WARN < _limit_level)
                return;
            // 对fmt格式化字符串和不定参进行字符串组织，得到日志消息字符串
            va_list ap;
            va_start(ap, fmt);
            char *res;
            int ret = vasprintf(&res, fmt.c_str(), ap);
            if (ret == -1)
            {
                std::cout << "vasprintf failed!\n";
                return;
            }
            va_end(ap);
            serialize(LogLevel::value::WARN, file, line, res);
            free(res);
        }
        void error(const std::string &file, const size_t &line, const std::string &fmt, ...)
        {
            // 先判断当前日志是否达到输出等级
            if (LogLevel::value::ERROR < _limit_level)
                return;
            // 对fmt格式化字符串和不定参进行字符串组织，得到日志消息字符串
            va_list ap;
            va_start(ap, fmt);
            char *res;
            int ret = vasprintf(&res, fmt.c_str(), ap);
            if (ret == -1)
            {
                std::cout << "vasprintf failed!\n";
                return;
            }
            va_end(ap);
            serialize(LogLevel::value::ERROR, file, line, res);
            free(res);
        }
        void fatal(const std::string &file, const size_t &line, const std::string &fmt, ...)
        {
            // 先判断当前日志是否达到输出等级
            if (LogLevel::value::FATAL < _limit_level)
                return;
            // 对fmt格式化字符串和不定参进行字符串组织，得到日志消息字符串
            va_list ap;
            va_start(ap, fmt);
            char *res;
            int ret = vasprintf(&res, fmt.c_str(), ap);
            if (ret == -1)
            {
                std::cout << "vasprintf failed!\n";
                return;
            }
            va_end(ap);
            serialize(LogLevel::value::FATAL, file, line, res);
            free(res);
        }

    protected:
        void serialize(const LogLevel::value &level, const std::string file, const size_t line, const char *str)
        {
            // 构造logMsg对象
            logMsg msg(level, line, file, _logger_name, str);
            // 进行格式化，得到格式化后的日志字符串
            std::string s = _formatter->format(msg);
            // 对日志进行落地
            log(s.c_str(), s.size());
        }
        // 抽象接口完成实际落地输出，不同的日志器有不同的落地方式
        virtual void log(const char *data, const int &len) = 0;

    protected:
        std::mutex _mutex;
        std::atomic<LogLevel::value> _limit_level;
        std::string _logger_name;
        Formatter::ptr _formatter;
        std::vector<LogSink::ptr> _sink;
    };

    class SyncLogger : public Logger
    {
    public:
        SyncLogger(const std::string &logger_name, const LogLevel::value &level, Formatter::ptr &formatter, std::vector<LogSink::ptr> &sinks)
            : Logger(level, logger_name, formatter, sinks) {}

    protected:
        void log(const char *data, const int &len)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            if (_sink.empty())
                return;
            for (auto &sink : _sink)
                sink->log(data, len);
        }
    };

    class AsyncLogger : public Logger
    {
    public:
        AsyncLogger(const std::string &logger_name, const LogLevel::value &level, Formatter::ptr &formatter, std::vector<LogSink::ptr> &sinks, AsyncType looper_type)
            : Logger(level, logger_name, formatter, sinks), _looper(std::make_shared<AsyncLooper>(std::bind(&AsyncLogger::realLog, this, std::placeholders::_1), looper_type)) {}
        void log(const char *data, const int &len) // 将数据写入缓冲区
        {
            _looper->push(data, len);
        }
        void realLog(Buffer &buf) // 将数据写入到文件中
        {
            if (_sink.empty())
                return;
            for (auto &sink : _sink)
                sink->log(buf.begin(), buf.readAbleSize());
        }

    private:
        AsyncLooper::ptr _looper;
    };

    enum class LoggerType
    {
        LOGGER_SYNC,
        LOGGER_ASYNC
    };

    class LoggerBuilder
    {
    public:
        LoggerBuilder() : _logger_type(LoggerType::LOGGER_SYNC),
                          _limit_level(LogLevel::value::DEBUG),
                          _looper_type(AsyncType::ASYNC_SAFE)
        {
        }
        void buildLoggerType(LoggerType logger_type)
        {
            _logger_type = logger_type;
        }
        void buildEnableUnsafeAsync()
        {
            _looper_type = AsyncType::ASYNC_UNSAFE;
        }
        void buildLoggername(const std::string &logger_name)
        {
            _logger_name = logger_name;
        }
        void buildLoggerLevel(LogLevel::value limit_level)
        {
            _limit_level = limit_level;
        }
        void buildFormatter(const std::string &pattern)
        {
            _formatter = std::make_shared<Formatter>(pattern);
        }
        template <typename SinkType, typename... Args>
        void buildSink(Args &&...args)
        {
            LogSink::ptr psink = SinkFactory::create<SinkType>(std::forward<Args>(args)...);
            _sinks.push_back(psink);
        }
        virtual Logger::ptr build() = 0;

    protected:
        AsyncType _looper_type;
        LoggerType _logger_type;
        std::string _logger_name;
        std::atomic<LogLevel::value> _limit_level;
        Formatter::ptr _formatter;
        std::vector<LogSink::ptr> _sinks;
    };

    class LocalLoggerBuilder : public LoggerBuilder
    {
    public:
        Logger::ptr build() override
        {
            assert(!_logger_name.empty()); // 必须要有日志器名
            if (_formatter.get() == nullptr)
            {
                _formatter = std::make_shared<Formatter>();
            }
            if (_sinks.empty())
            {
                buildSink<StdoutSink>();
            }
            if (_logger_type == LoggerType::LOGGER_ASYNC)
            {
                return std::make_shared<AsyncLogger>(_logger_name, _limit_level, _formatter, _sinks, _looper_type);
            }
            return std::make_shared<SyncLogger>(_logger_name, _limit_level, _formatter, _sinks);
        }
    };

    class LoggerManager
    {
    public:
        static LoggerManager &getInstance()
        {
            static LoggerManager eton;
            return eton;
        }
        void addLogger(Logger::ptr &logger)
        {
            if (hasLogger(logger->getLoggerName()))
                return;
            std::unique_lock<std::mutex> lock(_mutex);
            _logger.insert(std::make_pair(logger->getLoggerName(), logger));
        }
        bool hasLogger(const std::string &name)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            auto it = _logger.find(name);
            if (it == _logger.end())
                return false;
            return true;
        }
        Logger::ptr getLogger(const std::string &name)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            auto it = _logger.find(name);
            if (it == _logger.end())
                return Logger::ptr();
            return it->second;
        }
        Logger::ptr rootLogger()
        {
            return _root_logger;
        }

    private:
        LoggerManager()
        {
            std::unique_ptr<LoggerBuilder> builder(new LocalLoggerBuilder());
            builder->buildLoggername("root");
            _root_logger = builder->build();
            _logger.insert(std::make_pair("root", _root_logger));
        }

    private:
        std::mutex _mutex;
        Logger::ptr _root_logger; // 默认日志器
        std::unordered_map<std::string, Logger::ptr> _logger;
    };

    class GlobalLoggerBuilder : public LoggerBuilder
    {
    public:
        Logger::ptr build() override
        {
            assert(!_logger_name.empty()); // 必须要有日志器名
            if (_formatter.get() == nullptr)
            {
                _formatter = std::make_shared<Formatter>();
            }
            if (_sinks.empty())
            {
                buildSink<StdoutSink>();
            }
            Logger::ptr logger;
            if (_logger_type == LoggerType::LOGGER_ASYNC)
            {
                logger = std::make_shared<AsyncLogger>(_logger_name, _limit_level, _formatter, _sinks, _looper_type);
            }
            else
            {
                logger = std::make_shared<SyncLogger>(_logger_name, _limit_level, _formatter, _sinks);
            }
            LoggerManager::getInstance().addLogger(logger);
            return logger;
        }
    };
}

#endif

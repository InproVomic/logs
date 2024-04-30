#include "../mylog/mylog.h"
#include "../mylog/util.hpp"
#include "../mylog/sink.hpp"
enum class TimeGap
{
    GAP_SECOND = 0,
    GAP_MINUTE,
    GAP_HOUR,
    GAP_DAY
};

class RollByTimeSink : public mylog::LogSink
{
public:
    RollByTimeSink(const std::string &basename, TimeGap gap_size) : _basename(basename)
    {
        switch (gap_size)
        {
        case TimeGap::GAP_SECOND:
            _gap_size = 1;
            break;
        case TimeGap::GAP_MINUTE:
            _gap_size = 60;
            break;
        case TimeGap::GAP_HOUR:
            _gap_size = 3600;
            break;
        case TimeGap::GAP_DAY:
            _gap_size = 3600 * 24;
            break;
        }
        _cur_gap = mylog::util::Date::getTime() / _gap_size;
        std::string filename = createNewFile();
        mylog::util::File::createDirectory(mylog::util::File::path(filename));
        _ofs.open(filename, std::ios::binary | std::ios::app);
        assert(_ofs.is_open());
    }
    void log(const char *data, const size_t &len)
    {
        time_t cur = mylog::util::Date::getTime() / _gap_size;
        if (cur != _cur_gap)
        {
            _ofs.close();
            std::string filename = createNewFile();
            _ofs.open(filename, std::ios::binary | std::ios::app);
            assert(_ofs.is_open());
            _cur_gap = cur;
        }
        _ofs.write(data, len);
        assert(_ofs.good());
    }

private:
    std::string createNewFile()
    {
        time_t t = mylog::util::Date::getTime();
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
        ss << ".log";
        return ss.str();
    }

private:
    std::string _basename;
    std::ofstream _ofs;
    time_t _cur_gap;
    size_t _gap_size;
};

int main()
{
    std::unique_ptr<mylog::LoggerBuilder> builder(new mylog::GlobalLoggerBuilder());
    builder->buildFormatter("[%c][%f:%l]%m%n");
    builder->buildLoggerLevel(mylog::LogLevel::value::WARN);
    builder->buildLoggername("Async_logger");
    builder->buildLoggerType(mylog::LoggerType::LOGGER_ASYNC);
    builder->buildSink<mylog::FileSink>("./logfile/async.log");
    builder->buildSink<mylog::StdoutSink>();
    builder->buildSink<RollByTimeSink>("./logfile/roll-async-by-time", TimeGap::GAP_SECOND);

    mylog::Logger::ptr logger = builder->build();
    size_t cur = mylog::util::Date::getTime();
    while(cur + 5 > mylog::util::Date::getTime())
    {
        logger->fatal("這是一條測試用例");
        usleep(1000);
    }
    return 0;
}
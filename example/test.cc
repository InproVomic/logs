#include "../mylog/mylog.h"

void test_log(const std::string &name)
{
    INFO("%s", "测试开始");
    mylog::Logger::ptr logger = mylog::LoggerManager::getInstance().getLogger(name);
    logger->debug("%s", "测试日志");
    logger->info("%s", "测试日志");
    logger->warn("%s", "测试日志");
    logger->error("%s", "测试日志");
    logger->fatal("%s", "测试日志");
    INFO("%s", "测试完毕");
}

int main()
{
    std::unique_ptr<mylog::LoggerBuilder> builder(new mylog::GlobalLoggerBuilder());
    builder->buildFormatter("[%c][%f:%l][%p]%m%n");
    builder->buildLoggerLevel(mylog::LogLevel::value::DEBUG);
    builder->buildLoggername("sync_logger");
    builder->buildLoggerType(mylog::LoggerType::LOGGER_SYNC);
    builder->buildSink<mylog::FileSink>("./logfile/sync.log");
    builder->buildSink<mylog::StdoutSink>();
    builder->buildSink<mylog::RollBySizeSink>("./logfile/roll-sync-by-size", 1024 * 1024);
    builder->build();

    test_log("sync_logger");
    return 0;
}

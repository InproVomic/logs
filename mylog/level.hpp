#ifndef __LEVEL__
#define __LEVEL__
namespace mylog
{
    class LogLevel
    {
    public:
        enum class value
        {
            UNKOWN = 0,
            DEBUG,
            INFO,
            WARN,
            ERROR,
            FATAL,
            OFF
        };
        static const char *toString(const LogLevel::value &level)
        {
            switch (level)
            {
            case LogLevel::value::OFF:
                return "OFF";
            case LogLevel::value::DEBUG:
                return "DEBUG";
            case LogLevel::value::INFO:
                return "INFO";
            case LogLevel::value::WARN:
                return "WARN";
            case LogLevel::value::ERROR:
                return "ERROR";
            case LogLevel::value::FATAL:
                return "FATAL";
            }
            return "UNKOWN";
        }
    };
}
#endif

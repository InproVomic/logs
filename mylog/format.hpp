#ifndef __MY_FORMAT__
#define __MY_FORMAT__
#include "message.hpp"
#include <sstream>
#include <cassert>
#include <vector>

namespace mylog
{
    class FormatItem
    {
    public:
        using ptr = std::shared_ptr<FormatItem>;
        virtual void format(std::ostream &out, const logMsg &Msg) = 0;
    };

    class MsgFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const logMsg &Msg)
        {
            out << Msg._payload;
        }
    };

    class LevelFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const logMsg &Msg)
        {
            out << LogLevel::toString(Msg._level);
        }
    };

    class LineFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const logMsg &Msg)
        {
            out << Msg._line;
        }
    };

    class ThreadFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const logMsg &Msg)
        {
            out << Msg._tid;
        }
    };

    class LoggerFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const logMsg &Msg)
        {
            out << Msg._logger;
        }
    };

    class FileFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const logMsg &Msg)
        {
            out << Msg._file;
        }
    };

    class TimeFormatItem : public FormatItem
    {
    public:
        TimeFormatItem(const std::string &fmt = "%H:%M:%S") : _time_fmt(fmt)
        {
        }
        void format(std::ostream &out, const logMsg &Msg)
        {
            struct tm t;
            localtime_r(&Msg._ctime, &t);
            char tmp[32] = {0};
            strftime(tmp, 31, _time_fmt.c_str(), &t);
            out << tmp;
        }

    private:
        std::string _time_fmt;
    };

    class TabFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const logMsg &Msg)
        {
            out << "\t";
        }
    };

    class NLineFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const logMsg &Msg)
        {
            out << "\n";
        }
    };

    class OtherFormatItem : public FormatItem
    {
    public:
        OtherFormatItem(const std::string str) : _str(str)
        {
        }
        void format(std::ostream &out, const logMsg &Msg)
        {
            out << _str;
        }

    private:
        std::string _str;
    };

    /*
        %d 表示日期 ，包含子格式{%H:%M:%S}
        %t 表示线程ID
        %c 表示日志器名称
        %f 表示源文件名
        %l 表示源码行号
        %p 表示日志级别
        %T 表示制表符缩进
        %m 表示主题消息
        %n 表示换行
    */
    class Formatter
    {
    public:
        using ptr = std::shared_ptr<Formatter>;
        Formatter(const std::string &pattern = "[%d{%H:%M:%S}][%t][%c][%f:%l][%p]%T%m%n") : _pattern(pattern)
        {
            assert(parsePattern());
        }
        // 对msg进行格式化
        void format(std::ostream &out, const logMsg &msg)
        {
            for (auto &item : _items)
                item->format(out, msg);
        }
        std::string format(const logMsg &msg)
        {
            std::stringstream ss;
            format(ss, msg);
            return ss.str();
        }
        // 对格式化规则字符串进行解析
    private:
        bool parsePattern()
        {
            std::vector<std::pair<std::string, std::string>> fmt_order;
            size_t pos = 0;
            std::string key, val;
            while (pos < _pattern.size())
            {
                if (_pattern[pos] != '%') // 处理OtherFormatItem类型
                {
                    val.push_back(_pattern[pos++]);
                    continue;
                }

                if (val.size()) // 把OtherFormatItem先push进去
                {
                    fmt_order.push_back({"", val});
                    val.clear();
                    continue;
                }

                if (_pattern.size() == pos + 1)
                {
                    std::cout << "'%'后面没有其他字符，解析出错" << std::endl;
                    return false;
                }

                if (_pattern[pos + 1] == '%') // 这是"%%"的情况,那就只输出一个"%"
                {
                    val.push_back('%');
                    pos += 2;
                    continue;
                }

                // 到这里说明存在"%"且，"%"后面不为"%"
                key = _pattern[pos + 1];
                pos += 2;
                if (pos < _pattern.size() && _pattern[pos] == '{') // 说明遇到了子串情况
                {
                    ++pos;
                    while (pos < _pattern.size() && _pattern[pos] != '}')
                    {
                        val.push_back(_pattern[pos++]);
                    }
                    if (pos == _pattern.size())
                    {
                        std::cout << "子串匹配错误,未匹配到'}'" << std::endl;
                        return false;
                    }
                    ++pos; // 这里就说明匹配到了'}'
                }
                fmt_order.push_back({key, val});
                key.clear();
                val.clear();
            }
            for (auto &it : fmt_order)
                _items.push_back(createItem(it.first, it.second));
            //_items.push_back(createItem("n",""));
            return true;
        }
        // 根据不同的格式化字符创建不同的格式化子项对象
        FormatItem::ptr createItem(const std::string &key, const std::string &val)
        {
            if (key == "d")
                return std::make_shared<TimeFormatItem>(val);
            if (key == "t")
                return std::make_shared<ThreadFormatItem>();
            if (key == "c")
                return std::make_shared<LoggerFormatItem>();
            if (key == "f")
                return std::make_shared<FileFormatItem>();
            if (key == "l")
                return std::make_shared<LineFormatItem>();
            if (key == "p")
                return std::make_shared<LevelFormatItem>();
            if (key == "T")
                return std::make_shared<TabFormatItem>();
            if (key == "m")
                return std::make_shared<MsgFormatItem>();
            if (key == "n")
                return std::make_shared<NLineFormatItem>();
            if (key == "")
                return std::make_shared<OtherFormatItem>(val);
            std::cout << "输入格式错误！" << std::endl;
            abort();
            return std::make_shared<OtherFormatItem>(val);
        }

    private:
        std::string _pattern; // 格式化规则字符串
        std::vector<FormatItem::ptr> _items;
    };
}
#endif
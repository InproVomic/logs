#ifndef __MY_LOOPER_H__
#define __MY_LOOPER_H__
#include "buffer.hpp"
#include <functional>
#include <condition_variable>
#include <memory>
#include <atomic>
#include "thread"
#include <mutex>

namespace mylog
{
    enum class AsyncType
    {
        ASYNC_SAFE,
        ASYNC_UNSAFE
    };
    using Functor = std::function<void(Buffer &)>;
    class AsyncLooper
    {
    public:
        using ptr = std::shared_ptr<AsyncLooper>;
        AsyncLooper(const Functor &cb, AsyncType looper_type = AsyncType::ASYNC_SAFE) : _stop(false), _thread(std::thread(&AsyncLooper::threadEntry, this)),
                                                                                        _callback(cb), _looper_type(looper_type) {}
        ~AsyncLooper()
        {
            stop();
        }
        void push(const char *data, const size_t len)
        {
            // 加锁保证数据安全
            std::unique_lock<std::mutex> lock(_mutex);
            // 添加条件变量确保满足写入需求(只有在安全状态下才需要进行生产者的条件变量判断)
            if (_looper_type == AsyncType::ASYNC_SAFE)
                _cond_pro.wait(lock, [&]()
                               { return _pro_buf.writeAbleSize() >= len; });
            // 满足需求后将数据写入缓冲区
            _pro_buf.push(data, len);
            // 随机唤醒一个消费者进行数据处理
            _cond_pro.notify_one();
        }
        void stop()
        {
            _stop = true;
            _cond_con.notify_all(); // 唤醒所有线程，防止阻塞
            // 这里可能要加唤醒生产者线程的操作
            _thread.join();
        }

    private:
        void threadEntry() // 线程函数入口
        {
            while (1)
            {
                {
                    std::unique_lock<std::mutex> lock(_mutex);
                    // 根据条件变量判断是否满足消费条件
                    if (_stop && _pro_buf.empty())
                        break;
                    _cond_con.wait(lock, [&]()
                                   { return _stop || !_pro_buf.empty(); });
                    // 交换缓冲区
                    _con_buf.swap(_pro_buf);
                    // 唤醒生产者线程(只有在安全状态下才需要进行条件变量的判断和唤醒)
                    if (_looper_type == AsyncType::ASYNC_SAFE)
                        _cond_pro.notify_all();
                }
                // 对数据进行处理
                _callback(_con_buf);
                // 清空缓冲区
                _con_buf.reset();
            }
        }

    private:
        Functor _callback; // 回调函数
    private:
        AsyncType _looper_type;
        std::atomic<bool> _stop; // 工作器停止标志
        Buffer _pro_buf;         // 生产缓冲区
        Buffer _con_buf;         // 消费缓冲区
        std::mutex _mutex;
        std::condition_variable _cond_pro;
        std::condition_variable _cond_con;
        std::thread _thread; // 异步工作器对应的线程
    };
}

#endif
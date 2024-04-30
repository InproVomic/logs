#ifndef __MY_BUFFER__
#define __MY_BUFFER__
#include "util.hpp"
#include <vector>
#include <cassert>

namespace mylog
{
#define DEFAULT_BUFFER_SIZE (10 * 1024 * 1024)
#define THRESHOLD_BUFFER_SIZE (80 * 1024 * 1024)
#define INCREMENT_BUFFER_SIZE (10 * 1024 * 1024)
    class Buffer
    {
    public:
        Buffer() : _buffer(DEFAULT_BUFFER_SIZE), _writer_idx(0), _reader_idx(0)
        {
        }
        void push(const char *data, const size_t len)
        {
            // 先确保满足空间大小
            ensureEnoughSize(len);
            std::copy(data, data + len, &_buffer[_writer_idx]);
            moveWriter(len);
        }
        // 返回可读数据的开头
        const char *begin()
        {
            return &_buffer[_reader_idx];
        }
        size_t readAbleSize()
        {
            return (_writer_idx - _reader_idx);
        }
        size_t writeAbleSize()
        {
            return (_buffer.size() - _writer_idx);
        }
        void moveReader(const size_t len)
        {
            assert(len <= readAbleSize());
            _reader_idx += len;
        }
        void reset()
        {
            _reader_idx = 0;
            _writer_idx = 0;
        }
        void swap(Buffer &buffer)
        {
            _buffer.swap(buffer._buffer);
            std::swap(_writer_idx, buffer._writer_idx);
            std::swap(_reader_idx, buffer._reader_idx);
        }
        bool empty()
        {
            return _writer_idx == _reader_idx;
        }

    private:
        void moveWriter(const size_t len)
        {
            assert(len + _writer_idx <= _buffer.size());
            _writer_idx += len;
        }
        void ensureEnoughSize(size_t len)
        {
            if (len < writeAbleSize())
                return;
            size_t new_size = 0;
            if (_buffer.size() < THRESHOLD_BUFFER_SIZE)
            {
                new_size = _buffer.size() * 2 + len;
            }
            else
            {
                new_size = _buffer.size() + INCREMENT_BUFFER_SIZE + len;
            }
            _buffer.resize(new_size);
        }

    private:
        std::vector<char> _buffer;
        size_t _reader_idx;
        size_t _writer_idx;
    };
}

#endif
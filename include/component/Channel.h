//
// Created by yarten on 19-2-9.
//
#pragma once

#include <kernel/Segment.h>
#include <kernel/Manager.h>

namespace lsm
{
    /**
     * @class Channel
     * @brief 维护一个数据信道，其中包括数据段、锁与条件变量。
     * 可以通过该类进行字节流读写操作，通过一个长度进行创建。
     */
    class Channel
    {
    public:
        explicit Channel(const std::string & name);

        ~Channel();

        void SetTimeout(const Time & time);

    public:
        /**
         * 创建通信内存区域
         * @param length 用户数据长度
         * @return 是否创建成功
         */
        bool Create(size_t length);

        /**
         * 连接到通信内存区域（设置超时）
         * @return 是否连接成功
         */
        bool Connect();

        /**
         * 删除由 @see Create 创建的通信内存区域
         * @return 是否删除成功
         */
        void Remove();

    public:
        /**
         * 往数据段写数据，会进行加锁，操作时间短
         * @param message 数据（包括data*与length）
         * @throw lsm::Exception，若操作超时，则抛出异常
         * @return 是否写入成功，若长度太长，则不成功
         */
        bool Write(const std::string & message);

        /**
         * 等待数据更新，会进行加锁
         * @param time 轮询超时
         * @throw lsm::Exception，若操作超时，则抛出异常
         * @return 是否等待到数据更新
         */
        bool Wait(const Time & time);

        /**
         * 从数据段读数据，会进行加锁
         * @param message 数据（包括data*与length）
         * @throw lsm::Exception，若操作超时，则会抛出异常
         * @return 是否成功读取数据
         */
        bool Read(std::string & message);

    private:
        std::string   _name;
        bool          _created;
        Ptr<Manager>  _manager;
        Segment<boost::interprocess::interprocess_mutex> _mutex;
        Time          _timeout;
        bool          _first_read;
        char          _last_ack;
    };
}
//
// Created by yarten on 19-2-10.
//
#pragma once

#include <component/Connection.h>

namespace lsm
{
    class LSM
    {
    public:
        explicit LSM(const std::string & server = DefaultServerName);

        ~LSM();

        /// 新数据收到时的回调，第二个参数为新数据的信道，第三个参数则为数据本身（包含长度）
        using OnReceive = std::function<void(LSM &, const std::string &, std::string)>;

        /// 通信错误回调，通常意味着断开，第二个参数为出错信道（若为空，则说明是LSM本身连接断开）
        using OnError = std::function<void(LSM &, const std::string)>;

        void SetOnReceive(OnReceive && handler);

        void SetOnError(OnError && handler);

    public:
        /**
         * 建立内存管理连接到server
         * @param time 超时时间
         * @return 成功与否
         */
        bool Connect(Time time = Time::Second(1));

        /**
         * 断开内存管理连接
         */
        void Disconnect();

        /**
         * 打开信道。只有在建立了连接后才能执行。
         * @param name 信道名字
         * @param time 超时时间
         * @return 成功与否
         */
        bool Open(const std::string & name, Time time = Time::Second(1));

        /**
         * 关闭信道。
         * @param name 信道名字
         */
        void Close(const std::string & name);

        /**
         * 往指定信道发送消息。只有信道打开后才可以执行
         * @param name 信道名字
         * @param msg 消息，包括长度。
         * @return 发送成功与否
         */
        bool Send(const std::string & name, const std::string & msg);

    private:
        void _HandleACKMessage(ACKMessage msg, Message question);

        void _HandleUpdate(const std::string & channel, Connection & connection, std::string msg);

        void _HandleConnectionBroken(Connection & connection);

        void _HandleChannelBroken(const std::string & channel, Connection & connection);

        void InnerHandleConnectionBroken();

        void InnerHandleChannelBroken(const std::string & channel);

        void LockedDetach(std::function<void()> func);

    private:
        size_t          _id;
        std::string     _name;
        OnReceive       _on_receive;
        OnError         _on_error;

        Ptr<Connection> _connection;
        std::map<std::string, Ptr<Connection>> _channels;
        std::mutex      _mutex_this;

        std::atomic<Message> _msg_ack;
    };
}
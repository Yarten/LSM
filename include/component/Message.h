//
// Created by yarten on 19-2-10.
//
#pragma once

#include <string>
#include <functional>

namespace lsm
{
    enum Message
    {
        None,           // 代表空消息
        LinkIn,         // client要求连接到某个信道
        LinkOut,        // client告知server自己离开某个信道
        ACK,            // 确认回复
        Ping,           // keepalive机制报文
        Goodbye,        // client告知server自己即将关闭
    };

    class MessageBase
    {
    public:
        Message type;

        explicit MessageBase(Message type);

        virtual ~MessageBase() = default;

        virtual void Decode(const std::string & msg) = 0;

        virtual std::string Encode() const = 0;
    };

    class LinkInMessage : public MessageBase
    {
    public:
        std::string name;       // 信道名字
        size_t      reserve;    // 预留空间（字节）

    public:
        LinkInMessage();

        void Decode(const std::string &msg) override;

        std::string Encode() const override;
    };

    class LinkOutMessage : public MessageBase
    {
    public:
        std::string name;

    public:
        LinkOutMessage();

        void Decode(const std::string &msg) override;

        std::string Encode() const override;
    };

    class PingMessage : public MessageBase
    {
    public:
        PingMessage();

        void Decode(const std::string &msg) override;

        std::string Encode() const override;
    };

    class ACKMessage : public MessageBase
    {
    public:
        int value;

    public:
        ACKMessage();

        void Decode(const std::string &msg) override;

        std::string Encode() const override;
    };

    class GoodbyeMessage : public MessageBase
    {
    public:
        GoodbyeMessage();

        void Decode(const std::string &msg) override;

        std::string Encode() const override;
    };

    class MessageFilter
    {
    public:
        template <class T>
        using Handler = std::function<void(T msg, Message question)>;

        void Set(Handler<LinkInMessage> && handler);

        void Set(Handler<LinkOutMessage> && handler);

        void Set(Handler<PingMessage> && handler);

        void Set(Handler<ACKMessage> && handler);

        void Set(Handler<GoodbyeMessage> && handler);

    public:
        void Decode(const std::string & msg);

        std::string Encode(const MessageBase & msg, Message question = None);

    private:
        Handler<LinkInMessage>  _on_link_in;
        Handler<LinkOutMessage> _on_link_out;
        Handler<PingMessage>    _on_ping;
        Handler<ACKMessage>     _on_ack;
        Handler<GoodbyeMessage> _on_goodbye;
    };
}
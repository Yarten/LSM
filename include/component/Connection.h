//
// Created by yarten on 19-2-9.
//

#pragma once

#include <component/Channel.h>
#include <component/Message.h>
#include <thread>
#include <atomic>

namespace lsm
{
    class Connection
    {
    public:
        Connection(const std::string & name, const std::string & sender, const std::string & receiver);

        ~Connection();

        using OnTimeout = std::function<void(Connection &)>;

        using OnUpdate = std::function<void(Connection &, std::string)>;

        void SetOnUpdate(OnUpdate && handler);

        void SetOnTimeout(OnTimeout && handler);

        void SetMessageFilter(MessageFilter filter);

        void SetTimeout(const Time & time);

    public:
        bool Create(size_t length);

        bool Connect();

        bool Send(const MessageBase & msg, Message question = None);

        bool Send(const std::string & msg);

    private:
        void StartListen();

        void StopListen();

        void _ListeningMessage();

        void _ListeningData();

    private:
        Channel _send, _recv, _data;
        std::string _name;

        OnUpdate      _on_update;
        OnTimeout     _on_timeout;
        MessageFilter _message_filter;

        Ptr<std::thread>  _recv_listener;
        Ptr<std::thread>  _data_listener;
        std::atomic<bool> _finished;
        std::atomic<bool> _data_listener_finished;
        std::atomic<bool> _data_no_update;
        int               _watch_dog;
    };
}
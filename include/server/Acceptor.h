//
// Created by yarten on 19-2-7.
//

#pragma once

#include <kernel/Manager.h>
#include <thread>
#include <atomic>
#include <functional>

namespace lsm
{
    class Acceptor
    {
    public:
        explicit Acceptor(const std::string & name);

        ~Acceptor();

        using OnNewClient = std::function<void(const std::string & name, size_t id)>;

    public:
        void Run();

        void SetOnNewClient(OnNewClient && handler);

        void SetTimeout(const Time & time);

    private:
        void _Listening();

    private:
        std::string       _name;
        std::string       _public_area_name;

        Ptr<Manager>      _manager;
        Ptr<std::thread>  _listener;
        std::atomic<bool> _finished;

        Segment<size_t>   _cst, _amt, _ack;
        OnNewClient       _on_new_client;
        Time              _timeout;
    };
}
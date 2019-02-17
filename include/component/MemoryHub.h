//
// Created by yarten on 19-2-7.
//
#pragma once

#include <kernel/Manager.h>
#include <mutex>
#include <map>

namespace lsm
{
    class MemoryHub
    {
    public:
        static Ptr<MemoryHub> Instance();

        ~MemoryHub();

    private:
        MemoryHub() = default;

        static Ptr<MemoryHub> _instance;

    public:
        bool Find(const std::string & name);

        void Remove(const std::string & name);

        Ptr<Manager> Create(const std::string & name, size_t length);

    private:
        std::mutex _mutex_this;
        std::map<std::string, Ptr<Manager>> _managers;
    };
}
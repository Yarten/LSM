//
// Created by yarten on 19-2-7.
//

#pragma once

#include <map>
#include <string>
#include <mutex>
#include <kernel/Common.h>

namespace lsm
{
    template <class TLock>
    class NamedLock
    {
    public:
        std::unique_lock<TLock> Locking(const std::string & name);

    private:
        std::map<std::string, lsm::Ptr<TLock>> _locks;
        std::mutex _mutex_this;
    };

    using NamedMutexLock = NamedLock<std::mutex>;
    using NamedRecursiveLock = NamedLock<std::recursive_mutex>;

    template<class TLock>
    std::unique_lock<TLock> NamedLock<TLock>::Locking(const std::string &name)
    {
        std::unique_lock<std::mutex> lock(_mutex_this);

        if(not _locks[name])
            _locks[name] = New<TLock>();

        return std::move(std::unique_lock<TLock>(*_locks[name]));
    }

}
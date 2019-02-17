#include <component/MemoryHub.h>

namespace lsm
{
    Ptr<MemoryHub> MemoryHub::_instance;

    Ptr<MemoryHub> MemoryHub::Instance()
    {
        if(not _instance)
            _instance.reset(new MemoryHub());
        return _instance;
    }

    MemoryHub::~MemoryHub()
    {
        for(const auto &it : _managers)
        {
            if(it.second)
                it.second->Remove();
        }
    }

    bool MemoryHub::Find(const std::string &name)
    {
        std::unique_lock<std::mutex> lock(_mutex_this);
        return _managers.find(name) != _managers.end();
    }

    void MemoryHub::Remove(const std::string &name)
    {
        std::unique_lock<std::mutex> lock(_mutex_this);
        auto it = _managers.find(name);
        if(it != _managers.end())
        {
            it->second->Remove();
            _managers.erase(it);
        }
    }

    Ptr<Manager> MemoryHub::Create(const std::string &name, size_t length)
    {
        std::unique_lock<std::mutex> lock(_mutex_this);

        auto it = _managers.find(name);
        if(it != _managers.end())
            it->second->Remove();

        _managers[name] = New<lsm::Manager>();
        _managers[name]->Remove(name); // 防止因为server的不正常退出导致没有正常remove
        _managers[name]->Create(name, length);

        return _managers[name];
    }
}


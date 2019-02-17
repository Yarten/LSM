#include <kernel/Manager.h>

using namespace boost::interprocess;

namespace lsm
{

    void Manager::Create(const std::string &name, size_t length)
    {
        try
        {
            length += 512; // boost内部管理额外需要的内存
            _mem = New<managed_shared_memory>(create_only, name.c_str(), length);
            _name = name;
        }
        catch (interprocess_exception & ec)
        {
            throw Exception(ErrorCode::CreateFailed, name);
        }
    }

    void Manager::Connect(const std::string &name)
    {
        try
        {
            _mem = New<managed_shared_memory>(open_only, name.c_str());
            _name = name;
        }
        catch (interprocess_exception & ec)
        {
            throw Exception(ErrorCode::ConnectFailed, name);
        }
    }

    bool Manager::TryCreate(const std::string &name, size_t length)
    {
        try
        {
            Create(name, length);
            return true;
        }
        catch (Exception & ec)
        {
            return false;
        }
    }

    bool Manager::TryConnect(const std::string &name)
    {
        try
        {
            Connect(name);
            return true;
        }
        catch (Exception & ec)
        {
            return false;
        }
    }

    bool Manager::TryConnectUntil(const std::string &name, const Time &time)
    {
        return time.Try([&]() { return TryConnect(name); });
    }

    bool Manager::Remove()
    {
        return Remove(_name);
    }

    bool Manager::Remove(const std::string &name)
    {
        return shared_memory_object::remove(name.c_str());
    }

    bool Manager::Grow(size_t bytes)
    {
        return managed_shared_memory::grow(_name.c_str(), bytes);
    }

    bool Manager::Fit()
    {
        return managed_shared_memory::shrink_to_fit(_name.c_str());
    }
}

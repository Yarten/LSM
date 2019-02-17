#include <component/Channel.h>
#include <component/MemoryHub.h>

using namespace boost::interprocess;

namespace lsm
{

    Channel::Channel(const std::string &name)
        : _name(name), _created(false), _timeout(Time::Second(3))
    {}

    Channel::~Channel()
    {
        Remove();
    }

    bool Channel::Create(size_t length)
    {
        size_t total_length = length + sizeof(interprocess_mutex);

        _created = true;
        _manager = MemoryHub::Instance()->Create(_name, total_length);
        auto it = _manager->ConstructArray<char>("data", length, 0);
        _mutex = _manager->Construct<interprocess_mutex>("mutex");
        _last_ack = 0;
        _first_read = true;

        return true;
    }

    bool Channel::Connect()
    {
        _created = false;
        _manager = New<Manager>();
        bool status = _manager->TryConnectUntil(_name, _timeout);

        if(not status)
            return false;

        _mutex = _manager->TryOpenUntil<interprocess_mutex>("mutex", _timeout);
        _last_ack = 0;
        _first_read = true;

        return true;
    }

    void Channel::Remove()
    {
        if(_created)
            MemoryHub::Instance()->Remove(_name);
        _created = false;
    }

    void Channel::SetTimeout(const Time &time)
    {
        _timeout = time;
    }

    bool Channel::Write(const std::string &message)
    {
        if(not _mutex)
            throw Exception(ErrorCode::ManagerUninitialized, "Channel Write");

        auto mutex = _mutex.Data();
        if(not mutex->timed_lock(_timeout.raw()))
            throw Exception(ErrorCode::Timeout, "Channel Write");

        auto data = _manager->TryOpen<char>("data");
        if(not data)
        {
            printf("channel write data null\n");
            mutex->unlock();
            return false;
        }

        size_t length = message.length();
        size_t total_length = length + 1;
        if(total_length > data.Length())
        {
            _manager->Destroy<char>("data");
            if(not _manager->Grow(total_length))
            {
                mutex->unlock();
                return false;
            }

            data = _manager->ConstructArray<char>("data", total_length);
            if(not data)
            {
                mutex->unlock();
                return false;
            }
        }

        char * ptr = data.Data().get();
        _last_ack = ++ptr[0];
        _first_read = false;

        std::memcpy(ptr + 1, message.data(), length);
        std::memset(ptr + 1 + length, 0, data.Length() - length - 1);

        mutex->unlock();
        return true;
    }

    bool Channel::Wait(const Time &time)
    {
        return time.Try([&]()
        {
            if(not _mutex)
                throw Exception(ErrorCode::ManagerUninitialized, "Channel Wait");

            auto mutex = _mutex.Data();

            if(not mutex->timed_lock(_timeout.raw()))
                throw Exception(ErrorCode::Timeout, "Channel Wait");

            auto data = _manager->TryOpen<char>("data");
            if(not data)
            {
                printf("channel wait data null\n");
                mutex->unlock();
                Time::Millisecond(1).Sleep();
                return false;
            }

            auto ptr = data.Data().get();
            bool status = true;

            if(_first_read)
            {
                _first_read = false;
                _last_ack = ptr[0];
            }
            else if(_last_ack != ptr[0])
            {
                _last_ack = ptr[0];
            }
            else status = false;

            mutex->unlock();
            Time::Millisecond(1).Sleep();
            return status;
        });
    }

    bool Channel::Read(std::string &message)
    {
        if(not _mutex)
            throw Exception(ErrorCode::ManagerUninitialized, "Channel Read");

        auto mutex = _mutex.Data();
        if(not mutex->timed_lock(_timeout.raw()))
            throw Exception(ErrorCode::Timeout, "Channel Read");

        auto data = _manager->TryOpen<char>("data");
        if(not data)
        {
            printf("channel read data null\n");
            mutex->unlock();
            return false;
        }

        message = std::string(data.Data().get() + 1, data.Length() - 1);

        mutex->unlock();
        return true;
    }
}


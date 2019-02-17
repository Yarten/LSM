#include <client/LSM.h>
#include <sstream>

using namespace std;

namespace lsm
{
    static size_t InstanceCount = 0;
    static std::mutex InstanceCountMutex;

    LSM::LSM(const std::string &server) :
        _name(server)
    {
        unique_lock<mutex> lock(InstanceCountMutex);
        _id = InstanceCount++;
    }

    LSM::~LSM()
    {
        Disconnect();
    }

    void LSM::SetOnReceive(LSM::OnReceive &&handler)
    {
        _on_receive = handler;
    }

    void LSM::SetOnError(LSM::OnError &&handler)
    {
        _on_error = handler;
    }

    bool LSM::Connect(Time time)
    {
        Disconnect();

        Manager manager;
        if(not manager.TryConnectUntil(GetPublicArea(_name), time))
            return false;

        auto cst = manager.TryOpen<size_t>(HelloCST);
        if(not cst) return false;

        auto amt = manager.TryOpen<size_t>(HelloAMT);
        if(not amt) return false;

        auto ack = manager.TryOpen<size_t>(HelloACK);
        if(not ack) return false;

        stringstream stream;
        stream << this_thread::get_id();
        size_t pid;
        stream >> pid;
        pid += _id;

        bool status = time.Try([&]()
        {
            *cst.Data() = pid;
            Time::Millisecond(1).Sleep();

            if(*amt.Data() != pid)
                return false;

            *ack.Data() = pid;
            return true;
        });

        if(not status) return false;

        string private_area_name = GetPrivateArea(_name, pid);
        unique_lock<mutex> lock(_mutex_this);
        _connection = New<Connection>(private_area_name, private_area_name + "client", private_area_name + "server");

        MessageFilter filter;
        filter.Set(std::bind(&LSM::_HandleACKMessage, this, std::placeholders::_1, std::placeholders::_2));
        _connection->SetMessageFilter(filter);
        _connection->SetOnTimeout(std::bind(&LSM::_HandleConnectionBroken, this, std::placeholders::_1));

        if(not _connection->Connect())
        {
            _connection.reset();
            return false;
        }
        else return true;
    }

    void LSM::Disconnect()
    {
        unique_lock<mutex> lock(_mutex_this);
        if(not _connection) return;

        _channels.clear();
        _connection->Send(GoodbyeMessage());
        _connection.reset();
    }

    bool LSM::Open(const std::string &name, Time time)
    {
        if(unique_lock<mutex> lock(_mutex_this); not _connection)
            return false;

        LinkInMessage link_in;
        link_in.name = name;
        link_in.reserve = 1024;

        _msg_ack = None;
        _connection->Send(link_in);

        if(not time.Try([&]() { return _msg_ack == LinkIn; }))
            return false;

        string swap_name = GetSwapArea(_name, name);
        auto channel = New<Connection>(swap_name, swap_name + "client", swap_name + "server");
        channel->SetOnUpdate(std::bind(&LSM::_HandleUpdate, this, name, std::placeholders::_1, std::placeholders::_2));
        channel->SetOnTimeout(std::bind(&LSM::_HandleChannelBroken, this, name, std::placeholders::_1));

        if(not channel->Connect())
            return false;

        unique_lock<mutex> lock(_mutex_this);
        _channels[name] = channel;
        return true;
    }

    void LSM::Close(const std::string &name)
    {
        unique_lock<mutex> lock(_mutex_this);
        _channels.erase(name);
    }

    bool LSM::Send(const std::string &name, const std::string &msg)
    {
        Ptr<Connection> channel;
        {
            unique_lock<mutex> lock(_mutex_this);
            auto it = _channels.find(name);
            if(it == _channels.end())
                return false;

            channel = it->second;
        }

        return channel->Send(msg);
    }

    void LSM::_HandleACKMessage(ACKMessage msg, Message question)
    {
        if(msg.value)
            _msg_ack = question;
    }

    void LSM::_HandleUpdate(const std::string &channel, Connection &connection, std::string msg)
    {
        if(_on_receive)
            _on_receive(*this, channel, std::move(msg));
    }

    void LSM::_HandleConnectionBroken(Connection &connection)
    {
        LockedDetach([&]()
                     { InnerHandleConnectionBroken(); }
        );
    }

    void LSM::_HandleChannelBroken(const std::string &channel, Connection &connection)
    {
        LockedDetach([&]()
                     { InnerHandleChannelBroken(channel); }
        );
    }

    void LSM::InnerHandleConnectionBroken()
    {
        _channels.clear();
        _connection.reset();
        if(_on_error)
            _on_error(*this, "");
    }

    void LSM::InnerHandleChannelBroken(const std::string & channel)
    {
        _channels.erase(channel);
    }

    void LSM::LockedDetach(std::function<void()> func)
    {
        auto th = New<thread>([&, func]() {
            unique_lock<mutex> lock(_mutex_this);
            if(func) func();
        });
        th->detach();
    }
} // Namespace lsm


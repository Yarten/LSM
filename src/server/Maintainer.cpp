#include <server/Maintainer.h>

using namespace std;

namespace lsm
{

    Maintainer::Maintainer(const std::string &name)
        : _name(name)
    {}

    void Maintainer::AddClient(const std::string &name)
    {
        auto client = New<Connection>(name, name + "server", name + "client");

        MessageFilter filter;

        filter.Set(std::bind(&Maintainer::_HandleLinkInMessage,  this, name, std::placeholders::_1, std::placeholders::_2));
        filter.Set(std::bind(&Maintainer::_HandleLinkOutMessage, this, name, std::placeholders::_1, std::placeholders::_2));
        filter.Set(std::bind(&Maintainer::_HandleACKMessage,     this, name, std::placeholders::_1, std::placeholders::_2));
        filter.Set(std::bind(&Maintainer::_HandleGoodbyeMessage, this, name, std::placeholders::_1, std::placeholders::_2));

        client->SetMessageFilter(filter);
        client->SetOnTimeout(std::bind(&Maintainer::_HandleClientTimeout, this, name, std::placeholders::_1));
        client->SetTimeout(Time::Second(10));
        
        if(client->Create(0))
        {
            unique_lock<mutex> lock(_mutex_this);
            _clients[name] = client;
            cout << "Create private area for new client [" << name << "]." << endl;
        }
        else
            cerr << "Fail to create private area for client [" << name << "]." << endl;
    }

    void Maintainer::RemoveClient(const std::string &name)
    {
        LockedDetach([&, name](){
            auto it = _clients.find(name);
            if (it != _clients.end())
            {
                _clients.erase(it);
                cout << "Remove a private area of a client [" << name << "]." << endl;
            }
        });
    }

    void Maintainer::_HandleLinkInMessage(const std::string &client, LinkInMessage msg, Message question)
    {
        unique_lock<mutex> lock(_mutex_this);
        auto it = _clients.find(client);
        if(it == _clients.end())
            return;

        ACKMessage ack;
        ack.value = 1;

        string name = GetSwapArea(_name, msg.name);
        if(_data.find(name) == _data.end())
        {
            auto data = New<Connection>(name, name + "server", name + "client");
            data->SetOnTimeout(std::bind(&Maintainer::_HandleDataTimeout, this, name, std::placeholders::_1));
            data->SetTimeout(Time::Second(10));
            
            if(data->Create(msg.reserve))
            {
                _data[name] = data;
                cout << "Create swap area for new data channel [" << name << "]." << endl;
            }
            else
            {
                ack.value = 0;
                cerr << "Fail to create swap area for data channel [" << name << "]." << endl;
            }
        }

        it->second->Send(ack, Message::LinkIn);
    }

    void Maintainer::_HandleLinkOutMessage(const std::string &client, LinkOutMessage msg, Message question)
    {
        // Do nothing here ...
    }

    void Maintainer::_HandleACKMessage(const std::string &client, ACKMessage msg, Message question)
    {
        // Do nothing here ...
    }

    void Maintainer::_HandleGoodbyeMessage(const std::string &client, GoodbyeMessage msg, Message question)
    {
        RemoveClient(client);
    }

    void Maintainer::_HandleClientTimeout(const std::string &client, Connection &connection)
    {
        RemoveClient(client);
    }

    void Maintainer::_HandleDataTimeout(const std::string &name, Connection &connection)
    {
        LockedDetach([&](){
            auto it = _data.find(name);
            if (it != _data.end())
            {
                _data.erase(it);
                cout << "Remove a swap area [" << name << "]." << endl;
            }
        });
    }

    void Maintainer::LockedDetach(std::function<void()> func)
    {
        auto th = New<thread>([&, func]() {
            unique_lock<mutex> lock(_mutex_this);
            if(func) func();
        });
        th->detach();
    }
}


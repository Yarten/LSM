#include <component/Connection.h>
#include <kernel/Time.h>
using namespace std;

namespace lsm
{

    Connection::Connection(const std::string &name, const std::string &sender, const std::string &receiver)
        : _send(sender), _recv(receiver), _data(name), _name(name)
    {
        SetMessageFilter(MessageFilter());
    }

    Connection::~Connection()
    {
        StopListen();
    }

    bool Connection::Create(size_t length)
    {
        bool status = _send.Create(256) and _recv.Create(256) and _data.Create(length);
        if(status)
            StartListen();
        return status;
    }

    bool Connection::Connect()
    {
        bool status =  _send.Connect() and _recv.Connect() and _data.Connect();
        if(status)
            StartListen();
        return status;
    }

    void Connection::SetOnUpdate(Connection::OnUpdate &&handler)
    {
        _on_update = handler;
    }

    void Connection::SetOnTimeout(Connection::OnTimeout &&handler)
    {
        _on_timeout = handler;
    }

    void Connection::SetMessageFilter(MessageFilter filter)
    {
        _message_filter = filter;
        _message_filter.Set([&](PingMessage msg, Message question) {
            Send(ACKMessage()); // 自动回复ping消息
        });
    }

    void Connection::StartListen()
    {
        StopListen();
        _finished = false;
        _data_listener_finished = false;
        _data_no_update = true;

        _recv_listener = New<thread>(&Connection::_ListeningMessage, this);
        _data_listener = New<thread>(&Connection::_ListeningData, this);
    }

    void Connection::StopListen()
    {
        _finished = true;

        if(_recv_listener and _recv_listener->joinable())
            _recv_listener->join();

        if(_data_listener and _data_listener->joinable())
            _data_listener->join();

        _recv_listener.reset();
        _data_listener.reset();
    }

    void Connection::_ListeningMessage()
    {
        _watch_dog = 0;

        while (not _finished)
        {
            Time::Microsecond(100).Sleep();

            try
            {
                if(_data_listener_finished)
                    throw Exception(ErrorCode::Timeout, "connection data receive");

                if(_recv.Wait(Time::Millisecond(100)))
                {
                    if(string msg; _recv.Read(msg))
                    {
                        _message_filter.Decode(msg);
                        _watch_dog = 0;
                    }
                }
                else
                {
                    if(_data_no_update)
                        Send(PingMessage());
                    if(++_watch_dog > 30)
                        throw Exception(ErrorCode::Timeout, "connection data update");
                }
            }
            catch (Exception & ec)
            {
                _finished = true;
                if(_data_listener and _data_listener->joinable())
                    _data_listener->join();

                if(_on_timeout)
                    _on_timeout(*this);

                cerr << "Connection [" << _name << "] broken. error: " << ec.what() << endl;
                break;
            }
        }
    }

    void Connection::_ListeningData()
    {
        while (not _finished)
        {
            Time::Microsecond(100).Sleep();

            try
            {
                if(_data.Wait(Time::Millisecond(100)))
                {
                    string msg;
                    if(_on_update and _data.Read(msg))
                        _on_update(*this, std::move(msg));
                    _data_no_update = false;
                }
                else
                {
                    _data_no_update = true; // 交给Message Listener处理
                }
            }
            catch (Exception & ec)
            {
                _data_listener_finished = true; // 交给Message Listener处理
                break;
            }
        }
    }

    bool Connection::Send(const MessageBase &msg, Message question)
    {
        return _send.Write(_message_filter.Encode(msg, question));
    }

    bool Connection::Send(const std::string &msg)
    {
        return _data.Write(msg);
    }

    void Connection::SetTimeout(const Time &time)
    {
        _recv.SetTimeout(time);
        _send.SetTimeout(time);
        _data.SetTimeout(time);
    }
}


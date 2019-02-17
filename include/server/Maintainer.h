//
// Created by yarten on 19-2-7.
//
#pragma once

#include <component/Connection.h>

namespace lsm
{
    class Maintainer
    {
    public:
        explicit Maintainer(const std::string & name);

        void AddClient(const std::string & name);

        void RemoveClient(const std::string & name);

    private:
        void _HandleLinkInMessage(const std::string & client, LinkInMessage msg, Message question);

        void _HandleLinkOutMessage(const std::string & client, LinkOutMessage msg, Message question);

        void _HandleACKMessage(const std::string & client, ACKMessage msg, Message question);

        void _HandleGoodbyeMessage(const std::string & client, GoodbyeMessage msg, Message question);

        void _HandleClientTimeout(const std::string & client, Connection & connection);

        void _HandleDataTimeout(const std::string & name, Connection & connection);
        
        void LockedDetach(std::function<void()> func);

    private:
        std::string _name;
        std::mutex  _mutex_this;

        std::map<std::string, Ptr<Connection>> _clients;
        std::map<std::string, Ptr<Connection>> _data;
    };
}
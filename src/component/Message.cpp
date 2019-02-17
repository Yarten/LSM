#include <component/Message.h>
#include <utils/String.h>

namespace lsm
{
    MessageBase::MessageBase(Message type)
        : type(type)
    {}

    void LinkInMessage::Decode(const std::string &msg)
    {
        name.resize(256, 0);
        FormatIn(msg, "%s %lu", name[0], reserve);
        Trim(name);
    }

    std::string LinkInMessage::Encode() const
    {
        return FormatOut("%s %lu", name.c_str(), reserve);
    }

    LinkInMessage::LinkInMessage()
        : MessageBase(LinkIn)
    {}

    void LinkOutMessage::Decode(const std::string &msg)
    {
        name = msg;
        Trim(name);
    }

    std::string LinkOutMessage::Encode() const
    {
        return name;
    }

    LinkOutMessage::LinkOutMessage()
        : MessageBase(LinkOut)
    {}

    void PingMessage::Decode(const std::string &msg)
    {

    }

    std::string PingMessage::Encode() const
    {
        return "";
    }

    PingMessage::PingMessage()
        : MessageBase(Ping)
    {}

    void ACKMessage::Decode(const std::string &msg)
    {
        FormatIn(msg, "%d", value);
    }

    std::string ACKMessage::Encode() const
    {
        return FormatOut("%d", value);
    }

    ACKMessage::ACKMessage()
        : MessageBase(ACK)
    {}

    void GoodbyeMessage::Decode(const std::string &msg)
    {

    }

    std::string GoodbyeMessage::Encode() const
    {
        return "";
    }

    GoodbyeMessage::GoodbyeMessage()
        : MessageBase(Goodbye)
    {}

    void MessageFilter::Set(MessageFilter::Handler<lsm::LinkInMessage> &&handler)
    {
        _on_link_in = handler;
    }

    void MessageFilter::Set(MessageFilter::Handler<lsm::LinkOutMessage> &&handler)
    {
        _on_link_out = handler;
    }

    void MessageFilter::Set(MessageFilter::Handler<lsm::PingMessage> &&handler)
    {
        _on_ping = handler;
    }

    void MessageFilter::Set(MessageFilter::Handler<lsm::ACKMessage> &&handler)
    {
        _on_ack = handler;
    }

    void MessageFilter::Set(MessageFilter::Handler<lsm::GoodbyeMessage> &&handler)
    {
        _on_goodbye = handler;
    }

    void MessageFilter::Decode(const std::string &msg)
    {
        auto pos = msg.find(':');
        auto header = msg.substr(0, pos);
        auto data = msg.substr(pos + 1);

        int answer, question;
        FormatIn(header, "%d %d", answer, question);

        auto question_ = Message(question);
        switch (Message(answer))
        {
            case None: break;

            case LinkIn:
                if(_on_link_in)
                {
                    LinkInMessage message;
                    message.Decode(data);
                    _on_link_in(message, question_);
                }
                break;

            case LinkOut:
                if(_on_link_out)
                {
                    LinkOutMessage message;
                    message.Decode(data);
                    _on_link_out(message, question_);
                }
                break;

            case ACK:
                if(_on_ack)
                {
                    ACKMessage message;
                    message.Decode(data);
                    _on_ack(message, question_);
                }
                break;

            case Ping:
                if(_on_ping)
                {
                    PingMessage message;
                    message.Decode(data);
                    _on_ping(message, question_);
                }
                break;

            case Goodbye:
                if(_on_goodbye)
                {
                    GoodbyeMessage message;
                    message.Decode(data);
                    _on_goodbye(message, question_);
                }
                break;
        }
    }

    std::string MessageFilter::Encode(const MessageBase &msg, Message question)
    {
        return FormatOut("%d %d:%s", int(msg.type), int(question), msg.Encode().c_str());
    }
}


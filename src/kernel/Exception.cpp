#include <kernel/Exception.h>

namespace lsm
{
    Exception::Exception(const std::string &msg)
        : _msg(msg), _code(ErrorCode::Undefined)
    {}

    const char *Exception::what() const noexcept
    {
        return _msg.c_str();
    }

    Exception::Exception(ErrorCode::Code code)
        : _msg(ErrorCode::What(code)), _code(code)
    {}

    Exception::Exception(ErrorCode::Code code, const std::string &msg)
        : _msg(ErrorCode::What(code) + " [" + msg + "]"), _code(code)
    {}

    ErrorCode::Code Exception::code() const noexcept
    {
        return _code;
    }

    ErrorCode::ErrorCode(ErrorCode::Code code)
        : _code(code), _message(What(code))
    {}

    ErrorCode::Code ErrorCode::code() const
    {
        return _code;
    }

    const std::string &ErrorCode::message() const
    {
        return _message;
    }

    std::string ErrorCode::What(ErrorCode::Code code)
    {
        switch (code)
        {
            case CreateFailed:
                return "Fail to create memory block";

            case ConnectFailed:
                return "Fail to connect to memory block";

            case ManagerUninitialized:
                return "Manager doesn't initialize by .Create or .Connect";

            case SegmentAlreadyExisted:
                return "Segment with the same name is already existed";

            case SegmentUnExisted:
                return "The specified segment doesn't exit";

            case SegmentUnMatched:
                return "The specified segment doesn't match length";

            case InsufficientMemory:
                return "Not enough memory to use";

            case Timeout:
                return "Operation timeout";

            case None:
                return "None";

            case Undefined:
            default:
                return "Undefined";
        }
    }

    ErrorCode::operator bool() const
    {
        return _code == None;
    }
}


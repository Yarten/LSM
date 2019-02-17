//
// Created by yarten on 19-2-5.
//
#pragma once

#include <exception>
#include <string>

namespace lsm
{
    class ErrorCode
    {
    public:
        enum Code
        {
            Undefined, None,
            CreateFailed,
            ConnectFailed,
            ManagerUninitialized,
            SegmentAlreadyExisted,
            SegmentUnExisted,
            SegmentUnMatched,
            InsufficientMemory,
            Timeout
        };

    public:
        ErrorCode(Code code);

        Code code() const;

        const std::string & message() const;

        explicit operator bool() const;

    public:
        static std::string What(Code code);

    private:
        Code        _code;
        std::string _message;
    };

    class Exception : public std::exception
    {
    public:
        explicit Exception(const std::string & msg);

        explicit Exception(ErrorCode::Code code);

        explicit Exception(ErrorCode::Code code, const std::string & msg);

        const char *what() const noexcept override;

        ErrorCode::Code code() const noexcept;

    private:
        std::string     _msg;
        ErrorCode::Code _code;
    };
}
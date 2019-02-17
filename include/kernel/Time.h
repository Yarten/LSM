//
// Created by yarten on 19-2-6.
//
#pragma once

#include <cstddef>
#include <functional>
#include <boost/date_time.hpp>

namespace lsm
{
    class Time
    {
    public:
        static Time Second(double value);

        static Time Millisecond(double value);

        static Time Microsecond(double value);

    public:
        void Sleep() const;

        bool Try(const std::function<bool()> & func) const;

        boost::posix_time::ptime raw();

    private:
        explicit Time(size_t us);

    private:
        size_t _us;
    };
}
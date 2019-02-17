#include <kernel/Time.h>
#include <thread>
#include <chrono>
#include <boost/timer.hpp>
using namespace std;

namespace lsm
{

    Time Time::Second(double value)
    {
        return Time(value * 1e6);
    }

    Time Time::Millisecond(double value)
    {
        return Time(value * 1e3);
    }

    Time Time::Microsecond(double value)
    {
        return Time(value);
    }

    void Time::Sleep() const
    {
        this_thread::sleep_for(chrono::microseconds(_us));
    }

    Time::Time(size_t us)
        : _us(us)
    {}

    bool Time::Try(const std::function<bool()> &func) const
    {
        if(not func) return false;

        long us = _us;
        while (true)
        {
            auto t1 = boost::posix_time::microsec_clock::universal_time();
            if(func())
                return true;
            else if(us <= 0)
                break;
            auto t2 = boost::posix_time::microsec_clock::universal_time();

            boost::posix_time::time_duration dt = (t2 - t1);
            this_thread::sleep_for(chrono::microseconds(100));
            us = us - 100 - dt.total_microseconds();
        }

        return false;
    }

    boost::posix_time::ptime Time::raw()
    {
        return boost::posix_time::microsec_clock::universal_time() + boost::posix_time::microseconds(_us);
    }
}


//
// Created by yarten on 19-2-7.
//

#pragma once

#include <string>

namespace lsm
{
    template <typename ... Types>
    std::string FormatOut(const std::string &s, const Types & ... args)
    {
        char buffer[4096];
        sprintf(buffer, s.c_str(), args...);
        return std::string(buffer);
    }

    template <typename ... Types>
    bool FormatIn(const std::string &src, const std::string &fmt, Types &...  args)
    {
        return sscanf(src.c_str(), fmt.c_str(), &args...) != -1;
    }

    void Trim(std::string & s);
}
//
// Created by yarten on 19-2-5.
//
#pragma once

#include <memory>

namespace lsm
{
    template <class T>
    using Ptr = std::shared_ptr<T>;

    template <class T, class ... Args>
    Ptr<T> New(Args && ... args)
    {
        return std::make_shared<T>(args...);
    }

    template <class T>
    Ptr<T> AddressOf(T & data)
    {
        return Ptr<T>(&data, [](T * data_){});
    }

    template <class T>
    Ptr<const T> AddressOf(const T & data)
    {
        return Ptr<const T>(data, [](const T * data_) {});
    }

    template <class T>
    Ptr<T> AddressOf(T * data)
    {
        return Ptr<T>(data, [](T * data_) {});
    }

    template <class T>
    Ptr<const T> AddressOf(const T * data)
    {
        return Ptr<const T>(data, [](const T * data) {});
    }

    template <class T>
    Ptr<const T> AddressOf(T && data)
    {
        return Ptr<const T>(data, [](const T * data_) {});
    }

    /// 逻辑区域名字
    std::string GetPublicArea(const std::string & server);
    std::string GetPrivateArea(const std::string & server, size_t id);
    std::string GetSwapArea(const std::string & server, const std::string & channel);

    /// client连接server步骤所用到的segments
    static const std::string HelloCST("Cybersquatting");
    static const std::string HelloAMT("Admittance");
    static const std::string HelloACK("Acknowledge");
    static const std::string DefaultServerName("_lsm_");
}
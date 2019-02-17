//
// Created by yarten on 19-2-5.
//
#pragma once

#include <string>
#include <kernel/Common.h>

namespace lsm
{
    /**
     * @class Segment
     * @brief 对应一块memory block中的一个字段，使用string和类型来唯一对应。
     * @tparam T 需要指定类型
     */
    template <class T>
    class Segment
    {
    public:
        Segment() = default;

        virtual ~Segment() = default;

        Segment(const std::string & name, T * data, size_t length = 1);

    public:
        void Set(const std::string & name, T * data, size_t length = 1);

        const std::string & Name() const;

        const Ptr<T> & Data() const;

        Ptr<T> & Data();

        size_t  Length() const;

        operator bool() const;

    private:
        std::string _name;
        Ptr<T>      _data;
        size_t      _length = 0;
    };

    template<class T>
    Segment<T>::Segment(const std::string &name, T * data, size_t length)
    {
        Set(name, data, length);
    }

    template<class T>
    const std::string &Segment<T>::Name() const
    {
        return _name;
    }

    template<class T>
    const Ptr<T> &Segment<T>::Data() const
    {
        return _data;
    }

    template<class T>
    Ptr <T> &Segment<T>::Data()
    {
        return _data;
    }

    template<class T>
    size_t Segment<T>::Length() const
    {
        return _length;
    }

    template<class T>
    void Segment<T>::Set(const std::string &name, T * data, size_t length)
    {
        _name = name;
        _data = AddressOf(data);
        _length = length;
    }

    template<class T>
    Segment<T>::operator bool() const
    {
        return bool(_data);
    }
}
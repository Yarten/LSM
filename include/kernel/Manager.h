//
// Created by yarten on 19-2-5.
//
#pragma once

#include <kernel/Common.h>
#include <kernel/Segment.h>
#include <kernel/Exception.h>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <kernel/Time.h>
#include <utils/NamedLock.h>

namespace lsm
{
    /**
     * @class Manager
     * @brief 用于管理一块内存区域，需要先 @see Create 或 @see Connect
     * 创建完内存区域后，才可以创建或打开字段，@see Construct 或 @see Open
     */
    class Manager
    {
    public:
        Manager() = default;

        virtual ~Manager() = default;

    public:
        /**
         * 建立共享内存区域
         * @throw lsm::Exception，若区域已存在，则会抛出异常
         */
        void Create(const std::string & name, size_t length);

        bool TryCreate(const std::string & name, size_t length);

    public:
        /**
         * 删除指定的共享内存区域
         * @return 是否存在该内存区域，并删除成功
         */
        bool Remove();

        bool Remove(const std::string & name);

    public:
        /**
         * 连接到一处共享内存区域
         * @throw lsm::Exception，若内存区域不存在，则会抛出异常
         */
        void Connect(const std::string & name);

        bool TryConnect(const std::string & name);

        bool TryConnectUntil(const std::string &name, const Time &time);

    public:
        bool Grow(size_t bytes);

        bool Fit();

    public:
        /**
         * 在内存区域中创建指定字段
         * @param args 创建字段时构造参数
         * @throw lsm::Exception，若内存区域不存在，或字段已经存在，则会抛出异常
         */
        template <class T, class ... Args>
        Segment<T> Construct(const std::string & name, Args && ... args);

        template <class T, class ... Args>
        Segment<T> ConstructArray(const std::string & name, size_t N, Args && ... args);

        /**
         * 寻找在已经创建的共享内存中指定的字段
         * @throw Exception，若内存区域没有创建，则会抛出异常
         * @return 是否找到指定字段
         */
        template <class T>
        bool Find(const std::string & name);

        /**
         * 删除指定字段
         * @return 是否删除成功
         */
        template <class T>
        bool Destroy(const std::string & name);

    public:
        /**
         * 打开内存区域已有的字段
         * @throw lsm::Exception，若内存区域不存在，或字段不存在，则会抛出异常
         */
        template <class T>
        Segment<T> Open(const std::string & name);

        template <class T>
        Segment<T> TryOpen(const std::string & name);

        template <class T>
        Segment<T> TryOpenUntil(const std::string & name, const Time & time);

    private:
        Ptr<boost::interprocess::managed_shared_memory>  _mem;
        std::string _name;
        NamedMutexLock _locks; // 用于异步操作Segment
    };

    template<class T, class... Args>
    Segment<T> Manager::Construct(const std::string &name, Args &&... args)
    {
        return ConstructArray<T>(name, 1, args...);
    }

    template<class T, class... Args>
    Segment<T> Manager::ConstructArray(const std::string &name, size_t N, Args &&... args)
    {
        _locks.Locking(name);

        if(Find<T>(name))
            throw Exception(ErrorCode::SegmentAlreadyExisted, name);

        try
        {
            return Segment<T>
            (
                name,
                _mem->construct<T>(name.c_str())[N](args...),
                N
            );
        }
        catch (boost::interprocess::bad_alloc & ec)
        {
            throw Exception(ErrorCode::InsufficientMemory, name);
        }
    }

    template<class T>
    bool Manager::Find(const std::string &name)
    {
        if(not _mem)
            throw Exception(ErrorCode::ManagerUninitialized);
        else
        {
            return _mem->find<T>(name.c_str()).first != nullptr;
        }
    }

    template<class T>
    bool Manager::Destroy(const std::string &name)
    {
        if(not _mem)
            throw Exception(ErrorCode::ManagerUninitialized);
        else
            return _mem->destroy<T>(name.c_str());
    }

    template<class T>
    Segment<T> Manager::Open(const std::string &name)
    {
        _locks.Locking(name);

        if (not Find<T>(name))
            throw Exception(ErrorCode::SegmentUnExisted, name);

        auto it = _mem->find<T>(name.c_str());
        return Segment<T>(name, it.first, it.second);
    }

    template<class T>
    Segment<T> Manager::TryOpen(const std::string &name)
    {
        try
        {
            return Open<T>(name);
        }
        catch (Exception & ec)
        {
            return Segment<T>();
        }
    }

    template<class T>
    Segment<T> Manager::TryOpenUntil(const std::string &name, const Time &time)
    {
        Segment<T> r;
        time.Try([&]() { return bool(r = TryOpen<T>(name)); });
        return r;
    }
}

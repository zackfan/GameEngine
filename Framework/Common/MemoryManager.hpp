//
// Created by creator on 19-3-27.
//

//不同模块的对内存的使用情况是不一样的，所以我们把每一种Allocator代表一种分配策略
//这些策略就集中在memory manager中，我们把分配需求和分配器之间用一个映射表(lookup table)来快速实现内存分配
//另一个好处是我可以通过线程和Allocator之间绑定关系，迅速实现线程的本地堆
//现阶段提供一个单层支持不同分配尺寸的，线程不安全的内存管理模块
#pragma once

#include "IRuntimeModule.hpp"
#include "Allocator.hpp"
#include <new>

namespace My
{
    class MemoryManager : implements IRuntimeModule
    {
    public:
        template<typename T, typename ... Arguments>
        T *New(Arguments... parameters)
        {
            return new(Allocator(sizeof(T))) T(parameters...);
        }

        template<typename T>
        void Delete(T *p)
        {
            reinterpret_cast<T *>(p)->~T();
            Free(p, sizeof(T));
        }

    public:
        virtual ~MemoryManager()
        {}

        virtual int Initialize();

        virtual void Finalize();

        virtual void Tick();

        void *Allocate(size_t size);

        void Free(void *p, size_t size);

    private:
        static size_t *m_pBlockSizeLookup;
        static Alloccator *m_pAllocators;
    private:
        static Allocator *LookUpAllocator(size_t size);
    };
}

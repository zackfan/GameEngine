//
// Created by creator on 19-3-27.
//

#include "MemoryManager.hpp"
#include <malloc.h>

using namespace My;

namespace My
{
    static const uint32_t kBlockSizes[] = {
            // 4-increments
            4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48,
            52, 56, 60, 64, 68, 72, 76, 80, 84, 88, 92, 96,

            // 32-increments
            128, 160, 192, 224, 256, 288, 320, 352, 384,
            416, 448, 480, 512, 544, 576, 608, 640,

            // 64-increments
            704, 768, 832, 896, 960, 1024
    };

    static const uint32_t kPageSize = 8192;
    static const uint32_t kAlignment = 4;

    //number of elements in the blocksizes array
    static const uint32_t kNumBlockSizes = sizeof(kBlockSizes) / sizeof(kBlockSizes[0]);

    static const uint32_t kMaxBlockSize = kBlockSizes[kNumBlockSizes - 1];
}

int My::MemoryManager::Initialize()
{
    //one-time initialization
    static bool s_bInitialized = false;
    if (!s_bInitialized)
    {
        //initialize block size lookup table(查找表)
        //lookup table=0,0,0,0,0,1,1,1,1,2,2,2,2,
        // ...,kNumBlockSizes,..(64)..,kNumBlockSizes
        //意在当想分配4~8(uint8_t)时,m_pBlockSizeLookup的4~8位置都是2
        //指向kBlockSizes[2]=8,即都分配8(uint8_t)的空间
        //这里kMaxBlockSize + 1意思是即使我想分配空间0的时候也能分配到最小的4
        m_pBlockSizeLookup = new size_t[kMaxBlockSize + 1];
        size_t j = 0;
        for (size_t i = 0; i <= kMaxBlockSize; i++)
        {
            if (i > kBlockSizes[j]) ++j;
            m_pBlockSizeLookup[i] = j;
        }

        //initialize the allocators
        m_pAllocators = new Allocator[kNumBlockSizes];
        for (size_t i = 0; i < kNumBlockSizes; i++)
        {
            //有kNumBlockSizes个不同大小的分配器,用时从中选择合适的
            m_pAllocators[i].Reset(kBlockSizes[i], kPageSize, kAlignment);
        }

        s_bInitialized = true;
    }

    return 0;
}

void My::MemoryManager::Finalize()
{
    delete[] m_pAllocators;
    delete[] m_pBlockSizeLookup;
}

void My::MemoryManager::Tick()
{

}

Allocator *My::MemoryManager::LookUpAllocator(size_t size)
{
    //check eligibility for lookup
    if (size <= kMaxBlockSize)
        return m_pAllocators + m_pBlockSizeLookup[size];
    else
        return nullptr;
}

void *My::MemoryManager::Allocate(size_t size)
{
    Allocator *pAlloc = LookUpAllocator(size);
    if (pAlloc)
        return pAlloc->Allocate();
    else
        return malloc(size);
}

void My::MemoryManager::Free(void *p,size_t size)
{
    Allocator *pAlloc = LookUpAllocator(size);
    if(pAlloc)
        pAlloc->Free(p);
    else
        free(p);
}


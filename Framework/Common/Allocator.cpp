//
// Created by creator on 19-3-25.
//

#include "Allocator.hpp"
#include <cassert>
#include <cstring>

#ifndef ALIGN
#define ALIGN(x, a)  (((x)+((a)-1))&~((a)-1))
#endif

using namespace My;

My::Allocator::Allocator()
        :m_pPageList(nullptr),m_pFreeList(nullptr),
         m_szPageSize(0),m_szBlockSize(0),
         m_szDataSize(0),m_szAlignmentSize(0),m_nBlockPerPage(0)
{
}

My::Allocator::Allocator(size_t data_size, size_t page_size, size_t alignment)
        : m_pPageList(nullptr), m_pFreeList(nullptr)
{
    Reset(data_size, page_size, alignment);
}

My::Allocator::~Allocator()
{
    FreeAll();
}

//仅设置分配的一些参数，还未实际分配
void My::Allocator::Reset(size_t data_size, size_t page_size, size_t alignment)
{
    FreeAll();

    m_szDataSize = data_size;
    m_szPageSize = page_size;

    size_t minimal_size = (sizeof(BlockHeader) > m_szDataSize) ?
                          sizeof(BlockHeader) : m_szDataSize;

    //alignment must be 2^n
#if defined(_DEBUG)
    assert(alignment>0&&((alignment&(alignment-1)))==0);
#endif

    m_szBlockSize = ALIGN(minimal_size, alignment);

    m_szAlignmentSize = m_szBlockSize - minimal_size;

    m_nBlockPerPage = (m_szPageSize - sizeof(PageHeader)) / m_szBlockSize;
}

void *My::Allocator::Allocate()
{
    //如果本身没有一点内存则先申请一个page
    if (!m_pFreeList)
    {
        //allocate a new page
        PageHeader *pNewPage = reinterpret_cast<PageHeader *>(new uint8_t[m_szPageSize]);
        ++m_nPages;
        m_nBlocks += m_nBlockPerPage;
        m_nFreeBlocks += m_nBlockPerPage;

#if defined(_DEBUG)
        FillFreePage(pNewPage);
#endif

        if (m_pPageList)
        {
            pNewPage->pNext = m_pPageList;    //头插法
        }

        m_pPageList = pNewPage;

        BlockHeader *pBlock = pNewPage->Blocks();

        //link each block in the page
        for (uint32_t i = 0; i < m_nBlockPerPage - 1; ++i)
        {
            pBlock->pNext = NextBlock(pBlock);
            pBlock = NextBlock(pBlock);
        }
        pBlock->pNext = nullptr;

        m_pFreeList = pNewPage->Blocks();
    }

    //申请一个block
    BlockHeader *freeBlock = m_pFreeList;
    m_pFreeList = m_pFreeList->pNext;
    --m_nFreeBlocks;

#if defined(_DEBUG)
    FillAllocatedBlock(freeBlock);
#endif

    return reinterpret_cast<void *>(freeBlock);
}

void My::Allocator::Free(void *p)
{
    BlockHeader *block = reinterpret_cast<BlockHeader *>(p);

#if defined(_DEBUG)
    FillFreeBlock(block);
#endif

    //freelist是个链表不连续的，相当于头插法插入一个block
    block->pNext = m_pFreeList;
    m_pFreeList = block;
    ++m_nFreeBlocks;
}

void My::Allocator::FreeAll()
{
    PageHeader *pPage = m_pPageList;
    while (pPage)
    {
        PageHeader *_p = pPage;
        pPage = pPage->pNext;
        delete[] reinterpret_cast<uint8_t *>(_p);
    }

    m_pPageList = nullptr;
    m_pFreeList = nullptr;

    m_nPages = 0;
    m_nBlocks = 0;
    m_nFreeBlocks = 0;
}

#if defined(_DEBUG)
void My::Allocate::FillFreePage(PageHeader *pPage)
{
    //page header
    pPage->pNext = nullptr;

    //blocks
    BlockHeader *pBlock = pPage->Blocks();
    for (uint32_t i = 0; i < m_nBlockPerPage; ++i)
    {
        FillFreeBlock(pBlock);
        pBlock = NextBlock(pBlock);
    }
}

void My::Allocate::FillFreeBlock(BlockHeader *pBlock)
{
    //block header+data
    //memset全部设置为PATTERN_FREE
    std::memset(pBlock, PATTERN_FREE, m_szBlockSize - m_szAlignmentSize);

    //alignment
    std::memset(reinterpret_cast<uint8_t *>(pBlock) + m_szBlockSize - m_szAlignmentSize,
                PATTERN_ALIGN, m_szAlignmentSize);
}

void My::Allocate::FillAllocatedBlock(BlockHeader *pBlock)
{
    //block header+data
    //memset全部设置为PATTERN_ALLOC
    std::memset(pBlock, PATTERN_ALLOC, m_szBlockSize - m_szAlignmentSize);

    //alignment
    std::memset(reinterpret_cast<uint8_t *>(pBlock) + m_szBlockSize - m_szAlignmentSize,
                PATTERN_ALIGN, m_szAlignmentSize);
}
#endif

My::BlockHeader *My::Allocator::NextBlock(BlockHeader *pBlock)
{
    return reinterpret_cast<BlockHeader *>(reinterpret_cast<uint8_t *>(pBlock) + m_szBlockSize);
}




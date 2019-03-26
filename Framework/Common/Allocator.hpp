//
// Created by creator on 19-3-25.
//

//对内存分配方面，由于操作系统和用户程序不是在一个级别上的，用户程序跑在操作系统上的一个虚拟环境
//用户程序看到的内存地址也不是真是的物理地址(逻辑地址)，malloc/new这种系统调用看似同步的，其实是异步的
//同步是程序一直询问系统给予回复，不然一直询问(处于阻塞状态)，而异步是给系统一个请求，而不再询问(但由于会给线程一个block，看似同步而已)
//那么而这些系统调用过程中参数传递都是拷贝(因为处于不同地址空间，所以没法传指针)，所以提高CPU效率重要手段就是减少系统调用
//所以在初始化时就一次性申领所有资源，然后自己内部进行分配管理。这里我们使用基于块链(block chain)的内存管理方法

//为了跨平台所以使用size_t和uint32_t
#include <cstddef>
#include <cstdint>

//Pages=each memory chunks
//Block=one whole piece of data, one page has much blocks
//FreeList keeps track of all blocks that can be allocated
//data_size=the size of memory block the client wishes to allocate
//alignment_size=the number of extra bytes added to a block

namespace My
{

    //单项链表
    struct BlockHeader
    {
        //union-ed with data
        BlockHeader *pNext;
    };

    struct PageHeader
    {
        PageHeader *pNext;

        BlockHeader *Blocks()
        {
            //reinterpret_cast一般是指针/引用强制转换
            return reinterpret_cast<BlockHeader *>(this + 1);
        }
    };

    class Allocator
    {
    public:
        //debug patterns
        static const uint8_t PATTERN_ALIGN = 0xFC;
        static const uint8_t PATTERN_ALLOC = 0xFC;
        static const uint8_t PATTERN_FREE = 0xFE;

        Allocator(size_t data_size, size_t page_size, size_t alignment);

        ~Allocator();

        //resets the allocator to a new configuration
        void Reset(size_t data_size, size_t page_size, size_t alignment);

        //alloc and free blocks
        void *Allocate();

        void Free(void *p);

        void FreeAll();

    private:
#if defined(_DEBUG)
        //fill a free page with debug pattern
        void FillFreePage(PageHeader *pPage);

        //fill a free block with debug pattern
        void FillFreeBlock(BlockHeader *pBlock);

        //fill a allocated block with debug pattern
        void FillAllocatedBlock(BlockHearder *pBlock);
#endif

        //get the next block
        BlockHeader *NextBlock(BlockHeader *pBlock);

        //the page list, m_ represents member
        PageHeader *m_pPageList;

        //the free block list
        BlockHeader *m_pFreeList;

        size_t m_szDataSize;
        size_t m_szPageSize;
        size_t m_szAlignmentSize;   //内存对齐功能
        size_t m_szBlockSize;
        uint32_t m_nBlockPerPage;

        //statistics
        uint32_t m_nPages;
        uint32_t m_nBlocks;
        uint32_t m_nFreeBlocks;

        //disable copy & assignment
        //定义成私有并不定义，或者显式让构造函数=delete
        Allocator(const Allocator &clone);

        Allocator &operator=(const Allocator &rhs);
    };
}
/*
* ����: ���ʽ���ڴ��
*       �ṩ��Ч��,�ڴ��С�̶���,��̬�ڴ��������ͷ�,
*       ��һ�����й����ڴ�ڵ�,����Ƶ�����,�������Ч��.
* Ӧ�ó���: ÿ��ֻ������һ���ڴ�, һ����������ڵ�Ķ�̬����������
* 
* ����: ������<kerndev@foxmail.com>
*/
#include "kernel.h"
#include "mpool.h"

struct mpool
{
    uint32_t* pmem_addr;
    uint32_t  next_free;
    uint32_t  next_alloc;
    uint32_t  free_block;
    uint32_t  total_block;
    kmutex_t  mutex;
};


mpool_t mpool_create(uint32_t block_size, uint32_t block_count)
{
    uint32_t i;
    uint32_t mem_addr;
    struct mpool* mpool;
    
    mpool = kmem_malloc((block_size+4)*block_count+sizeof(struct mpool));
    if(mpool == NULL)
    {
        return NULL;
    }
    mpool->mutex= kmutex_create();
    if(mpool->mutex == NULL)
    {
        kmem_free(mpool);
        return NULL;
    }
    
    mpool->pmem_addr = (uint32_t*)(mpool+1);
    mem_addr =  (uint32_t)(mpool->pmem_addr+block_count);
    mpool->next_free   = 0;
    mpool->next_alloc  = 0;
    mpool->free_block  = block_count;
    mpool->total_block = block_count;
    for(i=0;i<block_count;i++)
    {
        mpool_free(mpool,(void*)mem_addr);
        mem_addr += block_size;
    }
    return mpool;
}

void mpool_destory(mpool_t mp)
{
    struct mpool* mpool;
    
    mpool = (struct mpool*)mp;
    kmutex_destroy(mpool->mutex);
    kmem_free(mpool);
}

void* mpool_alloc(mpool_t mp)
{
    uint32_t addr;
    struct mpool* mpool;
    
    mpool = (struct mpool*)mp;
    if(mpool->free_block > 0)
    {
        kmutex_lock(mpool->mutex);
        addr = mpool->pmem_addr[mpool->next_alloc];
        if(++mpool->next_alloc >= mpool->total_block)
        {
            mpool->next_alloc = 0;
        }
        mpool->free_block--;
        kmutex_unlock(mpool->mutex);
        return (void*)addr;
    }
    return NULL;
}

void mpool_free(mpool_t mp, void* mem)
{
    uint32_t addr;
    struct mpool* mpool;
    
    mpool = (struct mpool*)mp;
    addr = (uint32_t)mem;
    kmutex_lock(mpool->mutex);
    mpool->pmem_addr[mpool->next_free] = addr;
    if(++mpool->next_free >= mpool->total_block)
    {
        mpool->next_free = 0;
    }
    mpool->free_block++;
    kmutex_unlock(mpool->mutex);
}


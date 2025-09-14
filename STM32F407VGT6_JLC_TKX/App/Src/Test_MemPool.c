#include "Test.h"

void Test_MemPool(void)
{
    EK_rSerialPrintf(TestQueue, "=== 内存池测试 ===\r\n");

    // 获取内存池统计信息
    PoolStats_t stats;
    EK_vMemPool_GetStats(&stats);

    EK_rSerialPrintf(TestQueue, "总容量: %u字节\r\n", (uint32_t)stats.total_size);
    EK_rSerialPrintf(TestQueue, "可用字节: %u字节\r\n", (uint32_t)stats.free_bytes);
    EK_rSerialPrintf(TestQueue, "历史最少可用: %u字节\r\n", (uint32_t)stats.min_free_bytes);
    EK_rSerialPrintf(TestQueue, "分配次数: %u\r\n", (uint32_t)stats.alloc_count);
    EK_rSerialPrintf(TestQueue, "释放次数: %u\r\n", (uint32_t)stats.free_count);

    // 测试内存分配和释放
    void *ptr1 = EK_pMemPool_Malloc(128);
    void *ptr2 = EK_pMemPool_Malloc(256);
    void *ptr3 = EK_pMemPool_Malloc(64);

    if (ptr1 && ptr2 && ptr3)
    {
        EK_rSerialPrintf(TestQueue, "内存分配测试 ✅ - 已分配3个内存块: %p, %p, %p\r\n", ptr1, ptr2, ptr3);
        test_success_count++;
    }
    else
    {
        EK_rSerialPrintf(TestQueue, "内存分配测试 ❌ - 分配失败\r\n");
        test_failure_count++;
    }

    EK_rSerialPrintf(TestQueue, "分配后剩余字节: %u\r\n", EK_sMemPool_GetFreeSize());

    // 检查内存完整性
    bool integrity = EK_bMemPool_CheckIntegrity();
    if (integrity)
    {
        EK_rSerialPrintf(TestQueue, "内存完整性检查 ✅ - 正常\r\n");
        test_success_count++;
    }
    else
    {
        EK_rSerialPrintf(TestQueue, "内存完整性检查 ❌ - 异常\r\n");
        test_failure_count++;
    }

    // 释放内存
    bool free_success = true;
    if (ptr1) free_success &= EK_bMemPool_Free(ptr1);
    if (ptr2) free_success &= EK_bMemPool_Free(ptr2);
    if (ptr3) free_success &= EK_bMemPool_Free(ptr3);

    if (free_success)
    {
        EK_rSerialPrintf(TestQueue, "内存释放测试 ✅ - 释放成功\r\n");
        test_success_count++;
    }
    else
    {
        EK_rSerialPrintf(TestQueue, "内存释放测试 ❌ - 释放失败\r\n");
        test_failure_count++;
    }

    EK_rSerialPrintf(TestQueue, "释放后剩余字节: %u\r\n", EK_sMemPool_GetFreeSize());
    EK_rSerialPrintf(TestQueue, "内存池测试完成\r\n\r\n");
}
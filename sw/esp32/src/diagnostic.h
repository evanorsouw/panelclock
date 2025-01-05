#ifndef _DIAGNOSTIC_H_
#define _DIAGNOSTIC_H_

#include <stdio.h>
#include <esp_heap_caps.h>

class Diagnostic
{
public:
    static void printmeminfo()
    {
        printf("freemem: DRAM='%d(%d)'",heap_caps_get_free_size(MALLOC_CAP_8BIT), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
        printf(", DRAM-internal='%d(%d)'",heap_caps_get_free_size(MALLOC_CAP_8BIT|MALLOC_CAP_INTERNAL), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT|MALLOC_CAP_INTERNAL));
        printf(", IRAM='%d(%d)' ", heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_32BIT), heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_32BIT));
        printf(", DMA='%d(%d)\n", heap_caps_get_free_size(MALLOC_CAP_DMA), heap_caps_get_largest_free_block(MALLOC_CAP_DMA));
    }
};

#endif
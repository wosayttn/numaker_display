#include <stdint.h>
#include "nu_misc.h"

#if defined(__FREERTOS__)
    #include "FreeRTOS.h"
#else
    #include <stdlib.h>
#endif

void *nvt_malloc_align(uint32_t size, uint32_t align)
{
    void *ptr;
    uint32_t align_size;

    align = NVT_ALIGN(align, sizeof(void *));

    align_size = NVT_ALIGN(size, sizeof(void *)) + align;
#if defined(__FREERTOS__)
    if ((ptr = pvPortMalloc(align_size)) != NULL)
#else
    if ((ptr = malloc(align_size)) != NULL)
#endif
    {
        void *align_ptr;

        if (((uintptr_t)ptr & (align - 1)) == 0)
        {
            align_ptr = (void *)((uintptr_t)ptr + align);
        }
        else
        {
            align_ptr = (void *)NVT_ALIGN((uintptr_t)ptr, align);
        }

        *((uint32_t *)((uintptr_t)align_ptr - sizeof(void *))) = (uintptr_t)ptr;

        ptr = align_ptr;
    }


    return ptr;
}

void nvt_free_align(void *ptr)
{
    if (ptr == NULL) return;

#if defined(__FREERTOS__)
    vPortFree((void *) * ((uintptr_t *)((uintptr_t)ptr - sizeof(void *))));
#else
    free((void *) * ((uintptr_t *)((uintptr_t)ptr - sizeof(void *))));
#endif

}

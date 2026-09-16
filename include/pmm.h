#ifndef PMM_H
#define PMM_H

#include <stdint.h>

// PMM මූලික ශ්‍රිත
void pmm_init(uint32_t mem_size);
void* pmm_alloc_frame(void);
void pmm_free_frame(void *paddr);
void pmm_get_memory_stats(uint32_t *total, uint32_t *used, uint32_t *free);

#endif

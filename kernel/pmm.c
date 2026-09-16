#include <pmm.h>

#define BLOCK_SIZE 4096
#define MAX_FRAMES (32 * 1024 * 1024 / BLOCK_SIZE)

static uint8_t bitmap[MAX_FRAMES / 8];
static uint32_t total_memory = 0;
static uint32_t used_frames = 0;

static void set_frame(uint32_t frame_idx) {
    bitmap[frame_idx / 8] |= (1 << (frame_idx % 8));
}

static void clear_frame(uint32_t frame_idx) {
    bitmap[frame_idx / 8] &= ~(1 << (frame_idx % 8));
}

static int test_frame(uint32_t frame_idx) {
    return bitmap[frame_idx / 8] & (1 << (frame_idx % 8));
}

void pmm_init(uint32_t mem_size) {
    total_memory = mem_size;
    used_frames = 0; // අනිවාර්යයෙන්ම 0 ලෙස ආරම්භ කරන්න

    // මුළු bitmap එකම 0 (free) ලෙස සකසන්න
    for (uint32_t i = 0; i < (MAX_FRAMES / 8); i++) {
        bitmap[i] = 0;
    }

    // මුල් 1MB (පළමු frames 256) kernel එක සහ BIOS සඳහා reserved කරන්න
    uint32_t reserved_frames = (1024 * 1024) / BLOCK_SIZE;
    for (uint32_t i = 0; i < reserved_frames; i++) {
        set_frame(i);
        used_frames++;
    }
}

void* pmm_alloc_frame(void) {
    for (uint32_t i = 0; i < MAX_FRAMES; i++) {
        if (!test_frame(i)) {
            set_frame(i);
            used_frames++;
            return (void*)(i * BLOCK_SIZE);
        }
    }
    return 0; // මතකය මදි නම් (Out of memory)
}

void pmm_free_frame(void *paddr) {
    uint32_t frame_idx = ((uint32_t)paddr) / BLOCK_SIZE;
    if (test_frame(frame_idx)) {
        clear_frame(frame_idx);
        used_frames--;
    }
}

void pmm_get_memory_stats(uint32_t *total, uint32_t *used, uint32_t *free) {
    *total = total_memory;
    *used = used_frames * BLOCK_SIZE;
    *free = total_memory - *used;
}

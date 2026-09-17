#include "fs.h"

// L12 §1 — RAM disk: fixed-size byte array simulating a block device
static uint8_t ramdisk[BLOCK_SIZE * TOTAL_BLOCKS];

void ramdisk_read(uint32_t block_num, void *buf) {
    uint8_t *src = ramdisk + (block_num * BLOCK_SIZE);
    uint8_t *dst = (uint8_t *)buf;
    for (uint32_t i = 0; i < BLOCK_SIZE; i++) dst[i] = src[i];
}

void ramdisk_write(uint32_t block_num, const void *buf) {
    uint8_t *dst = ramdisk + (block_num * BLOCK_SIZE);
    const uint8_t *src = (const uint8_t *)buf;
    for (uint32_t i = 0; i < BLOCK_SIZE; i++) dst[i] = src[i];
}

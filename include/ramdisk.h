#ifndef RAMDISK_H
#define RAMDISK_H
#include <stdint.h>

void ramdisk_read(uint32_t block_num, void *buf);
void ramdisk_write(uint32_t block_num, const void *buf);

#endif

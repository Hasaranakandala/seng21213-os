#ifndef FS_H
#define FS_H
#include <stdint.h>

#define BLOCK_SIZE      4096
#define TOTAL_BLOCKS     32          // 1MB / 4KB
#define MAX_FILES         32
#define MAX_NAME_LEN      28
#define DIRECT_BLOCKS      8          // 8 * 4KB = 32KB max file size
#define FS_MAGIC    0x53454E47        // "SENG"

typedef struct {
    uint32_t magic;
    uint32_t total_blocks;
    uint32_t total_inodes;
} superblock_t;

typedef struct {
    uint32_t size;                    // file size in bytes
    uint32_t used;                    // 1 = in use, 0 = free
    uint32_t direct[DIRECT_BLOCKS];   // block indices
} inode_t;

typedef struct {
    char name[MAX_NAME_LEN];
    int32_t inode;                    // -1 = empty slot
} dirent_t;

void fs_init(void);
int  fs_create(const char *name);
int  fs_write(const char *name, const char *data, uint32_t len);
int  fs_read(const char *name, char *buf, uint32_t maxlen);
int  fs_unlink(const char *name);
void fs_list(void);   // used by 'ls' command

#endif

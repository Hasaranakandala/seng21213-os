#include "fs.h"
#include "ramdisk.h"
#include "vga.h"

// L12 §2/§3 — simple flat filesystem with fixed-size inode table
static inode_t   inodes[MAX_FILES];
static dirent_t  directory[MAX_FILES];
static uint8_t   block_used[TOTAL_BLOCKS];   // simple in-memory block bitmap
static superblock_t sb;
extern void serial_print(const char *s);
// helper: minimal strcmp/strlen/strcpy (avoid libc)
static int fs_strcmp(const char *a, const char *b) {
    while (*a && (*a == *b)) { a++; b++; }
    return *(unsigned char*)a - *(unsigned char*)b;

}

static int fs_strlen(const char *s) __attribute__((unused));

static int fs_strlen(const char *s) {
    int n = 0; while (s[n]) n++; return n;
}
static void fs_strcpy(char *dst, const char *src, int maxlen) {
    int i = 0;
    while (src[i] && i < maxlen - 1) { dst[i] = src[i]; i++; }
    dst[i] = '\0';
}

void fs_init(void) {
    sb.magic = FS_MAGIC;
    sb.total_blocks = TOTAL_BLOCKS;
    sb.total_inodes = MAX_FILES;

    for (int i = 0; i < MAX_FILES; i++) {
        inodes[i].used = 0;
        inodes[i].size = 0;
        for (int j = 0; j < DIRECT_BLOCKS; j++) inodes[i].direct[j] = 0;
        directory[i].inode = -1;
        directory[i].name[0] = '\0';
    }
    for (int i = 0; i < TOTAL_BLOCKS; i++) block_used[i] = 0;

    block_used[0] = 1;  // superblock
    block_used[1] = 1;  // directory block (conceptually reserved)
}

static int find_free_inode(void) {
    for (int i = 0; i < MAX_FILES; i++)
        if (!inodes[i].used) return i;
    return -1;
}

static int find_free_block(void) {
    for (int i = 2; i < TOTAL_BLOCKS; i++)   // 0,1 reserved
        if (!block_used[i]) return i;
    return -1;
}

static int find_dirent(const char *name) {
    for (int i = 0; i < MAX_FILES; i++)
        if (directory[i].inode != -1 && fs_strcmp(directory[i].name, name) == 0)
            return i;
    return -1;
}

int fs_create(const char *name) {
    if (find_dirent(name) != -1) return -1;   // already exists

    int inode_idx = find_free_inode();
    if (inode_idx == -1) return -1;            // no free inodes

    int dir_idx = -1;
    for (int i = 0; i < MAX_FILES; i++)
        if (directory[i].inode == -1) { dir_idx = i; break; }
    if (dir_idx == -1) return -1;               // directory full

    inodes[inode_idx].used = 1;
    inodes[inode_idx].size = 0;

    fs_strcpy(directory[dir_idx].name, name, MAX_NAME_LEN);
    directory[dir_idx].inode = inode_idx;

    return 0;
}

int fs_write(const char *name, const char *data, uint32_t len) {
    int dir_idx = find_dirent(name);
    if (dir_idx == -1) return -1;               // not found

    inode_t *inode = &inodes[directory[dir_idx].inode];

    uint32_t offset = inode->size;               // append mode
    uint32_t written = 0;
    uint8_t block_buf[BLOCK_SIZE];

    while (written < len) {
        uint32_t block_idx  = offset / BLOCK_SIZE;
        uint32_t block_off  = offset % BLOCK_SIZE;

        if (block_idx >= DIRECT_BLOCKS) break;   // exceeds max file size

        if (inode->direct[block_idx] == 0) {
            int new_block = find_free_block();
            if (new_block == -1) break;          // disk full
            block_used[new_block] = 1;
            inode->direct[block_idx] = new_block;
            for (int i = 0; i < BLOCK_SIZE; i++) block_buf[i] = 0;
        } else {
            ramdisk_read(inode->direct[block_idx], block_buf);
        }

        uint32_t space = BLOCK_SIZE - block_off;
        uint32_t to_copy = (len - written < space) ? (len - written) : space;

        for (uint32_t i = 0; i < to_copy; i++)
            block_buf[block_off + i] = data[written + i];

        ramdisk_write(inode->direct[block_idx], block_buf);

        written += to_copy;
        offset  += to_copy;
    }

    inode->size = offset;
    return (int)written;
}

int fs_read(const char *name, char *buf, uint32_t maxlen) {
    int dir_idx = find_dirent(name);
    if (dir_idx == -1) return -1;

    inode_t *inode = &inodes[directory[dir_idx].inode];
    uint32_t to_read = (inode->size < maxlen) ? inode->size : maxlen;
    uint32_t read_so_far = 0;
    uint8_t block_buf[BLOCK_SIZE];

    while (read_so_far < to_read) {
        uint32_t block_idx = read_so_far / BLOCK_SIZE;
        uint32_t block_off = read_so_far % BLOCK_SIZE;

        if (inode->direct[block_idx] == 0) break;
        ramdisk_read(inode->direct[block_idx], block_buf);

        uint32_t space = BLOCK_SIZE - block_off;
        uint32_t remaining = to_read - read_so_far;
        uint32_t chunk = (remaining < space) ? remaining : space;

        for (uint32_t i = 0; i < chunk; i++)
            buf[read_so_far + i] = block_buf[block_off + i];

        read_so_far += chunk;
    }

    buf[read_so_far] = '\0';
    return (int)read_so_far;
}

int fs_unlink(const char *name) {
    int dir_idx = find_dirent(name);
    if (dir_idx == -1) return -1;

    inode_t *inode = &inodes[directory[dir_idx].inode];

    // free blocks
    for (int i = 0; i < DIRECT_BLOCKS; i++) {
        if (inode->direct[i] != 0) {
            block_used[inode->direct[i]] = 0;
            inode->direct[i] = 0;
        }
    }
    inode->used = 0;
    inode->size = 0;

    directory[dir_idx].inode = -1;
    directory[dir_idx].name[0] = '\0';

    return 0;
}

void fs_list(void) {
    extern void serial_print(const char *s);
    int found = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (directory[i].inode != -1) {
            inode_t *inode = &inodes[directory[i].inode];
            vga_puts(directory[i].name);
            vga_puts("  (");

            // manual integer-to-string (avoid vga_printf %s+%d combo bug)
            char numbuf[12];
            int n = inode->size;
            int pos = 0;
            if (n == 0) { numbuf[pos++] = '0'; }
            while (n > 0) { numbuf[pos++] = '0' + (n % 10); n /= 10; }
            while (pos > 0) vga_putchar(numbuf[--pos]);

            vga_puts(" bytes)\n");
            found = 1;
        }
    }
    if (!found) vga_puts("(no files)\n");
}

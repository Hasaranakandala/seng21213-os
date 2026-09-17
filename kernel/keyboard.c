/* =============================================================================
 * SENG21213-OS :: PS/2 Keyboard Driver Implementation
 * File   : kernel/keyboard.c
 * ============================================================================*/
#include "keyboard.h"
#include "vga.h"
#include "../include/types.h"

/* I/O ports */
#define KB_DATA_PORT   0x60    /* Read scan code / write command */
#define KB_STATUS_PORT 0x64    /* Read status / write command */
#define KB_STATUS_OBF  0x01    /* Output Buffer Full bit */

/* Inline port I/O */
static inline uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ __volatile__("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

/* ---------------------------------------------------------------------------
 * Scancode Set 1 → ASCII translation table (unshifted)
 * Index = scancode. 0 = non-printable / not mapped.
 * --------------------------------------------------------------------------*/
static const char sc_ascii[128] = {
    [0x01] = 27,  [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4',
    [0x06] = '5', [0x07] = '6', [0x08] = '7', [0x09] = '8', [0x0A] = '9',
    [0x0B] = '0', [0x0C] = '-', [0x0D] = '=', [0x0E] = '\b',
    [0x0F] = '\t',
    [0x10] = 'q', [0x11] = 'w', [0x12] = 'e', [0x13] = 'r', [0x14] = 't',
    [0x15] = 'y', [0x16] = 'u', [0x17] = 'i', [0x18] = 'o', [0x19] = 'p',
    [0x1A] = '[', [0x1B] = ']', [0x1C] = '\n',
    [0x1E] = 'a', [0x1F] = 's', [0x20] = 'd', [0x21] = 'f', [0x22] = 'g',
    [0x23] = 'h', [0x24] = 'j', [0x25] = 'k', [0x26] = 'l', [0x27] = ';',
    [0x28] = '\'', [0x29] = '`',
    [0x2B] = '\\', [0x2C] = 'z', [0x2D] = 'x', [0x2E] = 'c', [0x2F] = 'v',
    [0x30] = 'b', [0x31] = 'n', [0x32] = 'm', [0x33] = ',', [0x34] = '.',
    [0x35] = '/',
    [0x37] = '*', [0x39] = ' ',
    [0x47] = '7', [0x48] = '8', [0x49] = '9', [0x4A] = '-',
    [0x4B] = '4', [0x4C] = '5', [0x4D] = '6', [0x4E] = '+',
    [0x4F] = '1', [0x50] = '2', [0x51] = '3', [0x52] = '0', [0x53] = '.',
};

static const char sc_ascii_shift[128] = {
    [0x01] = 27,  [0x02] = '!', [0x03] = '@', [0x04] = '#', [0x05] = '$',
    [0x06] = '%', [0x07] = '^', [0x08] = '&', [0x09] = '*', [0x0A] = '(',
    [0x0B] = ')', [0x0C] = '_', [0x0D] = '+', [0x0E] = '\b',
    [0x0F] = '\t',
    [0x10] = 'Q', [0x11] = 'W', [0x12] = 'E', [0x13] = 'R', [0x14] = 'T',
    [0x15] = 'Y', [0x16] = 'U', [0x17] = 'I', [0x18] = 'O', [0x19] = 'P',
    [0x1A] = '{', [0x1B] = '}', [0x1C] = '\n',
    [0x1E] = 'A', [0x1F] = 'S', [0x20] = 'D', [0x21] = 'F', [0x22] = 'G',
    [0x23] = 'H', [0x24] = 'J', [0x25] = 'K', [0x26] = 'L', [0x27] = ':',
    [0x28] = '"', [0x29] = '~',
    [0x2B] = '|', [0x2C] = 'Z', [0x2D] = 'X', [0x2E] = 'C', [0x2F] = 'V',
    [0x30] = 'B', [0x31] = 'N', [0x32] = 'M', [0x33] = '<', [0x34] = '>',
    [0x35] = '?',
    [0x37] = '*', [0x39] = ' ',
};



static bool shift_held = false;

void kb_init(void) {
    /* Flush any stale data in the keyboard buffer */
    while (inb(KB_STATUS_PORT) & KB_STATUS_OBF) {
        inb(KB_DATA_PORT);
    }
}

char kb_getchar(void) {
    uint8_t sc;
    while (true) {
        /* Wait until output buffer is full (key available) */
        while (!(inb(KB_STATUS_PORT) & KB_STATUS_OBF));
        sc = inb(KB_DATA_PORT);

        if (sc & 0x80) {
            /* Key release: bit 7 set, clear modifier state */
            uint8_t release = sc & 0x7F;
            if (release == 0x2A || release == 0x36) shift_held = false;
            continue;
        }

        /* Key press */
        if (sc == 0x2A || sc == 0x36) { shift_held = true; continue; }

        /* Caps lock / ctrl / alt – ignored in Stage 0 */

        char c = shift_held ? sc_ascii_shift[sc] : sc_ascii[sc];
        if (c) return c;
    }
}

int kb_readline(char *buf, int len) {
    int i = 0;
    while (i < len - 1) {
        char c = kb_getchar();
        if (c == '\n' || c == '\r') {
            vga_putchar('\n');
            break;
        }
        if (c == '\b') {
            if (i > 0) { i--; vga_putchar('\b'); }
            continue;
        }
        buf[i++] = c;
        vga_putchar(c);
    }
    buf[i] = '\0';
    return i;
}

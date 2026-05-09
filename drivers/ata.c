/*
 * ata.c — ATA PIO driver
 *
 * (c) 2026, Redstone2888
 * Read LICENSE.txt for details
 *
 * Polling-based (no interrupts). Uses LBA28.
 * Works on real x86 hardware and QEMU -drive format=raw.
 */

#include <ata.h>
#include <stdint.h>

// Port I/O helpers
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}
static inline uint16_t inw(uint16_t port) {
    uint16_t val;
    __asm__ volatile ("inw %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}
static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

// Wait until BSY clears. Returns 0 on success, -1 on timeout.
static int ata_wait_bsy() {
    for (int i = 0; i < 100000; i++) {
        if (!(inb(ATA_STATUS) & ATA_SR_BSY))
            return 0;
    }
    return -1; // timeout
}

// Wait until DRQ sets (drive ready to transfer data).
// Returns 0 on success, -1 on error/timeout.
static int ata_wait_drq() {
    for (int i = 0; i < 100000; i++) {
        uint8_t status = inb(ATA_STATUS);
        if (status & ATA_SR_ERR) return -1;
        if (status & ATA_SR_DRQ) return 0;
    }
    return -1;
}

// Send 400ns delay by reading alt status 4 times (standard practice)
static void ata_delay() {
    inb(ATA_ALT_STATUS);
    inb(ATA_ALT_STATUS);
    inb(ATA_ALT_STATUS);
    inb(ATA_ALT_STATUS);
}

// Detect and initialise primary master drive
void ata_init() {
    // Software reset
    outb(ATA_ALT_STATUS, 0x04);
    outb(ATA_ALT_STATUS, 0x00);
    ata_wait_bsy();
}

// Read one 512-byte sector at LBA `lba` into `buf`.
// Returns 0 on success, -1 on error.
int ata_read_sector(uint32_t lba, uint8_t* buf) {
    if (ata_wait_bsy() != 0) return -1;

    // Select master drive, LBA mode, top 4 bits of LBA
    outb(ATA_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_SECT_COUNT, 1);
    outb(ATA_LBA_LO,  (uint8_t)(lba));
    outb(ATA_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_LBA_HI,  (uint8_t)(lba >> 16));
    outb(ATA_COMMAND, ATA_CMD_READ);

    ata_delay();

    if (ata_wait_drq() != 0) return -1;

    // Read 256 words = 512 bytes
    uint16_t* dst = (uint16_t*)buf;
    for (int i = 0; i < 256; i++)
        dst[i] = inw(ATA_DATA);

    return 0;
}

// Write one 512-byte sector at LBA `lba` from `buf`.
// Returns 0 on success, -1 on error.
int ata_write_sector(uint32_t lba, uint8_t* buf) {
    if (ata_wait_bsy() != 0) return -1;

    outb(ATA_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_SECT_COUNT, 1);
    outb(ATA_LBA_LO,  (uint8_t)(lba));
    outb(ATA_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_LBA_HI,  (uint8_t)(lba >> 16));
    outb(ATA_COMMAND, ATA_CMD_WRITE);

    ata_delay();

    if (ata_wait_drq() != 0) return -1;

    // Write 256 words = 512 bytes
    uint16_t* src = (uint16_t*)buf;
    for (int i = 0; i < 256; i++)
        outw(ATA_DATA, src[i]);

    return ata_flush();
}

// Flush drive write cache — call after write_sector
int ata_flush() {
    outb(ATA_COMMAND, ATA_CMD_FLUSH);
    return ata_wait_bsy();
}

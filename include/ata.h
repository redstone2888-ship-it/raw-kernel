#pragma once
#include <stdint.h>

/*
 * ata.h — ATA PIO driver
 *
 * (c) 2026, Redstone2888
 * Read LICENSE.txt for details
 *
 * Handles raw sector read/write via ATA PIO mode (polling).
 * Works on real hardware and QEMU with -drive file=disk.img,format=raw
 *
 * Primary bus: I/O ports 0x1F0–0x1F7, control 0x3F6
 * We use LBA28 addressing — supports up to 128GB, more than enough.
 */

// ATA primary bus ports
#define ATA_DATA        0x1F0
#define ATA_ERROR       0x1F1
#define ATA_SECT_COUNT  0x1F2
#define ATA_LBA_LO      0x1F3
#define ATA_LBA_MID     0x1F4
#define ATA_LBA_HI      0x1F5
#define ATA_DRIVE_HEAD  0x1F6
#define ATA_STATUS      0x1F7
#define ATA_COMMAND     0x1F7
#define ATA_ALT_STATUS  0x3F6

// ATA status bits
#define ATA_SR_BSY  0x80  // busy
#define ATA_SR_DRQ  0x08  // data request ready
#define ATA_SR_ERR  0x01  // error

// ATA commands
#define ATA_CMD_READ  0x20
#define ATA_CMD_WRITE 0x30
#define ATA_CMD_FLUSH 0xE7

void ata_init();
int  ata_read_sector(uint32_t lba, uint8_t* buf);
int  ata_write_sector(uint32_t lba, uint8_t* buf);
int  ata_flush();

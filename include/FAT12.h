// fat12.h
#ifndef FAT12_H
#define FAT12_H

#include <stdint.h>

#define MAX_FILENAME 11
#define SECTOR_SIZE 512
#define MAX_FILE_SIZE 32768
#define DISK_SIZE 1474560

typedef struct {
    uint8_t jmp[3];
    uint8_t oem[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t num_fats;
    uint16_t root_entry_count;
    uint16_t total_sectors_short; // если 0, смотри total_sectors_long
    uint8_t media_descriptor;
    uint16_t fat_size_sectors;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_long;
} __attribute__((packed)) boot_sector_t;

typedef struct {
    char name[8];
    char ext[3];
    uint8_t attr;
    uint8_t reserved;
    uint8_t ctime_ms;
    uint16_t ctime;
    uint16_t cdate;
    uint16_t adate;
    uint16_t first_cluster_hi;
    uint16_t mtime;
    uint16_t mdate;
    uint16_t first_cluster_lo;
    uint32_t size;
} __attribute__((packed)) dir_entry_t;

extern uint8_t memory_buffer[MAX_FILE_SIZE];
extern uint8_t disk_image[DISK_SIZE];
extern dir_entry_t root_dir[224];

void read_fat(void);
void read_root_dir(void);
dir_entry_t* find_file(const char* name);
void read_file(dir_entry_t* entry, uint8_t* buf);
void read_sector(int sector_number, uint8_t* buffer);

#endif
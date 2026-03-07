#pragma once
#include <stdint.h>

#define SECTOR_SIZE 512
#define MAX_FILE_SIZE 65536

typedef struct {
    char name[8];
    char ext[3];
    uint8_t attr;
    uint8_t reserved[10];
    uint16_t first_cluster_lo;
    uint16_t first_cluster_hi;
    uint32_t size;
} dir_entry_t;

extern dir_entry_t root_dir[224];
extern uint8_t* disk_image;

void read_fat();
void read_root_dir();
dir_entry_t* find_file(const char* name, const char* ext);
void read_file(dir_entry_t* entry, uint8_t* buf, int buf_size);
void read_sector(int sector_number, uint8_t* buffer);
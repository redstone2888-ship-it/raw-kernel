/*
 * FAT12.c — Simple FAT12 filesystem driver
 *
 * (c) 2026, Redstone2888
 * Read LICENSE.txt for details
 *
 * Handles reading files from a 1.44MB floppy disk image:
 * - Reads the FAT table
 * - Reads the root directory
 * - Finds files by name/extension
 * - Reads file clusters into memory
 *
 * DO NOT DELETE this file — the kernel will return a NO_MODULES_FOUND error. 
 * However, you can remove the lines in kernel.c that are responsible for this.
 */

#include <FAT12.h>

// Storage structures
uint8_t fat_table[SECTOR_SIZE*9];
dir_entry_t root_dir[224];
uint8_t* disk_image;
uint8_t memory_buffer[MAX_FILE_SIZE];

// Read FAT table from disk
void read_fat() {
    for (int i = 0; i < 9; i++) {
        read_sector(1 + i, fat_table + i*SECTOR_SIZE);
    }
}

// Read root directory from disk
void read_root_dir() {
    int root_start = 1 + 9*2;
    for (int i = 0; i < 14; i++) {
        read_sector(root_start + i, ((uint8_t*)root_dir) + i*SECTOR_SIZE);
    }
}

// Find a file in the root directory
dir_entry_t* find_file(const char* name, const char* ext) {
    for (int i = 0; i < 224; i++) {
        int match = 1;
        for (int j = 0; j < 8; j++)
            if (name[j] != root_dir[i].name[j]) match = 0;
        for (int j = 0; j < 3; j++)
            if (ext[j] != root_dir[i].ext[j]) match = 0;
        if (match) return &root_dir[i];
    }
    return 0;
}

// Read a file from the disk into a buffer
void read_file(dir_entry_t* entry, uint8_t* buf, int buf_size) {
    uint16_t cluster = entry->first_cluster_lo;
    int offset = 0;
    int first_data_sector = 1 + 9*2 + 14;
    int cluster_size = SECTOR_SIZE;

    while (cluster < 0xFF8 && offset < entry->size && offset + cluster_size <= buf_size) {
        int sector = first_data_sector + (cluster - 2);
        read_sector(sector, buf + offset);
        offset += cluster_size;

        int fat_index = cluster + cluster / 2;
        uint16_t next;
        if (cluster & 1)
            next = (fat_table[fat_index] >> 4) | (fat_table[fat_index + 1] << 4);
        else
            next = fat_table[fat_index] | ((fat_table[fat_index + 1] & 0x0F) << 8);
        cluster = next;
    }
}

// Read a single sector from disk image
void read_sector(int sector_number, uint8_t* buffer) {
    uint32_t offset = sector_number * SECTOR_SIZE;
    for (int i = 0; i < SECTOR_SIZE; i++) {
        buffer[i] = disk_image[offset + i];
    }
}
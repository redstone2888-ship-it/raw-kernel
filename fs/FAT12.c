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
#include <ata.h>

// Storage structures
uint8_t fat_table[SECTOR_SIZE*9];
dir_entry_t root_dir[224];
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

// Read a single sector via ATA PIO
void read_sector(int sector_number, uint8_t* buffer) {
    ata_read_sector((uint32_t)sector_number, buffer);
}

// Write a single sector via ATA PIO
void write_sector(int sector_number, uint8_t* buffer) {
    ata_write_sector((uint32_t)sector_number, buffer);
}

// Flush FAT table (both copies) back to disk
void write_fat() {
    for (int i = 0; i < 9; i++) {
        write_sector(1 + i,     fat_table + i * SECTOR_SIZE); // FAT copy 1
        write_sector(1 + 9 + i, fat_table + i * SECTOR_SIZE); // FAT copy 2
    }
}

// Flush root directory back to disk
void write_root_dir() {
    int root_start = 1 + 9 * 2;
    for (int i = 0; i < 14; i++) {
        write_sector(root_start + i, ((uint8_t*)root_dir) + i * SECTOR_SIZE);
    }
}

// Read a 12-bit FAT entry for a given cluster
uint16_t fat_get_cluster(uint16_t cluster) {
    int fat_index = cluster + cluster / 2;
    if (cluster & 1)
        return (fat_table[fat_index] >> 4) | (fat_table[fat_index + 1] << 4);
    else
        return fat_table[fat_index] | ((fat_table[fat_index + 1] & 0x0F) << 8);
}

// Write a 12-bit FAT entry for a given cluster
void fat_set_cluster(uint16_t cluster, uint16_t value) {
    int fat_index = cluster + cluster / 2;
    if (cluster & 1) {
        fat_table[fat_index]     = (fat_table[fat_index] & 0x0F) | ((value & 0x0F) << 4);
        fat_table[fat_index + 1] = (value >> 4) & 0xFF;
    } else {
        fat_table[fat_index]     = value & 0xFF;
        fat_table[fat_index + 1] = (fat_table[fat_index + 1] & 0xF0) | ((value >> 8) & 0x0F);
    }
}

// Find a free cluster in the FAT (returns 0 if disk is full)
uint16_t fat_alloc_cluster() {
    for (uint16_t c = 2; c < 2848; c++) {
        if (fat_get_cluster(c) == 0x000)
            return c;
    }
    return 0; // disk full
}

// Free an entire cluster chain starting at `start`
void fat_free_chain(uint16_t start) {
    uint16_t cur = start;
    while (cur >= 0x002 && cur < 0xFF8) {
        uint16_t next = fat_get_cluster(cur);
        fat_set_cluster(cur, 0x000);
        cur = next;
    }
}

// Write data to a file (overwrites existing content)
// Returns 0 on success, -1 on error
int write_file(dir_entry_t* entry, uint8_t* buf, uint32_t size) {
    int first_data_sector = 1 + 9 * 2 + 14;

    if (entry->first_cluster_lo >= 2)
        fat_free_chain(entry->first_cluster_lo);

    uint32_t remaining = size;
    uint8_t* src = buf;
    uint16_t first_cluster = 0;
    uint16_t prev_cluster  = 0;
    uint8_t  sector_buf[SECTOR_SIZE];

    while (remaining > 0) {
        uint16_t cluster = fat_alloc_cluster();
        if (cluster == 0) return -1; // disk full

        if (prev_cluster == 0)
            first_cluster = cluster;
        else
            fat_set_cluster(prev_cluster, cluster);

        fat_set_cluster(cluster, 0xFFF);

        for (int i = 0; i < SECTOR_SIZE; i++) sector_buf[i] = 0;
        uint32_t to_copy = remaining < SECTOR_SIZE ? remaining : SECTOR_SIZE;
        for (uint32_t i = 0; i < to_copy; i++) sector_buf[i] = src[i];

        int sector = first_data_sector + (cluster - 2);
        write_sector(sector, sector_buf);

        src       += to_copy;
        remaining -= to_copy;
        prev_cluster = cluster;
    }

    entry->first_cluster_lo = first_cluster;
    entry->first_cluster_hi = 0;
    entry->size = size;

    write_fat();
    write_root_dir();
    return 0;
}

// Create a new file in the root directory
// name: 8 chars space-padded, ext: 3 chars space-padded
// Returns pointer to new dir_entry_t, or 0 if root dir is full
dir_entry_t* create_file(const char* name, const char* ext) {
    for (int i = 0; i < 224; i++) {
        uint8_t first = (uint8_t)root_dir[i].name[0];
        if (first == 0x00 || first == 0xE5) {
            for (int j = 0; j < 8; j++) root_dir[i].name[j] = name[j];
            for (int j = 0; j < 3; j++) root_dir[i].ext[j]  = ext[j];
            root_dir[i].attr             = 0x20; // archive bit
            root_dir[i].first_cluster_lo = 0;
            root_dir[i].first_cluster_hi = 0;
            root_dir[i].size             = 0;
            for (int j = 0; j < 10; j++) root_dir[i].reserved[j] = 0;
            return &root_dir[i];
        }
    }
    return 0; // root dir full
}
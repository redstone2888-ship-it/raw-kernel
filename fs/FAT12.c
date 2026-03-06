#include <FAT12.h>

uint8_t fat_table[SECTOR_SIZE*9]; // если FAT12 9 секторов, под один диск/образ
dir_entry_t root_dir[224];         // максимум записей в root
uint8_t* disk_base;   
uint8_t memory_buffer[MAX_FILE_SIZE];  // единственная дефиниция
uint8_t disk_image[1474560];  // указатель на образ/флешку в памяти
dir_entry_t root_dir[224];


// Читаем FAT
void read_fat() {
    for (int i = 0; i < 9; i++) {
        read_sector(i + 1, fat_table + i*SECTOR_SIZE); // sector 0 = boot
    }
}

// Читаем корневой каталог
void read_root_dir() {
    int root_start = 1 + 9*1; // boot + FAT*1 (если 1 FAT)
    for (int i = 0; i < 224; i++) {
        read_sector(root_start + i/16, ((uint8_t*)root_dir) + i*32);
    }
}

// Ищем файл
dir_entry_t* find_file(const char* name) {
    for (int i = 0; i < 224; i++) {
        int match = 1;
        for (int j = 0; j < 8; j++) {
            if (name[j] != root_dir[i].name[j]) match = 0;
        }
        if (match) return &root_dir[i];
    }
    return 0;
}

// Читаем цепочку кластеров файла
void read_file(dir_entry_t* entry, uint8_t* buf) {
    uint16_t cluster = entry->first_cluster_lo;
    int cluster_size = SECTOR_SIZE * 1; // если sectors_per_cluster = 1
    int offset = 0;

    while (cluster < 0xFF8) { // FAT12 EOF
        int sector = cluster + 31; // 1-й кластер = sector 33 примерно
        read_sector(sector, buf + offset);
        offset += cluster_size;

        // читаем FAT12 entry
        int fat_index = cluster + cluster/2;
        uint16_t next;
        if (cluster & 1) {
            next = (fat_table[fat_index] >> 4) | (fat_table[fat_index+1]<<4);
        } else {
            next = fat_table[fat_index] | ((fat_table[fat_index+1]&0x0F)<<8);
        }
        cluster = next;
    }
}

void read_sector(int sector_number, uint8_t* buffer) {
    uint32_t offset = sector_number * SECTOR_SIZE; // SECTOR_SIZE = 512
    for (int i = 0; i < SECTOR_SIZE; i++) {
        buffer[i] = disk_image[offset + i];
    }
}
/*#include <vga.h>
#include <panic.h>
#include <string.h>

#define MAX_FILES 16 //* max files count

// Simple file structure for our RAM_FS
struct file {
    char name[16];
    char* data;
    unsigned int size;
};

struct file files[MAX_FILES];
int file_count = 0;

// create a file in the RAM_FS
void mkfile(const char* name, char* data, int size) {

    if (file_count >= MAX_FILES) {
        kernel_panic("0x031RFSF", "RAM_FS_IS_FULL");
    }

    int i = file_count;

    int j = 0;
    while (name[j]) {
        files[i].name[j] = name[j];
        j++;
    }

    files[i].name[j] = 0;

    files[i].data = data;
    files[i].size = size;

    file_count++;
}

// find a file by name in the RAM_FS
struct file* fs_find(const char* name) {
    for (int i = 0; i < file_count; i++)
    {
        if (!strcmp(files[i].name, name)) {
            return &files[i];
        }
    }
    return 0;
}

void ls() {
    for (int i = 0; i < file_count; i++) {
        println(files[i].name);
    }
}

void fs_cat(const char* name) {
    struct file* f = fs_find(name);
    if (!f) {
        println("File not found");
        return;
    }

    print_text(f->data);
    putchar('\n');
}*/
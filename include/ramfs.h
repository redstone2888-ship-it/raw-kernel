#pragma once

struct file;
void mkfile(const char* name, char* data, int size);
struct file* fs_find(const char* name);
void ls(void);
void fs_cat(const char* name);
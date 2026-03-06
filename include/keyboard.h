#pragma once

extern char keymap[128];

char getkey(void);
void readline(char* buf, int max);
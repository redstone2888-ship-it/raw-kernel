#pragma once

#define KRAW_CMD_RESTART    1
#define KRAW_CMD_POWER_OFF  2

void kraw_syscall(int cmd);
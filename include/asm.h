// include/asm.h
#pragma once

static inline void cli(void)   { __asm__ volatile("cli"); }
static inline void sti(void)   { __asm__ volatile("sti"); }
static inline void hlt(void)   { __asm__ volatile("hlt"); }
static inline void nop(void)   { __asm__ volatile("nop"); }
static inline void pause(void) { __asm__ volatile("pause"); }
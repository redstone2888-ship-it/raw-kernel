/* ======== ASM FUNCS ======= */

// cli
static inline void cli() {
    __asm__ __volatile__("cli");
}

// hlt
static inline void hlt() {
    __asm__ __volatile__("hlt");
}

// sti
static inline void sti() {
    __asm__ __volatile__("sti");
}

// nop
static inline void nop() {
    __asm__ __volatile__("nop");
}

// pause
static inline void pause() {
    __asm__ __volatile__("pause");
}
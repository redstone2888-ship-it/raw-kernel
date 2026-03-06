#include <unistd.h>
#include <asm.h>
#include <stdint.h>

void sleep(unsigned int seconds) {
    unsigned int loops = seconds * 10000000;
    for (unsigned int i = 0; i < loops; i++) {
        pause();
    }
}

void usleep(unsigned int microseconds) {
    // настройте константу под ваш проц
    volatile uint32_t loops = microseconds * 10; 
    for (uint32_t i = 0; i < loops; i++)
        pause();
}
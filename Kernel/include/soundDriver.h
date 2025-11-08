#ifndef SOUND_DRIVER_H
#define SOUND_DRIVER_H

#include <stdint.h>

void play_sound(uint32_t nFrequence);
void nosound();
// void beep();
void beep(uint32_t nFrequence, uint32_t time);

#endif // SOUND_DRIVER_H
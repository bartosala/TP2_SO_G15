#ifndef TEXTMODULE_H
#define TEXTMODULE_H

#include <videoDriver.h>

void loadFont(char **newFont, int newFontHeight, int newFontWidth);

void putChar(unsigned char c, uint32_t color);

void lineFeed(int fontHeight);

void printStr(char * s, uint32_t color);

void printInt(uint64_t value, uint32_t color);

void clearText(uint32_t color);

uint64_t fontSizeUp(uint64_t increase);

uint64_t fontSizeDown(uint64_t decrease);

#endif // TEXTMODULE_H
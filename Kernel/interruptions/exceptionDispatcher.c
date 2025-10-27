#include <textModule.h>
#include "../include/keyboardDriver.h"
#include <stdint.h>
#include "../include/interrupts.h"
#include "../include/lib.h"

#define RED 0x00FF0000
#define CANT_REGS 19

extern uint64_t regs[CANT_REGS];

extern void init();

void printHex(uint64_t value, uint32_t color) {
    char hexStr[17];
    hexStr[16] = '\0';
    for (int i = 15; i >= 0; i--) {
        uint8_t nibble = value & 0xF;
        if (nibble < 10) {
            hexStr[i] = '0' + nibble;
        } else {
            hexStr[i] = 'A' + (nibble - 10);
        }
        value >>= 4;
    }
    printStr(hexStr, color);
}

void exception(char * name) {
    clearText(0);
    printStr(name,RED);
    printStr(" exception",RED);
    printStr("\n",RED);

    printStr("Estado de los registros: \n",RED);
    char * registersNames[CANT_REGS] = {"RAX: ", "RBX: ", "RCX: ", "RDX: ", "RSI: ", "RDI: ",
                                            "RBP: ", "RSP: ", "R8: ", "R9: ", "R10: ", "R11: ",
                                            "R12: ", "R13: ", "R14: ", "R15: ", "RFLAGS: ", "RIP: ", "CS: "};
    uint64_t registersRead[CANT_REGS];
    memcpy(registersRead, regs, sizeof(uint64_t) * CANT_REGS);
    for(int i = 0; i < CANT_REGS; i++){
        printStr(registersNames[i],RED);
        printStr("0x",RED);
        printHex(registersRead[i],RED);
        printStr("\n",RED);
    }
    printStr("Presiona cualquier tecla para volver.\n",RED);
    while(getChar() == 0){ _hlt(); }
    clearText(0);
    init(); // vuelve al main
}

void exceptionDispatcher(int ex) {
    switch (ex) {
        case 0:
            exception("Zero division");
            break;
        case 6:
            exception("Invalid opcode");
            break;
        default:
            break;
    }
}
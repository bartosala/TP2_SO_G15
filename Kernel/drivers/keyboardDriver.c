#include <keyboardDriver.h>
#include <lib.h>

#define BUFFER_SIZE 1000
#define CANT_SPECIAL_KEYS 8

#define LSHIFT_PRESS 0x2A
#define LSHIFT_RELEASE 0xAA
#define RSHIFT_PRESS 0x36
#define RSHIFT_RELEASE 0xB6
#define CAPS_PRESS 0x3A

#define F12_PRESS 0x58

#define ESC_PRESS 0x01
#define ALT_PRESS 0x3B
#define CTRL_PRESS 0x1D

extern int kb_getKey();

typedef struct {
    char ascii;
    char shift_ascii; 
} ScanCode;

ScanCode press_keys[] = { 
    {0,0},
    {0x1B, 0x1B}, 
    {'1','!'},
    {'2','"'},
    {'3','#'},
    {'4','$'},
    {'5','%'},
    {'6','&'},
    {'7','('},
    {'8',')'},
    {'9',')'},
    {'0','='},
    {'-','_'},
    {'=', '+'}, 
    {0x08, 0x08}, // backspace
    {'\t', '\t'}, // tab
    {'q', 'Q'},
    {'w', 'W'},
    {'e', 'E'},
    {'r', 'R'},
    {'t', 'T'},
    {'y', 'Y'},
    {'u', 'U'},
    {'i', 'I'},
    {'o', 'O'},
    {'p', 'P'},
    {'[', '{'},
    {']', '}'},
    {'\n', '\n'}, // enter
    {0, 0}, // left ctrl
    {'a', 'A'},
    {'s', 'S'},
    {'d', 'D'},
    {'f', 'F'},
    {'g', 'G'},
    {'h', 'H'},
    {'j', 'J'},
    {'k', 'K'},
    {'l', 'L'},
    {';', ':'}, 
    {'\'', '\"'},
    {'`', '~'}, 
    {0, 0}, 
    {'\\', '|'}, 
    {'z', 'Z'},
    {'x', 'X'},
    {'c', 'C'},
    {'v', 'V'},
    {'b', 'B'},
    {'n', 'N'},
    {'m', 'M'},
    {',', '<'}, 
    {'.', '>'}, 
    {'/', '?'}, 
    {0, 0}, 
    {'*', '*'}, 
    {0, 0}, 
    {' ', ' '}, 
    {0, 0}, 
    {0, 0}, 
    {0, 0}, 
    {0, 0}, 
    {0, 0}, 
    {0, 0}, 
    {0, 0}, 
    {0, 0}, 
    {0, 0}, 
    {0, 0}, 
    {0, 0},
    {0, 0}, 
    {0, 0}, 
    {'7', '7'},
    {'8', '8'},
    {'9', '9'}, 
    {'-', '_'}, 
    {'4', '4'}, 
    {'5', '5'}, 
    {'6', '6'}, 
    {'+', '+'},
    {'1', '1'},
    {'2', '2'},
    {'3', '3'},
    {'0', '0'}, 
    {'.', '>'}, 
    {0, 0}, 
    {0, 0},
    {0, 0},
    {0, 0}, 
    {0, 0}  
};

static unsigned int specialKeys[] = {LSHIFT_PRESS, LSHIFT_RELEASE, RSHIFT_RELEASE, RSHIFT_PRESS, CAPS_PRESS, ALT_PRESS, CTRL_PRESS, ESC_PRESS};
static char buffer[BUFFER_SIZE];

unsigned int shift = 0;
unsigned int caps = 0;
unsigned int mayus = 0;
unsigned int esc = 0;
unsigned int specialKey = 0;

static unsigned int current = 0;
static unsigned int next = 0;

static char isAlpha(unsigned int key){
    return press_keys[key].ascii >= 'a' && press_keys[key].ascii <= 'z';
}

static void handleSpecialKeys(unsigned int key){
    switch(key){
        case LSHIFT_PRESS:
        case RSHIFT_PRESS:
            shift = 1;
            break;
        case LSHIFT_RELEASE:
        case RSHIFT_RELEASE:
            shift = 0;
            break;
        case CAPS_PRESS:
            caps = !caps;
            break;
        case ESC_PRESS:
            getRegisters();
            break;
        default:
            break; 
    }
}


static void checkSpecialKeys(unsigned int key){
    for(int i = 0; i < CANT_SPECIAL_KEYS && !specialKey ; i++){
        if(key == specialKeys[i]){
            handleSpecialKeys(key);
            specialKey = 1;
        }
    }
}

static void addToBuffer(unsigned int key){
    current %= BUFFER_SIZE;
    next %= BUFFER_SIZE;
    buffer[current++] = key;
}

int bufferWrite(){
    specialKey = 0;
    int c = kb_getKey();

    checkSpecialKeys(c);

    if(!specialKey && c <= F12_PRESS){  
        mayus = (caps && !shift) || (!caps && shift);
        if( (isAlpha(c) && mayus) || (!isAlpha(c) && shift) ){
            addToBuffer(press_keys[c].shift_ascii);
        } else {
            addToBuffer(press_keys[c].ascii);
        }
        return 1;
    }
    return 0;
}

char getChar(){
    if(next < current){
        return buffer[next++];
    }
    return 0;
}

void clear_buffer(){
    current = 0;
    next = 0;
    buffer[0] = 0;
}
#include "../../Shared/shared_structs.h"
#include <keyboardDriver.h>
#include <lib.h>
#include <pipe.h>
#include <process.h>
#include <scheduler.h>
#include <stdint.h>
#include <textModule.h>

#define BUFFER_SIZE 1000
#define CANT_SPECIAL_KEYS 9
#define EOF -1

#define LSHIFT_PRESS 0x2A
#define LSHIFT_RELEASE 0xAA
#define RSHIFT_PRESS 0x36
#define RSHIFT_RELEASE 0xB6
#define CAPS_PRESS 0x3A

#define F12_PRESS 0x58

#define ESC_PRESS 0x01
#define ALT_PRESS 0x3B
#define CTRL_PRESS 0x1D
#define CTRL_RELEASE 0x9D

#define STDIN_FD 0

extern int kb_getKey();

typedef struct {
	char ascii;
	char shift_ascii;
} ScanCode;

ScanCode press_keys[] = {
    {0, 0},       {0x1B, 0x1B}, {'1', '!'}, {'2', '"'},   {'3', '#'},  {'4', '$'},   {'5', '%'}, {'6', '&'}, {'7', '('},
    {'8', ')'},   {'9', ')'},   {'0', '='}, {'-', '_'},   {'=', '+'},  {0x08, 0x08}, // backspace
    {'\t', '\t'},                                                                    // tab
    {'q', 'Q'},   {'w', 'W'},   {'e', 'E'}, {'r', 'R'},   {'t', 'T'},  {'y', 'Y'},   {'u', 'U'}, {'i', 'I'}, {'o', 'O'},
    {'p', 'P'},   {'[', '{'},   {']', '}'}, {'\n', '\n'}, // enter
    {0, 0},                                               // left ctrl
    {'a', 'A'},   {'s', 'S'},   {'d', 'D'}, {'f', 'F'},   {'g', 'G'},  {'h', 'H'},   {'j', 'J'}, {'k', 'K'}, {'l', 'L'},
    {';', ':'},   {'\'', '\"'}, {'`', '~'}, {0, 0},       {'\\', '|'}, {'z', 'Z'},   {'x', 'X'}, {'c', 'C'}, {'v', 'V'},
    {'b', 'B'},   {'n', 'N'},   {'m', 'M'}, {',', ';'},   {'.', ':'},  {'-', '_'},   {0, 0},     {'*', '*'}, {0, 0},
    {' ', ' '},   {0, 0},       {0, 0},     {0, 0},       {0, 0},      {0, 0},       {0, 0},     {0, 0},     {0, 0},
    {0, 0},       {0, 0},       {0, 0},     {0, 0},       {'7', '7'},  {'8', '8'},   {'9', '9'}, {'-', '_'}, {'4', '4'},
    {'5', '5'},   {'6', '6'},   {'+', '+'}, {'1', '1'},   {'2', '2'},  {'3', '3'},   {'0', '0'}, {'.', '>'}, {0, 0},
    {0, 0},       {0, 0},       {0, 0},     {0, 0}};

static unsigned int specialKeys[] = {LSHIFT_PRESS, LSHIFT_RELEASE, RSHIFT_RELEASE, RSHIFT_PRESS, CAPS_PRESS,
                                     ALT_PRESS,    CTRL_PRESS,     CTRL_RELEASE,   ESC_PRESS};

static char shift = 0;
static char caps = 0;
static char mayus = 0;
// static char esc = 0;
static char specialKey = 0;
static char ctrlPressed = 0;

static char isAlpha(unsigned int key)
{
	return press_keys[key].ascii >= 'a' && press_keys[key].ascii <= 'z';
}

static void handleSpecialKeys(unsigned int key)
{
	switch (key) {
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
	case CTRL_PRESS:
		ctrlPressed = 1;
		break;
	case CTRL_RELEASE:
		ctrlPressed = 0;
		break;
	default:
		break;
	}
}

static void checkSpecialKeys(unsigned int key)
{
	for (int i = 0; i < CANT_SPECIAL_KEYS && !specialKey; i++) {
		if (key == specialKeys[i]) {
			handleSpecialKeys(key);
			specialKey = 1;
		}
	}
}

static void addToBuffer(unsigned int key)
{
	char c = (char)key;
	pipeWrite(0, &c, 1);
}

int bufferWrite()
{
	specialKey = 0;
	int c = kb_getKey();

	checkSpecialKeys(c);

	if (!specialKey && c < F12_PRESS) {
		if (ctrlPressed && press_keys[c].ascii == 'c') {
			kill(getForegroundPid(), 1); // CTRL + C mata al proceso actual
			printStr("^C", 0x00FFFFFF);
			lineFeed();
			return 0;
		}
		if (ctrlPressed && press_keys[c].ascii == 'd') {
			addToBuffer(EOF); // Add EOF character to buffer
			printStr("^D", 0x00FFFFFF);
			lineFeed();
			return 0;
		}
		mayus = caps ^ shift;
		if ((isAlpha(c) && mayus) || (!isAlpha(c) && shift)) {
			addToBuffer(press_keys[c].shift_ascii);
		} else {
			addToBuffer(press_keys[c].ascii);
		}
		return 1;
	}
	return 0;
}

char getChar()
{
	int stdin_fd = getCurrentStdin();
	if (stdin_fd == -1) {
		return 0;
	}

	char c = 0;
	pipeRead(stdin_fd, &c, 1);
	return c;
}

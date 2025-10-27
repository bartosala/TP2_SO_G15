#include <clock.h>
#include <lib.h>
#include <naiveConsole.h>
#include <stdint.h>
#include <textModule.h> 
#include <interrupts.h>


#define Y2K 2000
#define CANT_PARAM 6

uint8_t rtc(unsigned char reg) {
    _cli();
    outb(0x70, 128 | reg); // 128 = 10000000b
    _sti();
    return inb(0x71);
}

enum RTC_REGS {SECONDS = 0x00, MINUTES = 0x02, HOURS = 0x04, DAY_OF_MONTH = 0x07, MONTH = 0x08, YEAR = 0x09};

uint8_t BCDToDecimal(uint8_t time);

uint8_t seconds(){
    return BCDToDecimal(rtc(SECONDS));
}

uint8_t minutes(){
    return BCDToDecimal(rtc(MINUTES));
}

uint8_t hours(){
    return BCDToDecimal(rtc(HOURS));
}

uint8_t day(){
    return BCDToDecimal(rtc(DAY_OF_MONTH));
}

uint8_t month(){
    return BCDToDecimal(rtc(MONTH));
}

uint8_t year(){
    return BCDToDecimal(rtc(YEAR));
}

int isLeapYear(int year){ // From K&R - The C Programming Language
    return ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0);
}

uint8_t BCDToDecimal(uint8_t time) {
	return ((time & 0xF0) >> 4) * 10 + (time & 0x0F);
}

void checkDay(int *day, int *month, int *year) {
    char daytab[2][13] = { // from K&R - The C Programming Language
        {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
        {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
    };
    
    int leap = isLeapYear(*year);
    
    if (*day > daytab[leap][*month]) {
        *day = 1;
        (*month)++;
        if (*month > 12) {
            *month = 1;
            (*year)++;
        }
    } else if (*day <= 0) {
        (*month)--;
        if (*month <= 0) {
            *month = 12;
            (*year)--;
        }
        *day = daytab[isLeapYear(*year)][*month];
    }
}

int getDaysInMonth(int month, int year) {
    char daytab[2][13] = {
        {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
        {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
    };
    return daytab[isLeapYear(year)][month];
}


#define GMT_ARG -3

char* intToString(int num) {
    static char buffer[12]; // Enough to hold -2147483648 and null terminator
    int i = 10;
    buffer[11] = '\0';
    int isNegative = num < 0;

    if (isNegative) {
        num = -num;
    }

    do {
        buffer[i--] = (num % 10) + '0';
        num /= 10;
    } while (num > 0);

    if (isNegative) {
        buffer[i--] = '-';
    }

    return &buffer[i + 1];
}

uint64_t getTimeParam(uint64_t param) {
    uint8_t sec = seconds();
    uint8_t min = minutes();
    uint8_t hour = hours() + GMT_ARG;
    uint8_t dayVar = day();
    uint8_t monthVar = month();
    uint16_t yearVar = year() + Y2K;


    // Ajustar la hora con cambio de día
    if (hour >= 24) {
        hour -= 24;
        dayVar++;
    } else if (hour < 0) {
        hour += 24;
        dayVar--;
    }

    // Ajustar el día con cambio de mes
    if (dayVar > getDaysInMonth(monthVar, yearVar)) {
        dayVar = 1;
        monthVar++;
        if (monthVar > 12) {
            monthVar = 1;
            yearVar++;
        }
    } else if (dayVar <= 0) {
        monthVar--;
        if (monthVar <= 0) {
            monthVar = 12;
            yearVar--;
        }
        dayVar = getDaysInMonth(monthVar, yearVar);
    }

    uint64_t time[CANT_PARAM] = {sec, min, hour, dayVar, monthVar, yearVar};
    if(param > 5) return 0;
    return time[param];
}


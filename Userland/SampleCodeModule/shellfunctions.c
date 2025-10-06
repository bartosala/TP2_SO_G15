#include <shellfunctions.h>
#include <syscall.h>
#include <stdlib.h>
#include <testfunctions.h>
#include <stddef.h> // Nose porqe no toma NULL desde stdlib.h

#define CANT_REGISTERS 19

// Como implemente hanlder, pongo las funciones de asm aca 

extern void div_zero();
extern void invalid_opcode();

char *months[] = {
    "Enero",
    "Febrero",
    "Marzo",
    "Abril",
    "Mayo",
    "Junio",
    "Julio",
    "Agosto",
    "Septiembre",
    "Octubre",
    "Noviembre",
    "Diciembre"
};

char * help =   " Lista de comandos disponibles:\n"        
                "    - exit: corta la ejecucion\n"
                "    - help: muestra este menu\n"
                "    - time: muestra la hora actual GMT-3\n"
                "    - registers: muestra el ultimo snapshot (tocar ESC)\n"
                "    - echo: imprime lo que le sigue a echo\n"
                "    - size_up: aumenta tamano de fuente\n"
                "    - size_down: decrementa tamano de fuente\n"
                "    - test_div_0: test zero division exception\n"
                "    - test_invalid_opcode: test invalid opcode exception\n"
                "    - clear: borra la pantalla y comienza arriba\n"
                "    - test_mm <max>: test de acceso a memoria mapeada\n"
                "    - test_processes <num>: test de creacion de procesos\n"
                "    - test_prio: test de planificacion por prioridades\n"
                "    - test_sync: test de sincronizacion\n";


void showTime(){
    uint64_t time[] = {
        syscall_time(0), // secs
        syscall_time(1), // mins
        syscall_time(2), // hours
        syscall_time(3), // day
        syscall_time(4), // month
        syscall_time(5)  // year
    };
    char s[3] = {'0' + time[0] /10 % 10,'0' + time[0] % 10, 0};
    char m[3] = {'0' + time[1] /10 % 10, '0' + time[1] % 10, 0}; 
    char h[3] = {'0' + time[2] /10 % 10, '0' + time[2] % 10, 0};
    printf("Son las %s:%s:%s del %d de %s del %d\n", h, m, s, time[3], months[time[4]-1], time[5]);
}

void showRegisters(){    
    char * registersNames[CANT_REGISTERS] = {"RAX: ", "RBX: ", "RCX: ", "RDX: ", "RSI: ", "RDI: ",
                                            "RBP: ", "RSP: ", "R8: ", "R9: ", "R10: ", "R11: ",
                                            "R12: ", "R13: ", "R14: ", "R15: ", "RFLAGS: ", "RIP: ", "CS: "};
    uint64_t registersRead[CANT_REGISTERS];
    syscall_getRegisters(registersRead); 
    uint64_t aux = registersRead[7]; // asumiendo RSP [7] distinto de 0
    if(!aux){
        printf("No hay un guardado de registros. Presione ESC para hacer un backup\n");
        return;
    }
    for(int i = 0; i < CANT_REGISTERS ; i++){
        printf("Valor del registro %s %x \n", registersNames[i] , registersRead[i]);
    }
}

void help_handler(char * arg){
    printf("\n");
    printf(help); 
}

void time_handler(char * arg){
    showTime();
}

void registers_handler(char * arg){
    showRegisters();
}

void echo_handler(char * arguments){
    printf("%s\n", arguments);
}

void size_up_handler(char * arg){
    syscall_sizeUpFont(1);
    syscall_clearScreen();
}

void size_down_handler(char * arg){
    syscall_sizeDownFont(1);
    syscall_clearScreen();
}

void test_div_0_handler(char * arg){
    div_zero(); 
}

void test_invalid_opcode_handler(char * arg){
    invalid_opcode(); 
}

void clear_handler(char * arg){
    syscall_clearScreen();
}


void test_mm_handler(char * arg){
    if(arg == NULL || arg[0] == 0){
        printferror("Uso: test_mm <max>\n");
        return;
    }
    uint64_t max = 0;
    // convierto a numero
    for(int i = 0; arg[i]; i++){
        if(arg[i] < '0' || arg[i] > '9'){
            printf("Uso: test_mm <max>\n");
            return;
        }
        max = max * 10 + (arg[i] - '0');
    }

    if(max <= 0){
        printf("Uso: test_mm <max>\n");
        return;
    }

    printf("Test de memoria con %d", max);

    char *argv[] = { arg, NULL };
    int res = test_mm(1, argv);
    if (res == 0) {
        printf("Test completado correctamente\n");
    } else {
        printf("Test fallido con %d como codigo\n", res);
    }
}

void test_prio_handler(char * arg){
    // POR AHORA NO PORQUE NO ESTA ACTUALIZADO
    return;
}

void test_processes_handler(char * arg){
    if(arg == NULL || arg[0] == 0){
        printferror("Uso: test_processes <num>\n");
        return;
    }
    uint64_t num = 0;
    // convierto a numero
    for(int i = 0; arg[i]; i++){
        if(arg[i] < '0' || arg[i] > '9'){
            printf("Uso: test_processes <num>\n");
            return;
        }
        num = num * 10 + (arg[i] - '0');
    }

    if(num <= 0){
        printf("Uso: test_processes <num>\n");
        return;
    }

    printf("Test de procesos con %d como maximo", num);

    char *argv[] = { arg, NULL };
    int res = test_processes(1, argv);
    if (res == 0) {
        printf("Test completado correctamente\n");
    } else {
        printf("Test fallido con %d como codigo\n", res);
    }
}

void test_sync_handler(char * arg){
    // hacer 
    return;
}




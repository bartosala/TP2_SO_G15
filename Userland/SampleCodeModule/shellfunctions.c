#include <shellfunctions.h>
#include <syscall.h>
#include <stdlib.h>
#include <testfunctions.h>
#include <stddef.h> // Nose porqe no toma NULL desde stdlib.h

// Como implemente hanlder, pongo las funciones de asm aca 


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
                "    - echo: imprime lo que le sigue a echo\n"
                "    - size_up: aumenta tamano de fuente\n"
                "    - size_down: decrementa tamano de fuente\n"
                "    - clear: borra la pantalla y comienza arriba\n"
                "    - test_mm <max>: test de acceso a memoria mapeada\n"
                "    - test_processes <num>: test de creacion de procesos\n"
                "    - test_prio: test de planificacion por prioridades\n"
                "    - test_sync: test de sincronizacion\n";




void help_handler(char * arg){
    printf("\n");
    printf(help); 
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




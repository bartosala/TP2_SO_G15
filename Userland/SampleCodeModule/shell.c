#include <shellfunctions.h>
#include <shell.h>
#include <stdlib.h>
#include <syscall.h>
#include <snake.h>
#include <stdint.h>

#define BUFFER_SPACE 1000
#define MAX_ECHO 1000
#define MAX_USERNAME_LENGTH 16
#define PROMPT "%s$>"
#define CANT_INSTRUCTIONS 11
uint64_t curr = 0;

extern void div_zero();
extern void invalid_opcode();

char * help =   " Lista de comandos disponibles:\n"        
                "    - exit: corta la ejecucion\n"
                "    - help: muestra este menu\n"
                "    - snake: juego de snakes\n"
                "    - time: muestra la hora actual GMT-3\n"
                "    - registers: muestra el ultimo snapshot (tocar ESC)\n"
                "    - echo: imprime lo que le sigue a echo\n"
                "    - size_up: aumenta tamano de fuente\n"
                "    - size_down: decrementa tamano de fuente\n"
                "    - test_div_0: test zero division exception\n"
                "    - test_invalid_opcode: test invalid opcode exception\n"
                "    - clear: borra la pantalla y comienza arriba\n";

typedef enum {
    EXIT = 0,
    HELP,
    SNAKE,
    TIME,
    REGISTERS,
    ECHO, 
    SIZE_UP,
    SIZE_DOWN,
    TEST_DIV_0,
    TEST_INVALID_OPCODE,
    CLEAR
} instructions;

static char * inst_list[CANT_INSTRUCTIONS] = {"exit", 
                                            "help", 
                                            "snake", 
                                            "time", 
                                            "registers", 
                                            "echo", 
                                            "size_up",
                                            "size_down",
                                            "test_div_0", 
                                            "test_invalid_opcode", 
                                            "clear"
                                            };


int verify_instruction(char * instruction){
    for(int i = 0; i < CANT_INSTRUCTIONS; i++){
        if(strcmp(inst_list[i], instruction) == 0){
            return i;
        }
    }
    return -1;
}

int getInstruction(char * arguments){
    char shell_buffer[BUFFER_SPACE] = {0};
    readLine(shell_buffer, BUFFER_SPACE);
    int i = 0;
    int j = 0;
    char instruction[BUFFER_SPACE] = {0};
    
    for(; i < BUFFER_SPACE; i++){
        if(shell_buffer[i] == ' ' || shell_buffer[i] == '\0'){
            instruction[j] = 0;
            i++;
            break;
        }
        else {
            instruction[j++] = shell_buffer[i];
        }
    }

    int arg_index = 0;

    while (shell_buffer[i] != '\0' && shell_buffer[i] != '\n') {
        arguments[arg_index++] = shell_buffer[i++];
    }
    arguments[arg_index] = '\0';


    int iNum = 0;
    if((iNum = verify_instruction(instruction)) == -1 && instruction[0] != 0){
        printferror("Comando no reconocido: %s\n", instruction);
        return -1;
    }
    return iNum;
}

void shell() {
    syscall_clearScreen();
    syscall_sizeUpFont(1);
    printf("  Bienvenido a la shell\n\n");
    syscall_sizeDownFont(1);
    printf(help);
    // pedir username
    printf("Ingrese su nombre de usuario: ");
    char username[MAX_USERNAME_LENGTH];
    readLine(username, MAX_USERNAME_LENGTH);
    unsigned int exit = EXIT; // 0
    int instruction;
    syscall_clearScreen();
    while(!exit){
        printf(PROMPT, username);
        char arg[BUFFER_SPACE] = {0};
        instruction = getInstruction(arg); // Lee el comando que ingresa el usuario en la shell
        switch(instruction) {
// TIENEN QUE SER EJECUTADOS COMO PROCESOS Y NO COMO SHELL BUILTINS  !!!!!!! HANDLER !!!

            case HELP: {
                printf("\n");
                printf(help); 
                break;
            }
            case SNAKE: {
                runSnake();
                break;
            }
            case TIME: {
                showTime();
                break;
            }
            case REGISTERS: {
                showRegisters();
                break;
            }
            case TEST_DIV_0: {
                div_zero(); // Funcion de asm
                break;
            }
            case ECHO: {
                printf("%s\n", arg);
                break;
            }
            case SIZE_UP: {
                syscall_sizeUpFont(1);
                syscall_clearScreen();
                break;
            }
            case SIZE_DOWN: {
                syscall_sizeDownFont(1);
                syscall_clearScreen();
                break;
            }
            case EXIT : {
                exit = 1;
                break;
            }
            case TEST_INVALID_OPCODE: {
                invalid_opcode(); // Funcion de asm
                break;
            }
            case CLEAR : {
                syscall_clearScreen();
                break;
            }




            default: {
                break;
            }

            // Salto al proximo renglon si la instruccion no es Snake : 

            if(instruction != SNAKE){
                printf("\n");
            }
        }
        
    }
    printf("Saliendo de la terminal...\n");
    syscall_wait(2000);
    return;
}
#include <shellfunctions.h>
#include <shell.h>
#include <stdlib.h>
#include <syscall.h>
#include <stdint.h>

#define BUFFER_SPACE 1000
#define MAX_ECHO 1000
#define MAX_USERNAME_LENGTH 16
#define PROMPT "%s$>"
#define CANT_INSTRUCTIONS (sizeof(inst_list) / sizeof(inst_list[0]))
#define TRUE 1
uint64_t curr = 0;



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
    CLEAR,
    TEST_MM,
    TEST_PROCESSES,
    TEST_PRIO,
    TEST_SYNC
} instructions;

static char * inst_list[] = {"exit", 
                                            "help", 
                                            "time", 
                                            "registers", 
                                            "echo", 
                                            "size_up",
                                            "size_down",
                                            "test_div_0", 
                                            "test_invalid_opcode", 
                                            "clear",
                                            "test_mm",
                                            "test_processes",
                                            "test_prio",
                                            "test_sync"
                                            };

void (*handler_instruction[CANT_INSTRUCTIONS-1])(char *) = {
    help_handler,
    time_handler,
    registers_handler,
    echo_handler,
    size_up_handler,
    size_down_handler,
    test_div_0_handler,
    test_invalid_opcode_handler,
    clear_handler,
    test_mm_handler,
    test_processes_handler,
    test_prio_handler,
    test_sync_handler
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
    help_handler(0); // Muestra la ayuda al iniciar la shell
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
        instruction = getInstruction(arg); 
        if(instruction != -1){
            if(instruction != EXIT){
                // handler_instruction is indexed starting at 0 for "help",
                // while EXIT == 0 in inst_list, so use instruction - 1
                handler_instruction[instruction - 1](arg);
            } else {
                exit = TRUE;
            }
        }
    }
    printf("Saliendo de la terminal...\n");
    syscall_wait(2000);
    return;
}
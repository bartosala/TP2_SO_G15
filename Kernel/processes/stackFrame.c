#include <defs.h>
#include <memoryManager.h>
#include <stackFrame.h>
#include <lib.h>
#include <process.h>

// To set up stack frame 



static char ** copyToStack(uint64_t *paramsBase, uint64_t argc, char **argv);

uint64_t setUpStackFrame(uint64_t *base, uint64_t entryPoint, uint64_t argc, char **argv) {
    *base = (uint64_t)allocMemory(STACK_SIZE);
    if(*base == 0) {
        return 0;
    }

    *base += STACK_SIZE;
    
    uint64_t paramsBase = *base ; // espacio para parametros 
    char** newArgv = copyToStack(&paramsBase, argc, argv);
    uint64_t rspFinal = stackFrame(paramsBase, entryPoint, argc, newArgv);   
    return rspFinal;
}

static char ** copyToStack(uint64_t *paramsBase, uint64_t argc, char **argv) {
        char ** newArgv = NULL;
    if(argc > 0 && argv != NULL){

        *paramsBase -= sizeof(char*) * (argc + 1);  
        newArgv = (char**)*paramsBase;
        
        for (int i = argc - 1; i >= 0; i--) {  
            int len = strlen(argv[i]) + 1;
            *paramsBase -= len;                     
            strncpy((char*)*paramsBase, argv[i], len);  
            newArgv[i] = (char*)*paramsBase;         
        }
        newArgv[argc] = NULL;  
    }
    return newArgv; 
}


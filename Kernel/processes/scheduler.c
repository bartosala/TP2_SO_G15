#include "../include/scheduler.h"
#include "../include/memoryManager.h"
#include <lib.h>
#include <string.h>
#include "../include/textModule.h"

static SchedulerADT scheduler = NULL;

SchedulerADT getScheduler() {
    return scheduler;
}

SchedulerADT createScheduler() {
    if (scheduler) return scheduler;
    scheduler = (SchedulerADT)allocMemory(sizeof(SchedulerCDT));
    if (!scheduler) return NULL;
    memset(scheduler, 0, sizeof(SchedulerCDT));
    scheduler->nextPid = 1;
    scheduler->quantum = 1;
    scheduler->started = 1;
    return scheduler;
}

static void enqueue(Process *p) {
    if (!p || !scheduler) return;
    int pr = p->priority < SCHED_MAX_PRIORITY ? p->priority : 0;
    if (!scheduler->readyLists[pr]) scheduler->readyLists[pr] = createDoubleLinkedList();
    insertLast(scheduler->readyLists[pr], p);
    p->state = 1; // READY
}

static Process *dequeue(int pr) {
    if (!scheduler) return NULL;
    if (pr < 0 || pr >= SCHED_MAX_PRIORITY) return NULL;
    if (!scheduler->readyLists[pr] || isEmpty(scheduler->readyLists[pr])) return NULL;
    Process *p = (Process*)getFirst(scheduler->readyLists[pr]);
    removeFirst(scheduler->readyLists[pr]);
    return p;
}

uint16_t Sched_createProcess(Code entryPoint, char *name, uint64_t argc, char *argv[], uint8_t priority) {
    if (!scheduler) createScheduler();
    Process *p = (Process*)allocMemory(sizeof(Process));
    if (!p) return (uint16_t)-1;
    memset(p, 0, sizeof(Process));
    uint16_t pid = scheduler->nextPid++;
    initProcess(p, pid, 0, entryPoint, argv, name, priority, NULL);

    // allocate stack
    void *stack = allocMemory(SCHED_STACK_SIZE);
    if (!stack) {
        freeMemory(p);
        return (uint16_t)-1;
    }
    p->stackBase = stack;
    uint64_t *sp = (uint64_t*)((uint8_t*)stack + SCHED_STACK_SIZE);

    // entry point will be processWrapper; pass original entryPoint in rdx
    extern void processWrapper(int, char**, Code);
    uint64_t rip = (uint64_t)processWrapper;
    uint64_t cs = 0x08;
    uint64_t rflags = 0x202;
    // copy args into process memory (do this before building the initial register frame)
    if (argc > 0 && argv) {
        p->args = (char **)allocMemory(sizeof(char*) * (argc + 1));
        if (p->args) {
            for (uint64_t i = 0; i < argc; i++) {
                if (argv[i]) {
                    size_t len = strlen(argv[i]) + 1;
                    char *s = (char*)allocMemory(len);
                    if (s) memcpy(s, argv[i], len);
                    p->args[i] = s;
                } else {
                    p->args[i] = NULL;
                }
            }
            p->args[argc] = NULL;
        }
    }

    // place interrupt return frame in order RFLAGS, CS, RIP so after popState the iretq reads RIP,CS,RFLAGS
    *(--sp) = rflags;
    *(--sp) = cs;
    *(--sp) = rip;

    // push registers in same order as pushState: r15,r14,r13,r12,r11,r10,r9,r8,rbp,rdi,rsi,rdx,rcx,rbx,rax
    uint64_t regs[15];
    for (int i = 0; i < 15; i++) regs[i] = 0;
    // set rdi = argc, rsi = argv pointer, rdx = original entryPoint
    regs[9] = (uint64_t)argc;
    regs[10] = (uint64_t)p->args;
    regs[11] = (uint64_t)entryPoint;
    for (int i = 14; i >= 0; i--) *(--sp) = regs[i];
    p->stackPos = (void*)sp;
    p->state = 1;
    // parent/children bookkeeping
    if (scheduler->current) {
        p->parentPid = scheduler->current->pid;
        if (!scheduler->current->children) scheduler->current->children = createDoubleLinkedList();
        insertLast(scheduler->current->children, p);
        scheduler->current->childrenCount++;
    } else {
        p->parentPid = 0;
    }

    enqueue(p);
    return pid;
}

// helper to find a process by pid (search ready lists, blocked list and current)
static Process *findProcessByPid(uint16_t pid) {
    if (!scheduler) return NULL;
    if (scheduler->current && scheduler->current->pid == pid) return scheduler->current;
    for (int pr = 0; pr < SCHED_MAX_PRIORITY; pr++) {
        if (!scheduler->readyLists[pr]) continue;
        Node *it = scheduler->readyLists[pr]->first;
        while (it) {
            Process *proc = (Process*)it->data;
            if (proc->pid == pid) return proc;
            it = it->next;
        }
    }
    if (scheduler->blockedList) {
        Node *it = scheduler->blockedList->first;
        while (it) {
            Process *proc = (Process*)it->data;
            if (proc->pid == pid) return proc;
            it = it->next;
        }
    }
    return NULL;
}

int Sched_killProcess(uint16_t pid) {
    if (!scheduler) return -1;
    for (int pr = 0; pr < SCHED_MAX_PRIORITY; pr++) {
        if (!scheduler->readyLists[pr]) continue;
        Node *it = scheduler->readyLists[pr]->first;
        while (it) {
            Process *proc = (Process*)it->data;
            if (proc->pid == pid) {
                removeElement(scheduler->readyLists[pr], proc);
                proc->state = 4; // ZOMBIE
                closeFileDescriptors(proc);
                freeProcess(proc);
                return 0;
            }
            it = it->next;
        }
    }
    if (scheduler->current && scheduler->current->pid == pid) {
        scheduler->current->state = 4;
        return 0;
    }
    return -1;
}

int Sched_blockProcess(uint16_t pid) {
    if (!scheduler) return -1;
    for (int pr = 0; pr < SCHED_MAX_PRIORITY; pr++) {
        if (!scheduler->readyLists[pr]) continue;
        Node *it = scheduler->readyLists[pr]->first;
        while (it) {
            Process *proc = (Process*)it->data;
            if (proc->pid == pid) {
                removeElement(scheduler->readyLists[pr], proc);
                if (!scheduler->blockedList) scheduler->blockedList = createDoubleLinkedList();
                insertLast(scheduler->blockedList, proc);
                proc->state = 3;
                return 0;
            }
            it = it->next;
        }
    }
    return -1;
}

int Sched_unblockProcess(uint16_t pid) {
    if (!scheduler || !scheduler->blockedList) return -1;
    Node *it = scheduler->blockedList->first;
    while (it) {
        Process *proc = (Process*)it->data;
        if (proc->pid == pid) {
            removeElement(scheduler->blockedList, proc);
            enqueue(proc);
            proc->state = 1;
            return 0;
        }
        it = it->next;
    }
    return -1;
}

int Sched_setPriority(uint16_t pid, uint8_t priority) {
    if (!scheduler) return -1;
    // naive: search ready lists and blocked list and update priority
    for (int pr = 0; pr < SCHED_MAX_PRIORITY; pr++) {
        if (!scheduler->readyLists[pr]) continue;
        Node *it = scheduler->readyLists[pr]->first;
        while (it) {
            Process *proc = (Process*)it->data;
            if (proc->pid == pid) {
                proc->priority = priority;
                // move to correct queue
                removeElement(scheduler->readyLists[pr], proc);
                enqueue(proc);
                return 0;
            }
            it = it->next;
        }
    }
    // also check blocked list
    if (scheduler->blockedList) {
        Node *it = scheduler->blockedList->first;
        while (it) {
            Process *proc = (Process*)it->data;
            if (proc->pid == pid) {
                proc->priority = priority;
                return 0;
            }
            it = it->next;
        }
    }
    if (scheduler->current && scheduler->current->pid == pid) {
        scheduler->current->priority = priority;
        return 0;
    }
    return -1;
}

int Sched_yield(void) {
    if (!scheduler || !scheduler->current) return -1;
    scheduler->current->state = 1;
    enqueue(scheduler->current);
    // actual context switch will occur at next tick (schedule called from irq)
    return 0;
}

uint64_t Sched_getPid(void) {
    if (!scheduler || !scheduler->current) return 0;
    return scheduler->current->pid;
}

int Sched_waitChildren(void) {
    if (!scheduler || !scheduler->current) return -1;
    while (scheduler->current->childrenCount > 0) {
        Sched_yield();
    }
    return 0;
}

void Sched_listProcesses(void) {
    if (!scheduler) return;
    for (int pr = SCHED_MAX_PRIORITY-1; pr >= 0; pr--) {
        if (!scheduler->readyLists[pr]) continue;
        Node *it = scheduler->readyLists[pr]->first;
        while (it) {
            Process *proc = (Process*)it->data;
            if (proc->name) printStr(proc->name, 0x00FFFFFF);
            printStr(" pid=", 0x00FFFFFF);
            printInt(proc->pid, 0x00FFFFFF);
            printStr(" pr=", 0x00FFFFFF);
            printInt(proc->priority, 0x00FFFFFF);
            printStr(" state=", 0x00FFFFFF);
            printInt(proc->state, 0x00FFFFFF);
            printStr(" stackBase=", 0x00FFFFFF);
            printInt((uint64_t)proc->stackBase, 0x00FFFFFF);
            printStr(" stackPos=", 0x00FFFFFF);
            printInt((uint64_t)proc->stackPos, 0x00FFFFFF);
            printStr(" fg=", 0x00FFFFFF);
            printInt(proc->foreground, 0x00FFFFFF);
            printStr(" children=", 0x00FFFFFF);
            printInt(proc->childrenCount, 0x00FFFFFF);
            printStr(" exit=", 0x00FFFFFF);
            printInt(proc->exitCode, 0x00FFFFFF);
            printStr("\n", 0x00FFFFFF);
            it = it->next;
        }
    }
    if (scheduler->current) {
        printStr("Current pid=", 0x00FF00);
        printInt(scheduler->current->pid, 0x00FF00);
        printStr("\n", 0x00FF00);
    }
}

// schedule: called from irq handler with current RSP in rdi (rsp param)
uint64_t schedule(uint64_t rsp) {
    if (!scheduler) return rsp;
    // save current stack pointer
    if (scheduler->current) {
        scheduler->current->stackPos = (void*)rsp;
        if (scheduler->current->state == 4) { // ZOMBIE
            // notify parent: remove from parent's children list and decrement count
            Process *child = scheduler->current;
            Process *parent = findProcessByPid(child->parentPid);
            if (parent && parent->children) {
                removeElement(parent->children, child);
                if (parent->childrenCount > 0) parent->childrenCount--;
            }
            closeFileDescriptors(child);
            if (child->stackBase) freeMemory(child->stackBase);
            // free child's children list structure (does not free child objects)
            if (child->children) freeList(child->children);
            freeMemory(child);
            scheduler->current = NULL;
        } else if (scheduler->current->state == 2) { // RUNNING
            scheduler->current->state = 1;
            enqueue(scheduler->current);
        }
    }

    // pick next process
    Process *next = NULL;
    for (int pr = SCHED_MAX_PRIORITY - 1; pr >= 0; pr--) {
        if (scheduler->readyLists[pr] && !isEmpty(scheduler->readyLists[pr])) {
            next = dequeue(pr);
            break;
        }
    }

    if (!next) return rsp;
    next->state = 2; // RUNNING
    scheduler->current = next;
    if (scheduler->current->stackPos) return (uint64_t)scheduler->current->stackPos;
    return rsp;
}

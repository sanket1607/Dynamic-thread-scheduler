/**
 * Tony Givargis
 * Copyright (C), 2023
 * University of California, Irvine
 *
 * CS 238P - Operating Systems
 * scheduler.c
 */

#undef _FORTIFY_SOURCE

#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include "system.h"
#include "scheduler.h"

State state;

void initializer (void) {
    state.head = NULL;
    state.current_thread = NULL;
}

int scheduler_create(scheduler_fnc_t fnc, void *arg) {
    struct thread *new_thread;
    long unsigned pagesize = page_size();

    new_thread = (struct thread*) malloc(sizeof(struct thread));
    if (new_thread == NULL) {
        destroyer();
        EXIT("Malloc failed");
    }
    memset(new_thread, 0, sizeof (struct thread));
    new_thread->status = STATUS_;
    new_thread->function_pointer = fnc;
    new_thread->arg = arg;
    
    new_thread->stack.memory = malloc(1024*1024+pagesize);
    if (new_thread->stack.memory == NULL) {
        destroyer();
        EXIT("Malloc failed");
    }
    new_thread->stack.memory_ = memory_align(new_thread->stack.memory, pagesize);
    
    new_thread->next_thread = state.head;
    state.head = new_thread;

    return 0;
}

void scheduler_execute(void) {
    struct thread *runThread;
    setjmp(state.context);
    if (SIG_ERR == signal(SIGALRM, interrupt_handler)) {
        EXIT("Signal failed");
    }

    runThread = scheduler_find();
    if (runThread == NULL)
        return;

    state.current_thread = runThread;

    alarm(1);
    if ((runThread->status) == STATUS_) {
        uint64_t rsp = (uint64_t)runThread->stack.memory_ + (1024 * 1024);
        __asm__ volatile ("mov %[rs], %%rsp \n" : [rs] "+r" (rsp) ::);

        runThread->status = STATUS_RUNNING;            
        runThread->function_pointer(runThread->arg);
        runThread -> status = STATUS_TERMINATED;
        longjmp(state.context,1);
    }
    else {
        runThread->status = STATUS_RUNNING;
        longjmp(runThread->environ, 1);
    }
}

struct thread* scheduler_find(void) {
    struct thread *stored_current;
    struct thread *current = state.current_thread;
    if (current == NULL) {
        return state.head;
    }

    stored_current = current;
    current = current -> next_thread;   

    while (1) {
        if (current == NULL)
            current = state.head;
        
        if (current->status != STATUS_TERMINATED) {
            return current;
        }
        else {
            if (current == stored_current)
                break;
        }
        current = current -> next_thread;
    }
    return NULL;
}


void scheduler_yield(void) {
    int val;
    val = setjmp(state.current_thread->environ);
    if (val == 0) {
        state.current_thread->status = STATUS_SLEEPING;
        longjmp(state.context, 1);
    }
}

void interrupt_handler (int i) {
    assert(SIGALRM == i);
    scheduler_yield();
}

void destroyer () {
    struct thread *temp;
    if (state.head == NULL)
        return; 

    while(state.head != NULL) {
        temp = state.head->next_thread;
        FREE(state.head->stack.memory);
        FREE(state.head);
        state.head = temp;
    }
}

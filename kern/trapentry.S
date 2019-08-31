/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

#include <kern/picirq.h>


###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)                      \
    .data;\
    .long name;\
    .text;\
    .globl name;        /* define global symbol for 'name' */   \
    .type name, @function;  /* symbol type is function */       \
    .align 2;       /* align function definition */     \
    name:           /* function starts here */      \
    pushl $(num);                           \
    jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)                 \
    .data;     \
    .long name;   \
    .text;    \
    .globl name;                            \
    .type name, @function;                      \
    .align 2;                           \
    name:                               \
    pushl $0;                           \
    pushl $(num);                           \
    jmp _alltraps

#define padding()\
    .data;\
    .long 1;

.data
    .p2align 2
    .globl funs
funs:

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */

TRAPHANDLER_NOEC(handler_divide, T_DIVIDE)    /* 0 */
TRAPHANDLER_NOEC(handler_debug, T_DEBUG)      /* 1 */
TRAPHANDLER_NOEC(handler_nmi, T_NMI)          /* 2 */
TRAPHANDLER_NOEC(handler_brkpt, T_BRKPT)      /* 3 */
TRAPHANDLER_NOEC(handler_oflow, T_OFLOW)      /* 4 */
TRAPHANDLER_NOEC(handler_bound, T_BOUND)      /* 5 */
TRAPHANDLER_NOEC(handler_illop, T_ILLOP)      /* 6 */
TRAPHANDLER_NOEC(handler_device, T_DEVICE)    /* 7 */
TRAPHANDLER(handler_dblflt, T_DBLFLT)         /* 8 */
padding()                                             /* 9 reserved */
TRAPHANDLER(handler_tss, T_TSS)               /* 10 */
TRAPHANDLER(handler_segnp, T_SEGNP)           /* 11 */
TRAPHANDLER(handler_stack, T_STACK)           /* 12 */
TRAPHANDLER(handler_gpflt, T_GPFLT)           /* 13 */
TRAPHANDLER(handler_pgflt, T_PGFLT)           /* 14 */
padding()                                              /* 15 reserved */
TRAPHANDLER_NOEC(handler_fperr, T_FPERR)      /* 16 */
TRAPHANDLER(handler_align, T_ALIGN)           /* 17 */
TRAPHANDLER_NOEC(handler_mchk, T_MCHK)        /* 18 */
TRAPHANDLER_NOEC(handler_simderr, T_SIMDERR)  /* 19 */
TRAPHANDLER_NOEC(handler_syscall, T_SYSCALL)
TRAPHANDLER_NOEC(handler_32, 32)
TRAPHANDLER_NOEC(handler_33, 33)
TRAPHANDLER_NOEC(handler_34, 34)
TRAPHANDLER_NOEC(handler_35, 35)
TRAPHANDLER_NOEC(handler_36, 36)
TRAPHANDLER_NOEC(handler_37, 37)
TRAPHANDLER_NOEC(handler_38, 38)
TRAPHANDLER_NOEC(handler_39, 39)
TRAPHANDLER_NOEC(handler_40, 40)
TRAPHANDLER_NOEC(handler_41, 41)
TRAPHANDLER_NOEC(handler_42, 42)
TRAPHANDLER_NOEC(handler_43, 43)
TRAPHANDLER_NOEC(handler_44, 44)
TRAPHANDLER_NOEC(handler_45, 45)
TRAPHANDLER_NOEC(handler_46, 46)
TRAPHANDLER_NOEC(handler_47, 47)


/*
 * Lab 3: Your code here for _alltraps
 */
_alltraps:
    pushl %ds;
    pushl %es;
    pushal;
    pushl $GD_KD;
    popl %ds;
    pushl $GD_KD;
    popl %es;
    pushl %esp;
    call trap;

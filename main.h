#include <dlfcn.h>
#include <unistd.h>
#include <sys/mman.h>
#include <limits.h>
#include <stdint.h>
#ifndef PAGESIZE
#define PAGESIZE 4096
#endif

typedef uintptr_t t_ptr;


struct __attribute__((packed)) Sjmp
{
        unsigned char op;
#ifdef __LP64__
        unsigned char op2;
        void* value;
        unsigned char op3;
        unsigned char op4;

        Sjmp(void* value):
                op(0x48), op2(0xb8), op3(0xff), op4(0xe0), value(value)
        {
                /*
                 This JITs a AMD64 jmp instruction
                 */
        }
#else
        void* value;

        Sjmp(void* value):
                op(0xE9), value((void*)((t_ptr)value-(t_ptr)&op-5))
        {
                /*
                 This JITs a X86 jmp instruction
                 */
        }
#endif

};

extern bool debug;

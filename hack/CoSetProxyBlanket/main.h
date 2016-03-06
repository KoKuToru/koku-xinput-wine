#include <dlfcn.h>
#include <unistd.h>
#include <sys/mman.h>
#include <limits.h>
#ifndef PAGESIZE
#define PAGESIZE 4096
#endif

struct __attribute__((packed)) Sjmp
{
	unsigned char op;
	void* value;

	Sjmp(void* value):
		op(0xE9), value((void*)((long)value-(long)&op-5))
	{
		/*
		 This JITs a X86 jmp instruction
		 */
	}
};

extern bool debug;

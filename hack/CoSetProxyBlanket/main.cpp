#include <string>
#include "device.h"
#include "main.h"
#include <iostream>
#include <stdlib.h>
using namespace std;

#ifdef __LP64__
#warning "Add 64bit support !"
#endif

bool debug = false;

extern "C" void *wine_dll_load( const char *filename, char *error, int errorsize, int *file_exists )
{
	debug = (getenv("KOKU_XINPUT_DEBUG") != 0);

	/*
	 This is a wine intern function,
	 we get control of this function via LD_PRELOAD.

	 We check the filenames and hook some functions ;)
	*/

	if (debug)
	{
		clog << "koku-xinput-cosetproxyblanket: wine_dll_load(\"" << filename << "\", ...);" << endl;
	}

	//call original function:
	void* result = ((decltype(&wine_dll_load))dlsym(RTLD_NEXT, "wine_dll_load"))(filename, error, errorsize, file_exists);

	//check for dlls
	if (string("ole32.dll") ==  filename)
	{
		DeviceInit(result);
	}

	return result;
}

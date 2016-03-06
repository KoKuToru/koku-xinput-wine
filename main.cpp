#include "xinput.h"
#include <string>
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
		clog << "koku-xinput-wine: wine_dll_load(\"" << filename << "\", ...);" << endl;
	}

	//call original function:
	void* result = ((decltype(&wine_dll_load))dlsym(RTLD_NEXT, "wine_dll_load"))(filename, error, errorsize, file_exists);

	//check for dlls
	if (string("xinput1_3.dll") == filename)
	{
		long addr = 0;
		pair<string, void*> list[] =
		{
			{"XInputEnable"                    , (void*)&XInputEnable},
			{"XInputGetAudioDeviceIds"         , (void*)&XInputGetAudioDeviceIds},
			{"XInputGetBatteryInformation"     , (void*)&XInputGetBatteryInformation},
			{"XInputGetCapabilities"           , (void*)&XInputGetCapabilities},
			{"XInputGetDSoundAudioDeviceGuids" , (void*)&XInputGetDSoundAudioDeviceGuids},
			{"XInputGetKeystroke"              , (void*)&XInputGetKeystroke},
			{"XInputGetState"                  , (void*)&XInputGetState},
			{"XInputSetState"                  , (void*)&XInputSetState}
		};
		//hook functions
		for(int i = 0; i < 8; ++i)
		{
			addr = long(dlsym(result, list[i].first.c_str()));
			if (debug)
			{
				clog << "koku-xinput-wine: search for `" << list[i].first << "`";
			}
			if (addr != 0)
			{
				if (debug)
				{
					clog << ", found, redirect it";
				}
				long addr_start = (addr - PAGESIZE-1) & ~(PAGESIZE-1);
				long addr_end   = (addr + PAGESIZE-1) & ~(PAGESIZE-1);
				mprotect((void*)addr_start, addr_end-addr_start, PROT_READ|PROT_WRITE|PROT_EXEC);
				new ((void*)addr) Sjmp(list[i].second);
			}
			if (debug)
			{
				clog << endl;
			}
		}
	}

	return result;
}

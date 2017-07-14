#include "xinput.h"
#include <string>
#include "main.h"
#include "device.h"
#include <iostream>
#include <stdlib.h>
using namespace std;

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
		t_ptr addr = 0;
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
			addr = t_ptr(dlsym(result, list[i].first.c_str()));
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
				t_ptr addr_start = (addr - PAGESIZE-1) & ~(PAGESIZE-1);
				t_ptr addr_end   = (addr + PAGESIZE-1) & ~(PAGESIZE-1);
				mprotect((void*)addr_start, addr_end-addr_start, PROT_READ|PROT_WRITE|PROT_EXEC);
				new ((void*)addr) Sjmp(list[i].second);
			}
			if (debug)
			{
				clog << endl;
			}
		}
	}
	if (string("ole32.dll") ==  filename)
	{
		#ifndef __LP64__
                // 64 bit pointer arithmetic will need more work on the CoSetProxyBlanket module
		DeviceInit(result);
		#endif
	}

	return result;
}

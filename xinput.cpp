#include "xinput.h"
#include "config.h"
#include "main.h"
#ifndef USE_SDL2
#include <SDL/SDL.h>
#define SDL_INIT_HAPTIC 0
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_haptic.h>
#endif
#include <vector>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <iomanip>
#include <sstream>
using namespace std;

bool active = true;
static vector<SDL_Joystick*> gamepads_sdl;

GUID GUID_NULL = {0,0,0,{0,0,0,0,0,0,0,0}};

void GamepadInitSDL()
{
	static bool inited = false;
	if (inited)
	{
		return;
	}

	inited = true;
	//init:
	SDL_Init(SDL_INIT_JOYSTICK|SDL_INIT_HAPTIC);
	SDL_JoystickEventState(SDL_IGNORE);
	for(int i = 0; i < SDL_NumJoysticks(); ++i)
	{
		SDL_Joystick* joy = SDL_JoystickOpen(i);
		if (joy)
		{
			gamepads_sdl.push_back(joy);
		}
	}

	XINPUT_VIBRATION welcome_vibration;
	welcome_vibration.wLeftMotorSpeed = 65535;
	welcome_vibration.wRightMotorSpeed = 0;
	XInputSetState(0, &welcome_vibration);
	SDL_Delay(250);
	welcome_vibration.wLeftMotorSpeed = 0;
	welcome_vibration.wRightMotorSpeed = 0;
	XInputSetState(0, &welcome_vibration);
	SDL_Delay(250);
	welcome_vibration.wLeftMotorSpeed = 0;
	welcome_vibration.wRightMotorSpeed = 65535;
	XInputSetState(0, &welcome_vibration);
	SDL_Delay(250);
	welcome_vibration.wLeftMotorSpeed = 0;
	welcome_vibration.wRightMotorSpeed = 0;
	XInputSetState(0, &welcome_vibration);

	//load config:
	string path = string(getenv("HOME"))+"/.config/koku-xinput-wine.ini";
	ifstream iconfig(path.c_str(), ifstream::in);
	if (iconfig.is_open())
	{
		if (debug)
		{
			clog << "koku-xinput-wine: Load config \"" << path << "\"" << endl;
		}
		//load config

		while (iconfig.good())
		{
			string line;
			std::getline(iconfig, line);

			if (line.size() == 0)
			{
				//empty line
				continue;
			}

			if (line[0] == ';')
			{
				//comment
				continue;
			}

			int pos = 0;
			//read until not A-Z a-z 0-9
			string name;
			while(pos < line.size())
			{
				char c = line[pos++];
				if (((c >= 'A')&&(c <= 'Z')) ||
					((c >= 'a')&&(c <= 'z')) ||
					((c >= '0')&&(c <= '9')) || (c == '_'))
				{
					name += c;
				}
				else
				{
					break;
				}
			}

			//search name in mapping
			for(int i = 0; i < 20; ++i)
			{
				if (mapping[i].name == name)
				{
					//name found
					if (debug)
					{
						clog << "koku-xinput-wine: Load parameter for \"" << name << "\"" << endl;
					}
					//read until "="
					while(pos < line.size())
					{
						char c = line[pos++];
						if (c == '=')
						{
							break;
						}
					}

					//read until A-Z a-z 0-9
					while(pos < line.size())
					{
						char c = line[pos++];
						if (((c >= 'A')&&(c <= 'Z')) ||
							((c >= 'a')&&(c <= 'z')) ||
							((c >= '0')&&(c <= '9')))
						{
							--pos;
							break;
						}
					}

					if (pos >= line.size()) break;

					//read the type
					settings[i].type  = line[pos++];
					settings[i].mask  = short(0xFFFF);
					settings[i].scale = 1;
					settings[i].id    = 0;

					if (debug)
					{
						switch(settings[i].type)
						{
							case 'A':
								clog << "koku-xinput-wine: Type = Axis" << endl;
								break;
							case 'B':
								clog << "koku-xinput-wine: Type = Buttons" << endl;
								break;
							case 'H':
								clog << "koku-xinput-wine: Type = Hats" << endl;
								break;
							default:
								clog << "koku-xinput-wine: Type = Unknow" << endl;
								break;
						}
					}

					if (pos >= line.size()) break;

					//parse number
					{
						stringstream ss(string(line.c_str()+pos, line.size()-pos));
						int id;
						ss >> id;
						settings[i].id = id;
					}

					if (debug)
					{
						clog << "koku-xinput-wine: Id   = " << right << setw(2) << setfill('0') << int(settings[i].id) << setfill(' ') << setw(0) << left << endl;
					}

					//read until '&'
					while(pos < line.size())
					{
						char c = line[pos++];
						if (c == '&')
						{
							pos--;
							break;
						}
						if (c == '*')
						{
							pos--;
							break;
						}
					}

					if (pos >= line.size()) break;

					if (line[pos++] == '&')
					{
						if (pos >= line.size()) break;
						if (line[pos++] != '0') break;
						if (pos >= line.size()) break;
						if (line[pos++] != 'x') break;
						if (pos >= line.size()) break;

						//parse number
						{
							stringstream ss(string(line.c_str()+pos, line.size()-pos));
							int mask;
							ss >> hex >> mask >> dec;
							settings[i].mask = short(mask);
						}

						if (debug)
						{
							clog << "koku-xinput-wine: Mask = 0x" << right << setw(4) << setfill('0') << hex << settings[i].mask << setfill(' ') << dec << setw(0) << left << endl;
						}

						//read until '*'
						while(pos < line.size())
						{
							char c = line[pos++];
							if (c == '*')
							{
								break;
							}
						}
					}

					if (pos >= line.size()) break;

					//parse number
					{
						stringstream ss(string(line.c_str()+pos, line.size()-pos));
						ss >> settings[i].scale;
					}

					if (debug)
					{
						clog << "koku-xinput-wine: Scale = " <<  settings[i].scale << endl;
					}

					//exit loop
					break;
				}
			}
		}

		iconfig.close();
	}
	else
	{
		if (debug)
		{
			clog << "koku-xinput-wine: Save default config \"" << path << "\"" << endl;
		}
		//write default config
		ofstream oconfig(path.c_str(), ifstream::out);
		if (oconfig.is_open())
		{
			oconfig << ";koku-xinput-wine config, for more information see https://github.com/KoKuToru/koku-xinput-wine" << endl;
			for(int i = 0; i < 20; ++i)
			{
				oconfig << left << setw(30) << mapping[i].name << " = " << right << settings[i].type << setfill('0') << setw(2) << int(settings[i].id) << setfill(' ') ;
				if (settings[i].mask != short(0xFFFF))
				{
					oconfig << "&0x" << setfill('0') << setw(4) << hex << settings[i].mask << setfill(' ') << dec;
				}
				if (settings[i].scale != 1)
				{
					oconfig << "*" << settings[i].scale;
				}
				oconfig << endl;
			}
			oconfig.close();
		}else
		{
			clog << "koku-xinput-wine: couldn't open file" << endl;
		}
	}
}

void WINAPI XInputEnable(bool enable)
{
	active = enable;
	if (!active)
	{
		XINPUT_VIBRATION stop_vibration;
		stop_vibration.wLeftMotorSpeed = 0;
		stop_vibration.wRightMotorSpeed = 0;
		XInputSetState(0, &stop_vibration);
	}
}

unsigned WINAPI XInputGetAudioDeviceIds(unsigned dwUserIndex, short* pRenderDeviceId, unsigned *pRenderCount, short* pCaptureDeviceId, unsigned *pCaptureCount)
{
	GamepadInitSDL();
	if (dwUserIndex >= gamepads_sdl.size())
	{
		return ERROR_DEVICE_NOT_CONNECTED;
	}

	/*
	 If there is no headset connected to the controller,
	 the function will also retrieve ERROR_SUCCESS with NULL as the values
	 for pRenderDeviceId and pCaptureDeviceId.
	 */
	*pRenderDeviceId = 0;
	*pCaptureDeviceId = 0;
	return ERROR_SUCCESS;
}

unsigned WINAPI XInputGetBatteryInformation(unsigned dwUserIndex, char devType, XINPUT_BATTERY_INFORMATION *pBatteryInformation)
{
	GamepadInitSDL();
	if (dwUserIndex >= gamepads_sdl.size())
	{
		return ERROR_DEVICE_NOT_CONNECTED;
	}

	if (devType == BATTERY_DEVTYPE_GAMEPAD)
	{
		//sorry no real battery check
		pBatteryInformation->BatteryType  = BATTERY_TYPE_WIRED;
		pBatteryInformation->BatteryLevel = BATTERY_LEVEL_FULL;
	}
	else
	{
		pBatteryInformation->BatteryType  = BATTERY_TYPE_DISCONNECTED;
		pBatteryInformation->BatteryLevel = BATTERY_LEVEL_EMPTY;
	}
	return ERROR_SUCCESS;
}

unsigned WINAPI XInputGetCapabilities(unsigned dwUserIndex, unsigned dwFlags, XINPUT_CAPABILITIES *pCapabilities)
{
	GamepadInitSDL();
	if (dwUserIndex >= gamepads_sdl.size())
	{
		return ERROR_DEVICE_NOT_CONNECTED;
	}

	pCapabilities->Type    = XINPUT_DEVTYPE_GAMEPAD;
	pCapabilities->SubType = XINPUT_DEVSUBTYPE_GAMEPAD;
	pCapabilities->Flags   = XINPUT_CAPS_FFB_SUPPORTED;
	pCapabilities->Gamepad.wButtons = 0xFFFF;
	pCapabilities->Gamepad.bLeftTrigger = 255;
	pCapabilities->Gamepad.bRightTrigger = 255;
	pCapabilities->Gamepad.sThumbLX = 32767;
	pCapabilities->Gamepad.sThumbLY = 32767;
	pCapabilities->Gamepad.xThumbRX = 32767;
	pCapabilities->Gamepad.xThumbRY = 32767;
	pCapabilities->Vibration.wLeftMotorSpeed  = 65535;
	pCapabilities->Vibration.wRightMotorSpeed = 65535;

	return ERROR_SUCCESS;
}

unsigned WINAPI XInputGetDSoundAudioDeviceGuids(unsigned dwUserIndex, GUID* pDSoundRenderGuid, GUID* pDSoundCaptureGuid)
{
	GamepadInitSDL();
	if (dwUserIndex >= gamepads_sdl.size())
	{
		return ERROR_DEVICE_NOT_CONNECTED;
	}

	/*
	 If there is no headset connected to the controller,
	 the function also retrieves ERROR_SUCCESS with GUID_NULL as the values
	 for pDSoundRenderGuid and pDSoundCaptureGuid.
	 */
	*pDSoundRenderGuid  = GUID_NULL;
	*pDSoundCaptureGuid = GUID_NULL;
	return ERROR_SUCCESS;
}

unsigned WINAPI XInputGetKeystroke(unsigned dwUserIndex, unsigned dwReserved, XINPUT_KEYSTROKE* pKeystroke)
{
	GamepadInitSDL();
	if (dwUserIndex >= gamepads_sdl.size())
	{
		return ERROR_DEVICE_NOT_CONNECTED;
	}

	//If no new keys have been pressed, the return value is ERROR_EMPTY.
	return ERROR_EMPTY;
}

unsigned WINAPI XInputGetState(unsigned dwUserIndex, XINPUT_STATE *pState)
{
	GamepadInitSDL();
	if (dwUserIndex >= gamepads_sdl.size())
	{
		return ERROR_DEVICE_NOT_CONNECTED;
	}

	SDL_JoystickUpdate();
	//load data
	for(int i = 0; i < 20; ++i)
	{
		short value = 0;
		switch(settings[i].type)
		{
			case 'A':
				value = max(SDL_JoystickGetAxis(gamepads_sdl[dwUserIndex]   , settings[i].id-1)&settings[i].mask, -32767)*settings[i].scale;
				break;
			case 'H':
				value = max(SDL_JoystickGetHat(gamepads_sdl[dwUserIndex]    , settings[i].id-1)&settings[i].mask, -32767)*settings[i].scale;
				break;
			case 'B':
				value = max(SDL_JoystickGetButton(gamepads_sdl[dwUserIndex] , settings[i].id-1)&settings[i].mask, -32767)*settings[i].scale;
				break;
			default:
				//uhm
				break;
		}
		if (mapping[i].mask == 0)
		{
			*(mapping[i].addr) = value;
		}
		else if (value != 0)
		{
			*(mapping[i].addr) |= mapping[i].mask;
		}
		else
		{
			*(mapping[i].addr) &= (~mapping[i].mask);
		}
	}

	//set data
	static int dwPacketNumber = 0;
	pState->dwPacketNumber = ++dwPacketNumber;
	pState->Gamepad.wButtons     = *(reinterpret_cast<unsigned short*>(&gamepad.buttons));
	pState->Gamepad.bLeftTrigger  = gamepad.left_shoulder;
	pState->Gamepad.bRightTrigger = gamepad.right_shoulder;
	pState->Gamepad.sThumbLX      = gamepad.left_thumb_x;
	pState->Gamepad.sThumbLY      = gamepad.left_thumb_y;
	pState->Gamepad.xThumbRX      = gamepad.right_thumb_x;
	pState->Gamepad.xThumbRY      = gamepad.right_thumb_y;
	return ERROR_SUCCESS;
}

unsigned WINAPI XInputSetState(unsigned dwUserIndex, XINPUT_VIBRATION *pVibration)
{
	GamepadInitSDL();
	if (dwUserIndex >= gamepads_sdl.size())
	{
		return ERROR_DEVICE_NOT_CONNECTED;
	}

	if (active)
	{
		//Sorry no vibration in SDL1.2, maybe SDL2
		#ifdef USE_SDL2
		static SDL_Haptic *haptic = 0;
		static int effect_id[2];

		SDL_HapticEffect effect[2];
		memset(&effect, 0, sizeof(SDL_HapticEffect)*2);

		//#define USE_CONSTANT .. not working no idea why

		#ifdef USE_CONSTANT
		effect[0].type = SDL_HAPTIC_CONSTANT;
		effect[0].constant.direction.type = SDL_HAPTIC_CARTESIAN;
		effect[0].constant.direction.dir[0] =  0;
		effect[0].constant.direction.dir[1] =  1;
		effect[0].constant.level = pVibration->wLeftMotorSpeed>>1;
		effect[0].constant.length = SDL_HAPTIC_INFINITY;
		effect[0].constant.attack_length = 1000; // Takes 1 second to get max strength
		effect[0].constant.fade_length = 1000; // Takes 1 second to fade away

		effect[1] = effect[0];
		effect[1].constant.direction.dir[0] = -1;
		effect[1].periodic.magnitude = pVibration->wRightMotorSpeed>>1;
		#else
		effect[0].type = SDL_HAPTIC_SINE;
		effect[0].constant.direction.type = SDL_HAPTIC_CARTESIAN;
		effect[0].constant.direction.dir[0] =  1;
		effect[0].constant.direction.dir[1] =  0;
		effect[0].periodic.direction.dir[0] = 18000; // Force comes from south
		effect[0].periodic.period = SDL_HAPTIC_INFINITY; // 1000 ms
		effect[0].periodic.magnitude = pVibration->wLeftMotorSpeed>>1; // 20000/32767 strength
		effect[0].periodic.length = SDL_HAPTIC_INFINITY; // 5 seconds long

		effect[1] = effect[0];
		effect[1].constant.direction.dir[0] = -1;
		effect[1].periodic.magnitude = pVibration->wRightMotorSpeed>>1;
		#endif

		if (haptic == 0)
		{
			haptic = SDL_HapticOpenFromJoystick(gamepads_sdl[dwUserIndex]);

			effect_id[0] = SDL_HapticNewEffect(haptic, &(effect[0]));
			effect_id[1] = SDL_HapticNewEffect(haptic, &(effect[1]));

			SDL_HapticRunEffect(haptic, effect_id[0], 1);
			SDL_HapticRunEffect(haptic, effect_id[1], 1);
		}
		else
		{
			SDL_HapticUpdateEffect(haptic, effect_id[0], &(effect[0]));
			SDL_HapticUpdateEffect(haptic, effect_id[1], &(effect[1]));
		}
		#endif
	}
	return ERROR_SUCCESS;
}

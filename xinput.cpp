#include "xinput.h"
#include "config.h"

#include <SDL/SDL.h>
#include <vector>
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
	SDL_Init(SDL_INIT_JOYSTICK);
	SDL_JoystickEventState(SDL_IGNORE);
	for(int i = 0; i < SDL_NumJoysticks(); ++i)
	{
		SDL_Joystick* joy = SDL_JoystickOpen(i);
		if (joy)
		{
			gamepads_sdl.push_back(joy);
		}
	}
}

void WINAPI XInputEnable(bool enable)
{
	active = enable;
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
	}
	return ERROR_SUCCESS;
}

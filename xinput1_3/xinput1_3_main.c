/*
 * The Wine project - Xinput Joystick Library
 * Copyright 2008 Andrew Fenn
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, 
USA
 */

#include "config.h"
#include <assert.h>
#include <stdarg.h>
#include <string.h>

#include <wine/debug.h>

#include <windef.h>
#include <winbase.h>
#include <winerror.h>
#include <winnt.h>
#include <xinput.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_haptic.h>

#define BATTERY_DEVTYPE_GAMEPAD		0x00
#define BATTERY_DEVTYPE_HEADSET		0x01
#define BATTERY_TYPE_DISCONNECTED	0x00
#define BATTERY_TYPE_WIRED			0x01
#define BATTERY_TYPE_ALKALINE		0x02
#define BATTERY_TYPE_NIMH			0x03
#define BATTERY_TYPE_UNKNOWN		0xFF
#define BATTERY_LEVEL_EMPTY			0x00
#define BATTERY_LEVEL_LOW			0x01
#define BATTERY_LEVEL_MEDIUM		0x02
#define BATTERY_LEVEL_FULL			0x03
#define XINPUT_CAPS_FFB_SUPPORTED   0x0001
#define XINPUT_GAMEPAD_DPAD_UP	 	 0x0001
#define XINPUT_GAMEPAD_DPAD_DOWN	 0x0002
#define XINPUT_GAMEPAD_DPAD_LEFT	 0x0004
#define XINPUT_GAMEPAD_DPAD_RIGHT	 0x0008
#define XINPUT_GAMEPAD_START	 	 0x0010
#define XINPUT_GAMEPAD_BACK	 	 0x0020
#define XINPUT_GAMEPAD_LEFT_THUMB	 0x0040
#define XINPUT_GAMEPAD_RIGHT_THUMB	 0x0080
#define XINPUT_GAMEPAD_LEFT_SHOULDER	 0x0100
#define XINPUT_GAMEPAD_RIGHT_SHOULDER	 0x0200
#define XINPUT_GAMEPAD_A		 0x1000
#define XINPUT_GAMEPAD_B		 0x2000
#define XINPUT_GAMEPAD_X		 0x4000
#define XINPUT_GAMEPAD_Y		 0x8000
#define XINPUT_DEVTYPE_GAMEPAD 0x01
#define XINPUT_DEVSUBTYPE_GAMEPAD 0x01

#define WINE_XINPUT_AXES     8
#define MAX_XINPUT_BUTTONS 14
static const unsigned int xbuttons[MAX_XINPUT_BUTTONS] = {XINPUT_GAMEPAD_START, XINPUT_GAMEPAD_BACK, XINPUT_GAMEPAD_LEFT_THUMB, XINPUT_GAMEPAD_RIGHT_THUMB, XINPUT_GAMEPAD_LEFT_SHOULDER, XINPUT_GAMEPAD_RIGHT_SHOULDER, XINPUT_GAMEPAD_A, XINPUT_GAMEPAD_B, XINPUT_GAMEPAD_X, XINPUT_GAMEPAD_Y, XINPUT_GAMEPAD_DPAD_DOWN, XINPUT_GAMEPAD_DPAD_LEFT, XINPUT_GAMEPAD_DPAD_RIGHT, XINPUT_GAMEPAD_DPAD_UP};
static const SDL_GameControllerButton sdlbuttons[MAX_XINPUT_BUTTONS] = {SDL_CONTROLLER_BUTTON_START, SDL_CONTROLLER_BUTTON_BACK, SDL_CONTROLLER_BUTTON_LEFTSTICK, SDL_CONTROLLER_BUTTON_RIGHTSTICK, SDL_CONTROLLER_BUTTON_LEFTSHOULDER, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, SDL_CONTROLLER_BUTTON_A, SDL_CONTROLLER_BUTTON_B, SDL_CONTROLLER_BUTTON_X, SDL_CONTROLLER_BUTTON_Y, SDL_CONTROLLER_BUTTON_DPAD_DOWN, SDL_CONTROLLER_BUTTON_DPAD_LEFT, SDL_CONTROLLER_BUTTON_DPAD_RIGHT, SDL_CONTROLLER_BUTTON_DPAD_UP};

GUID EMPTY_GUID = {0,0,0,{0,0,0,0,0,0,0,0}};

WINE_DEFAULT_DEBUG_CHANNEL(xinput);

struct Gamepad
{
    SDL_Joystick *joystick;
    SDL_GameController* controller;
    SDL_Haptic   *haptic;
    int           hapticEffects[2];
};
int gamepadsSdlCount = 0;
struct Gamepad* gamepadsSdl;

BOOL WINAPI DllMain(HINSTANCE inst, DWORD reason, LPVOID reserved) {
    switch(reason)
    {
    case DLL_WINE_PREATTACH:
        break;
        // return FALSE; /* prefer native version */
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(inst);
        
        // The following will memory leak hard, but I don't care
        
        SDL_Init(SDL_INIT_JOYSTICK|SDL_INIT_HAPTIC|SDL_INIT_GAMECONTROLLER);
        SDL_JoystickEventState(SDL_IGNORE);
        SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");
        
        int numJoystick = SDL_NumJoysticks();
        gamepadsSdl = (struct Gamepad*)calloc(numJoystick, sizeof(struct Gamepad));
        // get all gamepads
        struct Gamepad* gamepad = gamepadsSdl;
        for (int i = 0; i < numJoystick; ++i) {
            gamepad->joystick = SDL_JoystickOpen(i);
            if (!gamepad->joystick) {
                continue;
            }
            gamepad->controller = SDL_GameControllerOpen(i);
            if (!gamepad->controller) {
                continue;
            }
            gamepad->haptic = SDL_HapticOpenFromJoystick(gamepad->joystick);
            if (gamepad->haptic) {
                SDL_HapticEffect effect[2];
                memset(&effect, 0, sizeof(SDL_HapticEffect)*2);

                effect[0].type = SDL_HAPTIC_SINE;
                effect[0].periodic.direction.type = SDL_HAPTIC_CARTESIAN;
                effect[0].periodic.direction.dir[0] =  1;
                effect[0].periodic.period = 0;
                effect[0].periodic.magnitude = 0;
                effect[0].periodic.length = SDL_HAPTIC_INFINITY;

                effect[1] = effect[0];
                effect[1].periodic.direction.dir[0] = -1;
                effect[1].periodic.magnitude = 0;

                gamepad->hapticEffects[0] = SDL_HapticNewEffect(gamepad->haptic, &(effect[0]));
                gamepad->hapticEffects[1] = SDL_HapticNewEffect(gamepad->haptic, &(effect[1]));

                SDL_HapticRunEffect(gamepad->haptic, gamepad->hapticEffects[0], 1);
                SDL_HapticRunEffect(gamepad->haptic, gamepad->hapticEffects[1], 1);
            }
            //next controller
            gamepad++;
            gamepadsSdlCount++;
        }
        
        XINPUT_VIBRATION welcomeVibration;
        welcomeVibration.wLeftMotorSpeed = 65535;
        welcomeVibration.wRightMotorSpeed = 0;
        XInputSetState(0, &welcomeVibration);
        SDL_Delay(250);
        welcomeVibration.wLeftMotorSpeed = 0;
        welcomeVibration.wRightMotorSpeed = 0;
        XInputSetState(0, &welcomeVibration);
        SDL_Delay(250);
        welcomeVibration.wLeftMotorSpeed = 0;
        welcomeVibration.wRightMotorSpeed = 65535;
        XInputSetState(0, &welcomeVibration);
        SDL_Delay(250);
        welcomeVibration.wLeftMotorSpeed = 0;
        welcomeVibration.wRightMotorSpeed = 0;
        XInputSetState(0, &welcomeVibration);
        break;
    }
    return TRUE;
}

BOOL active = FALSE;

void WINAPI XInputEnable(BOOL enable) {
    if (!active) {
        XINPUT_VIBRATION stopVibration;
        stopVibration.wLeftMotorSpeed = 0;
        stopVibration.wRightMotorSpeed = 0;
        for(int i = 0; i < 4; ++i) {
            XInputSetState(i, &stopVibration);
        }
    }
    active = enable;
}

DWORD WINAPI XInputSetState(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration) {
    if (dwUserIndex >= gamepadsSdlCount) {
        return ERROR_DEVICE_NOT_CONNECTED;
    }

    if (active) {
        struct Gamepad* gamepad = gamepadsSdl + dwUserIndex;
        if (gamepad->haptic != 0) {
            SDL_HapticEffect effect[2];
            memset(&effect, 0, sizeof(SDL_HapticEffect)*2);

            effect[0].type = SDL_HAPTIC_SINE;
            effect[0].periodic.direction.type = SDL_HAPTIC_CARTESIAN;
            effect[0].periodic.direction.dir[0] =  1;
            effect[0].periodic.period = 0;
            effect[0].periodic.magnitude = pVibration->wLeftMotorSpeed>>1;
            effect[0].periodic.length = SDL_HAPTIC_INFINITY;

            effect[1] = effect[0];
            effect[1].periodic.direction.dir[0] = -1;
            effect[1].periodic.magnitude = pVibration->wRightMotorSpeed>>1;

            SDL_HapticUpdateEffect(gamepad->haptic, gamepad->hapticEffects[0], &(effect[0]));
            SDL_HapticUpdateEffect(gamepad->haptic, gamepad->hapticEffects[1], &(effect[1]));
        }
    }
    return ERROR_SUCCESS;
}

DWORD WINAPI  XInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState) {
    if (dwUserIndex >= gamepadsSdlCount) {
        return ERROR_DEVICE_NOT_CONNECTED;
    }

    SDL_JoystickUpdate();
    SDL_GameControllerUpdate();

    //set data
    SDL_GameController* controller = gamepadsSdl[dwUserIndex].controller;

    for (int j = 0; j < MAX_XINPUT_BUTTONS; j++) {
        Uint8 result = SDL_GameControllerGetButton(controller, sdlbuttons[j]);

        if (result) {
            pState->Gamepad.wButtons |= xbuttons[j];
        } else {
            pState->Gamepad.wButtons &= ~(xbuttons[j]);
        }
    }

    short ly = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
    short ry = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY);

    pState->Gamepad.sThumbLX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
    pState->Gamepad.sThumbLY = (ly == 0 ? 0 : -ly -1);
    pState->Gamepad.sThumbRX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX);
    pState->Gamepad.sThumbRY = ry == 0 ? 0 : -ry -1;
    pState->Gamepad.bLeftTrigger = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
    pState->Gamepad.bRightTrigger = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);

    static int dwPacketNumber = 0;
    pState->dwPacketNumber = ++dwPacketNumber;

    return ERROR_SUCCESS;
}

DWORD WINAPI XInputGetKeystroke(DWORD dwUserIndex, DWORD dwReserve, PXINPUT_KEYSTROKE pKeystroke) {
    if (dwUserIndex >= gamepadsSdlCount) {
        return ERROR_DEVICE_NOT_CONNECTED;
    }

    //If no new keys have been pressed, the return value is ERROR_EMPTY.
    return ERROR_EMPTY;
}

DWORD WINAPI XInputGetCapabilities(DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES* pCapabilities) {
    std::cerr << "koku-xinput-wine:" << "XInputGetCapabilities(" << dwUserIndex << ", " << dwFlags << ", " << (void*) pCapabilities << ") called" << std::endl;
    if (dwUserIndex >= gamepadsSdlCount) {
        std::cerr << "koku-xinput-wine:" << "return ERROR_DEVICE_NOT_CONNECTED" << std::endl;
        return ERROR_DEVICE_NOT_CONNECTED;
    }
    std::cerr << "koku-xinput-wine:" << "fill Data";
    pCapabilities->Type    = XINPUT_DEVTYPE_GAMEPAD;
    pCapabilities->SubType = XINPUT_DEVSUBTYPE_GAMEPAD;
    pCapabilities->Flags   = (gamepadsSdl[dwUserIndex].haptic != 0)?XINPUT_CAPS_FFB_SUPPORTED:0;
    pCapabilities->Gamepad.wButtons = 0xFFFF;
    pCapabilities->Gamepad.bLeftTrigger = 255;
    pCapabilities->Gamepad.bRightTrigger = 255;
    pCapabilities->Gamepad.sThumbLX = 32767;
    pCapabilities->Gamepad.sThumbLY = 32767;
    pCapabilities->Gamepad.sThumbRX = 32767;
    pCapabilities->Gamepad.sThumbRY = 32767;
    if (gamepadsSdl[dwUserIndex].haptic != 0) {
        pCapabilities->Vibration.wLeftMotorSpeed  = 65535;
        pCapabilities->Vibration.wRightMotorSpeed = 65535;
    }
    else {
        pCapabilities->Vibration.wLeftMotorSpeed  = 0;
        pCapabilities->Vibration.wRightMotorSpeed = 0;
    }
    std::cerr << "koku-xinput-wine:" << "return ERROR_SUCCESS" << std::endl;
    return ERROR_SUCCESS;
}

DWORD WINAPI XInputGetDSoundAudioDeviceGuids(DWORD dwUserIndex, GUID* pDSoundRenderGuid, GUID* pDSoundCaptureGuid) {
    if (dwUserIndex >= gamepadsSdlCount) {
        return ERROR_DEVICE_NOT_CONNECTED;
    }

    /*
     If there is no headset connected to the controller,
     the function also retrieves ERROR_SUCCESS with EMPTY_GUID as the values
     for pDSoundRenderGuid and pDSoundCaptureGuid.
     */
    *pDSoundRenderGuid  = EMPTY_GUID;
    *pDSoundCaptureGuid = EMPTY_GUID;
    return ERROR_SUCCESS;
}

DWORD WINAPI XInputGetBatteryInformation(DWORD dwUserIndex, BYTE deviceType, XINPUT_BATTERY_INFORMATION* pBatteryInfo) {
    if (dwUserIndex >= gamepadsSdlCount) {
        return ERROR_DEVICE_NOT_CONNECTED;
    }

    if (deviceType == BATTERY_DEVTYPE_GAMEPAD) {
        //sorry no real battery check
        pBatteryInfo->BatteryType  = BATTERY_TYPE_WIRED;
        pBatteryInfo->BatteryLevel = BATTERY_LEVEL_FULL;
    }
    else {
        pBatteryInfo->BatteryType  = BATTERY_TYPE_DISCONNECTED;
        pBatteryInfo->BatteryLevel = BATTERY_LEVEL_EMPTY;
    }
    return ERROR_SUCCESS;
}

DWORD WINAPI XInputGetAudioDeviceIds(DWORD dwUserIndex, LPWSTR pRenderDeviceId, UINT* pRenderCount, LPWSTR pCaptureDeviceId, UINT* pCaptureCount) {
    if (dwUserIndex >= gamepadsSdlCount) {
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

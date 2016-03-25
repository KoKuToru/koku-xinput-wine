#include "xinput.h"
#include "main.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_haptic.h>

#include <vector>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <iomanip>
#include <sstream>

#define WINE_XINPUT_AXES     8
#define MAX_XINPUT_BUTTONS 14
using namespace std;

bool active = true;
struct Sgamepad_sdl
{
    SDL_Joystick *joystick;
    SDL_GameController* controller;
    SDL_Haptic   *haptic;
    int           haptic_effects[2];
};
static vector<Sgamepad_sdl> gamepads_sdl;
static const unsigned int xbuttons[MAX_XINPUT_BUTTONS] = {XINPUT_GAMEPAD_START, XINPUT_GAMEPAD_BACK, XINPUT_GAMEPAD_LEFT_THUMB, XINPUT_GAMEPAD_RIGHT_THUMB, XINPUT_GAMEPAD_LEFT_SHOULDER, XINPUT_GAMEPAD_RIGHT_SHOULDER, XINPUT_GAMEPAD_A, XINPUT_GAMEPAD_B, XINPUT_GAMEPAD_X, XINPUT_GAMEPAD_Y, XINPUT_GAMEPAD_DPAD_DOWN, XINPUT_GAMEPAD_DPAD_LEFT, XINPUT_GAMEPAD_DPAD_RIGHT, XINPUT_GAMEPAD_DPAD_UP};
static const SDL_GameControllerButton sdlbuttons[MAX_XINPUT_BUTTONS] = {SDL_CONTROLLER_BUTTON_START, SDL_CONTROLLER_BUTTON_BACK, SDL_CONTROLLER_BUTTON_LEFTSTICK, SDL_CONTROLLER_BUTTON_RIGHTSTICK, SDL_CONTROLLER_BUTTON_LEFTSHOULDER, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, SDL_CONTROLLER_BUTTON_A, SDL_CONTROLLER_BUTTON_B, SDL_CONTROLLER_BUTTON_X, SDL_CONTROLLER_BUTTON_Y, SDL_CONTROLLER_BUTTON_DPAD_DOWN, SDL_CONTROLLER_BUTTON_DPAD_LEFT, SDL_CONTROLLER_BUTTON_DPAD_RIGHT, SDL_CONTROLLER_BUTTON_DPAD_UP};
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
    SDL_Init(SDL_INIT_JOYSTICK|SDL_INIT_HAPTIC|SDL_INIT_GAMECONTROLLER);
    SDL_JoystickEventState(SDL_IGNORE);
    SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");
    for(int i = 0; i < SDL_NumJoysticks(); ++i)
    {
        SDL_Joystick* joy = SDL_JoystickOpen(i);
        if (joy)
        {
            Sgamepad_sdl new_gamepad;
            SDL_GameController* controller = SDL_GameControllerOpen(i);
            if (controller) {

                new_gamepad.joystick = joy;
                new_gamepad.controller = controller;
                new_gamepad.haptic = 0;
                //check for haptic

                new_gamepad.haptic = SDL_HapticOpenFromJoystick(new_gamepad.joystick);
                if (new_gamepad.haptic != 0)
                {
                    //start haptic effects
                    SDL_HapticEffect effect[2];
                    memset(&effect, 0, sizeof(SDL_HapticEffect)*2);

                    effect[0].type = SDL_HAPTIC_SINE; //constant somehow don't work, or I don't undestand what it does..
                    effect[0].periodic.direction.type = SDL_HAPTIC_CARTESIAN;
                    effect[0].periodic.direction.dir[0] =  1;
                    effect[0].periodic.period = 0;
                    effect[0].periodic.magnitude = 0;
                    effect[0].periodic.length = SDL_HAPTIC_INFINITY;

                    effect[1] = effect[0];
                    effect[1].periodic.direction.dir[0] = -1;
                    effect[1].periodic.magnitude = 0;

                    new_gamepad.haptic_effects[0] = SDL_HapticNewEffect(new_gamepad.haptic, &(effect[0]));
                    new_gamepad.haptic_effects[1] = SDL_HapticNewEffect(new_gamepad.haptic, &(effect[1]));

                    SDL_HapticRunEffect(new_gamepad.haptic, new_gamepad.haptic_effects[0], 1);
                    SDL_HapticRunEffect(new_gamepad.haptic, new_gamepad.haptic_effects[1], 1);
                }
                gamepads_sdl.push_back(new_gamepad);
            }

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
}

void WINAPI XInputEnable(bool enable)
{
    if (!enable)
    {
        XINPUT_VIBRATION stop_vibration;
        stop_vibration.wLeftMotorSpeed = 0;
        stop_vibration.wRightMotorSpeed = 0;
        for(int i = 0; i < 4; ++i)
        {
            XInputSetState(i, &stop_vibration);
        }
    }
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
    if (pRenderCount)
    {
        *pRenderDeviceId = 0;
    }
    if (pCaptureDeviceId)
    {
        *pCaptureDeviceId = 0;
    }
    return ERROR_SUCCESS;
}

unsigned WINAPI XInputGetBatteryInformation(unsigned dwUserIndex, char devType, XINPUT_BATTERY_INFORMATION *pBatteryInformation)
{
    GamepadInitSDL();
    if (dwUserIndex >= gamepads_sdl.size())
    {
        return ERROR_DEVICE_NOT_CONNECTED;
    }

    if (pBatteryInformation)
    {
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
    
    if (pCapabilities)
    {
        pCapabilities->Type    = XINPUT_DEVTYPE_GAMEPAD;
        pCapabilities->SubType = XINPUT_DEVSUBTYPE_GAMEPAD;
        pCapabilities->Flags   = (gamepads_sdl[dwUserIndex].haptic != 0)?XINPUT_CAPS_FFB_SUPPORTED:0;
        pCapabilities->Gamepad.wButtons = 0xFFFF;
        pCapabilities->Gamepad.bLeftTrigger = 255;
        pCapabilities->Gamepad.bRightTrigger = 255;
        pCapabilities->Gamepad.sThumbLX = 32767;
        pCapabilities->Gamepad.sThumbLY = 32767;
        pCapabilities->Gamepad.sThumbRX = 32767;
        pCapabilities->Gamepad.sThumbRY = 32767;
        if (gamepads_sdl[dwUserIndex].haptic != 0)
        {
            pCapabilities->Vibration.wLeftMotorSpeed  = 65535;
            pCapabilities->Vibration.wRightMotorSpeed = 65535;
        }
        else
        {
            pCapabilities->Vibration.wLeftMotorSpeed  = 0;
            pCapabilities->Vibration.wRightMotorSpeed = 0;
        }
    }
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
    if (pDSoundRenderGuid) {
        *pDSoundRenderGuid  = GUID_NULL;
    }
    if (pDSoundCaptureGuid) {
        *pDSoundCaptureGuid = GUID_NULL;
    }
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
    SDL_GameControllerUpdate();

    //set data
    if (pState) 
    {
        SDL_GameController* controller = gamepads_sdl[dwUserIndex].controller;

        for (int j = 0; j < MAX_XINPUT_BUTTONS; j++) {

            Uint8 result = SDL_GameControllerGetButton(controller, sdlbuttons[j]);
            //printf("result for %d: %d\n", j, result);
            if (result) {
                pState->Gamepad.wButtons |= xbuttons[j];
            }
            else {
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
    }
    return ERROR_SUCCESS;
}

unsigned WINAPI XInputSetState(unsigned dwUserIndex, XINPUT_VIBRATION *pVibration)
{
    GamepadInitSDL();
    if (dwUserIndex >= gamepads_sdl.size())
    {
        return ERROR_DEVICE_NOT_CONNECTED;
    }

    if (active && pVibration)
    {
        if (gamepads_sdl[dwUserIndex].haptic != 0)
        {
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

            SDL_HapticUpdateEffect(gamepads_sdl[dwUserIndex].haptic, gamepads_sdl[dwUserIndex].haptic_effects[0], &(effect[0]));
            SDL_HapticUpdateEffect(gamepads_sdl[dwUserIndex].haptic, gamepads_sdl[dwUserIndex].haptic_effects[1], &(effect[1]));
        }
    }
    return ERROR_SUCCESS;
}

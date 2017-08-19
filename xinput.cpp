#include "main.h"

#include <cstdio>
#include <cstring>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_haptic.h>

#include <winerror.h>
#include <xinput.h>

#define XINPUT_CAPS_FFB_SUPPORTED 0x0001

DWORD WINAPI XInputGetAudioDeviceIds(DWORD dwUserIndex, LPWSTR pRenderDeviceId,
                                     UINT *pRenderCount,
                                     LPWSTR pCaptureDeviceId,
                                     UINT *pCaptureCount);

#define WINE_XINPUT_AXES 8
#define MAX_XINPUT_BUTTONS 14

namespace koku {
struct SDLGamepad {
  SDL_Joystick *joystick;
  SDL_GameController *controller;
  SDL_Haptic *haptic;
  int haptic_effects[2];
};
static std::vector<SDLGamepad> gamepads;
static const unsigned int xbuttons[MAX_XINPUT_BUTTONS] = {
    XINPUT_GAMEPAD_START,
    XINPUT_GAMEPAD_BACK,
    XINPUT_GAMEPAD_LEFT_THUMB,
    XINPUT_GAMEPAD_RIGHT_THUMB,
    XINPUT_GAMEPAD_LEFT_SHOULDER,
    XINPUT_GAMEPAD_RIGHT_SHOULDER,
    XINPUT_GAMEPAD_A,
    XINPUT_GAMEPAD_B,
    XINPUT_GAMEPAD_X,
    XINPUT_GAMEPAD_Y,
    XINPUT_GAMEPAD_DPAD_DOWN,
    XINPUT_GAMEPAD_DPAD_LEFT,
    XINPUT_GAMEPAD_DPAD_RIGHT,
    XINPUT_GAMEPAD_DPAD_UP};
static const SDL_GameControllerButton sdlbuttons[MAX_XINPUT_BUTTONS] = {
    SDL_CONTROLLER_BUTTON_START,
    SDL_CONTROLLER_BUTTON_BACK,
    SDL_CONTROLLER_BUTTON_LEFTSTICK,
    SDL_CONTROLLER_BUTTON_RIGHTSTICK,
    SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
    SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
    SDL_CONTROLLER_BUTTON_A,
    SDL_CONTROLLER_BUTTON_B,
    SDL_CONTROLLER_BUTTON_X,
    SDL_CONTROLLER_BUTTON_Y,
    SDL_CONTROLLER_BUTTON_DPAD_DOWN,
    SDL_CONTROLLER_BUTTON_DPAD_LEFT,
    SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
    SDL_CONTROLLER_BUTTON_DPAD_UP};

static bool enabled = true;

DWORD WINAPI XInputSetState(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration);
void GamepadInitSDL() {
  static bool inited = false;
  if (inited)
    return;
  debug("");

  inited = true;

  SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMECONTROLLER);
  SDL_JoystickEventState(SDL_IGNORE);
  SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");

  for (int i = 0; i < SDL_NumJoysticks(); ++i) {
    SDL_Joystick *joy = SDL_JoystickOpen(i);
    if (joy) {
      SDLGamepad gamepad;
      SDL_GameController *controller = SDL_GameControllerOpen(i);

      if (controller) {
        gamepad.joystick = joy;
        gamepad.controller = controller;
        gamepad.haptic = 0;

        gamepad.haptic = SDL_HapticOpenFromJoystick(gamepad.joystick);
        if (gamepad.haptic != 0) {
          SDL_HapticEffect effect[2];
          memset(&effect, 0, sizeof(SDL_HapticEffect) * 2);

          effect[0].type = SDL_HAPTIC_SINE;
          effect[0].periodic.direction.type = SDL_HAPTIC_CARTESIAN;
          effect[0].periodic.direction.dir[0] = 1;
          effect[0].periodic.period = 0;
          effect[0].periodic.magnitude = 0;
          effect[0].periodic.length = SDL_HAPTIC_INFINITY;

          effect[1] = effect[0];
          effect[1].periodic.direction.dir[0] = -1;
          effect[1].periodic.magnitude = 0;

          gamepad.haptic_effects[0] =
              SDL_HapticNewEffect(gamepad.haptic, &(effect[0]));
          gamepad.haptic_effects[1] =
              SDL_HapticNewEffect(gamepad.haptic, &(effect[1]));

          SDL_HapticRunEffect(gamepad.haptic, gamepad.haptic_effects[0], 1);
          SDL_HapticRunEffect(gamepad.haptic, gamepad.haptic_effects[1], 1);
        }

        gamepads.push_back(gamepad);
      }
    }
  }

  XINPUT_VIBRATION welcome_vibration;
  welcome_vibration.wLeftMotorSpeed = 65535;
  welcome_vibration.wRightMotorSpeed = 0;
  koku::XInputSetState(0, &welcome_vibration);
  SDL_Delay(250);
  welcome_vibration.wLeftMotorSpeed = 0;
  welcome_vibration.wRightMotorSpeed = 0;
  koku::XInputSetState(0, &welcome_vibration);
  SDL_Delay(250);
  welcome_vibration.wLeftMotorSpeed = 0;
  welcome_vibration.wRightMotorSpeed = 65535;
  koku::XInputSetState(0, &welcome_vibration);
  SDL_Delay(250);
  welcome_vibration.wLeftMotorSpeed = 0;
  welcome_vibration.wRightMotorSpeed = 0;
  koku::XInputSetState(0, &welcome_vibration);
}

koku::jumper<decltype(XInputEnable)> XInputEnableJumper;
void WINAPI XInputEnable(BOOL enable) {
  debug("");
  if (!enable) {
    XINPUT_VIBRATION vibration;
    vibration.wLeftMotorSpeed = 0;
    vibration.wRightMotorSpeed = 0;
    for (int i = 0; i < 4; ++i)
      koku::XInputSetState(i, &vibration);
  }

  enabled = enable;
}

koku::jumper<decltype(XInputSetState)> XInputSetStateJumper;
DWORD WINAPI XInputSetState(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration) {
  GamepadInitSDL();
  debug("");
  if (dwUserIndex >= gamepads.size())
    return ERROR_DEVICE_NOT_CONNECTED;

  if (enabled && pVibration) {
    if (gamepads[dwUserIndex].haptic != 0) {
      SDL_HapticEffect effect[2];
      memset(&effect, 0, sizeof(SDL_HapticEffect) * 2);

      effect[0].type = SDL_HAPTIC_SINE;
      effect[0].periodic.direction.type = SDL_HAPTIC_CARTESIAN;
      effect[0].periodic.direction.dir[0] = 1;
      effect[0].periodic.period = 0;
      effect[0].periodic.magnitude = pVibration->wLeftMotorSpeed >> 1;
      effect[0].periodic.length = SDL_HAPTIC_INFINITY;

      effect[1] = effect[0];
      effect[1].periodic.direction.dir[0] = -1;
      effect[1].periodic.magnitude = pVibration->wRightMotorSpeed >> 1;

      SDL_HapticUpdateEffect(gamepads[dwUserIndex].haptic,
                             gamepads[dwUserIndex].haptic_effects[0],
                             &(effect[0]));
      SDL_HapticUpdateEffect(gamepads[dwUserIndex].haptic,
                             gamepads[dwUserIndex].haptic_effects[1],
                             &(effect[1]));
    }
  }

  return ERROR_SUCCESS;
}

koku::jumper<decltype(XInputGetState)> XInputGetStateJumper;
DWORD WINAPI XInputGetState(DWORD dwUserIndex, XINPUT_STATE *pState) {
  GamepadInitSDL();
  debug("");
  if (dwUserIndex >= gamepads.size())
    return ERROR_DEVICE_NOT_CONNECTED;

  SDL_JoystickUpdate();
  SDL_GameControllerUpdate();

  if (pState) {
    SDL_GameController *controller = gamepads[dwUserIndex].controller;

    for (int j = 0; j < MAX_XINPUT_BUTTONS; j++) {
      Uint8 result = SDL_GameControllerGetButton(controller, sdlbuttons[j]);
      if (result) {
        pState->Gamepad.wButtons |= xbuttons[j];
      } else {
        pState->Gamepad.wButtons &= ~(xbuttons[j]);
      }
    }

    short ly = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
    short ry =
        SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY);

    pState->Gamepad.sThumbLX =
        SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
    pState->Gamepad.sThumbLY = (ly == 0 ? 0 : -ly - 1);
    pState->Gamepad.sThumbRX =
        SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX);
    pState->Gamepad.sThumbRY = ry == 0 ? 0 : -ry - 1;
    pState->Gamepad.bLeftTrigger =
        SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
    pState->Gamepad.bRightTrigger =
        SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);

    static int dwPacketNumber = 0;
    pState->dwPacketNumber = ++dwPacketNumber;
  }

  return ERROR_SUCCESS;
}

koku::jumper<decltype(XInputGetKeystroke)> XInputGetKeystrokeJumper;
DWORD WINAPI XInputGetKeystroke(DWORD dwUserIndex, DWORD dwReserved,
                                PXINPUT_KEYSTROKE pKeystroke) {
  GamepadInitSDL();
  debug("");
  if (dwUserIndex >= gamepads.size())
    return ERROR_DEVICE_NOT_CONNECTED;

  return ERROR_EMPTY;
}

koku::jumper<decltype(XInputGetCapabilities)> XInputGetCapabilitiesJumper;
DWORD WINAPI XInputGetCapabilities(DWORD dwUserIndex, DWORD dwFlags,
                                   XINPUT_CAPABILITIES *pCapabilities) {
  GamepadInitSDL();
  debug("");
  if (dwUserIndex >= gamepads.size())
    return ERROR_DEVICE_NOT_CONNECTED;

  if (pCapabilities) {
    pCapabilities->Type = XINPUT_DEVTYPE_GAMEPAD;
    pCapabilities->SubType = XINPUT_DEVSUBTYPE_GAMEPAD;
    pCapabilities->Flags = (gamepads[dwUserIndex].haptic != 0)
                               ? /* XINPUT_CAPS_FFB_SUPPORTED */ 0x0001
                               : 0;
    pCapabilities->Gamepad.wButtons = 0xFFFF;
    pCapabilities->Gamepad.bLeftTrigger = 255;
    pCapabilities->Gamepad.bRightTrigger = 255;
    pCapabilities->Gamepad.sThumbLX = 32767;
    pCapabilities->Gamepad.sThumbLY = 32767;
    pCapabilities->Gamepad.sThumbRX = 32767;
    pCapabilities->Gamepad.sThumbRY = 32767;
    if (gamepads[dwUserIndex].haptic != 0) {
      pCapabilities->Vibration.wLeftMotorSpeed = 65535;
      pCapabilities->Vibration.wRightMotorSpeed = 65535;
    } else {
      pCapabilities->Vibration.wLeftMotorSpeed = 0;
      pCapabilities->Vibration.wRightMotorSpeed = 0;
    }
  }
  return ERROR_SUCCESS;
}

koku::jumper<decltype(XInputGetDSoundAudioDeviceGuids)>
    XInputGetDSoundAudioDeviceGuidsJumper;
DWORD WINAPI XInputGetDSoundAudioDeviceGuids(DWORD dwUserIndex,
                                             GUID *pDSoundRenderGuid,
                                             GUID *pDSoundCaptureGuid) {
  GamepadInitSDL();
  debug("");
  if (dwUserIndex >= gamepads.size())
    return ERROR_DEVICE_NOT_CONNECTED;

  /*
   If there is no headset connected to the controller,
   the function also retrieves ERROR_SUCCESS with GUID_NULL as the values
   for pDSoundRenderGuid and pDSoundCaptureGuid.
  */
  if (pDSoundRenderGuid)
    std::memset(pDSoundRenderGuid, 0, sizeof(GUID));

  if (pDSoundCaptureGuid)
    std::memset(pDSoundCaptureGuid, 0, sizeof(GUID));

  return ERROR_SUCCESS;
}

koku::jumper<decltype(XInputGetBatteryInformation)>
    XInputGetBatteryInformationJumper;
DWORD WINAPI
XInputGetBatteryInformation(DWORD dwUserIndex, BYTE devType,
                            XINPUT_BATTERY_INFORMATION *pBatteryInformation) {
  GamepadInitSDL();
  debug("");
  if (dwUserIndex >= gamepads.size())
    return ERROR_DEVICE_NOT_CONNECTED;

  if (pBatteryInformation) {
    if (devType == BATTERY_DEVTYPE_GAMEPAD) {
      pBatteryInformation->BatteryType = BATTERY_TYPE_WIRED;
      pBatteryInformation->BatteryLevel = BATTERY_LEVEL_FULL;
    } else {
      pBatteryInformation->BatteryType = BATTERY_TYPE_DISCONNECTED;
      pBatteryInformation->BatteryLevel = BATTERY_LEVEL_EMPTY;
    }
  }

  return ERROR_SUCCESS;
}

koku::jumper<decltype(XInputGetStateEx)> XInputGetStateExJumper;
DWORD WINAPI XInputGetStateEx(DWORD dwUserIndex, XINPUT_STATE_EX *pState) {
  return koku::XInputGetState(dwUserIndex, (XINPUT_STATE *)pState);
}

void XInputInit(void *handle) {
  if (auto address = (decltype(&XInputEnable))dlsym(handle, "XInputEnable")) {
    debug("found XInputEnable at %p, redirecting it to %p", address,
          &koku::XInputEnable);
    XInputEnableJumper = koku::make_jumper(address, &koku::XInputEnable);
  }

  if (auto address =
          (decltype(&XInputSetState))dlsym(handle, "XInputSetState")) {
    debug("found XInputSetState at %p, redirecting it to %p", address,
          &koku::XInputSetState);
    XInputSetStateJumper = koku::make_jumper(address, koku::XInputSetState);
  }

  if (auto address =
          (decltype(&XInputGetState))dlsym(handle, "XInputGetState")) {
    debug("found XInputGetState at %p, redirecting it to %p", address,
          &koku::XInputGetState);
    XInputGetStateJumper = koku::make_jumper(address, koku::XInputGetState);
  }

  if (auto address =
          (decltype(&XInputGetKeystroke))dlsym(handle, "XInputGetKeystroke")) {
    debug("found XInputGetKeystroke at %p, redirecting it to %p", address,
          &koku::XInputGetKeystroke);
    XInputGetKeystrokeJumper =
        koku::make_jumper(address, koku::XInputGetKeystroke);
  }

  if (auto address = (decltype(&XInputGetCapabilities))dlsym(
          handle, "XInputGetCapabilities")) {
    debug("found XInputGetCapabilities at %p, redirecting it to %p", address,
          &koku::XInputGetCapabilities);
    XInputGetCapabilitiesJumper =
        koku::make_jumper(address, koku::XInputGetCapabilities);
  }

  if (auto address = (decltype(&XInputGetDSoundAudioDeviceGuids))dlsym(
          handle, "XInputGetDSoundAudioDeviceGuids")) {
    debug("found XInputGetDSoundAudioDeviceGuids at %p, redirecting it to %p",
          address, &koku::XInputGetDSoundAudioDeviceGuids);
    XInputGetDSoundAudioDeviceGuidsJumper =
        koku::make_jumper(address, koku::XInputGetDSoundAudioDeviceGuids);
  }

  if (auto address = (decltype(&XInputGetBatteryInformation))dlsym(
          handle, "XInputGetBatteryInformation")) {
    debug("found XInputGetBatteryInformation %p, redirecting it to %p", address,
          &koku::XInputGetBatteryInformation);
    XInputGetBatteryInformationJumper =
        koku::make_jumper(address, koku::XInputGetBatteryInformation);
  }

  if (auto address =
          (decltype(&XInputGetStateEx))dlsym(handle, "XInputGetStateEx")) {
    debug("found XInputGetStateEx at %p, redirecting it to %p", address,
          &koku::XInputGetStateEx);
    XInputGetStateExJumper =
        koku::make_jumper(address, &koku::XInputGetStateEx);
  }
}
} // namespace koku

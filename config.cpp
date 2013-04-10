#include "config.h"
#include <SDL/SDL.h>

//global gamepad struct
SGamepadData gamepad;

//Setup mapping data
const Smapping mapping[20] =
{
	{"XINPUT_GAMEPAD_LEFT_THUMB_X",    &gamepad.left_thumb_x ,  0},
	{"XINPUT_GAMEPAD_LEFT_THUMB_Y",    &gamepad.left_thumb_y ,  0},
	{"XINPUT_GAMEPAD_RIGHT_THUMB_X",   &gamepad.right_thumb_x , 0},
	{"XINPUT_GAMEPAD_RIGHT_THUMB_Y",   &gamepad.right_thumb_y , 0},
	{"XINPUT_GAMEPAD_DPAD_UP",         &gamepad.buttons       , short(XINPUT_GAMEPAD_DPAD_UP)},
	{"XINPUT_GAMEPAD_DPAD_DOWN",       &gamepad.buttons       , short(XINPUT_GAMEPAD_DPAD_DOWN)},
	{"XINPUT_GAMEPAD_DPAD_LEFT",       &gamepad.buttons       , short(XINPUT_GAMEPAD_DPAD_LEFT)},
	{"XINPUT_GAMEPAD_DPAD_RIGHT",      &gamepad.buttons       , short(XINPUT_GAMEPAD_DPAD_RIGHT)},
	{"XINPUT_GAMEPAD_START",           &gamepad.buttons       , short(XINPUT_GAMEPAD_START)},
	{"XINPUT_GAMEPAD_BACK",            &gamepad.buttons       , short(XINPUT_GAMEPAD_BACK)},
	{"XINPUT_GAMEPAD_LEFT_THUMB",      &gamepad.buttons       , short(XINPUT_GAMEPAD_LEFT_THUMB)},
	{"XINPUT_GAMEPAD_RIGHT_THUMB",     &gamepad.buttons       , short(XINPUT_GAMEPAD_RIGHT_THUMB)},
	{"XINPUT_GAMEPAD_LEFT_SHOULDER",   &gamepad.buttons       , short(XINPUT_GAMEPAD_LEFT_SHOULDER)},
	{"XINPUT_GAMEPAD_LEFT_SHOULDER2",  &gamepad.left_shoulder , 0},
	{"XINPUT_GAMEPAD_RIGHT_SHOULDER",  &gamepad.buttons       , short(XINPUT_GAMEPAD_RIGHT_SHOULDER)},
	{"XINPUT_GAMEPAD_RIGHT_SHOULDER2", &gamepad.right_shoulder, 0},
	{"XINPUT_GAMEPAD_A",               &gamepad.buttons       , short(XINPUT_GAMEPAD_A)},
	{"XINPUT_GAMEPAD_B",               &gamepad.buttons       , short(XINPUT_GAMEPAD_B)},
	{"XINPUT_GAMEPAD_X",               &gamepad.buttons       , short(XINPUT_GAMEPAD_X)},
	{"XINPUT_GAMEPAD_Y",               &gamepad.buttons       , short(XINPUT_GAMEPAD_Y)}
};

//Setup default settings, can be overwritten by config file (need to be in same order as "mapping")
//Works for Logitech Rumplepad 2
Ssettings settings[20] =
{
	/*XINPUT_GAMEPAD_LEFT_THUMB_X*/   {'A',  1, short(0xFFFF),  1},
	/*XINPUT_GAMEPAD_LEFT_THUMB_Y*/   {'A',  2, short(0xFFFF), -1},
	/*XINPUT_GAMEPAD_RIGHT_THUMB_X*/  {'A',  3, short(0xFFFF),  1},
	/*XINPUT_GAMEPAD_RIGHT_THUMB_Y*/  {'A',  4, short(0xFFFF), -1},
	/*XINPUT_GAMEPAD_DPAD_UP*/        {'H',  1, short(SDL_HAT_UP),   1},
	/*XINPUT_GAMEPAD_DPAD_DOWN*/	  {'H',  1, short(SDL_HAT_DOWN), 1},
	/*XINPUT_GAMEPAD_DPAD_LEFT*/      {'H',  1, short(SDL_HAT_LEFT), 1},
	/*XINPUT_GAMEPAD_DPAD_RIGHT*/	  {'H',  1, short(SDL_HAT_RIGHT),1},
	/*XINPUT_GAMEPAD_START*/          {'B', 10, short(0xFFFF), 1},
	/*XINPUT_GAMEPAD_BACK*/	          {'B',  9, short(0xFFFF), 1},
	/*XINPUT_GAMEPAD_LEFT_THUMB*/     {'B', 11, short(0xFFFF), 1},
	/*XINPUT_GAMEPAD_RIGHT_THUMB*/	  {'B', 12, short(0xFFFF), 1},
	/*XINPUT_GAMEPAD_LEFT_SHOULDER*/  {'B',  5, short(0xFFFF), 1},
	/*XINPUT_GAMEPAD_LEFT_SHOULDER2*/ {'B',  7, short(0xFFFF), 255}, //scale 1 to 255 L2 and R2 are analog values in XInput
	/*XINPUT_GAMEPAD_RIGHT_SHOULDER*/ {'B',  6, short(0xFFFF), 1},
	/*XINPUT_GAMEPAD_RIGHT_SHOULDER2*/{'B',  8, short(0xFFFF), 255},
	/*XINPUT_GAMEPAD_A*/              {'B',  2, short(0xFFFF), 1},
	/*XINPUT_GAMEPAD_B*/              {'B',  3, short(0xFFFF), 1},
	/*XINPUT_GAMEPAD_X*/              {'B',  1, short(0xFFFF), 1},
	/*XINPUT_GAMEPAD_Y*/              {'B',  4, short(0xFFFF), 1}
};

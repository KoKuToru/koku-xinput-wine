#ifndef KOKU_CONFIG_H
#define KOKU_CONFIG_H
#include "xinput.h"

struct SGamepadData
{
	short left_thumb_x;
	short left_thumb_y;
	short right_thumb_x;
	short right_thumb_y;
	short buttons;
	short left_shoulder;
	short right_shoulder;
};

struct Smapping
{
	const char* name; //name of the map
	short*	addr;     //adress to SGamepadData members
	short	mask;     //masking
};

struct Ssettings
{
	char type;   //A = Axis, B = Button, H = Hat (DPad)
	char id;     //id of A,B or H
	short mask;  //masking
	short scale; //scaling
};

extern const Smapping mapping[20];
extern Ssettings settings[20];
extern SGamepadData gamepad;

#endif

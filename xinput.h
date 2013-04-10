#ifndef KOKU_XINPUT_H
#define KOKU_XINPUT_H
#define WINAPI  __attribute__((__stdcall__))
#define ERROR_SUCCESS 			 0x0
#define ERROR_DEVICE_NOT_CONNECTED 	 0x48f
#define ERROR_EMPTY					0x10D2
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

struct XINPUT_GAMEPAD
{
	unsigned short wButtons;
	unsigned char  bLeftTrigger;
	unsigned char  bRightTrigger;
	short sThumbLX;
	short sThumbLY;
	short xThumbRX;
	short xThumbRY;
};

struct XINPUT_STATE
{
	unsigned dwPacketNumber;
	XINPUT_GAMEPAD Gamepad;
};

struct XINPUT_BATTERY_INFORMATION
{
	unsigned char BatteryType;
	unsigned char BatteryLevel;
};

struct XINPUT_VIBRATION
{
	unsigned short wLeftMotorSpeed;
	unsigned short wRightMotorSpeed;
};

struct XINPUT_CAPABILITIES
{
	unsigned char  Type;
	unsigned char  SubType;
	unsigned short Flags;
	XINPUT_GAMEPAD   Gamepad;
	XINPUT_VIBRATION Vibration;
};

struct XINPUT_KEYSTROKE
{
	unsigned short VirtualKey;
	unsigned short Unicode;
	unsigned short Flags;
	unsigned char  UserIndex;
	unsigned char  HidCode;
};

struct GUID
{
  unsigned        Data1;
  unsigned short  Data2;
  unsigned short  Data3;
  unsigned char   Data4[8];
};

void WINAPI XInputEnable(bool enable);
unsigned WINAPI XInputGetAudioDeviceIds(unsigned dwUserIndex, short* pRenderDeviceId, unsigned *pRenderCount, short* pCaptureDeviceId, unsigned *pCaptureCount);
unsigned WINAPI XInputGetBatteryInformation(unsigned dwUserIndex, char devType, XINPUT_BATTERY_INFORMATION *pBatteryInformation);
unsigned WINAPI XInputGetCapabilities(unsigned dwUserIndex, unsigned dwFlags, XINPUT_CAPABILITIES *pCapabilities);
unsigned WINAPI XInputGetDSoundAudioDeviceGuids(unsigned dwUserIndex, GUID* pDSoundRenderGuid, GUID* pDSoundCaptureGuid);
unsigned WINAPI XInputGetKeystroke(unsigned dwUserIndex, unsigned dwReserved, XINPUT_KEYSTROKE* pKeystroke);
unsigned WINAPI XInputGetState(unsigned dwUserIndex, XINPUT_STATE *pState);
unsigned WINAPI XInputSetState(unsigned dwUserIndex, XINPUT_VIBRATION *pVibration);
#endif

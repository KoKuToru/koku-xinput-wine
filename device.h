/*

 Pretty much for
 http://msdn.microsoft.com/en-us/library/windows/desktop/ee417014(v=vs.85).aspx

 Some games check this way for XInput support

 The most simple way would be to fake the values wine returns..
 Sadly it's not that easy.. because wine doesn't return any devices..

 Means we need todo some dark magic, this code will be ugly..
 */

 #ifndef KOKU_DEVICE_H
 #define KOKU_DEVICE_H

 #include "xinput.h" //for WINAPI macro

struct VARIANT
{
	unsigned short vt;
	short          wReserved1;
	short          wReserved2;
	short          wReserved3;
	const short    *STR;
};

void DeviceInit(void* handle);
bool WINAPI Devices_Get(void* pDevices, short* wszName, unsigned lFlags, VARIANT* pVal, void* o1, void* o2);
void WINAPI Devices_Release(void* pDevices);
void* WINAPI EnumDevices_Next(void* pEnumDevices, unsigned a, unsigned b, void** pDevices, unsigned* uReturned);
void* WINAPI WbemServices_CreateInstanceEnum(void* pIWbemServices, short* bstrClassName, unsigned null1, void* null2, void* pEnumDevices);
void* WINAPI CreateInstanceEnum(void* pIWbemServices, short* bstrClassName, unsigned null1, void* null2, void* pEnumDevices);
void* WINAPI CoSetProxyBlanket(void* pProxy, unsigned dwAuthnSvc, unsigned dwAuthzSvc, void* pServerPrincName, unsigned dwAuthnLevel, unsigned dwImpLevel, void* pAuthInfo, unsigned dwCapabilities);
 #endif

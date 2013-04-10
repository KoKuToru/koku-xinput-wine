#include "device.h"
#include "main.h"
#include <cstring>
#include <new>
#include <string>
using namespace std;

std::string check_bstrNamespace = "\\\\.\\root\\cimv2";
std::string check_bstrClassName = "Win32_PNPEntity";
std::string check_bstrDeviceID  = "DeviceID";

const short wine_gamepad[] = {'V','I','D','_','3','E','D','9','&',
							  'P','I','D','_','9','E','5','7','&',
							  'I','G','_','0','0'}; //you can get VID/PID from wine-source

//what this does ? well.. hard to explain, please don't mind as long as it works
long CoSetProxyBlanket_addr = 0;
char  CoSetProxyBlanket_hook[sizeof(Sjmp)];
long CreateInstanceEnum_addr = 0;
long CreateInstanceEnum_original;
long Next_addr = 0;
long Next_original;

void DeviceInit(void* handle)
{
	//hook functions
	long addr = long(dlsym(handle, "CoSetProxyBlanket"));

	if (addr != 0)
	{
		//backup data
		CoSetProxyBlanket_addr = addr;
		memcpy(CoSetProxyBlanket_hook, (void*)CoSetProxyBlanket_addr, sizeof(Sjmp));

		long addr_start = (addr - PAGESIZE-1) & ~(PAGESIZE-1);
		long addr_end   = (addr + PAGESIZE-1) & ~(PAGESIZE-1);
		mprotect((void*)addr_start, addr_end-addr_start , PROT_READ|PROT_WRITE|PROT_EXEC);

		new ((void*)addr) Sjmp((void*)CoSetProxyBlanket);
	}
}

void* WINAPI CoSetProxyBlanket(void* pProxy, unsigned dwAuthnSvc, unsigned dwAuthzSvc, void* pServerPrincName, unsigned dwAuthnLevel, unsigned dwImpLevel, void* pAuthInfo, unsigned dwCapabilities)
{
	//disable hook
	memcpy((void*)CoSetProxyBlanket_addr, CoSetProxyBlanket_hook, sizeof(Sjmp));
	//call original
	void* result = ((decltype(&CoSetProxyBlanket))CoSetProxyBlanket_addr)(pProxy, dwAuthnSvc, dwAuthzSvc, pServerPrincName, dwAuthnLevel, dwImpLevel, pAuthInfo, dwCapabilities);
	//enable hook
	new ((void*)CoSetProxyBlanket_addr) Sjmp((void*)CoSetProxyBlanket);
	//overwrite the function-table that CreateInstanceEnum goes to our function
	long pProxy_func = *((long*)pProxy);
	long pProxy_func_createinstanceenum = pProxy_func+0x48;

	long addr_start = (pProxy_func_createinstanceenum - PAGESIZE-1) & ~(PAGESIZE-1);
	long addr_end   = (pProxy_func_createinstanceenum + PAGESIZE-1) & ~(PAGESIZE-1);
	mprotect((void*)addr_start, addr_end-addr_start , PROT_READ|PROT_WRITE|PROT_EXEC);

	if (*((void**)(pProxy_func_createinstanceenum)) != (void*)CreateInstanceEnum)
	{
		CreateInstanceEnum_original = *((long*)(pProxy_func_createinstanceenum));
		CreateInstanceEnum_addr = pProxy_func_createinstanceenum;
	}

	*((void**)(pProxy_func_createinstanceenum)) = (void*)CreateInstanceEnum;
}

void* WINAPI CreateInstanceEnum(void* pIWbemServices, short* bstrClassName, unsigned null1, void* null2, void* pEnumDevices)
{
	//check, uhm i have no idea how to work with unicode..
	string bstrClassName_s;
	for(int i = 0; bstrClassName[i] != 0; ++i)
	{
		bstrClassName_s += bstrClassName[i];
	}

	//call original
	void* result = ((decltype(&CreateInstanceEnum))CreateInstanceEnum_original)(pIWbemServices, bstrClassName, null1, null2, pEnumDevices);

	if (bstrClassName_s != check_bstrClassName)
	{

		return result;
	}

	//overwrite the function-table that Next goes to our function
	long pEnumDevices_func = **((long**)pEnumDevices);
	long pEnumDevices_func_next = pEnumDevices_func+0x10;

	long addr_start = (pEnumDevices_func_next - PAGESIZE-1) & ~(PAGESIZE-1);
	long addr_end   = (pEnumDevices_func_next + PAGESIZE-1) & ~(PAGESIZE-1);
	mprotect((void*)addr_start, addr_end-addr_start , PROT_READ|PROT_WRITE|PROT_EXEC);

	if (*((void**)(pEnumDevices_func_next)) != (void*)EnumDevices_Next)
	{
		Next_addr     = pEnumDevices_func_next;
		Next_original = *((long*)(pEnumDevices_func_next));
	}

	*((void**)(pEnumDevices_func_next)) = (void*)EnumDevices_Next;

	return result;
}

void* WINAPI EnumDevices_Next(void* pEnumDevices, unsigned a, unsigned b, void** pDevices, unsigned* uReturned)
{
	//call original
	void* result = ((decltype(&EnumDevices_Next))Next_original)(pEnumDevices, a, b, pDevices, uReturned);

	if (uReturned == 0)
	{
		//restore original
		*((void**)(Next_addr)) = (void*)Next_original;

		//end reach add our own stuff ;)
		*uReturned = 1;

		//Very ugly stuff will happen now:
		*pDevices = (void*)(new char[1024]);
		*((void**)*pDevices) = (void*)(unsigned(*pDevices)+1);

		long pDevices_func = **((long**)pDevices);
		long pDevices_func_get = pDevices_func+0x10;

		long addr_start = (pDevices_func_get - PAGESIZE-1) & ~(PAGESIZE-1);
		long addr_end   = (pDevices_func_get + PAGESIZE-1) & ~(PAGESIZE-1);
		mprotect((void*)addr_start, addr_end-addr_start , PROT_READ|PROT_WRITE|PROT_EXEC);

		*((void**)(pDevices_func_get)) = (void*)Devices_Get;
		pDevices_func_get = pDevices_func+0x08;
		*((void**)(pDevices_func_get)) = (void*)Devices_Release;
	}
	return result;
}

bool WINAPI Devices_Get(void* pDevices, short* wszName, unsigned lFlags, VARIANT* pVal, void* o1, void* o2)
{
	//check, uhm i have no idea how to work with unicode..
	string wszName_s;
	for(int i = 0; wszName[i] != 0; ++i)
	{
		wszName_s += wszName[i];
	}

	if (check_bstrDeviceID != wszName_s)
	{
		//uhm nothing
		return 1; //not ERROR_SUCCESS
	}

	pVal->vt = /*VT_BSTR*/8;
	pVal->STR = wine_gamepad;

	return 0;
}

void WINAPI Devices_Release(void* pDevices)
{
	delete[] (char*)pDevices;
}

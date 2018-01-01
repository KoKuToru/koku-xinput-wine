#include <cstddef>

#include <dinput.h>
#include <math.h>
#include <stdio.h>
#include <windows.h>

#include <oleauto.h>
#include <wbemidl.h>
// #include <wmsstd.h>

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x)                                                        \
  if (x != NULL) {                                                             \
    x->Release();                                                              \
    x = NULL;                                                                  \
  }
#endif

//-----------------------------------------------------------------------------
// Enum each PNP device using WMI and check each device ID to see if it contains
// "IG_" (ex. "VID_045E&PID_028E&IG_00").  If it does, then it's an XInput
// device Unfortunately this information can not be found by just using
// DirectInput
//-----------------------------------------------------------------------------
BOOL IsXInputDevice(const GUID *pGuidProductFromDirectInput) {
  IWbemLocator *pIWbemLocator = NULL;
  IEnumWbemClassObject *pEnumDevices = NULL;
  IWbemClassObject *pDevices[20] = {0};
  IWbemServices *pIWbemServices = NULL;
  BSTR bstrNamespace = NULL;
  BSTR bstrDeviceID = NULL;
  BSTR bstrClassName = NULL;
  DWORD uReturned = 0;
  bool bIsXinputDevice = false;
  UINT iDevice = 0;
  VARIANT var;
  HRESULT hr;

  // CoInit if needed
  hr = CoInitialize(NULL);
  bool bCleanupCOM = SUCCEEDED(hr);

  // Create WMI
  hr = CoCreateInstance(__uuidof(WbemLocator), NULL, CLSCTX_INPROC_SERVER,
                        __uuidof(IWbemLocator), (LPVOID *)&pIWbemLocator);
  if (FAILED(hr) || pIWbemLocator == NULL)
    goto LCleanup;

  bstrNamespace = SysAllocString(L"\\\\.\\root\\cimv2");
  if (bstrNamespace == NULL)
    goto LCleanup;
  bstrClassName = SysAllocString(L"Win32_PNPEntity");
  if (bstrClassName == NULL)
    goto LCleanup;
  bstrDeviceID = SysAllocString(L"DeviceID");
  if (bstrDeviceID == NULL)
    goto LCleanup;

  // Connect to WMI
  hr = pIWbemLocator->ConnectServer(bstrNamespace, NULL, NULL, 0L, 0L, NULL,
                                    NULL, &pIWbemServices);
  if (FAILED(hr) || pIWbemServices == NULL)
    goto LCleanup;

  // Switch security level to IMPERSONATE.
  CoSetProxyBlanket(pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
                    RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL,
                    EOAC_NONE);

  hr =
      pIWbemServices->CreateInstanceEnum(bstrClassName, 0, NULL, &pEnumDevices);
  if (FAILED(hr) || pEnumDevices == NULL)
    goto LCleanup;

  // Loop over all devices
  for (;;) {
    // Get 20 at a time
    hr = pEnumDevices->Next(10000, 20, pDevices, &uReturned);
    if (FAILED(hr))
      goto LCleanup;
    if (uReturned == 0)
      break;

    for (iDevice = 0; iDevice < uReturned; iDevice++) {
      // For each device, get its device ID
      hr = pDevices[iDevice]->Get(bstrDeviceID, 0L, &var, NULL, NULL);
      if (SUCCEEDED(hr) && var.vt == VT_BSTR && var.bstrVal != NULL) {
        // Check if the device ID contains "IG_".  If it does, then it's an
        // XInput device This information can not be found from DirectInput
        if (wcsstr(var.bstrVal, L"IG_")) {
          // If it does, then get the VID/PID from var.bstrVal
          DWORD dwPid = 0, dwVid = 0;
          WCHAR *strVid = wcsstr(var.bstrVal, L"VID_");
          if (strVid && swscanf(strVid, L"VID_%4X", &dwVid) != 1)
            dwVid = 0;
          WCHAR *strPid = wcsstr(var.bstrVal, L"PID_");
          if (strPid && swscanf(strPid, L"PID_%4X", &dwPid) != 1)
            dwPid = 0;

          // Compare the VID/PID to the DInput device
          DWORD dwVidPid = MAKELONG(dwVid, dwPid);
          printf("Found device %04x\n", pGuidProductFromDirectInput->Data1);
          if (dwVidPid == pGuidProductFromDirectInput->Data1) {
            printf("Found XINPUT device %04x\n",
                   pGuidProductFromDirectInput->Data1);
            bIsXinputDevice = true;
            goto LCleanup;
          }
        }
      }
      SAFE_RELEASE(pDevices[iDevice]);
    }
  }

LCleanup:
  if (bstrNamespace)
    SysFreeString(bstrNamespace);
  if (bstrDeviceID)
    SysFreeString(bstrDeviceID);
  if (bstrClassName)
    SysFreeString(bstrClassName);
  for (iDevice = 0; iDevice < 20; iDevice++)
    SAFE_RELEASE(pDevices[iDevice]);
  SAFE_RELEASE(pEnumDevices);
  SAFE_RELEASE(pIWbemLocator);
  SAFE_RELEASE(pIWbemServices);

  if (bCleanupCOM)
    CoUninitialize();

  return bIsXinputDevice;
}

LPDIRECTINPUT8 g_pDI;
LPDIRECTINPUTDEVICE8 g_pJoystick;

//-----------------------------------------------------------------------------
// Name: EnumJoysticksCallback()
// Desc: Called once for each enumerated joystick. If we find one, create a
//       device interface on it so we can play with it.
//-----------------------------------------------------------------------------
BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE *pdidInstance,
                                    VOID *pContext) {
  HRESULT hr;

  if (!IsXInputDevice(&pdidInstance->guidProduct))
    return DIENUM_CONTINUE;

  // Device is verified not XInput, so add it to the list of DInput devices

  hr = g_pDI->CreateDevice(pdidInstance->guidInstance, &g_pJoystick, NULL);
  return DIENUM_CONTINUE;
}

BOOL CALLBACK EnumAxesCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef) {
  DIPROPRANGE range;
  range.diph.dwSize = sizeof(DIPROPRANGE);
  range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
  range.diph.dwHow = DIPH_BYID;
  range.diph.dwObj = lpddoi->dwType;
  range.lMin = -1000;
  range.lMax = +1000;

  HRESULT hr = g_pJoystick->SetProperty(DIPROP_RANGE, &range.diph);
  if (FAILED(hr))
    return DIENUM_STOP;

  return DIENUM_CONTINUE;
}

void shutdown() {
  if (g_pJoystick)
    g_pJoystick->Release();

  if (g_pDI)
    g_pDI->Release();
}

int main() {
  HRESULT hr;

  hr = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION,
                          IID_IDirectInput8, (void **)&g_pDI, NULL);
  if (FAILED(hr))
    return 10;

  printf("DirectInput initialized.\n");

  g_pDI->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, NULL,
                     DIEDFL_ATTACHEDONLY);

  if (!g_pJoystick) {
    printf("Couldn't find a joystick.\n");
    shutdown();
    return 10;
  }

  printf("Joystick created.\n");

  if (FAILED(hr = g_pJoystick->SetDataFormat(&c_dfDIJoystick2))) {
    printf("Failed to set joystick data format.\n");
    shutdown();
    return 10;
  }

  WNDCLASS wc;

  wc.style = 0;
  wc.lpfnWndProc = DefWindowProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = GetModuleHandle(NULL);
  wc.hIcon = NULL;
  wc.hCursor = NULL;
  wc.hbrBackground = NULL;
  wc.lpszMenuName = NULL;
  wc.lpszClassName = "foo";

  HWND hwnd = CreateWindow((LPCTSTR)RegisterClass(&wc), "", WS_POPUP, 0, 0, 0,
                           0, NULL, NULL, wc.hInstance, NULL);

  if (FAILED(hr = g_pJoystick->SetCooperativeLevel(
                 hwnd, DISCL_EXCLUSIVE | DISCL_BACKGROUND))) {
    printf("Failed to set cooperative level.\n");
    shutdown();
    return 10;
  }

  if (FAILED(
          hr = g_pJoystick->EnumObjects(EnumAxesCallback, NULL, DIDFT_AXIS))) {
    printf("Failed to enumerate axes.\n");
    shutdown();
    return 10;
  }

  printf("All OK!\n");

  bool acq = false;

  int winfx = 800 << 16;
  int winfy = 600 << 16;
  int winx = 0;
  int winy = 0;
  int acscale = 0;
  bool left = false, right = false, abs = false;
  bool absmode = false;

  for (;;) {
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    Sleep(10);

    hr = S_OK;
    if (!acq) {
      hr = g_pJoystick->Acquire();
      acq = SUCCEEDED(hr);
      if (FAILED(hr)) {
        printf("failed to acquire! %08x\n", hr);
      }
    }
    if (SUCCEEDED(hr)) {
      hr = g_pJoystick->Poll();
      if (FAILED(hr))
        printf("poll failed!\n");
    }
    if (FAILED(hr)) {
      hr = g_pJoystick->Acquire();
      while (hr == DIERR_INPUTLOST)
        hr = g_pJoystick->Acquire();
      if (FAILED(hr))
        acq = false;
      else
        acq = true;
    }

    if (SUCCEEDED(hr)) {
      DIJOYSTATE2 js;
      HRESULT hr = g_pJoystick->GetDeviceState(sizeof js, &js);

      if (FAILED(hr))
        continue;

#if 1
      if (js.lX != 500 && js.lY != 500) {
        POINT pt;
        GetCursorPos(&pt);

        winfx = ((winfx + 0x8000) & 0xffff) + (pt.x << 16) - 0x8000;
        winfy = ((winfy + 0x8000) & 0xffff) + (pt.y << 16) - 0x8000;

        winfx += js.lX * acscale;
        winfy += js.lY * acscale;

        acscale += pow(hypot(js.lX, js.lY) / 1000.0, 2.0) * 1000.0 / 16.0;

        winx = (winfx + 0x8000) >> 16;
        winy = (winfy + 0x8000) >> 16;

        if (absmode) {
          winx = 1600 * (js.lX + 1000) / 2001;
          winy = 1200 * (js.lY + 1000) / 2001;
        }

        if (winfx < (0 << 16))
          winfx = 0 << 16;
        if (winfx > (1600 << 16))
          winfx = 1600 << 16;
        if (winfy < (0 << 16))
          winfy = 0 << 16;
        if (winfy > (1200 << 16))
          winfy = 1200 << 16;

        //      SetCursorPos(winx + 300*js.lX/1000, winy +
        // 300*js.lY/1000);

        SetCursorPos(winx, winy);
      }

      bool nowleft =
          (js.rgbButtons[1] | js.rgbButtons[10] | js.rgbButtons[11]) >= 0x80 ||
          LOWORD(js.rgdwPOV[0]) != 0xffff;
      bool nowright = js.rgbButtons[0] >= 0x80;
      bool nowabs = js.rgbButtons[2] >= 0x80;

      if (fabs(js.lX) < 128 && fabs(js.lY) < 128)
        acscale = 128;

      if (left && !nowleft)
        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, NULL);
      if (!left && nowleft)
        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, NULL);
      if (right && !nowright)
        mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, NULL);
      if (!right && nowright)
        mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, NULL);

      if (!abs && nowabs)
        absmode = !absmode;

      left = nowleft;
      right = nowright;
      abs = nowabs;
#else
      bool nowleft = (js.rgbButtons[0] | js.rgbButtons[1] | js.rgbButtons[2] |
                      js.rgbButtons[3]) >= 0x80;

      if (nowleft && !left) {
        keybd_event(VK_NEXT, 0, 0, 0);
      } else if (!nowleft && left) {
        keybd_event(VK_NEXT, 0, KEYEVENTF_KEYUP, 0);
      }
      left = nowleft;
#endif
    }
  }
  shutdown();
  return 0;
}

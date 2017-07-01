#ifndef KOKU_XINPUT_H
#define KOKU_XINPUT_H

#define _FORCENAMELESSUNION
#define CINTERFACE
#define INITGUID

#include <xinput.h>

#define XINPUT_CAPS_FFB_SUPPORTED   0x0001

DWORD WINAPI XInputGetAudioDeviceIds
(
    DWORD   dwUserIndex,        // Index of the gamer associated with the device
    LPWSTR  pRenderDeviceId,    // Windows Core Audio device ID string for render (speakers)
    UINT*   pRenderCount,       // Size of render device ID string buffer (in wide-chars)
    LPWSTR  pCaptureDeviceId,   // Windows Core Audio device ID string for capture (microphone)
    UINT*   pCaptureCount       // Size of capture device ID string buffer (in wide-chars)
);

#endif

#include "main.h"

#define _FORCENAMELESSUNION
#define CINTERFACE
#define INITGUID

#include <objbase.h>
#include <wbemcli.h>

namespace koku {
unsigned short wine_gamepad[] = {'V', 'I', 'D', '_', '3', 'E', 'D', '9',
                                 '&', 'P', 'I', 'D', '_', '9', 'E', '5',
                                 '7', '&', 'I', 'G', '_', '0', '0'};
unsigned short x360_gamepad[] = {'V', 'I', 'D', '_', '0', '4', '5', 'E',
                                 '&', 'P', 'I', 'D', '_', '0', '2', '8',
                                 'E', '&', 'I', 'G', '_', '0', '0'};

HRESULT STDMETHODCALLTYPE IWbemClassObject_Get(IWbemClassObject *This,
                                               LPCWSTR wszName, LONG lFlags,
                                               VARIANT *pVal, CIMTYPE *pType,
                                               LONG *plFlavor) {
  debug("");

  auto wszNameLen = ((uint32_t *)wszName)[-1];
  if (std::memcmp(wszName, u"DeviceID", wszNameLen) == 0) {
    debug("DeviceID");

    pVal->vt = VT_BSTR;
    pVal->bstrVal = x360_gamepad;

    return 0;
  }

  return 1;
}

ULONG STDMETHODCALLTYPE IWbemClassObject_Release(IWbemClassObject *This) {
  debug("");
  delete This->lpVtbl;
  delete This;
  return 0;
}

koku::jumper<std::decay<decltype(*IEnumWbemClassObjectVtbl::Next)>::type>
    IEnumWbemClassObject_Next_Jumper;
HRESULT STDMETHODCALLTYPE IEnumWbemClassObject_Next_Koku(
    IEnumWbemClassObject *This, LONG lTimeout, ULONG uCount,
    IWbemClassObject **apObjects, ULONG *puReturned) {
  debug("");

  if (uCount == 0)
    return WBEM_S_FALSE;

  if (puReturned == nullptr || *puReturned != 0)
    return WBEM_E_INVALID_PARAMETER;

  auto wbemClassObject = new IWbemClassObject{};
  wbemClassObject->lpVtbl = new IWbemClassObjectVtbl{};

  wbemClassObject->lpVtbl->Get = &IWbemClassObject_Get;
  wbemClassObject->lpVtbl->Release = &IWbemClassObject_Release;

  *puReturned = 1;
  *apObjects = wbemClassObject;

  return WBEM_S_FALSE;
}

koku::jumper<std::decay<decltype(*IWbemServicesVtbl::CreateInstanceEnum)>::type>
    IWbemServices_CreateInstanceEnum_Jumper;
HRESULT STDMETHODCALLTYPE IWbemServices_CreateInstanceEnum_Koku(
    IWbemServices *This, const BSTR strFilter, LONG lFlags, IWbemContext *pCtx,
    IEnumWbemClassObject **ppEnum) {
  debug("");
  auto result = IWbemServices_CreateInstanceEnum_Jumper(This, strFilter, lFlags,
                                                        pCtx, ppEnum);

  auto strFilterLen = ((uint32_t *)strFilter)[-1];
  if (std::memcmp(strFilter, u"Win32_PNPEntity", strFilterLen) == 0) {
    auto enumWbemClassObject = *ppEnum;
    if (enumWbemClassObject != nullptr &&
        IEnumWbemClassObject_Next_Jumper.src !=
            enumWbemClassObject->lpVtbl->Next) {
      IEnumWbemClassObject_Next_Jumper = koku::make_jumper(
          enumWbemClassObject->lpVtbl->Next, &IEnumWbemClassObject_Next_Koku);
      debug("found IEnumWbemClassObject_Next at %p, redirecting it to %p",
            enumWbemClassObject->lpVtbl->Next, &IEnumWbemClassObject_Next_Koku);
    }
  }

  return result;
}

koku::jumper<std::decay<decltype(*IWbemLocatorVtbl::ConnectServer)>::type>
    IWbemLocator_ConnectServer_Jumper;
HRESULT STDMETHODCALLTYPE IWbemLocator_ConnectServer_Koku(
    IWbemLocator *This, const BSTR strNetworkResource, const BSTR strUser,
    const BSTR strPassword, const BSTR strLocale, LONG lSecurityFlags,
    const BSTR strAuthority, IWbemContext *pCtx, IWbemServices **ppNamespace) {
  debug("");
  auto result = IWbemLocator_ConnectServer_Jumper(
      This, strNetworkResource, strUser, strPassword, strLocale, lSecurityFlags,
      strAuthority, pCtx, ppNamespace);

  auto wbemServices = *ppNamespace;
  if (wbemServices != nullptr &&
      IWbemServices_CreateInstanceEnum_Jumper.src !=
          wbemServices->lpVtbl->CreateInstanceEnum) {
    IWbemServices_CreateInstanceEnum_Jumper =
        koku::make_jumper(wbemServices->lpVtbl->CreateInstanceEnum,
                          &IWbemServices_CreateInstanceEnum_Koku);
    debug("found IWbemServices_CreateInstanceEnum at %p, redirecting it to %p",
          wbemServices->lpVtbl->CreateInstanceEnum,
          &IWbemServices_CreateInstanceEnum_Koku);
  }

  return result;
}

koku::jumper<decltype(CoCreateInstance)> CoCreateInstance_Jumper;
HRESULT WINAPI CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter,
                                DWORD dwClsContext, REFIID iid, LPVOID *ppv) {
  debug("");
  auto result =
      CoCreateInstance_Jumper(rclsid, pUnkOuter, dwClsContext, iid, ppv);

  if (std::memcmp(&iid, &IID_IWbemLocator, sizeof(iid)) == 0) {
    auto wbemLocator = *(IWbemLocator **)ppv;
    if (wbemLocator != nullptr &&
        IWbemLocator_ConnectServer_Jumper.src !=
            wbemLocator->lpVtbl->ConnectServer) {
      IWbemLocator_ConnectServer_Jumper = koku::make_jumper(
          wbemLocator->lpVtbl->ConnectServer, &IWbemLocator_ConnectServer_Koku);
      debug("found IWbemLocator_ConnectServer at %p, redirecting it to %p",
            wbemLocator->lpVtbl->ConnectServer,
            &IWbemLocator_ConnectServer_Koku);
    }
  }

  return result;
}

void DeviceInit(void *handle) {
  if (auto address =
          (decltype(&CoCreateInstance))dlsym(handle, "CoCreateInstance")) {
    CoCreateInstance_Jumper =
        koku::make_jumper(address, &koku::CoCreateInstance);
    debug("found CoCreateInstance at %p, redirecting it to %p", address,
          &koku::CoCreateInstance);
  }
}
} // namespace koku

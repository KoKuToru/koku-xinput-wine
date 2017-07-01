#include "main.h"

#include <string>

extern "C" void *wine_dll_load(const char *filename, char *error, int errorsize,
                               int *file_exists) {
  auto result = ((decltype(&wine_dll_load))dlsym(RTLD_NEXT, "wine_dll_load"))(
      filename, error, errorsize, file_exists);
  debug("wine_dll_load(%s, ...)", filename);

  if (std::string{"xinput1_3.dll"} == filename ||
      std::string{"xinput9_1_0.dll"} == filename ||
      std::string{"xinput1_4.dll"} == filename) {
    koku::XInputInit(result);
  }

  if (std::string{"ole32.dll"} == filename) {
    koku::DeviceInit(result);
  }

  return result;
}

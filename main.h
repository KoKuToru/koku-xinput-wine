#ifndef KOKU_MAIN_H
#define KOKU_MAIN_H

#include <dlfcn.h>

#include "jumper.h"

#define debug(message, ...)                                                    \
  if (getenv("KOKU_XINPUT_DEBUG") != nullptr)                                  \
    std::printf("koku-xinput-wine: [%d] %s:%d %s " message "\n", getpid(),     \
                __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);

namespace koku {
void XInputInit(void *handle);
void DeviceInit(void *handle);
}; // namespace koku

#endif // KOKU_MAIN_H

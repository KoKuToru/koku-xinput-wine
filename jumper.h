#ifndef KOKU_JUMPER_H
#define KOKU_JUMPER_H

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include <sys/mman.h>
#include <unistd.h>

namespace koku {
template <typename F> struct jumper {
  F *src = nullptr;
  F *dst = nullptr;

#if UINTPTR_MAX == UINT64_MAX
  std::array<uint8_t, 12> header = {{0x48, 0xb8, 0x00, 0x00, 0x00, 0x00,
                                     0x00, 0x00, 0x00, 0x00, 0xff, 0xe0}};
#elif UINTPTR_MAX == UINT32_MAX
  std::array<uint8_t, 5> header = {{0xe9, 0x00, 0x00, 0x00, 0x00}};
#else
#error "Unsupported architecture"
#endif

  jumper() = default;
  jumper(jumper const &) = default;
  jumper(jumper &&) = default;
  jumper &operator=(jumper const &) = default;
  jumper &operator=(jumper &&) = default;

  jumper(F *src, F *dst) : src(src), dst(dst) {
    assert(src != nullptr);
    assert(dst != nullptr);

#if UINTPTR_MAX == UINT64_MAX
    std::memcpy(&header[2], &dst, 8);
#elif UINTPTR_MAX == UINT32_MAX
    auto distance = (uintptr_t)std::abs((intptr_t)src - (intptr_t)dst);
    assert(distance <= INT32_MAX);
    auto rel = (uintptr_t)dst - ((uintptr_t)src + header.size());
    std::memcpy(&header[1], &rel, 4);
#else
#error "Unsupported architecture"
#endif

    unprotect();
    install();
  }

  void unprotect() {
    assert(src != nullptr);
    auto pagesize = sysconf(_SC_PAGESIZE);
    auto address = (void *)((uintptr_t)src & ~(pagesize - 1));
    auto result =
        mprotect(address, header.size(), PROT_READ | PROT_WRITE | PROT_EXEC);
    assert(result == 0);
  }

  void install() {
    assert(src != nullptr);
    auto tmp = header;
    std::memcpy(&header[0], (const void *)src, header.size());
    std::memcpy((void *)src, &tmp[0], tmp.size());
  }

  void uninstall() {
    assert(src != nullptr);
    auto tmp = header;
    std::memcpy(&header[0], (const void *)src, header.size());
    std::memcpy((void *)src, &tmp[0], tmp.size());
  }

  struct scoped_uninstall {
    jumper &jmp;
    scoped_uninstall(jumper &jmp) : jmp(jmp) { jmp.uninstall(); }
    ~scoped_uninstall() { jmp.install(); }
  };

  template <typename... As>
  auto operator()(As &&... as) -> decltype(src(std::forward<As>(as)...)) {
    auto _ = scoped_uninstall(*this);
    return src(std::forward<As>(as)...);
  }
};

template <typename F> jumper<F> make_jumper(F *src, F *dst) {
  return jumper<F>{src, dst};
}
} // namespace koku

#endif // KOKU_JUMPER_H

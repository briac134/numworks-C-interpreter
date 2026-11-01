extern "C" {
#include <eadk.h>
}

#include <cstdio>

namespace {
constexpr eadk_keyboard_state_t key(eadk_key_t value) {
  return static_cast<eadk_keyboard_state_t>(1) << value;
}

const eadk_keyboard_state_t kScript[] = {
    0, key(eadk_key_ok),   0, key(eadk_key_alpha), 0, key(eadk_key_exe),
    0, key(eadk_key_home), 0,
};
std::size_t script_index = 0;
eadk_keyboard_state_t current_keys = 0;
} // namespace

extern "C" uint32_t _eadk_keyboard_scan_low() {
  return static_cast<uint32_t>(current_keys);
}

extern "C" uint32_t _eadk_keyboard_scan_high() {
  return static_cast<uint32_t>(current_keys >> 32);
}

extern "C" void _eadk_keyboard_scan_do_scan() {
  if (script_index >= sizeof(kScript) / sizeof(kScript[0]))
    current_keys = key(eadk_key_home);
  else
    current_keys = kScript[script_index++];
}

extern "C" void eadk_display_push_rect_uniform(eadk_rect_t, eadk_color_t) {}

extern "C" void eadk_display_draw_string(const char *text, eadk_point_t point,
                                         bool, eadk_color_t, eadk_color_t) {
  std::printf("DRAW[%u,%u]: %s\n", point.x, point.y, text);
}

extern "C" void eadk_timing_msleep(uint32_t) {}

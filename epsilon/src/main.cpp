#include "numworkscc/core.hpp"

extern "C" {
#include <eadk.h>
}

#include <algorithm>
#include <array>
#include <cstdint>
#include <string>

#if defined(PLATFORM_DEVICE)
extern "C" {
#include "storage.h"
}
#endif

#if defined(PLATFORM_DEVICE)
extern const char eadk_app_name[]
    __attribute__((section(".rodata.eadk_app_name"))) = "NW Compiler";
extern const std::uint32_t eadk_api_level
    __attribute__((section(".rodata.eadk_api_level"))) = 0;
#endif

namespace {
// The colors are messed up, I didn't want to replace every color names throughout the file
// (The original theme was black and green, I replaced it with a more Numworks-like one)
constexpr eadk_color_t kBlack = 0xf7be;
constexpr eadk_color_t kWhite = 0x0000;
constexpr eadk_color_t kGreen = 0xfda6;
constexpr eadk_color_t kRed = 0xf800;
constexpr std::size_t kMaxSource = 512;

constexpr std::uint16_t kCharacterWidth = 7;
constexpr std::uint16_t kLineNumberX = 4;
constexpr std::uint16_t kSourceX = 24;
constexpr std::size_t kVisibleCharacters = (320 - kSourceX) / kCharacterWidth;
constexpr std::size_t kScriptCount = 5;

#if defined(PLATFORM_DEVICE)
std::string storage_name(std::size_t index) {
  return "nwcc-" + std::to_string(index) + ".c";
}
#endif

void load_scripts(std::array<std::string, kScriptCount> &sources,
                  std::size_t &count) {
#if defined(PLATFORM_DEVICE)
  if (extapp_calculatorModel() == 0)
    return;
  for (std::size_t index = 0; index < kScriptCount; ++index) {
    const auto name = storage_name(index);
    if (!extapp_fileExists(name.c_str()))
      continue;
    size_t length = 0;
    const char *content = extapp_fileRead(name.c_str(), &length);
    if (content == nullptr || length == 0 || length > kMaxSource)
      continue;
    sources[index].assign(content, length);
    if (!sources[index].empty() && sources[index].back() == '\0')
      sources[index].pop_back();
    count = std::max(count, index + 1);
  }
#else
  (void)sources;
  (void)count;
#endif
}

void save_script(std::size_t index, const std::string &source) {
#if defined(PLATFORM_DEVICE)
  if (extapp_calculatorModel() == 0 || index >= kScriptCount)
    return;
  const auto name = storage_name(index);
  if (extapp_fileExists(name.c_str()))
    extapp_fileErase(name.c_str());
  extapp_fileWrite(name.c_str(), source.c_str(), source.size() + 1);
#else
  (void)index;
  (void)source;
#endif
}

void erase_script(std::size_t index) {
#if defined(PLATFORM_DEVICE)
  if (extapp_calculatorModel() == 0 || index >= kScriptCount)
    return;
  const auto name = storage_name(index);
  if (extapp_fileExists(name.c_str()))
    extapp_fileErase(name.c_str());
#else
  (void)index;
#endif
}

void draw(const std::string &text, std::uint16_t y,
          eadk_color_t color = kWhite) {
  eadk_display_draw_string(text.c_str(), {4, y}, false, color, kBlack);
}

bool pressed(eadk_keyboard_state_t now, eadk_keyboard_state_t previous,
             eadk_key_t key) {
  return eadk_keyboard_key_down(now, key) &&
         !eadk_keyboard_key_down(previous, key);
}

void wait_for_keyboard_release() {
  while (eadk_keyboard_scan() != 0)
    eadk_timing_msleep(20);
}

void render_editor(const std::string &source, std::size_t cursor) {
  eadk_display_push_rect_uniform(eadk_screen_rect, kBlack);
  draw("C editor OK:run ALPHA+EXE:&", 4, kGreen);
  std::size_t cursor_line = 0;
  for (std::size_t position = 0; position < cursor && position < source.size();
       ++position) {
    if (source[position] == '\n')
      ++cursor_line;
  }
  const std::size_t first_line = cursor_line > 4 ? cursor_line - 4 : 0;
  std::size_t start = 0;
  for (std::size_t line = 0; line < first_line; ++line) {
    const auto newline = source.find('\n', start);
    if (newline == std::string::npos)
      break;
    start = newline + 1;
  }
  std::uint16_t y = 25;
  for (std::size_t line = first_line;
       line < first_line + 9 && start <= source.size(); ++line) {
    const auto end = source.find('\n', start);
    std::string text = source.substr(
        start, end == std::string::npos ? source.size() - start : end - start);
    const auto line_end = end == std::string::npos ? source.size() : end;
    std::size_t column = 0;
    if (cursor >= start && cursor <= line_end) {
      column = cursor - start;
    }
    const std::size_t offset =
        column >= kVisibleCharacters ? column - kVisibleCharacters + 1 : 0;
    if (offset > 0) {
      text.erase(0, offset);
    }
    if (text.size() > kVisibleCharacters)
      text.resize(kVisibleCharacters);
    eadk_display_draw_string(std::to_string(line + 1).c_str(),
                             {kLineNumberX, y}, false, kWhite, kBlack);
    eadk_display_draw_string(text.c_str(), {kSourceX, y}, false, kWhite,
                             kBlack);
    if (cursor >= start && cursor <= line_end) {
      column = cursor - start - offset;
      eadk_display_push_rect_uniform(
          {static_cast<std::uint16_t>(kSourceX + column * kCharacterWidth), y,
           2, 15},
          kGreen);
    }
    y += 19;
    if (end == std::string::npos)
      break;
    start = end + 1;
  }
  draw("Arrows:move BACK:list HOME:exit", 211);
}

std::size_t move_vertical(const std::string &source, std::size_t cursor,
                          int direction) {
  const auto current_start = source.rfind('\n', cursor == 0 ? 0 : cursor - 1);
  const std::size_t line_start =
      current_start == std::string::npos ? 0 : current_start + 1;
  const auto current_end = source.find('\n', line_start);
  const std::size_t line_end =
      current_end == std::string::npos ? source.size() : current_end;
  const std::size_t column = cursor - line_start;

  if (direction < 0) {
    if (line_start == 0)
      return cursor;
    const auto previous_end = line_start - 1;
    const auto previous_start =
        source.rfind('\n', previous_end == 0 ? 0 : previous_end - 1);
    const std::size_t target_start =
        previous_start == std::string::npos ? 0 : previous_start + 1;
    return target_start + std::min(column, previous_end - target_start);
  }

  if (line_end == source.size())
    return cursor;
  const std::size_t next_start = line_end + 1;
  const auto next_end = source.find('\n', next_start);
  const std::size_t next_line_end =
      next_end == std::string::npos ? source.size() : next_end;
  return next_start + std::min(column, next_line_end - next_start);
}

enum class Modifier {
  Default,
  Shift,
  Alpha,
  ShiftAlpha,
  AlphaLock,
  ShiftAlphaLock
};

bool is_shift(Modifier modifier) {
  return modifier == Modifier::Shift || modifier == Modifier::ShiftAlpha ||
         modifier == Modifier::ShiftAlphaLock;
}

bool is_alpha(Modifier modifier) {
  return modifier == Modifier::Alpha || modifier == Modifier::ShiftAlpha ||
         modifier == Modifier::AlphaLock ||
         modifier == Modifier::ShiftAlphaLock;
}

bool is_lock(Modifier modifier) {
  return modifier == Modifier::AlphaLock ||
         modifier == Modifier::ShiftAlphaLock;
}

void update_modifier(Modifier &modifier, eadk_key_t key) {
  if (key == eadk_key_shift) {
    switch (modifier) {
    case Modifier::Default:
      modifier = Modifier::Shift;
      break;
    case Modifier::Shift:
      modifier = Modifier::Default;
      break;
    case Modifier::Alpha:
      modifier = Modifier::ShiftAlpha;
      break;
    case Modifier::ShiftAlpha:
      modifier = Modifier::Alpha;
      break;
    case Modifier::AlphaLock:
      modifier = Modifier::ShiftAlphaLock;
      break;
    case Modifier::ShiftAlphaLock:
      modifier = Modifier::AlphaLock;
      break;
    }
  } else if (key == eadk_key_alpha) {
    switch (modifier) {
    case Modifier::Default:
      modifier = Modifier::Alpha;
      break;
    case Modifier::Shift:
      modifier = Modifier::ShiftAlpha;
      break;
    case Modifier::Alpha:
      modifier = Modifier::AlphaLock;
      break;
    case Modifier::ShiftAlpha:
      modifier = Modifier::ShiftAlphaLock;
      break;
    case Modifier::AlphaLock:
    case Modifier::ShiftAlphaLock:
      modifier = Modifier::Default;
      break;
    }
  }
}

std::string typed_text(eadk_key_t pressed_key, Modifier modifier) {
  const auto hit = [&](eadk_key_t key) { return pressed_key == key; };
  const bool alpha = is_alpha(modifier);
  const bool shift = is_shift(modifier);
  if (alpha && shift) {
    const struct Letter {
      eadk_key_t key;
      char value;
    } letters[] = {{eadk_key_exp, 'A'},
                   {eadk_key_ln, 'B'},
                   {eadk_key_log, 'C'},
                   {eadk_key_imaginary, 'D'},
                   {eadk_key_comma, 'E'},
                   {eadk_key_power, 'F'},
                   {eadk_key_sine, 'G'},
                   {eadk_key_cosine, 'H'},
                   {eadk_key_tangent, 'I'},
                   {eadk_key_pi, 'J'},
                   {eadk_key_sqrt, 'K'},
                   {eadk_key_square, 'L'},
                   {eadk_key_seven, 'M'},
                   {eadk_key_eight, 'N'},
                   {eadk_key_nine, 'O'},
                   {eadk_key_left_parenthesis, 'P'},
                   {eadk_key_right_parenthesis, 'Q'},
                   {eadk_key_four, 'R'},
                   {eadk_key_five, 'S'},
                   {eadk_key_six, 'T'},
                   {eadk_key_multiplication, 'U'},
                   {eadk_key_division, 'V'},
                   {eadk_key_one, 'W'},
                   {eadk_key_two, 'X'},
                   {eadk_key_three, 'Y'},
                   {eadk_key_plus, 'Z'}};
    for (const auto &letter : letters) {
      if (hit(letter.key))
        return std::string(1, letter.value);
    }
    if (hit(eadk_key_toolbox))
      return "'";
    return {};
  }
  if (alpha) {
    // Alpha letters follow the labels shown on the calculator keyboard.
    const struct Letter {
      eadk_key_t key;
      char value;
    } letters[] = {{eadk_key_exp, 'a'},
                   {eadk_key_ln, 'b'},
                   {eadk_key_log, 'c'},
                   {eadk_key_imaginary, 'd'},
                   {eadk_key_comma, 'e'},
                   {eadk_key_power, 'f'},
                   {eadk_key_sine, 'g'},
                   {eadk_key_cosine, 'h'},
                   {eadk_key_tangent, 'i'},
                   {eadk_key_pi, 'j'},
                   {eadk_key_sqrt, 'k'},
                   {eadk_key_square, 'l'},
                   {eadk_key_seven, 'm'},
                   {eadk_key_eight, 'n'},
                   {eadk_key_nine, 'o'},
                   {eadk_key_left_parenthesis, 'p'},
                   {eadk_key_right_parenthesis, 'q'},
                   {eadk_key_four, 'r'},
                   {eadk_key_five, 's'},
                   {eadk_key_six, 't'},
                   {eadk_key_multiplication, 'u'},
                   {eadk_key_division, 'v'},
                   {eadk_key_one, 'w'},
                   {eadk_key_two, 'x'},
                   {eadk_key_three, 'y'},
                   {eadk_key_plus, 'z'}};
    for (const auto &letter : letters) {
      if (hit(letter.key))
        return std::string(1, letter.value);
    }
    if (hit(eadk_key_xnt))
      return ":";
    if (hit(eadk_key_var))
      return ";";
    if (hit(eadk_key_toolbox))
      return "\"";
    if (hit(eadk_key_backspace))
      return "%";
    if (hit(eadk_key_minus))
      return " ";
    if (hit(eadk_key_zero))
      return "?";
    if (hit(eadk_key_dot))
      return "!";
    if (hit(eadk_key_ans))
      return "Ans";
    if (hit(eadk_key_exe))
      return "&";
    return {};
  }
  if (shift) {
    const struct ShiftMapping {
      eadk_key_t key;
      const char *value;
    } mappings[] = {{eadk_key_exp, "["},         {eadk_key_ln, "]"},
                    {eadk_key_log, "{"},         {eadk_key_imaginary, "}"},
                    {eadk_key_comma, "_"},       {eadk_key_power, "->"},
                    {eadk_key_sine, "asin("},    {eadk_key_cosine, "acos("},
                    {eadk_key_tangent, "atan("}, {eadk_key_pi, "="},
                    {eadk_key_sqrt, "<"},        {eadk_key_square, ">"}};
    for (const auto &mapping : mappings) {
      if (hit(mapping.key))
        return mapping.value;
    }
    return {};
  }
  const struct Mapping {
    eadk_key_t key;
    const char *value;
  } mappings[] = {{eadk_key_zero, "0"},
                  {eadk_key_one, "1"},
                  {eadk_key_two, "2"},
                  {eadk_key_three, "3"},
                  {eadk_key_four, "4"},
                  {eadk_key_five, "5"},
                  {eadk_key_six, "6"},
                  {eadk_key_seven, "7"},
                  {eadk_key_eight, "8"},
                  {eadk_key_nine, "9"},
                  {eadk_key_plus, "+"},
                  {eadk_key_minus, "-"},
                  {eadk_key_multiplication, "*"},
                  {eadk_key_division, "/"},
                  {eadk_key_left_parenthesis, "("},
                  {eadk_key_right_parenthesis, ")"},
                  {eadk_key_exp, "^"},
                  {eadk_key_imaginary, "i"},
                  {eadk_key_pi, "="},
                  {eadk_key_comma, ","},
                  {eadk_key_dot, "."},
                  {eadk_key_exe, "\n"},
                  {eadk_key_sine, "sin("},
                  {eadk_key_cosine, "cos("},
                  {eadk_key_tangent, "tan("},
                  {eadk_key_sqrt, "<"},
                  {eadk_key_square, ">"}};
  for (const auto &mapping : mappings) {
    if (hit(mapping.key))
      return mapping.value;
  }
  return {};
}

struct InputUiContext {
  std::string *output;
};

void draw_output(const std::string &output) {
  std::size_t start = 0;
  std::uint16_t y = 35;
  while (start < output.size() && y < 130) {
    const auto end = output.find('\n', start);
    draw(output.substr(start, end == std::string::npos ? output.size() - start
                                                       : end - start),
         y, kGreen);
    y += 19;
    if (end == std::string::npos)
      break;
    start = end + 1;
  }
}

bool read_input_line(std::string &line, void *context) {
  line.clear();
  Modifier modifier = Modifier::Default;
  auto *ui = static_cast<InputUiContext *>(context);
  eadk_keyboard_state_t previous = eadk_keyboard_scan();
  while (true) {
    eadk_display_push_rect_uniform(eadk_screen_rect, kBlack);
    draw("Output:", 8, kGreen);
    if (ui != nullptr && ui->output != nullptr)
      draw_output(*ui->output);
    draw("Input for scanf:", 145, kGreen);
    draw(line, 164);
    draw("EXE:submit  DEL:erase", 211);
    const auto keys = eadk_keyboard_scan();
    if (pressed(keys, previous, eadk_key_home) ||
        pressed(keys, previous, eadk_key_back))
      return false;
    if (pressed(keys, previous, eadk_key_exe))
      return true;
    if (pressed(keys, previous, eadk_key_alpha)) {
      update_modifier(modifier, eadk_key_alpha);
      previous = keys;
      eadk_timing_msleep(150);
      continue;
    }
    if (pressed(keys, previous, eadk_key_shift)) {
      update_modifier(modifier, eadk_key_shift);
      previous = keys;
      eadk_timing_msleep(150);
      continue;
    }
    if (pressed(keys, previous, eadk_key_backspace)) {
      if (!line.empty())
        line.pop_back();
      previous = keys;
      eadk_timing_msleep(150);
      continue;
    }
    int pressed_index = -1;
    const auto newly_pressed = keys & ~previous;
    for (int index = 0; index < 64; ++index) {
      if ((newly_pressed & (static_cast<eadk_keyboard_state_t>(1) << index)) !=
          0) {
        pressed_index = index;
        break;
      }
    }
    const std::string text =
        pressed_index < 0
            ? std::string()
            : typed_text(static_cast<eadk_key_t>(pressed_index), modifier);
    if (!text.empty() && line.size() + text.size() < 240)
      line += text;
    previous = keys;
    eadk_timing_msleep(30);
  }
}

void render_result(const nwcc::CompileResult &result) {
  eadk_display_push_rect_uniform(eadk_screen_rect, kBlack);
  if (!result.ok()) {
    draw("Compile error", 8, kRed);
    if (!result.diagnostics.empty()) {
      const auto &diagnostic = result.diagnostics.front();
      draw(std::to_string(diagnostic.line) + ":" +
               std::to_string(diagnostic.column),
           35);
      draw(diagnostic.message, 58);
    }
  } else {
    std::string output;
    std::string error;
    InputUiContext input_context{&output};
    if (!nwcc::Runtime(result.bytecode)
             .run(output, error, read_input_line, &input_context)) {
      draw("Runtime error", 8, kRed);
      draw(error, 35);
    } else {
      draw("Output:", 8, kWhite);
      draw_output(output);
    }
  }
  draw("BACK: edit  HOME: exit", 211);
}

void render_selector(const std::array<std::string, 5> &names, std::size_t count,
                     std::size_t selected) {
  eadk_display_push_rect_uniform(eadk_screen_rect, kBlack);
  draw("C scripts", 8, kGreen);
  draw("OK:edit  +:new  DEL:delete", 30);
  for (std::size_t index = 0; index < count; ++index) {
    const auto y = static_cast<std::uint16_t>(58 + index * 25);
    if (index == selected) {
      eadk_display_push_rect_uniform(
          {0, static_cast<std::uint16_t>(y - 2), 320, 20}, kGreen);
      eadk_display_draw_string(names[index].c_str(), {8, y}, false, kBlack,
                               kGreen);
    } else {
      eadk_display_draw_string(names[index].c_str(), {8, y}, false, kWhite,
                               kBlack);
    }
  }
  draw("Arrows:select  HOME:exit", 211);
}

bool edit_script(std::size_t selected_script, std::string &source) {
  std::size_t cursor = source.size();
  eadk_keyboard_state_t keys_seen_up = 0;
  Modifier modifier = Modifier::Default;
  int repeat_key = -1;
  int repeat_ticks = 0;

  while (true) {
    render_editor(source, cursor);
    while (true) {
      const auto keys = eadk_keyboard_scan();
      keys_seen_up |= ~keys;
      const auto newly_pressed = keys_seen_up & keys;
      int pressed_index = -1;
      if (newly_pressed != 0) {
        for (int index = 63; index >= 0; --index) {
          if ((newly_pressed &
               (static_cast<eadk_keyboard_state_t>(1) << index)) != 0) {
            pressed_index = index;
            break;
          }
        }
      }

      bool repeated = false;
      if (pressed_index < 0 && repeat_key >= 0 &&
          eadk_keyboard_key_down(keys, static_cast<eadk_key_t>(repeat_key))) {
        ++repeat_ticks;
        repeated = repeat_ticks >= 35 && (repeat_ticks - 35) % 7 == 0;
      } else if (pressed_index < 0) {
        repeat_key = -1;
        repeat_ticks = 0;
      }
      if (pressed_index < 0 && !repeated) {
        eadk_timing_msleep(10);
        continue;
      }
      if (pressed_index >= 0) {
        keys_seen_up &=
            ~(static_cast<eadk_keyboard_state_t>(1) << pressed_index);
      } else {
        pressed_index = repeat_key;
      }
      const auto pressed_key = static_cast<eadk_key_t>(pressed_index);
      bool changed = false;
      if (pressed_key == eadk_key_home)
        return true;
      if (pressed_key == eadk_key_back)
        return false;
      if (pressed_key == eadk_key_left || pressed_key == eadk_key_right ||
          pressed_key == eadk_key_up || pressed_key == eadk_key_down) {
        repeat_key = pressed_index;
        if (!repeated)
          repeat_ticks = 0;
      } else if (!repeated) {
        repeat_key = -1;
        repeat_ticks = 0;
      }
      if (pressed_key == eadk_key_alpha || pressed_key == eadk_key_shift) {
        update_modifier(modifier, pressed_key);
        changed = true;
      }
      if (pressed_key == eadk_key_left && cursor > 0) {
        --cursor;
        changed = true;
      }
      if (pressed_key == eadk_key_right && cursor < source.size()) {
        ++cursor;
        changed = true;
      }
      if (pressed_key == eadk_key_up) {
        const auto next_cursor = move_vertical(source, cursor, -1);
        changed = next_cursor != cursor;
        cursor = next_cursor;
      }
      if (pressed_key == eadk_key_down) {
        const auto next_cursor = move_vertical(source, cursor, 1);
        changed = next_cursor != cursor;
        cursor = next_cursor;
      }
      Modifier event_modifier = modifier;
      if (eadk_keyboard_key_down(keys, eadk_key_shift) &&
          !is_shift(event_modifier)) {
        update_modifier(event_modifier, eadk_key_shift);
      }
      if (eadk_keyboard_key_down(keys, eadk_key_alpha) &&
          !is_alpha(event_modifier)) {
        update_modifier(event_modifier, eadk_key_alpha);
      }
      if (pressed_key == eadk_key_backspace && !is_alpha(event_modifier) &&
          !is_shift(event_modifier) && cursor > 0) {
        source.erase(cursor - 1, 1);
        --cursor;
        changed = true;
        save_script(selected_script, source);
      }
      if (pressed_key == eadk_key_ok) {
        render_result(nwcc::compile(source));
        eadk_keyboard_state_t result_previous = keys;
        while (true) {
          const auto result_keys = eadk_keyboard_scan();
          if (pressed(result_keys, result_previous, eadk_key_home))
            return true;
          if (pressed(result_keys, result_previous, eadk_key_back))
            break;
          result_previous = result_keys;
          eadk_timing_msleep(30);
        }
        keys_seen_up = 0;
        render_editor(source, cursor);
      }
      const std::string text = typed_text(pressed_key, event_modifier);
      if (!text.empty() && source.size() + text.size() <= kMaxSource) {
        source.insert(cursor, text);
        cursor += text.size();
        if (!is_lock(event_modifier)) {
          modifier = Modifier::Default;
        } else if (is_shift(event_modifier)) {
          modifier = Modifier::AlphaLock;
        }
        changed = true;
        save_script(selected_script, source);
      }
      if (changed)
        render_editor(source, cursor);
      eadk_timing_msleep(10);
    }
  }
}
} // namespace

// main.c file inside the app
int main() {
  const std::string default_source = "#include <stdio.h>\n"
                                     "\n"
                                     "int main(void) {\n"
                                     "  int number;\n"
                                     "  int total = 0;\n"
                                     "\n"
                                     "  printf(\"NumWorks C demo\\n\");\n"
                                     "  printf(\"Enter a number: \");\n"
                                     "\n"
                                     "  if (scanf(\"%d\", &number) != 1) {\n"
                                     "    printf(\"Invalid input\\n\");\n"
                                     "    return 1;\n"
                                     "  }\n"
                                     "\n"
                                     "  for (int i = 1; i <= number; i++) {\n"
                                     "    total = total + i;\n"
                                     "  }\n"
                                     "\n"
                                     "  printf(\"\\n\\nSum from 1 to %d= %d\\n\", number, total);\n"
                                     "  return 0;\n"
                                     "}\n";


  std::array<std::string, kScriptCount> names = {
      "main.c", "script2.c", "script3.c", "script4.c", "script5.c"};
  std::array<std::string, kScriptCount> sources;
  sources[0] = default_source;
  std::size_t count = 1;
  std::size_t selected = 0;
  load_scripts(sources, count);
  wait_for_keyboard_release();

  while (true) {
    render_selector(names, count, selected);
    eadk_keyboard_state_t previous = 0;
    while (true) {
      const auto keys = eadk_keyboard_scan();
      if (pressed(keys, previous, eadk_key_home))
        return 0;
      if (pressed(keys, previous, eadk_key_up) && selected > 0) {
        --selected;
        break;
      }
      if (pressed(keys, previous, eadk_key_down) && selected + 1 < count) {
        ++selected;
        break;
      }
      if (pressed(keys, previous, eadk_key_ok)) {
        if (edit_script(selected, sources[selected]))
          return 0;
        break;
      }
      if (pressed(keys, previous, eadk_key_plus) && count < sources.size()) {
        sources[count].clear();
        save_script(count, sources[count]);
        ++count;
        selected = count - 1;
        while (eadk_keyboard_key_down(eadk_keyboard_scan(), eadk_key_plus))
          eadk_timing_msleep(20);
        break;
      }
      if (pressed(keys, previous, eadk_key_backspace) && count > 1) {
        for (std::size_t index = selected; index + 1 < count; ++index) {
          sources[index] = sources[index + 1];
          names[index] = names[index + 1];
          save_script(index, sources[index]);
        }
        erase_script(count - 1);
        --count;
        if (selected >= count)
          selected = count - 1;
        break;
      }
      previous = keys;
      eadk_timing_msleep(30);
    }
  }
}

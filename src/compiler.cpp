#include "numworkscc/core.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

extern "C" {
#include "../third_party/picoc/picoc.h"
void nwcc_picoc_set_output_callback(void (*callback)(const char *, int));
void nwcc_picoc_set_input_callback(bool (*callback)(char *, int, void *),
                                   void *context);
}

namespace nwcc {
namespace {

std::string *active_output = nullptr;

struct InputBridge {
  InputProvider provider;
  void *context;
};

void capture_output(const char *data, int length) {
  if (active_output != nullptr && data != nullptr && length > 0)
    active_output->append(data, static_cast<std::size_t>(length));
}

bool bridge_input(char *buffer, int length, void *callback_context) {
  auto *bridge = static_cast<InputBridge *>(callback_context);
  std::string line;
  if (!bridge->provider(line, bridge->context) ||
      line.size() + 2 > static_cast<std::size_t>(length))
    return false;
  std::copy(line.begin(), line.end(), buffer);
  buffer[line.size()] = '\n';
  buffer[line.size() + 1] = '\0';
  return true;
}

bool run_picoc(const std::string &source, bool execute, std::string &output,
               InputProvider input, void *context) {
  Picoc picoc;
  InputBridge input_bridge{input, context};
  active_output = &output;
  nwcc_picoc_set_output_callback(capture_output);
  nwcc_picoc_set_input_callback(input == nullptr ? nullptr : bridge_input,
                                input == nullptr ? nullptr : &input_bridge);
  PicocInitialize(&picoc, 96 * 1024);

  bool success = true;
  if (PicocPlatformSetExitPoint(&picoc) != 0) {
    success = false;
  } else {
    PicocParse(&picoc, "editor.c", source.c_str(),
               static_cast<int>(source.size()), execute, true, true, false);
    if (execute)
      PicocParse(&picoc, "startup", "main();", 7, true, true, false, false);
  }

  PicocCleanup(&picoc);
  nwcc_picoc_set_output_callback(nullptr);
  nwcc_picoc_set_input_callback(nullptr, nullptr);
  active_output = nullptr;
  return success;
}

} // namespace

bool run_picoc_for_runtime(const std::string &source, std::string &output,
                           InputProvider input, void *context) {
  return run_picoc(source, true, output, input, context);
}

CompileResult compile(std::string_view source_view) {
  CompileResult result;
  const std::string source(source_view);
  std::string diagnostics;
  if (!run_picoc(source, false, diagnostics, nullptr, nullptr)) {
    result.diagnostics.push_back(
        {1, 1,
         diagnostics.empty() ? "PicoC rejected the source" : diagnostics});
    return result;
  }
  result.bytecode.assign(source.begin(), source.end());
  return result;
}

} // namespace nwcc

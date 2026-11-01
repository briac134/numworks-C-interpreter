#include "numworkscc/core.hpp"

#include <string>
#include <utility>

namespace nwcc {

bool run_picoc_for_runtime(const std::string &source, std::string &output,
                           InputProvider input, void *context);

Runtime::Runtime(std::vector<std::uint8_t> program)
    : program_(std::move(program)) {}

bool Runtime::run(std::string &output, std::string &error, InputProvider input,
                  void *context) {
  const std::string source(program_.begin(), program_.end());
  if (source.empty()) {
    error = "empty program";
    return false;
  }
  if (!run_picoc_for_runtime(source, output, input, context)) {
    error = output.empty() ? "program execution failed" : output;
    return false;
  }
  return true;
}

} // namespace nwcc

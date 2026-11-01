#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace nwcc {

struct Diagnostic {
  std::size_t line;
  std::size_t column;
  std::string message;
};

struct CompileResult {
  std::vector<std::uint8_t> bytecode;
  std::vector<Diagnostic> diagnostics;
  bool ok() const { return diagnostics.empty(); }
};

CompileResult compile(std::string_view source);
using InputProvider = bool (*)(std::string &line, void *context);

class Runtime {
public:
  explicit Runtime(std::vector<std::uint8_t> program);
  bool run(std::string &output, std::string &error,
           InputProvider input = nullptr, void *context = nullptr);

private:
  std::vector<std::uint8_t> program_;
};

} // namespace nwcc

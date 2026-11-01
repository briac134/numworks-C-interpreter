#pragma once

#include "numworkscc/core.hpp"

#include <string>

namespace nwcc::platform {
class Adapter {
public:
  virtual ~Adapter() = default;
  virtual void show_text(const std::string &text) = 0;
  virtual std::string read_source() = 0;
  bool compile_and_run() {
    const auto result = nwcc::compile(read_source());
    if (!result.ok()) {
      std::string message;
      for (const auto &diagnostic : result.diagnostics)
        message += std::to_string(diagnostic.line) + ":" +
                   std::to_string(diagnostic.column) + ": " +
                   diagnostic.message + "\n";
      show_text(message);
      return false;
    }
    std::string output, error;
    if (!nwcc::Runtime(result.bytecode).run(output, error)) {
      show_text(error);
      return false;
    }
    show_text(output);
    return true;
  }
};
} // namespace nwcc::platform

#include "numworkscc/core.hpp"

#include <fstream>
#include <iostream>
#include <iterator>

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "usage: numworkscc <file.nwc>\n";
    return 2;
  }
  std::ifstream file(argv[1]);
  if (!file) {
    std::cerr << "cannot open " << argv[1] << "\n";
    return 2;
  }
  const std::string source((std::istreambuf_iterator<char>(file)), {});
  const auto result = nwcc::compile(source);
  for (const auto &diagnostic : result.diagnostics)
    std::cerr << diagnostic.line << ":" << diagnostic.column << ": "
              << diagnostic.message << "\n";
  if (!result.ok())
    return 1;
  std::string output, error;
  if (!nwcc::Runtime(result.bytecode).run(output, error)) {
    std::cerr << error << "\n";
    return 1;
  }
  std::cout << output;
}

#include "numworkscc/core.hpp"

#include <cassert>
#include <string>

bool provide_input(std::string &line, void *context) {
  auto *value = static_cast<std::string *>(context);
  line = *value;
  return true;
}

int main() {
  const auto arithmetic = nwcc::compile("#include <stdio.h>\n"
                                        "int main(void) {\n"
                                        "  int value = 2 + 3 * 4;\n"
                                        "  printf(\"%d\\n\", value);\n"
                                        "  return 0;\n"
                                        "}\n");
  assert(arithmetic.ok());
  std::string output;
  std::string error;
  assert(nwcc::Runtime(arithmetic.bytecode).run(output, error));
  assert(output == "14\n");

  const auto c_features =
      nwcc::compile("#include <stdio.h>\n"
                    "int factorial(int value) {\n"
                    "  if (value <= 1) return 1;\n"
                    "  return value * factorial(value - 1);\n"
                    "}\n"
                    "int main(void) {\n"
                    "  int values[3] = {1, 2, 3};\n"
                    "  int i;\n"
                    "  for (i = 0; i < 3; i++) printf(\"%d \", values[i]);\n"
                    "  printf(\"%d\\n\", factorial(5));\n"
                    "  return 0;\n"
                    "}\n");
  assert(c_features.ok());
  output.clear();
  error.clear();
  assert(nwcc::Runtime(c_features.bytecode).run(output, error));
  assert(output == "1 2 3 120\n");

  const auto advanced = nwcc::compile(
      "#include <stdio.h>\n"
      "#include <stdlib.h>\n"
      "#include <string.h>\n"
      "#include <math.h>\n"
      "struct Pair { int left; int right; };\n"
      "enum Kind { FIRST = 4, SECOND };\n"
      "int main(void) {\n"
      "  struct Pair pair;\n"
      "  char text[8];\n"
      "  int value = 0;\n"
      "  int *pointer = malloc(sizeof(int));\n"
      "  pair.left = FIRST;\n"
      "  pair.right = SECOND;\n"
      "  *pointer = 9;\n"
      "  strcpy(text, \"ok\");\n"
      "  sscanf(\"42\", \"%d\", &value);\n"
      "  switch (pair.left) {\n"
      "    case 4: printf(\"switch \"); break;\n"
      "    default: printf(\"wrong \");\n"
      "  }\n"
      "  printf(\"%s %d %d %d %d\\n\", text, strlen(text), *pointer,\n"
      "         value, (int)sqrt(16));\n"
      "  free(pointer);\n"
      "  return 0;\n"
      "}\n");
  assert(advanced.ok());
  output.clear();
  error.clear();
  assert(nwcc::Runtime(advanced.bytecode).run(output, error));
  assert(output == "switch ok 2 9 42 4\n");

  const auto scanf_without_input =
      nwcc::compile("#include <stdio.h>\n"
                    "int main(void) {\n"
                    "  int value = 7;\n"
                    "  printf(\"%d\\n\", scanf(\"%d\", &value));\n"
                    "  return 0;\n"
                    "}\n");
  assert(scanf_without_input.ok());
  output.clear();
  error.clear();
  assert(nwcc::Runtime(scanf_without_input.bytecode).run(output, error));
  assert(output == "-1\n");

  const auto scanf_with_input =
      nwcc::compile("#include <stdio.h>\n"
                    "int main(void) {\n"
                    "  int value = 0;\n"
                    "  int count = scanf(\"%d\", &value);\n"
                    "  printf(\"%d %d\\n\", count, value);\n"
                    "  return 0;\n"
                    "}\n");
  assert(scanf_with_input.ok());
  std::string input = "123";
  output.clear();
  error.clear();
  assert(nwcc::Runtime(scanf_with_input.bytecode)
             .run(output, error, provide_input, &input));
  assert(output == "1 123\n");

  const auto invalid = nwcc::compile("int main(void) {\n"
                                     "  printf(\"unterminated);\n"
                                     "}\n");
  assert(!invalid.ok());
  assert(!invalid.diagnostics.front().message.empty());
  return 0;
}

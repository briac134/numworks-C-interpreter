CXX ?= c++
CC ?= cc
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -DUNIX_HOST -Iinclude
CFLAGS ?= -std=gnu11 -Wall -Wextra -Wpedantic -DUNIX_HOST
BUILD := build

PICO_C := table lex parse expression heap type variable clibrary platform include debug
PICO_C += cstdlib/stdio cstdlib/math cstdlib/string cstdlib/stdlib
PICO_C += cstdlib/time cstdlib/errno cstdlib/ctype cstdlib/stdbool cstdlib/unistd
PICO_OBJECTS := $(addprefix $(BUILD)/picoc/,$(addsuffix .o,$(PICO_C)))
CORE := $(BUILD)/compiler.o $(BUILD)/runtime.o $(PICO_OBJECTS) $(BUILD)/picoc/platform_nwcc.o

.PHONY: all test clean

all: $(BUILD)/numworkscc $(BUILD)/core_tests $(BUILD)/epsilon_adapter.o $(BUILD)/upsilon_adapter.o

$(BUILD):
	mkdir -p $@

$(BUILD)/compiler.o: src/compiler.cpp include/numworkscc/core.hpp third_party/picoc/picoc.h | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/runtime.o: src/runtime.cpp include/numworkscc/core.hpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/picoc/%.o: third_party/picoc/%.c | $(BUILD)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -Ithird_party/picoc -Ithird_party/picoc/cstdlib -c $< -o $@

$(BUILD)/picoc/cstdlib/%.o: third_party/picoc/cstdlib/%.c | $(BUILD)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -Ithird_party/picoc -Ithird_party/picoc/cstdlib -c $< -o $@

$(BUILD)/picoc/platform_nwcc.o: third_party/picoc/platform_nwcc.c | $(BUILD)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -Ithird_party/picoc -c $< -o $@

$(BUILD)/numworkscc: $(CORE) src/main.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD)/core_tests: $(CORE) tests/core_tests.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD)/epsilon_adapter.o: platform/epsilon/epsilon_adapter.cpp platform/adapter.hpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -Iplatform -c $< -o $@

$(BUILD)/upsilon_adapter.o: platform/upsilon/upsilon_adapter.cpp platform/adapter.hpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -Iplatform -c $< -o $@

test: $(BUILD)/core_tests
	$(BUILD)/core_tests

clean:
	rm -rf $(BUILD)

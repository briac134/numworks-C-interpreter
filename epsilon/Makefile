Q ?= @
CC = arm-none-eabi-gcc
CXX = arm-none-eabi-g++
SIM_CXX ?= c++
BUILD_DIR = build
NWLINK ?= nwlink

CPPFLAGS = -DPLATFORM_DEVICE -I../include $(shell $(NWLINK) eadk-cflags-device)
CXXFLAGS = -std=c++17 -Os -Wall -Wextra -Wpedantic -DUNIX_HOST -fno-exceptions -fno-rtti
CXXFLAGS += -ffunction-sections -fdata-sections
CFLAGS = -std=gnu11 -Os -Wall -Wextra -Wpedantic -DUNIX_HOST -DNO_TIME -DNO_UNISTD
CFLAGS += -ffunction-sections -fdata-sections
LDFLAGS = $(shell $(NWLINK) eadk-ldflags-device)
LDFLAGS += --specs=nano.specs
LDFLAGS += -Wl,-e,main -Wl,-u,eadk_app_name -Wl,-u,eadk_app_icon -Wl,-u,eadk_api_level
LDFLAGS += -Wl,--gc-sections

PICO_C = table lex parse expression heap type variable clibrary platform include debug
PICO_C += cstdlib/stdio cstdlib/math cstdlib/string cstdlib/stdlib
PICO_C += cstdlib/errno cstdlib/ctype cstdlib/stdbool
PICO_OBJECTS = $(addprefix $(BUILD_DIR)/picoc/,$(addsuffix .o,$(PICO_C)))
OBJECTS = $(BUILD_DIR)/src/compiler.o $(BUILD_DIR)/src/runtime.o \
	$(BUILD_DIR)/src/main.o $(BUILD_DIR)/src/syscalls.o \
	$(BUILD_DIR)/src/storage.o $(PICO_OBJECTS) \
	$(BUILD_DIR)/picoc/platform_nwcc.o

.PHONY: all build run simulator clean

all build: $(BUILD_DIR)/numworkscc.nwa

run: $(BUILD_DIR)/numworkscc.nwa
	$(NWLINK) install-nwa $<

simulator: $(BUILD_DIR)/numworkscc-simulator
	$<

$(BUILD_DIR)/numworkscc-simulator: $(BUILD_DIR)/src/main.simulator.o \
		$(BUILD_DIR)/src/simulator_stubs.o ../build/compiler.o \
		../build/runtime.o ../build/picoc/table.o ../build/picoc/lex.o \
		../build/picoc/parse.o ../build/picoc/expression.o \
		../build/picoc/heap.o ../build/picoc/type.o \
		../build/picoc/variable.o ../build/picoc/clibrary.o \
		../build/picoc/platform.o ../build/picoc/include.o \
		../build/picoc/debug.o ../build/picoc/cstdlib/stdio.o \
		../build/picoc/cstdlib/math.o ../build/picoc/cstdlib/string.o \
		../build/picoc/cstdlib/stdlib.o ../build/picoc/cstdlib/time.o \
		../build/picoc/cstdlib/errno.o ../build/picoc/cstdlib/ctype.o \
		../build/picoc/cstdlib/stdbool.o ../build/picoc/cstdlib/unistd.o \
		../build/picoc/platform_nwcc.o
	@echo "LD      $@"
	$(SIM_CXX) $^ -o $@

$(BUILD_DIR)/src/main.simulator.o: src/main.cpp
	@mkdir -p $(dir $@)
	@echo "CXX     $< (simulator)"
	$(SIM_CXX) -std=c++17 -Wall -Wextra -Wpedantic -I../include \
		$(shell $(NWLINK) eadk-cflags-simulator) \
		-c $< -o $@

$(BUILD_DIR)/src/simulator_stubs.o: src/simulator_stubs.cpp
	@mkdir -p $(dir $@)
	@echo "CXX     $<"
	$(SIM_CXX) -std=c++17 -Wall -Wextra -Wpedantic \
		$(shell $(NWLINK) eadk-cflags-simulator) \
		-c $< -o $@

$(BUILD_DIR)/numworkscc.nwa: $(OBJECTS) $(BUILD_DIR)/icon.o
	@mkdir -p $(dir $@)
	@echo "LD      $@"
	$(Q)$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: ../%.cpp
	@mkdir -p $(dir $@)
	@echo "CXX     $<"
	$(Q)$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/picoc/%.o: ../third_party/picoc/%.c
	@mkdir -p $(dir $@)
	@echo "CC      $<"
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -I../third_party/picoc -I../third_party/picoc/cstdlib -c $< -o $@

$(BUILD_DIR)/picoc/cstdlib/%.o: ../third_party/picoc/cstdlib/%.c
	@mkdir -p $(dir $@)
	@echo "CC      $<"
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -I../third_party/picoc -I../third_party/picoc/cstdlib -c $< -o $@

$(BUILD_DIR)/picoc/platform_nwcc.o: ../third_party/picoc/platform_nwcc.c
	@mkdir -p $(dir $@)
	@echo "CC      $<"
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -I../third_party/picoc -c $< -o $@

$(BUILD_DIR)/src/main.o: src/main.cpp
	@mkdir -p $(dir $@)
	@echo "CXX     $<"
	$(Q)$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/src/syscalls.o: src/syscalls.cpp
	@mkdir -p $(dir $@)
	@echo "CXX     $<"
	$(Q)$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/src/storage.o: src/storage.c src/storage.h
	@mkdir -p $(dir $@)
	@echo "CC      $<"
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/icon.o: src/icon.png
	@mkdir -p $(dir $@)
	$(Q)$(NWLINK) png-icon-o $< $@

clean:
	rm -rf $(BUILD_DIR)

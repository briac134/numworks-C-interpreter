Q ?= @
PLATFORM ?= device
NWLINK ?= nwlink
BUILD_DIR := build/$(PLATFORM)
SIMULATOR ?= sim/epsilon.bin

PICO_C := table lex parse expression heap type variable clibrary platform include debug
PICO_C += cstdlib/stdio cstdlib/math cstdlib/string cstdlib/stdlib
PICO_C += cstdlib/errno cstdlib/ctype cstdlib/stdbool

ifeq ($(PLATFORM),simulator)
CC := cc
CXX := c++
CPPFLAGS := -I../include $(shell $(NWLINK) eadk-cflags-simulator)
CFLAGS := -std=gnu11 -Wall -Wextra -Wpedantic -DUNIX_HOST
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -DUNIX_HOST
PICO_C += cstdlib/time cstdlib/unistd
PICO_OBJECTS := $(addprefix $(BUILD_DIR)/picoc/,$(addsuffix .o,$(PICO_C)))
OBJECTS := $(BUILD_DIR)/src/compiler.o $(BUILD_DIR)/src/runtime.o \
	$(BUILD_DIR)/src/main.o $(BUILD_DIR)/src/simulator_stubs.o \
	$(PICO_OBJECTS) $(BUILD_DIR)/picoc/platform_nwcc.o
TARGET := $(BUILD_DIR)/numworkscc.nwb
else ifeq ($(PLATFORM),device)
CC := arm-none-eabi-gcc
CXX := arm-none-eabi-g++
CPPFLAGS := -DPLATFORM_DEVICE -I../include $(shell $(NWLINK) eadk-cflags-device)
CFLAGS := -std=gnu11 -Os -Wall -Wextra -Wpedantic -DUNIX_HOST -DNO_TIME -DNO_UNISTD
CFLAGS += -ffunction-sections -fdata-sections
CXXFLAGS := -std=c++17 -Os -Wall -Wextra -Wpedantic -DUNIX_HOST
CXXFLAGS += -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections
LDFLAGS := $(shell $(NWLINK) eadk-ldflags-device)
LDFLAGS += --specs=nano.specs
LDFLAGS += -Wl,-e,main -Wl,-u,eadk_app_name -Wl,-u,eadk_app_icon
LDFLAGS += -Wl,-u,eadk_api_level -Wl,--gc-sections
PICO_OBJECTS := $(addprefix $(BUILD_DIR)/picoc/,$(addsuffix .o,$(PICO_C)))
OBJECTS := $(BUILD_DIR)/src/compiler.o $(BUILD_DIR)/src/runtime.o \
	$(BUILD_DIR)/src/main.o $(BUILD_DIR)/src/syscalls.o \
	$(BUILD_DIR)/src/storage.o $(PICO_OBJECTS) \
	$(BUILD_DIR)/picoc/platform_nwcc.o
TARGET := $(BUILD_DIR)/numworkscc.nwa
else
$(error PLATFORM must be device or simulator)
endif

.PHONY: all build run clean

all build: $(TARGET)

ifeq ($(PLATFORM),simulator)
run: $(TARGET)
	@test -x "$(SIMULATOR)" || \
		(echo "Simulator not found: $(SIMULATOR)" >&2; \
		 echo "Set SIMULATOR=/path/to/epsilon.bin or build Epsilon's simulator." >&2; \
		 exit 1)
	"$(SIMULATOR)" --nwb "$<"
else
run: $(TARGET)
	$(NWLINK) install-nwa $<
endif

$(BUILD_DIR):
	mkdir -p $@

$(TARGET): $(OBJECTS) $(if $(filter device,$(PLATFORM)),$(BUILD_DIR)/icon.o)
	@mkdir -p $(dir $@)
	@echo "LD      $@"
	$(CXX) $(CXXFLAGS) $(if $(filter device,$(PLATFORM)),$(CPPFLAGS) $(LDFLAGS),-shared) $^ -o $@

$(BUILD_DIR)/%.o: ../%.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "CXX     $<"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/src/main.o: src/main.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "CXX     $<"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/src/simulator_stubs.o: src/simulator_stubs.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "CXX     $<"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/picoc/%.o: ../third_party/picoc/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "CC      $<"
	$(CC) $(CPPFLAGS) $(CFLAGS) -I../third_party/picoc \
		-I../third_party/picoc/cstdlib -c $< -o $@

$(BUILD_DIR)/picoc/cstdlib/%.o: ../third_party/picoc/cstdlib/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "CC      $<"
	$(CC) $(CPPFLAGS) $(CFLAGS) -I../third_party/picoc \
		-I../third_party/picoc/cstdlib -c $< -o $@

$(BUILD_DIR)/src/syscalls.o: src/syscalls.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "CXX     $<"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/src/storage.o: src/storage.c src/storage.h | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "CC      $<"
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/icon.o: src/icon.png | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "ICON    $<"
	$(NWLINK) png-icon-o $< $@

clean:
	rm -rf build

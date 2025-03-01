# Makefile for DRV8833 Motor Driver (C code)
 
# Artifact name: output binary name
ARTIFACT = drv8833-motor-driver
 
# Build architecture/variant string, e.g., aarch64le, x86_64le, etc.
PLATFORM ?= aarch64le
 
# Build profile, possible values: release, debug, profile, coverage
BUILD_PROFILE ?= debug
 
# Derived variables for output directory and target binary
CONFIG_NAME ?= $(PLATFORM)-$(BUILD_PROFILE)
OUTPUT_DIR = build/$(CONFIG_NAME)
TARGET = $(OUTPUT_DIR)/$(ARTIFACT)
 
# Compiler and linker definitions
CC = qcc -Vgcc_nto$(PLATFORM)
LD = $(CC)
 
# Include paths
INCLUDES = -I../../common/system/gpio/ -I../../common/rpi_gpio
 
# Build profile flags
CCFLAGS_release = -O2
CCFLAGS_debug   = -g -O0 -fno-builtin
 
# Generic compiler flags (common flags plus flags for the chosen profile)
CCFLAGS_all = -Wall -fmessage-length=0 $(CCFLAGS_$(BUILD_PROFILE)) $(INCLUDES)
 
# Dependency generation flags
DEPS = -Wp,-MMD,$(@:%.o=%.d),-MT,$@
 
# Source files: drv8833-motor-driver.c and rpi_gpio.c
SRCS = src/drv8833-motor-driver.c ../../common/rpi_gpio/rpi_gpio.c
 
# Object files list (explicitly naming objects)
OBJS = $(OUTPUT_DIR)/drv8833-motor-driver.o $(OUTPUT_DIR)/rpi_gpio.o
 
# Rule for compiling MotorDriver.c
$(OUTPUT_DIR)/drv8833-motor-driver.o: src/drv8833-motor-driver.c
	@mkdir -p $(dir $@)
	$(CC) -c $(DEPS) -o $@ $(CCFLAGS_all) $<
 
# Rule for compiling rpi_gpio.c
$(OUTPUT_DIR)/rpi_gpio.o: ../../common/rpi_gpio/rpi_gpio.c
	@mkdir -p $(dir $@)
	$(CC) -c $(DEPS) -o $@ $(CCFLAGS_all) $<
 
# Linking rule to create the final binary
$(TARGET): $(OBJS)
	$(LD) -o $(TARGET) $(OBJS)
 
# Default rule
all: $(TARGET)
 
# Clean up build artifacts
clean:
	rm -rf $(OUTPUT_DIR)
 
# Rebuild: clean then build
rebuild: clean all
 
# Include dependency files (if present)
-include $(OBJS:.o=.d)

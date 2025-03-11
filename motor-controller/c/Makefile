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
INCLUDES = -I../../common/system/gpio/ -I../../common/rpi_gpio/public
 
# Build profile flags
CCFLAGS_release = -O2
CCFLAGS_debug   = -g -O0 -fno-builtin
 
# Generic compiler flags (common flags plus flags for the chosen profile)
CCFLAGS_all = -Wall -fmessage-length=0 $(CCFLAGS_$(BUILD_PROFILE)) $(INCLUDES)
 
# Dependency generation flags
DEPS = -Wp,-MMD,$(@:%.o=%.d),-MT,$@
 
# Source files: drv8833-motor-driver.c
SRCS = src/drv8833-motor-driver.c 
 
# Convert source file paths into object file paths in the build output directory
OBJS = $(addprefix $(OUTPUT_DIR)/, $(SRCS:.c=.o))
 
# Rule for compiling .c files into .o
$(OUTPUT_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(DEPS) -o $@ $(CCFLAGS_all) $<
 
# Rule for compiling .c files into .o
$(OUTPUT_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(DEPS) -o $@ $(CCFLAGS_all) $<
 
# Build static library for rpi_gpio from the common folder.
librpi_gpio.a:
	$(MAKE) -C ../../common/rpi_gpio
 
# Add library search path and libraries (adjust path as needed)
LIBS = -L../../common/rpi_gpio/build/$(CONFIG_NAME) -lrpi_gpio -lm
 
# Linking rule to create the final binary
$(TARGET): $(OBJS) librpi_gpio.a
	$(LD) -o $(TARGET) $(OBJS) $(LIBS)
 
# Default rule
all: $(TARGET)
 
# Clean up build artifacts
clean:
	rm -rf $(OUTPUT_DIR)
 
# Rebuild: clean then build
rebuild: clean all
 
# Include auto-generated dependency files if present
-include $(OBJS:.o=.d)

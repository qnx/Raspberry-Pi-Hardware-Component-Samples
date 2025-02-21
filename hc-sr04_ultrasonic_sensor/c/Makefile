# Makefile for HC-SR04 Ultrasonic Sensor (C code)
 
# Artifact name: output binary name
ARTIFACT = Ultrasonic
 
# Build architecture/variant string, possible values: aarch64le, x86_64le, etc.
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
INCLUDES = -I../../common/system/gpio -I../../common/rpi_gpio
 
# Build profile flags
CCFLAGS_release = -O2
CCFLAGS_debug   = -g -O0 -fno-builtin
# CCFLAGS_profile   = -g -O0 -finstrument-functions
# CCFLAGS_coverage  = -g -O0 -ftest-coverage -fprofile-arcs
 
# Generic compiler flags (common flags plus flags for the chosen profile)
CCFLAGS_all = -Wall -fmessage-length=0 $(CCFLAGS_$(BUILD_PROFILE)) $(INCLUDES)
 
# Dependency generation flags
DEPS = -Wp,-MMD,$(@:%.o=%.d),-MT,$@
 
# Source file 
SRCS = src/Ultrasonic.c
 
# Object files list (placed in OUTPUT_DIR)
OBJS = $(addprefix $(OUTPUT_DIR)/, $(SRCS:.c=.o))
 
# Rule for compiling .c files
$(OUTPUT_DIR)/%.o: %.c
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

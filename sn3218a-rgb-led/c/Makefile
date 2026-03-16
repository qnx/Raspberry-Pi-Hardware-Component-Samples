#
# Copyright (c) 2026, BlackBerry Limited. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

include ../../common/config.mk

# Artifact name: the final binary
ARTIFACT_TYPE = exe
ARTIFACT = sn3218a-rgb-led
 
# Build architecture/variant string, e.g. aarch64le, x86_64le, etc.
PLATFORM ?= aarch64le
 
# Build profile: possible values: release, debug, profile, coverage
BUILD_PROFILE ?= debug
 
# Derived variables for output directory and target binary
CONFIG_NAME ?= $(PLATFORM)-$(BUILD_PROFILE)
OUTPUT_DIR = build/$(CONFIG_NAME)
TARGET = $(OUTPUT_DIR)/$(ARTIFACT)
 
# Include paths (adjusted: use the folder where rpi_i2c.h is now located)
INCLUDES = -I../../common/rpi_i2c/public/ -I../../common/rpi_gpio/public/ -I../../common/system/gpio/ 
 
# Build profile flags
CCFLAGS_release = -O2
CCFLAGS_debug   = -g -O0 -fno-builtin
#CCFLAGS_profile  = -g -O0 -finstrument-functions
#CCFLAGS_coverage = -g -O0 -ftest-coverage -fprofile-arcs
 
# Combine common flags with the profile-specific flags
CCFLAGS_all = -Wall -fmessage-length=0 $(CCFLAGS_$(BUILD_PROFILE)) $(INCLUDES)
 
# Dependency generation flags
DEPS = -Wp,-MMD,$(@:%.o=%.d),-MT,$@
 
# Source files for your IMU demo (do not compile rpi_i2c.c directly)
SRCS = src/sn3218a-rgb-led.c
 
# Convert source file paths into object file paths in the build output directory
OBJS = $(addprefix $(OUTPUT_DIR)/, $(SRCS:.c=.o))
 
# Rule for compiling .c files into .o
$(OUTPUT_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(DEPS) -o $@ $(CCFLAGS_all) $<
 
# Add library search path and libraries (adjust path as needed)
LIBS = -L../../common/rpi_i2c/build/$(CONFIG_NAME) -lrpi_i2c -lm -L../../common/rpi_gpio/build/$(CONFIG_NAME) -lrpi_gpio -lm
 
# Linking rule to create the final binary
$(TARGET): $(OBJS) librpi_i2c.a librpi_gpio.a
	$(LD) $(LDFLAGS_$(ARTIFACT_TYPE)) $(TARGET) $(LDFLAGS_all) $(LDFLAGS) $(OBJS) $(LIBS_all) $(LIBS)
 
# Default rule
all: $(TARGET)
 
# Build static library for rpi_i2c from the common folder.
librpi_i2c.a:
	$(MAKE) -C ../../common/rpi_i2c
	
# Build static library for rpi_i2c from the common folder.
librpi_gpio.a:
	$(MAKE) -C ../../common/rpi_gpio
 
# Clean up build artifacts
clean:
	rm -rf $(OUTPUT_DIR)
 
# Rebuild: clean then build
rebuild: clean all
 
# Include auto-generated dependency files if present
-include $(OBJS:.o=.d)

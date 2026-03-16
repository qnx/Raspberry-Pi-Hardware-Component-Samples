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

# Makefile for HC-SR04 Ultrasonic Sensor (C code)
 
# Artifact name: output binary name
ARTIFACT_TYPE = exe
ARTIFACT = Ultrasonic
 
# Build architecture/variant string, possible values: aarch64le, x86_64le, etc.
PLATFORM ?= aarch64le
 
# Build profile, possible values: release, debug, profile, coverage
BUILD_PROFILE ?= debug
 
# Derived variables for output directory and target binary
CONFIG_NAME ?= $(PLATFORM)-$(BUILD_PROFILE)
OUTPUT_DIR = build/$(CONFIG_NAME)
TARGET = $(OUTPUT_DIR)/$(ARTIFACT)
 
# Include paths 
INCLUDES = -I../../common/system/gpio -I../../common/rpi_gpio/public/
 
# Build profile flags
CCFLAGS_release = -O2
CCFLAGS_debug   = -g -O0 -fno-builtin
# CCFLAGS_profile   = -g -O0 -finstrument-functions
# CCFLAGS_coverage  = -g -O0 -ftest-coverage -fprofile-arcs
 
# Generic compiler flags (common flags plus flags for the chosen profile)
CCFLAGS_all = -Wall -fmessage-length=0 $(CCFLAGS_$(BUILD_PROFILE)) $(INCLUDES)
 
# Dependency generation flags
LDFLAGS_all += $(LDFLAGS_$(BUILD_PROFILE))
LIBS_all += $(LIBS_$(BUILD_PROFILE))
DEPS = -Wp,-MMD,$(@:%.o=%.d),-MT,$@
 
# Macro to expand files recursively: parameters $1 -  directory, $2 - extension, i.e. cpp
rwildcard = $(wildcard $(addprefix $1/*.,$2)) $(foreach d,$(wildcard $1/*),$(call rwildcard,$d,$2))

#Source list
SRCS = $(call rwildcard, src, c)

#Object files list
OBJS = $(addprefix $(OUTPUT_DIR)/,$(addsuffix .o, $(basename $(SRCS))))
 
# Rule for compiling .c files into .o
$(OUTPUT_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(DEPS) -o $@ $(INCLUDES) $(CCFLAGS_all) $(CCFLAGS) $<
 
# Add library search path and libraries (adjust path as needed)
LIBS = -L../../common/rpi_gpio/build/$(CONFIG_NAME) -lrpi_gpio -lm
 
# Linking rule to create the final binary
$(TARGET): $(OBJS) librpi_gpio.a
	$(LD) $(LDFLAGS_$(ARTIFACT_TYPE)) $(TARGET) $(LDFLAGS_all) $(LDFLAGS) $(OBJS) $(LIBS_all) $(LIBS)
 
# Default rule
all: $(TARGET)

# Build static library for rpi_gpio from the common folder.
librpi_gpio.a:
	$(MAKE) -C ../../common/rpi_gpio
 
# Clean up build artifacts
clean:
	rm -rf $(OUTPUT_DIR)
 
# Rebuild: clean then build
rebuild: clean all
 
# Include auto-generated dependency files if present
-include $(OBJS:.o=.d)

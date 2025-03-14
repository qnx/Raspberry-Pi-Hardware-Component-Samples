# Artifact name: the final binary
ARTIFACT = imu-icm-20948
 
# Build architecture/variant string, e.g. aarch64le, x86_64le, etc.
PLATFORM ?= aarch64le
 
# Build profile: possible values: release, debug, profile, coverage
BUILD_PROFILE ?= debug
 
# Derived variables for output directory and target binary
CONFIG_NAME ?= $(PLATFORM)-$(BUILD_PROFILE)
OUTPUT_DIR = build/$(CONFIG_NAME)
TARGET = $(OUTPUT_DIR)/$(ARTIFACT)
 
# Compiler and linker definitions
CC = qcc -Vgcc_nto$(PLATFORM)
LD = $(CC)
 
# Include paths (adjusted: use the folder where rpi_i2c.h is now located)
INCLUDES = -I../../common/rpi_i2c/public/
 
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
SRCS = src/imu-icm-20948.c
 
# Convert source file paths into object file paths in the build output directory
OBJS = $(addprefix $(OUTPUT_DIR)/, $(SRCS:.c=.o))
 
# Rule for compiling .c files into .o
$(OUTPUT_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(DEPS) -o $@ $(CCFLAGS_all) $<
 
# Build static library for rpi_i2c from the common folder.
librpi_i2c.a:
	$(MAKE) -C ../../common/rpi_i2c
 
# Add library search path and libraries (adjust path as needed)
LIBS = -L../../common/rpi_i2c/build/$(CONFIG_NAME) -lrpi_i2c -lm
 
# Linking rule to create the final binary
$(TARGET): $(OBJS) librpi_i2c.a
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

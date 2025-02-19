
ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

NAME=Ultrasonic

# Include path for the shared GPIO client API
CCFLAGS += -I../../../../../common/system/gpio/aarch64 -I../../../../../common/system/gpio/sys

#This has to be included last
include $(MKFILES_ROOT)/qtargets.mk

##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#
GIT_BRANCH := $(shell git rev-parse --abbrev-ref HEAD) 
GIT_SHA := $(shell git rev-parse --short HEAD)
GIT_TAG := $(shell git describe --abbrev=0 --dirty --always --tags)

# Compiler options here.
# -Wdouble-promotion -fno-omit-frame-pointer

DEBUG := DEBUG
SPEED := SPEED
SIZE := SIZE
SMALL := SMALL

ifeq    ($(BUILD),)
        BUILD := $(SIZE)
endif


GCCVERSIONGTEQ10 := $(shell expr `arm-none-eabi-gcc -dumpversion | cut -f1 -d.` \>= 10)
GCC_DIAG =  -Werror -Wno-error=unused-variable -Wno-error=format \
            -Wno-error=cpp  \
            -Wno-error=unused-function \
            -Wunused -Wpointer-arith \
            -Werror=sign-compare \
            -Wshadow -Wparentheses -fmax-errors=5 \
            -ftrack-macro-expansion=2 -Wno-error=strict-overflow -Wstrict-overflow=2 \
	    -Wvla-larger-than=128 -Wduplicated-branches -Wdangling-else \
            -Wformat-overflow=2

ifeq "$(GCCVERSIONGTEQ10)" "1"
#    GCC_DIAG = -Wno-error=volatile 
    USE_CPPOPT = -Wno-volatile -Wno-error=deprecated-declarations
endif

ifeq ($(BUILD),$(DEBUG)) 
  USE_LTO = no
  USE_OPT =  -Og -ggdb3  -Wall -Wextra \
	    -falign-functions=16 -fomit-frame-pointer \
	    $(GCC_DIAG) -DNO_AUDIO_SET -DTRACE
endif

ifeq ($(BUILD),$(SMALL)) 
  USE_LTO = yes
  USE_OPT =  -Os -Wall -Wextra \
	    -falign-functions=16 -fomit-frame-pointer \
	    $(GCC_DIAG) -DNO_AUDIO_SET -DTRACE
endif

ifeq ($(BUILD),$(SPEED))
    USE_LTO = yes
    USE_OPT =  -Ofast -Wall -Wextra \
	    -falign-functions=16 -fomit-frame-pointer \
	     $(GCC_DIAG)
endif

#            --specs=nano.specs -DSMALL_AUDIO_SET
ifeq ($(BUILD),$(SIZE)) 
    USE_LTO = yes
    USE_OPT =  -Os  -flto  -Wall -Wextra \
	    -falign-functions=16 -fomit-frame-pointer \
            -DCH_DBG_STATISTICS=0 -DCH_DBG_SYSTEM_STATE_CHECK=0 -DCH_DBG_ENABLE_CHECKS=0 \
	    -DCH_DBG_ENABLE_ASSERTS=0 -DCH_DBG_ENABLE_STACK_CHECK=0 -DCH_DBG_FILL_THREADS=0 \
	    -DCH_CFG_ST_TIMEDELTA=2 -DCH_CFG_TIME_QUANTUM=0 \
	    -DCH_DBG_THREADS_PROFILING=0 -DNOSHELL=1  \
	     $(GCC_DIAG)
endif


# C specific options here (added to USE_OPT).
ifeq ($(USE_COPT),)
  USE_COPT = -std=gnu11  -Wunsuffixed-float-constants 
endif

# C++ specific options here (added to USE_OPT).

USE_CPPOPT += -std=c++20 -fno-rtti -fno-exceptions -fno-threadsafe-statics


# Enable this if you want the linker to remove unused code and data
ifeq ($(USE_LINK_GC),)
  USE_LINK_GC = yes
endif

# Linker extra options here.
ifeq ($(USE_LDOPT),)
  USE_LDOPT = 
endif

# Enable this if you want link time optimizations (LTO).
ifeq ($(USE_LTO),)
  USE_LTO = no
endif


# Enable this if you want to see the full log while compiling.
ifeq ($(USE_VERBOSE_COMPILE),)
  USE_VERBOSE_COMPILE = no
endif

# If enabled, this option makes the build process faster by not compiling
# modules not used in the current configuration.
ifeq ($(USE_SMART_BUILD),)
  USE_SMART_BUILD = yes
endif

#
# Build global options
##############################################################################

##############################################################################
# Architecture or project specific options
#

# Stack size to be allocated to the Cortex-M process stack. This stack is
# the stack used by the main() thread.
ifeq ($(USE_PROCESS_STACKSIZE),)
  USE_PROCESS_STACKSIZE = 0x2000
endif

# Stack size to the allocated to the Cortex-M main/exceptions stack. This
# stack is used for processing interrupts and exceptions.
ifeq ($(USE_EXCEPTIONS_STACKSIZE),)
  USE_EXCEPTIONS_STACKSIZE = 0x400
endif

# Enables the use of FPU (no, softfp, hard).
ifeq ($(USE_FPU),)
  USE_FPU = hard
endif

# FPU-related options.
ifeq ($(USE_FPU_OPT),)
  USE_FPU_OPT = -mfloat-abi=$(USE_FPU) -mfpu=fpv4-sp-d16
endif

#
# Architecture or project specific options
##############################################################################

##############################################################################
# Project, target, sources and paths
#

# Define project name here
PROJECT = ch
BOARD = NUCLEO476

# Target settings.
MCU  = cortex-m4

# Imported source files and paths.
MY_DIRNAME=../../../ChibiOS_stable
ifneq "$(wildcard $(MY_DIRNAME) )" ""
   RELATIVE=../../..
else
  RELATIVE=../..
endif

CHIBIOS  := $(RELATIVE)/ChibiOS_20.3_stable
CONFDIR  := ./cfg
BUILDDIR := ./build
DEPDIR   := ./.dep
STMSRC = $(RELATIVE)/COMMON/stm
VARIOUS = $(RELATIVE)/COMMON/various
USBD_LIB = $(VARIOUS)/Chibios-USB-Devices
ETL_LIB = ../../../../etl/include

# Licensing files.
include $(CHIBIOS)/os/license/license.mk
# Startup files.
include $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC/mk/startup_stm32l4xx.mk
# HAL-OSAL files (optional).
include $(CHIBIOS)/os/hal/hal.mk
include $(CHIBIOS)/os/hal/ports/STM32/STM32L4xx/platform.mk
include cfg/board.mk
include $(CHIBIOS)/os/hal/osal/rt-nil/osal.mk
# RTOS files (optional).
include $(CHIBIOS)/os/rt/rt.mk
include $(CHIBIOS)/os/common/ports/ARMCMx/compilers/GCC/mk/port_v7m.mk
# Auto-build files in ./source recursively.
include $(CHIBIOS)/tools/mk/autobuild.mk
# Other files (optional).


# Define linker script file here
LDSCRIPT= $(STARTUPLD)/STM32L476xG.ld

# C sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CSRC = $(ALLCSRC) \
       $(CHIBIOS)/os/various/syscalls.c \
       $(VARIOUS)/stdutil.c \
       $(VARIOUS)/printf.c \
       $(VARIOUS)/microrl/microrlShell.c \
       $(VARIOUS)/microrl/microrl.c \
       $(VARIOUS)/hd44780.c



# C++ sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CPPSRC = $(ALLCPPSRC)

# List ASM source files here.
ASMSRC = $(ALLASMSRC)

# List ASM with preprocessor source files here.
ASMXSRC = $(ALLXASMSRC)

# Inclusion directories.
INCDIR = $(CONFDIR) $(ALLINC) $(VARIOUS) $(STMSRC) \
         $(USBD_LIB) $(ETL_LIB) 

# Define C warning options here.
CWARN = -Wall -Wextra -Wundef -Wstrict-prototypes

# Define C++ warning options here
CPPWARN = -Wall -Wextra -Wundef

#
# Compiler settings
##############################################################################
LD   = $(TRGT)g++
##############################################################################
# Start of user section
#

# List all user C define here, like -D_DEBUG=1
UDEFS = -DGIT_BRANCH="$(GIT_BRANCH)" -DGIT_TAG="$(GIT_TAG)" -DGIT_SHA="$(GIT_SHA)" -DBUILD="$(BUILD)"

# Define ASM defines here
UADEFS =

# List all user directories here
UINCDIR =

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS = -lm -lstdc++

#
# End of user defines
##############################################################################

##############################################################################
# Common rules
#

RULESPATH = $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC/mk
include $(RULESPATH)/arm-none-eabi.mk
include $(RULESPATH)/rules.mk
$(OBJS): $(CONFDIR)/board.h


$(CONFDIR)/board.h: $(CONFDIR)/board.cfg
	boardGen.pl --no-pp-line --no-adcp-in $<  $@


stflash: all
	@echo write $(BUILDDIR)/$(PROJECT).bin to flash memory
	/usr/local/bin/st-flash write  $(BUILDDIR)/$(PROJECT).bin 0x08000000
	@echo Done

flash: all
	@echo write $(BUILDDIR)/$(PROJECT).bin to flash memory
	bmpflash  $(BUILDDIR)/$(PROJECT).elf
	@echo Done

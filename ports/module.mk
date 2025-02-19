##########################################################################
#
#	Make include for ports
#	
#   Platform: MPOS rev.3.0	
#
#
##########################################################################

ifndef USE_MCU_DETECT
USE_MCU_DETECT  :=y
endif

# local sources
local_as_src-y	:=
local_c_src-y 	:= 
local_cpp_src-y	:=   
local_h_src-y 	:=

local_as_src-$(USE_CRC32)		+= crc32.S

local_c_src-$(USE_CRC32)		+= crc32.c

local_cpp_src-$(USE_CACHE)		+= cache.cpp
local_cpp_src-$(USE_TMOS_STDLIB)+= tmos_stdlib.cpp
local_cpp_src-$(USE_MCU_DETECT)	+= mcu_id.cpp

local_h_src-y 					+= tmos.h tmos_cfg.h tmos_inc.h tmos_types.h 
local_h_src-$(USE_MCU_DETECT)	+= mcu_id.h 
local_h_src-$(USE_TMOS_STDLIB)	+= tmos_stdlib.h

local_h_src-$(USE_CACHE)		+= cache.h
local_h_src-$(USE_CRC32)		+= crc32.h

#updating global variables
as_sources 	+= $(call changepath,$(local_as_src-y))
c_sources  	+= $(call changepath,$(local_c_src-y))
cpp_sources += $(call changepath,$(local_cpp_src-y))
h_sources  	+= $(call changepath,$(local_h_src-y))

#add current directory to include path
inc_dirs += $(subdirectory) 

#submodules
ifeq ($(CFG_CORE), cortex-m4)
port_core := cortex-m3
else
port_core := $(CFG_CORE)
endif
local_modules-y := $(port_core)

sub_modules := $(call changepath,$(local_modules-y))
modules += $(sub_modules)

include $(addsuffix /module.mk,$(sub_modules))

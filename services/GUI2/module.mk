##########################################################################
#
#	Make include for GUI2
#	
#   Platform: MPOS rev.2.3
#
#   www.thumbos.com
#
##########################################################################

# local sources
local_as_src-y	:=
local_c_src-y 	:=
local_cpp_src-y	:=
local_h_src-y 	:=

local_cpp_src-y	+= gui_drv.cpp stdgui.cpp gdbg_opt.cpp
local_cpp_src-y += gobject.cpp gcontainer.cpp basic_shapes.cpp

local_h_src-y 	+= gui_drv.h stdgui.h gdbg_opt.h
local_h_src-y 	+= gobject.h gcontainer.h basic_shapes.h

#updating global variables
as_sources 	+= $(call changepath,$(local_as_src-y))
c_sources  	+= $(call changepath,$(local_c_src-y))
cpp_sources += $(call changepath,$(local_cpp_src-y))
h_sources  	+= $(call changepath,$(local_h_src-y))


#add current directory to include path
inc_dirs += $(subdirectory)

#submodules
local_modules-y := fonts lcds api_windows controls

sub_modules := $(call changepath,$(local_modules-y))
modules += $(sub_modules)

include $(addsuffix /module.mk,$(sub_modules))

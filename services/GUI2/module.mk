##########################################################################
#
#	Make include for GUI
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
local_cpp_src-y += gwindow.cpp gobject.cpp gcontainer.cpp gbutton.cpp gtext.cpp
local_cpp_src-y += gdialog.cpp gradio.cpp gedit.cpp gfloating_text.cpp
local_cpp_src-y += gfloating_button.cpp gmenu.cpp gscroll.cpp gmsgbox.cpp
local_cpp_src-y += glistbox.cpp message.cpp gdowait.cpp
local_cpp_src-y += gedit_vkb.cpp gclockview.cpp

local_h_src-y 	+= gui_drv.h stdgui.h gdbg_opt.h
local_h_src-y 	+= gwindow.h gobject.h gcontainer.h gbutton.h gtext.h message.h
local_h_src-y 	+= gdialog.h gradio.h gedit.h gfloating_text.h
local_h_src-y 	+= gfloating_button.h gmenu.h gscroll.h gmsgbox.h glistbox.h gdowait.h
local_h_src-y 	+= gedit_vkb.h gclockview.h 

#updating global variables
as_sources 	+= $(call changepath,$(local_as_src-y))
c_sources  	+= $(call changepath,$(local_c_src-y))
cpp_sources += $(call changepath,$(local_cpp_src-y))
h_sources  	+= $(call changepath,$(local_h_src-y))


#add current directory to include path
inc_dirs += $(subdirectory)

#submodules
local_modules-y := fonts lcds

sub_modules := $(call changepath,$(local_modules-y))
modules += $(sub_modules)

include $(addsuffix /module.mk,$(sub_modules))

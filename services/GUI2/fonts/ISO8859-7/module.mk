##########################################################################
#
#	Make include for GUI2/fonts/ISO8859-7
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

local_cpp_src-y	+= FNT5x7-ISO8859-7.cpp 
local_cpp_src-y	+= FNT7x9-ISO8859-7.cpp 
local_cpp_src-y	+= FNT10x12-ISO8859-7.cpp 
local_cpp_src-y	+= FNT10x14-ISO8859-7.cpp 
local_cpp_src-y	+= FNT10x21-ISO8859-7.cpp 

#updating global variables
as_sources 	+= $(call changepath,$(local_as_src-y))
c_sources  	+= $(call changepath,$(local_c_src-y))
cpp_sources += $(call changepath,$(local_cpp_src-y))
h_sources  	+= $(call changepath,$(local_h_src-y))



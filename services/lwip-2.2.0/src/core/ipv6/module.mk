##########################################################################
#
#	Make include for lwIP/src/core/ipv4
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

local_c_src-y 	+= dhcp6.c ethip6.c icmp6.c inet6.c    
local_c_src-y 	+= ip6_addr.c ip6_frag.c ip6.c
local_c_src-y 	+= mld6.c nd6.c

#updating global variables
as_sources 	+= $(call changepath,$(local_as_src-y))
c_sources  	+= $(call changepath,$(local_c_src-y))
cpp_sources += $(call changepath,$(local_cpp_src-y))
h_sources  	+= $(call changepath,$(local_h_src-y))


#add current directory to include path
#inc_dirs += $(subdirectory)

#submodules
local_modules-y := 

sub_modules := $(call changepath,$(local_modules-y))
modules += $(sub_modules)

include $(addsuffix /module.mk,$(sub_modules))

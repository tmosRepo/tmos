##########################################################################
#
#	Make include for lwIP/src/core
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

local_c_src-y 	+= altcp_alloc.c altcp_tcp.c altcp.c
local_c_src-y 	+= def.c dns.c inet_chksum.c init.c ip.c mem.c memp.c netif.c pbuf.c raw.c 
local_c_src-y 	+= stats.c sys.c tcp_in.c tcp_out.c tcp.c timeouts.c udp.c

#updating global variables
as_sources 	+= $(call changepath,$(local_as_src-y))
c_sources  	+= $(call changepath,$(local_c_src-y))
cpp_sources += $(call changepath,$(local_cpp_src-y))
h_sources  	+= $(call changepath,$(local_h_src-y))


#add current directory to include path
#inc_dirs += $(subdirectory)

#submodules
local_modules-y := ipv4 #ipv6

sub_modules := $(call changepath,$(local_modules-y))
modules += $(sub_modules)

include $(addsuffix /module.mk,$(sub_modules))

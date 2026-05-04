##########################################################################
#
#	Make include for lwIP/src/include
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

lwip_h_src 	+= lwip/opt.h lwip/debug.h lwip/arch.h lwip/netif.h lwip/err.h 
lwip_h_src 	+= lwip/ip_addr.h lwip/def.h lwip/pbuf.h lwip/tcp.h lwip/sys.h 
lwip_h_src 	+= lwip/mem.h lwip/ip.h lwip/icmp.h lwip/inet.h lwip/dns.h

lwip_h_src 	+= lwip/ip4_addr.h lwip/ip6_addr.h lwip/stats.h lwip/memp.h
lwip_h_src 	+= lwip/priv/memp_std.h lwip/priv/memp_priv.h lwip/priv/mem_priv.h
lwip_h_src 	+= lwip/apps/mdns.h lwip/apps/mdns_opts.h
#updating global variables
as_sources 	+= $(call changepath,$(local_as_src-y))
c_sources  	+= $(call changepath,$(local_c_src-y))
cpp_sources += $(call changepath,$(local_cpp_src-y))
h_sources  	+= $(call changepath,$(local_h_src-y))


#add current directory to include path
inc_dirs += $(subdirectory)

#submodules
local_modules-y := compat lwip netif

sub_modules := $(call changepath,$(local_modules-y))
modules += $(sub_modules)

include $(addsuffix /module.mk,$(sub_modules))

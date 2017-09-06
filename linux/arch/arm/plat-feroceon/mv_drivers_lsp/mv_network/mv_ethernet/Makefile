#
# Makefile for the Marvell Gigabit Ethernet driver
#
ifeq ($(CONFIG_ARCH_FEROCEON_ORION),y)
	include $(srctree)/arch/arm/mach-feroceon-orion/config/mvRules.mk
endif
 
ifeq ($(CONFIG_ARCH_FEROCEON_KW),y)
	include $(srctree)/arch/arm/mach-feroceon-kw/config/mvRules.mk
endif

ifeq ($(CONFIG_ARCH_FEROCEON_MV78XX0),y)
	include $(srctree)/arch/arm/mach-feroceon-mv78xx0/config/mvRules.mk
endif

obj-$(CONFIG_MV_ETHERNET) += mv_netdev.o mv_ethernet.o
obj-$(CONFIG_MV_ETH_PROC) += mv_eth_proc.o
ifeq ($(CONFIG_EFM_ROUTER_PATCH),y)
# obj-y += mv_gtw_igmp.o
else
obj-$(CONFIG_MV_GATEWAY) +=  mv_gateway.o
obj-$(CONFIG_MV_GTW_IGMP) += mv_gtw_igmp.o
endif
obj-$(CONFIG_MV_ETH_TOOL) += mv_eth_tool.o
obj-$(CONFIG_EFM_ROUTER_PATCH) += mv_salsa2_api.o


$(info include $(notdir $(lastword $(MAKEFILE_LIST))))

TARGET = $(shell cat $(USERAPPS_ROOT)/.product_name)

.PHONY : DUMMY
DUMMY:

include $(USERAPPS_ROOT)/rootfs/clones/$(TARGET)/clone_info.mk

ifeq ($(.DEFAULT_GOAL),DUMMY)
.DEFAULT_GOAL:=all
endif

check_defined = $(strip $(foreach 1,$1, $(call __check_defined,$1,$(strip $(value 2)))))
__check_defined = $(if $(value $1),, $(error Undefined $1$(if $2, ($2))))

$(call check_defined,KERNEL_PATH KCONFIG_FILE_NAME)

MAKE_KERNEL:=kernel install
.PHONY : all $(MAKE_KERNEL)
all: $(MAKE_KERNEL)

.PHONY : before_make
before_make:
	$(MAKE) -C $(KERNEL_PATH) oldconfig

kernel: before_make
	$(MAKE) uImage -C $(KERNEL_PATH)

install:
	@echo -e "\tcopy rootfs/uImage"
	@cp $(KERNEL_PATH)/arch/arm/boot/uImage $(USERAPPS_ROOT)/rootfs/


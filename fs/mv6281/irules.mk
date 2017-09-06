include $(USERAPPS_ROOT)/config
-include $(USERAPPS_ROOT)/reg_config
include $(USERAPPS_ROOT)/rootfs/clone_info.mk
ifeq ($(USE_CUSTOM_VERSION),y)
include $(USERAPPS_ROOT)/rootfs/clones/$(TARGET)/version.mk
else
include $(USERAPPS_ROOT)/rootfs/version.mk
endif
include $(USERAPPS_ROOT)/lang_profile
-include $(USERAPPS_ROOT)/mkdefs


LINUX_KERNEL_DIR=$(USERAPPS_ROOT)/linux
ROOT_DIR:= root
PLUGIN_DIR:=./plugin

KERNEL := ./uImage
BOOTIMG := clones/$(TARGET)/u-boot-rd88f6281a_400rd_flash.bin
BOOTIMG_SPI := clones/$(TARGET)/uboot.spi.img

RAMDISK  = ramdisk
COMP := lzma

CGI_DEBUG := y

IMAGE_POSTFIX:=$(LANGUAGE_POSTFIX)

# specify ram device: you may change the device : ram1 ~ ram19 
RAM_DEV := /dev/ram4
RAMDEV_SIZE := 4096
SAVE_BLOCK_COUNT := 256

CHIPSET_APP_INSTALL_DIR:=
CLIB_DIR:=
IPTABLES_BIN_PATH:=
IPTABLES_BINS:=
IPTABLES_LIB_PATH:=
#IPTABLES_LIBS:=libipq.so libip4tc.so libxtables.so
TNTFS_MODULE_PATH:=
STRIP_OPTION:=
LDCONFIG_CMD:=
MAKE_FS_BIANRY_CMD:=

NET_MODULE:=

CONF_DIR := ../conf

$(TARGET): target.fs image

include $(USERAPPS_ROOT)/mkscripts/target.mk

# for reserving upgrade memory
save:
	dd if=/dev/zero bs=1k count=$(SAVE_BLOCK_COUNT) of=$(RAM_DEV)
	/sbin/mke2fs -c $(RAM_DEV) $(SAVE_BLOCK_COUNT) -v -i 1024
	dd if=$(RAM_DEV) bs=1k count=$(SAVE_BLOCK_COUNT) of=save.fs
	gzip save.fs

post_targetfs:
	@echo -e "\t--->Post processing..."
	@cp -ra clones/$(TARGET)/home $(ROOT_DIR)/
	@cp -ra $(USERAPPS_ROOT)/$(BUSYBOX_DIR)/_install/usr/bin/* $(ROOT_DIR)/usr/bin
	@cp -ra $(USERAPPS_ROOT)/mv6281_app/lib/libmv6281.so $(ROOT_DIR)/lib

ifneq ($(USE_NO_KMODULE),y)
	@cp -ra $(USERAPPS_ROOT)/nvmod/$(PROJECT_ID)/* $(ROOT_DIR)/lib/modules
endif
ifeq ($(USE_UPNP),y)
	@cp -ra $(USERAPPS_ROOT)/upnp/libupnp-1.2.1a/upnp/bin/mipsel-linux-uclibc/* $(ROOT_DIR)/lib
	@cp -ra /opt/buildroot-gdb/lib/libpthread-0.9.28.so $(ROOT_DIR)/lib/libpthread.so.0
	@cp -ra $(USERAPPS_ROOT)/upnp/igd/upnpd $(ROOT_DIR)/bin
endif

	@cp -ra $(USERAPPS_ROOT)/fs/lib/$(CPU_ID)/* $(ROOT_DIR)/lib
	@ln -s /lib/ld-2.8.so $(ROOT_DIR)/lib/ld-linux.so.3
	@ln -s /lib/libc-2.8.so $(ROOT_DIR)/lib/libc.so.6
	@ln -s /lib/libcrypt-2.8.so $(ROOT_DIR)/lib/libcrypt.so.1
	@ln -s /lib/libdl-2.8.so $(ROOT_DIR)/lib/libdl.so.2
	@ln -s /lib/libgcc_s.so.1 $(ROOT_DIR)/lib/libgcc_s.so
	@ln -s /lib/libm-2.8.so $(ROOT_DIR)/lib/libm.so.6
	@ln -s /lib/libnsl-2.8.so $(ROOT_DIR)/lib/libnsl.so.1
	@ln -s /lib/libpthread-2.8.so $(ROOT_DIR)/lib/libpthread.so.0
	@ln -s /lib/libresolv-2.8.so $(ROOT_DIR)/lib/libresolv.so.2
	@ln -s /lib/librt-2.8.so $(ROOT_DIR)/lib/librt.so.1
	@ln -s /lib/libutil-2.8.so $(ROOT_DIR)/lib/libutil.so.1
	
	@mkdir $(ROOT_DIR)/dev/mtd

	@sudo mknod -m664 $(ROOT_DIR)/dev/ram b 1 1
	@sudo mknod -m664 $(ROOT_DIR)/dev/ram0 b 1 0
	@sudo mknod -m664 $(ROOT_DIR)/dev/ram1 b 1 1
	@sudo mknod -m664 $(ROOT_DIR)/dev/ram2 b 1 2
	@sudo mknod -m664 $(ROOT_DIR)/dev/ram3 b 1 3
	@sudo mknod -m664 $(ROOT_DIR)/dev/mtr0 c 250 0
	@sudo mknod -m664 $(ROOT_DIR)/dev/mtd/0 c 90 0
	@sudo mknod -m664 $(ROOT_DIR)/dev/mtd/0ro c 90 1
	@sudo mknod -m664 $(ROOT_DIR)/dev/mtd/1 c 90 2
	@sudo mknod -m664 $(ROOT_DIR)/dev/mtd/1ro c 90 3
	@sudo mknod -m664 $(ROOT_DIR)/dev/mtd/2 c 90 4
	@sudo mknod -m664 $(ROOT_DIR)/dev/mtd/2ro c 90 5
	@sudo mknod -m664 $(ROOT_DIR)/dev/mtd/3 c 90 6
	@sudo mknod -m664 $(ROOT_DIR)/dev/mtd/3ro c 90 7
	@sudo mknod -m664 $(ROOT_DIR)/dev/mtd/4 c 90 8
	@sudo mknod -m664 $(ROOT_DIR)/dev/mtd/4ro c 90 9
	@sudo mknod -m664 $(ROOT_DIR)/dev/mtd/5 c 90 10
	@sudo mknod -m664 $(ROOT_DIR)/dev/mtd/5ro c 90 11

	@sudo mknod -m664 $(ROOT_DIR)/dev/flash0 c 200 0
	@sudo mknod -m664 $(ROOT_DIR)/dev/ppp c 108 0
	@sudo mknod -m664 $(ROOT_DIR)/dev/rdm0 c 254 0

	@mkdir $(ROOT_DIR)/dev/cua
	@sudo mknod -m664 $(ROOT_DIR)/dev/cua/0 c 5 64
	@sudo mknod -m664 $(ROOT_DIR)/dev/cua/1 c 5 65

	@mkdir $(ROOT_DIR)/dev/tts
	@sudo mknod -m664 $(ROOT_DIR)/dev/tts/0 c 4 64
	@sudo mknod -m664 $(ROOT_DIR)/dev/tts/1 c 4 65

	@sudo mknod -m664 $(ROOT_DIR)/dev/gpio c 252 0

	@mkdir $(ROOT_DIR)/dev/pts
	@sudo mknod -m664 $(ROOT_DIR)/dev/pts/0 c 136 0
	@sudo mknod -m664 $(ROOT_DIR)/dev/pts/1 c 136 1
	@sudo mknod -m664 $(ROOT_DIR)/dev/pts/2 c 136 2
	@sudo mknod -m664 $(ROOT_DIR)/dev/pts/3 c 136 3
	@sudo mknod -m664 $(ROOT_DIR)/dev/pts/4 c 136 4
	@sudo mknod -m664 $(ROOT_DIR)/dev/pts/5 c 136 5

	@mkdir $(ROOT_DIR)/dev/pty
	@sudo mknod -m664 $(ROOT_DIR)/dev/pty/m0 c 2 0
	@sudo mknod -m664 $(ROOT_DIR)/dev/pty/m1 c 2 1
	@sudo mknod -m664 $(ROOT_DIR)/dev/pty/m2 c 2 2
	@sudo mknod -m664 $(ROOT_DIR)/dev/pty/m3 c 2 3
	@sudo mknod -m664 $(ROOT_DIR)/dev/pty/m4 c 2 4
	@sudo mknod -m664 $(ROOT_DIR)/dev/pty/m5 c 2 5
	@sudo mknod -m664 $(ROOT_DIR)/dev/pty/m6 c 2 6

	@sudo mknod -m664 $(ROOT_DIR)/dev/ptyp0 c 2 0
	@sudo mknod -m664 $(ROOT_DIR)/dev/ptyp1 c 2 1
	@sudo mknod -m664 $(ROOT_DIR)/dev/ptyp2 c 2 2
	@sudo mknod -m664 $(ROOT_DIR)/dev/ptyp3 c 2 3
	@sudo mknod -m664 $(ROOT_DIR)/dev/ptyp4 c 2 4

	@sudo mknod -m664 $(ROOT_DIR)/dev/ttyp0 c 3 0
	@sudo mknod -m664 $(ROOT_DIR)/dev/ttyp1 c 3 1
	@sudo mknod -m664 $(ROOT_DIR)/dev/ttyp2 c 3 2
	@sudo mknod -m664 $(ROOT_DIR)/dev/ttyp3 c 3 3
	@sudo mknod -m664 $(ROOT_DIR)/dev/ttyp4 c 3 4

	@sudo mknod -m664 $(ROOT_DIR)/dev/tty c 5 0
	@sudo mknod -m664 $(ROOT_DIR)/dev/console c 5 1
	@sudo mknod -m664 $(ROOT_DIR)/dev/ptmx c 5 2

	@sudo mknod -m664 $(ROOT_DIR)/dev/urandom c 1 9
	@sudo mknod -m664 $(ROOT_DIR)/dev/random c 1 8
	@sudo mknod -m664 $(ROOT_DIR)/dev/full c 1 7
	@sudo mknod -m664 $(ROOT_DIR)/dev/zero c 1 5
	@sudo mknod -m664 $(ROOT_DIR)/dev/port c 1 4
	@sudo mknod -m664 $(ROOT_DIR)/dev/null c 1 3
	@sudo mknod -m664 $(ROOT_DIR)/dev/kmem c 1 2
	@sudo mknod -m664 $(ROOT_DIR)/dev/mem c 1 1

	@mkdir $(ROOT_DIR)/dev/mtdblock
	@sudo mknod -m664 $(ROOT_DIR)/dev/mtdblock/0 c 31 0
	@sudo mknod -m664 $(ROOT_DIR)/dev/mtdblock/1 c 31 1
	@sudo mknod -m664 $(ROOT_DIR)/dev/mtdblock/2 c 31 2
	@sudo mknod -m664 $(ROOT_DIR)/dev/mtdblock/3 c 31 3
	@sudo mknod -m664 $(ROOT_DIR)/dev/mtdblock/4 c 31 4
	@sudo mknod -m664 $(ROOT_DIR)/dev/mtdblock/5 c 31 5

	@sudo mknod -m664 $(ROOT_DIR)/dev/mv_salsa2 c 201 16

	@sudo mknod -m664 $(ROOT_DIR)/dev/btns c 10 72

	@rm -rf `find ./$(ROOT_DIR) -name 'CVS'` 

	#Squash File System
	@rm -f $(RAMDISK)
	@./mksquashfs $(ROOT_DIR) $(RAMDISK)
#	rm -rf $(LINUX_KERNEL_DIR)/usr/.initramfs_data.cpio.gz.d
#	rm -rf $(LINUX_KERNEL_DIR)/usr/initramfs_data.cpio.gz
#	rm -rf $(LINUX_KERNEL_DIR)/usr/initramfs_data.o
#	rm -rf $(LINUX_KERNEL_DIR)/usr/built-in.o

#	make -C $(LINUX_KERNEL_DIR)
#	make -C $(LINUX_KERNEL_DIR) modules_install


FIRMWARE_FILENAME:=t24000_ml_10_008.bin

FIRMWARE_TYPE:=kernel

# standalone : bootloader + kernel
# FIRMWARE_TYPE:=standalone
# CONFIG_MTD_KERNEL_PART_SIZ := 0xD2000
CONFIG_MTD_KERNEL_PART_SIZ := 0xD3000
# For Driver 2.3.0.0
#CONFIG_MTD_KERNEL_PART_SIZ := 0xD4000
# For 3.5.0.0 SDK
#CONFIG_MTD_KERNEL_PART_SIZ := 0xF4000

R_DIR:=./clones/$(PRODUCT_ID)
BOOT_SIG_FILE:=u-boot.sig.bin
BOOT_FILE:=u-boot-rd88f6281a_400rd_flash.bin
BOOT_TMP_FILE:=u-boot.tmp
FLASH_SIZE:=0x800000
BOOT_SIZE:=0x40000
SAVE_SIZE:=0x20000
SECRET_KEY:=0x19283745
SECTOR_SIZE:=0x10000
MAX_FIRMWARE_SIZE:=$(FLASH_SIZE)

image:
	@echo "---------------------------------------------------------------------------------"

	@./utils/bootds -o $(R_DIR)/$(BOOT_SIG_FILE) -a $(PRODUCT_ID) -d 1 -i 192.168.0.1
	@cp $(R_DIR)/$(BOOT_FILE) $(R_DIR)/$(BOOT_TMP_FILE)
	@./utils/addpad $(R_DIR)/$(BOOT_TMP_FILE) 0x3FF80 0xFF
	@cat $(R_DIR)/$(BOOT_SIG_FILE) >> $(R_DIR)/$(BOOT_TMP_FILE)
	@mv -f $(R_DIR)/$(BOOT_TMP_FILE) $(R_DIR)/$(PRODUCT_ID)_xboot.bin

	./mknas -a $(PRODUCT_ID) -v $(MAJOR_VER)_$(MINOR_VER) -k $(KERNEL) -c $(RAMDISK) -m $(FLASH_SIZE) -p $(SECRET_KEY) -o $(FIRMWARE_FILENAME) -b $(BOOT_SIZE) -s $(SECTOR_SIZE) -e $(SAVE_SIZE)
#	@cp $(FIRMWARE_FILENAME) $(R_DIR)/
	@./firmware_size_check.sh $(FIRMWARE_FILENAME) $(MAX_FIRMWARE_SIZE)
	@cat $(R_DIR)/$(PRODUCT_ID)_xboot.bin > $(FIRMWARE_FILENAME).burn
	@cat $(FIRMWARE_FILENAME) >> $(FIRMWARE_FILENAME).burn

	@mv $(FIRMWARE_FILENAME) binary
	@mv $(FIRMWARE_FILENAME).burn binary
	@rm -rf tmp.bin
	@echo "Done"
	@echo "---------------------------------------------------------------------------------"


clean:
	rm -rf save.fs.gz initrd.gz

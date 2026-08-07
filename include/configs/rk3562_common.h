/* SPDX-License-Identifier:     GPL-2.0+ */
/*
 * (C) Copyright 2022 Rockchip Electronics Co., Ltd
 *
 */

#ifndef __CONFIG_RK3562_COMMON_H
#define __CONFIG_RK3562_COMMON_H

#define CFG_CPUID_OFFSET		0xa
#define CFG_IRAM_BASE			0xfe480000

#include "rockchip-common.h"

#define CFG_SYS_SDRAM_BASE		0
/* Used by board_get_usable_ram_top(), space below the 4G address boundary */
#define SDRAM_MAX_SIZE			0xfc000000

#define COUNTER_FREQUENCY		24000000

#define GICD_BASE			0xfe901000
#define GICC_BASE			0xfe902000

/* secure otp */
#define OTP_UBOOT_ROLLBACK_OFFSET	0x350
#define OTP_UBOOT_ROLLBACK_WORDS	2	/* 64 bits, 2 words */
#define OTP_ALL_ONES_NUM_BITS		32
#define OTP_SECURE_BOOT_ENABLE_ADDR	0x20
#define OTP_SECURE_BOOT_ENABLE_SIZE	1
#define OTP_RSA_HASH_ADDR		0x180
#define OTP_RSA_HASH_SIZE		32

/* rockusb */
#define CONFIG_ROCKUSB_G_DNL_PID	0x350d

#ifndef ROCKCHIP_DEVICE_SETTINGS
#define ROCKCHIP_DEVICE_SETTINGS
#endif

#define ENV_MEM_LAYOUT_SETTINGS \
	"scriptaddr=0x00c00000\0" \
	"pxefile_addr_r=0x00e00000\0" \
	"fdt_addr_r=0x08300000\0" \
	"kernel_addr_r=0x00400000\0" \
	"kernel_comp_addr_r=0x04080000\0" \
	"ramdisk_addr_r=0x0a200000\0"

#define CFG_EXTRA_ENV_SETTINGS \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
	ENV_MEM_LAYOUT_SETTINGS \
	ROCKCHIP_DEVICE_SETTINGS \
	"boot_targets=" BOOT_TARGETS "\0"

#endif

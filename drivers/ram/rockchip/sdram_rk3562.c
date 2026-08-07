// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2022 Rockchip Electronics Co., Ltd.
 */

#include <dm.h>
#include <ram.h>
#include <asm/arch-rockchip/sdram.h>

#define PMU_GRF_BASE			0xff010000
#define OS_REG2_REG			0x208

static int rk3562_dmc_get_info(struct udevice *dev, struct ram_info *info)
{
	info->base = CFG_SYS_SDRAM_BASE;
	info->size = rockchip_sdram_size(PMU_GRF_BASE + OS_REG2_REG);

	return 0;
}

static struct ram_ops rk3562_dmc_ops = {
	.get_info = rk3562_dmc_get_info,
};

static const struct udevice_id rk3562_dmc_ids[] = {
	{ .compatible = "rockchip,rk3562-dmc" },
	{ }
};

U_BOOT_DRIVER(rockchip_rk3562_dmc) = {
	.name = "rockchip_rk3562_dmc",
	.id = UCLASS_RAM,
	.of_match = rk3562_dmc_ids,
	.ops = &rk3562_dmc_ops,
};

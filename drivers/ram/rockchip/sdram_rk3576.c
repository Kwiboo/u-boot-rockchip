// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2024 Rockchip Electronics Co., Ltd.
 */

#include <config.h>
#include <dm.h>
#include <ram.h>
#include <syscon.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/grf_rk3576.h>
#include <asm/arch-rockchip/sdram.h>

struct dram_info {
	struct ram_info info;
	struct rk3576_pmu1grf *pmugrf;
};

static int rk3576_dmc_probe(struct udevice *dev)
{
	struct dram_info *priv = dev_get_priv(dev);

	priv->pmugrf = syscon_get_first_range(ROCKCHIP_SYSCON_PMUGRF);

	/*
	 * On a 16GB board the DDR ATAG reports:
	 * start 0x40000000, size 0x400000000
	 * While the size value from the pmugrf below reports
	 * pmugrf->osreg2: 0x400000000
	 * pmugrf->osreg4:  0x10000000
	 * So it seems only osreg2 is responsible for the ram size.
	 */
	priv->info.base = CFG_SYS_SDRAM_BASE;
	priv->info.size =
		rockchip_sdram_size((phys_addr_t)&priv->pmugrf->os_reg[2]);

	return 0;
}

static int rk3576_dmc_get_info(struct udevice *dev, struct ram_info *info)
{
	struct dram_info *priv = dev_get_priv(dev);

	*info = priv->info;

	return 0;
}

static struct ram_ops rk3576_dmc_ops = {
	.get_info = rk3576_dmc_get_info,
};

static const struct udevice_id rk3576_dmc_ids[] = {
	{ .compatible = "rockchip,rk3576-dmc" },
	{ }
};

U_BOOT_DRIVER(dmc_rk3576) = {
	.name = "rockchip_rk3576_dmc",
	.id = UCLASS_RAM,
	.of_match = rk3576_dmc_ids,
	.ops = &rk3576_dmc_ops,
	.probe = rk3576_dmc_probe,
	.priv_auto = sizeof(struct dram_info),
};

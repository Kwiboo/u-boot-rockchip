// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021 Rockchip Electronics Co., Ltd.
 */

#include <dm.h>
#include <ram.h>
#include <asm/arch-rockchip/grf_rk3588.h>
#include <asm/arch-rockchip/sdram.h>

struct dram_info {
	struct ram_info info;
};

int rockchip_ram_get_info(struct ram_info *ram)
{
	struct rk3588_pmu1grf * const pmu1grf = (void *)RK3588_PMU1GRF_BASE;

	ram->base = CFG_SYS_SDRAM_BASE;
	ram->size =
		rockchip_sdram_size((phys_addr_t)&pmu1grf->os_reg[2]) +
		rockchip_sdram_size((phys_addr_t)&pmu1grf->os_reg[4]);

	return 0;
}

static int rk3588_dmc_probe(struct udevice *dev)
{
	struct dram_info *priv = dev_get_priv(dev);

	return rockchip_ram_get_info(&priv->info);
}

static int rk3588_dmc_get_info(struct udevice *dev, struct ram_info *info)
{
	struct dram_info *priv = dev_get_priv(dev);

	*info = priv->info;

	return 0;
}

static struct ram_ops rk3588_dmc_ops = {
	.get_info = rk3588_dmc_get_info,
};

static const struct udevice_id rk3588_dmc_ids[] = {
	{ .compatible = "rockchip,rk3588-dmc" },
	{ }
};

U_BOOT_DRIVER(dmc_rk3588) = {
	.name = "rockchip_rk3588_dmc",
	.id = UCLASS_RAM,
	.of_match = rk3588_dmc_ids,
	.ops = &rk3588_dmc_ops,
	.probe = rk3588_dmc_probe,
	.priv_auto = sizeof(struct dram_info),
};

/*
 * (C) Copyright 2022 Rockchip Electronics Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <syscon.h>
#include <linux/err.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/cru_rk3562.h>

int rockchip_get_clk(struct udevice **devp)
{
	return uclass_get_device_by_driver(UCLASS_CLK,
			DM_DRIVER_GET(rockchip_rk3562_cru), devp);
}

#if CONFIG_IS_ENABLED(CLK_SCMI)
int rockchip_get_scmi_clk(struct udevice **devp)
{
	return uclass_get_device_by_driver(UCLASS_CLK,
			DM_DRIVER_GET(scmi_clock), devp);
}
#endif

void *rockchip_get_cru(void)
{
	struct rk3562_clk_priv *priv;
	struct udevice *dev;
	int ret;

	ret = rockchip_get_clk(&dev);
	if (ret)
		return ERR_PTR(ret);

	priv = dev_get_priv(dev);

	return priv->cru;
}


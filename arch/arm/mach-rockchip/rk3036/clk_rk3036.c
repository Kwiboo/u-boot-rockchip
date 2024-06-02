// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <dm.h>
#include <asm/arch-rockchip/cru_rk3036.h>

int rockchip_get_clk(struct udevice **devp)
{
	return uclass_get_device_by_driver(UCLASS_CLK,
			DM_DRIVER_GET(rockchip_rk3036_cru), devp);
}

void *rockchip_get_cru(void)
{
	return RK3036_CRU_BASE;
}

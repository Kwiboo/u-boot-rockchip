// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2017 Rockchip Electronics Co., Ltd.
 */

#include <dm.h>
#include <asm/arch-rockchip/cru_px30.h>

int rockchip_get_clk(struct udevice **devp)
{
	return uclass_get_device_by_driver(UCLASS_CLK,
			DM_DRIVER_GET(rockchip_px30_cru), devp);
}

void *rockchip_get_cru(void)
{
	return PX30_CRU_BASE;
}

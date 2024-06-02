// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2017 Rockchip Electronics Co., Ltd
 */

#include <dm.h>
#include <asm/arch-rockchip/cru_rk3328.h>

int rockchip_get_clk(struct udevice **devp)
{
	return uclass_get_device_by_driver(UCLASS_CLK,
			DM_DRIVER_GET(rockchip_rk3328_cru), devp);
}

void *rockchip_get_cru(void)
{
	return RK3328_CRU_BASE;
}

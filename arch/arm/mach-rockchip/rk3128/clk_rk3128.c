// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Rockchip Electronics Co., Ltd
 */

#include <dm.h>
#include <asm/arch-rockchip/cru_rk3128.h>

int rockchip_get_clk(struct udevice **devp)
{
	return uclass_get_device_by_driver(UCLASS_CLK,
			DM_DRIVER_GET(rockchip_rk3128_cru), devp);
}

void *rockchip_get_cru(void)
{
	return RK3128_CRU_BASE;
}

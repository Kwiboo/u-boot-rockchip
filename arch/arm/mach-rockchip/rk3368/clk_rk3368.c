// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Rockchip Electronics Co., Ltd
 * Author: Andy Yan <andy.yan@rock-chips.org>
 */

#include <dm.h>
#include <asm/arch-rockchip/cru_rk3368.h>

int rockchip_get_clk(struct udevice **devp)
{
	return uclass_get_device_by_driver(UCLASS_CLK,
			DM_DRIVER_GET(rockchip_rk3368_cru), devp);
}

void *rockchip_get_cru(void)
{
	return RK3368_CRU_BASE;
}

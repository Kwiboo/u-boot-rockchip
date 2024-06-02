// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 * Author: Andy Yan <andy.yan@rock-chips.com>
 */

#include <dm.h>
#include <asm/arch-rockchip/cru_rv1108.h>

int rockchip_get_clk(struct udevice **devp)
{
	return uclass_get_device_by_driver(UCLASS_CLK,
			DM_DRIVER_GET(clk_rv1108), devp);
}

void *rockchip_get_cru(void)
{
	return RV1108_CRU_BASE;
}

// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2018 Fuzhou Rockchip Electronics Co., Ltd
 * Copyright (c) 2022 Edgeble AI Technologies Pvt. Ltd.
 */

#include <dm.h>
#include <asm/arch-rockchip/cru_rv1126.h>

int rockchip_get_clk(struct udevice **devp)
{
	return uclass_get_device_by_driver(UCLASS_CLK,
			DM_DRIVER_GET(rockchip_rv1126_cru), devp);
}

void *rockchip_get_cru(void)
{
	return RV1126_CRU_BASE;
}

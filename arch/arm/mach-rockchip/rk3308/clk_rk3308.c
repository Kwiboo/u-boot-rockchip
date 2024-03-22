// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2018 Fuzhou Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <asm/arch-rockchip/cru_rk3308.h>

void *rockchip_get_cru(void)
{
	return (void *)RK3308_CRU_BASE;
}

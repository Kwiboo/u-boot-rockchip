// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2017 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <asm/arch-rockchip/cru_rk3328.h>

void *rockchip_get_cru(void)
{
	return (void *)RK3328_CRU_BASE;
}

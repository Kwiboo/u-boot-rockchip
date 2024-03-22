// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2020 Rockchip Electronics Co., Ltd.
 */

#include <common.h>
#include <asm/arch-rockchip/cru_rk3588.h>

void *rockchip_get_cru(void)
{
	return (void *)RK3588_CRU_BASE;
}

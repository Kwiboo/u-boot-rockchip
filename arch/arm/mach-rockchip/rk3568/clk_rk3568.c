// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2021 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <asm/arch-rockchip/cru_rk3568.h>

void *rockchip_get_cru(void)
{
	return (void *)RK3568_CRU_BASE;
}

void *rockchip_get_pmucru(void)
{
	return (void *)RK3568_PMUCRU_BASE;
}

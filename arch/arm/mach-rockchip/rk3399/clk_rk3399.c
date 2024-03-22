// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <asm/arch-rockchip/cru.h>

void *rockchip_get_cru(void)
{
	return (void *)RK3399_CRU_BASE;
}

void *rockchip_get_pmucru(void)
{
	return (void *)RK3399_PMUCRU_BASE;
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021 Rockchip Electronics Co., Ltd
 */

#include <dm.h>
#include <syscon.h>
#include <asm/arch-rockchip/clock.h>

static const struct udevice_id rk3588_syscon_ids[] = {
	{ .compatible = "rockchip,rk3588-sys-grf", .data = ROCKCHIP_SYSCON_GRF },
	{ }
};

U_BOOT_DRIVER(syscon_rk3588) = {
	.name = "rk3588_syscon",
	.id = UCLASS_SYSCON,
	.of_match = rk3588_syscon_ids,
#if CONFIG_IS_ENABLED(OF_REAL)
	.bind = dm_scan_fdt_dev,
#endif
};

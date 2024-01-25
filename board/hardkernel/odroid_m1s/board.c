// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2024 Hardkernel Co., Ltd
 */

#include <mmc.h>
#include <net.h>
#include <vsprintf.h>

#ifdef CONFIG_MISC_INIT_R

/* Read first block 512 bytes from the first BOOT partition of eMMC
 * that stores device identifcation sting in UUID type 1, this string
 * is written in factory to give device seranl number and MAC address.
 */
static int odroid_setup_macaddr(void)
{
	struct mmc *mmc;
	struct blk_desc *desc;
	unsigned long mac_addr;
	unsigned long count;
	int ret;
	u8 buf[512];

	mmc = find_mmc_device(0);
	if (!mmc)
		return -ENODEV;

	desc = mmc_get_blk_desc(mmc);

	// Switch to the first BOOT partition
	ret = blk_select_hwpart_devnum(UCLASS_MMC, 0, 1);
	if (ret)
		return -EIO;

	count = blk_dread(desc, 0, 1, (void *)buf);

	// Switch back to USER partition
	ret = blk_dselect_hwpart(desc, 0);
	if (ret || count != 1)
		return -EIO;

	*(char *)(buf + 36) = 0;

	// Serial number
	env_set("serial#", (char *)buf);

	// MAC address
	mac_addr = cpu_to_be64(simple_strtoul((char *)buf + 24, NULL, 16)) >> 16;

	eth_env_set_enetaddr("ethaddr", (unsigned char *)&mac_addr);
	eth_env_set_enetaddr("eth1addr", (unsigned char *)&mac_addr);

	return 0;
}

extern int rockchip_setup_macaddr(void);

int rockchip_early_misc_init_r(void)
{
	int ret;

	ret = odroid_setup_macaddr();
	if (ret)
		ret = rockchip_setup_macaddr();

	return ret;
}
#endif

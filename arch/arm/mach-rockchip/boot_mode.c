// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <adc.h>
#include <bootstage.h>
#include <command.h>
#include <env.h>
#include <log.h>
#include <asm/io.h>
#include <asm/arch-rockchip/boot_mode.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <linux/printk.h>

#if (CONFIG_ROCKCHIP_BOOT_MODE_REG == 0)

int setup_boot_mode(void)
{
	return 0;
}

#else

static void set_back_to_bootrom_dnl_flag(void)
{
	/*
	 * These SOC_CON1 regs needs to be cleared before a reset or the
	 * BOOT_MODE_REG do not retain its value and it is not possible
	 * to reset to bootrom download mode once TF-A has been started.
	 *
	 * TF-A blobs for RK3568 already clear SOC_CON1 for PSCI reset.
	 * However, the TF-A blobs for RK3588 does not clear SOC_CON1.
	 */
	if (IS_ENABLED(CONFIG_ROCKCHIP_RK3568))
		writel(0x40000, 0xFDC20104);
	if (IS_ENABLED(CONFIG_ROCKCHIP_RK3588))
		writel(0xFFFF0000, 0xFD58A004);

	writel(BOOT_BROM_DOWNLOAD, CONFIG_ROCKCHIP_BOOT_MODE_REG);
}

static void rockchip_reset_to_dnl_mode(void)
{
	puts("entering download mode ...\n");
	set_back_to_bootrom_dnl_flag();
	do_reset(NULL, 0, 0, NULL);
}

#if IS_ENABLED(CONFIG_USB_FUNCTION_ROCKUSB)
void rkusb_set_reboot_flag(int flag)
{
	if (flag == 0x03)
		set_back_to_bootrom_dnl_flag();
	else
		writel(BOOT_NORMAL, CONFIG_ROCKCHIP_BOOT_MODE_REG);
}
#endif

/*
 * detect download key status by adc, most rockchip
 * based boards use adc sample the download key status,
 * but there are also some use gpio. So it's better to
 * make this a weak function that can be override by
 * some special boards.
 */
#define KEY_DOWN_MIN_VAL	0
#define KEY_DOWN_MAX_VAL	30

__weak int rockchip_dnl_key_pressed(void)
{
	unsigned int val;
	struct udevice *dev;
	struct uclass *uc;
	int ret;

	ret = uclass_get(UCLASS_ADC, &uc);
	if (ret)
		return false;

	ret = -ENODEV;
	uclass_foreach_dev(dev, uc) {
		if (!strncmp(dev->name, "saradc", 6)) {
			ret = adc_channel_single_shot(dev->name, 1, &val);
			break;
		}
	}

	if (ret == -ENODEV) {
		pr_warn("%s: no saradc device found\n", __func__);
		return false;
	} else if (ret) {
		pr_err("%s: adc_channel_single_shot fail!\n", __func__);
		return false;
	}

	if ((val >= KEY_DOWN_MIN_VAL) && (val <= KEY_DOWN_MAX_VAL))
		return true;
	else
		return false;
}

#if CONFIG_IS_ENABLED(ROCKCHIP_HANG_TO_BROM)
void show_boot_progress(int val)
{
	if (val == -BOOTSTAGE_ID_NEED_RESET)
		rockchip_reset_to_dnl_mode();
}
#endif

static void rockchip_dnl_mode_check(void)
{
	if (rockchip_dnl_key_pressed()) {
		puts("download key pressed, ");
		rockchip_reset_to_dnl_mode();
	}
}

int setup_boot_mode(void)
{
	void *reg = (void *)CONFIG_ROCKCHIP_BOOT_MODE_REG;
	int boot_mode = readl(reg);

	rockchip_dnl_mode_check();

	boot_mode = readl(reg);
	debug("%s: boot mode 0x%08x\n", __func__, boot_mode);

	/* Clear boot mode */
	writel(BOOT_NORMAL, reg);

	switch (boot_mode) {
	case BOOT_FASTBOOT:
		debug("%s: enter fastboot!\n", __func__);
		env_set("preboot", "setenv preboot; fastboot usb 0");
		break;
	case BOOT_UMS:
		debug("%s: enter UMS!\n", __func__);
		env_set("preboot", "setenv preboot; ums mmc 0");
		break;
	}

	return 0;
}

#endif

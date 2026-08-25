// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2022 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <dwc3-uboot.h>
#include <usb.h>
#include <asm/io.h>
#include <rockusb.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_USB_DWC3_GADGET) && !defined(CONFIG_DM_USB_GADGET)
static struct dwc3_device dwc3_device_data = {
	.maximum_speed = USB_SPEED_HIGH,
	.base = 0xfe500000,
	.dr_mode = USB_DR_MODE_PERIPHERAL,
	.hsphy_mode = USBPHY_INTERFACE_MODE_UTMIW,
	.index = 0,
	.dis_u2_susphy_quirk = 1,
};

int rkusb_dev_bind_to_udc_data(struct udevice *dev)
{
	if (IS_ERR_OR_NULL(dev))
		return -EINVAL;

	dwc3_device_data.dev = dev;

	return 0;
}

int board_usb_init(int index, enum usb_init_type init)
{
	return dwc3_uboot_init(&dwc3_device_data);
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	dwc3_uboot_exit(index);
	return 0;
}
#endif

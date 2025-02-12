// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright Contributors to the U-Boot project.

#include <dm.h>
#include <dm/device-internal.h>
#include <linux/usb/otg.h>

static int rockchip_dwc2_bind(struct udevice *dev)
{
	int ret = 0;

	if (IS_ENABLED(CONFIG_USB_GADGET_DWC2_OTG) &&
	    (usb_get_dr_mode(dev_ofnode(dev)) == USB_DR_MODE_PERIPHERAL ||
	     usb_get_dr_mode(dev_ofnode(dev)) == USB_DR_MODE_OTG))
		ret = device_bind(dev->parent, DM_DRIVER_GET(dwc2_udc_otg),
				  dev->name, NULL, dev_ofnode(dev), NULL);
	else if (IS_ENABLED(CONFIG_USB_DWC2))
		ret = device_bind(dev->parent, DM_DRIVER_GET(usb_dwc2),
				  dev->name, NULL, dev_ofnode(dev), NULL);

	return ret;
}

static const struct udevice_id rockchip_dwc2_ids[] = {
	{ .compatible = "rockchip,rk3066-usb" },
	{},
};

U_BOOT_DRIVER(rockchip_dwc2) = {
	.name = "rockchip_dwc2",
	.id = UCLASS_NOP,
	.of_match = rockchip_dwc2_ids,
	.bind = rockchip_dwc2_bind,
};

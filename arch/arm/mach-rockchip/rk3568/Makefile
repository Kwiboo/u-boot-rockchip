#
# (C) Copyright 2021 Rockchip Electronics Co., Ltd
#
# SPDX-License-Identifier:     GPL-2.0+
#

obj-y += clk_rk3568.o
obj-y += rk3568.o
ifneq ($(CONFIG_TPL_BUILD)$(CONFIG_TPL_TINY_FRAMEWORK),yy)
obj-y += syscon_rk3568.o
endif

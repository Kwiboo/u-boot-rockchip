# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2016 Google, Inc

obj-$(CONFIG_$(PHASE_)X86_16BIT_INIT) += cpu.o
obj-$(CONFIG_$(PHASE_)X86_32BIT_INIT) += cpu_full.o

ifdef CONFIG_SPL
ifndef CONFIG_XPL_BUILD
obj-y += cpu_from_spl.o
obj-y += cpu_full.o
obj-y += refcode.o
endif
ifndef CONFIG_XPL_BUILD
# obj-y += cpu_from_spl.o
obj-y += adsp.o
obj-y += sata.o
endif
endif

ifeq ($(CONFIG_$(PHASE_)X86_32BIT_INIT),)
#obj-y += cpu_from_spl.o
endif

obj-y += iobp.o
obj-y += lpc.o
obj-y += me.o
obj-y += northbridge.o
obj-y += pch.o
obj-y += pinctrl_broadwell.o
obj-y += power_state.o
obj-$(CONFIG_$(PHASE_)X86_32BIT_INIT) += refcode.o
obj-$(CONFIG_$(PHASE_)X86_32BIT_INIT) += sdram.o

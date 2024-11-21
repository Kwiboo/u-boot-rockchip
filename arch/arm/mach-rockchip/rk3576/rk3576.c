// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2024 Rockchip Electronics Co., Ltd
 */

#include <spl.h>
#include <asm/armv8/mmu.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/arch-rockchip/grf_rk3576.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/arch-rockchip/ioc_rk3576.h>

#define SYS_GRF_BASE		0x2600A000
#define SYS_GRF_SOC_CON2	0x0008
#define SYS_GRF_SOC_CON7	0x001c
#define SYS_GRF_SOC_CON11	0x002c
#define SYS_GRF_SOC_CON12	0x0030

#define GPIO0_IOC_BASE		0x26040000
#define GPIO0B_PULL_L		0x0024
#define GPIO0B_IE_L		0x002C

#define SYS_SGRF_BASE		0x26004000
#define SYS_SGRF_SOC_CON14	0x0058
#define SYS_SGRF_SOC_CON15	0x005C
#define SYS_SGRF_SOC_CON20	0x0070

#define FW_SYS_SGRF_BASE	0x26005000
#define SGRF_DOMAIN_CON1	0x4
#define SGRF_DOMAIN_CON2	0x8
#define SGRF_DOMAIN_CON3	0xc
#define SGRF_DOMAIN_CON4	0x10
#define SGRF_DOMAIN_CON5	0x14

const char * const boot_devices[BROM_LAST_BOOTSOURCE + 1] = {
	[BROM_BOOTSOURCE_EMMC] = "/soc/mmc@2a330000",
	[BROM_BOOTSOURCE_SD] = "/soc/mmc@2a310000",
};

static struct mm_region rk3576_mem_map[] = {
	{
		/*
		 * sdhci_send_command sets the start_addr to 0, while
		 * sdhci_transfer_data calls dma_unmap_single on that
		 * address when the transfer is done, which in turn calls
		 * invalidate_dcache_range on that memory block.
		 * Map the Bootrom that sits in that memory area, to just
		 * let the invalidate_dcache_range call pass.
		 */
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x00008000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* I/O area */
		.virt = 0x20000000UL,
		.phys = 0x20000000UL,
		.size = 0xb080000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* PMU_SRAM, CBUF, SYSTEM_SRAM */
		.virt = 0x3fe70000UL,
		.phys = 0x3fe70000UL,
		.size = 0x190000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* MSCH_DDR_PORT */
		.virt = 0x40000000UL,
		.phys = 0x40000000UL,
		.size = 0x400000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* PCIe 0+1 */
		.virt = 0x900000000UL,
		.phys = 0x900000000UL,
		.size = 0x100800000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = rk3576_mem_map;

void board_debug_uart_init(void)
{
}

#ifdef CONFIG_XPL_BUILD
void rockchip_stimer_init(void)
{
	u32 reg;

	/* If Timer already enabled, don't re-init it */
	reg = readl(CONFIG_ROCKCHIP_STIMER_BASE + 0x4);
	if (reg & 0x1)
		return;

	asm volatile("msr CNTFRQ_EL0, %0" : : "r" (CONFIG_COUNTER_FREQUENCY));
	writel(0xffffffff, CONFIG_ROCKCHIP_STIMER_BASE + 0x14);
	writel(0xffffffff, CONFIG_ROCKCHIP_STIMER_BASE + 0x18);
	writel(0x00010001, CONFIG_ROCKCHIP_STIMER_BASE + 0x04);
}
#endif

#ifndef CONFIG_TPL_BUILD
int arch_cpu_init(void)
{
#ifdef CONFIG_XPL_BUILD
	u32 val;

	/* Set the emmc to access ddr memory */
	val = readl(FW_SYS_SGRF_BASE + SGRF_DOMAIN_CON2);
	writel(val | 0x7, FW_SYS_SGRF_BASE + SGRF_DOMAIN_CON2);

	/* Set the sdmmc0 to access ddr memory */
	val = readl(FW_SYS_SGRF_BASE + SGRF_DOMAIN_CON5);
	writel(val | 0x700, FW_SYS_SGRF_BASE + SGRF_DOMAIN_CON5);

	/* Set the UFS to access ddr memory */
	val = readl(FW_SYS_SGRF_BASE + SGRF_DOMAIN_CON3);
	writel(val | 0x70000, FW_SYS_SGRF_BASE + SGRF_DOMAIN_CON3);

	/* Set the fspi0 and fspi1 to access ddr memory */
	val = readl(FW_SYS_SGRF_BASE + SGRF_DOMAIN_CON4);
	writel(val | 0x7700, FW_SYS_SGRF_BASE + SGRF_DOMAIN_CON4);

	/* Set the decom to access ddr memory */
	val = readl(FW_SYS_SGRF_BASE + SGRF_DOMAIN_CON1);
	writel(val | 0x700, FW_SYS_SGRF_BASE + SGRF_DOMAIN_CON1);

	/*
	 * Set the GPIO0B0~B3 pull up and input enable.
	 * Keep consistent with other IO.
	 */
	writel(0x00ff00ff, GPIO0_IOC_BASE + GPIO0B_PULL_L);
	writel(0x000f000f, GPIO0_IOC_BASE + GPIO0B_IE_L);

	/*
	 * Set SYS_GRF_SOC_CON2[12](input of pwm2_ch0) as 0,
	 * keep consistent with other pwm.
	 */
	writel(0x10000000, SYS_GRF_BASE + SYS_GRF_SOC_CON2);

	/* Enable noc slave response timeout */
	writel(0x80008000, SYS_GRF_BASE + SYS_GRF_SOC_CON11);
	writel(0xffffffe0, SYS_GRF_BASE + SYS_GRF_SOC_CON12);

	/*
	 * Enable cci channels for below module AXI R/W
	 * Module: GMAC0/1, MMU0/1(PCIe, SATA, USB3)
	 */
	writel(0xffffff00, SYS_SGRF_BASE + SYS_SGRF_SOC_CON20);
#endif

	return 0;
}
#endif


// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2021 Rockchip Electronics Co., Ltd
 * Copyright (c) 2022 Edgeble AI Technologies Pvt. Ltd.
 */

#define LOG_CATEGORY LOGC_ARCH

#include <dm.h>
#include <fdt_support.h>
#include <misc.h>
#include <spl.h>
#include <asm/armv8/mmu.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/arch-rockchip/grf_rk3588.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/arch-rockchip/ioc_rk3588.h>

#define FIREWALL_DDR_BASE		0xfe030000
#define FW_DDR_MST5_REG			0x54
#define FW_DDR_MST13_REG		0x74
#define FW_DDR_MST21_REG		0x94
#define FW_DDR_MST26_REG		0xa8
#define FW_DDR_MST27_REG		0xac
#define FIREWALL_SYSMEM_BASE		0xfe038000
#define FW_SYSM_MST5_REG		0x54
#define FW_SYSM_MST13_REG		0x74
#define FW_SYSM_MST21_REG		0x94
#define FW_SYSM_MST26_REG		0xa8
#define FW_SYSM_MST27_REG		0xac

#define BUS_IOC_GPIO2A_IOMUX_SEL_L	0x40
#define BUS_IOC_GPIO2B_IOMUX_SEL_L	0x48
#define BUS_IOC_GPIO2D_IOMUX_SEL_L	0x58
#define BUS_IOC_GPIO2D_IOMUX_SEL_H	0x5c
#define BUS_IOC_GPIO3A_IOMUX_SEL_L	0x60

#define SYS_GRF_FORCE_JTAG		BIT(14)

/**
 * Boot-device identifiers used by the BROM on RK3588 when device is booted
 * from SPI flash. IOMUX used for SPI flash affect the value used by the BROM
 * and not the type of SPI flash used.
 */
enum {
	BROM_BOOTSOURCE_FSPI_M0 = 3,
	BROM_BOOTSOURCE_FSPI_M1 = 4,
	BROM_BOOTSOURCE_FSPI_M2 = 6,
};

const char * const boot_devices[BROM_LAST_BOOTSOURCE + 1] = {
	[BROM_BOOTSOURCE_EMMC] = "/mmc@fe2e0000",
	[BROM_BOOTSOURCE_FSPI_M0] = "/spi@fe2b0000/flash@0",
	[BROM_BOOTSOURCE_FSPI_M1] = "/spi@fe2b0000/flash@0",
	[BROM_BOOTSOURCE_FSPI_M2] = "/spi@fe2b0000/flash@0",
	[BROM_BOOTSOURCE_SD] = "/mmc@fe2c0000",
};

static struct mm_region rk3588_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0xf0000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0xf0000000UL,
		.phys = 0xf0000000UL,
		.size = 0x10000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},  {
		.virt = 0x900000000,
		.phys = 0x900000000,
		.size = 0x150000000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},  {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = rk3588_mem_map;

/* GPIO0B_IOMUX_SEL_H */
enum {
	GPIO0B5_SHIFT		= 4,
	GPIO0B5_MASK		= GENMASK(7, 4),
	GPIO0B5_REFER		= 8,
	GPIO0B5_UART2_TX_M0	= 10,

	GPIO0B6_SHIFT		= 8,
	GPIO0B6_MASK		= GENMASK(11, 8),
	GPIO0B6_REFER		= 8,
	GPIO0B6_UART2_RX_M0	= 10,
};

void board_debug_uart_init(void)
{
	__maybe_unused static struct rk3588_bus_ioc * const bus_ioc = (void *)BUS_IOC_BASE;
	static struct rk3588_pmu2_ioc * const pmu2_ioc = (void *)PMU2_IOC_BASE;

	/* Refer to BUS_IOC */
	rk_clrsetreg(&pmu2_ioc->gpio0b_iomux_sel_h,
		     GPIO0B6_MASK | GPIO0B5_MASK,
		     GPIO0B6_REFER << GPIO0B6_SHIFT |
		     GPIO0B5_REFER << GPIO0B5_SHIFT);

	/* UART2_M0 Switch iomux */
	rk_clrsetreg(&bus_ioc->gpio0b_iomux_sel_h,
		     GPIO0B6_MASK | GPIO0B5_MASK,
		     GPIO0B6_UART2_RX_M0 << GPIO0B6_SHIFT |
		     GPIO0B5_UART2_TX_M0 << GPIO0B5_SHIFT);
}

#ifdef CONFIG_XPL_BUILD
void rockchip_stimer_init(void)
{
	/* If Timer already enabled, don't re-init it */
	u32 reg = readl(CONFIG_ROCKCHIP_STIMER_BASE + 0x4);

	if (reg & 0x1)
		return;

	asm volatile("msr CNTFRQ_EL0, %0" : : "r" (CONFIG_COUNTER_FREQUENCY));
	writel(0xffffffff, CONFIG_ROCKCHIP_STIMER_BASE + 0x14);
	writel(0xffffffff, CONFIG_ROCKCHIP_STIMER_BASE + 0x18);
	writel(0x1, CONFIG_ROCKCHIP_STIMER_BASE + 0x4);
}
#endif

#ifndef CONFIG_TPL_BUILD
int arch_cpu_init(void)
{
#ifdef CONFIG_XPL_BUILD
#ifdef CONFIG_ROCKCHIP_DISABLE_FORCE_JTAG
	static struct rk3588_sysgrf * const sys_grf = (void *)SYS_GRF_BASE;
#endif
	int secure_reg;

	/* Set the SDMMC eMMC crypto_ns FSPI access secure area */
	secure_reg = readl(FIREWALL_DDR_BASE + FW_DDR_MST5_REG);
	secure_reg &= 0xffff;
	writel(secure_reg, FIREWALL_DDR_BASE + FW_DDR_MST5_REG);
	secure_reg = readl(FIREWALL_DDR_BASE + FW_DDR_MST13_REG);
	secure_reg &= 0xffff;
	writel(secure_reg, FIREWALL_DDR_BASE + FW_DDR_MST13_REG);
	secure_reg = readl(FIREWALL_DDR_BASE + FW_DDR_MST21_REG);
	secure_reg &= 0xffff;
	writel(secure_reg, FIREWALL_DDR_BASE + FW_DDR_MST21_REG);
	secure_reg = readl(FIREWALL_DDR_BASE + FW_DDR_MST26_REG);
	secure_reg &= 0xffff;
	writel(secure_reg, FIREWALL_DDR_BASE + FW_DDR_MST26_REG);
	secure_reg = readl(FIREWALL_DDR_BASE + FW_DDR_MST27_REG);
	secure_reg &= 0xffff0000;
	writel(secure_reg, FIREWALL_DDR_BASE + FW_DDR_MST27_REG);

	secure_reg = readl(FIREWALL_SYSMEM_BASE + FW_SYSM_MST5_REG);
	secure_reg &= 0xffff;
	writel(secure_reg, FIREWALL_SYSMEM_BASE + FW_SYSM_MST5_REG);
	secure_reg = readl(FIREWALL_SYSMEM_BASE + FW_SYSM_MST13_REG);
	secure_reg &= 0xffff;
	writel(secure_reg, FIREWALL_SYSMEM_BASE + FW_SYSM_MST13_REG);
	secure_reg = readl(FIREWALL_SYSMEM_BASE + FW_SYSM_MST21_REG);
	secure_reg &= 0xffff;
	writel(secure_reg, FIREWALL_SYSMEM_BASE + FW_SYSM_MST21_REG);
	secure_reg = readl(FIREWALL_SYSMEM_BASE + FW_SYSM_MST26_REG);
	secure_reg &= 0xffff;
	writel(secure_reg, FIREWALL_SYSMEM_BASE + FW_SYSM_MST26_REG);
	secure_reg = readl(FIREWALL_SYSMEM_BASE + FW_SYSM_MST27_REG);
	secure_reg &= 0xffff0000;
	writel(secure_reg, FIREWALL_SYSMEM_BASE + FW_SYSM_MST27_REG);

#ifdef CONFIG_ROCKCHIP_DISABLE_FORCE_JTAG
	/* Disable JTAG exposed on SDMMC */
	rk_clrreg(&sys_grf->soc_con[6], SYS_GRF_FORCE_JTAG);
#endif
#endif

	return 0;
}
#endif

#define RK3588_OTP_CPU_CODE_OFFSET		0x02
#define RK3588_OTP_SPECIFICATION_OFFSET		0x06
#define RK3588_OTP_IP_STATE_OFFSET		0x1d

#define FAIL_CPU_CLUSTER0		GENMASK(3, 0)
#define FAIL_CPU_CLUSTER1		GENMASK(5, 4)
#define FAIL_CPU_CLUSTER2		GENMASK(7, 6)
#define FAIL_GPU				GENMASK(4, 1)
#define FAIL_RKVDEC0			BIT(6)
#define FAIL_RKVDEC1			BIT(7)
#define FAIL_RKVENC0			BIT(0)
#define FAIL_RKVENC1			BIT(2)

int checkboard(void)
{
	u8 cpu_code[2], specification, package;
	struct udevice *dev;
	char suffix[3];
	int ret;

	if (!IS_ENABLED(CONFIG_ROCKCHIP_OTP) || !CONFIG_IS_ENABLED(MISC))
		return 0;

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(rockchip_otp), &dev);
	if (ret) {
		log_debug("Could not find otp device, ret=%d\n", ret);
		return 0;
	}

	/* cpu-code: SoC model, e.g. 0x35 0x82 or 0x35 0x88 */
	ret = misc_read(dev, RK3588_OTP_CPU_CODE_OFFSET, cpu_code, 2);
	if (ret < 0) {
		log_debug("Could not read cpu-code, ret=%d\n", ret);
		return 0;
	}

	/* specification: SoC variant, e.g. 0xA for RK3588J and 0x13 for RK3588S */
	ret = misc_read(dev, RK3588_OTP_SPECIFICATION_OFFSET, &specification, 1);
	if (ret < 0) {
		log_debug("Could not read specification, ret=%d\n", ret);
		return 0;
	}
	/* package: likely SoC variant revision, 0x2 for RK3588S2 */
	package = specification >> 5;
	specification &= 0x1f;

	/* for RK3588J i.e. '@' + 0xA = 'J' */
	suffix[0] = specification > 1 ? '@' + specification : '\0';
	/* for RK3588S2 i.e. '0' + 0x2 = '2' */
	suffix[1] = package > 1 ? '0' + package : '\0';
	suffix[2] = '\0';

	printf("SoC:   RK%02x%02x%s\n", cpu_code[0], cpu_code[1], suffix);

	return 0;
}

static int fdt_path_del_node(void *fdt, const char *path)
{
	int nodeoffset;

	nodeoffset = fdt_path_offset(fdt, path);
	if (nodeoffset < 0)
		return nodeoffset;

	return fdt_del_node(fdt, nodeoffset);
}

static int fdt_path_set_name(void *fdt, const char *path, const char *name)
{
	int nodeoffset;

	nodeoffset = fdt_path_offset(fdt, path);
	if (nodeoffset < 0)
		return nodeoffset;

	return fdt_set_name(fdt, nodeoffset, name);
}

/*
 * RK3582 is a variant of the RK3588S with some IP blocks disabled. What blocks
 * are disabled/non-working is indicated by ip-state in OTP. ft_system_setup()
 * is used to mark any cpu and/or gpu node with status=fail as indicated by
 * ip-state. Apply same policy as vendor U-Boot for RK3582, i.e. two big cpu
 * cores and the gpu is always failed/disabled. Enable OF_SYSTEM_SETUP to make
 * use of the required DT fixups for RK3582 board variants.
 */
int ft_system_setup(void *blob, struct bd_info *bd)
{
	static const char * const cpu_node_names[] = {
		"cpu@0", "cpu@100", "cpu@200", "cpu@300",
		"cpu@400", "cpu@500", "cpu@600", "cpu@700",
	};
	u8 cpu_code[2], ip_state[3];
	int parent, node, i, ret;
	struct udevice *dev;

	if (!IS_ENABLED(CONFIG_OF_SYSTEM_SETUP))
		return 0;

	if (!IS_ENABLED(CONFIG_ROCKCHIP_OTP) || !CONFIG_IS_ENABLED(MISC))
		return -ENOSYS;

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(rockchip_otp), &dev);
	if (ret) {
		log_debug("Could not find otp device, ret=%d\n", ret);
		return ret;
	}

	/* cpu-code: SoC model, e.g. 0x35 0x82 or 0x35 0x88 */
	ret = misc_read(dev, RK3588_OTP_CPU_CODE_OFFSET, cpu_code, 2);
	if (ret < 0) {
		log_debug("Could not read cpu-code, ret=%d\n", ret);
		return ret;
	}

	log_debug("cpu-code: %02x %02x\n", cpu_code[0], cpu_code[1]);

	/* only fail devices on rk3582/rk3583 */
	if (!(cpu_code[0] == 0x35 && cpu_code[1] == 0x82) &&
	    !(cpu_code[0] == 0x35 && cpu_code[1] == 0x83))
		return 0;

	ret = misc_read(dev, RK3588_OTP_IP_STATE_OFFSET, &ip_state, 3);
	if (ret < 0) {
		log_debug("Could not read ip-state, ret=%d\n", ret);
		return ret;
	}

	log_debug("ip-state: %02x %02x %02x\n",
		  ip_state[0], ip_state[1], ip_state[2]);

	if (cpu_code[0] == 0x35 && cpu_code[1] == 0x82) {
		/* policy: always fail gpu on rk3582 */
		ip_state[1] |= FAIL_GPU;

		/* policy: always fail rkvdec on rk3582 */
		ip_state[1] |= FAIL_RKVDEC0 | FAIL_RKVDEC1;
	} else if (cpu_code[0] == 0x35 && cpu_code[1] == 0x83) {
		/* policy: always fail one rkvdec core on rk3583 */
		if (!(ip_state[1] & (FAIL_RKVDEC0 | FAIL_RKVDEC1)))
			ip_state[1] |= FAIL_RKVDEC1;
	}

	/* policy: always fail one rkvenc core on rk3582/rk3583 */
	if (!(ip_state[2] & (FAIL_RKVENC0 | FAIL_RKVENC1)))
		ip_state[2] |= FAIL_RKVENC1;

	/* policy: always fail one big core cluster on rk3582/rk3583 */
	if (!(ip_state[0] & (FAIL_CPU_CLUSTER1 | FAIL_CPU_CLUSTER2)))
		ip_state[0] |= FAIL_CPU_CLUSTER2;

	if (ip_state[0] & FAIL_CPU_CLUSTER1) {
		/* fail entire cluster when one or more core is bad */
		ip_state[0] |= FAIL_CPU_CLUSTER1;
		fdt_path_del_node(blob, "/cpus/cpu-map/cluster1");
	}

	if (ip_state[0] & FAIL_CPU_CLUSTER2) {
		/* fail entire cluster when one or more core is bad */
		ip_state[0] |= FAIL_CPU_CLUSTER2;
		fdt_path_del_node(blob, "/cpus/cpu-map/cluster2");
	} else if (ip_state[0] & FAIL_CPU_CLUSTER1) {
		/* cluster nodes must be named in a continuous series */
		fdt_path_set_name(blob, "/cpus/cpu-map/cluster2", "cluster1");
	}

	/* gpu: ip_state[1]: bit1~4 */
	if (ip_state[1] & FAIL_GPU) {
		log_debug("fail gpu\n");
		fdt_status_fail_by_pathf(blob, "/gpu@fb000000");
	}

	/* rkvdec: ip_state[1]: bit6,7 */
	if (ip_state[1] & FAIL_RKVDEC0) {
		log_debug("fail rkvdec0\n");
		fdt_status_fail_by_pathf(blob, "/video-codec@fdc38000");
		fdt_status_fail_by_pathf(blob, "/iommu@fdc38700");
	}
	if (ip_state[1] & FAIL_RKVDEC1) {
		log_debug("fail rkvdec1\n");
		fdt_status_fail_by_pathf(blob, "/video-codec@fdc40000");
		fdt_status_fail_by_pathf(blob, "/iommu@fdc40700");
	}

	/* rkvenc: ip_state[2]: bit0,2 */
	if (ip_state[2] & FAIL_RKVENC0) {
		log_debug("fail rkvenc0\n");
		fdt_status_fail_by_pathf(blob, "/video-codec@fdbd0000");
		fdt_status_fail_by_pathf(blob, "/iommu@fdbdf000");
	}
	if (ip_state[2] & FAIL_RKVENC1) {
		log_debug("fail rkvenc1\n");
		fdt_status_fail_by_pathf(blob, "/video-codec@fdbe0000");
		fdt_status_fail_by_pathf(blob, "/iommu@fdbef000");
	}

	parent = fdt_path_offset(blob, "/cpus");
	if (parent < 0) {
		log_debug("Could not find /cpus, parent=%d\n", parent);
		return parent;
	}

	/* cpu: ip_state[0]: bit0~7 */
	for (i = 0; i < 8; i++) {
		/* fail any bad cpu core */
		if (!(ip_state[0] & BIT(i)))
			continue;

		node = fdt_subnode_offset(blob, parent, cpu_node_names[i]);
		if (node >= 0) {
			log_debug("fail cpu %s\n", cpu_node_names[i]);
			fdt_status_fail(blob, node);
		} else {
			log_debug("Could not find %s, node=%d\n",
				  cpu_node_names[i], node);
		}
	}

	// TODO: inject rockchip,rk3582 compatible

	return 0;
}

// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2021 Rockchip Electronics Co., Ltd
 * Copyright (c) 2022 Edgeble AI Technologies Pvt. Ltd.
 */

#include <dm.h>
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

#define CPU_CODE_OFFSET			0x2
#define CPU_VERSION_OFFSET		0x1c
#define IP_STATE_OFFSET			0x1d
#define BAD_CPU_CLUSTER0		GENMASK(3, 0)
#define BAD_CPU_CLUSTER1		GENMASK(5, 4)
#define BAD_CPU_CLUSTER2		GENMASK(7, 6)

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

#ifdef CONFIG_SPL_BUILD
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
#ifdef CONFIG_SPL_BUILD
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

#ifdef CONFIG_OF_SYSTEM_SETUP
static void fdt_path_del_node(void *fdt, const char *path)
{
	int node;

	node = fdt_path_offset(fdt, path);
	if (node >= 0)
		fdt_del_node(fdt, node);
}

static void fdt_path_set_name(void *fdt, const char *path, const char *name)
{
	int node;

	node = fdt_path_offset(fdt, path);
	if (node >= 0)
		fdt_set_name(fdt, node, name);
}

static bool in_u32_array(u32 match, u32 *array, int len)
{
	int i;

	for (i = 0; i < len; ++i) {
		if (array[i] == match)
			return true;
	}

	return false;
}

static void fdt_fixup_phandle_prop(void *fdt, int node, const char *name,u32 *array, int array_len)
{
	int len, new_len, i;
	const u32 *values;
	u32 new_values[8];

	values = fdt_getprop(fdt, node, name, &len);
	if (!values || len > sizeof(new_values))
		return;

	for (i = 0, new_len = 0; i < (len / sizeof(u32)); ++i) {
		u32 phandle = fdt32_to_cpu(values[i]);
		if (in_u32_array(phandle, array, array_len))
			continue;

		new_values[new_len++] = cpu_to_fdt32(phandle);
	}

	if (new_len != (len / sizeof(u32)))
		fdt_setprop(fdt, node, name, new_values, new_len * sizeof(u32));
}

int ft_system_setup(void *blob, struct bd_info *bd)
{
	const char *cpu_node_names[] = {
		"cpu@0", "cpu@100", "cpu@200", "cpu@300",
		"cpu@400", "cpu@500", "cpu@600", "cpu@700",
	};
	struct udevice *dev;
	u8 cpu_code[2], ip_state[3];
	u32 cpu_phandle[8], cpu_phandles = 0;
	int parent, node, i, ret;

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(rockchip_otp), &dev);
	if (ret)
		return ret;

	ret = misc_read(dev, CPU_CODE_OFFSET, &cpu_code, sizeof(cpu_code));
	if (ret < 0)
		return ret;

	debug("cpu-code: %02x%02x\n", cpu_code[0], cpu_code[1]);

	if (!(cpu_code[0] == 0x35 && cpu_code[1] == 0x82))
		return 0;

	ret = misc_read(dev, IP_STATE_OFFSET, &ip_state, sizeof(ip_state));
	if (ret < 0)
		return ret;

	debug("ip-state: %02x %02x %02x\n", ip_state[0], ip_state[1], ip_state[2]);

	if (ip_state[0] & BAD_CPU_CLUSTER1) {
		ip_state[0] |= BAD_CPU_CLUSTER1;
		fdt_path_del_node(blob, "/cpus/cpu-map/cluster1");
	}

	if (ip_state[0] & BAD_CPU_CLUSTER2) {
		ip_state[0] |= BAD_CPU_CLUSTER2;
		fdt_path_del_node(blob, "/cpus/cpu-map/cluster2");
	} else if (ip_state[0] & BAD_CPU_CLUSTER1) {
		fdt_path_set_name(blob, "/cpus/cpu-map/cluster2", "cluster1");
	}

	parent = fdt_path_offset(blob, "/cpus");
	if (parent < 0)
		return 0;

	for (i = 0; i < 8; i++) {
		if (!(ip_state[0] & BIT(i)))
			continue;

		node = fdt_subnode_offset(blob, parent, cpu_node_names[i]);
		if (node >= 0) {
			cpu_phandle[cpu_phandles++] = fdt_get_phandle(blob, node);
			fdt_del_node(blob, node);
		}
	}

	if (!cpu_phandles)
		return 0;

	parent = fdt_path_offset(blob, "/interrupt-controller@fe600000/ppi-partitions");
	if (parent >= 0) {
		fdt_for_each_subnode(node, blob, parent) {
			fdt_fixup_phandle_prop(blob, node, "affinity", cpu_phandle, cpu_phandles);
		}
	}

	return 0;
}
#endif

int rockchip_early_misc_init_r(void)
{
	struct udevice *dev;
	u8 cpu_code[2], ip_state[3], package, specification;
	int ret, i;

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(rockchip_otp), &dev);
	if (ret) {
		debug("%s: could not find otp device, ret=%d\n", __func__, ret);
		return 0;
	}

	ret = misc_read(dev, CPU_CODE_OFFSET, &cpu_code, 2);
	if (ret < 0) {
		debug("%s: could not read cpu-code, ret=%d\n", __func__, ret);
		return 0;
	}

	debug("cpu-code: %02x%02x\n", cpu_code[0], cpu_code[1]);

	ret = misc_read(dev, CPU_VERSION_OFFSET, &cpu_code, 2);
	if (ret < 0) {
		debug("%s: could not read cpu-version, ret=%d\n", __func__, ret);
		return 0;
	}
	debug("cpu-version: %02x %02x\n", cpu_code[0], cpu_code[1]);

	ret = misc_read(dev, 0x05, &cpu_code, 2);
	if (ret < 0) {
		debug("%s: could not read cpu-version, ret=%d\n", __func__, ret);
		return 0;
	}
	debug("data: %02x %02x\n", cpu_code[0], cpu_code[1]);

	package = ((cpu_code[0] & GENMASK(1, 0)) << 3) | ((cpu_code[1] & GENMASK(7, 5)) >> 5);
	specification = cpu_code[1] & GENMASK(4, 0);

	debug("package: %02x\n", package);
	debug("specification: %02x\n", specification);

	ret = misc_read(dev, IP_STATE_OFFSET, &ip_state, sizeof(ip_state));
	if (ret < 0) {
		debug("%s: could not read ip state, ret=%d\n", __func__, ret);
		return ret;
	}

	debug("ip-state: %02x %02x %02x\n", ip_state[0], ip_state[1], ip_state[2]);

	/* cpu: ip_state[0]: bit0~7 */
	for (i = 0; i < 8; ++i) {
		if (ip_state[0] & BIT(i))
			debug("bad-state: cpu core %d\n", i);
	}
	/* gpu: ip_state[1]: bit1~4 */
	for (i = 0; i < 4; ++i) {
		if (ip_state[1] & BIT(i + 1))
			debug("bad-state: gpu core %d\n", i);
	}
	/* rkvdec: ip_state[1]: bit6,7 */
	for (i = 0; i < 2; ++i) {
		if (ip_state[1] & BIT(i + 6))
			debug("bad-state: rkvdec core %d\n", i);
	}
	/* rkvenc: ip_state[2]: bit0,2 */
	for (i = 0; i < 2; ++i) {
		if (ip_state[2] & BIT(i * 2))
			debug("bad-state: rkvenc core %d\n", i);
	}

	return 0;
}

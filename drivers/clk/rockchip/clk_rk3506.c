// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2023 Rockchip Electronics Co., Ltd.
 * Author: Finley Xiao <finley.xiao@rock-chips.com>
 */

#include <bitfield.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <syscon.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/cru_rk3506.h>
#include <asm/arch-rockchip/hardware.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dt-bindings/clock/rockchip,rk3506-cru.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

#define RK3506_SCRU_BASE 0xFF9A8000

#define DIV_TO_RATE(input_rate, div)    ((input_rate) / ((div) + 1))

/*
 * [FRAC PLL]: GPLL, V0PLL, V1PLL
 *   - VCO Frequency: 950MHz to 3800MHZ
 *   - Output Frequency: 19MHz to 3800MHZ
 *   - refdiv: 1 to 63 (Int Mode), 1 to 2 (Frac Mode)
 *   - fbdiv: 16 to 3800 (Int Mode), 20 to 380 (Frac Mode)
 *   - post1div: 1 to 7
 *   - post2div: 1 to 7
 */
static struct rockchip_pll_rate_table rk3506_pll_rates[] = {
	/* _mhz, _refdiv, _fbdiv, _postdiv1, _postdiv2, _dsmpd, _frac */
	RK3036_PLL_RATE(1896000000, 1, 79, 1, 1, 1, 0),
	RK3036_PLL_RATE(1800000000, 1, 75, 1, 1, 1, 0),
	RK3036_PLL_RATE(1704000000, 1, 71, 1, 1, 1, 0),
	RK3036_PLL_RATE(1608000000, 1, 67, 1, 1, 1, 0),
	RK3036_PLL_RATE(1512000000, 1, 63, 1, 1, 1, 0),
	RK3036_PLL_RATE(1500000000, 1, 125, 2, 1, 1, 0),
	RK3036_PLL_RATE(1416000000, 1, 59, 1, 1, 1, 0),
	RK3036_PLL_RATE(1350000000, 4, 225, 1, 1, 1, 0),
	RK3036_PLL_RATE(1296000000, 1, 54, 1, 1, 1, 0),
	RK3036_PLL_RATE(1200000000, 1, 50, 1, 1, 1, 0),
	RK3036_PLL_RATE(1188000000, 1, 99, 2, 1, 1, 0),
	RK3036_PLL_RATE(1179648000, 1, 49, 1, 1, 0, 2550137),
	RK3036_PLL_RATE(1008000000, 1, 84, 2, 1, 1, 0),
	RK3036_PLL_RATE(1000000000, 3, 125, 1, 1, 1, 0),
	RK3036_PLL_RATE(993484800, 1, 41, 1, 1, 0, 6630355),
	RK3036_PLL_RATE(983040000, 1, 40, 1, 1, 0, 16106127),
	RK3036_PLL_RATE(960000000, 1, 80, 2, 1, 1, 0),
	RK3036_PLL_RATE(912000000, 1, 76, 2, 1, 1, 0),
	RK3036_PLL_RATE(903168000, 1, 75, 2, 1, 0, 4429185),
	RK3036_PLL_RATE(816000000, 1, 68, 2, 1, 1, 0),
	RK3036_PLL_RATE(800000000, 3, 200, 2, 1, 1, 0),
	RK3036_PLL_RATE(600000000, 1, 50, 2, 1, 1, 0),
	RK3036_PLL_RATE(594000000, 2, 99, 2, 1, 1, 0),
	RK3036_PLL_RATE(408000000, 1, 68, 2, 2, 1, 0),
	RK3036_PLL_RATE(312000000, 1, 78, 6, 1, 1, 0),
	RK3036_PLL_RATE(216000000, 1, 72, 4, 2, 1, 0),
	RK3036_PLL_RATE(96000000, 1, 48, 6, 2, 1, 0),
	{ /* sentinel */ },
};

static struct rockchip_pll_clock rk3506_pll_clks[] = {
	[GPLL] = PLL(pll_rk3328, PLL_GPLL, RK3506_PLL_CON(0),
		     RK3506_MODE_CON, 0, 10, 0, rk3506_pll_rates),
	[V0PLL] = PLL(pll_rk3328, PLL_V0PLL, RK3506_PLL_CON(8),
		     RK3506_MODE_CON, 2, 10, 0, rk3506_pll_rates),
	[V1PLL] = PLL(pll_rk3328, PLL_V1PLL, RK3506_PLL_CON(16),
		     RK3506_MODE_CON, 4, 10, 0, rk3506_pll_rates),
};

#define RK3506_CPUCLK_RATE(_rate, _aclk_m_core, _pclk_dbg)	\
{								\
	.rate = _rate##U,					\
	.aclk_div = _aclk_m_core,				\
	.pclk_div = _pclk_dbg,					\
}

/* SIGN-OFF: aclk_core: 500M, pclk_core: 125M, */
static struct rockchip_cpu_rate_table rk3506_cpu_rates[] = {
	RK3506_CPUCLK_RATE(1179648000, 1, 6),
	RK3506_CPUCLK_RATE(903168000, 1, 5),
	RK3506_CPUCLK_RATE(800000000, 1, 4),
	RK3506_CPUCLK_RATE(589824000, 1, 3),
	RK3506_CPUCLK_RATE(400000000, 1, 2),
	RK3506_CPUCLK_RATE(200000000, 1, 1),
	{ /* sentinel */ },
};

static int rk3506_armclk_get_rate(struct rk3506_clk_priv *priv)
{
	struct rk3506_cru *cru = priv->cru;
	u32 sel, con, div;
	ulong prate;

	con = readl(&cru->clksel_con[15]);
	sel = (con & CLK_CORE_SRC_SEL_MASK) >> CLK_CORE_SRC_SEL_SHIFT;
	div = (con & CLK_CORE_SRC_DIV_MASK) >> CLK_CORE_SRC_DIV_SHIFT;

	if (sel == CLK_CORE_SEL_GPLL)
		prate = priv->gpll_hz;
	else if (sel ==  CLK_CORE_SEL_V0PLL)
		prate = priv->v0pll_hz;
	else if (sel ==  CLK_CORE_SEL_V1PLL)
		prate = priv->v1pll_hz;
	else
		return -EINVAL;

	return DIV_TO_RATE(prate, div);
}

static int rk3506_armclk_set_rate(struct rk3506_clk_priv *priv, ulong new_rate)
{
	const struct rockchip_cpu_rate_table *rate;
	struct rk3506_cru *cru = priv->cru;
	ulong old_rate, prate;
	u32 con, sel, div, old_div;

	rate = rockchip_get_cpu_settings(rk3506_cpu_rates, new_rate);
	if (!rate) {
		printf("%s unsupported rate\n", __func__);
		return -EINVAL;
	}

	/*
	 * set up dependent divisors for PCLK and ACLK clocks.
	 */
	old_rate = rk3506_armclk_get_rate(priv);
	if (new_rate >= old_rate) {
		rk_clrsetreg(&cru->clksel_con[15], ACLK_CORE_DIV_MASK,
			     rate->aclk_div << ACLK_CORE_DIV_SHIFT);
		rk_clrsetreg(&cru->clksel_con[16], PCLK_CORE_DIV_MASK,
			     rate->pclk_div << PCLK_CORE_DIV_SHIFT);
	}

	if (new_rate == 589824000 || new_rate == 1179648000) {
		sel = CLK_CORE_SEL_V0PLL;
		div = DIV_ROUND_UP(priv->v0pll_hz, new_rate);
		prate = priv->v0pll_hz;
	} else if (new_rate == 903168000) {
		sel = CLK_CORE_SEL_V1PLL;
		div = DIV_ROUND_UP(priv->v1pll_hz, new_rate);
		prate = priv->v1pll_hz;
	} else {
		sel = CLK_CORE_SEL_GPLL;
		div = DIV_ROUND_UP(priv->gpll_hz, new_rate);
		prate = priv->gpll_hz;
	}
	assert(div - 1 <= 31);

	con = readl(&cru->clksel_con[15]);
	old_div = (con & CLK_CORE_SRC_DIV_MASK) >> CLK_CORE_SRC_DIV_SHIFT;
	if (DIV_TO_RATE(prate, old_div) > new_rate) {
		rk_clrsetreg(&cru->clksel_con[15], CLK_CORE_SRC_DIV_MASK,
			     (div - 1) << CLK_CORE_SRC_DIV_SHIFT);
		rk_clrsetreg(&cru->clksel_con[15], CLK_CORE_SRC_SEL_MASK,
			     sel << CLK_CORE_SRC_SEL_SHIFT);
	} else {
		rk_clrsetreg(&cru->clksel_con[15], CLK_CORE_SRC_SEL_MASK,
			     sel << CLK_CORE_SRC_SEL_SHIFT);
		rk_clrsetreg(&cru->clksel_con[15], CLK_CORE_SRC_DIV_MASK,
			     (div - 1) << CLK_CORE_SRC_DIV_SHIFT);
	}

	if (new_rate < old_rate) {
		rk_clrsetreg(&cru->clksel_con[15], ACLK_CORE_DIV_MASK,
			     rate->aclk_div << ACLK_CORE_DIV_SHIFT);
		rk_clrsetreg(&cru->clksel_con[16], PCLK_CORE_DIV_MASK,
			     rate->pclk_div << PCLK_CORE_DIV_SHIFT);
	}

	return 0;
}

static ulong rk3506_pll_div_get_rate(struct rk3506_clk_priv *priv, ulong clk_id)
{
	struct rk3506_cru *cru = priv->cru;
	u32 con, div;
	ulong prate;

	switch (clk_id) {
	case CLK_GPLL_DIV:
		con = readl(&cru->clksel_con[0]);
		div = (con & CLK_GPLL_DIV_MASK) >> CLK_GPLL_DIV_SHIFT;
		prate = priv->gpll_hz;
		break;
	case CLK_GPLL_DIV_100M:
		con = readl(&cru->clksel_con[0]);
		div = (con & CLK_GPLL_DIV_100M_MASK) >> CLK_GPLL_DIV_100M_SHIFT;
		prate = priv->gpll_div_hz;
		break;
	case CLK_V0PLL_DIV:
		con = readl(&cru->clksel_con[1]);
		div = (con & CLK_V0PLL_DIV_MASK) >> CLK_V0PLL_DIV_SHIFT;
		prate = priv->v0pll_hz;
		break;
	case CLK_V1PLL_DIV:
		con = readl(&cru->clksel_con[1]);
		div = (con & CLK_V1PLL_DIV_MASK) >> CLK_V1PLL_DIV_SHIFT;
		prate = priv->v1pll_hz;
		break;
	default:
		return -ENOENT;
	}

	return DIV_TO_RATE(prate, div);
}

static ulong rk3506_pll_div_set_rate(struct rk3506_clk_priv *priv, ulong clk_id,
				 ulong rate)
{
	struct rk3506_cru *cru = priv->cru;
	u32 div;

	switch (clk_id) {
	case CLK_GPLL_DIV:
		div = DIV_ROUND_UP(priv->gpll_hz, rate);
		assert(div - 1 <= 15);
		rk_clrsetreg(&cru->clksel_con[0], CLK_GPLL_DIV_MASK,
			     ((div - 1) << CLK_GPLL_DIV_SHIFT));
		break;
	case CLK_GPLL_DIV_100M:
		div = DIV_ROUND_UP(priv->gpll_div_hz, rate);
		assert(div - 1 <= 15);
		rk_clrsetreg(&cru->clksel_con[0], CLK_GPLL_DIV_100M_MASK,
			     ((div - 1) << CLK_GPLL_DIV_100M_SHIFT));
		break;
	case CLK_V0PLL_DIV:
		div = DIV_ROUND_UP(priv->v0pll_hz, rate);
		assert(div - 1 <= 15);
		rk_clrsetreg(&cru->clksel_con[1], CLK_V0PLL_DIV_MASK,
			     ((div - 1) << CLK_V0PLL_DIV_SHIFT));
		break;
	case CLK_V1PLL_DIV:
		div = DIV_ROUND_UP(priv->v1pll_hz, rate);
		assert(div - 1 <= 15);
		rk_clrsetreg(&cru->clksel_con[1], CLK_V1PLL_DIV_MASK,
			     ((div - 1) << CLK_V1PLL_DIV_SHIFT));
		break;
	default:
		return -ENOENT;
	}

	return rk3506_pll_div_get_rate(priv, clk_id);
}

static ulong rk3506_bus_get_rate(struct rk3506_clk_priv *priv, ulong clk_id)
{
	struct rk3506_cru *cru = priv->cru;
	u32 sel, con, div;
	ulong prate;

	switch (clk_id) {
	case ACLK_BUS_ROOT:
		con = readl(&cru->clksel_con[21]);
		sel = (con & ACLK_BUS_SEL_MASK) >> ACLK_BUS_SEL_SHIFT;
		div = (con & ACLK_BUS_DIV_MASK) >> ACLK_BUS_DIV_SHIFT;
		break;
	case HCLK_BUS_ROOT:
		con = readl(&cru->clksel_con[21]);
		sel = (con & HCLK_BUS_SEL_MASK) >> HCLK_BUS_SEL_SHIFT;
		div = (con & HCLK_BUS_DIV_MASK) >> HCLK_BUS_DIV_SHIFT;
		break;
	case PCLK_BUS_ROOT:
		con = readl(&cru->clksel_con[22]);
		sel = (con & PCLK_BUS_SEL_MASK) >> PCLK_BUS_SEL_SHIFT;
		div = (con & PCLK_BUS_DIV_MASK) >> PCLK_BUS_DIV_SHIFT;
		break;
	default:
		return -ENOENT;
	}

	if (sel == ACLK_BUS_SEL_GPLL_DIV)
		prate = priv->gpll_div_hz;
	else if (sel == ACLK_BUS_SEL_V0PLL_DIV)
		prate = priv->v0pll_div_hz;
	else if (sel == ACLK_BUS_SEL_V1PLL_DIV)
		prate = priv->v1pll_div_hz;
	else
		return -EINVAL;

	return DIV_TO_RATE(prate, div);
}

static ulong rk3506_bus_set_rate(struct rk3506_clk_priv *priv, ulong clk_id,
				 ulong rate)
{
	struct rk3506_cru *cru = priv->cru;
	u32 sel, div;

	if (priv->v0pll_div_hz % rate == 0) {
		sel = ACLK_BUS_SEL_V0PLL_DIV;
		div = DIV_ROUND_UP(priv->v0pll_div_hz, rate);
	} else if (priv->v1pll_div_hz % rate == 0) {
		sel= ACLK_BUS_SEL_V1PLL_DIV;
		div = DIV_ROUND_UP(priv->v1pll_div_hz, rate);
	} else {
		sel= ACLK_BUS_SEL_GPLL_DIV;
		div = DIV_ROUND_UP(priv->gpll_div_hz, rate);
	}
	assert(div - 1 <= 31);

	switch (clk_id) {
	case ACLK_BUS_ROOT:
		rk_clrsetreg(&cru->clksel_con[21],
			     ACLK_BUS_DIV_MASK | ACLK_BUS_SEL_MASK,
			     (sel << ACLK_BUS_SEL_SHIFT) |
			     ((div - 1) << ACLK_BUS_DIV_SHIFT));
		break;
	case HCLK_BUS_ROOT:
		rk_clrsetreg(&cru->clksel_con[21],
			     HCLK_BUS_SEL_MASK | HCLK_BUS_DIV_MASK,
			     (sel << HCLK_BUS_SEL_SHIFT) |
			     ((div - 1) << HCLK_BUS_DIV_SHIFT));
		break;
	case PCLK_BUS_ROOT:
		rk_clrsetreg(&cru->clksel_con[22],
			     PCLK_BUS_SEL_MASK | PCLK_BUS_DIV_MASK,
			     (sel << PCLK_BUS_SEL_SHIFT) |
			     ((div - 1) << PCLK_BUS_DIV_SHIFT));
		break;
	default:
		return -ENOENT;
	}

	return rk3506_bus_get_rate(priv, clk_id);
}

static ulong rk3506_peri_get_rate(struct rk3506_clk_priv *priv, ulong clk_id)
{
	struct rk3506_cru *cru = priv->cru;
	u32 sel, con, div;
	ulong prate;

	switch (clk_id) {
	case ACLK_HSPERI_ROOT:
		con = readl(&cru->clksel_con[49]);
		sel = (con & ACLK_HSPERI_SEL_MASK) >> ACLK_HSPERI_SEL_SHIFT;
		div = (con & ACLK_HSPERI_DIV_MASK) >> ACLK_HSPERI_DIV_SHIFT;
		break;
	case HCLK_LSPERI_ROOT:
		con = readl(&cru->clksel_con[29]);
		sel = (con & HCLK_LSPERI_SEL_MASK) >> HCLK_LSPERI_SEL_SHIFT;
		div = (con & HCLK_LSPERI_DIV_MASK) >> HCLK_LSPERI_DIV_SHIFT;
		break;
	default:
		return -ENOENT;
	}

	if (sel == ACLK_HSPERI_SEL_GPLL_DIV)
		prate = priv->gpll_div_hz;
	else if (sel == ACLK_HSPERI_SEL_V0PLL_DIV)
		prate = priv->v0pll_div_hz;
	else if (sel == ACLK_HSPERI_SEL_V1PLL_DIV)
		prate = priv->v1pll_div_hz;
	else
		return -EINVAL;

	return DIV_TO_RATE(prate, div);
}

static ulong rk3506_peri_set_rate(struct rk3506_clk_priv *priv, ulong clk_id,
				  ulong rate)
{
	struct rk3506_cru *cru = priv->cru;
	u32 sel, div;

	if (priv->v0pll_div_hz % rate == 0) {
		sel = ACLK_BUS_SEL_V0PLL_DIV;
		div = DIV_ROUND_UP(priv->v0pll_div_hz, rate);
	} else if (priv->v1pll_div_hz % rate == 0) {
		sel = ACLK_BUS_SEL_V1PLL_DIV;
		div = DIV_ROUND_UP(priv->v1pll_div_hz, rate);
	} else {
		sel = ACLK_BUS_SEL_GPLL_DIV;
		div = DIV_ROUND_UP(priv->gpll_div_hz, rate);
	}
	assert(div - 1 <= 31);

	switch (clk_id) {
	case ACLK_HSPERI_ROOT:
		rk_clrsetreg(&cru->clksel_con[49],
			     ACLK_HSPERI_SEL_MASK | ACLK_HSPERI_DIV_MASK,
			     (sel << ACLK_HSPERI_SEL_SHIFT) |
			     ((div - 1) << ACLK_HSPERI_DIV_SHIFT));
		break;
	case HCLK_LSPERI_ROOT:
		rk_clrsetreg(&cru->clksel_con[29],
			     HCLK_LSPERI_SEL_MASK | HCLK_LSPERI_DIV_MASK,
			     (sel << HCLK_LSPERI_SEL_SHIFT) |
			     ((div - 1) << HCLK_LSPERI_DIV_SHIFT));
		break;
	default:
		return -ENOENT;
	}

	return rk3506_peri_get_rate(priv, clk_id);
}

static ulong rk3506_sdmmc_get_rate(struct rk3506_clk_priv *priv, ulong clk_id)
{
	struct rk3506_cru *cru = priv->cru;
	u32 sel, con, div;
	ulong prate;

	con = readl(&cru->clksel_con[49]);
	sel = (con & CCLK_SDMMC_SEL_MASK) >> CCLK_SDMMC_SEL_SHIFT;
	div = (con & CCLK_SDMMC_DIV_MASK) >> CCLK_SDMMC_DIV_SHIFT;

	if (sel == CCLK_SDMMC_SEL_24M)
		prate = OSC_HZ;
	else if (sel == CCLK_SDMMC_SEL_GPLL)
		prate = priv->gpll_hz;
	else if (sel == CCLK_SDMMC_SEL_V0PLL)
		prate = priv->v0pll_hz;
	else if (sel == CCLK_SDMMC_SEL_V1PLL)
		prate = priv->v1pll_hz;
	else
		return -EINVAL;

	return DIV_TO_RATE(prate, div);
}

static ulong rk3506_sdmmc_set_rate(struct rk3506_clk_priv *priv, ulong clk_id,
				  ulong rate)
{
	struct rk3506_cru *cru = priv->cru;
	u32 sel, div;

	if (OSC_HZ % rate == 0) {
		sel = CCLK_SDMMC_SEL_24M;
		div = DIV_ROUND_UP(OSC_HZ, rate);
	} else if (priv->v0pll_hz % rate == 0) {
		sel = CCLK_SDMMC_SEL_V0PLL;
		div = DIV_ROUND_UP(priv->v0pll_hz, rate);
	} else if (priv->v1pll_hz % rate == 0) {
		sel= CCLK_SDMMC_SEL_V1PLL;
		div = DIV_ROUND_UP(priv->v1pll_hz, rate);
	} else {
		sel= CCLK_SDMMC_SEL_GPLL;
		div = DIV_ROUND_UP(priv->gpll_hz, rate);
	}
	assert(div - 1 <= 63);

	rk_clrsetreg(&cru->clksel_con[49],
		     CCLK_SDMMC_SEL_MASK | CCLK_SDMMC_DIV_MASK,
		     (sel << CCLK_SDMMC_SEL_SHIFT) |
		     ((div - 1) << CCLK_SDMMC_DIV_SHIFT));

	return rk3506_sdmmc_get_rate(priv, clk_id);
}

static ulong rk3506_saradc_get_rate(struct rk3506_clk_priv *priv, ulong clk_id)
{
	struct rk3506_cru *cru = priv->cru;
	u32 div, con, sel;
	ulong prate;

	con = readl(&cru->clksel_con[54]);
	div = (con & CLK_SARADC_DIV_MASK) >> CLK_SARADC_DIV_SHIFT;
	sel = (con & CLK_SARADC_SEL_MASK) >> CLK_SARADC_SEL_SHIFT;

	if (sel == CLK_SARADC_SEL_24M)
		prate = OSC_HZ;
	else if (sel == CLK_SARADC_SEL_400K)
		prate = 400000;
	else if (sel == CLK_SARADC_SEL_32K)
		prate = 32000;
	else
		return -EINVAL;

	return DIV_TO_RATE(prate, div);
}

static ulong rk3506_saradc_set_rate(struct rk3506_clk_priv *priv, ulong clk_id,
				    ulong rate)
{
	struct rk3506_cru *cru = priv->cru;
	u32 div, sel;

	if (32000 % rate == 0) {
		sel = CLK_SARADC_SEL_32K;
		div = 1;
	} else if (400000 % rate == 0) {
		sel = CLK_SARADC_SEL_400K;
		div = 1;
	} else {
		sel= CLK_SARADC_SEL_24M;
		div = DIV_ROUND_UP(OSC_HZ, rate);
	}
	assert(div - 1 <= 15);

	rk_clrsetreg(&cru->clksel_con[54],
		     CLK_SARADC_SEL_MASK | CLK_SARADC_DIV_MASK,
		     (sel << CLK_SARADC_SEL_SHIFT) |
		     ((div - 1) << CLK_SARADC_DIV_SHIFT));

	return rk3506_saradc_get_rate(priv, clk_id);
}

static ulong rk3506_tsadc_get_rate(struct rk3506_clk_priv *priv, ulong clk_id)
{
	struct rk3506_cru *cru = priv->cru;
	u32 div, con;

	con = readl(&cru->clksel_con[61]);
	switch (clk_id) {
	case CLK_TSADC_TSEN:
		div = (con & CLK_TSADC_TSEN_DIV_MASK) >> CLK_TSADC_TSEN_DIV_SHIFT;
		break;
	case CLK_TSADC:
		div = (con & CLK_TSADC_DIV_MASK) >> CLK_TSADC_DIV_SHIFT;
		break;
	default:
		return -ENOENT;
	}

	return DIV_TO_RATE(OSC_HZ, div);
}

static ulong rk3506_tsadc_set_rate(struct rk3506_clk_priv *priv, ulong clk_id,
				   ulong rate)
{
	struct rk3506_cru *cru = priv->cru;
	u32 div;

	switch (clk_id) {
	case CLK_TSADC_TSEN:
		div = DIV_ROUND_UP(OSC_HZ, rate);
		assert(div - 1 <= 7);
		rk_clrsetreg(&cru->clksel_con[61], CLK_TSADC_TSEN_DIV_MASK,
			     (div - 1) << CLK_TSADC_TSEN_DIV_SHIFT);
		break;
	case CLK_TSADC:
		div = DIV_ROUND_UP(OSC_HZ, rate);
		assert(div - 1 <= 255);
		rk_clrsetreg(&cru->clksel_con[61], CLK_TSADC_DIV_MASK,
			     (div - 1) << CLK_TSADC_DIV_SHIFT);
		break;
	default:
		return -ENOENT;
	}


	return rk3506_tsadc_get_rate(priv, clk_id);
}

static ulong rk3506_i2c_get_rate(struct rk3506_clk_priv *priv, ulong clk_id)
{
	struct rk3506_cru *cru = priv->cru;
	u32 sel, con, div;
	ulong prate;

	switch (clk_id) {
	case CLK_I2C0:
		con = readl(&cru->clksel_con[32]);
		sel = (con & CLK_I2C0_SEL_MASK) >> CLK_I2C0_SEL_SHIFT;
		div = (con & CLK_I2C0_DIV_MASK) >> CLK_I2C0_DIV_SHIFT;
	case CLK_I2C1:
		con = readl(&cru->clksel_con[32]);
		sel = (con & CLK_I2C1_SEL_MASK) >> CLK_I2C1_SEL_SHIFT;
		div = (con & CLK_I2C1_DIV_MASK) >> CLK_I2C1_DIV_SHIFT;
	case CLK_I2C2:
		con = readl(&cru->clksel_con[33]);
		sel = (con & CLK_I2C2_SEL_MASK) >> CLK_I2C2_SEL_SHIFT;
		div = (con & CLK_I2C2_DIV_MASK) >> CLK_I2C2_DIV_SHIFT;
		break;
	default:
		return -ENOENT;
	}

	if (sel == CLK_I2C_SEL_GPLL)
		prate = priv->gpll_hz;
	else if (sel == CLK_I2C_SEL_V0PLL)
		prate = priv->v0pll_hz;
	else if (sel == CLK_I2C_SEL_V1PLL)
		prate = priv->v1pll_hz;
	else
		return -EINVAL;

	return DIV_TO_RATE(prate, div);
}

static ulong rk3506_i2c_set_rate(struct rk3506_clk_priv *priv, ulong clk_id,
				 ulong rate)
{
	struct rk3506_cru *cru = priv->cru;
	u32 sel, div;

	if (priv->v0pll_hz % rate == 0) {
		sel = CLK_I2C_SEL_V0PLL;
		div = DIV_ROUND_UP(priv->v0pll_hz, rate);
	} else if (priv->v1pll_hz % rate == 0) {
		sel = CLK_I2C_SEL_V1PLL;
		div = DIV_ROUND_UP(priv->v1pll_hz, rate);
	} else {
		sel = CLK_I2C_SEL_GPLL;
		div = DIV_ROUND_UP(priv->gpll_hz, rate);
	}
	assert(div - 1 <= 15);

	switch (clk_id) {
	case CLK_I2C0:
		rk_clrsetreg(&cru->clksel_con[32],
			     CLK_I2C0_SEL_MASK | CLK_I2C0_DIV_MASK,
			     (sel << CLK_I2C0_SEL_SHIFT) |
			     ((div - 1) << CLK_I2C0_DIV_SHIFT));
		break;
	case CLK_I2C1:
		rk_clrsetreg(&cru->clksel_con[32],
			     CLK_I2C1_SEL_MASK | CLK_I2C1_DIV_MASK,
			     (sel << CLK_I2C1_SEL_SHIFT) |
			     ((div - 1) << CLK_I2C1_DIV_SHIFT));
		break;
	case CLK_I2C2:
		rk_clrsetreg(&cru->clksel_con[33],
			     CLK_I2C2_SEL_MASK | CLK_I2C2_DIV_MASK,
			     (sel << CLK_I2C2_SEL_SHIFT) |
			     ((div - 1) << CLK_I2C2_DIV_SHIFT));
		break;
	default:
		return -ENOENT;
	}


	return rk3506_i2c_get_rate(priv, clk_id);
}

static ulong rk3506_pwm_get_rate(struct rk3506_clk_priv *priv, ulong clk_id)
{
	struct rk3506_cru *cru = priv->cru;
	u32 sel, con, div;
	ulong prate;

	switch (clk_id) {
	case CLK_PWM0:
		con = readl(&cru->pmuclksel_con[0]);
		div = (con & CLK_PWM0_DIV_MASK) >> CLK_PWM0_DIV_SHIFT;
		prate = priv->gpll_div_100mhz;
		break;
	case CLK_PWM1:
		con = readl(&cru->clksel_con[33]);
		sel = (con & CLK_PWM1_SEL_MASK) >> CLK_PWM1_SEL_SHIFT;
		div = (con & CLK_PWM1_DIV_MASK) >> CLK_PWM1_DIV_SHIFT;
		if (sel == CLK_PWM1_SEL_GPLL_DIV)
			prate = priv->gpll_div_hz;
		else if (sel == CLK_PWM1_SEL_V0PLL_DIV)
			prate = priv->v0pll_div_hz;
		else if (sel == CLK_PWM1_SEL_V1PLL_DIV)
			prate = priv->v1pll_div_hz;
		else
			return -EINVAL;
		break;
	default:
		return -ENOENT;
	}

	return DIV_TO_RATE(prate, div);
}

static ulong rk3506_pwm_set_rate(struct rk3506_clk_priv *priv, ulong clk_id,
				 ulong rate)
{
	struct rk3506_cru *cru = priv->cru;
	u32 sel, div;

	switch (clk_id) {
	case CLK_PWM0:
		div = DIV_ROUND_UP(priv->gpll_div_100mhz, rate);
		assert(div - 1 <= 15);
		rk_clrsetreg(&cru->pmuclksel_con[0], CLK_PWM0_DIV_MASK,
			     (div - 1) << CLK_PWM0_DIV_SHIFT);
		break;
	case CLK_PWM1:
		if (priv->v0pll_hz % rate == 0) {
			sel = CLK_PWM1_SEL_V0PLL_DIV;
			div = DIV_ROUND_UP(priv->v0pll_div_hz, rate);
		} else if (priv->v1pll_hz % rate == 0) {
			sel = CLK_PWM1_SEL_V1PLL_DIV;
			div = DIV_ROUND_UP(priv->v1pll_div_hz, rate);
		} else {
			sel = CLK_PWM1_SEL_GPLL_DIV;
			div = DIV_ROUND_UP(priv->gpll_div_hz, rate);
		}
		assert(div - 1 <= 15);
		rk_clrsetreg(&cru->clksel_con[33],
			     CLK_PWM1_SEL_MASK | CLK_PWM1_DIV_MASK,
			     (sel << CLK_PWM1_SEL_SHIFT) |
			     ((div - 1) << CLK_PWM1_DIV_SHIFT));
		break;
	default:
		return -ENOENT;
	}

	return rk3506_pwm_get_rate(priv, clk_id);
}

static ulong rk3506_spi_get_rate(struct rk3506_clk_priv *priv, ulong clk_id)
{
	struct rk3506_cru *cru = priv->cru;
	u32 sel, con, div;
	ulong prate;

	switch (clk_id) {
	case CLK_SPI0:
		con = readl(&cru->clksel_con[34]);
		sel = (con & CLK_SPI0_SEL_MASK) >> CLK_SPI0_SEL_SHIFT;
		div = (con & CLK_SPI0_DIV_MASK) >> CLK_SPI0_DIV_SHIFT;
		break;
	case CLK_SPI1:
		con = readl(&cru->clksel_con[34]);
		sel = (con & CLK_SPI1_SEL_MASK) >> CLK_SPI1_SEL_SHIFT;
		div = (con & CLK_SPI1_DIV_MASK) >> CLK_SPI1_DIV_SHIFT;
		break;
	default:
		return -ENOENT;
	}

	if (sel == CLK_SPI_SEL_24M)
		prate = OSC_HZ;
	else if (sel == CLK_SPI_SEL_GPLL_DIV)
		prate = priv->gpll_div_hz;
	else if (sel == CLK_SPI_SEL_V0PLL_DIV)
		prate = priv->v0pll_div_hz;
	else if (sel == CLK_SPI_SEL_V1PLL_DIV)
		prate = priv->v1pll_div_hz;
	else
		return -EINVAL;

	return DIV_TO_RATE(prate, div);
}

static ulong rk3506_spi_set_rate(struct rk3506_clk_priv *priv, ulong clk_id,
				 ulong rate)
{
	struct rk3506_cru *cru = priv->cru;
	u32 sel, div;

	if (OSC_HZ % rate == 0) {
		sel = CLK_SPI_SEL_24M;
		div = DIV_ROUND_UP(OSC_HZ, rate);
	} else if (priv->v0pll_div_hz % rate == 0) {
		sel = CLK_SPI_SEL_V0PLL_DIV;
		div = DIV_ROUND_UP(priv->v0pll_div_hz, rate);
	} else if (priv->v1pll_div_hz % rate == 0) {
		sel = CLK_SPI_SEL_V1PLL_DIV;
		div = DIV_ROUND_UP(priv->v1pll_div_hz, rate);
	} else {
		sel = CLK_SPI_SEL_GPLL_DIV;
		div = DIV_ROUND_UP(priv->gpll_div_hz, rate);
	}
	assert(div - 1 <= 15);

	switch (clk_id) {
	case CLK_SPI0:
		rk_clrsetreg(&cru->clksel_con[34],
			     CLK_SPI0_SEL_MASK | CLK_SPI0_DIV_MASK,
			     (sel << CLK_SPI0_SEL_SHIFT) |
			     ((div - 1) << CLK_SPI0_DIV_SHIFT));
		break;
	case CLK_SPI1:
		rk_clrsetreg(&cru->clksel_con[34],
			     CLK_SPI1_SEL_MASK | CLK_SPI1_DIV_MASK,
			     (sel << CLK_SPI1_SEL_SHIFT) |
			     ((div - 1) << CLK_SPI1_DIV_SHIFT));
		break;
	default:
		return -ENOENT;
	}

	return rk3506_spi_get_rate(priv, clk_id);
}

static ulong rk3506_fspi_get_rate(struct rk3506_clk_priv *priv)
{
	struct rk3506_cru *cru = priv->cru;
	u32 div, sel, con, prate;

	con = readl(&cru->clksel_con[50]);
	div = (con & SCLK_FSPI_DIV_MASK) >> SCLK_FSPI_DIV_SHIFT;
	sel = (con & SCLK_FSPI_SEL_MASK) >> SCLK_FSPI_SEL_SHIFT;
	if (sel == SCLK_FSPI_SEL_24M)
		prate = OSC_HZ;
	else if (sel == SCLK_FSPI_SEL_GPLL)
		prate = priv->gpll_hz;
	else if (sel == SCLK_FSPI_SEL_V0PLL)
		prate = priv->v0pll_hz;
	else if (sel == SCLK_FSPI_SEL_V1PLL)
		prate = priv->v1pll_hz;
	else
		return -EINVAL;

	return DIV_TO_RATE(prate, div);
}

static ulong rk3506_fspi_set_rate(struct rk3506_clk_priv *priv, ulong rate)
{
	struct rk3506_cru *cru = priv->cru;
	int div, sel;

	if (OSC_HZ % rate == 0) {
		sel = SCLK_FSPI_SEL_24M;
		div = DIV_ROUND_UP(OSC_HZ, rate);
	} else if ((priv->v0pll_hz % rate) == 0) {
		sel = SCLK_FSPI_SEL_V0PLL;
		div = DIV_ROUND_UP(priv->v0pll_hz, rate);
	} else if ((priv->v1pll_hz % rate) == 0) {
		sel = SCLK_FSPI_SEL_V1PLL;
		div = DIV_ROUND_UP(priv->v1pll_hz, rate);
	} else {
		sel = SCLK_FSPI_SEL_GPLL;
		div = DIV_ROUND_UP(priv->gpll_hz, rate);
	}
	assert(div - 1 <= 31);

	rk_clrsetreg(&cru->clksel_con[50],
		     SCLK_FSPI_SEL_MASK | SCLK_FSPI_DIV_MASK,
		     sel << SCLK_FSPI_SEL_SHIFT |
		     (div - 1) << SCLK_FSPI_DIV_SHIFT);

	return rk3506_fspi_get_rate(priv);
}

static ulong rk3506_vop_dclk_get_rate(struct rk3506_clk_priv *priv)
{
	struct rk3506_cru *cru = priv->cru;
	u32 div, sel, con, prate;

	con = readl(&cru->clksel_con[60]);
	div = (con & DCLK_VOP_DIV_MASK) >> DCLK_VOP_DIV_SHIFT;
	sel = (con & DCLK_VOP_SEL_MASK) >> DCLK_VOP_SEL_SHIFT;

	if (sel == DCLK_VOP_SEL_24M)
		prate = OSC_HZ;
	else if (sel == DCLK_VOP_SEL_GPLL)
		prate = priv->gpll_hz;
	else if (sel == DCLK_VOP_SEL_V0PLL)
		prate = priv->v0pll_hz;
	else if (sel == DCLK_VOP_SEL_V1PLL)
		prate = priv->v1pll_hz;
	else
		return -EINVAL;

	return DIV_TO_RATE(prate, div);
}

static ulong rk3506_vop_dclk_set_rate(struct rk3506_clk_priv *priv, ulong rate)
{
	struct rk3506_cru *cru = priv->cru;
	int div, sel;

	if (OSC_HZ % rate == 0) {
		sel = DCLK_VOP_SEL_24M;
		div = DIV_ROUND_UP(OSC_HZ, rate);
	} else if ((priv->v0pll_hz % rate) == 0) {
		sel = DCLK_VOP_SEL_V0PLL;
		div = DIV_ROUND_UP(priv->v0pll_hz, rate);
	} else if ((priv->v1pll_hz % rate) == 0) {
		sel = DCLK_VOP_SEL_V1PLL;
		div = DIV_ROUND_UP(priv->v1pll_hz, rate);
	} else {
		sel = DCLK_VOP_SEL_GPLL;
		div = DIV_ROUND_UP(priv->gpll_hz, rate);
	}
	assert(div - 1 <= 255);

	rk_clrsetreg(&cru->clksel_con[60],
		     DCLK_VOP_SEL_MASK | DCLK_VOP_DIV_MASK,
		     sel << DCLK_VOP_SEL_SHIFT |
		     (div - 1) << DCLK_VOP_DIV_SHIFT);

	return rk3506_vop_dclk_get_rate(priv);
}

static ulong rk3506_mac_get_rate(struct rk3506_clk_priv *priv, ulong clk_id)
{
	struct rk3506_cru *cru = priv->cru;
	u32 div, con;

	switch (clk_id) {
	case CLK_MAC0:
	case CLK_MAC1:
		con = readl(&cru->clksel_con[50]);
		div = (con & CLK_MAC_DIV_MASK) >> CLK_MAC_DIV_SHIFT;
		break;
	case CLK_MAC0_PTP:
	case CLK_MAC1_PTP:
		con = readl(&cru->clksel_con[55]);
		div = (con & CLK_MAC_PTP_ROOT_DIV_MASK) >> CLK_MAC_PTP_ROOT_DIV_SHIFT;
		break;
	case CLK_MAC_OUT:
		con = readl(&cru->pmuclksel_con[0]);
		div = (con & CLK_MAC_OUT_DIV_MASK) >> CLK_MAC_OUT_DIV_SHIFT;
		break;
	default:
		return -ENOENT;
	}

	return DIV_TO_RATE(priv->gpll_hz, div);
}

static ulong rk3506_mac_set_rate(struct rk3506_clk_priv *priv, ulong clk_id,
				 ulong rate)
{
	struct rk3506_cru *cru = priv->cru;
	u32 div;

	switch (clk_id) {
	case CLK_MAC0:
	case CLK_MAC1:
		div = DIV_ROUND_UP(priv->gpll_hz, rate);
		rk_clrsetreg(&cru->clksel_con[50], CLK_MAC_DIV_MASK,
			     ((div - 1) << CLK_MAC_DIV_SHIFT));
		break;
	case CLK_MAC0_PTP:
	case CLK_MAC1_PTP:
		div = DIV_ROUND_UP(priv->gpll_hz, rate);
		rk_clrsetreg(&cru->clksel_con[55], CLK_MAC_PTP_ROOT_DIV_MASK,
			     ((div - 1) << CLK_MAC_PTP_ROOT_DIV_SHIFT));
		break;
	case CLK_MAC_OUT:
		div = DIV_ROUND_UP(priv->gpll_hz, rate);
		rk_clrsetreg(&cru->pmuclksel_con[0], CLK_MAC_OUT_DIV_MASK,
			     ((div - 1) << CLK_MAC_OUT_DIV_SHIFT));
		break;
	default:
		return -ENOENT;
	}

	return rk3506_mac_get_rate(priv, clk_id);
}

static ulong rk3506_clk_get_rate(struct clk *clk)
{
	struct rk3506_clk_priv *priv = dev_get_priv(clk->dev);
	ulong rate = 0;

	if (!priv->gpll_hz || !priv->v0pll_hz || !priv->v1pll_hz) {
		printf("%s: gpll=%lu, v0pll=%lu, v1pll=%lu\n",
		       __func__, priv->gpll_hz, priv->v0pll_hz, priv->v1pll_hz);
		return -ENOENT;
	}

	switch (clk->id) {
	case PLL_GPLL:
		rate = priv->gpll_hz;
		break;
	case PLL_V0PLL:
		rate = priv->v0pll_hz;
		break;
	case PLL_V1PLL:
		rate = priv->v1pll_hz;
		break;
	case ARMCLK:
		rate = rk3506_armclk_get_rate(priv);
		break;
	case CLK_GPLL_DIV:
	case CLK_GPLL_DIV_100M:
	case CLK_V0PLL_DIV:
	case CLK_V1PLL_DIV:
		rate = rk3506_pll_div_get_rate(priv, clk->id);
		break;
	case ACLK_BUS_ROOT:
	case HCLK_BUS_ROOT:
	case PCLK_BUS_ROOT:
		rate = rk3506_bus_get_rate(priv, clk->id);
		break;
	case ACLK_HSPERI_ROOT:
	case HCLK_LSPERI_ROOT:
		rate = rk3506_peri_get_rate(priv, clk->id);
		break;
	case HCLK_SDMMC:
	case CCLK_SRC_SDMMC:
		rate = rk3506_sdmmc_get_rate(priv, clk->id);
		break;
	case CLK_SARADC:
		rate = rk3506_saradc_get_rate(priv, clk->id);
		break;
	case CLK_TSADC:
	case CLK_TSADC_TSEN:
		rate = rk3506_tsadc_get_rate(priv, clk->id);
		break;
	case CLK_I2C0:
	case CLK_I2C1:
	case CLK_I2C2:
		rate = rk3506_i2c_get_rate(priv, clk->id);
		break;
	case CLK_PWM0:
	case CLK_PWM1:
		rate = rk3506_pwm_get_rate(priv, clk->id);
		break;
	case CLK_SPI0:
	case CLK_SPI1:
		rate = rk3506_spi_get_rate(priv, clk->id);
		break;
	case SCLK_FSPI:
		rate = rk3506_fspi_get_rate(priv);
		break;
	case DCLK_VOP:
		rate = rk3506_vop_dclk_get_rate(priv);
		break;
	case CLK_MAC0:
	case CLK_MAC1:
	case CLK_MAC0_PTP:
	case CLK_MAC1_PTP:
	case CLK_MAC_OUT:
		rate = rk3506_mac_get_rate(priv, clk->id);
		break;
	default:
		return -ENOENT;
	}

	return rate;
};

static ulong rk3506_clk_set_rate(struct clk *clk, ulong rate)
{
	struct rk3506_clk_priv *priv = dev_get_priv(clk->dev);
	ulong ret = 0;

	if (!priv->gpll_hz || !priv->v0pll_hz || !priv->v1pll_hz) {
		printf("%s: gpll=%lu, v0pll=%lu, v1pll=%lu\n",
		       __func__, priv->gpll_hz, priv->v0pll_hz, priv->v1pll_hz);
		return -ENOENT;
	}

	debug("%s: id=%ld, rate=%ld\n", __func__, clk->id, rate);

	switch (clk->id) {
	case ARMCLK:
		if (priv->armclk_hz)
			rk3506_armclk_set_rate(priv, rate);
		priv->armclk_hz = rate;
		break;
	case CLK_GPLL_DIV:
	case CLK_GPLL_DIV_100M:
	case CLK_V0PLL_DIV:
	case CLK_V1PLL_DIV:
		ret = rk3506_pll_div_set_rate(priv, clk->id, rate);
		break;
	case ACLK_BUS_ROOT:
	case HCLK_BUS_ROOT:
	case PCLK_BUS_ROOT:
		ret = rk3506_bus_set_rate(priv, clk->id, rate);
		break;
	case ACLK_HSPERI_ROOT:
	case HCLK_LSPERI_ROOT:
		ret = rk3506_peri_set_rate(priv, clk->id, rate);
		break;
	case HCLK_SDMMC:
	case CCLK_SRC_SDMMC:
		ret = rk3506_sdmmc_set_rate(priv, clk->id, rate);
		break;
	case CLK_SARADC:
		ret = rk3506_saradc_set_rate(priv, clk->id, rate);
		break;
	case CLK_TSADC:
	case CLK_TSADC_TSEN:
		ret = rk3506_tsadc_set_rate(priv, clk->id, rate);
		break;
	case CLK_I2C0:
	case CLK_I2C1:
	case CLK_I2C2:
		ret = rk3506_i2c_set_rate(priv, clk->id, rate);
		break;
	case CLK_PWM0:
	case CLK_PWM1:
		ret = rk3506_pwm_set_rate(priv, clk->id, rate);
		break;
	case CLK_SPI0:
	case CLK_SPI1:
		ret = rk3506_spi_set_rate(priv, clk->id, rate);
		break;
	case HCLK_FSPI:
	case SCLK_FSPI:
		ret = rk3506_fspi_set_rate(priv, rate);
		break;
	case DCLK_VOP:
		ret = rk3506_vop_dclk_set_rate(priv, rate);
		break;
	case CLK_MAC0:
	case CLK_MAC1:
	case CLK_MAC0_PTP:
	case CLK_MAC1_PTP:
	case CLK_MAC_OUT:
		ret = rk3506_mac_set_rate(priv, clk->id, rate);
		break;
	default:
		return -ENOENT;
	}

	return ret;
};

static struct clk_ops rk3506_clk_ops = {
	.get_rate = rk3506_clk_get_rate,
	.set_rate = rk3506_clk_set_rate,
};

static void rk3506_clk_init(struct rk3506_clk_priv *priv)
{
	priv->sync_kernel = false;

#ifdef CONFIG_XPL_BUILD
	/* Init pka crypto rate, sel=v0pll, div=3 */
	writel(0x3f801180, RK3506_SCRU_BASE + 0x0010);
	/* Change clk core src rate, sel=gpll, div=3 */
	writel(0x007f0003, ((ulong)RK3506_CRU_BASE) + 0x033c);
#endif

	if (!priv->gpll_hz) {
		priv->gpll_hz = rockchip_pll_get_rate(&rk3506_pll_clks[GPLL],
						      priv->cru, GPLL);
		priv->gpll_hz = roundup(priv->gpll_hz, 1000);
	}
	if (!priv->v0pll_hz) {
		priv->v0pll_hz = rockchip_pll_get_rate(&rk3506_pll_clks[V0PLL],
						       priv->cru, V0PLL);
		priv->v0pll_hz = roundup(priv->v0pll_hz, 1000);
	}
	if (!priv->v1pll_hz) {
		priv->v1pll_hz = rockchip_pll_get_rate(&rk3506_pll_clks[V1PLL],
						       priv->cru, V1PLL);
		priv->v1pll_hz = roundup(priv->v1pll_hz, 1000);
	}
	if (!priv->gpll_div_hz) {
		priv->gpll_div_hz = rk3506_pll_div_get_rate(priv, CLK_GPLL_DIV);
		priv->gpll_div_hz = roundup(priv->gpll_div_hz, 1000);
	}
	if (!priv->gpll_div_100mhz) {
		priv->gpll_div_100mhz = rk3506_pll_div_get_rate(priv, CLK_GPLL_DIV_100M);
		priv->gpll_div_100mhz = roundup(priv->gpll_div_100mhz, 1000);
	}
	if (!priv->v0pll_div_hz) {
		priv->v0pll_div_hz = rk3506_pll_div_get_rate(priv, CLK_V0PLL_DIV);
		priv->v0pll_div_hz = roundup(priv->v0pll_div_hz, 1000);
	}
	if (!priv->v1pll_div_hz) {
		priv->v1pll_div_hz = rk3506_pll_div_get_rate(priv, CLK_V1PLL_DIV);
		priv->v1pll_div_hz = roundup(priv->v1pll_div_hz, 1000);
	}
	if (!priv->armclk_enter_hz) {
		priv->armclk_enter_hz = rk3506_armclk_get_rate(priv);
		priv->armclk_init_hz = priv->armclk_enter_hz;
	}
}

static int rk3506_clk_probe(struct udevice *dev)
{
	struct rk3506_clk_priv *priv = dev_get_priv(dev);
	int ret;

	rk3506_clk_init(priv);

	/* Process 'assigned-{clocks/clock-parents/clock-rates}' properties */
	ret = clk_set_defaults(dev, 1);
	if (ret)
		debug("%s clk_set_defaults failed %d\n", __func__, ret);
	else
		priv->sync_kernel = true;

	return 0;
}

static int rk3506_clk_ofdata_to_platdata(struct udevice *dev)
{
	struct rk3506_clk_priv *priv = dev_get_priv(dev);

	priv->cru = dev_read_addr_ptr(dev);

	return 0;
}

static int rk3506_clk_bind(struct udevice *dev)
{
	struct udevice *sys_child;
	struct sysreset_reg *priv;
	int ret;

	/* The reset driver does not have a device node, so bind it here */
	ret = device_bind_driver(dev, "rockchip_sysreset", "sysreset",
				 &sys_child);
	if (ret) {
		debug("Warning: No sysreset driver: ret=%d\n", ret);
	} else {
		priv = malloc(sizeof(struct sysreset_reg));
		priv->glb_srst_fst_value = offsetof(struct rk3506_cru,
						    glb_srst_fst);
		priv->glb_srst_snd_value = offsetof(struct rk3506_cru,
						    glb_srst_snd);
		dev_set_priv(sys_child, priv);
	}

#if CONFIG_IS_ENABLED(RESET_ROCKCHIP)
	ret = offsetof(struct rk3506_cru, softrst_con[0]);
	ret = rk3506_reset_bind_lut(dev, ret, 23);
	if (ret)
		debug("Warning: software reset driver bind failed\n");
#endif

	return 0;
}

static const struct udevice_id rk3506_clk_ids[] = {
	{ .compatible = "rockchip,rk3506-cru" },
	{ }
};

U_BOOT_DRIVER(rockchip_rk3506_cru) = {
	.name		= "rockchip_rk3506_cru",
	.id		= UCLASS_CLK,
	.of_match	= rk3506_clk_ids,
	.priv_auto	= sizeof(struct rk3506_clk_priv),
	.of_to_plat	= rk3506_clk_ofdata_to_platdata,
	.ops		= &rk3506_clk_ops,
	.bind		= rk3506_clk_bind,
	.probe		= rk3506_clk_probe,
};

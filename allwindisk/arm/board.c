/*
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * Some init for sunxi platform.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <io.h>
#include <diskboot.h>

#define SUNXI_CCM_BASE              0X01C20000
#define SUNXI_CCM_PLL1_CFG			(SUNXI_CCM_BASE + 0x00)		/* PLL1 control */
#define SUNXI_CCM_OSC24M_CFG		(SUNXI_CCM_BASE + 0x50)		/* osc 24M control */
#define SUNXI_CCM_CPU_AHB_APB0_CFG	(SUNXI_CCM_BASE + 0x54)		/* cpu ahb apb0 divide ratio */
#define SUNXI_CCM_APB1_CLK_DIV		(SUNXI_CCM_BASE + 0x58)
#define SUNXI_CCM_AXI_GATING		(SUNXI_CCM_BASE + 0x5C)
#define SUNXI_CCM_AHB_GATING0		(SUNXI_CCM_BASE + 0x60)
#define SUNXI_CCM_AHB_GATING1		(SUNXI_CCM_BASE + 0x64)
#define SUNXI_CCM_APB0_GATING		(SUNXI_CCM_BASE + 0x68)
#define SUNXI_CCM_APB1_GATING		(SUNXI_CCM_BASE + 0x6C)
#define SUNXI_CCM_NAND_SCLK_CFG		(SUNXI_CCM_BASE + 0x80)

/* pll1 factors */
#define PLL1_FACTOR_N			8
#define PLL1_FACTOR_K			1
#define PLL1_FACTOR_M			0
#define PLL1_FACTOR_P			0

/* apb1 bit field */
#define APB1_CLK_SRC_OSC24M		0
#define APB1_FACTOR_M			0
#define APB1_FACTOR_N			0

/* clock divide */
#define CPU_CLK_SRC_OSC24M		1
#define CPU_CLK_SRC_PLL1		2
#define AXI_DIV					1
#define AHB_DIV					1
#define APB0_DIV				1


#define CLK_GATE_OPEN			0x1
#define CLK_GATE_CLOSE			0x0

/* nand clock */
#define NAND_CLK_SRC_OSC24		0
#define NAND_CLK_DIV_N			0
#define NAND_CLK_DIV_M			0


/*****************************************************************
 * sr32 - clear & set a value in a bit range for a 32 bit address
 *****************************************************************/
static void sr32(u32 addr, u32 start_bit, u32 num_bits, u32 value)
{
	u32 tmp, msk = 0;
	msk = 1 << num_bits;
	--msk;
	tmp = readl((u32)addr) & ~(msk << start_bit);
	tmp |= value << start_bit;
	writel(tmp, (u32)addr);
}

void clock_init(void)
{
	/* set clock source to OSC24M */
	sr32(SUNXI_CCM_CPU_AHB_APB0_CFG, 16, 2, CPU_CLK_SRC_OSC24M);		/* CPU_CLK_SRC_SEL [17:16] */

	/* set the pll1 factors, pll1 out = 24MHz*n*k/m/p */	

	sr32(SUNXI_CCM_PLL1_CFG, 8, 5, PLL1_FACTOR_N);		/* PLL1_FACTOR_N [12:8] */
	sr32(SUNXI_CCM_PLL1_CFG, 4, 2, PLL1_FACTOR_K);		/* PLL1_FACTOR_K [5:4] */
	sr32(SUNXI_CCM_PLL1_CFG, 0, 2, PLL1_FACTOR_M);		/* PLL1_FACTOR_M [1:0] */
	sr32(SUNXI_CCM_PLL1_CFG, 16, 2, PLL1_FACTOR_P);		/* PLL1_FACTOR_P [17:16] */

	/* wait for clock to be stable*/	
	sdelay(0x400);
	/* set clock divider, cpu:axi:ahb:apb0 = 8:4:2:1 */
	sr32(SUNXI_CCM_CPU_AHB_APB0_CFG, 0, 2, AXI_DIV);	/* AXI_CLK_DIV_RATIO [1:0] */
	sr32(SUNXI_CCM_CPU_AHB_APB0_CFG, 4, 2, AHB_DIV);	/* AHB_CLK_DIV_RATIO [5:4] */
	sr32(SUNXI_CCM_CPU_AHB_APB0_CFG, 8, 2, APB0_DIV);	/* APB0_CLK_DIV_RATIO [9:8] */

	/* change cpu clock source to pll1 */
	sr32(SUNXI_CCM_CPU_AHB_APB0_CFG, 16, 2, CPU_CLK_SRC_PLL1);/* CPU_CLK_SRC_SEL [17:16] */
	/* 
	 * if the clock source is changed,
	 * at most wait for 8 present running clock cycles
	 */
	sdelay(10);

	/* config apb1 clock */
	sr32(SUNXI_CCM_APB1_CLK_DIV, 24, 2, APB1_CLK_SRC_OSC24M);
	sr32(SUNXI_CCM_APB1_CLK_DIV, 16, 2, APB1_FACTOR_N);
	sr32(SUNXI_CCM_APB1_CLK_DIV, 0, 5, APB1_FACTOR_M);

	/* open the clock for uart0 */
	sr32(SUNXI_CCM_APB1_GATING, 16, 1, CLK_GATE_OPEN);

	/* config nand clock */
	sr32(SUNXI_CCM_NAND_SCLK_CFG, 24, 2, NAND_CLK_SRC_OSC24);
	sr32(SUNXI_CCM_NAND_SCLK_CFG, 16, 2, NAND_CLK_DIV_N);
	sr32(SUNXI_CCM_NAND_SCLK_CFG, 0, 4, NAND_CLK_DIV_M);
	sr32(SUNXI_CCM_NAND_SCLK_CFG, 31, 1, CLK_GATE_OPEN);
	/* open clock for nand */
	sr32(SUNXI_CCM_AHB_GATING0, 13, 1, CLK_GATE_OPEN);

    /* open DMA clock */
    sr32(SUNXI_CCM_AHB_GATING0, 6, 1, CLK_GATE_OPEN);
}

void clock_restore(void)
{
    /* close DMA clock */
    sr32(SUNXI_CCM_AHB_GATING0, 6, 1, CLK_GATE_CLOSE);

    /* set clock source to OSC24M */
    sr32(SUNXI_CCM_CPU_AHB_APB0_CFG, 16, 2, CPU_CLK_SRC_OSC24M);
}


/*
 * (C) Copyright 2009
 * jung hyun kim, Nexell Co, <jhkim@nexell.co.kr>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <netdev.h>

/*------------------------------------------------------------------------------
 * u-boot eth interface
 */

#include <asm/io.h>
#include <asm/gpio.h>
//#include <asm/sections.h>

#include <platform.h>
#include <mach-api.h>
#include <rtc_nxp.h>
#include <net.h>
#include <phy.h>

#define	CFG_ETHER_GMAC_PHY_RST_NUM				(PAD_GPIO_E + 22)

int  nxpmac_init()
{
  u32 addr;

	/* Clock control */
	NX_CLKGEN_Initialize();
	addr = NX_CLKGEN_GetPhysicalAddress(CLOCKINDEX_OF_DWC_GMAC_MODULE);
	NX_CLKGEN_SetBaseAddress( CLOCKINDEX_OF_DWC_GMAC_MODULE, (void*)IO_ADDRESS(addr) );

	NX_CLKGEN_SetClockSource( CLOCKINDEX_OF_DWC_GMAC_MODULE, 0, 4);     // Sync mode for 100 & 10Base-T : External RX_clk
	NX_CLKGEN_SetClockDivisor( CLOCKINDEX_OF_DWC_GMAC_MODULE, 0, 1);    // Sync mode for 100 & 10Base-T

	NX_CLKGEN_SetClockOutInv( CLOCKINDEX_OF_DWC_GMAC_MODULE, 0, CFALSE);    // TX Clk invert off : 100 & 10Base-T

	NX_CLKGEN_SetClockDivisorEnable( CLOCKINDEX_OF_DWC_GMAC_MODULE, CTRUE);


	/* Reset control */
	NX_RSTCON_Initialize();
	addr = NX_RSTCON_GetPhysicalAddress();
	NX_RSTCON_SetBaseAddress( (void*)IO_ADDRESS(addr) );
	NX_RSTCON_SetRST(RESETINDEX_OF_DWC_GMAC_MODULE_aresetn_i, RSTCON_NEGATE);
	udelay(100);
	NX_RSTCON_SetRST(RESETINDEX_OF_DWC_GMAC_MODULE_aresetn_i, RSTCON_ASSERT);
	udelay(100);
	NX_RSTCON_SetRST(RESETINDEX_OF_DWC_GMAC_MODULE_aresetn_i, RSTCON_NEGATE);
	udelay(100);

    gpio_request(CFG_ETHER_GMAC_PHY_RST_NUM,"Ethernet Rst pin");
	gpio_direction_output(CFG_ETHER_GMAC_PHY_RST_NUM, 1 );
	udelay( 100 );
	gpio_set_value(CFG_ETHER_GMAC_PHY_RST_NUM, 0 );
	udelay( 100 );
	gpio_set_value(CFG_ETHER_GMAC_PHY_RST_NUM, 1 );

    gpio_free(CFG_ETHER_GMAC_PHY_RST_NUM);

     printf("GEC6818 mac init...\n");
	return 0;
}




#if defined(CONFIG_DESIGNWARE_ETH)
int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);
	return 0;
}
#endif

int GEC6818_board_eth_init(bd_t *bis)
{
#if defined(CONFIG_DESIGNWARE_ETH)
	u32 interface;
	int num = 0;

	nxpmac_init();

	interface = PHY_INTERFACE_MODE_RGMII;

	if (designware_initialize(CONFIG_DWCGMAC_BASE, interface) >= 0)
		num++;

	return num;
#endif  /* CONFIG_DESIGNWARE_ETH */
}



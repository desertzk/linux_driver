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
#include <config.h>
#include <common.h>
#include <asm/arch/display.h>

extern void display_rgb(int module, unsigned int fbbase,
					struct disp_vsync_info *pvsync, struct disp_syncgen_param *psgen,
					struct disp_multily_param *pmly, struct disp_rgb_param *prgb);

#define	INIT_VIDEO_SYNC(name)								\
	struct disp_vsync_info name = {							\
		.h_active_len	= CFG_DISP_PRI_RESOL_WIDTH,         \
		.h_sync_width	= CFG_DISP_PRI_HSYNC_SYNC_WIDTH,    \
		.h_back_porch	= CFG_DISP_PRI_HSYNC_BACK_PORCH,    \
		.h_front_porch	= CFG_DISP_PRI_HSYNC_FRONT_PORCH,   \
		.h_sync_invert	= CFG_DISP_PRI_HSYNC_ACTIVE_HIGH,   \
		.v_active_len	= CFG_DISP_PRI_RESOL_HEIGHT,        \
		.v_sync_width	= CFG_DISP_PRI_VSYNC_SYNC_WIDTH,    \
		.v_back_porch	= CFG_DISP_PRI_VSYNC_BACK_PORCH,    \
		.v_front_porch	= CFG_DISP_PRI_VSYNC_FRONT_PORCH,   \
		.v_sync_invert	= CFG_DISP_PRI_VSYNC_ACTIVE_HIGH,   \
		.pixel_clock_hz	= CFG_DISP_PRI_PIXEL_CLOCK,   		\
		.clk_src_lv0	= CFG_DISP_PRI_CLKGEN0_SOURCE,      \
		.clk_div_lv0	= CFG_DISP_PRI_CLKGEN0_DIV,         \
		.clk_src_lv1	= CFG_DISP_PRI_CLKGEN1_SOURCE,      \
		.clk_div_lv1	= CFG_DISP_PRI_CLKGEN1_DIV,         \
	};

#define	INIT_PARAM_SYNCGEN(name)						\
	struct disp_syncgen_param name = {						\
		.interlace 		= CFG_DISP_PRI_MLC_INTERLACE,       \
		.out_format		= CFG_DISP_PRI_OUT_FORMAT,          \
		.lcd_mpu_type 	= 0,                                \
		.invert_field 	= CFG_DISP_PRI_OUT_INVERT_FIELD,    \
		.swap_RB		= CFG_DISP_PRI_OUT_SWAPRB,          \
		.yc_order		= CFG_DISP_PRI_OUT_YCORDER,         \
		.delay_mask		= 0,                                \
		.vclk_select	= CFG_DISP_PRI_PADCLKSEL,           \
		.clk_delay_lv0	= CFG_DISP_PRI_CLKGEN0_DELAY,       \
		.clk_inv_lv0	= CFG_DISP_PRI_CLKGEN0_INVERT,      \
		.clk_delay_lv1	= CFG_DISP_PRI_CLKGEN1_DELAY,       \
		.clk_inv_lv1	= CFG_DISP_PRI_CLKGEN1_INVERT,      \
		.clk_sel_div1	= CFG_DISP_PRI_CLKSEL1_SELECT,		\
	};

#define	INIT_PARAM_MULTILY(name)					\
	struct disp_multily_param name = {						\
		.x_resol		= CFG_DISP_PRI_RESOL_WIDTH,			\
		.y_resol		= CFG_DISP_PRI_RESOL_HEIGHT,		\
		.pixel_byte		= CFG_DISP_PRI_SCREEN_PIXEL_BYTE,	\
		.fb_layer		= CFG_DISP_PRI_SCREEN_LAYER,		\
		.video_prior	= CFG_DISP_PRI_VIDEO_PRIORITY,		\
		.mem_lock_size	= 16,								\
		.rgb_format		= CFG_DISP_PRI_SCREEN_RGB_FORMAT,	\
		.bg_color		= CFG_DISP_PRI_BACK_GROUND_COLOR,	\
		.interlace		= CFG_DISP_PRI_MLC_INTERLACE,		\
	};

#define	INIT_PARAM_RGB(name)							\
	struct disp_rgb_param name = {							\
		.lcd_mpu_type 	= 0,                                \
	};

#define	INIT_PARAM_LVDS(name)							\
	struct disp_lvds_param name = {							\
		.lcd_format 	= CFG_DISP_LVDS_LCD_FORMAT,         \
	};

#define	INIT_PARAM_MIPI(name)	\
	struct disp_mipi_param name = {	\
		.pllpms 	= CFG_DISP_MIPI_PLLPMS,       \
		.bandctl	= CFG_DISP_MIPI_BANDCTL,      \
		.pllctl		= CFG_DISP_MIPI_PLLCTL,    	\
		.phyctl		= CFG_DISP_MIPI_DPHYCTL,      \
		.lcd_init	= MIPI_LCD_INIT	\
	};

static void mipilcd_dcs_write( unsigned int id, unsigned int data0, unsigned int data1 )
{
	U32 index = 0;
	volatile NX_MIPI_RegisterSet* pmipi = (volatile NX_MIPI_RegisterSet*)IO_ADDRESS(NX_MIPI_GetPhysicalAddress(index));

	pmipi->DSIM_PKTHDR = id | (data0<<8) | (data1<<16);
}

static void  mipilcd_dcs_long_write(U32 cmd, U32 ByteCount, U8* pByteData )
{
	U32 DataCount32 = (ByteCount+3)/4;
	int i = 0;
	U32 index = 0;
	volatile NX_MIPI_RegisterSet* pmipi = (volatile NX_MIPI_RegisterSet*)IO_ADDRESS(NX_MIPI_GetPhysicalAddress(index));

	for( i=0; i<DataCount32; i++ )
	{
		pmipi->DSIM_PAYLOAD = (pByteData[3]<<24)|(pByteData[2]<<16)|(pByteData[1]<<8)|pByteData[0];
		pByteData += 4;
	}
	pmipi->DSIM_PKTHDR  = (cmd & 0xff) | (ByteCount<<8);
}

static int MIPI_LCD_INIT(int width, int height, void *data)
{
	struct mipi_reg_val * next = CFG_DISP_MIPI_INIT_DATA;
	u32 index = 0;
	u32 value = 0;
	u8 pByteData[64];
	u8 bitrate=CFG_DISP_MIPI_BANDCTL;

	gpio_direction_output(PAD_GPIO_B + 8, 1);
	mdelay(20);
	gpio_direction_output(PAD_GPIO_B + 8, 0);
	mdelay(20);
	gpio_direction_output(PAD_GPIO_B + 8, 1);
	mdelay(120);

	volatile NX_MIPI_RegisterSet* pmipi = (volatile NX_MIPI_RegisterSet*)IO_ADDRESS(NX_MIPI_GetPhysicalAddress(index));
	value = pmipi->DSIM_ESCMODE;
	pmipi->DSIM_ESCMODE = value|(3 << 6);
	value = pmipi->DSIM_ESCMODE;
	printf("DSIM_ESCMODE 1 : 0x%x\n", value);
	value = pmipi->DSIM_STATUS;
	printf("DSIM_STATUS : 0x%x\n", value);
	
	switch(bitrate)
	{
		case 0xF:	printf("MIPI clk: 1000MHz \n");	break;
		case 0xE:	printf("MIPI clk:  900MHz \n");	break;
		case 0xD:	printf("MIPI clk:  840MHz \n");	break;
		case 0xC:	printf("MIPI clk:  760MHz \n");	break;
		case 0xB:	printf("MIPI clk:  660MHz \n");	break;
		case 0xA:	printf("MIPI clk:  600MHz \n");	break;
		case 0x9:	printf("MIPI clk:  540MHz \n");	break;
		case 0x8:	printf("MIPI clk:  480MHz \n");	break;
		case 0x7:	printf("MIPI clk:  420MHz \n");	break;
		case 0x6:	printf("MIPI clk:  330MHz \n");	break;
		case 0x5:	printf("MIPI clk:  300MHz \n");	break;
		case 0x4:	printf("MIPI clk:  210MHz \n");	break;
		case 0x3:	printf("MIPI clk:  180MHz \n");	break;
		case 0x2:	printf("MIPI clk:  150MHz \n");	break;
		case 0x1:	printf("MIPI clk:  100MHz \n");	break;
		case 0x0:	printf("MIPI clk:   80MHz \n");	break;
		default :	printf("MIPI clk:  unknown \n"); break;
	}

	mdelay(10);

	while(!((next->cmd == 0x00) && (next->addr == 0x00)))
	{
		switch(next->cmd)
		{
			case 0x05:
				mipilcd_dcs_write(next->cmd, next->data.data[0], 0x00);
				break;
			case 0x15:
				mipilcd_dcs_write(next->cmd, next->addr, next->data.data[0]);
				break;
			case 0x29:
 			case 0x39:
				pByteData[0] = next->addr;
				memcpy(&pByteData[1], &(next->data.data[0]), 64);
				mipilcd_dcs_long_write(next->cmd, next->cnt+1, &pByteData[0]);
				break;
			case 0xff:
				mdelay(next->addr);
				break;
		}

		next++;
	}

	value = pmipi->DSIM_ESCMODE;
	pmipi->DSIM_ESCMODE = value&(~(3 << 6));
	value = pmipi->DSIM_ESCMODE;
	printf("DSIM_ESCMODE 2 : 0x%x\n", value);
	value = pmipi->DSIM_STATUS;
	printf("DSIM_STATUS : 0x%x\n", value);
	return 0;
}

int bd_display(void)
{
	INIT_VIDEO_SYNC(vsync);
	INIT_PARAM_SYNCGEN(syncgen);
	INIT_PARAM_MULTILY(multily);
	INIT_PARAM_RGB(rgb);
	INIT_PARAM_LVDS(lvds);
	INIT_PARAM_MIPI(mipi);

	if(CFG_DISP_HDMI_USING == 0)
	{
		display_rgb(0, CONFIG_FB_ADDR, &vsync, &syncgen, &multily, &rgb);
		display_lvds(0, CONFIG_FB_ADDR, &vsync, &syncgen, &multily, &lvds);
		display_mipi(0, CONFIG_FB_ADDR, &vsync, &syncgen, &multily, &mipi);
	}
	else
	{
		if((CFG_DISP_PRI_RESOL_WIDTH == 1280) && (CFG_DISP_PRI_RESOL_HEIGHT == 720))
			display_hdmi(0, 0, CONFIG_FB_ADDR, &vsync, &syncgen, &multily);
		else if((CFG_DISP_PRI_RESOL_WIDTH == 1920) && (CFG_DISP_PRI_RESOL_HEIGHT == 1080))
			display_hdmi(0, 1, CONFIG_FB_ADDR, &vsync, &syncgen, &multily);
	}
	mdelay(50);
	return 0;
}

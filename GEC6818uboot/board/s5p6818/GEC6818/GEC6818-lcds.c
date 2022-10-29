#include <config.h>
#include <common.h>
#include <platform.h>
#include <asm/arch/display.h>

/* Default to lcd  at070tn92  */
int CFG_DISP_PRI_SCREEN_LAYER = 0;
int CFG_DISP_PRI_SCREEN_RGB_FORMAT = MLC_RGBFMT_X8R8G8B8;
int CFG_DISP_PRI_SCREEN_PIXEL_BYTE = 4;
int CFG_DISP_PRI_SCREEN_COLOR_KEY = 0x090909;
int CFG_DISP_PRI_VIDEO_PRIORITY = 2;
int CFG_DISP_PRI_BACK_GROUND_COLOR = 0x000000;
int CFG_DISP_PRI_MLC_INTERLACE = 0;

int	CFG_DISP_PRI_LCD_WIDTH_MM = 155;
int	CFG_DISP_PRI_LCD_HEIGHT_MM = 93;
int CFG_DISP_PRI_RESOL_WIDTH = 800;
int CFG_DISP_PRI_RESOL_HEIGHT = 480;
int CFG_DISP_PRI_HSYNC_SYNC_WIDTH = 10;
int CFG_DISP_PRI_HSYNC_BACK_PORCH = 36;
int CFG_DISP_PRI_HSYNC_FRONT_PORCH = 80;
int CFG_DISP_PRI_HSYNC_ACTIVE_HIGH = 1;
int CFG_DISP_PRI_VSYNC_SYNC_WIDTH = 8;
int CFG_DISP_PRI_VSYNC_BACK_PORCH = 15;
int CFG_DISP_PRI_VSYNC_FRONT_PORCH = 22;
int CFG_DISP_PRI_VSYNC_ACTIVE_HIGH = 1;

int CFG_DISP_PRI_CLKGEN0_SOURCE = DPC_VCLK_SRC_PLL2;
int CFG_DISP_PRI_CLKGEN0_DIV = 26;
int CFG_DISP_PRI_CLKGEN0_DELAY = 0;
int CFG_DISP_PRI_CLKGEN0_INVERT = 0;
int CFG_DISP_PRI_CLKGEN1_SOURCE = DPC_VCLK_SRC_VCLK2;
int CFG_DISP_PRI_CLKGEN1_DIV = 1;
int CFG_DISP_PRI_CLKGEN1_DELAY = 0;
int CFG_DISP_PRI_CLKGEN1_INVERT = 0;
int CFG_DISP_PRI_CLKSEL1_SELECT = 0;
int CFG_DISP_PRI_PADCLKSEL = DPC_PADCLKSEL_VCLK;
int	CFG_DISP_PRI_PIXEL_CLOCK = (780000000 / 26);
int	CFG_DISP_PRI_OUT_SWAPRB = 0;
int CFG_DISP_PRI_OUT_FORMAT = DPC_FORMAT_RGB888;
int CFG_DISP_PRI_OUT_YCORDER = DPC_YCORDER_CbYCrY;
int CFG_DISP_PRI_OUT_INTERLACE = 0;
int CFG_DISP_PRI_OUT_INVERT_FIELD = 0;
int CFG_DISP_LCD_MPY_TYPE = 0;
int CFG_DISP_LVDS_LCD_FORMAT = LVDS_LCDFORMAT_JEIDA;
int CFG_DISP_HDMI_USING = 0;

int CFG_DISP_MIPI_PLLPMS = 0x2281;
int CFG_DISP_MIPI_BANDCTL = 0x7;
int CFG_DISP_MIPI_PLLCTL = 0;
int CFG_DISP_MIPI_DPHYCTL = 0;
static struct mipi_reg_val mipidef[] = {
	{0,    0,     0, {0}},
};
struct mipi_reg_val * CFG_DISP_MIPI_INIT_DATA = &mipidef[0];

extern void core_rate_init(void);
static void GEC6818_set_pll2(unsigned int pdiv, unsigned int mdiv, unsigned int sdiv, unsigned int k)
{
	unsigned int val = (pdiv<<18 | mdiv<<8 | sdiv<<0);

	writel(k<<16, (0xC0010000 + 0x0050));
	writel(val, (0xC0010000 + 0x0010));
	writel(0x1<<2, (0xC0010000 + 0x0000));
	while((readl(0xC0010000 + 0x0000) & (0x1 << 31)));
	core_rate_init();
}

void GEC6818_lcd_select(void)
{
	char *commandline = getenv ("bootargs");
	if(!commandline)
		commandline = CONFIG_BOOTARGS;

	if(strstr(commandline, "lcd=vga-1024x768"))
	{
		/* PLL2 = 780MHZ */
		GEC6818_set_pll2(3, 195, 1, 0);

		CFG_DISP_PRI_RESOL_WIDTH = 1024;
		CFG_DISP_PRI_RESOL_HEIGHT = 768;

		CFG_DISP_PRI_HSYNC_SYNC_WIDTH = 136;
		CFG_DISP_PRI_HSYNC_BACK_PORCH = 160;
		CFG_DISP_PRI_HSYNC_FRONT_PORCH = 24;
		CFG_DISP_PRI_HSYNC_ACTIVE_HIGH = 0;
		CFG_DISP_PRI_VSYNC_SYNC_WIDTH = 6;
		CFG_DISP_PRI_VSYNC_BACK_PORCH = 29;
		CFG_DISP_PRI_VSYNC_FRONT_PORCH = 3;
		CFG_DISP_PRI_VSYNC_ACTIVE_HIGH = 0;

		CFG_DISP_PRI_CLKGEN0_DIV = 12;
		CFG_DISP_PRI_PIXEL_CLOCK = (780000000 / CFG_DISP_PRI_CLKGEN0_DIV);
	}
	else if(strstr(commandline, "lcd=vga-1440x900"))
	{
		/* PLL2 = 710MHZ */
		GEC6818_set_pll2(2, 118, 1, 21845);

		CFG_DISP_PRI_RESOL_WIDTH = 1440;
		CFG_DISP_PRI_RESOL_HEIGHT = 900;

		CFG_DISP_PRI_HSYNC_SYNC_WIDTH = 32;
		CFG_DISP_PRI_HSYNC_BACK_PORCH = 80;
		CFG_DISP_PRI_HSYNC_FRONT_PORCH = 48;
		CFG_DISP_PRI_HSYNC_ACTIVE_HIGH = 1;
		CFG_DISP_PRI_VSYNC_SYNC_WIDTH = 6;
		CFG_DISP_PRI_VSYNC_BACK_PORCH = 17;
		CFG_DISP_PRI_VSYNC_FRONT_PORCH = 3;
		CFG_DISP_PRI_VSYNC_ACTIVE_HIGH = 0;

		CFG_DISP_PRI_CLKGEN0_DIV = 8;
		CFG_DISP_PRI_PIXEL_CLOCK = (710000000 / CFG_DISP_PRI_CLKGEN0_DIV);
	}
	else if(strstr(commandline, "lcd=hdmi-720p"))
	{
		CFG_DISP_PRI_RESOL_WIDTH = 1280;
		CFG_DISP_PRI_RESOL_HEIGHT = 720;

		CFG_DISP_PRI_HSYNC_SYNC_WIDTH = 20;
		CFG_DISP_PRI_HSYNC_BACK_PORCH = 140;
		CFG_DISP_PRI_HSYNC_FRONT_PORCH = 160;
		CFG_DISP_PRI_HSYNC_ACTIVE_HIGH = 1;
		CFG_DISP_PRI_VSYNC_SYNC_WIDTH = 3;
		CFG_DISP_PRI_VSYNC_BACK_PORCH = 20;
		CFG_DISP_PRI_VSYNC_FRONT_PORCH = 12;
		CFG_DISP_PRI_VSYNC_ACTIVE_HIGH = 1;

		CFG_DISP_PRI_CLKGEN0_DIV = 5;
		CFG_DISP_PRI_PIXEL_CLOCK = (780000000 / CFG_DISP_PRI_CLKGEN0_DIV);
		CFG_DISP_HDMI_USING = 1;
	}
	else if(strstr(commandline, "lcd=hdmi-1080p"))
	{
		CFG_DISP_PRI_RESOL_WIDTH = 1920;
		CFG_DISP_PRI_RESOL_HEIGHT = 1080;

		CFG_DISP_PRI_HSYNC_SYNC_WIDTH = 20;
		CFG_DISP_PRI_HSYNC_BACK_PORCH = 140;
		CFG_DISP_PRI_HSYNC_FRONT_PORCH = 160;
		CFG_DISP_PRI_HSYNC_ACTIVE_HIGH = 1;
		CFG_DISP_PRI_VSYNC_SYNC_WIDTH = 3;
		CFG_DISP_PRI_VSYNC_BACK_PORCH = 20;
		CFG_DISP_PRI_VSYNC_FRONT_PORCH = 12;
		CFG_DISP_PRI_VSYNC_ACTIVE_HIGH = 1;

		CFG_DISP_PRI_CLKGEN0_DIV = 5;
		CFG_DISP_PRI_PIXEL_CLOCK = (780000000 / CFG_DISP_PRI_CLKGEN0_DIV);
		CFG_DISP_HDMI_USING = 1;
	}
	else if(strstr(commandline, "lcd=vs070cxn"))
	{
		CFG_DISP_PRI_RESOL_WIDTH = 1024;
		CFG_DISP_PRI_RESOL_HEIGHT = 600;

		CFG_DISP_PRI_HSYNC_SYNC_WIDTH = 20;
		CFG_DISP_PRI_HSYNC_BACK_PORCH = 140;
		CFG_DISP_PRI_HSYNC_FRONT_PORCH = 160;
		CFG_DISP_PRI_HSYNC_ACTIVE_HIGH = 1;
		CFG_DISP_PRI_VSYNC_SYNC_WIDTH = 3;
		CFG_DISP_PRI_VSYNC_BACK_PORCH = 20;
		CFG_DISP_PRI_VSYNC_FRONT_PORCH = 12;
		CFG_DISP_PRI_VSYNC_ACTIVE_HIGH = 1;

		CFG_DISP_PRI_CLKGEN0_DIV = 15;
		CFG_DISP_PRI_PIXEL_CLOCK = (710000000 / CFG_DISP_PRI_CLKGEN0_DIV); //65M
	}
	else if(strstr(commandline, "lcd=at070tn92"))
	{
		CFG_DISP_PRI_RESOL_WIDTH = 800;
		CFG_DISP_PRI_RESOL_HEIGHT = 480;

		CFG_DISP_PRI_HSYNC_SYNC_WIDTH = 10;
		CFG_DISP_PRI_HSYNC_BACK_PORCH = 36;
		CFG_DISP_PRI_HSYNC_FRONT_PORCH = 80;
		CFG_DISP_PRI_HSYNC_ACTIVE_HIGH = 1;
		CFG_DISP_PRI_VSYNC_SYNC_WIDTH = 8;
		CFG_DISP_PRI_VSYNC_BACK_PORCH = 15;
		CFG_DISP_PRI_VSYNC_FRONT_PORCH = 22;;
		CFG_DISP_PRI_VSYNC_ACTIVE_HIGH = 1;

		CFG_DISP_PRI_CLKGEN0_DIV = 26;
		CFG_DISP_PRI_PIXEL_CLOCK = (780000000 / CFG_DISP_PRI_CLKGEN0_DIV); //65M
	}
	else if(strstr(commandline, "lcd=b116xtn04"))
	{
		CFG_DISP_PRI_RESOL_WIDTH = 1366;
		CFG_DISP_PRI_RESOL_HEIGHT = 768;

		CFG_DISP_PRI_HSYNC_SYNC_WIDTH = 2;
		CFG_DISP_PRI_HSYNC_BACK_PORCH = 44;
		CFG_DISP_PRI_HSYNC_FRONT_PORCH = 44;
		CFG_DISP_PRI_HSYNC_ACTIVE_HIGH = 1;
		CFG_DISP_PRI_VSYNC_SYNC_WIDTH = 3;
		CFG_DISP_PRI_VSYNC_BACK_PORCH = 11;
		CFG_DISP_PRI_VSYNC_FRONT_PORCH = 11;
		CFG_DISP_PRI_VSYNC_ACTIVE_HIGH = 0;

		CFG_DISP_PRI_CLKGEN0_DIV = 12;
		CFG_DISP_PRI_PIXEL_CLOCK = (780000000 / CFG_DISP_PRI_CLKGEN0_DIV);
	}
	else if(strstr(commandline, "lcd=wy070ml"))
	{
		static struct mipi_reg_val cfg[] = {
			{0x05, 0x00,  1, {0x01}},
			{0xff, 30,    0, {0}},
			{0x15, 0x80,  1, {0x47}},
			{0x15, 0x81,  1, {0x40}},
			{0x15, 0x82,  1, {0x04}},
			{0x15, 0x83,  1, {0x77}},
			{0x15, 0x84,  1, {0x0f}},
			{0x15, 0x85,  1, {0x70}},
			{0x15, 0x86,  1, {0x70}},
			{0,    0,     0, {0}},
		};

		CFG_DISP_PRI_RESOL_WIDTH = 1024;
		CFG_DISP_PRI_RESOL_HEIGHT = 600;

		CFG_DISP_PRI_HSYNC_SYNC_WIDTH = 2;
		CFG_DISP_PRI_HSYNC_BACK_PORCH = 80;
		CFG_DISP_PRI_HSYNC_FRONT_PORCH = 168;
		CFG_DISP_PRI_HSYNC_ACTIVE_HIGH = 1;
		CFG_DISP_PRI_VSYNC_SYNC_WIDTH = 3;
		CFG_DISP_PRI_VSYNC_BACK_PORCH = 20;
		CFG_DISP_PRI_VSYNC_FRONT_PORCH = 12;
		CFG_DISP_PRI_VSYNC_ACTIVE_HIGH = 1;

		CFG_DISP_PRI_CLKGEN0_DIV = 12;
		CFG_DISP_PRI_PIXEL_CLOCK = (780000000 / CFG_DISP_PRI_CLKGEN0_DIV);

		CFG_DISP_MIPI_PLLPMS = 0x2231;
		CFG_DISP_MIPI_BANDCTL = 0x7;
		CFG_DISP_MIPI_PLLCTL = 0;
		CFG_DISP_MIPI_DPHYCTL = 0;
		CFG_DISP_MIPI_INIT_DATA = &cfg[0];
	}
	else if(strstr(commandline, "lcd=p101kda"))
	{
		static struct mipi_reg_val cfg[] = {
			{0,    0,     0, {0}},
		};

		CFG_DISP_PRI_RESOL_WIDTH = 1200;
		CFG_DISP_PRI_RESOL_HEIGHT = 1920;

		CFG_DISP_PRI_HSYNC_SYNC_WIDTH = 2;
		CFG_DISP_PRI_HSYNC_BACK_PORCH = 80;
		CFG_DISP_PRI_HSYNC_FRONT_PORCH = 168;
		CFG_DISP_PRI_HSYNC_ACTIVE_HIGH = 1;
		CFG_DISP_PRI_VSYNC_SYNC_WIDTH = 3;
		CFG_DISP_PRI_VSYNC_BACK_PORCH = 20;
		CFG_DISP_PRI_VSYNC_FRONT_PORCH = 12;
		CFG_DISP_PRI_VSYNC_ACTIVE_HIGH = 1;

		CFG_DISP_PRI_CLKGEN0_DIV = 5;
		CFG_DISP_PRI_PIXEL_CLOCK = (780000000 / CFG_DISP_PRI_CLKGEN0_DIV);

		CFG_DISP_MIPI_PLLPMS = 0x33E8;
		CFG_DISP_MIPI_BANDCTL = 0xF;
		CFG_DISP_MIPI_PLLCTL = 0;
		CFG_DISP_MIPI_DPHYCTL = 0;
		CFG_DISP_MIPI_INIT_DATA = &cfg[0];
	}
	else if(strstr(commandline, "lcd=ts8055pn"))
	{
		static struct mipi_reg_val cfg[] = {
			{0x39, 0xB9, 3, {0xFF, 0x83, 0x94}},
			{0x39, 0xBA, 6, {0x63, 0x03, 0x68, 0x6B, 0xB2, 0xC0}},
			{0x39, 0xB1, 10, {0x48, 0x12, 0x72, 0x09, 0x32, 0x44, 0x71, 0x31, 0x4F, 0x35}},
			{0x39, 0xB2, 5, {0x65, 0x80, 0x64, 0x05, 0x07}},
			{0x39, 0xB4, 30, {0x26, 0x76, 0x26, 0x76, 0x26, 0x26, 0x05, 0x10, 0x86, 0x35, 0x00, 0x3F, 0x26, 0x76, 0x26, 0x76, 0x26, 0x26, 0x05, 0x10, 0x86, 0x3F, 0x00, 0xFF, 0x81, 0x81, 0x81, 0x81, 0x08, 0x01}},
			{0x39, 0xD3, 33, {0x00, 0x00, 0x0F, 0x0F, 0x01, 0x01, 0x10, 0x10, 0x32, 0x10, 0x00, 0x00, 0x00, 0x32, 0x15, 0x04, 0x05, 0x04, 0x32, 0x15, 0x14, 0x05, 0x14, 0x37, 0x33, 0x04, 0x04, 0x37, 0x00, 0x00, 0x47, 0x05, 0x40}},
			{0x39, 0xD5, 44, {0x18, 0x18, 0x25, 0x24, 0x27, 0x26, 0x11, 0x10, 0x15, 0x14, 0x13, 0x12, 0x17, 0x16, 0x01, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x05, 0x04, 0x03, 0x02, 0x07, 0x06, 0x18, 0x18, 0x18, 0x18, 0x21, 0x20, 0x23, 0x22, 0x18, 0x18, 0x18, 0x18}},
			{0x39, 0xD6, 44, {0x18, 0x18, 0x22, 0x23, 0x20, 0x21, 0x12, 0x13, 0x16, 0x17, 0x10, 0x11, 0x14, 0x15, 0x06, 0x07, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x02, 0x03, 0x04, 0x05, 0x00, 0x01, 0x18, 0x18, 0x18, 0x18, 0x26, 0x27, 0x24, 0x25, 0x18, 0x18, 0x18, 0x18}},
			{0x39, 0xE0, 58, {0x00, 0x03, 0x0B, 0x0E, 0x10, 0x13, 0x17, 0x15, 0x2D, 0x3D, 0x51, 0x51, 0x5E, 0x75, 0x7C, 0x84, 0x94, 0x9A, 0x98, 0xA6, 0xB2, 0x57, 0x57, 0x5A, 0x60, 0x64, 0x6A, 0x72, 0x7F, 0x00, 0x03, 0x0B, 0x0E, 0x10, 0x13, 0x17, 0x15, 0x2D, 0x3D, 0x51, 0x51, 0x5E, 0x75, 0x7C, 0x84, 0x94, 0x9A, 0x98, 0xA6, 0xB2, 0x57, 0x57, 0x5A, 0x60, 0x64, 0x6A, 0x72, 0x7F}},
			{0x39, 0xB6, 2, {0x34, 0x34}},
			{0x39, 0xCC, 1, {0x0b}},
			{0x39, 0xC0, 2, {0x1F, 0x31}},
			{0x39, 0xD2, 1, {0x88}},
			{0x39, 0xD4, 1, {0x02}},
			{0x39, 0xBD, 1, {0x01}},
			{0x39, 0xB1, 1, {0x60}},
			{0x39, 0xBD, 1, {0x00}},
			{0x39, 0xBF, 7, {0x40, 0x81, 0x50, 0x00, 0x1A, 0xFC, 0x01}},
			{0x05, 0, 0, {0x11}},
			{0xff, 120, 0, {0}},
			{0x05, 0, 0, {0x29}},
			{0xff, 20, 0, {0}},
			{0, 0, 0, {0}},
		};

		CFG_DISP_PRI_RESOL_WIDTH = 720;
		CFG_DISP_PRI_RESOL_HEIGHT = 1280;

		CFG_DISP_PRI_HSYNC_SYNC_WIDTH = 2;
		CFG_DISP_PRI_HSYNC_BACK_PORCH = 28 + 80;
		CFG_DISP_PRI_HSYNC_FRONT_PORCH = 12 + 80;
		CFG_DISP_PRI_HSYNC_ACTIVE_HIGH = 1;
		CFG_DISP_PRI_VSYNC_SYNC_WIDTH = 2;
		CFG_DISP_PRI_VSYNC_BACK_PORCH = 5;
		CFG_DISP_PRI_VSYNC_FRONT_PORCH = 9;
		CFG_DISP_PRI_VSYNC_ACTIVE_HIGH = 1;

		CFG_DISP_PRI_CLKGEN0_DIV = 13;
		CFG_DISP_PRI_PIXEL_CLOCK = (780000000 / CFG_DISP_PRI_CLKGEN0_DIV);

		CFG_DISP_MIPI_PLLPMS = 0x2281;
		CFG_DISP_MIPI_BANDCTL = 0x8;
		CFG_DISP_MIPI_PLLCTL = 0;
		CFG_DISP_MIPI_DPHYCTL = 0;
		CFG_DISP_MIPI_INIT_DATA = &cfg[0];
	}
}


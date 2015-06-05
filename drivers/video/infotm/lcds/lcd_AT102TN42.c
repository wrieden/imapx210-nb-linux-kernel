/***************************************************************************** 
** drivers/video/infotm/imapfb_lcd.c
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Implementation file of Display Controller.
**
** Author:
**     Feng Jiaxing <jiaxing_feng@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0  09/14/2009  Feng Jiaxing
*****************************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/string.h>
#include <linux/ioctl.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/wait.h>

#include <asm/io.h>
#include <asm/uaccess.h>

#include "../imapfb.h"

/* configurations */
#define IMAPFB_HFP		96//134		/* front porch */
#define IMAPFB_HSW		24//32		/* hsync width */
#define IMAPFB_HBP		200//200//250		/* back porch */
#define IMAPFB_VFP		12//10		/* front porch */
#define IMAPFB_VSW		1//5//4		/* vsync width */
#define IMAPFB_VBP		23//18//14		/* back porch */
#define IMAPFB_HRES		1024	/* horizon pixel x resolition */
#define IMAPFB_VRES		600		/* line cnt y resolution */
#define IMAPFB_RGB		0x6//0x24		/* rgb sequence */
#define IMAPFB_PIXEL_CLOCK	18519 /* 1000,000,000,000 / vclk */
//#define IMAPFB_LCD_DPLL	0x80001027	/* 480 MHz */
#define IMAPFB_IDS_CFG	0x0e0e	/* this value will be put in DIV_CFG4 */
#define IMAPFB_CLK_DIV	1//0	/* this value will be put in DIV_CFG4 */


#define IMAPFB_VFRAME_FREQ	60		/* frame rate freq */
#define IMAPFB_HRES_OSD	IMAPFB_HRES		/* horizon pixel x resolition */
#define IMAPFB_VRES_OSD	IMAPFB_VRES		/* line cnt y resolution */
#define IMAPFB_HRES_OSD_VIRTUAL	IMAPFB_HRES	/* horizon pixel x resolition */
#define IMAPFB_VRES_OSD_VIRTUAL	IMAPFB_VRES		/* line cnt y resolution */

imapfb_fimd_info_t imapfb_fimd = {
	.lcdcon1 = IMAP_LCDCON1_PNRMODE_TFTLCD | IMAP_LCDCON1_ENVID_DISABLE |
		IMAP_LCDCON1_CLKVAL(IMAPFB_CLK_DIV),
	.lcdcon2 = IMAP_LCDCON2_VBPD(IMAPFB_VBP - 1) | IMAP_LCDCON2_LINEVAL(IMAPFB_VRES - 1) | IMAP_LCDCON2_VFPD(IMAPFB_VFP - 1) | IMAP_LCDCON2_VSPW(IMAPFB_VSW - 1),
	.lcdcon3 = IMAP_LCDCON3_HBPD(IMAPFB_HBP - 1) | IMAP_LCDCON3_HOZVAL(IMAPFB_HRES - 1) | IMAP_LCDCON3_HFPD(IMAPFB_HFP - 1),
	.lcdcon4 = IMAP_LCDCON4_HSPW(IMAPFB_HSW - 1),
	.lcdcon5 = (IMAPFB_RGB << 24) | IMAP_LCDCON5_INVVCLK_FALLING_EDGE | IMAP_LCDCON5_INVVLINE_INVERTED | IMAP_LCDCON5_INVVFRAME_INVERTED | IMAP_LCDCON5_INVVD_NORMAL
		| IMAP_LCDCON5_INVVDEN_NORMAL | IMAP_LCDCON5_INVPWREN_NORMAL | IMAP_LCDCON5_PWREN_ENABLE,

	.ovcdcr = IMAP_OVCDCR_IFTYPE_RGB,

#if defined (CONFIG_FB_IMAP_BPP8)
	.ovcw0cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE
		| IMAP_OVCWxCR_BPPMODE_8BPP_ARGB232 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw1cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_8BPP_ARGB232 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw2cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_8BPP_ARGB232 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw3cr = IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE | IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE
		| IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 | IMAP_OVCWxCR_BLD_PIX_PLANE
		| IMAP_OVCWxCR_BPPMODE_8BPP_ARGB232 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.bpp = 8,
	.bytes_per_pixel = 1,
	
#elif defined (CONFIG_FB_IMAP_BPP16)
	.ovcw0cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE
		| IMAP_OVCWxCR_BPPMODE_16BPP_RGB565 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw1cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_16BPP_RGB565 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw2cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_16BPP_RGB565 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw3cr = IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE | IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE
		| IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 | IMAP_OVCWxCR_BLD_PIX_PLANE
		| IMAP_OVCWxCR_BPPMODE_16BPP_RGB565 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.bpp = 16,
	.bytes_per_pixel = 2,
	
#elif defined (CONFIG_FB_IMAP_BPP18)
	.ovcw0cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE
		| IMAP_OVCWxCR_BPPMODE_18BPP_RGB666 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw1cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_18BPP_RGB666 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw2cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_18BPP_RGB666 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw3cr = IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE | IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE
		| IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 | IMAP_OVCWxCR_BLD_PIX_PLANE
		| IMAP_OVCWxCR_BPPMODE_18BPP_RGB666 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.bpp = 18,
	.bytes_per_pixel = 4,

#elif defined (CONFIG_FB_IMAP_BPP19)
	.ovcw0cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE
		| IMAP_OVCWxCR_BPPMODE_19BPP_ARGB666 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw1cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_19BPP_ARGB666 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw2cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_19BPP_ARGB666 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw3cr = IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE | IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE
		| IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 | IMAP_OVCWxCR_BLD_PIX_PLANE
		| IMAP_OVCWxCR_BPPMODE_19BPP_ARGB666 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.bpp = 19,
	.bytes_per_pixel = 4,

#elif defined (CONFIG_FB_IMAP_BPP24)
	.ovcw0cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE
		| IMAP_OVCWxCR_BPPMODE_24BPP_RGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw1cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_24BPP_RGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw2cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_24BPP_RGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw3cr = IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE | IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE
		| IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 | IMAP_OVCWxCR_BLD_PIX_PLANE
		| IMAP_OVCWxCR_BPPMODE_24BPP_RGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.bpp = 24,
	.bytes_per_pixel = 4,

#elif defined (CONFIG_FB_IMAP_BPP25)
	.ovcw0cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE
		| IMAP_OVCWxCR_BPPMODE_25BPP_ARGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw1cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_25BPP_ARGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw2cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_25BPP_ARGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw3cr = IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE | IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE
		| IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 | IMAP_OVCWxCR_BLD_PIX_PLANE
		| IMAP_OVCWxCR_BPPMODE_25BPP_ARGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.bpp = 25,
	.bytes_per_pixel = 4,

#elif defined (CONFIG_FB_IMAP_BPP28)
	.ovcw0cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE
		| IMAP_OVCWxCR_BPPMODE_28BPP_A4RGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw1cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_28BPP_A4RGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw2cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_28BPP_A4RGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw3cr = IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE | IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE
		| IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 | IMAP_OVCWxCR_BLD_PIX_PLANE
		| IMAP_OVCWxCR_BPPMODE_28BPP_A4RGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.bpp = 28,
	.bytes_per_pixel = 4,

#elif defined (CONFIG_FB_IMAP_BPP32)
	.ovcw0cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE
		| IMAP_OVCWxCR_BPPMODE_32BPP_A8RGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw1cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_32BPP_A8RGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw2cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_32BPP_A8RGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw3cr = IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE | IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE
		| IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 | IMAP_OVCWxCR_BLD_PIX_PLANE
		| IMAP_OVCWxCR_BPPMODE_32BPP_A8RGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.bpp = 32,
	.bytes_per_pixel = 4,

#endif

	.ovcw0pcar = IMAP_OVCWxPCAR_LEFTTOPX(0) | IMAP_OVCWxPCAR_LEFTTOPY(0),
	.ovcw0pcbr = IMAP_OVCWxPCBR_RIGHTBOTX(IMAPFB_HRES_OSD - 1) | IMAP_OVCWxPCBR_RIGHTBOTY(IMAPFB_VRES_OSD - 1),
	.ovcw0cmr = IMAP_OVCWxCMR_MAPCOLEN_DISABLE,

#if (CONFIG_FB_IMAP_NUM > 1)
	.ovcw1pcar = IMAP_OVCWxPCAR_LEFTTOPX(0) | IMAP_OVCWxPCAR_LEFTTOPY(0),
	.ovcw1pcbr = IMAP_OVCWxPCBR_RIGHTBOTX(IMAPFB_HRES_OSD - 1) | IMAP_OVCWxPCBR_RIGHTBOTY(IMAPFB_VRES_OSD - 1),
	.ovcw1pccr = IMAP_OVCWxPCCR_ALPHA0_R(IMAPFB_MAX_ALPHA_LEVEL) | IMAP_OVCWxPCCR_ALPHA0_G(IMAPFB_MAX_ALPHA_LEVEL)
		| IMAP_OVCWxPCCR_ALPHA0_B(IMAPFB_MAX_ALPHA_LEVEL) | IMAP_OVCWxPCCR_ALPHA1_R(IMAPFB_MAX_ALPHA_LEVEL)
		| IMAP_OVCWxPCCR_ALPHA1_G(IMAPFB_MAX_ALPHA_LEVEL) | IMAP_OVCWxPCCR_ALPHA1_B(IMAPFB_MAX_ALPHA_LEVEL),
	.ovcw1cmr = IMAP_OVCWxCMR_MAPCOLEN_DISABLE,
#endif

#if (CONFIG_FB_IMAP_NUM > 2)	
	.ovcw2pcar = IMAP_OVCWxPCAR_LEFTTOPX(0) | IMAP_OVCWxPCAR_LEFTTOPY(0),
	.ovcw2pcbr = IMAP_OVCWxPCBR_RIGHTBOTX(IMAPFB_HRES_OSD - 1) | IMAP_OVCWxPCBR_RIGHTBOTY(IMAPFB_VRES_OSD - 1),
	.ovcw2pccr = IMAP_OVCWxPCCR_ALPHA0_R(IMAPFB_MAX_ALPHA_LEVEL) | IMAP_OVCWxPCCR_ALPHA0_G(IMAPFB_MAX_ALPHA_LEVEL)
		| IMAP_OVCWxPCCR_ALPHA0_B(IMAPFB_MAX_ALPHA_LEVEL) | IMAP_OVCWxPCCR_ALPHA1_R(IMAPFB_MAX_ALPHA_LEVEL)
		| IMAP_OVCWxPCCR_ALPHA1_G(IMAPFB_MAX_ALPHA_LEVEL) | IMAP_OVCWxPCCR_ALPHA1_B(IMAPFB_MAX_ALPHA_LEVEL),
	.ovcw2cmr = IMAP_OVCWxCMR_MAPCOLEN_DISABLE,
#endif

#if (CONFIG_FB_IMAP_NUM > 3)
	.ovcw3pcar = IMAP_OVCWxPCAR_LEFTTOPX(0) | IMAP_OVCWxPCAR_LEFTTOPY(0),
	.ovcw3pcbr = IMAP_OVCWxPCBR_RIGHTBOTX(IMAPFB_HRES_OSD - 1) | IMAP_OVCWxPCBR_RIGHTBOTY(IMAPFB_VRES_OSD - 1),
	.ovcw3pccr = IMAP_OVCWxPCCR_ALPHA0_R(IMAPFB_MAX_ALPHA_LEVEL) | IMAP_OVCWxPCCR_ALPHA0_G(IMAPFB_MAX_ALPHA_LEVEL)
		| IMAP_OVCWxPCCR_ALPHA0_B(IMAPFB_MAX_ALPHA_LEVEL) | IMAP_OVCWxPCCR_ALPHA1_R(IMAPFB_MAX_ALPHA_LEVEL)
		| IMAP_OVCWxPCCR_ALPHA1_G(IMAPFB_MAX_ALPHA_LEVEL) | IMAP_OVCWxPCCR_ALPHA1_B(IMAPFB_MAX_ALPHA_LEVEL),
	.ovcw3cmr = IMAP_OVCWxCMR_MAPCOLEN_DISABLE,
#endif	

	.sync = 0,
	.cmap_static = 1,

	.xres = IMAPFB_HRES,
	.yres = IMAPFB_VRES,

	.osd_xres = IMAPFB_HRES_OSD,
	.osd_yres = IMAPFB_VRES_OSD,

	.osd_xres_virtual = IMAPFB_HRES_OSD_VIRTUAL,
	.osd_yres_virtual = IMAPFB_VRES_OSD_VIRTUAL,

	.osd_xoffset = 0,
	.osd_yoffset = 0,

	.pixclock = IMAPFB_PIXEL_CLOCK,

	.hsync_len = IMAPFB_HSW,
	.vsync_len = IMAPFB_VSW,
	.left_margin = IMAPFB_HBP,
	.upper_margin = IMAPFB_VBP,
	.right_margin = IMAPFB_HFP,
	.lower_margin = IMAPFB_VFP,
	.set_lcd_power = NULL,
	.set_backlight_power = NULL,
	.set_brightness = NULL,
};

void imapfb_set_clk(void)
{
#ifdef  IMAPFB_LCD_DPLL
	writel(readl(rDPLL_CFG) & ~(1 << 31), rDPLL_CFG);
	writel(IMAPFB_LCD_DPLL, rDPLL_CFG);
	writel(readl(rDPLL_CFG) | (1 << 31), rDPLL_CFG);

	/*wait untill dpll is locked*/
	while(!(readl(rPLL_LOCKED) & 0x2));
#endif
	writel(IMAPFB_IDS_CFG, rDIV_CFG4);
}

EXPORT_SYMBOL(imapfb_set_clk);

/********************************************************************************
** linux-2.6.28.5/arch/arm/plat-imap/devs.c
**
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
**
** Use of Infotm's code is governed by terms and conditions
** stated in the accompanying licensing statement.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** Author:
**     Raymond Wang   <raymond.wang@infotmic.com.cn>
**
** Revision History:
**     1.2  25/11/2009    Raymond Wang
********************************************************************************/
 
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <linux/input.h>
#include <linux/gpio_keys.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>
#include <mach/hardware.h>
#include <asm/irq.h>
#include <plat/sdhci.h>

#include <plat/devs.h>
#include <plat/cpu.h>
#include <plat/imapx.h>
#include <linux/amba/bus.h>
#include <linux/ata_platform.h>

#include <mach/spi.h>
#include <mach/gpio.h>

#include <linux/android_pmem.h>
#include <linux/switch.h>

#ifdef CONFIG_USB_ANDROID
#include <linux/usb/android_composite.h>
#endif


void __init imapx200_register_device(struct platform_device *dev, void *data)
{
	int ret;

	dev->dev.platform_data = data;

	ret = platform_device_register(dev);
	if (ret)
		dev_err(&dev->dev, "unable to register device: %d\n", ret);
}


/* Serial port registrations */
static struct resource imapx200_uart0_resource[] = {
	[0] = {
		.start	= UART0_BASE_ADDR,
		.end   	= UART0_BASE_ADDR + 0xFFF,
		.flags 	= IORESOURCE_MEM,
	},
	[1] = {
		.start 	= IRQ_UART0,
		.end   	= IRQ_UART0,
		.flags 	= IORESOURCE_IRQ,
	}
};

static struct resource imapx200_uart1_resource[] = {
	[0] = {
		.start	= UART1_BASE_ADDR,
		.end   	= UART1_BASE_ADDR + 0xFFF,
		.flags 	= IORESOURCE_MEM,
	},
	[1] = {
		.start 	= IRQ_UART1,
		.end   	= IRQ_UART1,
		.flags 	= IORESOURCE_IRQ,
	}
};

static struct resource imapx200_uart2_resource[] = {
	[0] = {
		.start	= UART2_BASE_ADDR,
		.end   	= UART2_BASE_ADDR + 0xFFF,
		.flags 	= IORESOURCE_MEM,
	},
	[1] = {
		.start 	= IRQ_UART2,
		.end   	= IRQ_UART2,
		.flags 	= IORESOURCE_IRQ,
	}
};

static struct resource imapx200_uart3_resource[] = {
	[0] = {
		.start	= UART3_BASE_ADDR,
		.end   	= UART3_BASE_ADDR + 0xFFF,
		.flags 	= IORESOURCE_MEM,
	},
	[1] = {
		.start 	= IRQ_UART3,
		.end   	= IRQ_UART3,
		.flags 	= IORESOURCE_IRQ,
	}
};

struct imap_uart_resources imapx200_uart_resources[] __initdata = {
	[0] = {
		.resources	= imapx200_uart0_resource,
		.nr_resources	= ARRAY_SIZE(imapx200_uart0_resource),
	},
	[1] = {
		.resources	= imapx200_uart1_resource,
		.nr_resources	= ARRAY_SIZE(imapx200_uart1_resource),
	},
	[2] = {
		.resources	= imapx200_uart2_resource,
		.nr_resources	= ARRAY_SIZE(imapx200_uart2_resource),
	},
	[3] = {
		.resources	= imapx200_uart3_resource,
		.nr_resources	= ARRAY_SIZE(imapx200_uart3_resource),
	}
};

/* yart devices */

static struct platform_device imap_uart_device0 = {
	.id		= 0,
};

static struct platform_device imap_uart_device1 = {
	.id		= 1,
};

static struct platform_device imap_uart_device2 = {
	.id		= 2,
};

static struct platform_device imap_uart_device3 = {
	.id		= 3,
};

struct platform_device *imap_uart_src[4] = {
	&imap_uart_device0,
	&imap_uart_device1,
	&imap_uart_device2,
	&imap_uart_device3,
};

struct platform_device *imap_uart_devs[4] = {
};

/****Nand Controller****/
static struct resource imapx200_nand_resource[]={
	[0] = {
		.start = NAND_BASE_REG_PA,
		.end   = NAND_BASE_REG_PA + 0xfff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_NFCON,
		.end	= IRQ_NFCON,
		.flags	= IORESOURCE_IRQ,
	}
};

struct platform_device imapx200_device_nand = {
	.name		= "imapx200_nand",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imapx200_nand_resource),
	.resource	= imapx200_nand_resource,
};

EXPORT_SYMBOL(imapx200_device_nand);

static struct gpio_keys_button imapx200_buttons[] = {

	{
		.gpio		= IRQ_PowerMode,	/* INT30 */
		.code		= KEY_POWER,
		.desc		= "Power",
		.active_low	= 0,
	},
};

static struct gpio_keys_platform_data imapx200_button_data = {
	.buttons	= imapx200_buttons,
	.nbuttons	= ARRAY_SIZE(imapx200_buttons),
};

struct platform_device imapx200_button_device  = {
	.name		= "gpio-keys",
	.id		= -1,
	.dev		= {
		.platform_data	= &imapx200_button_data,
	}
};
EXPORT_SYMBOL(imapx200_button_device);
/****SDIO*****/
char *imapx_sdi0_clocks[2] =
 {
        [0] = "sdio0",
        [1] = "sdio0-ext",
 };

char *imapx_sdi1_clocks[2] =
 {
        [0] = "sdio1",
        [1] = "sdio1-ext",
 };

char *imapx_sdi2_clocks[2] =
 {
        [0] = "sdio2",
        [1] = "sdio2-ext",
 };

struct imapx_sdi_platdata imapx200_sdi0_platdata = {
        .hw_port        = 0,
        .width          = 4,
        .caps   = ( MMC_CAP_4_BIT_DATA |MMC_CAP_MMC_HIGHSPEED
                          | MMC_CAP_SD_HIGHSPEED),
        .clocks         =  imapx_sdi0_clocks,
};

static struct resource imapx200_sdi0_resource[]={
	[0] = {
		.start = SD0_BASE_REG_PA,
		.end   = SD0_BASE_REG_PA + 0xfff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_SDIO0,
		.end	= IRQ_SDIO0,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imapx200_device_sdi0 = {
	.name		= "imapx200_sdi0",
	.id 		= 0,
	.num_resources	= ARRAY_SIZE(imapx200_sdi0_resource),
	.resource	= imapx200_sdi0_resource,
	.dev		= {
		.platform_data	=&imapx200_sdi0_platdata,
	},

};

EXPORT_SYMBOL(imapx200_device_sdi0);

static struct resource imapx200_sdi1_resource[]={
	[0] = {
		.start = SD1_BASE_REG_PA,
		.end   = SD1_BASE_REG_PA + 0xfff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_SDIO1,
		.end	= IRQ_SDIO1,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct  imapx_sdi_platdata  imapx200_sdi1_platdata = {
        .hw_port        = 1,
        .width          = 4,
        .caps   = ( MMC_CAP_4_BIT_DATA |MMC_CAP_MMC_HIGHSPEED
                          | MMC_CAP_SD_HIGHSPEED),
        .clocks         = imapx_sdi1_clocks,
};


struct platform_device imapx200_device_sdi1 = {
        .name           = "imapx200_sdi1",
        .id             = 1,
        .num_resources  = ARRAY_SIZE(imapx200_sdi1_resource),
        .resource       = imapx200_sdi1_resource,
        .dev            = {
                .platform_data          = &imapx200_sdi1_platdata,
        },
};



EXPORT_SYMBOL(imapx200_device_sdi1);

static struct resource imapx200_sdi2_resource[]={
	[0] = {
		.start = SD2_BASE_REG_PA,
		.end   = SD2_BASE_REG_PA + 0xfff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_SDIO2,
		.end	= IRQ_SDIO2,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct  imapx_sdi_platdata  imapx200_sdi2_platdata = {
	.hw_port        = 2,
	.width          = 4,
	.caps   = ( MMC_CAP_4_BIT_DATA | MMC_CAP_MMC_HIGHSPEED
			| MMC_CAP_SD_HIGHSPEED),
	.clocks         = imapx_sdi2_clocks,
};

struct platform_device imapx200_device_sdi2 = {
	.name		= "imapx200_sdi2",
	.id 		= 2,
	.num_resources	= ARRAY_SIZE(imapx200_sdi2_resource),
	.resource	= imapx200_sdi2_resource,
	.dev		= {
		.platform_data 		= &imapx200_sdi2_platdata,
	},
};

EXPORT_SYMBOL(imapx200_device_sdi2);

/*********CF************/
static struct resource imapx200_cf_resource[]={
        [0] = {
                .start = CF_BASE_REG_PA,
                .end   = CF_BASE_REG_PA + 0xfff,
                .flags = IORESOURCE_MEM,
        },
        [1] = {
                .start  = IRQ_IDE_CF,
                .end    = IRQ_IDE_CF,
                .flags  = IORESOURCE_IRQ,
        }
};

static u64 ide_dma_mask = DMA_BIT_MASK(32);

struct platform_device imapx200_device_cf = {
        .name           = "imapx200_cf",
        .id             = -1,
        .num_resources  = ARRAY_SIZE(imapx200_cf_resource),
        .resource       = imapx200_cf_resource,
        .dev = {
                .dma_mask = &ide_dma_mask,
                .coherent_dma_mask = DMA_BIT_MASK(32),
        }
};

EXPORT_SYMBOL(imapx200_device_cf);

static u64 usb_dma_mask=0xffffffffUL;
/*****USBHOST1.1******/
static struct resource imapx200_usbhost11_resource[]={
	[0] = {
		.start = USBH11_BASE_REG_PA,
		.end   = USBH11_BASE_REG_PA + 0xfff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_USBOHCI,
		.end	= IRQ_USBOHCI,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imapx200_device_usbhost11 = {
	.name		= "imapx200_usbhost11",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imapx200_usbhost11_resource),
	.resource	= imapx200_usbhost11_resource,
	.dev		= {
		.dma_mask = &usb_dma_mask,
		.coherent_dma_mask = 0xffffffffUL,
	}
};

EXPORT_SYMBOL(imapx200_device_usbhost11);

/*****USBHOST2.0*****/
static struct resource imapx200_usbhost20_resource[]={
	[0] = {
		.start = USBH20_BASE_REG_PA,
		.end   = USBH20_BASE_REG_PA + 0xfff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_USBEHCI,
		.end	= IRQ_USBEHCI,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imapx200_device_usbhost20 = {
	.name		= "imapx200_usbhost20",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imapx200_usbhost20_resource),
	.resource	= imapx200_usbhost20_resource,
	.dev		= {
		.dma_mask = &usb_dma_mask,
		.coherent_dma_mask = 0xffffffffUL,
	}

};

EXPORT_SYMBOL(imapx200_device_usbhost20);

/********USBOTG********/
static struct resource imapx200_usbotg_resource[]={
	[0] = {
		.start = USBOTG_BASE_REG_PA,
		.end   = USBOTG_BASE_REG_PA + 0xfff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_USBOTG,
		.end	= IRQ_USBOTG,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imapx200_device_usbotg = {
	.name		= "imapx200_usbotg",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imapx200_usbotg_resource),
	.resource	= imapx200_usbotg_resource,
};

EXPORT_SYMBOL(imapx200_device_usbotg);

/*******USBDEVICE********/
static struct resource imapx200_udc_resource[] = {
	[0] = {
		.start	= USBOTG_BASE_REG_PA,
		.end	= USBOTG_BASE_REG_PA + 0xffff,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_USBOTG,
		.end	= IRQ_USBOTG,
		.flags	= IORESOURCE_IRQ,
	}
};

struct platform_device imapx200_device_udc = {
	.name       = "ix-udc",
	.id			= -1,
	.num_resources	= ARRAY_SIZE(imapx200_udc_resource),
	.resource		= imapx200_udc_resource,
};

EXPORT_SYMBOL(imapx200_device_udc);

#ifdef CONFIG_USB_ANDROID
/*********** USB OTG MASS FOR ANDROID *********************/
static char *usb_functions_adb[] = {"adb"};
static char *usb_functions_mass[] = {"usb_mass_storage"};
static char *usb_functions[] = {
#ifdef CONFIG_USB_ANDROID_ADB
	"adb",
#endif
#ifdef CONFIG_USB_ANDROID_MASS_STORAGE
	"usb_mass_storage",
#endif
};

static struct android_usb_product usb_products[] = {
#ifdef CONFIG_USB_ANDROID_ADB
	{
		.product_id		= 0x1fd7,
		.num_functions	= ARRAY_SIZE(usb_functions_adb),
		.functions		= usb_functions_adb,
	},
#endif
#ifdef CONFIG_USB_ANDROID_MASS_STORAGE
	{
		.product_id		= 0x1fd9,
		.num_functions	= ARRAY_SIZE(usb_functions_mass),
		.functions		= usb_functions_mass,
	},
#endif
};

static struct android_usb_platform_data andusb_plat = {
	.vendor_id		= 0x18d1,
	.product_id		= 0x1f00,
	.manufacturer_name	= "InfoTMIC",
	.product_name		= "MID",
	.serial_number		= "InfoTMIC-MID",
	.num_products		= ARRAY_SIZE(usb_products),
	.products			= usb_products,
	.num_functions		= ARRAY_SIZE(usb_functions),
	.functions			= usb_functions,
};

struct platform_device android_usb_device = {
	.name       = "android_usb",
	.id			= -1,
	.dev		= {
		.platform_data = &andusb_plat,
	},
};

EXPORT_SYMBOL(android_usb_device);
#endif
/***********************************************************/

static struct resource imapx200_sensor_resource[]={

};


struct platform_device imapx200_device_orientation = {
	.name		= "imapx200-sensor-orientation",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imapx200_sensor_resource),
	.resource	= imapx200_sensor_resource,
};

EXPORT_SYMBOL(imapx200_device_orientation);

static struct resource imapx200_lights_resource[]={

};


struct platform_device imapx200_device_backlights = {
	.name		= "imapx200-lcd-backlights",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imapx200_lights_resource),
	.resource	= imapx200_lights_resource,
};

EXPORT_SYMBOL(imapx200_device_backlights);


/*********CAMERA**********/
static struct resource imapx200_camera_resource[]={
	[0] = {
		.start = CAMERA_BASE_REG_PA,
		.end   = CAMERA_BASE_REG_PA + 0xfff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_CAM,
		.end	= IRQ_CAM,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imapx200_device_camera = {
	.name		= "imapx200_camif",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imapx200_camera_resource),
	.resource	= imapx200_camera_resource,
};

EXPORT_SYMBOL(imapx200_device_camera);

/*******IDS*********/
static struct resource imapx200_lcd_resource[] = {
	[0] = {
		.start = LCD_BASE_REG_PA,
		.end   = LCD_BASE_REG_PA + 0x4000,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_IDS,
		.end	= IRQ_IDS,
		.flags = IORESOURCE_IRQ,  
	}
};

struct platform_device imapx200_device_lcd = {
	.name		= "imap-fb",
	.id 		= 0,
	.num_resources	= ARRAY_SIZE(imapx200_lcd_resource),
	.resource	= imapx200_lcd_resource,
};

EXPORT_SYMBOL(imapx200_device_lcd);

#ifdef CONFIG_RGB2VGA_OUTPUT_SUPPORT
struct platform_device imapx200_device_rgb2vga = {
    .name	= "imap-vga",
    .id	= 0,
};

EXPORT_SYMBOL(imapx200_device_rgb2vga);
#endif

#ifdef CONFIG_BOSCH_BMA150
struct platform_device imapx200_device_BMA150= {
    .name	= "imapx200_bma150",
    .id	= 0,
};

EXPORT_SYMBOL(imapx200_device_BMA150);
#endif

#ifdef CONFIG_HDMI_EP932
struct platform_device imapx200_device_hdmi_ep932= {
    .name	= "imap-HDMI",
    .id	= 0,
};

EXPORT_SYMBOL(imapx200_device_hdmi_ep932);
#endif

#ifdef CONFIG_HDMI_SI9022
struct platform_device imapx200_device_hdmi_si9022= {
    .name	= "imap-HDMI",
    .id	= 0,
};

EXPORT_SYMBOL(imapx200_device_hdmi_si9022);
#endif

/*********MAC*************/
static struct resource imapx200_mac_resource[]={
	[0] = {
		.start = MAC_BASE_REG_PA,
		.end   = MAC_BASE_REG_PA + 0x2000,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_Ethernet,
		.end	= IRQ_Ethernet,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imapx200_device_mac = {
	.name		= "imapx200_mac",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imapx200_mac_resource),
	.resource	= imapx200_mac_resource,
	.dev            = {
		.dma_mask = &imapx200_device_mac.dev.coherent_dma_mask,
		.coherent_dma_mask = 0xffffffffUL,
	}


};

EXPORT_SYMBOL(imapx200_device_mac);

struct platform_device imapx200_device_rda5868 = {
	.name		= "rda5868-bt",
	.id			= -1,
};

EXPORT_SYMBOL(imapx200_device_rda5868);

/**************Video Encoder****************/
static struct resource imapx200_venc_resource[]={
	[0] = {
		.start = VENC_BASE_REG_PA,
		.end   = VENC_BASE_REG_PA + 0xfff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_VENC,
		.end	= IRQ_VENC,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imapx200_device_venc = {
	.name		= "imapx200_venc",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imapx200_venc_resource),
	.resource	= imapx200_venc_resource,
};

EXPORT_SYMBOL(imapx200_device_venc);

/**************Video Decoder*************/
static struct resource imapx200_vdec_resource[]={
	[0] = {
		.start = VDEC_BASE_REG_PA,
		.end   = VDEC_BASE_REG_PA + 0xfff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_VDEC,
		.end	= IRQ_VDEC,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imapx200_device_vdec = {
	.name		= "imapx200_vdec",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imapx200_vdec_resource),
	.resource	= imapx200_vdec_resource,
};

EXPORT_SYMBOL(imapx200_device_vdec);

/* Memalloc */
/*
 *  * This platform is so unique cuz it's not a 
 *   * hardware device. We just need to do allocation
 *    * to get continuous physics memory.
 *     */
/***************Memory Alloc******************/
static struct resource imapx200_memalloc_resource[]={
	[0] = {
		.start = 0,
		.end   = 0,
		.flags = IORESOURCE_MEM,
	},
};

struct platform_device imapx200_device_memalloc = 
{
	.name		= "memalloc",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(imapx200_memalloc_resource),
	.resource	= imapx200_memalloc_resource,
};

EXPORT_SYMBOL(imapx200_device_memalloc);

/*******************GPS***********************/
static struct resource imapx200_gps_resource[]={
	[0] = {
		.start = 0,
		.end   = 0,
		.flags = IORESOURCE_MEM,
	},
};

static u64 gps_dma_mask = DMA_BIT_MASK(32);

struct platform_device imapx200_device_gps = 
{
	.name		= "imap_gps",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(imapx200_gps_resource),
	.resource	= imapx200_gps_resource,
	.dev = {
		.dma_mask = &gps_dma_mask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
	}
};

EXPORT_SYMBOL(imapx200_device_gps);

/*******************DSP***********************/
static struct resource imapx200_dsp_resource[]={
	[0] = {
		.start	= iDSP_REG_BASE,
		.end	= iDSP_REG_BASE + 0xfff,
		.flags	= IORESOURCE_MEM,  
	},
	[1] = {
		.start	= IRQ_DSPIPC,
		.end	= IRQ_DSPIPC,
		.flags	= IORESOURCE_IRQ,  
	},
};

static u64 dsp_dma_mask = DMA_BIT_MASK(32);

struct platform_device imapx200_device_dsp = {
	.name		= "imap_dsp",
	.id			= -1,
	.num_resources	= ARRAY_SIZE(imapx200_dsp_resource),
	.resource		= imapx200_dsp_resource,
	.dev = {
		.dma_mask = &dsp_dma_mask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
	}
};

EXPORT_SYMBOL(imapx200_device_dsp);

/*********PWM**************/
static struct resource imapx200_pwm_resource[]={
	[0] = {
		.start = PWM_BASE_REG_PA,
		.end   = PWM_BASE_REG_PA + 0xfff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_PWM0,
		.end	= IRQ_PWM0,
		.flags	= IORESOURCE_IRQ,  
	},
	[2] = {
		.start	= IRQ_PWM1,
		.end	= IRQ_PWM1,
		.flags	= IORESOURCE_IRQ,  
	},
	[3] = {
		.start	= IRQ_PWM2,
		.end	= IRQ_PWM2,
		.flags	= IORESOURCE_IRQ,  
	},
	[4] = {
		.start	= IRQ_PWM4,
		.end	= IRQ_PWM4,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imapx200_device_pwm = {
	.name		= "imapx200_pwm",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imapx200_pwm_resource),
	.resource	= imapx200_pwm_resource,
};

EXPORT_SYMBOL(imapx200_device_pwm);

/************IIC***************/
static struct resource imapx200_iic0_resource[]={
	[0] = {
		.start = I2C0_BASE_REG_PA,
		.end   = I2C0_BASE_REG_PA + 0xfff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_IIC0,
		.end	= IRQ_IIC0,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imapx200_device_iic0 = {
	.name		= "imapx200_iic0",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imapx200_iic0_resource),
	.resource	= imapx200_iic0_resource,
};

EXPORT_SYMBOL(imapx200_device_iic0);

static struct resource imapx200_iic1_resource[]={
	[0] = {
		.start = I2C1_BASE_REG_PA,
		.end   = I2C1_BASE_REG_PA + 0xfff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_IIC1,
		.end	= IRQ_IIC1,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imapx200_device_iic1 = {
	.name		= "imapx200_iic1",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imapx200_iic1_resource),
	.resource	= imapx200_iic1_resource,
};

EXPORT_SYMBOL(imapx200_device_iic1);

/*************RTC******************/
static struct resource imapx200_rtc_resource[]={
	[0] = {
		.start = RTC_BASE_REG_PA,
		.end   = RTC_BASE_REG_PA + 0xfff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_RTCALARM,
		.end	= IRQ_RTCALARM,
		.flags	= IORESOURCE_IRQ,  
	},
	[2] = {
		.start	= IRQ_RTCTICK,
		.end	= IRQ_RTCTICK,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imapx200_device_rtc = {
	.name		= "imapx200_rtc",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imapx200_rtc_resource),
	.resource	= imapx200_rtc_resource,
};

EXPORT_SYMBOL(imapx200_device_rtc);

/*****************IIS*****************/
static struct resource imapx200_iis_resource[]={
	[0] = {
		.start = IIS_BASE_REG_PA,
		.end   = IIS_BASE_REG_PA + 0xfff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_IIS,
		.end	= IRQ_IIS,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imapx200_device_iis = {
	.name		= "imapx200_iis",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imapx200_iis_resource),
	.resource	= imapx200_iis_resource,
};

EXPORT_SYMBOL(imapx200_device_iis);

/******************* multi touch screen ******************************/
static struct resource imapx200_multi_touch_resource[]={
         [0] = {
                 .start = IRQ_EINT5,
                 .end = IRQ_EINT5,
                 .flags = IORESOURCE_IRQ,
         }
 
};
 
struct platform_device imapx200_devcie_mtouch = {
         .name           ="imapx200_multi_touch",
         .id             =0,
         .num_resources  = ARRAY_SIZE(imapx200_multi_touch_resource),
         .resource       = imapx200_multi_touch_resource,
         .dev = {
                 .platform_data = 1,
         }
};
 
EXPORT_SYMBOL(imapx200_devcie_mtouch);

/**************AC97*********************/
static struct resource imapx200_ac97_resource[]={
	[0] = {
		.start = AC97_BASE_REG_PA,
		.end   = AC97_BASE_REG_PA + 0xfff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_AC97,
		.end	= IRQ_AC97,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imapx200_device_ac97 = {
	.name		= "imapx200_ac97",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imapx200_ac97_resource),
	.resource	= imapx200_ac97_resource,
};

EXPORT_SYMBOL(imapx200_device_ac97);

/******************SPI*****************/
static struct s3c2410_spi_info imapx200_spi_platdata = {
	.pin_cs = IMAPX200_GPE(14),
	.num_cs = 1,
	.bus_num =0,
//	.board_info = imapx200_spi_board,
//	.board_size = ARRAY_SIZE(imapx200_spi_board),
};

//static u64 spi_dmamask = DMA_BIT_MASK(32);

static struct resource imapx200_ssim0_resource[]={
	[0] = {
		.start = SSI_MST0_BASE_REG_PA,
		.end   = SSI_MST0_BASE_REG_PA + 0xfff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_SSI0_MST,
		.end	= IRQ_SSI0_MST,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imapx200_device_ssim0 = {
	.name		= "imapx200_ssim0",
	.id 		= 0,
	.num_resources	= ARRAY_SIZE(imapx200_ssim0_resource),
	.resource	= imapx200_ssim0_resource,
	.dev = {
		.platform_data = &imapx200_spi_platdata,
//		.dma_mask               = &spi_dmamask,
//		.coherent_dma_mask      = DMA_BIT_MASK(32),
	}
};
EXPORT_SYMBOL(imapx200_device_ssim0);

static struct resource imapx200_ssim1_resource[]={
	[0] = {
		.start = SSI_MST1_BASE_REG_PA,
		.end   = SSI_MST1_BASE_REG_PA + 0xfff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_SSI1_MST,
		.end	= IRQ_SSI1_MST,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imapx200_device_ssim1 = {
	.name		= "imapx200_spi",
	.id 		= 1,
	.num_resources	= ARRAY_SIZE(imapx200_ssim1_resource),
	.resource	= imapx200_ssim1_resource,
};

EXPORT_SYMBOL(imapx200_device_ssim1);

static struct resource imapx200_ssim2_resource[]={
	[0] = {
		.start = SSI_MST2_BASE_REG_PA,
		.end   = SSI_MST2_BASE_REG_PA + 0xfff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_SSI2_MST,
		.end	= IRQ_SSI2_MST,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imapx200_device_ssim2 = {
	.name		= "imapx200_spi",
	.id 		= 2,
	.num_resources	= ARRAY_SIZE(imapx200_ssim2_resource),
	.resource	= imapx200_ssim2_resource,
};

EXPORT_SYMBOL(imapx200_device_ssim2);

static struct resource imapx200_ssis_resource[]={
	[0] = {
		.start = SSI_SLV_BASE_REG_PA,
		.end   = SSI_SLV_BASE_REG_PA + 0xfff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_SSI1_MST,
		.end	= IRQ_SSI1_MST,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imapx200_device_ssis = {
	.name		= "imapx200_ssis",
	.id 		= 3,
	.num_resources	= ARRAY_SIZE(imapx200_ssis_resource),
	.resource	= imapx200_ssis_resource,
};

EXPORT_SYMBOL(imapx200_device_ssis);

static struct resource imapx200_spi_resource[]={
	[0] = {
		.start = SPI_BASE_REG_PA,
		.end   = SPI_BASE_REG_PA + 0xfff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_UART1,
		.end	= IRQ_UART1,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imapx200_device_spi = {
	.name		= "imapx200_spi",
	.id 		= 4,
	.num_resources	= ARRAY_SIZE(imapx200_spi_resource),
	.resource	= imapx200_spi_resource,
};

EXPORT_SYMBOL(imapx200_device_spi);

/******************* watchdog ******************************/
static struct resource imapx200_watchdog_resource[]={
	[0] = {
		.start = SYSMGR_BASE_REG_PA,//WDT_BASE_REG_PA,
		.end   = SYSMGR_BASE_REG_PA + 0xfff,//WDT_BASE_REG_PA + 0xfff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_WDT,
		.end = IRQ_WDT,
		.flags = IORESOURCE_IRQ,
	}

};

struct platform_device imapx200_devcie_watchdog = {
	.name           ="imapx200-wdt",
	.id             =0,
	.num_resources  = ARRAY_SIZE(imapx200_watchdog_resource),
	.resource       = imapx200_watchdog_resource,
	.dev = {
		.platform_data = 1,
	}
};

EXPORT_SYMBOL(imapx200_devcie_watchdog);

/************Keyboard*************/
static struct resource imapx200_keybd_resource[]={
	[0] = {
		.start = KEYBD_BASE_REG_PA,
		.end   = KEYBD_BASE_REG_PA + 0xfff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_KeyBoard,
		.end	= IRQ_KeyBoard,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imapx200_device_keybd = {
	.name		= "imapx200_keybd",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imapx200_keybd_resource),
	.resource	= imapx200_keybd_resource,
};

EXPORT_SYMBOL(imapx200_device_keybd);

/*************PIC**********/
static struct resource imapx200_pic0_resource[]={
	[0] = {
		.start = PIC0_BASE_REG_PA,
		.end   = PIC0_BASE_REG_PA + 0xfff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_PS2_0,
		.end	= IRQ_PS2_0,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imapx200_device_pic0 = {
	.name		= "imapx200_pic",
	.id 		= 0,
	.num_resources	= ARRAY_SIZE(imapx200_pic0_resource),
	.resource	= imapx200_pic0_resource,
};

EXPORT_SYMBOL(imapx200_device_pic0);

static struct resource imapx200_pic1_resource[]={
	[0] = {
		.start = PIC1_BASE_REG_PA,
		.end   = PIC1_BASE_REG_PA + 0xfff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_PS2_1,
		.end	= IRQ_PS2_1,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imapx200_device_pic1 = {
	.name		= "imapx200_pic",
	.id 		= 1,
	.num_resources	= ARRAY_SIZE(imapx200_pic1_resource),
	.resource	= imapx200_pic1_resource,
};

EXPORT_SYMBOL(imapx200_device_pic1);

/**************BACKLIGHT***************/
static struct resource imapx200_bl_resource[]={
	[0] = {
		.start = PIC1_BASE_REG_PA,
		.end   = PIC1_BASE_REG_PA + 0xfff,
		.flags = IORESOURCE_MEM,
	},
};

struct platform_device imapx200_device_bl = {
	.name		= "imap-bl",
	.id 		= -1,
	.num_resources	= 1,
	.resource	= imapx200_bl_resource,
};

EXPORT_SYMBOL(imapx200_device_bl);

/***************GRAPHIC*****************/
static struct resource imapx200_graphic_resource[]={
	[0] = {
		.start = GRAPHIC_BASE_REG_PA,
		.end   = GRAPHIC_BASE_REG_PA + SZ_1M -1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_2D3DGPU,
		.end	= IRQ_2D3DGPU,
		.flags	= IORESOURCE_IRQ,  
	}
};

struct platform_device imapx200_device_graphic = {
	.name		= "imapx200_graphic",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(imapx200_graphic_resource),
	.resource	= imapx200_graphic_resource,
};

EXPORT_SYMBOL(imapx200_device_graphic);

/*************vibrate********************/
static struct timed_gpio android_gpio = {
	.name = "vibrator",
	.max_timeout = 10000,
	.active_low = 0,
};

struct timed_gpio_platform_data android_vibrator_data = {
	.num_gpios	= 1,
	.gpios		= &android_gpio,
	/*
	.gpios		= {
		.name = "vibrator",
		.max_timeout = 10000,
		.active_low = 0,
	}
	*/
};

struct platform_device android_vibrator_device = {
	.name		= "timed-gpio",
	.id		= -1,
	.dev = { .platform_data = &android_vibrator_data},                                             
};
EXPORT_SYMBOL(android_vibrator_device);


//ps2_0
struct amba_device imap_ps2_device = {
        .dev            = {
                .init_name = "kmi-pl050",
                .coherent_dma_mask = ~0,
        },
        .res            = {
                .start  = 0x20E70000,
                .end    = 0x20E71000,
                .flags  = IORESOURCE_MEM,
        },
        .irq            = { IRQ_PS2_0, NO_IRQ },
        .periphid       = 0x00041050,
};
EXPORT_SYMBOL(imap_ps2_device);

/*
//ps2_1
struct amba_device imap_ps2_device = {
        .dev            = {
                .init_name = "kmi-pl050",
                .coherent_dma_mask = ~0,
        },
        .res            = {
                .start  = 0x20E71000,
                .end    = 0x20E73000,
                .flags  = IORESOURCE_MEM,
        },
        .irq            = { IRQ_PS2_1, NO_IRQ },
        .periphid       = 0x00041050,
};
EXPORT_SYMBOL(imap_ps2_device);
*/

//android pmem
#ifdef CONFIG_ANDROID_PMEM
static struct android_pmem_platform_data android_pmem_pdata = {                                      
	.name = "pmem",
//	.start = 0x4f800000,
//	.size = SZ_4M, 
	.no_allocator = 1, 
	.cached = 1,                                                                                 
};                                                                                                   

static struct android_pmem_platform_data android_pmem_adsp_pdata = {                                 
	.name = "pmem_adsp",                                                                         
//	.start = 0x4fc00000,
//	.size = SZ_4M,
	.no_allocator = 0,
	.cached = 0,
};      

struct platform_device android_pmem_device = {                                                       
	.name = "android_pmem",
	.id = 0,
	.dev = { .platform_data = &android_pmem_pdata },                                             
};      

struct platform_device android_pmem_adsp_device = {
	.name = "android_pmem",                                                                      
	.id = 1,
	.dev = { .platform_data = &android_pmem_adsp_pdata },                                        
};      
EXPORT_SYMBOL(android_pmem_device);
EXPORT_SYMBOL(android_pmem_adsp_device);
#endif

static struct gpio_switch_platform_data android_wifi_switch_pdata = {
	.name = "switch-wifi",
	.gpio = IRQ_GPIO,
	.name_on = "switch-wifi",
	.name_off = "switch-wifi",
	.state_on = "1",
	.state_off = "0",
};

struct platform_device android_wifi_switch_device = {
	.name = "switch-wifi",
	.id = 0,
	.dev = {
		.platform_data = &android_wifi_switch_pdata
	},
};

#ifdef CONFIG_SWITCH_SENSOR
static struct gpio_switch_platform_data android_sensor_switch_pdata = {
	.name = "switch-sensor",
	.gpio = IRQ_GPIO,
	.name_on = "switch-sensor",
	.name_off = "switch-sensor",
	.state_on = "1",
	.state_off = "0",
};

struct platform_device android_sensor_switch_device = {
	.name = "switch-sensor",
	.id = 0,
	.dev = {
		.platform_data = &android_sensor_switch_pdata
	},
};
EXPORT_SYMBOL(android_sensor_switch_device);
#endif

struct platform_device android_device_batt = {
	.name = "battery",
	.id = -1,
};
EXPORT_SYMBOL(android_device_batt);

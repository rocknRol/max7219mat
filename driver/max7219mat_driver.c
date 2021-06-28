/*
 * Copyright (C) 2021 Rolando Spennato (@rocknRol)
 *
 * This driver is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This driver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/slab.h>

/* define macro to handle hardware Raspberry PI 3 model B v1.2 (ref: BROADCOM BCM2837 ARM Peripherals) */
#define BCM2837_PERI_PHYS_ADDRS_BASE	0x3F000000 /* Peripherals physical address */
#define GPIO_PHYS_ADDRS_BASE	(BCM2837_PERI_PHYS_ADDRS_BASE + 0x200000)	/* GPIO controller physical address */

/* GPIO Pin used */
#define GPIO21	21 /* CS (Chip Select, Enable on SN74154) - Raspberry pin 40 */
#define GPIO22	22 /* DIN (Data Input on MAXIM7219) - Raspberry pin 15 */
#define GPIO23	23 /* A3/D (Address Input on SN74154) - Raspberry pin 16 */
#define GPIO24	24 /* A2/C (Address Input on SN74154) - Raspberry pin 18 */
#define GPIO25	25 /* A1/B (Address Input on SN74154) - Raspberry pin 22 */
#define GPIO26	26 /* A0/A (Address Input on SN74154) - Raspberry pin 37 */
#define GPIO27	27 /* CLK (Clock Signal on MAXIM7219) - Raspberry pin 13 */

#define GPFSEL2		GPIO_PHYS_ADDRS_BASE + 0x08 /* GPIO Function Select Registers (GPFSEL2: GPIO 20...29) */
#define GPSET0		GPIO_PHYS_ADDRS_BASE + 0x1C /* GPIO Pin Output Set Registers (GPSET0: GPIO 0...31) */
#define GPCLR0		GPIO_PHYS_ADDRS_BASE + 0x28 /* GPIO Pin Output Clear Registers (GPCLR0: GPIO 0...31) */

/* GPFSEL2 select bit mask */
#define FSEL21_MASK		0b111 << ((GPIO21 % 10) * 3)
#define FSEL22_MASK		0b111 << ((GPIO22 % 10) * 3)
#define FSEL23_MASK 	0b111 << ((GPIO23 % 10) * 3)
#define FSEL24_MASK 	0b111 << ((GPIO24 % 10) * 3)
#define FSEL25_MASK 	0b111 << ((GPIO25 % 10) * 3)
#define FSEL26_MASK 	0b111 << ((GPIO26 % 10) * 3)
#define FSEL27_MASK 	0b111 << ((GPIO27 % 10) * 3)
#define GPFSEL2_ALL_MASK  (FSEL21_MASK | FSEL22_MASK | FSEL23_MASK | FSEL24_MASK | FSEL25_MASK | FSEL26_MASK | FSEL27_MASK)

/* GPFSEL2 select bit index */
#define FSEL21_INDEX 	0b1 << ((GPIO21 % 10) * 3)
#define FSEL22_INDEX 	0b1 << ((GPIO22 % 10) * 3)
#define FSEL23_INDEX 	0b1 << ((GPIO23 % 10) * 3)
#define FSEL24_INDEX 	0b1 << ((GPIO24 % 10) * 3)
#define FSEL25_INDEX 	0b1 << ((GPIO25 % 10) * 3)
#define FSEL26_INDEX 	0b1 << ((GPIO26 % 10) * 3)
#define FSEL27_INDEX 	0b1 << ((GPIO27 % 10) * 3)
#define GPFSEL2_ALL_INDEX  (FSEL21_INDEX | FSEL22_INDEX | FSEL23_INDEX | FSEL24_INDEX | FSEL25_INDEX | FSEL26_INDEX | FSEL27_INDEX)

/* GPSET0 and GPCLR0 select bit index */
#define GPX21_INDEX 	0b1 << (GPIO21 % 32)
#define GPX22_INDEX 	0b1 << (GPIO22 % 32)
#define GPX23_INDEX 	0b1 << (GPIO23 % 32)
#define GPX24_INDEX 	0b1 << (GPIO24 % 32)
#define GPX25_INDEX 	0b1 << (GPIO25 % 32)
#define GPX26_INDEX 	0b1 << (GPIO26 % 32)
#define GPX27_INDEX 	0b1 << (GPIO27 % 32)
#define GPX_ALL_INDEX  (GPX21_INDEX | GPX22_INDEX | GPX23_INDEX | GPX24_INDEX | GPX25_INDEX | GPX26_INDEX | GPX27_INDEX)

/* declare __iomem pointers that will keep virtual addresses */
static void __iomem *GPFSEL2_V;
static void __iomem *GPSET0_V;
static void __iomem *GPCLR0_V;

/* number of display selected */
u8 display_selected;

void select_display_module(void) {
	if ((display_selected >> 0) & 0x0001)
		iowrite32(GPX26_INDEX, GPSET0_V);
	else
		iowrite32(GPX26_INDEX, GPCLR0_V);

	if ((display_selected >> 1) & 0x0001)
		iowrite32(GPX25_INDEX, GPSET0_V);
	else
		iowrite32(GPX25_INDEX, GPCLR0_V);

	if ((display_selected >> 2) & 0x0001)
		iowrite32(GPX24_INDEX, GPSET0_V);
	else
		iowrite32(GPX24_INDEX, GPCLR0_V);

	if ((display_selected >> 3) & 0x0001)
		iowrite32(GPX23_INDEX, GPSET0_V);
	else
		iowrite32(GPX23_INDEX, GPCLR0_V);
}

void enable_demux(int cs)
{
	if(cs)
		iowrite32(GPX21_INDEX, GPCLR0_V);
	else
		iowrite32(GPX21_INDEX, GPSET0_V);
}

static ssize_t max7219mat_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	int i, j;
	u16 *com;
	unsigned short dt = 25; /* in ns */

	com = kmalloc(count, GFP_KERNEL);

	if(copy_from_user(com, buf, count)) {
		pr_info("max7219mat - Error in copy_from_user\n");
		return -EFAULT;
	}

	for(j = 0; j < (count - (count % 2)) / 2; j++) {
		ndelay(dt);
		select_display_module();
		enable_demux(1);

		for(i = 15; i >= 0 ; i--) {
			if ((*(com + j) >> i) & 0x0001)
				iowrite32(GPX22_INDEX, GPSET0_V);
			else
				iowrite32(GPX22_INDEX, GPCLR0_V);

			ndelay(dt);

			iowrite32(GPX27_INDEX, GPSET0_V);
			ndelay(dt);

			iowrite32(GPX27_INDEX, GPCLR0_V);
			ndelay(dt);
		}

		ndelay(dt);
		enable_demux(0);
	}
	kfree(com);
	return count;
}

static long max7219mat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	if (cmd == 1 && arg >= 1 && arg <= 16)
		display_selected = (u8) --arg;
	else
		return -1;

	return 0;
}

static const struct file_operations max7219mat_fops = {
	.owner = THIS_MODULE,
	.write = max7219mat_write,
	.unlocked_ioctl = max7219mat_ioctl
};

static struct miscdevice max7219mat_miscdevice = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "max7219mat",
	.fops = &max7219mat_fops,
	.mode = 0666   /* set permission at 0666 instead those of default 0600 */
};

static int __init max7219mat_probe(struct platform_device *pdev)
{
	int ret;

	ret = misc_register(&max7219mat_miscdevice);
	if (ret != 0) {
		pr_info("max7219mat - error in misc_register. Value returned is %d\n", ret);
		return ret;
  }
	return 0;
}

static int __exit max7219mat_remove(struct platform_device *pdev)
{
	misc_deregister(&max7219mat_miscdevice);
	return 0;
}

static const struct of_device_id list_of_dev_id[] = {
	{ .compatible = "rocknrol,max7219mat"},
	{},
};

MODULE_DEVICE_TABLE(of, list_of_dev_id);

static struct platform_driver max7219mat_driver = {
	.probe = max7219mat_probe,
	.remove = max7219mat_remove,
	.driver = {
		.name = "max7219mat",
		.of_match_table = list_of_dev_id,
		.owner = THIS_MODULE,
	}
};

static int max7219mat_init(void)
{
	int ret;
	u32 GPFSEL2_oldValue, GPFSEL2_newValue;

	ret = platform_driver_register(&max7219mat_driver);
	if (ret !=0) {
    	pr_err("max7219mat - error in platform_driver_register. Value returned is %d\n", ret);
    	return ret;
	}

	/* convert physical the addresses of registers in virtual addresses */
	GPFSEL2_V = ioremap(GPFSEL2, sizeof(u32));
	GPSET0_V = ioremap(GPSET0, sizeof(u32));
	GPCLR0_V = ioremap(GPCLR0, sizeof(u32));

	/* read the current value and merge it with the new. It is safe for any other applications */
	GPFSEL2_oldValue = ioread32(GPFSEL2_V); 
	GPFSEL2_newValue = (GPFSEL2_oldValue & ~GPFSEL2_ALL_MASK) | GPFSEL2_ALL_INDEX;
	iowrite32(GPFSEL2_newValue, GPFSEL2_V);

	return 0;
}

static void max7219mat_exit(void)
{
	/* set output high in accordance with DTS*/
	iowrite32(GPX_ALL_INDEX, GPSET0_V);

	iounmap(GPFSEL2_V);
	iounmap(GPSET0_V);
	iounmap(GPCLR0_V);

	platform_driver_unregister(&max7219mat_driver);
}

module_init(max7219mat_init);
module_exit(max7219mat_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rolando Spennato <rocknrol76dev@gmail.com>");
MODULE_DESCRIPTION("Raspberry PI 3B platform driver with device tree for an homebrew display 32x32 based on MAXIM 7219");

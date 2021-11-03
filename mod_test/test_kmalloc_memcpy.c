/*
	demo_container_of.c - template for char device, char device interface

	Copyright (C) 2021 Zheng Hua <writeforever@foxmail.com>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License along
	with this program; if not, write to the Free Software Foundation, Inc.,
	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/gpio.h>		/* for kmalloc */


#define drv_inf(msg) printk(KERN_INFO "%s: "msg ,__func__)
#define drv_dbg(msg) printk(KERN_DEBUG "%s: "msg, __func__)
#define drv_wrn(msg) printk(KERN_WARNING "%s: "msg, __func__)
#define drv_err(msg) printk(KERN_ERR "%s: "msg, __func__)
#define BUFFER_WIDTH 4096
#define BUFFER_HEIGHT 2160
#define BUFFER_SIZE BUFFER_WIDTH*BUFFER_HEIGHT*4
#define NANOTIME_PER_MSECOND 1000000L

unsigned char *srcBuffer;
unsigned char *dstBuffer;



static __init int demo_init(void)
{
	volatile uint64_t start_time,end_time;

	srcBuffer = vmalloc(BUFFER_SIZE);
	dstBuffer = vmalloc(BUFFER_SIZE);

	start_time = jiffies;
	printk("start: %lld\n", jiffies);
	memcpy(srcBuffer, dstBuffer, BUFFER_SIZE);
	end_time = jiffies;
	printk("end: %lld\n", jiffies);

	printk("machine HZ: %d\n", HZ);
	
	printk("end_time: %lld, cost: %lld\n", end_time, end_time - start_time);

	vfree(srcBuffer);
	vfree(dstBuffer);

    return 0;
}

static __exit void demo_exit(void)
{
    printk(KERN_INFO "%s Exiting.\n", __func__);

}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zheng Hua <writeforever@foxmail.com>");
MODULE_DESCRIPTION("CDEV/ a driver template for cdev");

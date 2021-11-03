#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>
#include <linux/io.h>

static struct fb_info *myfb_info;

static struct fb_ops myfb_ops = {
	.owner = THIS_MODULE,
	.fb_fillrect = cfb_fillrect,
	.fb_copyarea = cfb_copyarea,
	.fb_imageblit = cfb_imageblit,
};

int __init lcd_drv_init(void)
{
	dma_addr_t phy_addr;

	myfb_info = framebuffer_alloc(0,  NULL);

	myfb_info->var.xres = 800;
	myfb_info->var.yres = 480;
	myfb_info->var.bits_per_pixel = 16;
	
	myfb_info->var.red.offset = 11;
	myfb_info->var.red.length = 5;
	
	myfb_info->var.green.offset=5;
	myfb_info->var.green.length=6;
	
	myfb_info->var.blue.offset=5;
	myfb_info->var.blue.length=5;
	
	myfb_info->fix.smem_len = myfb_info->var.xres * myfb_info->var.yres \
								* myfb_info->var.bits_per_pixel / 8;
	if(myfb_info->var.bits_per_pixel == 24)
		myfb_info->fix.smem_len = myfb_info->var.xres * myfb_info->var.yres * 4;

	/**
	 * why use `dma_alloc_wc` not `kmalloc` ?
	 * the `dma_alloc_wc` get the virtual memory start and space, same as kmalloc
	 * but physical continuity is for `dma_alloc_wc` 
	 */
	myfb_info->screen_base = dma_alloc_wc(NULL, myfb_info->fix.smem_len, &phy_addr, GFP_KERNEL);

	myfb_info->fix.smem_start = phy_addr;

	myfb_info->fix.type = FB_TYPE_PACKED_PIXELS;
	myfb_info->fix.visual=FB_VISUAL_TRUECOLOR;

	/* setting framebuffer operations */
	myfb_info->fbops = &myfb_ops;

	/* register it  */
	register_framebuffer(myfb_info);

	/* hardware operations */
	
	return 0;
}

static void __exit lcd_drv_cleanup(void)
{
	unregister_framebuffer(myfb_info);

	framebuffer_release(myfb_info);
}

module_init(lcd_drv_init);
module_exit(lcd_drv_cleanup);

MODULE_AUTHOR("Zheng Hua <writeforever@foxmail.com>");
MODULE_DESCRIPTION("Framebuffer driver");
MODULE_LICENSE("GPL");

#include <linux/kernel.h>
#include <linux/dma-mapping.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/gfp.h>
#include <linux/module.h>
#include <asm/gpio.h>

volatile unsigned int *GPF0CON;
volatile unsigned int *GPF1CON;
volatile unsigned int *GPF2CON;
volatile unsigned int *GPF3CON;
volatile unsigned int *GPD0CON ;
volatile unsigned int *GPD0DAT;
volatile unsigned int *CLK_SRC_LCD0;
volatile unsigned int *CLK_DIV_LCD;
volatile unsigned int *LCDBLK_CFG;
volatile unsigned int *VIDCON0;
volatile unsigned int *VIDCON1;
volatile unsigned int *VIDCON2;
volatile unsigned int *VIDTCON0;
volatile unsigned int *VIDTCON1; 
volatile unsigned int *VIDTCON2;
volatile unsigned int *WINCON0;
volatile unsigned int *SHADOWCON;
volatile unsigned int *WINCHMAP2;
volatile unsigned int *VIDOSD0A; 
volatile unsigned int *VIDOSD0B;
volatile unsigned int *VIDOSD0C;
volatile unsigned int *VIDW0nADD0Bn;
volatile unsigned int *VIDW0nADD1Bn;
volatile unsigned int *VIDW0nADD2;

u32	 pseudo_palette[16];
static inline unsigned int chan_to_field(unsigned int chan,
					 struct fb_bitfield *bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}

static int lcd_setcolreg(unsigned int regno, unsigned int red,
			     unsigned int green, unsigned int blue,
			     unsigned int transp, struct fb_info *info)
{
	unsigned int val;
	u32 *pal;

	if (regno < 16) {
		pal = pseudo_palette;

		val  = chan_to_field(red, &info->var.red);
		val |= chan_to_field(green, &info->var.green);
		val |= chan_to_field(blue, &info->var.blue);

		pal[regno] = val;
	}
	return 0;

}

struct fb_ops lcd_ops = {
	.owner		= THIS_MODULE,
	.fb_setcolreg	= lcd_setcolreg,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,

};

struct fb_info *lcd_info;
static int __init  lcd_init(void)
{
	int ret;
	/*1.分配结构体*/
	lcd_info  = framebuffer_alloc(0,NULL);
	if (!lcd_info) {
		printk("cannot allocate memory\n");
		return -ENOMEM;
	}
	/*2.结构体的初始化*/
	memcpy(lcd_info->fix.id,"lcd",16);
	//lcd_info->fix.smem_start = 
	lcd_info->fix.smem_len = 1024*600*16/8;
	lcd_info->fix.type = FB_TYPE_PACKED_PIXELS;
	lcd_info->fix.visual = FB_VISUAL_TRUECOLOR;
	lcd_info->fix.line_length = 1024*2;

	lcd_info->var.xres = 1024;
	lcd_info->var.yres = 600;
	lcd_info->var.xres_virtual = 1024;
	lcd_info->var.yres_virtual = 600;
	lcd_info->var.xoffset = 0;
	lcd_info->var.yoffset = 0;
	lcd_info->var.bits_per_pixel = 16;
	lcd_info->var.red.offset = 11;
	lcd_info->var.red.length = 5;
	lcd_info->var.green.offset = 5;
	lcd_info->var.green.length = 6;
	lcd_info->var.blue.offset = 0;
	lcd_info->var.blue.length = 5;
	lcd_info->var.activate = FB_ACTIVATE_NOW;

	lcd_info->fbops = &lcd_ops;

	lcd_info->screen_base = dma_alloc_writecombine(NULL, lcd_info->fix.smem_len,
		(dma_addr_t *)&lcd_info->fix.smem_start, GFP_KERNEL);
	
	lcd_info->screen_size =  lcd_info->fix.smem_len;

	lcd_info->pseudo_palette = pseudo_palette;
		
	/*3.硬件相关的操作*/
	/*3.1gpio初始化*/
	GPF0CON = ioremap(0x11400180,4);
	GPF1CON = ioremap(0x114001a0,4);
	GPF2CON = ioremap(0x114001c0,4);
	GPF3CON = ioremap(0x114001e0,4);
	GPD0CON = ioremap(0x114000a0,4);
	GPD0DAT = ioremap(0x114000a4,4);
	writel(0x22222222,GPF0CON);
	writel(0x22222222,GPF1CON);
	writel(0x22222222,GPF2CON);
	writel(0x00222222,GPF3CON);
	writel(((readl(GPD0CON)&~(0xf<<4))|(0x1<<4)),GPD0CON);
	writel((readl(GPD0DAT)|(0x1<<1)),GPD0DAT);

	/*3.2LCD控制器的初始化*/
	/*控制器时钟*/
	CLK_SRC_LCD0 = ioremap(0x1003c234,4);
	CLK_DIV_LCD = ioremap(0x1003c534,4);
	LCDBLK_CFG = ioremap(0x10010210,4);
	writel(((readl(CLK_SRC_LCD0)&~(0xf))|0x6),CLK_SRC_LCD0);
	writel((readl(CLK_DIV_LCD)&~(0xf)),CLK_DIV_LCD);
	writel((readl(LCDBLK_CFG)|(0x1<<1)),LCDBLK_CFG);

	/*LCD控制器的初始化*/
	VIDCON0 = ioremap(0x11c00000,4);
	VIDCON1 = ioremap(0x11c00004,4);
	VIDCON2 = ioremap(0x11c00008,4);
	VIDTCON0 = ioremap(0x11c00010,4);
	VIDTCON1 = ioremap(0x11c00014,4);
	VIDTCON2 = ioremap(0x11c00018,4);
	WINCON0 = ioremap(0x11c00020,4);
	SHADOWCON= ioremap(0x11c00034,4);
	WINCHMAP2 = ioremap(0x11c0003c,4);
	VIDOSD0A = ioremap(0x11c00040,4);
	VIDOSD0B = ioremap(0x11c00044,4);
	VIDOSD0C = ioremap(0x11c00048,4);
	VIDW0nADD0Bn = ioremap(0x11c000a0,4);
	VIDW0nADD1Bn = ioremap(0x11c000d0,4);
	VIDW0nADD2 = ioremap(0x11c00100,4);
	
	*VIDCON0 = 14<<6;
	*VIDCON1 = (1<<9)|(1<<6)|(1<<5); 
	*VIDCON2 = (3<<14);
	*VIDTCON0 = (22<<16)|(11<<8)|(9<<0);
	*VIDTCON1 = (159<<16)|(159<<8)|(10<<0);
	*VIDTCON2 = (599<<11)|(1023<<0);
	*WINCON0 = (1<<16)|(5<<2);
	*SHADOWCON = 1;
	*WINCHMAP2 = (1 << 0) | (1 << 16);
	*VIDOSD0A  = 0;
	*VIDOSD0B  = (1024<<11)|(600<<0);
	*VIDOSD0C = (1024*600);
	*VIDW0nADD0Bn = lcd_info->fix.smem_start;
	*VIDW0nADD1Bn = lcd_info->fix.smem_start + lcd_info->fix.smem_len;
	*VIDW0nADD2 = (1024*2<<0);
		
	/*使能*/
	*VIDCON0 |=0x3;
	*WINCON0 |= 1; 
	
	/*4.注册和注销*/
	ret = register_framebuffer(lcd_info);
	if(ret < 0)
	{
		printk("register farmebuffer is fail.\n");
		return -EAGAIN;
	}
	
	return 0;
}

static void __exit lcd_exit(void)
{
	unregister_framebuffer(lcd_info);
}

module_init(lcd_init);
module_exit(lcd_exit);
MODULE_LICENSE("GPL");


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#include <asm/io.h>
#include <asm/uaccess.h>

MODULE_LICENSE("Dual BSD/GPL");

#define LED_MAGIC 'L'
/*
 * need arg = 1/2 
 */

#define LED_ON	_IOW(LED_MAGIC, 0, int)
#define LED_OFF	_IOW(LED_MAGIC, 1, int)


#define LED_MA 500
#define LED_MI 0
#define LED_NUM 1

#define FS4412_GPX2CON	0x11000C40
#define FS4412_GPX2DAT	0x11000C44

static unsigned int *gpx2con;
static unsigned int *gpx2dat;

struct cdev cdev;

static int s5pv210_led_open(struct inode *inode, struct file *file)
{
	return 0;
}
	
static int s5pv210_led_release(struct inode *inode, struct file *file)
{
	return 0;
}
	
static long s5pv210_led_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{

	switch (cmd) {
		case LED_ON:
			writel(readl(gpx2dat) | 1 << 7, gpx2dat);
			break;
		case LED_OFF:
			writel(readl(gpx2dat) & ~(1 << 7), gpx2dat);
			break;
	}

	return 0;
}
	
struct file_operations s5pv210_led_fops = {
	.owner = THIS_MODULE,
	.open = s5pv210_led_open,
	.release = s5pv210_led_release,
	.unlocked_ioctl = s5pv210_led_unlocked_ioctl,
};

static int s5pv210_led_init(void)
{
	dev_t devno = MKDEV(LED_MA, LED_MI); 
	int ret;

	ret = register_chrdev_region(devno, LED_NUM, "newled");
	if (ret < 0) {
		printk("register_chrdev_region\n");
		return ret;
	}

	cdev_init(&cdev, &s5pv210_led_fops);
	cdev.owner = THIS_MODULE;
	ret = cdev_add(&cdev, devno, LED_NUM);
	if (ret < 0) {
		printk("cdev_add\n");
		goto err1;
	}

	gpx2con = ioremap(FS4412_GPX2CON, 4);
	if (gpx2con == NULL) {
		printk("ioremap gpx2con\n");
		ret = -ENOMEM;
		goto err2;
	}

	gpx2dat = ioremap(FS4412_GPX2DAT, 4);
	if (gpx2dat == NULL) {
		printk("ioremap gpx2dat\n");
		ret = -ENOMEM;
		goto err3;
	}

	writel((readl(gpx2con) & ~(0xf << 28)) | (0x1 << 28), gpx2con);
	writel(readl(gpx2dat) & ~(0x1<<7), gpx2dat);

	printk("Led init\n");

	return 0;
err3:
	iounmap(gpx2con);
err2:
	cdev_del(&cdev);
err1:
	unregister_chrdev_region(devno, LED_NUM);
	return ret;
}

static void s5pv210_led_exit(void)
{
	dev_t devno = MKDEV(LED_MA, LED_MI);

	iounmap(gpx2dat);
	iounmap(gpx2con);
	cdev_del(&cdev);
	unregister_chrdev_region(devno, LED_NUM);
	printk("Led exit\n");
}

module_init(s5pv210_led_init);
module_exit(s5pv210_led_exit);

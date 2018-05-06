#include <linux/module.h>
#include <linux/stat.h>
int n = 0;
module_param(n, int, 0000);
MODULE_PARM_DESC(n, "n int !!  param");

int m = 0;
module_param(m, int, 0664);
MODULE_PARM_DESC(m, "m int !!  param");


static int __init my_module_init(void) 
{
	printk("module init \n");

	return 0;
}


static void __exit my_module_exit(void) 
{
	printk("module exit\n");

	return;
}


MODULE_LICENSE("GPL");
MODULE_AUTHOR("wusz_ck@farsight.com.cn");
MODULE_DESCRIPTION("a simple driver example!");

module_init(my_module_init);
module_exit(my_module_exit);




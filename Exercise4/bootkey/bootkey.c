#include <linux/gpio.h> 
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/module.h>

#define MAJORNUM 64
#define MINORNUM 0

#define GPIO_NUM 7

MODULE_LICENSE("Dual BSD/GPL");

static struct cdev my_cdev;
struct file_operations my_fops;

static int bootkey_init(void) {
	int errNo;
	int devno = MKDEV(MAJORNUM, MINORNUM);

	errNo = gpio_request(GPIO_NUM, "gpio7");
	if (errNo < 0) {
		printk(KERN_ALERT "gpio_request failed: %d", errNo);
		return -1;
	}

	gpio_direction_input(GPIO_NUM);
	cdev_init(&my_cdev, &my_fops);

	errNo = register_chrdev_region(devno, 1, "boot_key");
	if (errNo < 0) {
		printk(KERN_ALERT "Registration of device number failed! %d\n", errNo);
		gpio_free(GPIO_NUM);
	}

	if (cdev_add(&my_cdev, devno, 1) < 0) {
		printk(KERN_ALERT "Cdev_add failed! :(");
		unregister_chrdev_region(my_cdev.dev, 1);
		gpio_free(GPIO_NUM);
	}

	return 0;
}

static void bootkey_exit(void) {
	cdev_del(&my_cdev);
	unregister_chrdev_region(my_cdev.dev, 1);
	gpio_free(GPIO_NUM);
}

int bootkey_open(struct inode *inode, struct file * fp) {
	int major, minor;
	major = MAJOR(inode->i_rdev);
	minor = MINOR(inode->i_rdev);
	printk("Opening Boot_key device [major], [minor]: %i, %i\n", major, minor);

	return 0;
}

int bootkey_release(struct inode * inode, struct file * fp) {
	int major, minor;

	major = MAJOR(inode->i_rdev);
	minor = MINOR(inode->i_rdev);
	printk("Closing/releasing Boot_key device [major], [minor]: %i, %i\n", major, minor);

	return 0;
}

ssize_t bootkey_read(struct file * fp, char __user * buf, size_t count, loff_t * fpos) {
	int gpioValue = gpio_get_value(GPIO_NUM);
	char strValue[1];
	sprintf(strValue, "%d", gpioValue);
	unsigned long copyFailBytes = copy_to_user(buf, strValue, 1);
	if (copyFailBytes != 0) {
		printk(KERN_WARNING "Some bytes could not be copied. Amount: %lu", copyFailBytes);
	}

	return 1;
}

struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.read = bootkey_read,
	.open = bootkey_open,
	.release = bootkey_release,
};

module_init(bootkey_init);
module_exit(bootkey_exit);


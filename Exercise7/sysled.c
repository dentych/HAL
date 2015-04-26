#include <linux/gpio.h> 
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/timer.h>

#define GPIO_NUM 164

MODULE_LICENSE("Dual BSD/GPL");

struct file_operations my_fops;
static dev_t devt;
static struct class *sysled_class;
static struct device *sysled_device;
struct timer_list led_timer;

// Variables for toggle rate and toggle state.
static int togglestate = 0;
static int togglerate = 0;

static ssize_t sysled_toggle_state_show(struct device *dev, struct device_attribute *attr, char *buf) {
	int value = togglestate;
	int len = sprintf(buf, "%d\n", value);
	return len;
}

static ssize_t sysled_toggle_state_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	ssize_t ret;
	unsigned long value;

	if ((ret = kstrtoul(buf, 10, &value)) < 0) {
		printk(KERN_ALERT "STORE ERROR: %d\n", ret);
		return ret;
	}

	ret = count;
	
	togglestate = value;
	
	if (togglestate > 0) {
		gpio_set_value(GPIO_NUM, 0);
		led_timer.expires = jiffies + togglerate;
		add_timer(&led_timer);
	}

	return ret;
}

static ssize_t sysled_toggle_rate_show(struct device *dev, struct device_attribute *attr, char *buf) {
	int value = togglerate;
	int len = sprintf(buf, "%d\n", value);
	return len;
}

static ssize_t sysled_toggle_rate_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	ssize_t ret;
	unsigned long data;
	if ((ret = kstrtoul(buf, 10, &data)) < 0) {
		printk(KERN_ALERT "STORE ERROR: %d\n", ret);
		return ret;
	}
	ret = count;
	togglerate = data;
	return ret;
}

static void timer_funct(unsigned long param) {
	int value;
	if (togglestate > 0) {
		led_timer.expires = jiffies + togglerate;
		add_timer(&led_timer);

		// Toggle LED
		value = gpio_get_value(GPIO_NUM);
		if (value > 0)
			gpio_set_value(GPIO_NUM, 0);
		else
			gpio_set_value(GPIO_NUM, 1);
	}
}

static struct device_attribute sysled_class_attrs[] = {
	__ATTR(toggle_state, 0644, sysled_toggle_state_show, sysled_toggle_state_store),
	__ATTR(toggle_rate, 0644, sysled_toggle_rate_show, sysled_toggle_rate_store),
	__ATTR_NULL,
};

static int sysled_init(void) {
	int errNo;

	if ((errNo = alloc_chrdev_region(&devt, 0, 1, "sysleds_region")) != 0) {
		printk(KERN_ALERT "Memory alloc failed: %d", errNo);
		goto error_memalloc;
	}

	sysled_class = class_create(THIS_MODULE, "sysleds");
	if (IS_ERR(sysled_class)) {
		printk(KERN_ALERT "Class creation failed.");
		goto error_classcreate;
	}

	sysled_class->dev_attrs = sysled_class_attrs;

	sysled_device = device_create(sysled_class, NULL, devt, NULL, "sysled4");
	if (IS_ERR(sysled_device)) {
		printk(KERN_ALERT "Device creation failed.");
		goto error_devicecreate;
	}

	// Init timer
	init_timer(&led_timer);
	led_timer.function = timer_funct;

	return 0;

	error_devicecreate:
		PTR_ERR(sysled_device);
		class_destroy(sysled_class);
	error_classcreate:
		PTR_ERR(sysled_class);
		unregister_chrdev_region(devt, 1);
	error_memalloc:
	
	return -1;
}

static void __exit sysled_exit(void) {
	device_destroy(sysled_class, devt);
	class_destroy(sysled_class);
	unregister_chrdev_region(devt, 1);
}
/*

	errNo = gpio_request(GPIO_NUM, "gpio164");
	if (errNo < 0) {
		printk(KERN_ALERT "gpio_request failed: %d", errNo);
		return -1;
	}

	gpio_direction_output(GPIO_NUM, 1);
	cdev_init(&my_cdev, &my_fops);
	
	errNo = register_chrdev_region(devno, 1, "sysled4");
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

static void sysled_exit(void) {
	cdev_del(&my_cdev);
	unregister_chrdev_region(my_cdev.dev, 1);
	gpio_free(GPIO_NUM);
}

int sysled_open(struct inode *inode, struct file * fp) {
	int major, minor;
	major = MAJOR(inode->i_rdev);
	minor = MINOR(inode->i_rdev);
	printk("Opening Sys_led device [major], [minor]: %i, %i\n", major, minor);

	return 0;
}

int sysled_release(struct inode * inode, struct file * fp) {
	int major, minor;

	major = MAJOR(inode->i_rdev);
	minor = MINOR(inode->i_rdev);
	printk("Closing/releasing Sys_led device [major], [minor]: %i, %i\n", major, minor);

	return 0;
}

ssize_t sysled_read(struct file * fp, char __user * buf, size_t count, loff_t * fpos) {
	int gpioValue = gpio_get_value(GPIO_NUM);
	unsigned long err;

	if (gpioValue == 0) {
		err = copy_to_user(buf, "0", 2);
	}
	else {
		err = copy_to_user(buf, "1", 2);
	}

	if (err != 0) {
		printk(KERN_ALERT "Copy to user gave an error..!\n");
	}

	return 2;
}

ssize_t sysled_write(struct file * fp, const char __user * ubuf, size_t count, loff_t * fpos) {
	int value;
	if (sscanf(ubuf, "%d", &value) != 1) {
		printk(KERN_ALERT "Error reading");
	}

	gpio_set_value(GPIO_NUM, value);

	return count;
}

struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.read = sysled_read,
	.open = sysled_open,
	.write = sysled_write,
	.release = sysled_release,
};
*/

module_init(sysled_init);
module_exit(sysled_exit);


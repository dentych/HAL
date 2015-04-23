#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/pci.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/module.h>

struct input_dev* vms_input_dev;
static struct platform_device* vms_dev;



static ssize_t write_vms(struct device* dev,
		struct device_attribute* attr,
		const char* buffer, size_t count)
{
	int x,y;
	/* count is not used !? */
	sscanf(buffer, "%d%d", &x, &y);

	/*
	input_report_rel sætter en rapportering af de relative x og y værdier op.
	input_sync sender eventen med REL_X og REL_Y værdierne.
	*/
	input_report_rel(vms_input_dev, REL_X, x);
	input_report_rel(vms_input_dev, REL_Y, y);

	input_sync(vms_input_dev);
	printk("x = %d, y = %d\n", x, y);

	return count;
}

DEVICE_ATTR(coordinates, 0666, NULL, write_vms);

static struct attribute* vms_attr[] = 
{
	&dev_attr_coordinates.attr,
	NULL
};

static struct attribute_group vms_attr_group = 
{
	.attrs = vms_attr,
};


static int vms_open(struct input_dev *dev)
{
	return 0;
}

static void vms_close(struct input_dev *dev)
{
}

int __init vms_init(void)
{
	// Registerer en platform device
	vms_dev = platform_device_register_simple("vms", -1, NULL, 0);
	// Error handling
	if(IS_ERR(vms_dev))
	{
		PTR_ERR(vms_dev);
		printk("vms_init: error\n");
	}

	// Creates a sysfs group
	sysfs_create_group(&vms_dev->dev.kobj, &vms_attr_group);

	// Allocate memory for input device
	vms_input_dev = input_allocate_device();

	// Error handling
	if(!vms_input_dev)
	{
		printk("Bad input_alloc_device()\n");
	}

	// Set input device name
	vms_input_dev->name = "Test";

	/* Enable relative X/Y events */
	input_set_capability(vms_input_dev, EV_REL, REL_X);
	input_set_capability(vms_input_dev, EV_REL, REL_Y);
	/* Same as :
	   set_bit(EV_REL, vms_input_dev->evbit);   
	   set_bit(REL_X, vms_input_dev->relbit);
	   set_bit(REL_Y, vms_input_dev->relbit);
	 */

	/* Enable mouse button events
	   AT LEAST ONE BUTTON IS REQUIRED FOR THE INPUT 
	   DEVICE TO BE RECKOGNIZED AS A MOUSE DEVICE ==>
	   creation of /dev/input/mouse<NO> device
	 */   
	input_set_capability(vms_input_dev, EV_KEY, BTN_LEFT);
	input_set_capability(vms_input_dev, EV_KEY, BTN_MIDDLE);
	input_set_capability(vms_input_dev, EV_KEY, BTN_RIGHT);

	input_register_device(vms_input_dev);

	printk("Virtual Mouse Driver Initialized.\n");

	return 0;
}

void __exit vms_cleanup(void)
{
	input_unregister_device(vms_input_dev);

	sysfs_remove_group(&vms_dev->dev.kobj, &vms_attr_group);

	platform_device_unregister(vms_dev);

	return;
}


module_init(vms_init);
module_exit(vms_cleanup);

MODULE_AUTHOR("Danish Technological Institut");
MODULE_LICENSE("GPL");

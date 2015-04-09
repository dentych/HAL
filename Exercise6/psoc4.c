#include <linux/err.h>
#include <linux/ctype.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/input.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <plat/mcspi.h>

MODULE_LICENSE("GPL");

#define PSOC4_MAJOR 70
#define PSOC4_MINOR 0
#define DEVNUMS 1

// Variable for device number
static int devno;

// CDev structs
static struct cdev psoc4dev;
struct file_operations psoc4fops;

// SPI structs
static struct spi_device * psoc4_spi_device = NULL;
static struct spi_driver psoc4_spi_driver = {
	.driver = {1
		.name = "psoc4",
		.bus = &spi_bus_type,
		.owner = THIS_MODULE,
	},
	.probe = psoc4_spi_probe,
	.remove = __devexit_p(psoc4_spi_remove),
};

/* PSoC4 Device Data */
struct psoc4 {
  int revision;
};

static int __devinit psoc4_spi_probe(struct spi_device * spi) {
	int err;
	uint8 value;
	struct psoc4 * psocdev;

	printk(KERN_NOTICE "New SPI device: %s using chip select: %i\n",
		spi->modalias, spi->chip_select);

	spi->bits_per_word = 8;
	spi_setup(spi);

	// Only one device
	psoc4_spi_device = spi;

	err = psoc4_spi_read_reg8(spi, PSOC4_ID, &value);
	printk(KERN_NOTICE "Probing PSoC4, Revision %i\n",
		value);
}

static int __init psoc4_cdrv_init(void) {
	int err;
	printk(KERN_NOTICE "PSoC4 driver initialising\n");

	// Register SPI Driver
	err = spi_register_driver(&psoc4_spi_driver);
	// SPI Register error handling
	if (err < 0) {
		printk(KERN_ALERT "Error registering SPI driver! :( Error code: %d\n", err);
		goto error;
	}

	/* Allocate chrdev region */
	// Make a devno from the MAJOR and MINOR number
	devno = MKDEV(PSOC4_MAJOR, PSOC4_MINOR);
	err = register_chrdev_region(devno, DEVNUMS, "psoc4");
	if (err < 0) {
		printk(KERN_ALERT "Failed to register memory region. :( Error code: %d\n", err);
		goto err_spi_init;
	}

	/* Register Char Device */
	cdev_init(&psoc4dev, &psoc4fops);
	err = cdev_add(&psoc4dev, devno, DEVNUMS);
	if (err < 0) {
		printk(KERN_ALERT "Failed to add cdev. :( Error code: %d\n", err);
		goto err_register;
	}

	return 0;

err_register:
	// Memory region unregister
	unregister_chrdev_region(devno, DEVNUMS);
err_spi_init:
	// Spi unregister
error:
	// Simply return error
	return err;
}

module_init(psoc4_cdrv_init);

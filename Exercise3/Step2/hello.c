// Libraries
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/ioport.h>
#include <asm/io.h>
#include <linux/module.h>
MODULE_LICENSE("Dual BSD/GPL");


//Pointers to IO6 registers
volatile unsigned long *REG_GPIO6_OE;
volatile unsigned long *REG_GPIO6_DOUT;


static int hello_init(void) {

	uint32_t value = 0;
	//Bede kernen om lov til at benytte memory området
	if (request_mem_region (0x49058034, 12, "hello") == NULL) {
		printk("Allocation for I/O memory range is failed\n");
		return 0;
	}

	// Assign addresses to pointers.
	REG_GPIO6_OE = ioremap(0x49058034, 4);
	REG_GPIO6_DOUT = ioremap(0x4905803c, 4);

	// Read 32-bit
	value = ioread32(REG_GPIO6_OE);

	// Modify bit
	value &= ~(1<<4); 

	// Write 32-bit
	iowrite32(value, REG_GPIO6_OE);

	// Read & write 32-bit DOUT
	value = ioread32(REG_GPIO6_DOUT);
	value &= ~(1<<4);
	iowrite32(value, REG_GPIO6_DOUT);

}

static void hello_exit(void) {
	uint32_t value;
	printk(KERN_ALERT "Goodbye cruel world!");

	value = ioread32(REG_GPIO6_DOUT);
	value |= (1 << 4);
	iowrite32(value, REG_GPIO6_DOUT);
	release_mem_region (0x49058034, 12);
}

module_init(hello_init);
module_exit(hello_exit);

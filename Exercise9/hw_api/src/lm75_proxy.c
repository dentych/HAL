/*
 * lm75_proxy.c
 *
 *  Created on: 26/02/2014
 *      Author: phm
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include "temp_sensor.h"

/* Excerpt from i2c-dev.h */
#define I2C_SLAVE       0x0703  /* Set slave addr IOCTL cmd ID */

/* Local Definitions */
#define LM75_I2C_ADDR	0x48    /* Devkit8000 Add-On Board LM75 Address */
#define LM75_I2C_BUS	3

/*
 * Private Data Structures and Methods
 */

/*
 * Public Methods
 */
float lm75_getTempC(temp_sensor *cthis){
	return (float)(54.3);
}

/* Destructor */
void lm75_proxy_release(temp_sensor *cthis){
}

/* Constructor */
int lm75_proxy_init(temp_sensor *cthis, void *device){
	int fp=0;

	/* Assign Public Methods */

	/* Open Linux i2c-dev device */

	/* Set I2C Device address */
	if (ioctl(fp, I2C_SLAVE, LM75_I2C_ADDR) < 0) {
		printf("LM75 device not found at addr:%x\n", LM75_I2C_ADDR);
		exit(1);
	}

	/* Create and init LM75 data struct */
	return 0; // Success
}

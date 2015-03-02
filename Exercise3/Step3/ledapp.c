#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

int main(void) {
	int fd = open("/sys/class/leds/led2/brightness", O_WRONLY);
	if (fd == -1) {
		printf("There was an error opening the file!\n");
		return 0;
	}
	
	int i;
	for (i = 0; i < 5; i++) {
		pwrite(fd, "1", 1, 0);
		sleep(1);
		pwrite(fd, "0", 1, 0);
		sleep(1);
	}

	close(fd);

	return 0;
}

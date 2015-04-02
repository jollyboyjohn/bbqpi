// gcc -o gpio-mon gpio-mon.c -lpigpio -lpthread -lrt
#include <stdio.h>
#include <stdlib.h>
#include <pigpio.h>

static const int bcm_gpio = 4;
static volatile int count = 0;
static volatile gpioSample_t data[1000];

void samples(const gpioSample_t *samples, int sample_idx) {
    int i, level, tick;
    
    for (i=0; i<sample_idx; i++) {
	data[count].level = (samples[i].level & (1<<bcm_gpio))>>bcm_gpio;
	data[count].tick = samples[i].tick;
        count++;
    }
}

void gpioCleanup (void) {
    gpioSetGetSamplesFunc(NULL, 1<<bcm_gpio);
    gpioTerminate();
}

void gpioStartup (void) {
    // Sample 5us, binary, N/A
    gpioCfgClock(5, 1, 0);
    // Setup GPIO
    if (gpioInitialise()<0) exit(1);
    gpioSetGetSamplesFunc(samples, 1<<bcm_gpio);
    gpioSetMode(bcm_gpio, PI_INPUT);
}

int main(int argc, char *argv[]) {
    int i;
    gpioStartup();
    gpioDelay(500000);
    for (i=0; i<count; i++) {
	printf("%d, %d, %d\n", i, data[i].level, data[i].tick);
    }
    gpioCleanup();
    exit(0);
}

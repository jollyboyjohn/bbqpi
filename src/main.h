// gcc -o maverick maverick-pig.c -lpigpio -lpthread -lrt
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <pigpio.h>

#ifndef MAIN_H
#define MAIN_H

// BCM GPIO numbering
static const int rf_gpio = 17; // WiringPi:0 - Pin:11
static const int ow_gpio = 4; // WiringPi:7 - Pin:7

#endif

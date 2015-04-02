// gcc -o maverick maverick-pig.c -lpigpio -lpthread -lrt
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <pigpio.h>

#ifndef MAVERICK_H
#define MAVERICK_H

#define PULSEBUF 10000
static const int datagram = 71;
static const int deq[11] = { 0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 3};
static const int chksum[8] = { 2, 0, 0, 2, 2, 2, 3, 3 };
enum { PREAMBLE, DATAREAD, COMPLETE };

void samples(const gpioSample_t *samples, int sample_idx, void *userdata);
int getPreamble(gpioSample_t *l_buf, int *preamble, int *offset);
int readData(gpioSample_t *l_buf, char *realdata, int *l_idx, int *offset);
int outputData(int *probe_one, int *probe_two, char *d);
int getMaverickTemp(int rf_gpio, int *probe_one, int *probe_two);
#endif

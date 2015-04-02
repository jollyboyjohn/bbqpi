#include <stdio.h>
#include <stdlib.h>
#include <linux/types.h>
#include <pigpio.h>

unsigned char ow_reset(int ow_gpio);
unsigned char read_bit(int ow_gpio);
void write_bit(int ow_gpio, char bit);
unsigned char read_byte(int ow_gpio);
void write_byte(int ow_gpio, char val);
double convert_temp(char data[9]);
int getOneWireTemp(int ow_gpio, double *temp);

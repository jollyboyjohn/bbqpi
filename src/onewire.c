#include "onewire.h"

static char crc_table[] = {
    0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
    157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,
    35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,
    190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
    70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,
    219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
    101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
    248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
    140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
    17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
    175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
    50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
    202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,
    87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
    233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
    116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53
};

unsigned char crc_check(char *data, int len) {
    char crc = 0;
    while (len--)
        crc = crc_table[crc ^ *data++];

    return crc;
}

unsigned char ow_reset(int ow_gpio) {
    unsigned char i;
    // Master Tx Reset Pulse (LOW for 480us min.)
    gpioSetMode(ow_gpio, PI_OUTPUT);
    gpioWrite(ow_gpio,0); 
    gpioDelay(500); // Example was 480
    gpioSetMode(ow_gpio, PI_INPUT);

    // Master Rx (LOW between 60-240us, in 480us window)
    gpioDelay(70);
    i = gpioRead(ow_gpio);
    gpioDelay(930); // Going with example - 1 msec could be appropriate

    return(i); 
} 

unsigned char read_bit(int ow_gpio) {
    unsigned char i;

    // Bus master low for 1us to indicate read
    gpioSetMode(ow_gpio, PI_OUTPUT);
    gpioWrite(ow_gpio,0);
    gpioDelay(6); // Example was 1us
    gpioSetMode(ow_gpio, PI_INPUT);

    // Sample at 15us, wait for rest of slot
    gpioDelay(9); // Example was 14us
    i = gpioRead(ow_gpio); 
    gpioDelay(55); // Example was 45us

    return(i);
}

void write_bit(int ow_gpio, char bit) {
    gpioSetMode(ow_gpio, PI_OUTPUT);
    gpioWrite(ow_gpio, 0); // LOW to indicate write
    gpioDelay(6); // Example was 1us
    if (bit) {
	// Release early for 1
        gpioSetMode(ow_gpio, PI_INPUT);
        gpioDelay(54);
    } else {
	// Release late for 0
        gpioDelay(54);
        gpioSetMode(ow_gpio, PI_INPUT);
    }
    // wait for remainder of 60us slot
    gpioDelay(10); // Example was 5us
}

unsigned char read_byte(int ow_gpio) {
    unsigned char i;
    unsigned char value = 0;
    for (i=0;i<8;i++)
    {
        if(read_bit(ow_gpio)) 
            value|=0x01<<i;
    }
    return(value);
}

void write_byte(int ow_gpio, char val) {
    unsigned char i;
    unsigned char temp;
    for (i=0; i<8; i++) // writes byte, one bit at a time
    {
        temp = val>>i; // shifts val right 'i' spaces
        temp &= 0x01; // copy that bit to temp
        write_bit(ow_gpio, temp); // write bit in temp into
    }
}

double convert_temp(char data[9]) {
    double t, h;

    if (!data[7])
        return 0;
    
    if (data[1] == 0)
	t = 1000 * ((int32_t)data[0] >> 1);
    else 
	t = 1000 * (-1 * (int32_t)(0x100-data[0]) >> 1);

    t -= 250;
    h = 1000 * ((int32_t)data[7] - (int32_t)data[6]);
    h /= (int32_t)data[7];
    t += h;
    t /= 1000;

    return t;
}

int getOneWireTemp(int ow_gpio, double *temp) {
    char data[10];
    char crc;
    int i;

    if (ow_reset(ow_gpio) ) {
	return 0;
    } else {
        write_byte(ow_gpio, 0xCC); // Skip ROM (Address all devices)
        write_byte(ow_gpio, 0x44); // Initiate Temp Conversion

    	if ( ow_reset(ow_gpio) ) 
	    return(0);
    	write_byte(ow_gpio, 0xCC); // Skip ROM (Address all devices)
    	write_byte(ow_gpio, 0xBE); // Read the entire scratchpad (inc CRC byte)

    	for (i=0;i<9;i++) {
	    data[i] = read_byte(ow_gpio);
        }
        crc = crc_check(data,8);
	if (data[8] != crc)
	    return(0);
	*temp = convert_temp(data);
        return (1);
    }
}

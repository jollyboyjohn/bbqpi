#include "maverick.h"

static gpioSample_t g_buf[PULSEBUF];
static volatile int g_reset, g_idx;

void samples(const gpioSample_t *samples, int sample_idx, void *userdata) {
    int i, level;
    int *gpio = (int *) userdata;
	
    // This flushes the array and g_resets our counter
    if (g_reset) {
        g_reset = 0;
        g_idx = 0;

        for (i=0; i<PULSEBUF; i++) {
            g_buf[i].level = 0;
            g_buf[i].tick = 0;
        }
    }

    for (i=0; i<sample_idx; i++) {
        // Get bit on GPIO 4 = 0(1)0 0
        // Shift to boolean => 0 0 0(1)
        level = (samples[i].level & (1<<(*gpio)))>>(*gpio);

        // Use a NOR to detect a difference, then add it if there is
        if (g_buf[g_idx-1].level ^ level) {
            g_buf[g_idx].tick = samples[i].tick;
            g_buf[g_idx++].level = level;
        }
    }
}

int getPreamble(gpioSample_t *l_buf, int *preamble, int *offset) {
    int gap, i;
    for (i=0; i < PULSEBUF; i++) {
        gap = l_buf[i].tick - l_buf[i-1].tick;
        if (l_buf[i].level == 1) {
            if ((gap > 4500) && (gap < 5100))
                ++(*preamble);
            else
                *preamble = 0;
        } else if ((gap > 400) && (gap < 600) && (*preamble > 6) ) {
            *offset = i-1;
            *preamble = 0;
            return DATAREAD;
        }
    }
    return PREAMBLE;
}

int readData(gpioSample_t *l_buf, char *realdata, int *l_idx, int *offset) {
    int lasttick, gap, i;
    int j = 0;

    lasttick = l_buf[*offset].tick - 500;
    for (i = *offset; i < *l_idx; i++) {
        gap = l_buf[i].tick - lasttick;
        if ((gap > 400) && (gap < 600) && (j <= datagram)) {
            lasttick = l_buf[i].tick;
            realdata[j++] = l_buf[i].level;
        } else if (gap >= 600)
            return PREAMBLE;

        if (j > datagram)
            return COMPLETE;
    }
    return DATAREAD;
}

int outputData(int *probe_one, int *probe_two, char *d) {
    int i, j, qnib;
    int hex[datagram];

    // Convert bits to nibbles, the data is MSB first, so move backwards
    j = 0;
    for(i=datagram; i>=0; i--) {
	if (i % 4 == 3) {
	    qnib = d[i]<<3;
        } else {
            qnib >>= 1;
	    qnib += d[i]<<3;
	}
	// Once we have 4 bits, translate into quaternary bytes
	if (i % 4 == 0) 
            hex[j++] = deq[qnib];
    }

    if(memcmp(hex + 10, chksum, 8)) {
	fprintf(stderr, "Bad data received\n");
	return PREAMBLE;
    }
    *probe_one = 0;
    *probe_two = 0;
    // Extract both probe results
    for(j=0; j<=datagram; j++) {
	if (j < 5)
	    *probe_two += hex[j]<<(2*(j%5));  
	else if (j < 10)
	    *probe_one += hex[j]<<(2*(j%5));  
    }

    // Deskew the data
    if (*probe_one > 0) {
	*probe_one -= 532;
    } 
	
    if (*probe_two > 0) {
    	*probe_two -= 532;
    }
	
    return PREAMBLE;
}

int getMaverickTemp(int rf_gpio, int *probe_one, int *probe_two) {
    int preamble,l_idx;
    int mode = PREAMBLE;
    int offset; 
    char realdata[datagram];
    gpioSample_t l_buf[PULSEBUF];
    int i = 0;

    g_reset = 1;
 
    while((g_idx < PULSEBUF) && (i < 112)) {
	gpioSetGetSamplesFuncEx(samples, 1<<rf_gpio, &rf_gpio);
	gpioSetMode(rf_gpio, PI_INPUT);
        // wait for some readings
	gpioDelay(100000);

	// work with a local copy
        l_idx = g_idx;
        memcpy(l_buf, g_buf, sizeof(g_buf));

	// look for data preamble, data transmission, process some data
	if (mode == PREAMBLE) 
	    mode = getPreamble(l_buf,&preamble,&offset);
        if (mode == DATAREAD) 
	    mode = readData(l_buf,realdata,&l_idx,&offset);
        if (mode == COMPLETE) {
	    mode = outputData(probe_one, probe_two, realdata);  
    	    gpioSetGetSamplesFuncEx(NULL, 1<<rf_gpio, &rf_gpio);
            return 1;
	}
	i++;
    }
    gpioSetGetSamplesFuncEx(NULL, 1<<rf_gpio, &rf_gpio);
    return 0;
}

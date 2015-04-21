#include "main.h"
#include "maverick.h"
#include "onewire.h"

void createPidFile(pid_t pid) {
    FILE *fd;
    char *pidfile = "/var/run/bbqpi.pid";
    fd = fopen(pidfile, "w+");
    if (!fd) {
	exit(EXIT_FAILURE);
    }
    fprintf(fd, "%d\n", pid);
    fclose(fd);
}

int pollprobes(void) {
    int p_food, p_bbq;
    double p_amb;
    char line[100];
    time_t epoch;
    FILE *fh;

    fh = fopen ("/var/www/bbqpi.csv", "w+");
    fprintf(fh, "Time,Ambient,Food,Barbecue\n");
    fclose(fh);

    while (1) {
        gpioCfgClock(5, 1, 0);
        if (gpioInitialise()<0) exit(EXIT_SUCCESS);

        epoch = time(NULL);
        sprintf(line, "%d000", epoch);

        if (getOneWireTemp(ow_gpio, &p_amb)) 
            sprintf(line + strlen(line), ",%f", p_amb);
	else
            sprintf(line + strlen(line), ",");
	    
        if (getMaverickTemp(rf_gpio, &p_food, &p_bbq)) {
            sprintf(line + strlen(line), ",%d,%d\n", p_food, p_bbq);
            fh = fopen ("/var/www/bbqpi.csv", "a");
            fprintf(fh, line);
            fclose(fh);
	    sleep(11);
	} else {
            sprintf(line + strlen(line), ",,\n");
	    fh = fopen ("/var/www/bbqpi.csv", "a");
            fprintf(fh, line);
            fclose(fh);
	}

    	gpioTerminate();
    }
    return 0;
}

int main(int argc, char *argv[]) {
    FILE *fh;
    // Linux Daemon Writing HOWTO
    pid_t pid, sid;
	
    pid = fork();
    if (pid < 0) { 
	fprintf(stderr, "can't fork process\n");
	exit(EXIT_FAILURE);
    } 
    if (pid > 0) {
	createPidFile(pid);
	exit(EXIT_SUCCESS);
    }
	
    // Full access to the files
    umask(0);

    // Create new session ID
    sid = setsid();
    if (sid < 0) {
	fprintf(stderr, "can't create session id for child\n");
	exit (EXIT_FAILURE);
    }

    // Change the cwd
    if(( chdir("/") ) < 0) {
	exit (EXIT_FAILURE);
    }

    // Close of the standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    pollprobes();
	
    exit(EXIT_SUCCESS);
}

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>

int main(int argc, char *argv[])
{
    struct input_event ev;
    int fdState, fdEvent;

    if( (fdState = open("/sys/power/state", O_WRONLY)) < 0 ) {
        perror("/sys/power/state open fail");
        return 1;
    }
    /* axp20 regulatory events device */
    if( (fdEvent = open("/dev/input/event1", O_RDONLY)) < 0 ) {
        perror("/dev/input/event1 open fail");
        return 1;
    }
    if( write(fdState, "mem", 3) != 3 ) {
        perror("write state");
        return 1;
    }
    while(1) {
        if( read(fdEvent, &ev, sizeof(ev)) != sizeof(ev) ) {
            perror("read event");
            break;
        }
        if( ev.type == EV_KEY && ev.code == KEY_POWER )
            break;
    }
    write(fdState, "on", 2);
    return 0;
}

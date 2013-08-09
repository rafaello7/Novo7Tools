#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>

enum {
    KEYB_MAX = 8
};

int main(int argc, char *argv[])
{
    struct input_event ev;
    int fdState, fdsEv[KEYB_MAX], fdcount = 0, fdMax = -1, i, len;
    char path[40];
    unsigned char key_bitmask[KEY_CNT/8 + 1];
    fd_set fds;

    if( (fdState = open("/sys/power/state", O_WRONLY)) < 0 ) {
        perror("/sys/power/state open fail");
        return 1;
    }
    for(i = 0; fdcount < KEYB_MAX; ++i) {
        sprintf(path, "/dev/input/event%d", i);
        if( (fdsEv[fdcount] = open(path, O_RDONLY)) < 0 ) {
            if( errno == ENOENT )
                break;
            perror(path);
            return 1;
        }
        len = ioctl(fdsEv[fdcount], EVIOCGBIT(EV_KEY, sizeof(key_bitmask)),
                key_bitmask);
        if( len > KEY_POWER / 8 &&
                (key_bitmask[KEY_POWER/8] & (1 << KEY_POWER % 8)) )
        {
            if( fdsEv[fdcount] > fdMax )
                fdMax = fdsEv[fdcount];
            ++fdcount;
        }else
            close(fdsEv[fdcount]);
    }
    if( fdcount == 0 ) {
        fprintf(stderr, "No input Power key capable\n");
        return 1;
    }
    if( write(fdState, "mem", 3) != 3 ) {
        perror("write state");
        return 1;
    }
    FD_ZERO(&fds);
    while(1) {
        for(i = 0; i < fdcount; ++i)
            FD_SET(fdsEv[i], &fds);
        if( select(fdMax+1, &fds, NULL, NULL, NULL) < 0 ) {
            perror("select fail");
            /* go wakeup */
            break;
        }
        for(i = 0; i < fdcount; ++i) {
            if( FD_ISSET(fdsEv[i], &fds) ) {
                if( read(fdsEv[i], &ev, sizeof(ev)) != sizeof(ev) ) {
                    perror("read event");
                    break;
                }
                if( ev.type == EV_KEY && ev.code == KEY_POWER )
                    break;
            }
        }
        if(i < fdcount )
            break;
    }
    write(fdState, "on", 2);
    return 0;
}


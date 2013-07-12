#define _XOPEN_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>


enum {
    A_SYNC = 0x434e5953,
    A_CNXN = 0x4e584e43,
    A_OPEN = 0x4e45504f,
    A_OKAY = 0x59414b4f,
    A_CLSE = 0x45534c43,
    A_WRTE = 0x45545257,
    A_AUTH = 0x48545541
};

struct amessage {
    unsigned command;       /* command identifier constant      */
    unsigned arg0;          /* first argument                   */
    unsigned arg1;          /* second argument                  */
    unsigned data_length;   /* length of payload (0 is allowed) */
    unsigned data_check;    /* checksum of data payload         */
    unsigned magic;         /* command ^ 0xffffffff             */
};

static int gAdbFd = -1;
static int gShellFd = -1;
static unsigned gShellRemoteId;
pthread_t gThread;
pthread_mutex_t gSendMessageMutex = PTHREAD_MUTEX_INITIALIZER;

#define dolog printf

static int usbread(void *buf, int toRead)
{
    char *bpos = buf;

    while( toRead > 0 ) {
        int rd = read(gAdbFd, bpos, toRead);
        if( rd < 0 ) {
            perror("usbread");
            return 0;
        }
        bpos += rd;
        toRead -= rd;
    }
    return 1;
}

static void usbwrite(const void *data, int toWrite)
{
    while( toWrite > 0 ) {
        int wr = write(gAdbFd, data, toWrite);
        if( wr < 0 ) {
            perror("usbwrite");
            return;
        }
        data += wr;
        toWrite -= wr;
    }
}

static void send_packet(struct amessage *msg, const char *data)
{
    int count, sum;
    const unsigned char *x;

    count = msg->data_length;
    x = (const unsigned char*) data;
    sum = 0;
    while(count-- > 0){
        sum += *x++;
    }
    msg->data_check = sum;
    msg->magic = ~msg->command;
    pthread_mutex_lock(&gSendMessageMutex);
    if( gAdbFd >= 0 ) {
        usbwrite(msg, sizeof(struct amessage));
        if( msg->data_length > 0 )
            usbwrite(data, msg->data_length);
    }else{
        dolog("send_packet: usb channel is closed\n");
    }
    pthread_mutex_unlock(&gSendMessageMutex);
}

static void send_cnxn(void)
{
    struct amessage msg;
    static const char connect_data[] = "device::ro.product.name=Debian;"
        "ro.product.model=Novo7Aurora;ro.product.device=Debian;";

    dolog("send_cnxn: send connect_data\n");
    msg.command = 0x4e584e43;
    msg.arg0 = 0x01000000;
    msg.arg1 = 4096;
    msg.data_length = sizeof(connect_data);
    send_packet(&msg, connect_data);
}

static void send_ready(unsigned local, unsigned remote)
{
    struct amessage msg;
    msg.command = A_OKAY;
    msg.arg0 = local;
    msg.arg1 = remote;
    msg.data_length = 0;
    send_packet(&msg, NULL);
}

static void send_close(unsigned local, unsigned remote)
{
    struct amessage msg;
    msg.command = A_CLSE;
    msg.arg0 = local;
    msg.arg1 = remote;
    msg.data_length = 0;
    send_packet(&msg, NULL);
}

static void send_write(unsigned local, unsigned remote,
        const char *data, int datalen)
{
    struct amessage msg;
    msg.command = A_WRTE;
    msg.arg0 = local;
    msg.arg1 = remote;
    msg.data_length = datalen;
    send_packet(&msg, data);
}

static void *ShellReadFun(void *param)
{
    char buf[1024];

    while(1) {
        int rd = read(gShellFd, buf, sizeof(buf));
        if( rd > 0 ) {
            send_write(1234, gShellRemoteId, buf, rd);
        }else{
            dolog("ShellReadFun: rd=%d, errno=%d  exiting.\n", rd, errno);
            send_close(1234, gShellRemoteId);
            return NULL;
        }
    }
}

static void shell_open(unsigned remote)
{
    int ptm, pts;
    char *devname;

    if( gShellFd >= 0 ) {
        fprintf(stderr, "only one shell supported at a time\n");
        send_close(0, remote);
        return;
    }
    if( (ptm = open("/dev/ptmx", O_RDWR)) == -1 ) {
        perror("open(/dev/ptmx)");
        send_close(0, remote);
        return;
    }
    grantpt(ptm);
    unlockpt(ptm);
    fflush(NULL);
    switch( fork() ) {
    case -1:
        perror("shell_open: fork fail");
        close(ptm);
        send_close(0, remote);
        break;
    case 0:
        close(gAdbFd);
        setsid();
        devname = ptsname(ptm);
        pts = open(devname, O_RDWR);
        dup2(pts, fileno(stdin));
        dup2(pts, fileno(stdout));
        dup2(pts, fileno(stderr));
        close(ptm);
        close(pts);
        execlp("/bin/login", "login", NULL);
        exit(1);
    default:
        gShellFd = ptm;
        gShellRemoteId = remote;
        pthread_create(&gThread, NULL, ShellReadFun, NULL);
        send_ready(1234, remote);
        break;
    }
}

static void shell_write(unsigned remote, unsigned local, const char *data,
        unsigned datalen)
{
    if( gShellFd >= 0 ) {
        while( datalen > 0 ) {
            int wr = write(gShellFd, data, datalen);
            if( wr < 0 ) {
                perror("shell_write: write err\n");
                break;
            }
            data += wr;
            datalen -= wr;
        }
        send_ready(local, remote);
    }else{
        fprintf(stderr, "shell_write: not open\n");
        send_close(local, remote);
    }
}

static void shell_close(unsigned remote, unsigned local)
{
    dolog("shell_close: %d, %d\n", remote, local);
    if( gShellFd >= 0 ) {
        pthread_mutex_lock(&gSendMessageMutex);
        pthread_cancel(gThread);
        pthread_mutex_unlock(&gSendMessageMutex);
        pthread_join(gThread, NULL);
        close(gShellFd);
        gShellFd = -1;
    }else{
        fprintf(stderr, "shell_close error: not open\n");
    }
}

static void handle_packet(struct amessage *msg, const char *data)
{
    switch(msg->command){
    case A_SYNC:
        dolog("sync: %d, %d\n", msg->arg0, msg->arg1);
        if(msg->arg0){
            send_packet(msg, data);
        } else {
            send_packet(msg, data);
        }
        break;
    case A_CNXN:
        send_cnxn();
        if( gShellFd >= 0 )
            shell_close(gShellRemoteId, 1234);
        break;
    case A_AUTH:
        dolog("auth - ignore\n");
        break;
    case A_OPEN:
        dolog("open: <%s> arg0=%d\n", data, msg->arg0);
        if( strcmp(data, "shell:") ) {
            send_close(0, msg->arg0);
        } else {
            shell_open(msg->arg0);
        }
        break;
    case A_OKAY:
        break;
    case A_CLSE:
        shell_close(msg->arg0, msg->arg1);
        break;
    case A_WRTE:
        shell_write(msg->arg0, msg->arg1, data, msg->data_length);
        break;
    default:
        fprintf(stderr, "unknown command: %.4s\n", (char*)&msg->command);
        break;
    }
}

int main(int argc, char *argv[])
{
    struct amessage msg;
    char data[4096];

    signal(SIGCHLD, SIG_IGN);
    while(1) {
        if( gShellFd != -1 )
            shell_close(gShellRemoteId, 1234);
        if( gAdbFd != -1 )
            close(gAdbFd);
        if( (gAdbFd = open("/dev/android_adb", O_RDWR)) == -1 ) {
            perror("unable to open /dev/android_adb");
            return 1;
        }
        while(1) {
            /* read packet */
            if( ! usbread((void*)&msg, sizeof(struct amessage)) )
                break;
            if( msg.data_length > 0 ) {
                if( msg.data_length > sizeof(data) ) {
                    perror("data length exceeds buffer size");
                    return 1;
                }
                if( ! usbread(data, msg.data_length) )
                    break;
            }
            handle_packet(&msg, data);
        }
    }
    return 0;
}


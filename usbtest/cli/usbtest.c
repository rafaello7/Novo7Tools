#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <libusb.h>

static libusb_device_handle *usb;
static int do_finish = 0;

static void *ThreadFun(void *par)
{
    char buf[1000];
    int transferred;

    while( libusb_bulk_transfer(usb, 1 | LIBUSB_ENDPOINT_IN, buf,
                sizeof(buf), &transferred, 0) == 0)
    {
        printf("response: %.*s\n", transferred, buf);
    }
    if( ! do_finish )
        printf("read error\n");
    return NULL;
}

int main(int argc, char *argv[])
{
    int transferred;
    char buf[1000];
    pthread_t thr;

    if( libusb_init(NULL) != 0 ) {
        printf("libusb_init fail\n");
        return 1;
    }
    if( (usb = libusb_open_device_with_vid_pid(NULL, 0xbb4, 0xfff)) == NULL ) {
        printf("open device failed\n");
        return 1;
    }
    libusb_claim_interface(usb, 0);
    pthread_create(&thr, NULL, ThreadFun, NULL);
    while( gets(buf) != NULL ) {
        if( libusb_bulk_transfer(usb, 2 | LIBUSB_ENDPOINT_OUT,
                    buf, strlen(buf), &transferred, 0) != 0 )
        {
            printf("write error\n");
            break;
        }
    }
    do_finish = 1;
    libusb_release_interface(usb, 0);
    libusb_close(usb);
    libusb_exit(NULL);
}

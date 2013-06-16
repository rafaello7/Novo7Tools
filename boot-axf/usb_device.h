#ifndef USB_DEVICE_H
#define USB_DEVICE_H

struct usb_devparams;

int usb_start(int);
int usb_run(void);
int usb_detect_exit(void);
int usb_detect_enter(void);

#endif /* USB_DEVICE_H */

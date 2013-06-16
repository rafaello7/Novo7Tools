#ifndef POWER_H
#define POWER_H

void power_set_init(void);
void power_set_usbdc(void);
void power_set_usbpc(void);
int check_power_status(void);
void power_int_rel(void);

#endif /* POWER_H */

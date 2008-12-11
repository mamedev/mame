#ifndef _MICROTOUCH_H
#define _MICROTOUCH_H

INPUT_PORTS_EXTERN(microtouch);

void microtouch_init(running_machine *machine, void (*tx_cb)(UINT8 data), int (*touch_cb)(int *touch_x, int *touch_y));
void microtouch_rx(int count, UINT8* data);

#endif //_MICROTOUCH_H

#ifndef _MICROTOUCH_H
#define _MICROTOUCH_H

INPUT_PORTS_EXTERN(microtouch);

typedef void (*microtouch_tx_func)(running_machine *machine, UINT8 data);
typedef int (*microtouch_touch_func)(running_machine *machine, int *touch_x, int *touch_y);

void microtouch_init(running_machine *machine, microtouch_tx_func tx_cb, microtouch_touch_func touch_cb);
void microtouch_rx(int count, UINT8* data);

#endif //_MICROTOUCH_H

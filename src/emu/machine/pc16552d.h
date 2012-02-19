#ifndef PC16552D_H
#define PC16552D_H

void pc16552d_init(running_machine &machine, int chip, int frequency, void (* irq_handler)(running_machine &machine, int channel, int value), void (* tx_callback)(running_machine &machine, int channel, int count, UINT8* data));
void pc16552d_rx_data(running_machine &machine, int chip, int channel, UINT8 data);

READ8_HANDLER(pc16552d_0_r);
WRITE8_HANDLER(pc16552d_0_w);
READ8_HANDLER(pc16552d_1_r);
WRITE8_HANDLER(pc16552d_1_w);

#endif

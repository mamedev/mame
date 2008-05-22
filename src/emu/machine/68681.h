#ifndef _68681_H
#define _68681_H

void duart_68681_init(int frequency, void (*irq_handler)(running_machine *machine, int vector), void (*tx_callback)(int channel, UINT8 data));
void duart_68681_rx_data( running_machine *machine, int ch, UINT8 data );

READ16_HANDLER(duart_68681_r);
WRITE16_HANDLER(duart_68681_w);

#endif //_68681_H

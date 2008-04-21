#ifndef _68681_H
#define _68681_H

void duart_68681_init(int frequency, void (*irq_handler)(UINT8 vector), void (*tx_callback)(int channel, UINT8 data));
void duart_68681_rx_data( int ch, UINT8 data );

READ16_HANDLER(duart_68681_r);
WRITE16_HANDLER(duart_68681_w);

#endif //_68681_H

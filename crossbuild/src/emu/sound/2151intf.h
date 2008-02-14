#ifndef YM2151INTF_H
#define YM2151INTF_H

struct YM2151interface
{
	void (*irqhandler)(int irq);
	write8_handler portwritehandler;
};

READ8_HANDLER( YM2151_status_port_0_r );
READ8_HANDLER( YM2151_status_port_1_r );
READ8_HANDLER( YM2151_status_port_2_r );

WRITE8_HANDLER( YM2151_register_port_0_w );
WRITE8_HANDLER( YM2151_register_port_1_w );
WRITE8_HANDLER( YM2151_register_port_2_w );

WRITE8_HANDLER( YM2151_data_port_0_w );
WRITE8_HANDLER( YM2151_data_port_1_w );
WRITE8_HANDLER( YM2151_data_port_2_w );

WRITE8_HANDLER( YM2151_word_0_w );
WRITE8_HANDLER( YM2151_word_1_w );

READ16_HANDLER( YM2151_status_port_0_lsb_r );
READ16_HANDLER( YM2151_status_port_1_lsb_r );
READ16_HANDLER( YM2151_status_port_2_lsb_r );

WRITE16_HANDLER( YM2151_register_port_0_lsb_w );
WRITE16_HANDLER( YM2151_register_port_1_lsb_w );
WRITE16_HANDLER( YM2151_register_port_2_lsb_w );

WRITE16_HANDLER( YM2151_data_port_0_lsb_w );
WRITE16_HANDLER( YM2151_data_port_1_lsb_w );
WRITE16_HANDLER( YM2151_data_port_2_lsb_w );

#endif

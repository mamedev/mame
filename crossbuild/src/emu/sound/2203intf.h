#ifndef YM2203INTF_H
#define YM2203INTF_H

#include "ay8910.h"

struct YM2203interface
{
	read8_handler portAread;
	read8_handler portBread;
	write8_handler portAwrite;
	write8_handler portBwrite;
	void (*handler)(int irq);
};


READ8_HANDLER( YM2203_status_port_0_r );
READ8_HANDLER( YM2203_status_port_1_r );
READ8_HANDLER( YM2203_status_port_2_r );
READ8_HANDLER( YM2203_status_port_3_r );
READ8_HANDLER( YM2203_status_port_4_r );

READ8_HANDLER( YM2203_read_port_0_r );
READ8_HANDLER( YM2203_read_port_1_r );
READ8_HANDLER( YM2203_read_port_2_r );
READ8_HANDLER( YM2203_read_port_3_r );
READ8_HANDLER( YM2203_read_port_4_r );

WRITE8_HANDLER( YM2203_control_port_0_w );
WRITE8_HANDLER( YM2203_control_port_1_w );
WRITE8_HANDLER( YM2203_control_port_2_w );
WRITE8_HANDLER( YM2203_control_port_3_w );
WRITE8_HANDLER( YM2203_control_port_4_w );

WRITE8_HANDLER( YM2203_write_port_0_w );
WRITE8_HANDLER( YM2203_write_port_1_w );
WRITE8_HANDLER( YM2203_write_port_2_w );
WRITE8_HANDLER( YM2203_write_port_3_w );
WRITE8_HANDLER( YM2203_write_port_4_w );

READ16_HANDLER( YM2203_status_port_0_lsb_r );
READ16_HANDLER( YM2203_status_port_1_lsb_r );
READ16_HANDLER( YM2203_status_port_2_lsb_r );
READ16_HANDLER( YM2203_status_port_3_lsb_r );
READ16_HANDLER( YM2203_status_port_4_lsb_r );

READ16_HANDLER( YM2203_read_port_0_lsb_r );
READ16_HANDLER( YM2203_read_port_1_lsb_r );
READ16_HANDLER( YM2203_read_port_2_lsb_r );
READ16_HANDLER( YM2203_read_port_3_lsb_r );
READ16_HANDLER( YM2203_read_port_4_lsb_r );

WRITE16_HANDLER( YM2203_control_port_0_lsb_w );
WRITE16_HANDLER( YM2203_control_port_1_lsb_w );
WRITE16_HANDLER( YM2203_control_port_2_lsb_w );
WRITE16_HANDLER( YM2203_control_port_3_lsb_w );
WRITE16_HANDLER( YM2203_control_port_4_lsb_w );

WRITE16_HANDLER( YM2203_write_port_0_lsb_w );
WRITE16_HANDLER( YM2203_write_port_1_lsb_w );
WRITE16_HANDLER( YM2203_write_port_2_lsb_w );
WRITE16_HANDLER( YM2203_write_port_3_lsb_w );
WRITE16_HANDLER( YM2203_write_port_4_lsb_w );

WRITE8_HANDLER( YM2203_word_0_w );
WRITE8_HANDLER( YM2203_word_1_w );

#endif

#pragma once

#ifndef __2203INTF_H__
#define __2203INTF_H__

#include "ay8910.h"

void ym2203_update_request(void *param);

typedef struct _ym2203_interface ym2203_interface;
struct _ym2203_interface
{
	const ay8910_interface ay8910_intf;
	void (*handler)(running_machine *machine, int irq);
};


READ8_HANDLER( ym2203_status_port_0_r );
READ8_HANDLER( ym2203_status_port_1_r );
READ8_HANDLER( ym2203_status_port_2_r );
READ8_HANDLER( ym2203_status_port_3_r );
READ8_HANDLER( ym2203_status_port_4_r );

READ8_HANDLER( ym2203_read_port_0_r );
READ8_HANDLER( ym2203_read_port_1_r );
READ8_HANDLER( ym2203_read_port_2_r );
READ8_HANDLER( ym2203_read_port_3_r );
READ8_HANDLER( ym2203_read_port_4_r );

WRITE8_HANDLER( ym2203_control_port_0_w );
WRITE8_HANDLER( ym2203_control_port_1_w );
WRITE8_HANDLER( ym2203_control_port_2_w );
WRITE8_HANDLER( ym2203_control_port_3_w );
WRITE8_HANDLER( ym2203_control_port_4_w );

WRITE8_HANDLER( ym2203_write_port_0_w );
WRITE8_HANDLER( ym2203_write_port_1_w );
WRITE8_HANDLER( ym2203_write_port_2_w );
WRITE8_HANDLER( ym2203_write_port_3_w );
WRITE8_HANDLER( ym2203_write_port_4_w );

READ16_HANDLER( ym2203_status_port_0_lsb_r );
READ16_HANDLER( ym2203_status_port_1_lsb_r );
READ16_HANDLER( ym2203_status_port_2_lsb_r );
READ16_HANDLER( ym2203_status_port_3_lsb_r );
READ16_HANDLER( ym2203_status_port_4_lsb_r );

READ16_HANDLER( ym2203_read_port_0_lsb_r );
READ16_HANDLER( ym2203_read_port_1_lsb_r );
READ16_HANDLER( ym2203_read_port_2_lsb_r );
READ16_HANDLER( ym2203_read_port_3_lsb_r );
READ16_HANDLER( ym2203_read_port_4_lsb_r );

WRITE16_HANDLER( ym2203_control_port_0_lsb_w );
WRITE16_HANDLER( ym2203_control_port_1_lsb_w );
WRITE16_HANDLER( ym2203_control_port_2_lsb_w );
WRITE16_HANDLER( ym2203_control_port_3_lsb_w );
WRITE16_HANDLER( ym2203_control_port_4_lsb_w );

WRITE16_HANDLER( ym2203_write_port_0_lsb_w );
WRITE16_HANDLER( ym2203_write_port_1_lsb_w );
WRITE16_HANDLER( ym2203_write_port_2_lsb_w );
WRITE16_HANDLER( ym2203_write_port_3_lsb_w );
WRITE16_HANDLER( ym2203_write_port_4_lsb_w );

WRITE8_HANDLER( ym2203_word_0_w );
WRITE8_HANDLER( ym2203_word_1_w );

SND_GET_INFO( ym2203 );

#endif /* __2203INTF_H__ */

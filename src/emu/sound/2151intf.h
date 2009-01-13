#pragma once

#ifndef __2151INTF_H__
#define __2151INTF_H__

typedef struct _ym2151_interface ym2151_interface;
struct _ym2151_interface
{
	void (*irqhandler)(running_machine *machine, int irq);
	write8_space_func portwritehandler;
};

READ8_HANDLER( ym2151_status_port_0_r );
READ8_HANDLER( ym2151_status_port_1_r );
READ8_HANDLER( ym2151_status_port_2_r );

WRITE8_HANDLER( ym2151_register_port_0_w );
WRITE8_HANDLER( ym2151_register_port_1_w );
WRITE8_HANDLER( ym2151_register_port_2_w );

WRITE8_HANDLER( ym2151_data_port_0_w );
WRITE8_HANDLER( ym2151_data_port_1_w );
WRITE8_HANDLER( ym2151_data_port_2_w );

WRITE8_HANDLER( ym2151_word_0_w );
WRITE8_HANDLER( ym2151_word_1_w );

READ16_HANDLER( ym2151_status_port_0_lsb_r );
READ16_HANDLER( ym2151_status_port_1_lsb_r );
READ16_HANDLER( ym2151_status_port_2_lsb_r );

WRITE16_HANDLER( ym2151_register_port_0_lsb_w );
WRITE16_HANDLER( ym2151_register_port_1_lsb_w );
WRITE16_HANDLER( ym2151_register_port_2_lsb_w );

WRITE16_HANDLER( ym2151_data_port_0_lsb_w );
WRITE16_HANDLER( ym2151_data_port_1_lsb_w );
WRITE16_HANDLER( ym2151_data_port_2_lsb_w );

SND_GET_INFO( ym2151 );
#define SOUND_YM2151 SND_GET_INFO_NAME( ym2151 )

#endif /* __2151INTF_H__ */



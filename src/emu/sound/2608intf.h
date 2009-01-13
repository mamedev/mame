#pragma once

#ifndef __2608INTF_H__
#define __2608INTF_H__

#include "fm.h"
#include "ay8910.h"

void ym2608_update_request(void *param);

typedef struct _ym2608_interface ym2608_interface;
struct _ym2608_interface
{
	const ay8910_interface ay8910_intf;
	void ( *handler )( running_machine *machine, int irq );	/* IRQ handler for the YM2608 */
};

/************************************************/
/* Chip 0 functions             */
/************************************************/
READ8_HANDLER( ym2608_status_port_0_a_r );
READ8_HANDLER( ym2608_status_port_0_b_r );
READ8_HANDLER( ym2608_read_port_0_r );
WRITE8_HANDLER( ym2608_control_port_0_a_w );
WRITE8_HANDLER( ym2608_control_port_0_b_w );
WRITE8_HANDLER( ym2608_data_port_0_a_w );
WRITE8_HANDLER( ym2608_data_port_0_b_w );

/************************************************/
/* Chip 1 functions             */
/************************************************/
READ8_HANDLER( ym2608_status_port_1_a_r );
READ8_HANDLER( ym2608_status_port_1_b_r );
READ8_HANDLER( ym2608_read_port_1_r );
WRITE8_HANDLER( ym2608_control_port_1_a_w );
WRITE8_HANDLER( ym2608_control_port_1_b_w );
WRITE8_HANDLER( ym2608_data_port_1_a_w );
WRITE8_HANDLER( ym2608_data_port_1_b_w );

SND_GET_INFO( ym2608 );
#define SOUND_YM2608 SND_GET_INFO_NAME( ym2608 )

#endif /* __2608INTF_H__ */

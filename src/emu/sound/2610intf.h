#pragma once

#ifndef __2610INTF_H__
#define __2610INTF_H__

#include "fm.h"


typedef struct _ym2610_interface ym2610_interface;
struct _ym2610_interface
{
	void ( *handler )( running_machine *machine, int irq );	/* IRQ handler for the YM2610 */
};

/************************************************/
/* Chip 0 functions                             */
/************************************************/
READ8_HANDLER( ym2610_status_port_0_a_r );
READ16_HANDLER( ym2610_status_port_0_a_lsb_r );
READ8_HANDLER( ym2610_status_port_0_b_r );
READ16_HANDLER( ym2610_status_port_0_b_lsb_r );
READ8_HANDLER( ym2610_read_port_0_r );
READ16_HANDLER( ym2610_read_port_0_lsb_r );
WRITE8_HANDLER( ym2610_control_port_0_a_w );
WRITE16_HANDLER( ym2610_control_port_0_a_lsb_w );
WRITE8_HANDLER( ym2610_control_port_0_b_w );
WRITE16_HANDLER( ym2610_control_port_0_b_lsb_w );
WRITE8_HANDLER( ym2610_data_port_0_a_w );
WRITE16_HANDLER( ym2610_data_port_0_a_lsb_w );
WRITE8_HANDLER( ym2610_data_port_0_b_w );
WRITE16_HANDLER( ym2610_data_port_0_b_lsb_w );

/************************************************/
/* Chip 1 functions                             */
/************************************************/
READ8_HANDLER( ym2610_status_port_1_a_r );
READ16_HANDLER( ym2610_status_port_1_a_lsb_r );
READ8_HANDLER( ym2610_status_port_1_b_r );
READ16_HANDLER( ym2610_status_port_1_b_lsb_r );
READ8_HANDLER( ym2610_read_port_1_r );
READ16_HANDLER( ym2610_read_port_1_lsb_r );
WRITE8_HANDLER( ym2610_control_port_1_a_w );
WRITE16_HANDLER( ym2610_control_port_1_a_lsb_w );
WRITE8_HANDLER( ym2610_control_port_1_b_w );
WRITE16_HANDLER( ym2610_control_port_1_b_lsb_w );
WRITE8_HANDLER( ym2610_data_port_1_a_w );
WRITE16_HANDLER( ym2610_data_port_1_a_lsb_w );
WRITE8_HANDLER( ym2610_data_port_1_b_w );
WRITE16_HANDLER( ym2610_data_port_1_b_lsb_w );

#endif /* __2610INTF_H__ */

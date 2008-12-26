#pragma once

#ifndef __YMF278B_H__
#define __YMF278B_H__

#define YMF278B_STD_CLOCK (33868800)			/* standard clock for OPL4 */


typedef struct _ymf278b_interface ymf278b_interface;
struct _ymf278b_interface
{
	void (*irq_callback)(running_machine *machine, int state);	/* irq callback */
};

READ8_HANDLER( ymf278b_status_port_0_r );
READ8_HANDLER( ymf278b_data_port_0_r );
WRITE8_HANDLER( ymf278b_control_port_0_a_w );
WRITE8_HANDLER( ymf278b_data_port_0_a_w );
WRITE8_HANDLER( ymf278b_control_port_0_b_w );
WRITE8_HANDLER( ymf278b_data_port_0_b_w );
WRITE8_HANDLER( ymf278b_control_port_0_c_w );
WRITE8_HANDLER( ymf278b_data_port_0_c_w );

READ8_HANDLER( ymf278b_status_port_1_r );
READ8_HANDLER( ymf278b_data_port_1_r );
WRITE8_HANDLER( ymf278b_control_port_1_a_w );
WRITE8_HANDLER( ymf278b_data_port_1_a_w );
WRITE8_HANDLER( ymf278b_control_port_1_b_w );
WRITE8_HANDLER( ymf278b_data_port_1_b_w );
WRITE8_HANDLER( ymf278b_control_port_1_c_w );
WRITE8_HANDLER( ymf278b_data_port_1_c_w );

SND_GET_INFO( ymf278b );

#endif /* __YMF278B_H__ */

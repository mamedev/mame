/**********************************************************************************************
 *
 *   Yamaha YMZ280B driver
 *   by Aaron Giles
 *
 **********************************************************************************************/

#pragma once

#ifndef __YMZ280B_H__
#define __YMZ280B_H__


typedef struct _ymz280b_interface ymz280b_interface;
struct _ymz280b_interface
{
	void (*irq_callback)(running_machine *machine, int state);	/* irq callback */
	read8_machine_func ext_read;			/* external RAM read */
	write8_machine_func ext_write;		/* external RAM write */
};

READ8_HANDLER ( ymz280b_status_0_r );
WRITE8_HANDLER( ymz280b_register_0_w );
READ8_HANDLER( ymz280b_data_0_r );
WRITE8_HANDLER( ymz280b_data_0_w );

READ16_HANDLER ( ymz280b_status_0_lsb_r );
READ16_HANDLER ( ymz280b_status_0_msb_r );
WRITE16_HANDLER( ymz280b_register_0_lsb_w );
WRITE16_HANDLER( ymz280b_register_0_msb_w );
WRITE16_HANDLER( ymz280b_data_0_lsb_w );
WRITE16_HANDLER( ymz280b_data_0_msb_w );

READ8_HANDLER ( ymz280b_status_1_r );
WRITE8_HANDLER( ymz280b_register_1_w );
READ8_HANDLER( ymz280b_data_1_r );
WRITE8_HANDLER( ymz280b_data_1_w );

READ16_HANDLER ( ymz280b_status_1_lsb_r );
READ16_HANDLER ( ymz280b_status_1_msb_r );
WRITE16_HANDLER( ymz280b_register_1_lsb_w );
WRITE16_HANDLER( ymz280b_register_1_msb_w );
WRITE16_HANDLER( ymz280b_data_1_lsb_w );
WRITE16_HANDLER( ymz280b_data_1_msb_w );

#endif /* __YMZ280B_H__ */

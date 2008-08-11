#pragma once
	
#ifndef __262INTF_H__
#define __262INTF_H__


typedef struct _ymf262_interface ymf262_interface;
struct _ymf262_interface
{
	void (*handler)(running_machine *machine, int irq);
};



/* YMF262 */
READ8_HANDLER ( ymf262_status_0_r );
WRITE8_HANDLER( ymf262_register_a_0_w );
WRITE8_HANDLER( ymf262_register_b_0_w );
WRITE8_HANDLER( ymf262_data_a_0_w );
WRITE8_HANDLER( ymf262_data_b_0_w );


READ8_HANDLER ( ymf262_status_1_r );
WRITE8_HANDLER( ymf262_register_a_1_w );
WRITE8_HANDLER( ymf262_register_b_1_w );
WRITE8_HANDLER( ymf262_data_a_1_w );
WRITE8_HANDLER( ymf262_data_b_1_w );


#endif /* __262INTF_H__ */

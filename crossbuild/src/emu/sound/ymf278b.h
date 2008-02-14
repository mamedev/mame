#ifndef __YMF278B_H__
#define __YMF278B_H__

#define YMF278B_STD_CLOCK (33868800)			/* standard clock for OPL4 */

struct YMF278B_interface {
	int region;			/* memory region of sample ROMs */
	void (*irq_callback)(int state);	/* irq callback */
};

READ8_HANDLER( YMF278B_status_port_0_r );
READ8_HANDLER( YMF278B_data_port_0_r );
WRITE8_HANDLER( YMF278B_control_port_0_A_w );
WRITE8_HANDLER( YMF278B_data_port_0_A_w );
WRITE8_HANDLER( YMF278B_control_port_0_B_w );
WRITE8_HANDLER( YMF278B_data_port_0_B_w );
WRITE8_HANDLER( YMF278B_control_port_0_C_w );
WRITE8_HANDLER( YMF278B_data_port_0_C_w );

READ8_HANDLER( YMF278B_status_port_1_r );
READ8_HANDLER( YMF278B_data_port_1_r );
WRITE8_HANDLER( YMF278B_control_port_1_A_w );
WRITE8_HANDLER( YMF278B_data_port_1_A_w );
WRITE8_HANDLER( YMF278B_control_port_1_B_w );
WRITE8_HANDLER( YMF278B_data_port_1_B_w );
WRITE8_HANDLER( YMF278B_control_port_1_C_w );
WRITE8_HANDLER( YMF278B_data_port_1_C_w );

#endif

#ifndef YMF262INTF_H
#define YMF262INTF_H


struct YMF262interface
{
	void (*handler)(int irq);
};



/* YMF262 */
READ8_HANDLER ( YMF262_status_0_r );
WRITE8_HANDLER( YMF262_register_A_0_w );
WRITE8_HANDLER( YMF262_register_B_0_w );
WRITE8_HANDLER( YMF262_data_A_0_w );
WRITE8_HANDLER( YMF262_data_B_0_w );


READ8_HANDLER ( YMF262_status_1_r );
WRITE8_HANDLER( YMF262_register_A_1_w );
WRITE8_HANDLER( YMF262_register_B_1_w );
WRITE8_HANDLER( YMF262_data_A_1_w );
WRITE8_HANDLER( YMF262_data_B_1_w );


#endif

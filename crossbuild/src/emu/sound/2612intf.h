#ifndef __2612INTF_H__
#define __2612INTF_H__

struct YM2612interface
{
	void (*handler)(int irq);
};


/************************************************/
/* Chip 0 functions                             */
/************************************************/
READ8_HANDLER( YM2612_status_port_0_A_r );  /* A=0 : OPN status */
READ8_HANDLER( YM2612_status_port_0_B_r );  /* A=2 : don't care */
READ8_HANDLER( YM2612_read_port_0_r );        /* A=1 : don't care */
WRITE8_HANDLER( YM2612_control_port_0_A_w ); /* A=0:OPN  address */
WRITE8_HANDLER( YM2612_control_port_0_B_w ); /* A=2:OPN2 address */
WRITE8_HANDLER( YM2612_data_port_0_A_w );    /* A=1:OPN  data    */
WRITE8_HANDLER( YM2612_data_port_0_B_w );    /* A=3:OPN2 data    */

/************************************************/
/* Chip 1 functions                             */
/************************************************/
READ8_HANDLER( YM2612_status_port_1_A_r );
READ8_HANDLER( YM2612_status_port_1_B_r );
READ8_HANDLER( YM2612_read_port_1_r );
WRITE8_HANDLER( YM2612_control_port_1_A_w );
WRITE8_HANDLER( YM2612_control_port_1_B_w );
WRITE8_HANDLER( YM2612_data_port_1_A_w );
WRITE8_HANDLER( YM2612_data_port_1_B_w );



struct YM3438interface
{
	void (*handler)(int irq);
};


/************************************************/
/* Chip 0 functions                             */
/************************************************/
READ8_HANDLER( YM3438_status_port_0_A_r );  /* A=0 : OPN status */
READ8_HANDLER( YM3438_status_port_0_B_r );  /* A=2 : don't care */
READ8_HANDLER( YM3438_read_port_0_r );        /* A=1 : don't care */
WRITE8_HANDLER( YM3438_control_port_0_A_w ); /* A=0:OPN  address */
WRITE8_HANDLER( YM3438_control_port_0_B_w ); /* A=2:OPN2 address */
WRITE8_HANDLER( YM3438_data_port_0_A_w );    /* A=1:OPN  data    */
WRITE8_HANDLER( YM3438_data_port_0_B_w );    /* A=3:OPN2 data    */

/************************************************/
/* Chip 1 functions                             */
/************************************************/
READ8_HANDLER( YM3438_status_port_1_A_r );
READ8_HANDLER( YM3438_status_port_1_B_r );
READ8_HANDLER( YM3438_read_port_1_r );
WRITE8_HANDLER( YM3438_control_port_1_A_w );
WRITE8_HANDLER( YM3438_control_port_1_B_w );
WRITE8_HANDLER( YM3438_data_port_1_A_w );
WRITE8_HANDLER( YM3438_data_port_1_B_w );


#endif
/**************** end of file ****************/

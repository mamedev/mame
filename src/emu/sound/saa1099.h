#ifndef saa1099_h
#define saa1099_h

/**********************************************
    Philips SAA1099 Sound driver
**********************************************/

WRITE8_HANDLER( saa1099_control_port_0_w );
WRITE8_HANDLER( saa1099_write_port_0_w );
WRITE8_HANDLER( saa1099_control_port_1_w );
WRITE8_HANDLER( saa1099_write_port_1_w );

WRITE16_HANDLER( saa1099_control_port_0_lsb_w );
WRITE16_HANDLER( saa1099_write_port_0_lsb_w );
WRITE16_HANDLER( saa1099_control_port_1_lsb_w );
WRITE16_HANDLER( saa1099_write_port_1_lsb_w );

#endif

/******************************************************************************

    TMP68301 support (dummy) driver

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2000/12/23 -

******************************************************************************/

READ16_HANDLER( tmp68301_address_decoder_r );
WRITE16_HANDLER( tmp68301_address_decoder_w );
READ16_HANDLER( tmp68301_interrupt_controller_r );
WRITE16_HANDLER( tmp68301_interrupt_controller_w );
READ16_HANDLER( tmp68301_parallel_interface_r );
WRITE16_HANDLER( tmp68301_parallel_interface_w );
READ16_HANDLER( tmp68301_serial_interface_r );
WRITE16_HANDLER( tmp68301_serial_interface_w );
READ16_HANDLER( tmp68301_timer_r );
WRITE16_HANDLER( tmp68301_timer_w );

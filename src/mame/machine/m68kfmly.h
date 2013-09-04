/******************************************************************************

    TMP68301 support (dummy) driver

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2000/12/23 -

******************************************************************************/

ATTR_DEPRECATED DECLARE_READ16_HANDLER( tmp68301_address_decoder_r );
ATTR_DEPRECATED DECLARE_WRITE16_HANDLER( tmp68301_address_decoder_w );
ATTR_DEPRECATED DECLARE_READ16_HANDLER( tmp68301_interrupt_controller_r );
ATTR_DEPRECATED DECLARE_WRITE16_HANDLER( tmp68301_interrupt_controller_w );
ATTR_DEPRECATED DECLARE_READ16_HANDLER( tmp68301_parallel_interface_r );
ATTR_DEPRECATED DECLARE_WRITE16_HANDLER( tmp68301_parallel_interface_w );
ATTR_DEPRECATED DECLARE_READ16_HANDLER( tmp68301_serial_interface_r );
ATTR_DEPRECATED DECLARE_WRITE16_HANDLER( tmp68301_serial_interface_w );
ATTR_DEPRECATED DECLARE_READ16_HANDLER( tmp68301_timer_r );
ATTR_DEPRECATED DECLARE_WRITE16_HANDLER( tmp68301_timer_w );

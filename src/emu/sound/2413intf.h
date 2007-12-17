#ifndef YM2413INTF_H
#define YM2413INTF_H


WRITE8_HANDLER( YM2413_register_port_0_w );
WRITE8_HANDLER( YM2413_register_port_1_w );
WRITE8_HANDLER( YM2413_register_port_2_w );
WRITE8_HANDLER( YM2413_register_port_3_w );
WRITE8_HANDLER( YM2413_data_port_0_w );
WRITE8_HANDLER( YM2413_data_port_1_w );
WRITE8_HANDLER( YM2413_data_port_2_w );
WRITE8_HANDLER( YM2413_data_port_3_w );

WRITE16_HANDLER( YM2413_register_port_0_lsb_w );
WRITE16_HANDLER( YM2413_register_port_0_msb_w );
WRITE16_HANDLER( YM2413_register_port_1_lsb_w );
WRITE16_HANDLER( YM2413_register_port_2_lsb_w );
WRITE16_HANDLER( YM2413_register_port_3_lsb_w );
WRITE16_HANDLER( YM2413_data_port_0_lsb_w );
WRITE16_HANDLER( YM2413_data_port_0_msb_w );
WRITE16_HANDLER( YM2413_data_port_1_lsb_w );
WRITE16_HANDLER( YM2413_data_port_2_lsb_w );
WRITE16_HANDLER( YM2413_data_port_3_lsb_w );

#endif


#pragma once

#ifndef __2413INTF_H__
#define __2413INTF_H__


WRITE8_HANDLER( ym2413_register_port_0_w );
WRITE8_HANDLER( ym2413_register_port_1_w );
WRITE8_HANDLER( ym2413_register_port_2_w );
WRITE8_HANDLER( ym2413_register_port_3_w );
WRITE8_HANDLER( ym2413_data_port_0_w );
WRITE8_HANDLER( ym2413_data_port_1_w );
WRITE8_HANDLER( ym2413_data_port_2_w );
WRITE8_HANDLER( ym2413_data_port_3_w );

WRITE16_HANDLER( ym2413_register_port_0_lsb_w );
WRITE16_HANDLER( ym2413_register_port_0_msb_w );
WRITE16_HANDLER( ym2413_register_port_1_lsb_w );
WRITE16_HANDLER( ym2413_register_port_2_lsb_w );
WRITE16_HANDLER( ym2413_register_port_3_lsb_w );
WRITE16_HANDLER( ym2413_data_port_0_lsb_w );
WRITE16_HANDLER( ym2413_data_port_0_msb_w );
WRITE16_HANDLER( ym2413_data_port_1_lsb_w );
WRITE16_HANDLER( ym2413_data_port_2_lsb_w );
WRITE16_HANDLER( ym2413_data_port_3_lsb_w );

SND_GET_INFO( ym2413 );

#endif /* __2413INTF_H__ */

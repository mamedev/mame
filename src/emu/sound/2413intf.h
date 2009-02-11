#pragma once

#ifndef __2413INTF_H__
#define __2413INTF_H__

WRITE8_DEVICE_HANDLER( ym2413_w );

WRITE8_DEVICE_HANDLER( ym2413_register_port_w );
WRITE8_DEVICE_HANDLER( ym2413_data_port_w );

DEVICE_GET_INFO( ym2413 );
#define SOUND_YM2413 DEVICE_GET_INFO_NAME( ym2413 )

#endif /* __2413INTF_H__ */

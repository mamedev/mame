#pragma once

#ifndef __2413INTF_H__
#define __2413INTF_H__

#include "devlegcy.h"

WRITE8_DEVICE_HANDLER( ym2413_w );

WRITE8_DEVICE_HANDLER( ym2413_register_port_w );
WRITE8_DEVICE_HANDLER( ym2413_data_port_w );

DECLARE_LEGACY_SOUND_DEVICE(YM2413, ym2413);

#endif /* __2413INTF_H__ */

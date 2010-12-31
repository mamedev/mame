#pragma once

#ifndef __2608INTF_H__
#define __2608INTF_H__

#include "devlegcy.h"

#include "fm.h"
#include "ay8910.h"

void ym2608_update_request(void *param);

typedef struct _ym2608_interface ym2608_interface;
struct _ym2608_interface
{
	const ay8910_interface ay8910_intf;
	void ( *handler )( device_t *device, int irq );	/* IRQ handler for the YM2608 */
};

READ8_DEVICE_HANDLER( ym2608_r );
WRITE8_DEVICE_HANDLER( ym2608_w );

READ8_DEVICE_HANDLER( ym2608_read_port_r );
READ8_DEVICE_HANDLER( ym2608_status_port_a_r );
READ8_DEVICE_HANDLER( ym2608_status_port_b_r );

WRITE8_DEVICE_HANDLER( ym2608_control_port_a_w );
WRITE8_DEVICE_HANDLER( ym2608_control_port_b_w );
WRITE8_DEVICE_HANDLER( ym2608_data_port_a_w );
WRITE8_DEVICE_HANDLER( ym2608_data_port_b_w );

DECLARE_LEGACY_SOUND_DEVICE(YM2608, ym2608);

#endif /* __2608INTF_H__ */

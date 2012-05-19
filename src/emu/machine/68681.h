#ifndef _68681_H
#define _68681_H

#include "devlegcy.h"

typedef struct _duart68681_config duart68681_config;
struct _duart68681_config
{
	void (*irq_handler)(device_t *device, int state, UINT8 vector);
	void (*tx_callback)(device_t *device, int channel, UINT8 data);
	UINT8 (*input_port_read)(device_t *device);
	void (*output_port_write)(device_t *device, UINT8 data);

	/* clocks for external baud rates */
	INT32 ip3clk, ip4clk, ip5clk, ip6clk;
};

DECLARE_LEGACY_DEVICE(DUART68681, duart68681);

#define MCFG_DUART68681_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, DUART68681, _clock) \
	MCFG_DEVICE_CONFIG(_config)


READ8_DEVICE_HANDLER(duart68681_r);
WRITE8_DEVICE_HANDLER(duart68681_w);

void duart68681_rx_data( device_t* device, int ch, UINT8 data );

#endif //_68681_H

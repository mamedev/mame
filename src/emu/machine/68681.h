#ifndef _68681_H
#define _68681_H

typedef struct _duart68681_config duart68681_config;
struct _duart68681_config
{
	void (*irq_handler)(running_device *device, UINT8 vector);
	void (*tx_callback)(running_device *device, int channel, UINT8 data);
	UINT8 (*input_port_read)(running_device *device);
	void (*output_port_write)(running_device *device, UINT8 data);

	/* clocks for external baud rates */
	INT32 ip3clk, ip4clk, ip5clk, ip6clk;
};

#define DUART68681 DEVICE_GET_INFO_NAME(duart68681)
DEVICE_GET_INFO(duart68681);

#define MDRV_DUART68681_ADD(_tag, _clock, _config) \
	MDRV_DEVICE_ADD(_tag, DUART68681, _clock) \
	MDRV_DEVICE_CONFIG(_config)


READ8_DEVICE_HANDLER(duart68681_r);
WRITE8_DEVICE_HANDLER(duart68681_w);

void duart68681_rx_data( running_device* device, int ch, UINT8 data );

#endif //_68681_H

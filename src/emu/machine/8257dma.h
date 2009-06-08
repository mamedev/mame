/**********************************************************************

    8257 DMA interface and emulation

**********************************************************************/

#ifndef DMA8257_H
#define DMA8257_H

#define DMA8257 DEVICE_GET_INFO_NAME(dma8257)

#define MDRV_DMA8257_ADD(_tag, _clock, _config) \
	MDRV_DEVICE_ADD(_tag, DMA8257, _clock) \
	MDRV_DEVICE_CONFIG(_config)


#define DMA8257_STATUS_UPDATE		0x10
#define DMA8257_STATUS_TC_CH3		0x08
#define DMA8257_STATUS_TC_CH2		0x04
#define DMA8257_STATUS_TC_CH1		0x02
#define DMA8257_STATUS_TC_CH0		0x01

#define DMA8257_NUM_CHANNELS		(4)


typedef struct _dma8257_interface dma8257_interface;
struct _dma8257_interface
{
	/* CPU to halt when DMA is active */
	const char *cputag;

	/* accessors to main memory */
	read8_device_func	memory_read;
	write8_device_func	memory_write;

	/* channel accesors */
	read8_device_func	channel_read[DMA8257_NUM_CHANNELS];
	write8_device_func	channel_write[DMA8257_NUM_CHANNELS];

	/* function to call when DMA completes */
	write8_device_func out_tc[DMA8257_NUM_CHANNELS];
};

/* device interface */
DEVICE_GET_INFO( dma8257 );

READ8_DEVICE_HANDLER( dma8257_r);
WRITE8_DEVICE_HANDLER( dma8257_w);

WRITE8_DEVICE_HANDLER( dma8257_drq_w ); /* offset spceifies channel! */

#endif /* DMA8257_H */

/**********************************************************************

    8237 DMA interface and emulation

**********************************************************************/

#ifndef __DMA8237_H_
#define __DMA8237_H_

#define DMA8237		DEVICE_GET_INFO_NAME(dma8237)

typedef void  (*dma8237_hrq_func)(const device_config *device, int state);
typedef UINT8 (*dma8237_mem_read_func)(const device_config *device, int channel, offs_t offset);
typedef void  (*dma8237_mem_write_func)(const device_config *device, int channel, offs_t offset, UINT8 data);
typedef int   (*dma8237_channel_read_func)(const device_config *device);
typedef void  (*dma8237_channel_write_func)(const device_config *device, int data);
typedef void  (*dma8237_out_eop_func)(const device_config *device, int channel, int state);

#define DMA8237_HRQ_CHANGED(name)		void  name(const device_config *device, int state)
#define DMA8237_MEM_READ(name)			UINT8 name(const device_config *device, int channel, offs_t offset)
#define DMA8237_MEM_WRITE(name)			void  name(const device_config *device, int channel, offs_t offset, UINT8 data)
#define DMA8237_CHANNEL_READ(name)		int   name(const device_config *device)
#define DMA8237_CHANNEL_WRITE(name)		void  name(const device_config *device, int data)
#define DMA8237_OUT_EOP(name)			void  name(const device_config *device, int channel, int state)

struct dma8237_interface
{
	/* speed of DMA accesses (per byte) */
	double bus_speed;

	/* function that will be called when HRQ may have changed */
	dma8237_hrq_func			hrq_changed;

	/* accessors to main memory */
	dma8237_mem_read_func		memory_read_func;
	dma8237_mem_write_func		memory_write_func;

	/* channel accesors */
	dma8237_channel_read_func	channel_read_func[4];
	dma8237_channel_write_func	channel_write_func[4];

	/* function to call when DMA completes */
	dma8237_out_eop_func		out_eop_func;
};


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_DMA8237_ADD(_tag, _intrf) \
	MDRV_DEVICE_ADD(_tag, DMA8237, 0) \
	MDRV_DEVICE_CONFIG(_intrf)


/* device interface */
DEVICE_GET_INFO( dma8237 );
READ8_DEVICE_HANDLER( dma8237_r );
WRITE8_DEVICE_HANDLER( dma8237_w );
void dma8237_drq_write(const device_config *device, int channel, int state);
void dma8237_set_hlda(const device_config *device, int state);

/* unfortunate hack for the interim for PC HDC */
void dma8237_run_transfer(const device_config *device, int channel);

#endif /* __DMA8237_H_ */

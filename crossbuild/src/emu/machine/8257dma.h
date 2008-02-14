/**********************************************************************

    8257 DMA interface and emulation

**********************************************************************/

#ifndef DMA8257_H
#define DMA8257_H

#define DMA8257_STATUS_UPDATE		0x10
#define DMA8257_STATUS_TC_CH3		0x08
#define DMA8257_STATUS_TC_CH2		0x04
#define DMA8257_STATUS_TC_CH1		0x02
#define DMA8257_STATUS_TC_CH0		0x01

#define DMA8257_NUM_CHANNELS		(4)


struct dma8257_interface
{
	/* CPU to halt when DMA is active */
	int cpunum;

	/* clock */
	int clockhz;

	/* accessors to main memory */
	UINT8	(*memory_read_func)(int channel, offs_t offset);
	void    (*memory_write_func)(int channel, offs_t offset, UINT8 data);

	/* channel accesors */
	UINT8     (*channel_read_func[DMA8257_NUM_CHANNELS])(void);
	void    (*channel_write_func[DMA8257_NUM_CHANNELS])(UINT8 data);

	/* function to call when DMA completes */
	void    (*out_tc_func[DMA8257_NUM_CHANNELS])(int state);
};



int dma8257_init(int count);
void dma8257_config(int which, const struct dma8257_interface *intf);
void dma8257_reset(void);

void dma8257_drq_write(int which, int channel, int state);

READ8_HANDLER( dma8257_0_r );
READ8_HANDLER( dma8257_1_r );
WRITE8_HANDLER( dma8257_0_w );
WRITE8_HANDLER( dma8257_1_w );

READ16_HANDLER( dma8257_16le_0_r );
READ16_HANDLER( dma8257_16le_1_r );
WRITE16_HANDLER( dma8257_16le_0_w );
WRITE16_HANDLER( dma8257_16le_1_w );

READ32_HANDLER( dma8257_32le_0_r );
READ32_HANDLER( dma8257_32le_1_r );
WRITE32_HANDLER( dma8257_32le_0_w );
WRITE32_HANDLER( dma8257_32le_1_w );

READ64_HANDLER( dma8257_64be_0_r );
READ64_HANDLER( dma8257_64be_1_r );
WRITE64_HANDLER( dma8257_64be_0_w );
WRITE64_HANDLER( dma8257_64be_1_w );

#endif /* DMA8257_H */

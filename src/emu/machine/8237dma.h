/**********************************************************************

    8237 DMA interface and emulation

**********************************************************************/

#ifndef DMA8237_H
#define DMA8237_H

struct dma8237_interface
{
	/* CPU to halt when DMA is active */
	int cpunum;

	/* speed of DMA accesses (per byte) */
	double bus_speed;

	/* accessors to main memory */
	UINT8	(*memory_read_func)(int channel, offs_t offset);
	void    (*memory_write_func)(int channel, offs_t offset, UINT8 data);

	/* channel accesors */
	int     (*channel_read_func[4])(void);
	void    (*channel_write_func[4])(int data);

	/* function to call when DMA completes */
	void    (*out_eop_func)(int state);
};



int dma8237_init(int count);
void dma8237_config(int which, const struct dma8237_interface *intf);
void dma8237_reset(void);

void dma8237_drq_write(int which, int channel, int state);

/* unfortunate hack for the interim for PC HDC */
void dma8237_run_transfer(int which, int channel);

READ8_HANDLER( dma8237_0_r );
READ8_HANDLER( dma8237_1_r );
WRITE8_HANDLER( dma8237_0_w );
WRITE8_HANDLER( dma8237_1_w );

READ16_HANDLER( dma8237_16le_0_r );
READ16_HANDLER( dma8237_16le_1_r );
WRITE16_HANDLER( dma8237_16le_0_w );
WRITE16_HANDLER( dma8237_16le_1_w );

READ32_HANDLER( dma8237_32le_0_r );
READ32_HANDLER( dma8237_32le_1_r );
WRITE32_HANDLER( dma8237_32le_0_w );
WRITE32_HANDLER( dma8237_32le_1_w );

READ64_HANDLER( dma8237_64be_0_r );
READ64_HANDLER( dma8237_64be_1_r );
WRITE64_HANDLER( dma8237_64be_0_w );
WRITE64_HANDLER( dma8237_64be_1_w );

#endif /* DMA8237_H */

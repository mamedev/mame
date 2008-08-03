/**********************************************************************

 8257 DMA interface and emulation

 **********************************************************************/

#ifndef Z80DMA_H
#define Z80DMA_H

#define Z80DMA DEVICE_GET_INFO_NAME(z80dma)

typedef struct _z80dma_interface z80dma_interface;
struct _z80dma_interface
{
	/* CPU to halt when DMA is active */
	int cpunum;

	/* clock */
	int clockhz;

	/* accessors to main memory */
	read8_device_func	memory_read;
	write8_device_func	memory_write;

	/* port accesors */
	read8_device_func	portA_read;
	write8_device_func	portA_write;
	read8_device_func	portB_read;
	write8_device_func	portB_write;

	/* interrupt callback - not implemented */
	/* void (*irqcb)(int state); */
};

/* device interface */
DEVICE_GET_INFO( z80dma );

READ8_DEVICE_HANDLER( z80dma_r );
WRITE8_DEVICE_HANDLER( z80dma_w);

WRITE8_DEVICE_HANDLER( z80dma_rdy_w );

#endif /* Z80_H */

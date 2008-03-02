/**********************************************************************

 8257 DMA interface and emulation

 **********************************************************************/

#ifndef Z80DMA_H
#define Z80DMA_H

typedef struct _z80dma_t z80dma_t;
typedef struct _z80dma_interface z80dma_interface;

#define Z80DMA DEVICE_GET_INFO_NAME(z80dma)

typedef UINT8 (*z80dma_read_func)(running_machine *machine, z80dma_t *z80dma, offs_t offset);
#define Z80DMA_READ(name) UINT8 name(running_machine *machine, z80dma_t *z80dma, offs_t offset)

typedef void (*z80dma_write_func)(running_machine *machine, z80dma_t *z80dma, offs_t offset, UINT8 data);
#define Z80DMA_WRITE(name) void name(running_machine *machine, z80dma_t *z80dma, offs_t offset, UINT8 data)

struct _z80dma_interface
{
	/* CPU to halt when DMA is active */
	int cpunum;

	/* clock */
	int clockhz;

	/* accessors to main memory */
	z80dma_read_func	memory_read;
	z80dma_write_func	memory_write;

	/* port accesors */
	z80dma_read_func	portA_read;
	z80dma_write_func	portA_write;
	z80dma_read_func	portB_read;
	z80dma_write_func	portB_write;

	/* interrupt callback - not implemented */
	/* void (*irqcb)(int state); */
};

/* device interface */
DEVICE_GET_INFO( z80dma );

void z80dma_rdy_write( z80dma_t *z80dma, int state);
UINT8 z80dma_read( z80dma_t *z80dma);
void z80dma_write( z80dma_t *z80dma, UINT8 data);

/******************* Standard 8-bit CPU interfaces *******************/

#define Z80DMA_DEV_0_TAG	"z80dma0"

READ8_HANDLER(z80dma_0_r);
WRITE8_HANDLER(z80dma_0_rdy_w);
WRITE8_HANDLER(z80dma_0_w);

#define Z80DMA_DEV_1_TAG	"z80dma1"

READ8_HANDLER(z80dma_1_r);
WRITE8_HANDLER(z80dma_1_w);
WRITE8_HANDLER(z80dma_1_rdy_w);

#endif /* Z80_H */

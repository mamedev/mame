/**********************************************************************

 8257 DMA interface and emulation

 **********************************************************************/

#ifndef Z80DMA_H
#define Z80DMA_H

struct z80dma_interface
{
	/* CPU to halt when DMA is active */
	int cpunum;

	/* clock */
	int clockhz;

	/* accessors to main memory */
	UINT8 (*memory_read_func)(offs_t offset);
	void (*memory_write_func)(offs_t offset, UINT8 data);

	/* port accesors */
	UINT8 (*portA_read_func)(UINT16 addr);
	void (*portA_write_func)(UINT16 addr, UINT8 data);
	UINT8 (*portB_read_func)(UINT16 addr);
	void (*portB_write_func)(UINT16 addr, UINT8 data);

	/* interrupt callback - not implemented */
	/* void (*irqcb)(int state); */
};

int z80dma_init(int count);
void z80dma_config(int which, const struct z80dma_interface *intf);
void z80dma_reset(void);

void z80dma_rdy_write(int which, int state);

READ8_HANDLER(z80dma_0_r);
READ8_HANDLER(z80dma_1_r);
WRITE8_HANDLER(z80dma_0_w);
WRITE8_HANDLER(z80dma_1_w);

WRITE8_HANDLER(z80dma_0_rdy_w);
WRITE8_HANDLER(z80dma_1_rdy_w);

READ16_HANDLER(z80dma_16le_0_r);
READ16_HANDLER(z80dma_16le_1_r);
WRITE16_HANDLER(z80dma_16le_0_w);
WRITE16_HANDLER(z80dma_16le_1_w);

READ32_HANDLER(z80dma_32le_0_r);
READ32_HANDLER(z80dma_32le_1_r);
WRITE32_HANDLER(z80dma_32le_0_w);
WRITE32_HANDLER(z80dma_32le_1_w);

READ64_HANDLER(z80dma_64be_0_r);
READ64_HANDLER(z80dma_64be_1_r);
WRITE64_HANDLER(z80dma_64be_0_w);
WRITE64_HANDLER(z80dma_64be_1_w);

#endif /* Z80_H */

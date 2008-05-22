#ifndef LSI53C810_H
#define LSI53C810_H

#include "machine/scsi.h"

struct LSI53C810interface
{
	const SCSIConfigTable *scsidevs;			/* SCSI devices */
	void (*irq_callback)(running_machine *machine); /* IRQ callback */
	void (*dma_callback)(UINT32, UINT32, int, int);	/* DMA callback */
	UINT32 (*fetch)(UINT32 dsp);
};

extern void lsi53c810_init(const struct LSI53C810interface *interface);
extern void lsi53c810_exit(const struct LSI53C810interface *interface);

extern void lsi53c810_read_data(int bytes, UINT8 *pData);
extern void lsi53c810_write_data(int bytes, UINT8 *pData);

extern void *lsi53c810_get_device(int id);

UINT8 lsi53c810_reg_r(int reg);
void lsi53c810_reg_w(running_machine *machine, int reg, UINT8 value);

unsigned lsi53c810_dasm(char *buf, UINT32 pc);

#endif

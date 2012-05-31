#ifndef LSI53C810_H
#define LSI53C810_H

#include "machine/scsi.h"

struct LSI53C810interface
{
	const SCSIConfigTable *scsidevs;			/* SCSI devices */
	void (*irq_callback)(running_machine &machine, int); /* IRQ callback */
	void (*dma_callback)(running_machine &machine, UINT32, UINT32, int, int);	/* DMA callback */
	UINT32 (*fetch)(running_machine &machine, UINT32 dsp);
};

extern void lsi53c810_init(running_machine &machine, const struct LSI53C810interface *interface);

extern void lsi53c810_read_data(int bytes, UINT8 *pData);
extern void lsi53c810_write_data(int bytes, UINT8 *pData);

extern void *lsi53c810_get_device(int id);

READ8_HANDLER( lsi53c810_reg_r );
WRITE8_HANDLER( lsi53c810_reg_w );

unsigned lsi53c810_dasm(running_machine &machine, char *buf, UINT32 pc);

#endif

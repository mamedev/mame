/*
 * wd33c93.h
 *
 */

#ifndef _WD33C93_H_
#define _WD33C93_H_

#include "machine/scsi.h"

struct WD33C93interface
{
	const SCSIConfigTable *scsidevs;		/* SCSI devices */
	void (*irq_callback)(running_machine &machine, int state); /* irq callback */
};

extern void wd33c93_init( running_machine &machine, const struct WD33C93interface *interface );
extern void wd33c93_get_dma_data(int bytes, UINT8 *pData);
extern void wd33c93_write_data(int bytes, UINT8 *pData);
extern void *wd33c93_get_device(int id);
extern void wd33c93_clear_dma(void);
extern int wd33c93_get_dma_count(void);
extern READ8_HANDLER(wd33c93_r);
extern WRITE8_HANDLER(wd33c93_w);

#endif

/*
 * am53cf96.h
 *
 */

#ifndef _AM53CF96_H_
#define _AM53CF96_H_

#include "scsi.h"

struct AM53CF96interface
{
	const SCSIConfigTable *scsidevs;	/* SCSI devices */
	void (*irq_callback)(running_machine &machine);	/* irq callback */
};

extern void am53cf96_init( running_machine &machine, const struct AM53CF96interface *interface );
extern void am53cf96_read_data(int bytes, UINT8 *pData);
void am53cf96_write_data(int bytes, UINT8 *pData);
void *am53cf96_get_device(int id);
extern READ32_HANDLER( am53cf96_r );
extern WRITE32_HANDLER( am53cf96_w );

#endif

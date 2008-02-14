/***************************************************************************

 scsihd.h

***************************************************************************/

#ifndef _SCSIHD_H_
#define _SCSIHD_H_

#include "machine/scsi.h"

// CD-ROM handler
extern const SCSIClass SCSIClassHARDDISK;
#define SCSI_DEVICE_HARDDISK &SCSIClassHARDDISK

#endif

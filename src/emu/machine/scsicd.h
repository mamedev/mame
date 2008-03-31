/***************************************************************************

 scsicd.h

***************************************************************************/

#ifndef _SCSICD_H_
#define _SCSICD_H_

#include "machine/scsi.h"

// CD-ROM handler
extern const SCSIClass SCSIClassCDROM;
#define SCSI_DEVICE_CDROM &SCSIClassCDROM

// we pass in the disk ID for each SCSI device, but that's not unique across device types.
// to avoid collisions, each non-HDD SCSI device type should have it's own BASE to prevent
// this problem.
#define SCSI_DEVICE_CDROM_STATE_BASE	(32)

#endif


/***************************************************************************

 scsicd.h

***************************************************************************/

#ifndef _SCSICD_H_
#define _SCSICD_H_

#include "machine/scsi.h"

// CD-ROM handler
extern SCSIClass SCSIClassCDROM;
#define SCSI_DEVICE_CDROM &SCSIClassCDROM

#endif


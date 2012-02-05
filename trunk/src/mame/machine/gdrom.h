/***************************************************************************

 gdrom.h

***************************************************************************/

#ifndef _GDROM_H_
#define _GDROM_H_

#include "machine/scsi.h"

// Sega GD-ROM handler
extern const SCSIClass SCSIClassGDROM;
#define SCSI_DEVICE_GDROM &SCSIClassGDROM

#endif


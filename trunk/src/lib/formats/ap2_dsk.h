/*********************************************************************

    ap2_dsk.h

    Apple II disk images

*********************************************************************/

#ifndef AP2_DISK_H
#define AP2_DISK_H

#include "flopimg.h"


/***************************************************************************

    Constants

***************************************************************************/

#define APPLE2_NIBBLE_SIZE			416
#define APPLE2_SMALL_NIBBLE_SIZE	374
#define APPLE2_TRACK_COUNT			35
#define APPLE2_SECTOR_COUNT			16
#define APPLE2_SECTOR_SIZE			256



/**************************************************************************/

LEGACY_FLOPPY_OPTIONS_EXTERN(apple2);

#endif /* AP2_DISK_H */

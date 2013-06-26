/*********************************************************************

    formats/ti99_dsk.c

    TI99 and Geneve disk images

    Header file to be included by drivers which use these floppy options
*********************************************************************/

#ifndef TI99_DSK_H
#define TI99_DSK_H

#include "flopimg.h"

LEGACY_FLOPPY_OPTIONS_EXTERN(ti99);

void ti99_set_80_track_drives(int use80);

#endif /* TI99_DSK_H */

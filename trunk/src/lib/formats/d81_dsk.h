/*********************************************************************

    formats/d81_dsk.h

    Floppy format code for Commodore 1581 disk images

*********************************************************************/

#ifndef __D81_DSK__
#define __D81_DSK__

#include "flopimg.h"

/***************************************************************************
    PROTOTYPES
***************************************************************************/

FLOPPY_IDENTIFY( d81_dsk_identify );
FLOPPY_CONSTRUCT( d81_dsk_construct );

#endif

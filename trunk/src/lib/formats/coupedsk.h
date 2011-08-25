/*************************************************************************

    formats/coupedsk.h

    SAM Coupe disk image formats

**************************************************************************/

#ifndef __COUPEDSK_H__
#define __COUPEDSK_H__

#include "flopimg.h"


FLOPPY_CONSTRUCT( coupe_mgt_construct );
FLOPPY_IDENTIFY( coupe_mgt_identify );
FLOPPY_CONSTRUCT( coupe_sad_construct );
FLOPPY_IDENTIFY( coupe_sad_identify );
FLOPPY_CONSTRUCT( coupe_sdf_construct );
FLOPPY_IDENTIFY( coupe_sdf_identify );


#endif /* __COUPEDSK_H__ */

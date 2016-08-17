// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    formats/coco_dsk.h

    Tandy Color Computer / Dragon disk images

*********************************************************************/

#ifndef COCO_DSK_H
#define COCO_DSK_H

#include "formats/flopimg.h"


/**************************************************************************/

LEGACY_FLOPPY_OPTIONS_EXTERN(coco);

FLOPPY_IDENTIFY(coco_dmk_identify);
FLOPPY_CONSTRUCT(coco_dmk_construct);

#endif /* COCO_DSK_H */

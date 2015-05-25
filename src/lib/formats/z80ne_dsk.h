// license:???
// copyright-holders:Roberto Lavarone
/*********************************************************************

    formats/z80ne_dsk.h

    Nuova Elettronica Z80NE disk images

*********************************************************************/

#ifndef Z80NE_DSK_H
#define Z80NE_DSK_H

#include "flopimg.h"


/**************************************************************************/

LEGACY_FLOPPY_OPTIONS_EXTERN(z80ne);

FLOPPY_IDENTIFY(z80ne_dmk_identify);
FLOPPY_CONSTRUCT(z80ne_dmk_construct);

#endif /* Z80NE_DSK_H */

/*********************************************************************

    ap2_lang.h

    Implementation of the Apple II Language Card

*********************************************************************/

#ifndef __AP2_LANG__
#define __AP2_LANG__

#include "emu.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

DECLARE_LEGACY_DEVICE(APPLE2_LANGCARD, apple2_langcard);

#define MCFG_APPLE2_LANGCARD_ADD(_tag)	\
	MCFG_DEVICE_ADD((_tag), APPLE2_LANGCARD, 0)



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/
/* slot read function */
READ8_DEVICE_HANDLER(apple2_langcard_r);

/* slot write function */
WRITE8_DEVICE_HANDLER(apple2_langcard_w);

#endif /* __AP2_LANG__ */

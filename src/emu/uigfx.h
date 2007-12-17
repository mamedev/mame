/***************************************************************************

    uigfx.h

    Internal graphics viewer.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __UIGFX_H__
#define __UIGFX_H__

#include "mamecore.h"


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* initialization */
void ui_gfx_init(running_machine *machine);

/* master handler */
UINT32 ui_gfx_ui_handler(UINT32 state);


#endif	/* __UIGFX_H__ */

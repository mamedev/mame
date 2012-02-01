/*************************************************************************

    decocomn.h

**************************************************************************/

#pragma once
#ifndef __DECOCOMN_H__
#define __DECOCOMN_H__

#include "devcb.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


typedef struct _decocomn_interface decocomn_interface;
struct _decocomn_interface
{
	const char         *screen;
};

DECLARE_LEGACY_DEVICE(DECOCOMN, decocomn);


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_DECOCOMN_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, DECOCOMN, 0) \
	MCFG_DEVICE_CONFIG(_interface)

/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

WRITE16_DEVICE_HANDLER( decocomn_nonbuffered_palette_w );
WRITE16_DEVICE_HANDLER( decocomn_buffered_palette_w );
WRITE16_DEVICE_HANDLER( decocomn_palette_dma_w );

WRITE16_DEVICE_HANDLER( decocomn_priority_w );
READ16_DEVICE_HANDLER( decocomn_priority_r );

READ16_DEVICE_HANDLER( decocomn_71_r );

#endif

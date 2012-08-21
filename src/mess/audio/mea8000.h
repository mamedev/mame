/**********************************************************************

  Copyright (C) Antoine Mine' 2006

  Philips MEA 8000 emulation.

**********************************************************************/

#ifndef MEA8000_H
#define MEA8000_H

#include "devlegcy.h"

DECLARE_LEGACY_DEVICE(MEA8000, mea8000);

/* ---------- configuration ------------ */

typedef struct _mea8000_interface mea8000_interface;
struct _mea8000_interface
{
	/* output channel */
	const char *           channel;

	/* 1-bit 'ready' output, not negated */
	write8_device_func req_out_func;
};


#define MCFG_MEA8000_ADD(_tag, _intrf)	      \
	MCFG_DEVICE_ADD(_tag, MEA8000, 0)	      \
	MCFG_DEVICE_CONFIG(_intrf)

/* interface to CPU via address/data bus*/
extern READ8_DEVICE_HANDLER  ( mea8000_r );
extern WRITE8_DEVICE_HANDLER ( mea8000_w );

#endif

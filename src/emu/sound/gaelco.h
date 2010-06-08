#pragma once

#ifndef __GALELCO_H__
#define __GALELCO_H__

#include "devlegcy.h"

typedef struct _gaelcosnd_interface gaelcosnd_interface;
struct _gaelcosnd_interface
{
	const char *gfxregion;	/* shared gfx region name */
	int banks[4];			/* start of each ROM bank */
};

extern UINT16 *gaelco_sndregs;

WRITE16_DEVICE_HANDLER( gaelcosnd_w );
READ16_DEVICE_HANDLER( gaelcosnd_r );

DECLARE_LEGACY_SOUND_DEVICE(GAELCO_GAE1, gaelco_gae1);
DECLARE_LEGACY_SOUND_DEVICE(GAELCO_CG1V, gaelco_cg1v);

#endif /* __GALELCO_H__ */

#pragma once

#ifndef __GALELCO_H__
#define __GALELCO_H__

typedef struct _gaelcosnd_interface gaelcosnd_interface;
struct _gaelcosnd_interface
{
	const char *gfxregion;	/* shared gfx region name */
	int banks[4];			/* start of each ROM bank */
};

extern UINT16 *gaelco_sndregs;

WRITE16_HANDLER( gaelcosnd_w );
READ16_HANDLER( gaelcosnd_r );

#endif /* __GALELCO_H__ */

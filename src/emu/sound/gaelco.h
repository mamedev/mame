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

WRITE16_DEVICE_HANDLER( gaelcosnd_w );
READ16_DEVICE_HANDLER( gaelcosnd_r );

DEVICE_GET_INFO( gaelco_gae1 );
DEVICE_GET_INFO( gaelco_cg1v );

#define SOUND_GAELCO_GAE1 DEVICE_GET_INFO_NAME( gaelco_gae1 )
#define SOUND_GAELCO_CG1V DEVICE_GET_INFO_NAME( gaelco_cg1v )

#endif /* __GALELCO_H__ */

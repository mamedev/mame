#ifndef gaelco_snd_h
#define gaelco_snd_h

struct gaelcosnd_interface
{
	const char *gfxregion;	/* shared gfx region name */
	int banks[4];			/* start of each ROM bank */
};

extern UINT16 *gaelco_sndregs;

WRITE16_HANDLER( gaelcosnd_w );
READ16_HANDLER( gaelcosnd_r );

#endif

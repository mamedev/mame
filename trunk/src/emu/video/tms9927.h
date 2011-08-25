/**********************************************************************

    TI TMS9927 and compatible CRT controller emulation

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#ifndef __TMS9927__
#define __TMS9927__

#include "devlegcy.h"


DECLARE_LEGACY_DEVICE(TMS9927, tms9927);
DECLARE_LEGACY_DEVICE(CRT5027, crt5027);
DECLARE_LEGACY_DEVICE(CRT5037, crt5037);
DECLARE_LEGACY_DEVICE(CRT5057, crt5057);


#define MCFG_TMS9927_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, TMS9927, _clock) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_TMS9927_RECONFIG(_tag, _clock, _config) \
	MCFG_DEVICE_MODIFY(_tag) \
	MCFG_DEVICE_CLOCK(_clock) \
	MCFG_DEVICE_CONFIG(_config)



/* interface */
typedef struct _tms9927_interface tms9927_interface;
struct _tms9927_interface
{
	const char *screen_tag;			/* screen we are acting on */
	int hpixels_per_column;			/* number of pixels per video memory address */
	const char *selfload_region;	/* name of the region with self-load data */
};

extern const tms9927_interface tms9927_null_interface;


/* basic read/write handlers */
WRITE8_DEVICE_HANDLER( tms9927_w );
READ8_DEVICE_HANDLER( tms9927_r );

/* other queries */
int tms9927_screen_reset(device_t *device);
int tms9927_upscroll_offset(device_t *device);
int tms9927_cursor_bounds(device_t *device, rectangle *bounds);



#endif

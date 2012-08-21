
#ifndef __K2GE_H_
#define __K2GE_H_

#include "devcb.h"


#define K1GE_SCREEN_HEIGHT	199


DECLARE_LEGACY_DEVICE(K1GE, k1ge);
DECLARE_LEGACY_DEVICE(K2GE, k2ge);


#define MCFG_K1GE_ADD(_tag, _clock, _config ) \
	MCFG_DEVICE_ADD( _tag, K1GE, _clock ) \
	MCFG_DEVICE_CONFIG( _config )


#define MCFG_K2GE_ADD(_tag, _clock, _config ) \
	MCFG_DEVICE_ADD( _tag, K2GE, _clock ) \
	MCFG_DEVICE_CONFIG( _config )


typedef struct _k1ge_interface k1ge_interface;
struct _k1ge_interface
{
	const char		*screen_tag;		/* screen we are drawing on */
	const char		*vram_tag;			/* memory region we will use for video ram */
	devcb_write8	vblank_pin_w;		/* called back when VBlank pin may have changed */
	devcb_write8	hblank_pin_w;		/* called back when HBlank pin may have changed */
};


PALETTE_INIT( k1ge );
PALETTE_INIT( k2ge );

WRITE8_DEVICE_HANDLER( k1ge_w );
READ8_DEVICE_HANDLER( k1ge_r );

void k1ge_update( device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect );

#endif


/*************************************************************************

    Caveman Ninja (and other DECO 16bit titles)

*************************************************************************/

#include "sound/okim6295.h"
#include "video/deco16ic.h"
#include "video/decocomn.h"

class cninja_state : public driver_device
{
public:
	cninja_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  maincpu(*this, "maincpu"),
		  audiocpu(*this, "audiocpu"),
		  decocomn(*this, "deco_common"),
		  deco_tilegen1(*this, "tilegen1"),
		  deco_tilegen2(*this, "tilegen2"),
		  raster_irq_timer(*this, "raster_timer"),
		  oki2(*this, "oki2") { }

	/* memory pointers */
	UINT16 *   ram;
	UINT16 *   pf1_rowscroll;
	UINT16 *   pf2_rowscroll;
	UINT16 *   pf3_rowscroll;
	UINT16 *   pf4_rowscroll;

	/* misc */
	int        scanline, irq_mask;

	/* devices */
	required_device<cpu_device> maincpu;
	required_device<cpu_device> audiocpu;
	required_device<decocomn_device> decocomn;
	required_device<deco16ic_device> deco_tilegen1;
	required_device<deco16ic_device> deco_tilegen2;
	optional_device<timer_device> raster_irq_timer;
	optional_device<okim6295_device> oki2;
};

/*----------- defined in video/cninja.c -----------*/

VIDEO_START( stoneage );
VIDEO_START( mutantf );

SCREEN_UPDATE( cninja );
SCREEN_UPDATE( cninjabl );
SCREEN_UPDATE( edrandy );
SCREEN_UPDATE( robocop2 );
SCREEN_UPDATE( mutantf );

/*************************************************************************

    Caveman Ninja (and other DECO 16bit titles)

*************************************************************************/

#include "sound/okim6295.h"
#include "video/deco16ic.h"

class cninja_state : public driver_device
{
public:
	cninja_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  maincpu(*this, "maincpu"),
		  audiocpu(*this, "audiocpu"),
		  deco16ic(*this, "deco_custom"),
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
	required_device<deco16ic_device> deco16ic;
	optional_device<timer_device> raster_irq_timer;
	optional_device<okim6295_device> oki2;
};

/*----------- defined in video/cninja.c -----------*/

VIDEO_START( stoneage );

VIDEO_UPDATE( cninja );
VIDEO_UPDATE( cninjabl );
VIDEO_UPDATE( edrandy );
VIDEO_UPDATE( robocop2 );
VIDEO_UPDATE( mutantf );

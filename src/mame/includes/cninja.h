/*************************************************************************

    Caveman Ninja (and other DECO 16bit titles)

*************************************************************************/

#include "sound/okim6295.h"
#include "video/deco16ic.h"

class cninja_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, cninja_state(machine)); }

	cninja_state(running_machine &machine)
		: maincpu(machine.device<cpu_device>("maincpu")),
		  audiocpu(machine.device<cpu_device>("audiocpu")),
		  deco16ic(machine.device<deco16ic_device>("deco_custom")),
		  raster_irq_timer(machine.device<timer_device>("raster_timer")),
		  oki2(machine.device<okim6295_device>("oki2")) { }

	/* memory pointers */
	UINT16 *   ram;
	UINT16 *   pf1_rowscroll;
	UINT16 *   pf2_rowscroll;
	UINT16 *   pf3_rowscroll;
	UINT16 *   pf4_rowscroll;

	/* misc */
	int        scanline, irq_mask;

	/* devices */
	cpu_device *maincpu;
	cpu_device *audiocpu;
	deco16ic_device *deco16ic;
	timer_device *raster_irq_timer;
	okim6295_device *oki2;
};

/*----------- defined in video/cninja.c -----------*/

VIDEO_START( stoneage );

VIDEO_UPDATE( cninja );
VIDEO_UPDATE( cninjabl );
VIDEO_UPDATE( edrandy );
VIDEO_UPDATE( robocop2 );
VIDEO_UPDATE( mutantf );

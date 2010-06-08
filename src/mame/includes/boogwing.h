/*************************************************************************

    Boogie Wings

*************************************************************************/

#include "sound/okim6295.h"
#include "video/deco16ic.h"

class boogwing_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, boogwing_state(machine)); }

	boogwing_state(running_machine &machine)
		: maincpu(machine.device<cpu_device>("maincpu")),
		  audiocpu(machine.device<cpu_device>("audiocpu")),
		  deco16ic(machine.device<deco16ic_device>("deco_custom")),
		  oki1(machine.device<okim6295_device>("oki1")),
		  oki2(machine.device<okim6295_device>("oki2")) { }

	/* memory pointers */
	UINT16 *  pf1_rowscroll;
	UINT16 *  pf2_rowscroll;
	UINT16 *  pf3_rowscroll;
	UINT16 *  pf4_rowscroll;

	/* devices */
	cpu_device *maincpu;
	cpu_device *audiocpu;
	deco16ic_device *deco16ic;
	okim6295_device *oki1;
	okim6295_device *oki2;
};


/*----------- defined in video/boogwing.c -----------*/

VIDEO_UPDATE( boogwing );


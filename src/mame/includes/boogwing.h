/*************************************************************************

    Boogie Wings

*************************************************************************/

#include "sound/okim6295.h"
#include "video/deco16ic.h"

class boogwing_state : public driver_device
{
public:
	boogwing_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  maincpu(*this, "maincpu"),
		  audiocpu(*this, "audiocpu"),
		  deco16ic(*this, "deco_custom"),
		  oki1(*this, "oki1"),
		  oki2(*this, "oki2") { }

	/* memory pointers */
	UINT16 *  pf1_rowscroll;
	UINT16 *  pf2_rowscroll;
	UINT16 *  pf3_rowscroll;
	UINT16 *  pf4_rowscroll;

	/* devices */
	required_device<cpu_device> maincpu;
	required_device<cpu_device> audiocpu;
	required_device<deco16ic_device> deco16ic;
	required_device<okim6295_device> oki1;
	required_device<okim6295_device> oki2;
};


/*----------- defined in video/boogwing.c -----------*/

VIDEO_UPDATE( boogwing );


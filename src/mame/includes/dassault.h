/*************************************************************************

    Desert Assault

*************************************************************************/

#include "sound/okim6295.h"
#include "video/deco16ic.h"
#include "video/decocomn.h"

class dassault_state : public driver_device
{
public:
	dassault_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  maincpu(*this, "maincpu"),
		  audiocpu(*this, "audiocpu"),
		  subcpu(*this, "sub"),
		  decocomn(*this, "deco_common"),
		  deco_tilegen1(*this, "tilegen1"),
		  deco_tilegen2(*this, "tilegen2"),
		  oki2(*this, "oki2") { }

	/* memory pointers */
	UINT16 *  pf2_rowscroll;
	UINT16 *  pf4_rowscroll;
	UINT16 *  ram;
	UINT16 *  ram2;
	UINT16 *  shared_ram;

	/* devices */
	required_device<cpu_device> maincpu;
	required_device<cpu_device> audiocpu;
	required_device<cpu_device> subcpu;
	required_device<decocomn_device> decocomn;
	required_device<deco16ic_device> deco_tilegen1;
	required_device<deco16ic_device> deco_tilegen2;
	required_device<okim6295_device> oki2;
};



/*----------- defined in video/dassault.c -----------*/

SCREEN_UPDATE( dassault );

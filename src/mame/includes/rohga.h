/*************************************************************************

    Data East 'Rohga' era hardware

*************************************************************************/

#include "sound/okim6295.h"
#include "video/deco16ic.h"

class rohga_state : public driver_device
{
public:
	rohga_state(running_machine &machine, const driver_device_config_base &config)
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
	UINT16 *  spriteram;

	/* devices */
	required_device<cpu_device> maincpu;
	required_device<cpu_device> audiocpu;
	required_device<deco16ic_device> deco16ic;
	required_device<okim6295_device> oki1;
	required_device<okim6295_device> oki2;
};



/*----------- defined in video/rohga.c -----------*/

WRITE16_HANDLER( rohga_buffer_spriteram16_w );

VIDEO_START( rohga );

VIDEO_UPDATE( rohga );
VIDEO_UPDATE( schmeisr );
VIDEO_UPDATE( wizdfire );
VIDEO_UPDATE( nitrobal );

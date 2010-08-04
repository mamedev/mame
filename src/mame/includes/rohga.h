/*************************************************************************

    Data East 'Rohga' era hardware

*************************************************************************/

#include "sound/okim6295.h"
#include "video/deco16ic.h"

class rohga_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, rohga_state(machine)); }

	rohga_state(running_machine &machine)
		: driver_data_t(machine),
		  maincpu(machine.device<cpu_device>("maincpu")),
		  audiocpu(machine.device<cpu_device>("audiocpu")),
		  deco16ic(machine.device<deco16ic_device>("deco_custom")),
		  oki1(machine.device<okim6295_device>("oki1")),
		  oki2(machine.device<okim6295_device>("oki2")) { }

	/* memory pointers */
	UINT16 *  pf1_rowscroll;
	UINT16 *  pf2_rowscroll;
	UINT16 *  pf3_rowscroll;
	UINT16 *  pf4_rowscroll;
	UINT16 *  spriteram;

	/* devices */
	cpu_device *maincpu;
	cpu_device *audiocpu;
	deco16ic_device *deco16ic;
	okim6295_device *oki1;
	okim6295_device *oki2;
};



/*----------- defined in video/rohga.c -----------*/

WRITE16_HANDLER( rohga_buffer_spriteram16_w );

VIDEO_START( rohga );

VIDEO_UPDATE( rohga );
VIDEO_UPDATE( schmeisr );
VIDEO_UPDATE( wizdfire );
VIDEO_UPDATE( nitrobal );

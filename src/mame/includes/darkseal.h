#include "video/deco16ic.h"

class darkseal_state : public driver_device
{
public:
	darkseal_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
	      deco_tilegen1(*this, "tilegen1"),
		  deco_tilegen2(*this, "tilegen2") { }

	UINT16 *ram;
	UINT16 *pf1_rowscroll;
	//UINT16 *pf2_rowscroll;
	UINT16 *pf3_rowscroll;
	//UINT16 *pf4_rowscroll;

	required_device<deco16ic_device> deco_tilegen1;
	required_device<deco16ic_device> deco_tilegen2;

	int flipscreen;
};


/*----------- defined in video/darkseal.c -----------*/

VIDEO_START( darkseal );
SCREEN_UPDATE( darkseal );

WRITE16_HANDLER( darkseal_palette_24bit_rg_w );
WRITE16_HANDLER( darkseal_palette_24bit_b_w );

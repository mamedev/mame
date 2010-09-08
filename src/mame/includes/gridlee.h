/***************************************************************************

    Videa Gridlee hardware

    driver by Aaron Giles

***************************************************************************/

#include "devlegcy.h"


class gridlee_state : public driver_device
{
public:
	gridlee_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
};


/*----------- defined in audio/gridlee.c -----------*/

WRITE8_HANDLER( gridlee_sound_w );

DECLARE_LEGACY_SOUND_DEVICE(GRIDLEE, gridlee_sound);


/*----------- defined in video/gridlee.c -----------*/

/* video driver data & functions */
extern UINT8 gridlee_cocktail_flip;

PALETTE_INIT( gridlee );
VIDEO_START( gridlee );
VIDEO_UPDATE( gridlee );

WRITE8_HANDLER( gridlee_cocktail_flip_w );
WRITE8_HANDLER( gridlee_videoram_w );
WRITE8_HANDLER( gridlee_palette_select_w );

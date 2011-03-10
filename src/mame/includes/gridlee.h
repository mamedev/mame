/***************************************************************************

    Videa Gridlee hardware

    driver by Aaron Giles

***************************************************************************/

#include "devlegcy.h"


#define GRIDLEE_MASTER_CLOCK	(20000000)
#define GRIDLEE_CPU_CLOCK		(GRIDLEE_MASTER_CLOCK / 16)
#define GRIDLEE_PIXEL_CLOCK	(GRIDLEE_MASTER_CLOCK / 4)
#define GRIDLEE_HTOTAL			(0x140)
#define GRIDLEE_HBEND			(0x000)
#define GRIDLEE_HBSTART		(0x100)
#define GRIDLEE_VTOTAL			(0x108)
#define GRIDLEE_VBEND			(0x010)
#define GRIDLEE_VBSTART		(0x100)


class gridlee_state : public driver_device
{
public:
	gridlee_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	cpu_device *maincpu;
	UINT8 last_analog_input[2];
	UINT8 last_analog_output[2];
	UINT8 *poly17;
	UINT8 *rand17;
	emu_timer *irq_off;
	emu_timer *irq_timer;
	emu_timer *firq_off;
	emu_timer *firq_timer;
	UINT8 cocktail_flip;
	UINT8 *local_videoram;
	UINT8 palettebank_vis;
};


/*----------- defined in audio/gridlee.c -----------*/

WRITE8_DEVICE_HANDLER( gridlee_sound_w );

DECLARE_LEGACY_SOUND_DEVICE(GRIDLEE, gridlee_sound);


/*----------- defined in video/gridlee.c -----------*/

/* video driver data & functions */

PALETTE_INIT( gridlee );
VIDEO_START( gridlee );
SCREEN_UPDATE( gridlee );

WRITE8_HANDLER( gridlee_cocktail_flip_w );
WRITE8_HANDLER( gridlee_videoram_w );
WRITE8_HANDLER( gridlee_palette_select_w );

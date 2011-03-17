#include "devlegcy.h"

class flower_state : public driver_device
{
public:
	flower_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *spriteram;
	UINT8 *sn_irq_enable;
	UINT8 *sn_nmi_enable;
	UINT8 *textram;
	UINT8 *bg0ram;
	UINT8 *bg1ram;
	UINT8 *bg0_scroll;
	UINT8 *bg1_scroll;
	tilemap_t *bg0_tilemap;
	tilemap_t *bg1_tilemap;
	tilemap_t *text_tilemap;
	tilemap_t *text_right_tilemap;
};


/*----------- defined in audio/flower.c -----------*/

WRITE8_DEVICE_HANDLER( flower_sound1_w );
WRITE8_DEVICE_HANDLER( flower_sound2_w );

DECLARE_LEGACY_SOUND_DEVICE(FLOWER, flower_sound);


/*----------- defined in video/flower.c -----------*/

WRITE8_HANDLER( flower_textram_w );
WRITE8_HANDLER( flower_bg0ram_w );
WRITE8_HANDLER( flower_bg1ram_w );
WRITE8_HANDLER( flower_flipscreen_w );

SCREEN_UPDATE( flower );
VIDEO_START( flower );
PALETTE_INIT( flower );

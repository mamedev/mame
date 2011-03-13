class mcr3_state : public driver_device
{
public:
	mcr3_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	UINT8 *spriteram;
	size_t spriteram_size;
};


/*----------- defined in video/mcr3.c -----------*/

extern UINT8 spyhunt_sprite_color_mask;
extern INT16 spyhunt_scroll_offset;

extern UINT8 *spyhunt_alpharam;

WRITE8_HANDLER( mcr3_paletteram_w );
WRITE8_HANDLER( mcr3_videoram_w );
WRITE8_HANDLER( spyhunt_videoram_w );
WRITE8_HANDLER( spyhunt_alpharam_w );
WRITE8_HANDLER( spyhunt_scroll_value_w );

VIDEO_START( mcrmono );
VIDEO_START( spyhunt );

PALETTE_INIT( spyhunt );

SCREEN_UPDATE( mcr3 );
SCREEN_UPDATE( spyhunt );

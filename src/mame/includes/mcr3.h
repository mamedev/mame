class mcr3_state : public driver_device
{
public:
	mcr3_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	UINT8 *spriteram;
	size_t spriteram_size;
	UINT8 input_mux;
	UINT8 latched_input;
	UINT8 last_op4;
	UINT8 maxrpm_adc_control;
	UINT8 maxrpm_adc_select;
	UINT8 maxrpm_last_shift;
	INT8 maxrpm_p1_shift;
	INT8 maxrpm_p2_shift;
	UINT8 spyhunt_sprite_color_mask;
	INT16 spyhunt_scroll_offset;
	UINT8 *spyhunt_alpharam;
	INT16 spyhunt_scrollx;
	INT16 spyhunt_scrolly;
	tilemap_t *bg_tilemap;
	tilemap_t *alpha_tilemap;
};


/*----------- defined in video/mcr3.c -----------*/

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

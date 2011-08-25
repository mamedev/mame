class mcr3_state : public driver_device
{
public:
	mcr3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
	UINT8 m_input_mux;
	UINT8 m_latched_input;
	UINT8 m_last_op4;
	UINT8 m_maxrpm_adc_control;
	UINT8 m_maxrpm_adc_select;
	UINT8 m_maxrpm_last_shift;
	INT8 m_maxrpm_p1_shift;
	INT8 m_maxrpm_p2_shift;
	UINT8 m_spyhunt_sprite_color_mask;
	INT16 m_spyhunt_scroll_offset;
	UINT8 *m_spyhunt_alpharam;
	INT16 m_spyhunt_scrollx;
	INT16 m_spyhunt_scrolly;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_alpha_tilemap;
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

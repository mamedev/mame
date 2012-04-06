class sauro_state : public driver_device
{
public:
	sauro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	size_t m_spriteram_size;
	UINT8 *m_spriteram;
	UINT8 *m_videoram;
	UINT8 *m_colorram;
	UINT8 *m_videoram2;
	UINT8 *m_colorram2;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	UINT8 m_palette_bank;
	DECLARE_WRITE8_MEMBER(sauro_sound_command_w);
	DECLARE_READ8_MEMBER(sauro_sound_command_r);
	DECLARE_WRITE8_MEMBER(sauro_coin1_w);
	DECLARE_WRITE8_MEMBER(sauro_coin2_w);
	DECLARE_WRITE8_MEMBER(flip_screen_w);
	DECLARE_WRITE8_MEMBER(tecfri_videoram_w);
	DECLARE_WRITE8_MEMBER(tecfri_colorram_w);
	DECLARE_WRITE8_MEMBER(tecfri_videoram2_w);
	DECLARE_WRITE8_MEMBER(tecfri_colorram2_w);
	DECLARE_WRITE8_MEMBER(tecfri_scroll_bg_w);
	DECLARE_WRITE8_MEMBER(sauro_palette_bank_w);
	DECLARE_WRITE8_MEMBER(sauro_scroll_fg_w);
};


/*----------- defined in video/sauro.c -----------*/


VIDEO_START( sauro );
VIDEO_START( trckydoc );

SCREEN_UPDATE_IND16( sauro );
SCREEN_UPDATE_IND16( trckydoc );

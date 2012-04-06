class tecmo16_state : public driver_device
{
public:
	tecmo16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_videoram;
	UINT16 *m_colorram;
	UINT16 *m_videoram2;
	UINT16 *m_colorram2;
	UINT16 *m_charram;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_tx_tilemap;
	bitmap_ind16 m_sprite_bitmap;
	bitmap_ind16 m_tile_bitmap_bg;
	bitmap_ind16 m_tile_bitmap_fg;
	int m_flipscreen;
	int m_game_is_riot;
	UINT16 m_scroll_x_w;
	UINT16 m_scroll_y_w;
	UINT16 m_scroll2_x_w;
	UINT16 m_scroll2_y_w;
	UINT16 m_scroll_char_x_w;
	UINT16 m_scroll_char_y_w;
	UINT16 *m_spriteram;
	size_t m_spriteram_size;
	DECLARE_WRITE16_MEMBER(tecmo16_sound_command_w);
	DECLARE_WRITE16_MEMBER(tecmo16_videoram_w);
	DECLARE_WRITE16_MEMBER(tecmo16_colorram_w);
	DECLARE_WRITE16_MEMBER(tecmo16_videoram2_w);
	DECLARE_WRITE16_MEMBER(tecmo16_colorram2_w);
	DECLARE_WRITE16_MEMBER(tecmo16_charram_w);
	DECLARE_WRITE16_MEMBER(tecmo16_flipscreen_w);
	DECLARE_WRITE16_MEMBER(tecmo16_scroll_x_w);
	DECLARE_WRITE16_MEMBER(tecmo16_scroll_y_w);
	DECLARE_WRITE16_MEMBER(tecmo16_scroll2_x_w);
	DECLARE_WRITE16_MEMBER(tecmo16_scroll2_y_w);
	DECLARE_WRITE16_MEMBER(tecmo16_scroll_char_x_w);
	DECLARE_WRITE16_MEMBER(tecmo16_scroll_char_y_w);
};


/*----------- defined in video/tecmo16.c -----------*/



VIDEO_START( fstarfrc );
VIDEO_START( ginkun );
VIDEO_START( riot );
SCREEN_UPDATE_RGB32( tecmo16 );

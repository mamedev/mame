class gladiatr_state : public driver_device
{
public:
	gladiatr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_nvram(*this, "nvram") { }

	required_shared_ptr<UINT8>	m_nvram;
	int m_data1;
	int m_data2;
	int m_flag1;
	int m_flag2;
	UINT8 *m_videoram;
	UINT8 *m_colorram;
	UINT8 *m_textram;
	int m_video_attributes;
	int m_fg_scrollx;
	int m_fg_scrolly;
	int m_bg_scrollx;
	int m_bg_scrolly;
	int m_sprite_bank;
	int m_sprite_buffer;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	int m_fg_tile_bank;
	int m_bg_tile_bank;
	UINT8 *m_spriteram;
};

/*----------- defined in video/gladiatr.c -----------*/

WRITE8_HANDLER( gladiatr_videoram_w );
WRITE8_HANDLER( gladiatr_colorram_w );
WRITE8_HANDLER( gladiatr_textram_w );
WRITE8_HANDLER( gladiatr_paletteram_w );
WRITE8_HANDLER( ppking_video_registers_w );
WRITE8_HANDLER( gladiatr_video_registers_w );
WRITE8_HANDLER( gladiatr_spritebuffer_w );
WRITE8_HANDLER( gladiatr_spritebank_w );
VIDEO_START( ppking );
SCREEN_UPDATE( ppking );
VIDEO_START( gladiatr );
SCREEN_UPDATE( gladiatr );

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
	DECLARE_WRITE8_MEMBER(gladiatr_videoram_w);
	DECLARE_WRITE8_MEMBER(gladiatr_colorram_w);
	DECLARE_WRITE8_MEMBER(gladiatr_textram_w);
	DECLARE_WRITE8_MEMBER(gladiatr_paletteram_w);
	DECLARE_WRITE8_MEMBER(gladiatr_spritebuffer_w);
	DECLARE_WRITE8_MEMBER(gladiatr_spritebank_w);
	DECLARE_WRITE8_MEMBER(ppking_video_registers_w);
	DECLARE_WRITE8_MEMBER(gladiatr_video_registers_w);
};

/*----------- defined in video/gladiatr.c -----------*/

VIDEO_START( ppking );
SCREEN_UPDATE_IND16( ppking );
VIDEO_START( gladiatr );
SCREEN_UPDATE_IND16( gladiatr );

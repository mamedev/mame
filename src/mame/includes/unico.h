class unico_state : public driver_device
{
public:
	unico_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_vram;
	UINT16 *m_scroll;
	UINT32 *m_vram32;
	UINT32 *m_scroll32;
	tilemap_t *m_tilemap[3];
	int m_sprites_scrolldx;
	int m_sprites_scrolldy;
	UINT16 *m_spriteram;
	size_t m_spriteram_size;
	DECLARE_WRITE16_MEMBER(zeropnt_sound_bank_w);
	DECLARE_READ16_MEMBER(unico_gunx_0_msb_r);
	DECLARE_READ16_MEMBER(unico_guny_0_msb_r);
	DECLARE_READ16_MEMBER(unico_gunx_1_msb_r);
	DECLARE_READ16_MEMBER(unico_guny_1_msb_r);
	DECLARE_READ32_MEMBER(zeropnt2_gunx_0_msb_r);
	DECLARE_READ32_MEMBER(zeropnt2_guny_0_msb_r);
	DECLARE_READ32_MEMBER(zeropnt2_gunx_1_msb_r);
	DECLARE_READ32_MEMBER(zeropnt2_guny_1_msb_r);
	DECLARE_WRITE32_MEMBER(zeropnt2_sound_bank_w);
	DECLARE_WRITE32_MEMBER(zeropnt2_leds_w);
	DECLARE_WRITE16_MEMBER(unico_palette_w);
	DECLARE_WRITE32_MEMBER(unico_palette32_w);
	DECLARE_WRITE16_MEMBER(unico_vram_w);
	DECLARE_WRITE32_MEMBER(unico_vram32_w);
};


/*----------- defined in video/unico.c -----------*/



VIDEO_START( unico );
SCREEN_UPDATE_IND16( unico );

VIDEO_START( zeropnt2 );
SCREEN_UPDATE_IND16( zeropnt2 );


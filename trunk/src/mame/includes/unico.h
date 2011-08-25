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
};


/*----------- defined in video/unico.c -----------*/

WRITE16_HANDLER( unico_vram_w );
WRITE16_HANDLER( unico_palette_w );

WRITE32_HANDLER( unico_vram32_w );
WRITE32_HANDLER( unico_palette32_w );

VIDEO_START( unico );
SCREEN_UPDATE( unico );

VIDEO_START( zeropnt2 );
SCREEN_UPDATE( zeropnt2 );


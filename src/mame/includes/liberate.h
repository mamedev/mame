class liberate_state : public driver_device
{
public:
	liberate_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 *m_colorram;
	UINT8 *m_paletteram;
	UINT8 *m_spriteram;
	UINT8 *m_scratchram;
	UINT8 *m_charram;	/* prosoccr */
	UINT8 *m_bg_vram; /* prosport */

	UINT8 m_io_ram[16];

	int m_bank;
	int m_latch;
	UINT8 m_gfx_rom_readback;
	int m_background_color;
	int m_background_disable;

	tilemap_t *m_back_tilemap;
	tilemap_t *m_fix_tilemap;

	device_t *m_maincpu;
	device_t *m_audiocpu;
	DECLARE_READ8_MEMBER(deco16_bank_r);
	DECLARE_READ8_MEMBER(deco16_io_r);
	DECLARE_WRITE8_MEMBER(deco16_bank_w);
	DECLARE_READ8_MEMBER(prosoccr_bank_r);
	DECLARE_READ8_MEMBER(prosoccr_charram_r);
	DECLARE_WRITE8_MEMBER(prosoccr_charram_w);
	DECLARE_WRITE8_MEMBER(prosoccr_char_bank_w);
	DECLARE_WRITE8_MEMBER(prosoccr_io_bank_w);
	DECLARE_READ8_MEMBER(prosport_charram_r);
	DECLARE_WRITE8_MEMBER(prosport_charram_w);
	DECLARE_WRITE8_MEMBER(deco16_io_w);
	DECLARE_WRITE8_MEMBER(prosoccr_io_w);
	DECLARE_WRITE8_MEMBER(prosport_io_w);
	DECLARE_WRITE8_MEMBER(liberate_videoram_w);
	DECLARE_WRITE8_MEMBER(liberate_colorram_w);
	DECLARE_WRITE8_MEMBER(prosport_bg_vram_w);
	DECLARE_WRITE8_MEMBER(prosport_paletteram_w);
};


/*----------- defined in video/liberate.c -----------*/

PALETTE_INIT( liberate );
SCREEN_UPDATE_IND16( prosoccr );
SCREEN_UPDATE_IND16( prosport );
SCREEN_UPDATE_IND16( liberate );
SCREEN_UPDATE_IND16( boomrang );
VIDEO_START( prosoccr );
VIDEO_START( prosport );
VIDEO_START( boomrang );
VIDEO_START( liberate );



class punchout_state : public driver_device
{
public:
	punchout_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_rp5c01_mode_sel;
	int m_rp5c01_mem[16*4];
	UINT8 *m_bg_top_videoram;
	UINT8 *m_bg_bot_videoram;
	UINT8 *m_armwrest_fg_videoram;
	UINT8 *m_spr1_videoram;
	UINT8 *m_spr2_videoram;
	UINT8 *m_spr1_ctrlram;
	UINT8 *m_spr2_ctrlram;
	UINT8 *m_palettebank;
	tilemap_t *m_bg_top_tilemap;
	tilemap_t *m_bg_bot_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_spr1_tilemap;
	tilemap_t *m_spr1_tilemap_flipx;
	tilemap_t *m_spr2_tilemap;
	int m_palette_reverse_top;
	int m_palette_reverse_bot;

	UINT8 m_nmi_mask;
};


/*----------- defined in video/punchout.c -----------*/

WRITE8_HANDLER( punchout_bg_top_videoram_w );
WRITE8_HANDLER( punchout_bg_bot_videoram_w );
WRITE8_HANDLER( armwrest_fg_videoram_w );
WRITE8_HANDLER( punchout_spr1_videoram_w );
WRITE8_HANDLER( punchout_spr2_videoram_w );

VIDEO_START( punchout );
VIDEO_START( armwrest );
SCREEN_UPDATE( punchout );
SCREEN_UPDATE( armwrest );

DRIVER_INIT( punchout );
DRIVER_INIT( spnchout );
DRIVER_INIT( spnchotj );
DRIVER_INIT( armwrest );

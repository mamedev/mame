class goldstar_state : public driver_device
{
public:
	goldstar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_dataoffset;

	UINT8 *m_atrram;
	UINT8 *m_fg_atrram;
	UINT8 *m_fg_vidram;

	UINT8 *m_reel1_scroll;
	UINT8 *m_reel2_scroll;
	UINT8 *m_reel3_scroll;

	UINT8 *m_reel1_ram;
	UINT8 *m_reel2_ram;
	UINT8 *m_reel3_ram;

	/* reelx_attrram for unkch sets */
	UINT8 *m_reel1_attrram;
	UINT8 *m_reel2_attrram;
	UINT8 *m_reel3_attrram;
	UINT8 m_unkch_vidreg;

	tilemap_t *m_reel1_tilemap;
	tilemap_t *m_reel2_tilemap;
	tilemap_t *m_reel3_tilemap;

	int m_bgcolor;
	tilemap_t *m_fg_tilemap;
	UINT8 m_cmaster_girl_num;
	UINT8 m_cmaster_girl_pal;
	UINT8 m_cm_enable_reg;
	UINT8 m_cm_girl_scroll;
	UINT8 m_lucky8_nmi_enable;
	int m_tile_bank;

};


/*----------- defined in video/goldstar.c -----------*/

WRITE8_HANDLER( goldstar_reel1_ram_w );
WRITE8_HANDLER( goldstar_reel2_ram_w );
WRITE8_HANDLER( goldstar_reel3_ram_w );

WRITE8_HANDLER( unkch_reel1_attrram_w );
WRITE8_HANDLER( unkch_reel2_attrram_w );
WRITE8_HANDLER( unkch_reel3_attrram_w );

WRITE8_HANDLER( goldstar_fg_vidram_w );
WRITE8_HANDLER( goldstar_fg_atrram_w );
WRITE8_HANDLER( cm_girl_scroll_w );

WRITE8_HANDLER( goldstar_fa00_w );
WRITE8_HANDLER( cm_background_col_w );
WRITE8_HANDLER( cm_outport0_w );
VIDEO_START( goldstar );
VIDEO_START( bingowng );
VIDEO_START( cherrym );
VIDEO_START( unkch );
VIDEO_START( magical );
SCREEN_UPDATE( goldstar );
SCREEN_UPDATE( bingowng );
SCREEN_UPDATE( cmast91 );
SCREEN_UPDATE( amcoe1a );
SCREEN_UPDATE( unkch );
SCREEN_UPDATE( magical );

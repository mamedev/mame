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

	DECLARE_WRITE8_MEMBER(protection_w);
	DECLARE_READ8_MEMBER(protection_r);
	DECLARE_WRITE8_MEMBER(ncb3_port81_w);
	DECLARE_WRITE8_MEMBER(cm_outport1_w);
	DECLARE_WRITE8_MEMBER(lucky8_outport_w);
	DECLARE_WRITE8_MEMBER(magodds_outb850_w);
	DECLARE_WRITE8_MEMBER(magodds_outb860_w);
	DECLARE_WRITE8_MEMBER(ladylinr_outport_w);
	DECLARE_READ8_MEMBER(unkch_unk_r);
	DECLARE_WRITE8_MEMBER(unkcm_0x02_w);
	DECLARE_WRITE8_MEMBER(unkcm_0x03_w);
	DECLARE_WRITE8_MEMBER(unkcm_0x11_w);
	DECLARE_WRITE8_MEMBER(unkcm_0x12_w);
	DECLARE_READ8_MEMBER(fixedvalb4_r);
	DECLARE_READ8_MEMBER(fixedvala8_r);
	DECLARE_READ8_MEMBER(fixedval38_r);
	DECLARE_READ8_MEMBER(fixedvalea_r);
	DECLARE_READ8_MEMBER(fixedval68_r);
	DECLARE_READ8_MEMBER(fixedval58_r);
	DECLARE_READ8_MEMBER(fixedval80_r);
	DECLARE_READ8_MEMBER(fixedval96_r);
	DECLARE_READ8_MEMBER(fixedvalaa_r);
	DECLARE_READ8_MEMBER(fixedvalbe_r);
	DECLARE_READ8_MEMBER(fixedval90_r);
	DECLARE_READ8_MEMBER(fixedval84_r);
	DECLARE_READ8_MEMBER(fixedvalb2_r);
	DECLARE_READ8_MEMBER(fixedval48_r);
	DECLARE_READ8_MEMBER(fixedval09_r);
	DECLARE_READ8_MEMBER(fixedval74_r);
	DECLARE_READ8_MEMBER(fixedvale4_r);
	DECLARE_READ8_MEMBER(fixedvalc7_r);
	DECLARE_WRITE8_MEMBER(cm_girl_scroll_w);
	DECLARE_WRITE8_MEMBER(cm_outport0_w);
	DECLARE_WRITE8_MEMBER(goldstar_fg_vidram_w);
	DECLARE_WRITE8_MEMBER(goldstar_fg_atrram_w);
	DECLARE_WRITE8_MEMBER(goldstar_reel1_ram_w);
	DECLARE_WRITE8_MEMBER(goldstar_reel2_ram_w);
	DECLARE_WRITE8_MEMBER(goldstar_reel3_ram_w);
	DECLARE_WRITE8_MEMBER(unkch_reel1_attrram_w);
	DECLARE_WRITE8_MEMBER(unkch_reel2_attrram_w);
	DECLARE_WRITE8_MEMBER(unkch_reel3_attrram_w);
	DECLARE_WRITE8_MEMBER(goldstar_fa00_w);
	DECLARE_WRITE8_MEMBER(cm_background_col_w);
};


/*----------- defined in video/goldstar.c -----------*/




VIDEO_START( goldstar );
VIDEO_START( bingowng );
VIDEO_START( cherrym );
VIDEO_START( unkch );
VIDEO_START( magical );
SCREEN_UPDATE_IND16( goldstar );
SCREEN_UPDATE_IND16( bingowng );
SCREEN_UPDATE_IND16( cmast91 );
SCREEN_UPDATE_IND16( amcoe1a );
SCREEN_UPDATE_IND16( unkch );
SCREEN_UPDATE_IND16( magical );

/*************************************************************************

    various SNK triple Z80 games

*************************************************************************/

class snk_state : public driver_device
{
public:
	snk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_countryc_trackball;
	int m_last_value[2];
	int m_cp_count[2];

	int m_marvins_sound_busy_flag;
	// FIXME this should be initialised on machine reset
	int m_sound_status;

	UINT8 *m_spriteram;
	UINT8 *m_tx_videoram;
	UINT8 *m_fg_videoram;
	UINT8 *m_bg_videoram;

	tilemap_t *m_tx_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	int m_fg_scrollx;
	int m_fg_scrolly;
	int m_bg_scrollx;
	int m_bg_scrolly;
	int m_sp16_scrollx;
	int m_sp16_scrolly;
	int m_sp32_scrollx;
	int m_sp32_scrolly;
	UINT8 m_sprite_split_point;
	int m_num_sprites;
	int m_yscroll_mask;
	UINT32 m_bg_tile_offset;
	UINT32 m_tx_tile_offset;
	int m_is_psychos;

	UINT8 m_drawmode_table[16];
	UINT8 m_empty_tile[16*16];
	int m_hf_posy;
	int m_hf_posx;
	int m_tc16_posy;
	int m_tc16_posx;
	int m_tc32_posy;
	int m_tc32_posx;
	DECLARE_READ8_MEMBER(snk_cpuA_nmi_trigger_r);
	DECLARE_WRITE8_MEMBER(snk_cpuA_nmi_ack_w);
	DECLARE_READ8_MEMBER(snk_cpuB_nmi_trigger_r);
	DECLARE_WRITE8_MEMBER(snk_cpuB_nmi_ack_w);
	DECLARE_WRITE8_MEMBER(marvins_soundlatch_w);
	DECLARE_READ8_MEMBER(marvins_soundlatch_r);
	DECLARE_READ8_MEMBER(marvins_sound_nmi_ack_r);
	DECLARE_WRITE8_MEMBER(sgladiat_soundlatch_w);
	DECLARE_READ8_MEMBER(sgladiat_soundlatch_r);
	DECLARE_READ8_MEMBER(sgladiat_sound_nmi_ack_r);
	DECLARE_READ8_MEMBER(sgladiat_sound_irq_ack_r);
	DECLARE_WRITE8_MEMBER(snk_soundlatch_w);
	DECLARE_READ8_MEMBER(snk_sound_status_r);
	DECLARE_WRITE8_MEMBER(snk_sound_status_w);
	DECLARE_READ8_MEMBER(tnk3_cmdirq_ack_r);
	DECLARE_READ8_MEMBER(tnk3_ymirq_ack_r);
	DECLARE_READ8_MEMBER(tnk3_busy_clear_r);
	DECLARE_WRITE8_MEMBER(hardflags_scrollx_w);
	DECLARE_WRITE8_MEMBER(hardflags_scrolly_w);
	DECLARE_WRITE8_MEMBER(hardflags_scroll_msb_w);
	DECLARE_READ8_MEMBER(hardflags1_r);
	DECLARE_READ8_MEMBER(hardflags2_r);
	DECLARE_READ8_MEMBER(hardflags3_r);
	DECLARE_READ8_MEMBER(hardflags4_r);
	DECLARE_READ8_MEMBER(hardflags5_r);
	DECLARE_READ8_MEMBER(hardflags6_r);
	DECLARE_READ8_MEMBER(hardflags7_r);
	DECLARE_WRITE8_MEMBER(turbocheck16_1_w);
	DECLARE_WRITE8_MEMBER(turbocheck16_2_w);
	DECLARE_WRITE8_MEMBER(turbocheck32_1_w);
	DECLARE_WRITE8_MEMBER(turbocheck32_2_w);
	DECLARE_WRITE8_MEMBER(turbocheck_msb_w);
	DECLARE_READ8_MEMBER(turbocheck16_1_r);
	DECLARE_READ8_MEMBER(turbocheck16_2_r);
	DECLARE_READ8_MEMBER(turbocheck16_3_r);
	DECLARE_READ8_MEMBER(turbocheck16_4_r);
	DECLARE_READ8_MEMBER(turbocheck16_5_r);
	DECLARE_READ8_MEMBER(turbocheck16_6_r);
	DECLARE_READ8_MEMBER(turbocheck16_7_r);
	DECLARE_READ8_MEMBER(turbocheck16_8_r);
	DECLARE_READ8_MEMBER(turbocheck32_1_r);
	DECLARE_READ8_MEMBER(turbocheck32_2_r);
	DECLARE_READ8_MEMBER(turbocheck32_3_r);
	DECLARE_READ8_MEMBER(turbocheck32_4_r);
	DECLARE_WRITE8_MEMBER(athena_coin_counter_w);
	DECLARE_WRITE8_MEMBER(ikari_coin_counter_w);
	DECLARE_WRITE8_MEMBER(tdfever_coin_counter_w);
	DECLARE_WRITE8_MEMBER(countryc_trackball_w);
	DECLARE_WRITE8_MEMBER(snk_tx_videoram_w);
	DECLARE_WRITE8_MEMBER(marvins_fg_videoram_w);
	DECLARE_WRITE8_MEMBER(marvins_bg_videoram_w);
	DECLARE_WRITE8_MEMBER(snk_bg_videoram_w);
	DECLARE_WRITE8_MEMBER(snk_fg_scrollx_w);
	DECLARE_WRITE8_MEMBER(snk_fg_scrolly_w);
	DECLARE_WRITE8_MEMBER(snk_bg_scrollx_w);
	DECLARE_WRITE8_MEMBER(snk_bg_scrolly_w);
	DECLARE_WRITE8_MEMBER(snk_sp16_scrollx_w);
	DECLARE_WRITE8_MEMBER(snk_sp16_scrolly_w);
	DECLARE_WRITE8_MEMBER(snk_sp32_scrollx_w);
	DECLARE_WRITE8_MEMBER(snk_sp32_scrolly_w);
	DECLARE_WRITE8_MEMBER(snk_sprite_split_point_w);
	DECLARE_WRITE8_MEMBER(marvins_palette_bank_w);
	DECLARE_WRITE8_MEMBER(marvins_flipscreen_w);
	DECLARE_WRITE8_MEMBER(sgladiat_flipscreen_w);
	DECLARE_WRITE8_MEMBER(hal21_flipscreen_w);
	DECLARE_WRITE8_MEMBER(marvins_scroll_msb_w);
	DECLARE_WRITE8_MEMBER(jcross_scroll_msb_w);
	DECLARE_WRITE8_MEMBER(sgladiat_scroll_msb_w);
	DECLARE_WRITE8_MEMBER(aso_videoattrs_w);
	DECLARE_WRITE8_MEMBER(tnk3_videoattrs_w);
	DECLARE_WRITE8_MEMBER(aso_bg_bank_w);
	DECLARE_WRITE8_MEMBER(ikari_bg_scroll_msb_w);
	DECLARE_WRITE8_MEMBER(ikari_sp_scroll_msb_w);
	DECLARE_WRITE8_MEMBER(ikari_unknown_video_w);
	DECLARE_WRITE8_MEMBER(gwar_tx_bank_w);
	DECLARE_WRITE8_MEMBER(gwar_videoattrs_w);
	DECLARE_WRITE8_MEMBER(gwara_videoattrs_w);
	DECLARE_WRITE8_MEMBER(gwara_sp_scroll_msb_w);
	DECLARE_WRITE8_MEMBER(tdfever_sp_scroll_msb_w);
	DECLARE_WRITE8_MEMBER(tdfever_spriteram_w);
};


/*----------- defined in video/snk.c -----------*/

extern PALETTE_INIT( tnk3 );

extern VIDEO_START( marvins );
extern VIDEO_START( jcross );
extern VIDEO_START( sgladiat );
extern VIDEO_START( hal21 );
extern VIDEO_START( aso );
extern VIDEO_START( tnk3 );
extern VIDEO_START( ikari );
extern VIDEO_START( gwar );
extern VIDEO_START( psychos );
extern VIDEO_START( tdfever );

extern SCREEN_UPDATE_IND16( marvins );
extern SCREEN_UPDATE_IND16( tnk3 );
extern SCREEN_UPDATE_IND16( ikari );
extern SCREEN_UPDATE_IND16( gwar );
extern SCREEN_UPDATE_IND16( tdfever );




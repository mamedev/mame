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

extern SCREEN_UPDATE( marvins );
extern SCREEN_UPDATE( tnk3 );
extern SCREEN_UPDATE( ikari );
extern SCREEN_UPDATE( gwar );
extern SCREEN_UPDATE( tdfever );

extern WRITE8_HANDLER( snk_fg_scrollx_w );
extern WRITE8_HANDLER( snk_fg_scrolly_w );
extern WRITE8_HANDLER( snk_bg_scrollx_w );
extern WRITE8_HANDLER( snk_bg_scrolly_w );
extern WRITE8_HANDLER( snk_sp16_scrollx_w );
extern WRITE8_HANDLER( snk_sp16_scrolly_w );
extern WRITE8_HANDLER( snk_sp32_scrollx_w );
extern WRITE8_HANDLER( snk_sp32_scrolly_w );

extern WRITE8_HANDLER( snk_sprite_split_point_w );
extern WRITE8_HANDLER( marvins_palette_bank_w );
extern WRITE8_HANDLER( marvins_scroll_msb_w );
extern WRITE8_HANDLER( jcross_scroll_msb_w );
extern WRITE8_HANDLER( sgladiat_scroll_msb_w );
extern WRITE8_HANDLER( marvins_flipscreen_w );
extern WRITE8_HANDLER( sgladiat_flipscreen_w );
extern WRITE8_HANDLER( hal21_flipscreen_w );
extern WRITE8_HANDLER( tnk3_videoattrs_w );
extern WRITE8_HANDLER( aso_videoattrs_w );
extern WRITE8_HANDLER( aso_bg_bank_w );
extern WRITE8_HANDLER( ikari_bg_scroll_msb_w );
extern WRITE8_HANDLER( ikari_sp_scroll_msb_w );
extern WRITE8_HANDLER( ikari_unknown_video_w );
extern WRITE8_HANDLER( gwar_tx_bank_w );
extern WRITE8_HANDLER( gwar_videoattrs_w );
extern WRITE8_HANDLER( gwara_videoattrs_w );
extern WRITE8_HANDLER( gwara_sp_scroll_msb_w );
extern WRITE8_HANDLER( tdfever_sp_scroll_msb_w );
extern WRITE8_HANDLER( tdfever_spriteram_w );

extern WRITE8_HANDLER( snk_tx_videoram_w );
extern WRITE8_HANDLER( snk_bg_videoram_w );
extern WRITE8_HANDLER( marvins_fg_videoram_w );
extern WRITE8_HANDLER( marvins_bg_videoram_w );

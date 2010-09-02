/*************************************************************************

    various SNK triple Z80 games

*************************************************************************/

class snk_state : public driver_device
{
public:
	snk_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int countryc_trackball;
	int last_value[2];
	int cp_count[2];

	int marvins_sound_busy_flag;
	// FIXME this should be initialised on machine reset
	int sound_status;

	UINT8 *spriteram;
	UINT8 *tx_videoram;
	UINT8 *fg_videoram;
	UINT8 *bg_videoram;

	tilemap_t *tx_tilemap;
	tilemap_t *fg_tilemap;
	tilemap_t *bg_tilemap;
	int fg_scrollx, fg_scrolly, bg_scrollx, bg_scrolly;
	int sp16_scrollx, sp16_scrolly, sp32_scrollx, sp32_scrolly;
	UINT8 sprite_split_point;
	int num_sprites, yscroll_mask;
	UINT32 bg_tile_offset;
	UINT32 tx_tile_offset;
	int is_psychos;

	UINT8 drawmode_table[16];
	UINT8 empty_tile[16*16];
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

extern VIDEO_UPDATE( marvins );
extern VIDEO_UPDATE( tnk3 );
extern VIDEO_UPDATE( ikari );
extern VIDEO_UPDATE( gwar );
extern VIDEO_UPDATE( tdfever );

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

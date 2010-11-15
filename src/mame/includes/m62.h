class m62_state : public driver_device
{
public:
	m62_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *              spriteram;
	size_t               spriteram_size;

	UINT8 *              m62_tileram;
	UINT8 *              m62_textram;
	UINT8 *              scrollram;

	/* video-related */
	tilemap_t*             bg_tilemap;
	tilemap_t*             fg_tilemap;
	int                  flipscreen;

	const UINT8          *sprite_height_prom;
	INT32                m62_background_hscroll;
	INT32                m62_background_vscroll;
	UINT8                kidniki_background_bank;
	INT32                kidniki_text_vscroll;
	int                  ldrun3_topbottom_mask;
	INT32                spelunkr_palbank;

	/* misc */
	int                 ldrun2_bankswap;	//ldrun2
	int                 bankcontrol[2]; 	//ldrun2
};


/*----------- defined in video/m62.c -----------*/

WRITE8_HANDLER( m62_tileram_w );
WRITE8_HANDLER( m62_textram_w );
WRITE8_HANDLER( m62_flipscreen_w );
WRITE8_HANDLER( m62_hscroll_low_w );
WRITE8_HANDLER( m62_hscroll_high_w );
WRITE8_HANDLER( m62_vscroll_low_w );
WRITE8_HANDLER( m62_vscroll_high_w );

WRITE8_HANDLER( horizon_scrollram_w );
WRITE8_HANDLER( kidniki_text_vscroll_low_w );
WRITE8_HANDLER( kidniki_text_vscroll_high_w );
WRITE8_HANDLER( kidniki_background_bank_w );
WRITE8_HANDLER( kungfum_tileram_w );
WRITE8_HANDLER( ldrun3_topbottom_mask_w );
WRITE8_HANDLER( spelunkr_palbank_w );
WRITE8_HANDLER( spelunk2_gfxport_w );

PALETTE_INIT( m62 );
PALETTE_INIT( lotlot );
PALETTE_INIT( battroad );
PALETTE_INIT( spelunk2 );

VIDEO_START( battroad );
VIDEO_START( horizon );
VIDEO_START( kidniki );
VIDEO_START( kungfum );
VIDEO_START( ldrun );
VIDEO_START( ldrun2 );
VIDEO_START( ldrun4 );
VIDEO_START( lotlot );
VIDEO_START( spelunkr );
VIDEO_START( spelunk2 );
VIDEO_START( youjyudn );

VIDEO_UPDATE( battroad );
VIDEO_UPDATE( horizon );
VIDEO_UPDATE( kidniki );
VIDEO_UPDATE( kungfum );
VIDEO_UPDATE( ldrun );
VIDEO_UPDATE( ldrun3 );
VIDEO_UPDATE( ldrun4 );
VIDEO_UPDATE( lotlot );
VIDEO_UPDATE( spelunkr );
VIDEO_UPDATE( spelunk2 );
VIDEO_UPDATE( youjyudn );

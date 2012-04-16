class cclimber_state : public driver_device
{
public:
	cclimber_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_bigsprite_videoram(*this, "bigspriteram"),
		m_videoram(*this, "videoram"),
		m_column_scroll(*this, "column_scroll"),
		m_spriteram(*this, "spriteram"),
		m_bigsprite_control(*this, "bigspritectrl"),
		m_colorram(*this, "colorram"),
		m_flip_screen(*this, "flip_screen"),
		m_swimmer_side_background_enabled(*this, "sidebg_enable"),
		m_swimmer_palettebank(*this, "palettebank"),
		m_swimmer_background_color(*this, "bgcolor"),
		m_toprollr_bg_videoram(*this, "bg_videoram"),
		m_toprollr_bg_coloram(*this, "bg_coloram"){ }

	required_shared_ptr<UINT8> m_bigsprite_videoram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_column_scroll;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_bigsprite_control;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_flip_screen;
	optional_shared_ptr<UINT8> m_swimmer_side_background_enabled;
	optional_shared_ptr<UINT8> m_swimmer_palettebank;
	optional_shared_ptr<UINT8> m_swimmer_background_color;
	optional_shared_ptr<UINT8> m_toprollr_bg_videoram;
	optional_shared_ptr<UINT8> m_toprollr_bg_coloram;
	
	UINT8 m_yamato_p0;
	UINT8 m_yamato_p1;
	UINT8 m_toprollr_rombank;
	UINT8 m_nmi_mask;
	tilemap_t *m_pf_tilemap;
	tilemap_t *m_bs_tilemap;
	tilemap_t *m_toproller_bg_tilemap;
	DECLARE_WRITE8_MEMBER(swimmer_sh_soundlatch_w);
	DECLARE_WRITE8_MEMBER(yamato_p0_w);
	DECLARE_WRITE8_MEMBER(yamato_p1_w);
	DECLARE_READ8_MEMBER(yamato_p0_r);
	DECLARE_READ8_MEMBER(yamato_p1_r);
	DECLARE_WRITE8_MEMBER(toprollr_rombank_w);
	DECLARE_WRITE8_MEMBER(nmi_mask_w);
	DECLARE_WRITE8_MEMBER(cclimber_colorram_w);
	DECLARE_WRITE8_MEMBER(cannonb_flip_screen_w);
};


/*----------- defined in machine/cclimber.c -----------*/

DRIVER_INIT( cclimber );
DRIVER_INIT( cclimberj );
DRIVER_INIT( cannonb );
DRIVER_INIT( cannonb2 );
DRIVER_INIT( ckongb );

/*----------- defined in video/cclimber.c -----------*/


PALETTE_INIT( cclimber );
VIDEO_START( cclimber );
SCREEN_UPDATE_IND16( cclimber );

PALETTE_INIT( swimmer );
VIDEO_START( swimmer );
SCREEN_UPDATE_IND16( swimmer );

PALETTE_INIT( yamato );
SCREEN_UPDATE_IND16( yamato );

PALETTE_INIT( toprollr );
VIDEO_START( toprollr );
SCREEN_UPDATE_IND16( toprollr );

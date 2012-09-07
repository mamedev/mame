
class tumbleb_state : public driver_device
{
public:
	tumbleb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_mainram(*this, "mainram"),
		m_spriteram(*this, "spriteram"),
		m_pf1_data(*this, "pf1_data"),
		m_pf2_data(*this, "pf2_data"),
		m_control(*this, "control"){ }

	/* memory pointers */
	optional_shared_ptr<UINT16> m_mainram;
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_pf1_data;
	required_shared_ptr<UINT16> m_pf2_data;
	optional_shared_ptr<UINT16> m_control;
//  UINT16 *    m_paletteram;    // currently this uses generic palette handling

	/* misc */
	int         m_music_command;
	int         m_music_bank;
	int         m_music_is_playing;

	/* video-related */
	tilemap_t   *m_pf1_tilemap;
	tilemap_t   *m_pf1_alt_tilemap;
	tilemap_t   *m_pf2_tilemap;
	tilemap_t   *m_pf2_alt_tilemap;
	UINT16      m_control_0[8];
	int         m_flipscreen;
	UINT16      m_tilebank;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_oki;
	UINT8 m_semicom_prot_offset;
	UINT16 m_protbase;
	DECLARE_WRITE16_MEMBER(tumblepb_oki_w);
	DECLARE_READ16_MEMBER(tumblepb_prot_r);
	DECLARE_READ16_MEMBER(tumblepopb_controls_r);
	DECLARE_READ16_MEMBER(semibase_unknown_r);
	DECLARE_WRITE16_MEMBER(jumpkids_sound_w);
	DECLARE_WRITE16_MEMBER(semicom_soundcmd_w);
	DECLARE_WRITE8_MEMBER(oki_sound_bank_w);
	DECLARE_WRITE8_MEMBER(jumpkids_oki_bank_w);
	DECLARE_READ8_MEMBER(prot_io_r);
	DECLARE_WRITE8_MEMBER(prot_io_w);
	DECLARE_READ16_MEMBER(bcstory_1a0_read);
	DECLARE_WRITE16_MEMBER(bcstory_tilebank_w);
	DECLARE_WRITE16_MEMBER(chokchok_tilebank_w);
	DECLARE_WRITE16_MEMBER(wlstar_tilebank_w);
	DECLARE_WRITE16_MEMBER(suprtrio_tilebank_w);
	DECLARE_WRITE16_MEMBER(tumblepb_pf1_data_w);
	DECLARE_WRITE16_MEMBER(tumblepb_pf2_data_w);
	DECLARE_WRITE16_MEMBER(fncywld_pf1_data_w);
	DECLARE_WRITE16_MEMBER(fncywld_pf2_data_w);
	DECLARE_WRITE16_MEMBER(tumblepb_control_0_w);
	DECLARE_WRITE16_MEMBER(pangpang_pf1_data_w);
	DECLARE_WRITE16_MEMBER(pangpang_pf2_data_w);
	DECLARE_WRITE16_MEMBER(tumbleb2_soundmcu_w);
	DECLARE_DRIVER_INIT(dquizgo);
	DECLARE_DRIVER_INIT(jumpkids);
	DECLARE_DRIVER_INIT(htchctch);
	DECLARE_DRIVER_INIT(wlstar);
	DECLARE_DRIVER_INIT(suprtrio);
	DECLARE_DRIVER_INIT(tumblepb);
	DECLARE_DRIVER_INIT(bcstory);
	DECLARE_DRIVER_INIT(wondl96);
	DECLARE_DRIVER_INIT(tumbleb2);
	DECLARE_DRIVER_INIT(chokchok);
	DECLARE_DRIVER_INIT(fncywld);
	DECLARE_DRIVER_INIT(magicbal);
	TILEMAP_MAPPER_MEMBER(tumblep_scan);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_fncywld_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_fncywld_bg2_tile_info);
	TILE_GET_INFO_MEMBER(get_fncywld_fg_tile_info);
	TILE_GET_INFO_MEMBER(pangpang_get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(pangpang_get_bg2_tile_info);
	TILE_GET_INFO_MEMBER(pangpang_get_fg_tile_info);
};

/*----------- defined in video/tumbleb.c -----------*/



VIDEO_START( tumblepb );
VIDEO_START( fncywld );
VIDEO_START( sdfight );
VIDEO_START( suprtrio );
VIDEO_START( pangpang );

SCREEN_UPDATE_IND16( tumblepb );
SCREEN_UPDATE_IND16( jumpkids );
SCREEN_UPDATE_IND16( fncywld );
SCREEN_UPDATE_IND16( semicom );
SCREEN_UPDATE_IND16( semicom_altoffsets );
SCREEN_UPDATE_IND16( bcstory );
SCREEN_UPDATE_IND16(semibase );
SCREEN_UPDATE_IND16( suprtrio );
SCREEN_UPDATE_IND16( pangpang );
SCREEN_UPDATE_IND16( sdfight );

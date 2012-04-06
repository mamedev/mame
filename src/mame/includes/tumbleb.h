
class tumbleb_state : public driver_device
{
public:
	tumbleb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *    m_pf1_data;
	UINT16 *    m_pf2_data;
	UINT16 *    m_mainram;
	UINT16 *    m_spriteram;
	UINT16 *    m_control;
	size_t      m_spriteram_size;
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
	DECLARE_WRITE16_MEMBER(jumppop_sound_w);
	DECLARE_READ16_MEMBER(tumblepopb_controls_r);
	DECLARE_READ16_MEMBER(semibase_unknown_r);
	DECLARE_WRITE16_MEMBER(jumpkids_sound_w);
	DECLARE_WRITE16_MEMBER(semicom_soundcmd_w);
	DECLARE_WRITE8_MEMBER(oki_sound_bank_w);
	DECLARE_WRITE8_MEMBER(jumppop_z80_bank_w);
	DECLARE_READ8_MEMBER(jumppop_z80latch_r);
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
};

/*----------- defined in video/tumbleb.c -----------*/



VIDEO_START( tumblepb );
VIDEO_START( fncywld );
VIDEO_START( jumppop );
VIDEO_START( sdfight );
VIDEO_START( suprtrio );
VIDEO_START( pangpang );

SCREEN_UPDATE_IND16( tumblepb );
SCREEN_UPDATE_IND16( jumpkids );
SCREEN_UPDATE_IND16( fncywld );
SCREEN_UPDATE_IND16( jumppop );
SCREEN_UPDATE_IND16( semicom );
SCREEN_UPDATE_IND16( semicom_altoffsets );
SCREEN_UPDATE_IND16( bcstory );
SCREEN_UPDATE_IND16(semibase );
SCREEN_UPDATE_IND16( suprtrio );
SCREEN_UPDATE_IND16( pangpang );
SCREEN_UPDATE_IND16( sdfight );

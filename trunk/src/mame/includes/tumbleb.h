
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
	int         m_sprite_xoffset;
	int         m_sprite_yoffset;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_oki;
	UINT8 m_semicom_prot_offset;
	UINT16 m_protbase;
};

/*----------- defined in video/tumbleb.c -----------*/

WRITE16_HANDLER( tumblepb_pf1_data_w );
WRITE16_HANDLER( tumblepb_pf2_data_w );
WRITE16_HANDLER( fncywld_pf1_data_w );
WRITE16_HANDLER( fncywld_pf2_data_w );
WRITE16_HANDLER( tumblepb_control_0_w );
WRITE16_HANDLER( pangpang_pf1_data_w );
WRITE16_HANDLER( pangpang_pf2_data_w );

WRITE16_HANDLER( bcstory_tilebank_w );
WRITE16_HANDLER( suprtrio_tilebank_w );
WRITE16_HANDLER( chokchok_tilebank_w );
WRITE16_HANDLER( wlstar_tilebank_w );

VIDEO_START( tumblepb );
VIDEO_START( fncywld );
VIDEO_START( jumppop );
VIDEO_START( sdfight );
VIDEO_START( suprtrio );
VIDEO_START( pangpang );

SCREEN_UPDATE( tumblepb );
SCREEN_UPDATE( jumpkids );
SCREEN_UPDATE( fncywld );
SCREEN_UPDATE( jumppop );
SCREEN_UPDATE( semicom );
SCREEN_UPDATE( semicom_altoffsets );
SCREEN_UPDATE( bcstory );
SCREEN_UPDATE(semibase );
SCREEN_UPDATE( suprtrio );
SCREEN_UPDATE( pangpang );
SCREEN_UPDATE( sdfight );


class tumbleb_state : public driver_device
{
public:
	tumbleb_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *    pf1_data;
	UINT16 *    pf2_data;
	UINT16 *    mainram;
	UINT16 *    spriteram;
	UINT16 *    control;
	size_t      spriteram_size;
//  UINT16 *    paletteram;    // currently this uses generic palette handling

	/* misc */
	int         music_command;
	int         music_bank;
	int         music_is_playing;

	/* video-related */
	tilemap_t   *pf1_tilemap,*pf1_alt_tilemap,*pf2_tilemap,*pf2_alt_tilemap;
	UINT16      control_0[8];
	int         flipscreen;
	UINT16      tilebank;
	int         sprite_xoffset;
	int         sprite_yoffset;

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
	device_t *oki;
	UINT8 semicom_prot_offset;
	UINT16 protbase;
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

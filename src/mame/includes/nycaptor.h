
class nycaptor_state : public driver_device
{
public:
	nycaptor_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *      sharedram;
	UINT8 *      scrlram;
	UINT8 *      videoram;
	UINT8 *      spriteram;
	size_t       videoram_size;

	/* video-related */
	tilemap_t *bg_tilemap;
	int char_bank, palette_bank, gfxctrl;

	/* mcu */
	UINT8 from_main, from_mcu;
	int mcu_sent, main_sent;
	UINT8 port_a_in, port_a_out, ddr_a;
	UINT8 port_b_in, port_b_out, ddr_b;
	UINT8 port_c_in, port_c_out, ddr_c;

	/* misc */
	int generic_control_reg;
	int sound_nmi_enable, pending_nmi;
	UINT8 snd_data;
	int vol_ctrl[16];
	int  gametype;

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
	device_t *subcpu;
	device_t *mcu;
};


/*----------- defined in machine/nycaptor.c -----------*/

READ8_HANDLER( nycaptor_mcu_r );
READ8_HANDLER( nycaptor_mcu_status_r1 );
READ8_HANDLER( nycaptor_mcu_status_r2 );
READ8_HANDLER( nycaptor_68705_port_c_r );
READ8_HANDLER( nycaptor_68705_port_b_r );
READ8_HANDLER( nycaptor_68705_port_a_r );

WRITE8_HANDLER( nycaptor_mcu_w );
WRITE8_HANDLER( nycaptor_68705_port_a_w );
WRITE8_HANDLER( nycaptor_68705_port_b_w );
WRITE8_HANDLER( nycaptor_68705_port_c_w );
WRITE8_HANDLER( nycaptor_68705_ddr_a_w );
WRITE8_HANDLER( nycaptor_68705_ddr_b_w );
WRITE8_HANDLER( nycaptor_68705_ddr_c_w );


/*----------- defined in video/nycaptor.c -----------*/

READ8_HANDLER( nycaptor_videoram_r );
READ8_HANDLER( nycaptor_spriteram_r );
READ8_HANDLER( nycaptor_palette_r );
READ8_HANDLER( nycaptor_gfxctrl_r );
READ8_HANDLER( nycaptor_scrlram_r );

WRITE8_HANDLER( nycaptor_videoram_w );
WRITE8_HANDLER( nycaptor_spriteram_w );
WRITE8_HANDLER( nycaptor_palette_w );
WRITE8_HANDLER( nycaptor_gfxctrl_w );
WRITE8_HANDLER( nycaptor_scrlram_w );

VIDEO_START( nycaptor );
VIDEO_UPDATE( nycaptor );

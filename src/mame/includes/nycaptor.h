
class nycaptor_state : public driver_device
{
public:
	nycaptor_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *      m_sharedram;
	UINT8 *      m_scrlram;
	UINT8 *      m_videoram;
	UINT8 *      m_spriteram;
	size_t       m_videoram_size;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	int m_char_bank;
	int m_palette_bank;
	int m_gfxctrl;

	/* mcu */
	UINT8 m_from_main;
	UINT8 m_from_mcu;
	int m_mcu_sent;
	int m_main_sent;
	UINT8 m_port_a_in;
	UINT8 m_port_a_out;
	UINT8 m_ddr_a;
	UINT8 m_port_b_in;
	UINT8 m_port_b_out;
	UINT8 m_ddr_b;
	UINT8 m_port_c_in;
	UINT8 m_port_c_out;
	UINT8 m_ddr_c;

	/* misc */
	int m_generic_control_reg;
	int m_sound_nmi_enable;
	int m_pending_nmi;
	UINT8 m_snd_data;
	int m_vol_ctrl[16];
	int  m_gametype;
	int m_mask;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_subcpu;
	device_t *m_mcu;
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
SCREEN_UPDATE( nycaptor );

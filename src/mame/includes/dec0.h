class dec0_state : public driver_device
{
public:
	dec0_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_automat_adpcm_byte;
	int m_automat_msm5205_vclk_toggle;
	UINT16 *m_ram;
	UINT8 *m_robocop_shared_ram;

	int m_GAME;
	int m_i8751_return;
	int m_i8751_command;
	int m_slyspy_state;
	int m_share[0xff];
	int m_hippodrm_msb;
	int m_hippodrm_lsb;
	UINT8 m_i8751_ports[4];

	UINT16 *m_spriteram;
	UINT16 *m_buffered_spriteram;
	UINT16 m_pri;
	DECLARE_WRITE16_MEMBER(dec0_control_w);
	DECLARE_WRITE16_MEMBER(automat_control_w);
	DECLARE_WRITE16_MEMBER(slyspy_control_w);
	DECLARE_WRITE16_MEMBER(midres_sound_w);
	DECLARE_WRITE16_MEMBER(unmapped_w);
	DECLARE_WRITE8_MEMBER(automat_adpcm_w);
};


/*----------- defined in video/dec0.c -----------*/

/* Video emulation definitions */
VIDEO_START( dec0 );
VIDEO_START( dec0_nodma );
SCREEN_UPDATE_IND16( hbarrel );
SCREEN_UPDATE_IND16( baddudes );
SCREEN_UPDATE_IND16( birdtry );
SCREEN_UPDATE_IND16( robocop );
SCREEN_UPDATE_IND16( hippodrm );
SCREEN_UPDATE_IND16( slyspy );
SCREEN_UPDATE_IND16( midres );

WRITE16_HANDLER( dec0_priority_w );
WRITE16_HANDLER( dec0_update_sprites_w );

WRITE16_HANDLER( dec0_paletteram_rg_w );
WRITE16_HANDLER( dec0_paletteram_b_w );

READ8_HANDLER( dec0_pf3_data_8bit_r );
WRITE8_HANDLER( dec0_pf3_data_8bit_w );
WRITE8_HANDLER( dec0_pf3_control_8bit_w );

/*----------- defined in machine/dec0.c -----------*/

READ16_HANDLER( dec0_controls_r );
READ16_HANDLER( dec0_rotary_r );
READ16_HANDLER( midres_controls_r );
READ16_HANDLER( slyspy_controls_r );

DRIVER_INIT( slyspy );
DRIVER_INIT( hippodrm );
DRIVER_INIT( robocop );
DRIVER_INIT( baddudes );
DRIVER_INIT( hbarrel );
DRIVER_INIT( birdtry );

extern void dec0_i8751_write(running_machine &machine, int data);
extern void dec0_i8751_reset(running_machine &machine);
READ8_HANDLER( hippodrm_prot_r );
WRITE8_HANDLER( dec0_mcu_port_w );
READ8_HANDLER( dec0_mcu_port_r );
WRITE8_HANDLER( hippodrm_prot_w );
READ8_HANDLER( hippodrm_shared_r );
WRITE8_HANDLER( hippodrm_shared_w );


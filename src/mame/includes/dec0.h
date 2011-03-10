class dec0_state : public driver_device
{
public:
	dec0_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 automat_adpcm_byte;
	int automat_msm5205_vclk_toggle;
	UINT16 *ram;
	UINT8 *robocop_shared_ram;
	UINT16 *pf1_rowscroll;
	UINT16 *pf2_rowscroll;
	UINT16 *pf3_rowscroll;
	UINT16 *pf1_colscroll;
	UINT16 *pf2_colscroll;
	UINT16 *pf3_colscroll;
	UINT16 *pf1_data;
	UINT16 *pf2_data;
	UINT16 *pf3_data;
	int GAME;
	int i8751_return;
	int i8751_command;
	int slyspy_state;
	int share[0xff];
	int hippodrm_msb;
	int hippodrm_lsb;
	UINT8 i8751_ports[4];
	tilemap_t *pf1_tilemap_0;
	tilemap_t *pf1_tilemap_1;
	tilemap_t *pf1_tilemap_2;
	tilemap_t *pf2_tilemap_0;
	tilemap_t *pf2_tilemap_1;
	tilemap_t *pf2_tilemap_2;
	tilemap_t *pf3_tilemap_0;
	tilemap_t *pf3_tilemap_1;
	tilemap_t *pf3_tilemap_2;
	UINT16 pf1_control_0[4];
	UINT16 pf1_control_1[4];
	UINT16 pf2_control_0[4];
	UINT16 pf2_control_1[4];
	UINT16 pf3_control_0[4];
	UINT16 pf3_control_1[4];
	UINT16 *spriteram;
	UINT16 *buffered_spriteram;
	UINT16 pri;
	int buffer[0x20];
};


/*----------- defined in video/dec0.c -----------*/

/* Video emulation definitions */
VIDEO_START( dec0 );
VIDEO_START( dec0_nodma );
SCREEN_UPDATE( hbarrel );
SCREEN_UPDATE( baddudes );
SCREEN_UPDATE( birdtry );
SCREEN_UPDATE( robocop );
SCREEN_UPDATE( hippodrm );
SCREEN_UPDATE( slyspy );
SCREEN_UPDATE( midres );


WRITE16_HANDLER( dec0_pf1_control_0_w );
WRITE16_HANDLER( dec0_pf1_control_1_w );
WRITE16_HANDLER( dec0_pf1_data_w );
WRITE16_HANDLER( dec0_pf2_control_0_w );
WRITE16_HANDLER( dec0_pf2_control_1_w );
WRITE16_HANDLER( dec0_pf2_data_w );
WRITE16_HANDLER( dec0_pf3_control_0_w );
WRITE16_HANDLER( dec0_pf3_control_1_w );
WRITE16_HANDLER( dec0_pf3_data_w );
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
READ16_HANDLER( slyspy_protection_r );
WRITE16_HANDLER( slyspy_state_w );
READ16_HANDLER( slyspy_state_r );
WRITE16_HANDLER( slyspy_240000_w );
WRITE16_HANDLER( slyspy_242000_w );
WRITE16_HANDLER( slyspy_246000_w );
WRITE16_HANDLER( slyspy_248000_w );
WRITE16_HANDLER( slyspy_24c000_w );
WRITE16_HANDLER( slyspy_24e000_w );

DRIVER_INIT( slyspy );
DRIVER_INIT( hippodrm );
DRIVER_INIT( robocop );
DRIVER_INIT( baddudes );
DRIVER_INIT( hbarrel );
DRIVER_INIT( birdtry );

extern void dec0_i8751_write(running_machine *machine, int data);
extern void dec0_i8751_reset(running_machine *machine);
READ8_HANDLER( hippodrm_prot_r );
WRITE8_HANDLER( dec0_mcu_port_w );
READ8_HANDLER( dec0_mcu_port_r );
WRITE8_HANDLER( hippodrm_prot_w );
READ8_HANDLER( hippodrm_shared_r );
WRITE8_HANDLER( hippodrm_shared_w );


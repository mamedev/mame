class cischeat_state : public driver_device
{
public:
	cischeat_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *scrollram[3];
	UINT16 *objectram;
	UINT16 *vregs;
	UINT16 *ram;
	tilemap_t *tmap[3];
	tilemap_t *tilemap[3][2][4];
	int scrollx[3], scrolly[3];
	int active_layers;
	int bits_per_color_code;
	int scroll_flag[3];

	UINT16 *rom_1;
	UINT16 *rom_2;
	UINT16 *rom_3;
	UINT16 *sharedram1;
	UINT16 *sharedram2;
	int prev;
	int armold;
	UINT16 scudhamm_motor_command;
	UINT16 *roadram[2];
	UINT16 *f1gpstr2_ioready;
	int ip_select;
	int shift_ret;
	UINT8 drawmode_table[16];
	int debugsprites;
	int show_unknown;
	UINT16 *spriteram;
};


/*----------- defined in drivers/cischeat.c -----------*/

READ16_HANDLER( scudhamm_motor_pos_r );
READ16_HANDLER( scudhamm_motor_status_r );
READ16_HANDLER( scudhamm_analog_r );


/*----------- defined in video/cischeat.c -----------*/

WRITE16_HANDLER( cischeat_scrollram_0_w );
WRITE16_HANDLER( cischeat_scrollram_1_w );
WRITE16_HANDLER( cischeat_scrollram_2_w );

READ16_HANDLER( bigrun_vregs_r );
READ16_HANDLER( cischeat_vregs_r );
READ16_HANDLER( f1gpstar_vregs_r );
READ16_HANDLER( f1gpstr2_vregs_r );
READ16_HANDLER( wildplt_vregs_r );

WRITE16_HANDLER( bigrun_vregs_w );
WRITE16_HANDLER( cischeat_vregs_w );
WRITE16_HANDLER( f1gpstar_vregs_w );
WRITE16_HANDLER( f1gpstr2_vregs_w );
WRITE16_HANDLER( scudhamm_vregs_w );

CUSTOM_INPUT( cischeat_shift_r );

VIDEO_START( bigrun );
VIDEO_START( cischeat );
VIDEO_START( f1gpstar );

SCREEN_UPDATE( bigrun );
SCREEN_UPDATE( cischeat );
SCREEN_UPDATE( f1gpstar );
SCREEN_UPDATE( scudhamm );

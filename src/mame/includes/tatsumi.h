class tatsumi_state : public driver_device
{
public:
	tatsumi_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *videoram;
	UINT16 *cyclwarr_cpua_ram;
	UINT16 *cyclwarr_cpub_ram;
	UINT16 bigfight_a20000[8];
	UINT16 bigfight_a60000[2];
	UINT16 *apache3_g_ram;
	UINT16 bigfight_a40000[2];
	UINT8 *rom_sprite_lookup1;
	UINT8 *rom_sprite_lookup2;
	UINT8 *rom_clut0;
	UINT8 *rom_clut1;
	UINT16 *roundup5_d0000_ram;
	UINT16 *roundup5_e0000_ram;
	UINT16 *roundup5_unknown0;
	UINT16 *roundup5_unknown1;
	UINT16 *roundup5_unknown2;
	UINT16 *_68k_ram;
	UINT8 *apache3_z80_ram;
	UINT16 control_word;
	UINT16 apache3_rotate_ctrl[12];
	UINT16* sprite_control_ram;
	UINT16 *cyclwarr_videoram0;
	UINT16 *cyclwarr_videoram1;
	UINT16 *roundup_r_ram;
	UINT16 *roundup_p_ram;
	UINT16 *roundup_l_ram;
	UINT16 last_control;
	UINT8 apache3_adc;
	int apache3_rot_idx;
	tilemap_t *tx_layer;
	tilemap_t *layer0;
	tilemap_t *layer1;
	tilemap_t *layer2;
	tilemap_t *layer3;
	bitmap_t *temp_bitmap;
	UINT8 *apache3_road_x_ram;
	UINT8 apache3_road_z;
	UINT16* roundup5_vram;
	UINT16 bigfight_bank;
	UINT16 bigfight_last_bank;
	UINT8 roundupt_crt_selected_reg;
	UINT8 roundupt_crt_reg[64];
	UINT8* shadow_pen_array;
};


/*----------- defined in machine/tatsumi.c -----------*/

READ16_HANDLER( apache3_bank_r );
WRITE16_HANDLER( apache3_bank_w );
WRITE16_HANDLER( apache3_z80_ctrl_w );
READ16_HANDLER( apache3_v30_v20_r );
WRITE16_HANDLER( apache3_v30_v20_w );
READ16_HANDLER( roundup_v30_z80_r );
WRITE16_HANDLER( roundup_v30_z80_w );
READ16_HANDLER( tatsumi_v30_68000_r );
WRITE16_HANDLER( tatsumi_v30_68000_w ) ;
READ16_HANDLER( apache3_z80_r );
WRITE16_HANDLER( apache3_z80_w );
READ8_HANDLER( apache3_adc_r );
WRITE8_HANDLER( apache3_adc_w );
WRITE16_HANDLER( apache3_rotate_w );
WRITE16_HANDLER( cyclwarr_control_w );
READ16_HANDLER( cyclwarr_control_r );
WRITE16_HANDLER( roundup5_control_w );
WRITE16_HANDLER( roundup5_d0000_w );
WRITE16_HANDLER( roundup5_e0000_w );

READ8_DEVICE_HANDLER(tatsumi_hack_ym2151_r);
READ8_DEVICE_HANDLER(tatsumi_hack_oki_r);


void tatsumi_reset(running_machine *machine);

/*----------- defined in video/tatsumi.c -----------*/

WRITE16_HANDLER( roundup5_palette_w );
WRITE16_HANDLER( tatsumi_sprite_control_w );
WRITE16_HANDLER( roundup5_text_w );
WRITE16_HANDLER( roundup5_crt_w );
READ16_HANDLER( cyclwarr_videoram0_r );
WRITE16_HANDLER( cyclwarr_videoram0_w );
READ16_HANDLER( cyclwarr_videoram1_r );
WRITE16_HANDLER( cyclwarr_videoram1_w );
READ16_HANDLER( roundup5_vram_r );
WRITE16_HANDLER( roundup5_vram_w );
WRITE16_HANDLER( apache3_palette_w );
WRITE8_HANDLER( apache3_road_x_w );
WRITE16_HANDLER( apache3_road_z_w );


VIDEO_START( apache3 );
VIDEO_START( roundup5 );
VIDEO_START( cyclwarr );
VIDEO_START( bigfight );

SCREEN_UPDATE( roundup5 );
SCREEN_UPDATE( apache3 );
SCREEN_UPDATE( cyclwarr );
SCREEN_UPDATE( bigfight );


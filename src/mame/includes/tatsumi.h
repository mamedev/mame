class tatsumi_state : public driver_device
{
public:
	tatsumi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_videoram;
	UINT16 *m_cyclwarr_cpua_ram;
	UINT16 *m_cyclwarr_cpub_ram;
	UINT16 m_bigfight_a20000[8];
	UINT16 m_bigfight_a60000[2];
	UINT16 *m_apache3_g_ram;
	UINT16 m_bigfight_a40000[2];
	UINT8 *m_rom_sprite_lookup1;
	UINT8 *m_rom_sprite_lookup2;
	UINT8 *m_rom_clut0;
	UINT8 *m_rom_clut1;
	UINT16 *m_roundup5_d0000_ram;
	UINT16 *m_roundup5_e0000_ram;
	UINT16 *m_roundup5_unknown0;
	UINT16 *m_roundup5_unknown1;
	UINT16 *m_roundup5_unknown2;
	UINT16 *m_68k_ram;
	UINT8 *m_apache3_z80_ram;
	UINT16 m_control_word;
	UINT16 m_apache3_rotate_ctrl[12];
	UINT16* m_sprite_control_ram;
	UINT16 *m_cyclwarr_videoram0;
	UINT16 *m_cyclwarr_videoram1;
	UINT16 *m_roundup_r_ram;
	UINT16 *m_roundup_p_ram;
	UINT16 *m_roundup_l_ram;
	UINT16 m_last_control;
	UINT8 m_apache3_adc;
	int m_apache3_rot_idx;
	tilemap_t *m_tx_layer;
	tilemap_t *m_layer0;
	tilemap_t *m_layer1;
	tilemap_t *m_layer2;
	tilemap_t *m_layer3;
	bitmap_rgb32 m_temp_bitmap;
	UINT8 *m_apache3_road_x_ram;
	UINT8 m_apache3_road_z;
	UINT16* m_roundup5_vram;
	UINT16 m_bigfight_bank;
	UINT16 m_bigfight_last_bank;
	UINT8 m_roundupt_crt_selected_reg;
	UINT8 m_roundupt_crt_reg[64];
	UINT8* m_shadow_pen_array;
	UINT16 *m_spriteram;
	DECLARE_READ16_MEMBER(cyclwarr_cpu_bb_r);
	DECLARE_WRITE16_MEMBER(cyclwarr_cpu_bb_w);
	DECLARE_READ16_MEMBER(cyclwarr_palette_r);
	DECLARE_READ16_MEMBER(cyclwarr_sprite_r);
	DECLARE_WRITE16_MEMBER(cyclwarr_sprite_w);
	DECLARE_WRITE16_MEMBER(bigfight_a20000_w);
	DECLARE_WRITE16_MEMBER(bigfight_a40000_w);
	DECLARE_WRITE16_MEMBER(bigfight_a60000_w);
	DECLARE_READ16_MEMBER(cyclwarr_input_r);
	DECLARE_READ16_MEMBER(cyclwarr_input2_r);
	DECLARE_WRITE16_MEMBER(cyclwarr_sound_w);
	DECLARE_READ16_MEMBER(apache3_bank_r);
	DECLARE_WRITE16_MEMBER(apache3_bank_w);
	DECLARE_WRITE16_MEMBER(apache3_z80_ctrl_w);
	DECLARE_READ16_MEMBER(apache3_v30_v20_r);
	DECLARE_WRITE16_MEMBER(apache3_v30_v20_w);
	DECLARE_READ16_MEMBER(apache3_z80_r);
	DECLARE_WRITE16_MEMBER(apache3_z80_w);
	DECLARE_READ8_MEMBER(apache3_adc_r);
	DECLARE_WRITE8_MEMBER(apache3_adc_w);
	DECLARE_WRITE16_MEMBER(apache3_rotate_w);
	DECLARE_READ16_MEMBER(roundup_v30_z80_r);
	DECLARE_WRITE16_MEMBER(roundup_v30_z80_w);
	DECLARE_WRITE16_MEMBER(roundup5_control_w);
	DECLARE_WRITE16_MEMBER(roundup5_d0000_w);
	DECLARE_WRITE16_MEMBER(roundup5_e0000_w);
	DECLARE_READ16_MEMBER(cyclwarr_control_r);
	DECLARE_WRITE16_MEMBER(cyclwarr_control_w);
	DECLARE_READ16_MEMBER(tatsumi_v30_68000_r);
	DECLARE_WRITE16_MEMBER(tatsumi_v30_68000_w);
	DECLARE_WRITE16_MEMBER(tatsumi_sprite_control_w);
	DECLARE_WRITE16_MEMBER(apache3_road_z_w);
	DECLARE_WRITE8_MEMBER(apache3_road_x_w);
	DECLARE_READ16_MEMBER(roundup5_vram_r);
	DECLARE_WRITE16_MEMBER(roundup5_vram_w);
	DECLARE_WRITE16_MEMBER(roundup5_palette_w);
	DECLARE_WRITE16_MEMBER(apache3_palette_w);
	DECLARE_WRITE16_MEMBER(roundup5_text_w);
	DECLARE_READ16_MEMBER(cyclwarr_videoram0_r);
	DECLARE_READ16_MEMBER(cyclwarr_videoram1_r);
	DECLARE_WRITE16_MEMBER(cyclwarr_videoram0_w);
	DECLARE_WRITE16_MEMBER(cyclwarr_videoram1_w);
	DECLARE_WRITE16_MEMBER(roundup5_crt_w);
};


/*----------- defined in machine/tatsumi.c -----------*/


READ8_DEVICE_HANDLER(tatsumi_hack_ym2151_r);
READ8_DEVICE_HANDLER(tatsumi_hack_oki_r);


void tatsumi_reset(running_machine &machine);

/*----------- defined in video/tatsumi.c -----------*/



VIDEO_START( apache3 );
VIDEO_START( roundup5 );
VIDEO_START( cyclwarr );
VIDEO_START( bigfight );

SCREEN_UPDATE_RGB32( roundup5 );
SCREEN_UPDATE_RGB32( apache3 );
SCREEN_UPDATE_RGB32( cyclwarr );
SCREEN_UPDATE_RGB32( bigfight );


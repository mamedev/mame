class raiden2_state : public driver_device
{
public:
	raiden2_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	DECLARE_WRITE16_MEMBER( cop_itoa_low_w );
	DECLARE_WRITE16_MEMBER( cop_itoa_high_w );
	DECLARE_WRITE16_MEMBER( cop_itoa_digit_count_w );
	DECLARE_WRITE16_MEMBER( cop_dma_v1_w );
	DECLARE_WRITE16_MEMBER( cop_dma_v2_w );
	DECLARE_WRITE16_MEMBER( cop_scale_w );
	DECLARE_WRITE16_MEMBER( cop_dma_adr_rel_w );
	DECLARE_WRITE16_MEMBER( cop_dma_adr_w );
	DECLARE_WRITE16_MEMBER( cop_dma_size_w );
	DECLARE_WRITE16_MEMBER( cop_dma_v3_w );
	DECLARE_WRITE16_MEMBER( cop_dma_mode_w );
	DECLARE_READ16_MEMBER ( cop_reg_high_r );
	DECLARE_WRITE16_MEMBER( cop_reg_high_w );
	DECLARE_READ16_MEMBER ( cop_reg_low_r );
	DECLARE_WRITE16_MEMBER( cop_reg_low_w );
	DECLARE_WRITE16_MEMBER( cop_pgm_data_w );
	DECLARE_WRITE16_MEMBER( cop_pgm_addr_w );
	DECLARE_WRITE16_MEMBER( cop_pgm_value_w );
	DECLARE_WRITE16_MEMBER( cop_pgm_mask_w );
	DECLARE_WRITE16_MEMBER( cop_pgm_trigger_w );
	DECLARE_WRITE16_MEMBER( cop_cmd_w );
	DECLARE_READ16_MEMBER ( cop_itoa_digits_r );
	DECLARE_READ16_MEMBER ( cop_status_r );
	DECLARE_READ16_MEMBER ( cop_dist_r );
	DECLARE_READ16_MEMBER ( cop_angle_r );
	DECLARE_WRITE16_MEMBER ( cop_dma_trigger_w );
	DECLARE_WRITE16_MEMBER ( cop_tile_bank_2_w );
	DECLARE_WRITE16_MEMBER ( tilemap_enable_w );
	DECLARE_WRITE16_MEMBER ( tile_scroll_w );
	DECLARE_WRITE16_MEMBER ( tile_bank_01_w );
	DECLARE_WRITE16_MEMBER ( raiden2_background_w );
	DECLARE_WRITE16_MEMBER ( raiden2_foreground_w );
	DECLARE_WRITE16_MEMBER ( raiden2_midground_w );
	DECLARE_WRITE16_MEMBER ( raiden2_text_w );

	DECLARE_WRITE16_MEMBER( sprcpt_val_1_w );
	DECLARE_WRITE16_MEMBER( sprcpt_val_2_w );
	DECLARE_WRITE16_MEMBER( sprcpt_data_1_w );
	DECLARE_WRITE16_MEMBER( sprcpt_data_2_w );
	DECLARE_WRITE16_MEMBER( sprcpt_data_3_w );
	DECLARE_WRITE16_MEMBER( sprcpt_data_4_w );
	DECLARE_WRITE16_MEMBER( sprcpt_adr_w );
	DECLARE_WRITE16_MEMBER( sprcpt_flags_1_w );
	DECLARE_WRITE16_MEMBER( sprcpt_flags_2_w );

	DECLARE_WRITE16_MEMBER( mcu_prog_w );
	DECLARE_WRITE16_MEMBER( mcu_prog_w2 );
	DECLARE_WRITE16_MEMBER( mcu_prog_offs_w );
	DECLARE_READ16_MEMBER( rdx_v33_system_r );
	DECLARE_READ16_MEMBER( r2_playerin_r );
	DECLARE_READ16_MEMBER( rdx_v33_unknown_r );
	DECLARE_READ16_MEMBER( rdx_v33_unknown2_r );
	DECLARE_READ16_MEMBER( nzerotea_unknown_r );

	DECLARE_READ16_MEMBER( raiden2_sound_comms_r );
	DECLARE_WRITE16_MEMBER( raiden2_sound_comms_w );

	void common_reset();

	tilemap_t *background_layer,*midground_layer,*foreground_layer,*text_layer;
	UINT16 *back_data,*fore_data,*mid_data, *text_data;
	int bg_bank, fg_bank, mid_bank;
	UINT16 raiden2_tilemap_enable;

	UINT16 scrollvals[6];
	UINT32 cop_regs[4], cop_itoa;
	UINT16 cop_status, cop_scale, cop_itoa_digit_count, cop_angle, cop_dist;
	UINT8 cop_itoa_digits[10];
	UINT16 cop_dma_mode, cop_dma_adr, cop_dma_size, cop_dma_v1, cop_dma_v2, cop_dma_v3,cop_dma_adr_rel;
	UINT16 sprites[0x800], sprites_cur_start;

	UINT16 cop_func_trigger[0x100/8];		/* function trigger */
	UINT16 cop_func_value[0x100/8];			/* function value (?) */
	UINT16 cop_func_mask[0x100/8];			/* function mask (?) */
	UINT16 cop_program[0x100];				/* program "code" */
	UINT16 cop_latch_addr, cop_latch_trigger, cop_latch_value, cop_latch_mask;

	void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect ,int pri_mask );
};


/*----------- defined in machine/r2crypt.c -----------*/

void raiden2_decrypt_sprites(running_machine *machine);

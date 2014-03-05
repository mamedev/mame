#include "audio/seibu.h"

class raiden2_state : public driver_device
{
public:
	raiden2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			back_data(*this, "back_data"),
			fore_data(*this, "fore_data"),
			mid_data(*this, "mid_data"),
			text_data(*this, "text_data"),
			sprites(*this, "sprites") ,
		m_maincpu(*this, "maincpu"),
		m_seibu_sound(*this, "seibu_sound"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	required_shared_ptr<UINT16> back_data,fore_data,mid_data, text_data, sprites;
	required_device<cpu_device> m_maincpu;
	required_device<seibu_sound_device> m_seibu_sound;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_WRITE16_MEMBER( cop_itoa_low_w );
	DECLARE_WRITE16_MEMBER( cop_itoa_high_w );
	DECLARE_WRITE16_MEMBER( cop_itoa_digit_count_w );
	DECLARE_WRITE16_MEMBER( cop_dma_v1_w );
	DECLARE_WRITE16_MEMBER( cop_dma_v2_w );
	DECLARE_WRITE16_MEMBER( cop_scale_w );
	DECLARE_WRITE16_MEMBER( cop_dma_adr_rel_w );
	DECLARE_WRITE16_MEMBER( cop_dma_src_w );
	DECLARE_WRITE16_MEMBER( cop_dma_size_w );
	DECLARE_WRITE16_MEMBER( cop_dma_dst_w );
	DECLARE_READ16_MEMBER( cop_dma_mode_r );
	DECLARE_WRITE16_MEMBER( cop_dma_mode_w );
	DECLARE_WRITE16_MEMBER( cop_pal_brightness_val_w );
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
	DECLARE_READ16_MEMBER ( cop_collision_status_r );
	DECLARE_READ16_MEMBER (cop_collision_status_y_r);
	DECLARE_READ16_MEMBER (cop_collision_status_x_r);
	DECLARE_READ16_MEMBER (cop_collision_status_z_r);
	DECLARE_READ16_MEMBER (cop_collision_status_unk_r);

	DECLARE_READ16_MEMBER ( cop_status_r );
	DECLARE_READ16_MEMBER ( cop_dist_r );
	DECLARE_READ16_MEMBER ( cop_angle_r );
	DECLARE_WRITE16_MEMBER( cop_angle_compare_w );
	DECLARE_WRITE16_MEMBER( cop_angle_mod_val_w );

	DECLARE_WRITE16_MEMBER ( cop_dma_trigger_w );
	DECLARE_WRITE16_MEMBER ( raiden2_bank_w );
	DECLARE_READ16_MEMBER ( cop_tile_bank_2_r );
	DECLARE_WRITE16_MEMBER ( cop_tile_bank_2_w );
	DECLARE_WRITE16_MEMBER ( raidendx_cop_bank_2_w );
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

	DECLARE_READ16_MEMBER( raiden2_sound_comms_r );
	DECLARE_WRITE16_MEMBER( raiden2_sound_comms_w );

	void common_reset();

	tilemap_t *background_layer,*midground_layer,*foreground_layer,*text_layer;
	int bg_bank, fg_bank, mid_bank;
	UINT16 raiden2_tilemap_enable;
	UINT8 prg_bank,prot_data;
	UINT16 cop_bank;

	UINT16 scrollvals[6];
	UINT32 cop_regs[8], cop_itoa;
	UINT16 cop_status, cop_scale, cop_itoa_digit_count, cop_angle, cop_dist;
	UINT8 cop_itoa_digits[10];
	UINT16 cop_dma_mode, cop_dma_src[0x200], cop_dma_dst[0x200], cop_dma_size[0x200], cop_dma_v1, cop_dma_v2, cop_dma_adr_rel;
	UINT16 sprites_cur_start;
	UINT16 pal_brightness_val;

	UINT16 cop_func_trigger[0x100/8];       /* function trigger */
	UINT16 cop_func_value[0x100/8];         /* function value (?) */
	UINT16 cop_func_mask[0x100/8];          /* function mask (?) */
	UINT16 cop_program[0x100];              /* program "code" */
	UINT16 cop_latch_addr, cop_latch_trigger, cop_latch_value, cop_latch_mask;
	INT8 cop_angle_compare;
	UINT8 cop_angle_mod_val;

	DECLARE_WRITE16_MEMBER( sprite_prot_x_w );
	DECLARE_WRITE16_MEMBER( sprite_prot_y_w );
	DECLARE_WRITE16_MEMBER( sprite_prot_src_seg_w );
	DECLARE_WRITE16_MEMBER( sprite_prot_src_w );
	DECLARE_READ16_MEMBER ( sprite_prot_dst1_r );
	DECLARE_READ16_MEMBER( sprite_prot_dst2_r );
	DECLARE_WRITE16_MEMBER( sprite_prot_dst1_w );
	DECLARE_WRITE16_MEMBER( sprite_prot_dst2_w );

	UINT16 sprite_prot_x,sprite_prot_y,dst1,dst2;
	UINT16 sprite_prot_src_addr[2];

	struct
	{
		int x,y;
		int min_x,min_y,max_x,max_y;
		UINT16 hitbox;
		UINT16 hitbox_x,hitbox_y;
	}cop_collision_info[2];

	UINT16 cop_hit_status;
	INT16 cop_hit_val_x,cop_hit_val_y,cop_hit_val_z,cop_hit_val_unk;

	void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect ,int pri_mask );
	UINT8 cop_calculate_collsion_detection(running_machine &machine);
	void cop_take_hit_box_params(UINT8 offs);

	DECLARE_WRITE16_MEMBER(cop_sort_lookup_hi_w);
	DECLARE_WRITE16_MEMBER(cop_sort_lookup_lo_w);
	DECLARE_WRITE16_MEMBER(cop_sort_ram_addr_hi_w);
	DECLARE_WRITE16_MEMBER(cop_sort_ram_addr_lo_w);
	DECLARE_WRITE16_MEMBER(cop_sort_param_w);
	DECLARE_WRITE16_MEMBER(cop_sort_dma_trig_w);

	UINT32 cop_sort_ram_addr, cop_sort_lookup;
	UINT16 cop_sort_param;

	DECLARE_DRIVER_INIT(raidendx);
	DECLARE_DRIVER_INIT(xsedae);
	DECLARE_DRIVER_INIT(zeroteam);
	DECLARE_DRIVER_INIT(raiden2);
	TILE_GET_INFO_MEMBER(get_back_tile_info);
	TILE_GET_INFO_MEMBER(get_mid_tile_info);
	TILE_GET_INFO_MEMBER(get_fore_tile_info);
	TILE_GET_INFO_MEMBER(get_text_tile_info);
	DECLARE_MACHINE_RESET(raiden2);
	DECLARE_VIDEO_START(raiden2);
	DECLARE_MACHINE_RESET(zeroteam);
	DECLARE_MACHINE_RESET(xsedae);
	DECLARE_MACHINE_RESET(raidendx);
	UINT32 screen_update_raiden2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(raiden2_interrupt);
	UINT16 rps();
	UINT16 rpc();
	const UINT8 fade_table(int v);
	void combine32(UINT32 *val, int offset, UINT16 data, UINT16 mem_mask);
	void sprcpt_init(void);
};

/*----------- defined in machine/r2crypt.c -----------*/
void raiden2_decrypt_sprites(running_machine &machine);
void zeroteam_decrypt_sprites(running_machine &machine);

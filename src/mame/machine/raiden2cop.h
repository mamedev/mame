/***************************************************************************

 Seibu Cop (Co-Processor) emulation
  (new implementation, based on Raiden 2 code)

***************************************************************************/

#ifndef RAIDEN2COP_H
#define RAIDEN2COP_H


#define MCFG_RAIDEN2COP_ADD(_tag ) \
	MCFG_DEVICE_ADD(_tag, RAIDEN2COP, 0)

#define MCFG_RAIDEN2COP_VIDEORAM_OUT_CB(_devcb) \
	devcb = &raiden2cop_device::set_m_videoramout_cb(*device, DEVCB_##_devcb);

#define MCFG_ITOA_UNUSED_DIGIT_VALUE(value) \
	raiden2cop_device::set_itoa_unused_digit_value(*device, value);


class raiden2cop_device : public device_t
{
public:
	raiden2cop_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// Command Table
	UINT16 cop_func_trigger[0x100/8];       /* function trigger */
	UINT16 cop_func_value[0x100/8];         /* function value (?) */
	UINT16 cop_func_mask[0x100/8];          /* function mask (?) */
	UINT16 cop_program[0x100];              /* program "code" */
	UINT16 cop_latch_addr, cop_latch_trigger, cop_latch_value, cop_latch_mask;

	DECLARE_WRITE16_MEMBER( cop_pgm_data_w );
	DECLARE_WRITE16_MEMBER( cop_pgm_addr_w );
	DECLARE_WRITE16_MEMBER( cop_pgm_value_w );
	DECLARE_WRITE16_MEMBER( cop_pgm_mask_w );
	DECLARE_WRITE16_MEMBER( cop_pgm_trigger_w );

	// these are used by legionna.c to find the command based on trigger value
	// legionna/seicop implementation then looks up the command based on hardcoded sequences in the driver rather than actually using the trigger value (should be changed)
	UINT16 get_func_value(int command) { return cop_func_value[command]; }
	UINT16 get_func_mask(int command) { return cop_func_mask[command]; }
	int find_trigger_match(UINT16 triggerval, UINT16 mask);
	int check_command_matches(int command, UINT16 seq0, UINT16 seq1, UINT16 seq2, UINT16 seq3, UINT16 seq4, UINT16 seq5, UINT16 seq6, UINT16 seq7, UINT16 _funcval_, UINT16 _funcmask_);

	// DMA
	UINT16 cop_dma_v1, cop_dma_v2, cop_dma_mode;
	UINT16 cop_dma_src[0x200], cop_dma_dst[0x200], cop_dma_size[0x200];
	UINT16 cop_dma_adr_rel;

	UINT16 pal_brightness_val;
	UINT16 pal_brightness_mode;

	DECLARE_WRITE16_MEMBER( cop_dma_v1_w );
	DECLARE_WRITE16_MEMBER( cop_dma_v2_w );

	DECLARE_WRITE16_MEMBER( cop_dma_adr_rel_w );
	DECLARE_WRITE16_MEMBER( cop_dma_src_w );
	DECLARE_WRITE16_MEMBER( cop_dma_size_w );
	DECLARE_WRITE16_MEMBER( cop_dma_dst_w );
	DECLARE_READ16_MEMBER( cop_dma_mode_r );
	DECLARE_WRITE16_MEMBER( cop_dma_mode_w );

	DECLARE_WRITE16_MEMBER( cop_pal_brightness_val_w );
	DECLARE_WRITE16_MEMBER( cop_pal_brightness_mode_w );

	DECLARE_WRITE16_MEMBER ( cop_dma_trigger_w );

	const UINT8 fade_table(int v);

	template<class _Object> static devcb_base &set_m_videoramout_cb(device_t &device, _Object object) { return downcast<raiden2cop_device &>(device).m_videoramout_cb.set_callback(object); }

	// Number Conversion

	DECLARE_WRITE16_MEMBER( cop_itoa_low_w );
	DECLARE_WRITE16_MEMBER( cop_itoa_high_w );
	DECLARE_WRITE16_MEMBER( cop_itoa_digit_count_w );
	DECLARE_READ16_MEMBER ( cop_itoa_digits_r );
	
	UINT32 cop_itoa;
	UINT16 cop_itoa_digit_count;
	UINT8 cop_itoa_digits[10];
	UINT8 m_cop_itoa_unused_digit_value;

	static void set_itoa_unused_digit_value(device_t &device, int value) { downcast<raiden2cop_device &>(device).m_cop_itoa_unused_digit_value = value; }

	// Main COP functionality

	DECLARE_WRITE16_MEMBER( cop_scale_w );
	DECLARE_WRITE16_MEMBER( cop_angle_target_w );
	DECLARE_WRITE16_MEMBER( cop_angle_step_w );

	DECLARE_READ16_MEMBER ( cop_reg_high_r );
	DECLARE_WRITE16_MEMBER( cop_reg_high_w );
	DECLARE_READ16_MEMBER ( cop_reg_low_r );
	DECLARE_WRITE16_MEMBER( cop_reg_low_w );

	DECLARE_WRITE16_MEMBER( cop_cmd_w );
	DECLARE_READ16_MEMBER ( cop_collision_status_r );
	DECLARE_READ16_MEMBER (cop_collision_status_val_r);
	DECLARE_READ16_MEMBER (cop_collision_status_stat_r);

	DECLARE_READ16_MEMBER ( cop_status_r );
	DECLARE_READ16_MEMBER ( cop_dist_r );
	DECLARE_READ16_MEMBER ( cop_angle_r );
	DECLARE_WRITE16_MEMBER( cop_angle_compare_w );
	DECLARE_WRITE16_MEMBER( cop_angle_mod_val_w );

	DECLARE_WRITE16_MEMBER(cop_hitbox_baseadr_w);


	UINT32 cop_regs[8];
	UINT16 cop_status, cop_scale, cop_angle, cop_dist;
	

	UINT16 cop_angle_target;
	UINT16 cop_angle_step;


	struct colinfo {
		INT16 pos[3];
		INT8 dx[3];
		UINT8 size[3];
		bool allow_swap;
		UINT16 flags_swap;
		UINT32 spradr;
	};

	colinfo cop_collision_info[2];

	UINT16 cop_hit_status, cop_hit_baseadr;
	INT16 cop_hit_val[3];
	UINT16 cop_hit_val_stat;

	void cop_collision_read_pos(address_space &space, int slot, UINT32 spradr, bool allow_swap);
	void cop_collision_update_hitbox(address_space &space, int slot, UINT32 hitadr);

	// Sort DMA (zeroteam, cupsoc)

	UINT32 cop_sort_ram_addr, cop_sort_lookup;
	UINT16 cop_sort_param;

	DECLARE_WRITE16_MEMBER(cop_sort_lookup_hi_w);
	DECLARE_WRITE16_MEMBER(cop_sort_lookup_lo_w);
	DECLARE_WRITE16_MEMBER(cop_sort_ram_addr_hi_w);
	DECLARE_WRITE16_MEMBER(cop_sort_ram_addr_lo_w);
	DECLARE_WRITE16_MEMBER(cop_sort_param_w);
	DECLARE_WRITE16_MEMBER(cop_sort_dma_trig_w);

	// RNG (cupsoc)
	UINT16 m_cop_rng_max_value;
	DECLARE_READ16_MEMBER(cop_prng_r);
	DECLARE_WRITE16_MEMBER(cop_prng_maxvalue_w);
	DECLARE_READ16_MEMBER(cop_prng_maxvalue_r);

	// misc 68k (grainbow)
	UINT32 m_cop_sprite_dma_param;
	UINT32 m_cop_sprite_dma_src;

	DECLARE_WRITE16_MEMBER(cop_sprite_dma_param_hi_w);
	DECLARE_WRITE16_MEMBER(cop_sprite_dma_param_lo_w);
	DECLARE_WRITE16_MEMBER(cop_sprite_dma_size_w);
	DECLARE_WRITE16_MEMBER(cop_sprite_dma_src_hi_w);
	DECLARE_WRITE16_MEMBER(cop_sprite_dma_src_lo_w);
	DECLARE_WRITE16_MEMBER(cop_sprite_dma_inc_w);
	int m_cop_sprite_dma_size;

	// misc 68k
	UINT16 m_cop_rom_addr_lo,m_cop_rom_addr_hi,m_cop_rom_addr_unk;
	DECLARE_WRITE16_MEMBER(cop_rom_addr_unk_w);
	DECLARE_WRITE16_MEMBER(cop_rom_addr_lo_w);
	DECLARE_WRITE16_MEMBER(cop_rom_addr_hi_w);

	int m_cop_sprite_dma_abs_x,m_cop_sprite_dma_abs_y;
	DECLARE_WRITE16_MEMBER(cop_sprite_dma_abs_y_w);
	DECLARE_WRITE16_MEMBER(cop_sprite_dma_abs_x_w);

	// legacy code, to be removed / refactored into above
	INT8 m_LEGACY_cop_angle_compare;
	INT8 m_LEGACY_cop_angle_mod_val;
	DECLARE_WRITE16_MEMBER(LEGACY_cop_angle_compare_w);
	DECLARE_WRITE16_MEMBER(LEGACY_cop_angle_mod_val_w);
	INT16 m_LEGACY_cop_hit_val_x,m_LEGACY_cop_hit_val_y,m_LEGACY_m_cop_hit_val_z;
	int m_LEGACY_r0, m_LEGACY_r1;
	DECLARE_READ16_MEMBER(LEGACY_cop_collision_status_val_r);
	DECLARE_WRITE16_MEMBER(LEGACY_cop_cmd_w);

	struct LEGACY_collision_info
	{
		LEGACY_collision_info():
		x(0),
		y(0),
		min_x(0),
		min_y(0),
		max_x(0),
		max_y(0),
		hitbox(0),
		hitbox_x(0),
		hitbox_y(0) {}

		int x,y;
		INT16 min_x,min_y,max_x,max_y;
		UINT16 hitbox;
		UINT16 hitbox_x,hitbox_y;
	};

	struct LEGACY_collision_info m_LEGACY_cop_collision_info[2];
	void LEGACY_cop_take_hit_box_params(UINT8 offs);
	UINT8 LEGACY_cop_calculate_collsion_detection();


protected:
	// device-level overrides
	virtual void device_start();

private:
	// internal state
	devcb_write16       m_videoramout_cb;
	required_device<palette_device> m_palette;
};

extern const device_type RAIDEN2COP;

#endif  /* RAIDEN2COP_H */

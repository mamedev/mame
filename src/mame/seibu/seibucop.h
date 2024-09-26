// license:LGPL-2.1+
// copyright-holders:Olivier Galibert, Angelo Salese, David Haywood, Tomasz Slanina
/***************************************************************************

 Seibu Cop (Co-Processor) emulation
  (new implementation, based on Raiden 2 code)

***************************************************************************/

#ifndef MAME_SEIBU_SEIBUCOP_H
#define MAME_SEIBU_SEIBUCOP_H

#pragma once


class raiden2cop_device : public device_t
{
public:
	raiden2cop_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Command Table
	uint16_t cop_func_trigger[0x100/8];       /* function trigger */
	uint16_t cop_func_value[0x100/8];         /* function value (?) */
	uint16_t cop_func_mask[0x100/8];          /* function mask (?) */
	uint16_t cop_program[0x100];              /* program "code" */
	uint16_t cop_latch_addr, cop_latch_trigger, cop_latch_value, cop_latch_mask;

	void cop_pgm_data_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cop_pgm_addr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cop_pgm_value_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cop_pgm_mask_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cop_pgm_trigger_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// these are used by legionna.c to find the command based on trigger value
	// legionna/seicop implementation then looks up the command based on hardcoded sequences in the driver rather than actually using the trigger value (should be changed)
	uint16_t get_func_value(int command) { return cop_func_value[command]; }
	uint16_t get_func_mask(int command) { return cop_func_mask[command]; }
	int find_trigger_match(uint16_t triggerval, uint16_t mask);
	int check_command_matches(int command, uint16_t seq0, uint16_t seq1, uint16_t seq2, uint16_t seq3, uint16_t seq4, uint16_t seq5, uint16_t seq6, uint16_t seq7, uint16_t _funcval_, uint16_t _funcmask_);

	// DMA
	uint16_t cop_dma_v1, cop_dma_v2, cop_dma_mode;
	uint16_t cop_dma_src[0x200], cop_dma_dst[0x200], cop_dma_size[0x200];
	uint16_t cop_dma_adr_rel;

	uint16_t pal_brightness_val;
	uint16_t pal_brightness_mode;

	void cop_dma_v1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cop_dma_v2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void cop_dma_adr_rel_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cop_dma_src_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cop_dma_size_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cop_dma_dst_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t cop_dma_mode_r();
	void cop_dma_mode_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void cop_pal_brightness_val_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cop_pal_brightness_mode_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void cop_dma_trigger_w(uint16_t data);

	uint8_t fade_table(int v);

	auto videoramout_cb() { return m_videoramout_cb.bind(); }
	auto paletteramout_cb() { return m_paletteramout_cb.bind(); }
	template<class T> void set_host_cpu_tag(T &&tag) { m_host_cpu.set_tag(std::forward<T>(tag)); }

	// Number Conversion

	void cop_itoa_low_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cop_itoa_high_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cop_itoa_mode_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t cop_itoa_digits_r(offs_t offset);

	uint32_t cop_itoa;
	uint16_t cop_itoa_mode;
	uint8_t cop_itoa_digits[10];

	// Main COP functionality

	void cop_scale_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cop_angle_target_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cop_angle_step_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t cop_reg_high_r(offs_t offset);
	void cop_reg_high_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t cop_reg_low_r(offs_t offset);
	void cop_reg_low_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void cop_cmd_w(offs_t offset, uint16_t data);
	uint16_t cop_collision_status_r();
	uint16_t cop_collision_status_val_r(offs_t offset);
	uint16_t cop_collision_status_stat_r();

	uint16_t cop_status_r();
	uint16_t cop_dist_r();
	uint16_t cop_angle_r();

	void cop_hitbox_baseadr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);


	uint32_t cop_regs[8];
	uint16_t cop_status, cop_scale, cop_angle, cop_dist;


	uint16_t cop_angle_target;
	uint16_t cop_angle_step;


	struct colinfo {
		int16_t pos[3] = { 0, 0, 0 };
		int8_t dx[3] = { 0, 0, 0 };
		uint8_t size[3] = { 0, 0, 0 };
		bool allow_swap = false;
		uint16_t flags_swap = 0;
		uint32_t spradr = 0;
		int16_t min[3] = { 0, 0, 0 }, max[3] = { 0, 0, 0 };
	};

	colinfo cop_collision_info[2];

	uint16_t cop_hit_status, cop_hit_baseadr;
	int16_t cop_hit_val[3];
	uint16_t cop_hit_val_stat;

	// Sort DMA (zeroteam, cupsoc)

	uint32_t cop_sort_ram_addr, cop_sort_lookup;
	uint16_t cop_sort_param;

	void cop_sort_lookup_hi_w(uint16_t data);
	void cop_sort_lookup_lo_w(uint16_t data);
	void cop_sort_ram_addr_hi_w(uint16_t data);
	void cop_sort_ram_addr_lo_w(uint16_t data);
	void cop_sort_param_w(uint16_t data);
	void cop_sort_dma_trig_w(uint16_t data);

	// RNG (cupsoc)
	uint16_t m_cop_rng_max_value;
	uint16_t cop_prng_r();
	void cop_prng_maxvalue_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t cop_prng_maxvalue_r();

	// misc 68k (grainbow)
	uint32_t m_cop_sprite_dma_param;
	uint32_t m_cop_sprite_dma_src;

	void cop_sprite_dma_param_hi_w(uint16_t data);
	void cop_sprite_dma_param_lo_w(uint16_t data);
	void cop_sprite_dma_size_w(uint16_t data);
	void cop_sprite_dma_src_hi_w(uint16_t data);
	void cop_sprite_dma_src_lo_w(uint16_t data);
	void cop_sprite_dma_inc_w(uint16_t data);
	int m_cop_sprite_dma_size;

	// misc 68k
	uint16_t m_cop_unk_param_a,m_cop_unk_param_b;
	void cop_unk_param_a_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cop_unk_param_b_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t m_cop_rom_addr_lo,m_cop_rom_addr_hi,m_cop_precmd;
	void cop_precmd_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cop_rom_addr_lo_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cop_rom_addr_hi_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	int m_cop_sprite_dma_abs_x,m_cop_sprite_dma_abs_y;
	int m_sprite_dma_rel_x, m_sprite_dma_rel_y;
	int m_sprite_dma_x_clip;
	void cop_sprite_dma_abs_y_w(uint16_t data);
	void cop_sprite_dma_abs_x_w(uint16_t data);

	// legacy code, to be removed / refactored into above
	int m_LEGACY_r0, m_LEGACY_r1;
	void LEGACY_cop_cmd_w(offs_t offset, uint16_t data);

	// DEBUG
	void dump_table();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	// device callbacks
	devcb_write16       m_videoramout_cb;
	devcb_write16       m_paletteramout_cb;

	required_device<cpu_device> m_host_cpu; /**< reference to the host cpu */
	address_space *m_host_space; /**< reference to the host cpu space */
	bool m_host_endian;          /**< reference to the host cpu endianness, some commands cares! */
	uint8_t m_byte_endian_val;     /**< 2 if m_host_endian is big (68k) else 0 */
	uint8_t m_word_endian_val;     /**< 3 if m_host_endian is big (68k) else 0 */


	void cop_collision_read_pos(int slot, uint32_t spradr, bool allow_swap);

	// commands, TODO: needs commenting!
	void execute_8100(int offset, uint16_t data);
	void execute_8900(int offset, uint16_t data);

	void execute_42c2(int offset, uint16_t data);
	void execute_4aa0(int offset, uint16_t data);
	void execute_6200(int offset, uint16_t data);

	void execute_338e(int offset, uint16_t data, bool is_yflip);
	void execute_2288(int offset, uint16_t data);
	void execute_0205(int offset, uint16_t data);
	void execute_3b30(int offset, uint16_t data);
	void execute_130e(int offset, uint16_t data, bool is_yflip);
	void execute_0904(int offset, uint16_t data);
	void execute_2a05(int offset, uint16_t data);
	void execute_7e05(int offset, uint16_t data);
	void execute_5205(int offset, uint16_t data);
	void execute_5a05(int offset, uint16_t data);

	void execute_f205(int offset, uint16_t data);
	void execute_a100(int offset, uint16_t data);
	void execute_a900(int offset, uint16_t data);
	void execute_b100(int offset, uint16_t data);
	void execute_b900(int offset, uint16_t data);
	void execute_f105(int offset, uint16_t data);

	// TODO: remove/rename these
	//void LEGACY_execute_130e(int offset, uint16_t data);
	void LEGACY_execute_130e_cupsoc(int offset, uint16_t data);
	//void LEGACY_execute_3b30(int offset, uint16_t data);
	//void LEGACY_execute_42c2(int offset, uint16_t data);
	void LEGACY_execute_e30e(int offset, uint16_t data);
	void LEGACY_execute_6200(int offset, uint16_t data);
	void LEGACY_execute_dde5(int offset, uint16_t data);
	void LEGACY_execute_d104(int offset, uint16_t data);
	void LEGACY_execute_6980(int offset, uint16_t data);
	void LEGACY_execute_c480(int offset, uint16_t data);

	void cop_collision_update_hitbox(uint16_t data, int slot, uint32_t hitadr);
	void bcd_update();

	uint16_t cop_read_word(int address);
	uint8_t cop_read_byte(int address);
	void cop_write_word(int address, uint16_t data);
	void cop_write_byte(int address, uint8_t data);

	void dma_tilemap_buffer();
	void dma_palette_buffer();
	void dma_fill();
	void dma_palette_brightness();
	void dma_zsorting(uint16_t data);
};

DECLARE_DEVICE_TYPE(RAIDEN2COP, raiden2cop_device)

#endif // MAME_SEIBU_SEIBUCOP_H

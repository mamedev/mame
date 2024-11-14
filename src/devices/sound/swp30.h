// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha SWP30/30B, rompler/dsp combo

#ifndef DEVICES_SOUND_SWP30_H
#define DEVICES_SOUND_SWP30_H

#pragma once

#include "swp30d.h"

class swp30_device : public cpu_device, public device_sound_interface, public swp30_disassembler::info
{
public:
	enum { AS_REVERB = AS_IO };

	swp30_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 33868800);

	void map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;
	virtual uint32_t execute_min_cycles() const noexcept override;
	virtual uint32_t execute_max_cycles() const noexcept override;
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return (clocks + 1) / 2; }
	virtual void execute_run() override;
	virtual space_config_vector memory_space_config() const override;
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	enum {
		IDLE,
		ATTACK,
		DECAY1,
		DECAY2,
		RELEASE
	};

	struct mixer_slot {
		std::array<u16, 3> vol;
		std::array<u16, 3> route;
	};

	address_space_config m_program_config, m_rom_config, m_reverb_config;
	address_space *m_program, *m_rom, *m_reverb;
	memory_access<25, 2, -2, ENDIANNESS_LITTLE>::cache m_rom_cache;
	memory_access<18, 1, -1, ENDIANNESS_LITTLE>::cache m_reverb_cache;

	sound_stream *m_stream;

	static const std::array<s32, 0x80> attack_linear_step;
	static const std::array<s32, 0x20> decay_linear_step;
	static const std::array<s32, 16> panmap;
	static const std::array<u32, 0x400> pitch_base;
	std::array<s32,  0x80> m_global_step;
	std::array<s16, 0x100> m_dpcm;

	static const std::array<u32, 4> lfo_shape_centered_saw;
	static const std::array<u32, 4> lfo_shape_centered_tri;
	static const std::array<u32, 4> lfo_shape_offset_saw;
	static const std::array<u32, 4> lfo_shape_offset_tri;
	static const std::array<u8,  4> dpcm_offset;

	std::array<s32,  0x40> m_sample_start;
	std::array<s32,  0x40> m_sample_end;
	std::array<u32,  0x40> m_sample_address;
	std::array<u16,  0x40> m_pitch;

	std::array<u16,  0x40> m_attack;
	std::array<u16,  0x40> m_decay1;
	std::array<u16,  0x40> m_decay2;
	std::array<u16,  0x40> m_release_glo;
	std::array<u16,  0x40> m_lfo_step_pmod;
	std::array<u16,  0x40> m_lfo_amod;

	std::array<u32,  0x40> m_lfo_phase;
	std::array<s32,  0x40> m_sample_pos;
	std::array<s32,  0x40> m_envelope_level;
	std::array<s32,  0x40> m_envelope_timer;
	std::array<bool, 0x40> m_envelope_on_timer;
	std::array<bool, 0x40> m_decay2_done;
	std::array<u8,   0x40> m_envelope_mode;
	std::array<s32,  0x40> m_glo_level_cur;
	std::array<s32,  0x40> m_pan_l;
	std::array<s32,  0x40> m_pan_r;
	std::array<s16,  0x40> m_dpcm_current;
	std::array<s16,  0x40> m_dpcm_next;
	std::array<u32,  0x40> m_dpcm_address;
	std::array<s32,  0x40> m_dpcm_sum;

	std::array<u64, 0x180> m_meg_program;
	std::array<s16, 0x180> m_meg_const;
	std::array<u16,  0x80> m_meg_offset;
	std::array<u16,  0x18> m_meg_lfo;
	std::array<u16,     8> m_meg_map;

	std::array<mixer_slot, 0x80> m_mixer;

	std::array<s32,  0x40> m_meg_m;
	std::array<s32,  0x10> m_melo;
	std::array<s32,     2> m_meg_output;

	s32 m_sample_history[0x40][2][2];

	u32 m_waverom_adr, m_waverom_mode, m_waverom_val;
	u16 m_waverom_access;

	u16 m_lpf_cutoff[0x40], m_lpf_cutoff_inc[0x40], m_lpf_reso[0x40], m_hpf_cutoff[0x40];
	s16 m_eq_filter[0x40][6];

	u64 m_keyon_mask;
	u16 m_internal_adr;

	u16 m_meg_program_address;
	u16 m_meg_pc;
	int m_icount;

	// AWM2 per-channel registers
	u16 lpf_cutoff_r(offs_t offset);
	void lpf_cutoff_w(offs_t offset, u16 data);
	u16 lpf_cutoff_inc_r(offs_t offset);
	void lpf_cutoff_inc_w(offs_t offset, u16 data);
	u16 hpf_cutoff_r(offs_t offset);
	void hpf_cutoff_w(offs_t offset, u16 data);
	u16 lpf_reso_r(offs_t offset);
	void lpf_reso_w(offs_t offset, u16 data);
	u16 attack_r(offs_t offset);
	void attack_w(offs_t offset, u16 data);
	u16 decay1_r(offs_t offset);
	void decay1_w(offs_t offset, u16 data);
	u16 decay2_r(offs_t offset);
	void decay2_w(offs_t offset, u16 data);
	u16 release_glo_r(offs_t offset);
	void release_glo_w(offs_t offset, u16 data);
	template<int coef> u16 eq_filter_r(offs_t offset);
	template<int coef> void eq_filter_w(offs_t offset, u16 data);

	u16 sample_start_h_r(offs_t offset);
	u16 sample_start_l_r(offs_t offset);
	void sample_start_h_w(offs_t offset, u16 data);
	void sample_start_l_w(offs_t offset, u16 data);
	u16 sample_end_h_r(offs_t offset);
	u16 sample_end_l_r(offs_t offset);
	void sample_end_h_w(offs_t offset, u16 data);
	void sample_end_l_w(offs_t offset, u16 data);
	u16 sample_address_h_r(offs_t offset);
	u16 sample_address_l_r(offs_t offset);
	void sample_address_h_w(offs_t offset, u16 data);
	void sample_address_l_w(offs_t offset, u16 data);
	u16 pitch_r(offs_t offset);
	void pitch_w(offs_t offset, u16 data);

	u16 pan_r(offs_t offset);
	void pan_w(offs_t offset, u16 data);
	u16 dry_rev_r(offs_t offset);
	void dry_rev_w(offs_t offset, u16 data);
	u16 cho_var_r(offs_t offset);
	void cho_var_w(offs_t offset, u16 data);

	void lfo_step_pmod_w(offs_t offset, u16 data);
	u16 lfo_step_pmod_r(offs_t offset);
	void lfo_amod_w(offs_t offset, u16 data);
	u16 lfo_amod_r(offs_t offset);

	u16 internal_adr_r();
	void internal_adr_w(u16 data);
	u16 internal_r();
	template<int sel> u16 route_r(offs_t offset);
	template<int sel> void route_w(offs_t offset, u16 data);
	template<int sel> u16 vol_r(offs_t offset);
	template<int sel> void vol_w(offs_t offset, u16 data);

	// Envelope control
	void change_mode_attack_decay1(int chan);
	void change_mode_decay1_decay2(int chan);
	static bool istep(s32 &value, s32 limit, s32 step);
	static bool fpstep(s32 &value, s32 limit, s32 step);
	static s32 fpadd(s32 value, s32 step);
	static s32 fpsub(s32 value, s32 step);
	static s32 fpapply(s32 value, s32 sample);
	static s32 lpffpapply(s32 value, s32 sample);
	static s32 meg_att(s32 sample, s32 att);

	// Control registers
	template<int sel> u16 keyon_mask_r();
	template<int sel> void keyon_mask_w(u16 data);
	u16 keyon_r();
	void keyon_w(u16);
	u16 meg_prg_address_r();
	void meg_prg_address_w(u16 data);
	template<int sel> u16 meg_prg_r();
	template<int sel> void meg_prg_w(u16 data);
	template<int sel> u16 meg_map_r();
	template<int sel> void meg_map_w(u16 data);
	template<int sel> void waverom_adr_w(u16 data);
	template<int sel> u16 waverom_adr_r();
	template<int sel> void waverom_mode_w(u16 data);
	template<int sel> u16 waverom_mode_r();
	void waverom_access_w(u16 data);
	u16 waverom_access_r();
	u16 waverom_busy_r();
	template<int sel> u16 waverom_val_r();

	// MEG registers
	template<int sel> u16 meg_const_r(offs_t offset);
	template<int sel> void meg_const_w(offs_t offset, u16 data);
	template<int sel> u16 meg_offset_r(offs_t offset);
	template<int sel> void meg_offset_w(offs_t offset, u16 data);
	template<int sel> u16 meg_lfo_r(offs_t offset);
	template<int sel> void meg_lfo_w(offs_t offset, u16 data);

	void meg_prg_map(address_map &map) ATTR_COLD;
	u64 meg_prg_map_r(offs_t address);

	void meg_reverb_map(address_map &map) ATTR_COLD;

	virtual u16 swp30d_const_r(u16 address) const override;
	virtual u16 swp30d_offset_r(u16 address) const override;

	// Generic catch-all
	u16 snd_r(offs_t offset);
	void snd_w(offs_t offset, u16 data);

	inline auto &rchan(address_map &map, int idx) {
		return map(idx*2, idx*2+1).select(0x1f80);
	}

	inline auto &rctrl(address_map &map, int idx) {
		int slot = 0x40*(idx >> 1) | 0xe | (idx & 1);
		return map(slot*2, slot*2+1);
	}
};

DECLARE_DEVICE_TYPE(SWP30, swp30_device)

#endif

// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha MEG - Multiple effects generator
//
// Audio dsp dedicated to effects generation

#ifndef DEVICES_SOUND_MEG_H
#define DEVICES_SOUND_MEG_H

#pragma once

#include "megd.h"


class meg_base_device : public cpu_device, public meg_disassembler::info
{
public:
	enum {
		AS_FP = 1,
		AS_OFFSETS = 2
	};

	meg_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, u32 prg_size);

	void prg_w(u16 address, u64 opcode);
	void fp_w(u16 address, u16 value);
	void offset_w(u16 address, u16 value);
	void lfo_w(u8 reg, u16 value);
	void map_w(u8 reg, u16 value);
	u64 prg_r(u16 address) const;
	virtual u16 fp_r(u16 address) const override;
	virtual u16 offset_r(u16 address) const override;
	u16 lfo_r(u8 reg) const;
	u16 map_r(u8 reg) const;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual uint32_t execute_min_cycles() const override;
	virtual uint32_t execute_max_cycles() const override;
	virtual uint32_t execute_input_lines() const override;
	virtual void execute_run() override;
	virtual space_config_vector memory_space_config() const override;
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	address_space_config m_program_config, m_fp_config, m_offsets_config;
	address_space *m_program, *m_fp, *m_offsets;

	u32 m_prg_size, m_pc;
	int m_icount;

	u16 m_lfo[0x18], m_map[8];

	void prg_map(address_map &map);
	void fp_map(address_map &map);
	void offsets_map(address_map &map);
};

class meg_embedded_device : public meg_base_device
{
public:
	meg_embedded_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 44100*384);
};

class meg_device : public meg_base_device
{
public:
	meg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 44100*256);
	void map(address_map &map);

private:
	u8 m_r4[256];
	u8 m_r5[256];
	u8 m_r8[256];
	u8 m_r9[256];
	u8 m_re[256];
	u8 m_rf[256];
	u8 m_r12[256];
	u8 m_r13[256];
	u8 m_r14[256];
	u8 m_r16[256];
	u8 m_r17[256];
	u8 m_r18[256];
	u8 m_reg;
	u8 s2_r();
	u8 s10_r();
	u8 s11_r();
	void select_w(u8 reg);
	void s1_w(u8 data);
	void s2_w(u8 data);
	void s3_w(u8 data);
	void s4_w(u8 data);
	void s5_w(u8 data);
	void s7_w(u8 data);
	void s8_w(u8 data);
	void s9_w(u8 data);
	void sa_w(u8 data);
	void fph_w(u8 data);
	void fpl_w(u8 data);
	void se_w(u8 data);
	void sf_w(u8 data);
	void s10_w(u8 data);
	void s11_w(u8 data);
	void offseth_w(u8 data);
	void offsetl_w(u8 data);
	void s14_w(u8 data);
	void s15_w(u8 data);
	void s16_w(u8 data);
	void s17_w(u8 data);
	void s18_w(u8 data);
};


DECLARE_DEVICE_TYPE(MEG, meg_device)
DECLARE_DEVICE_TYPE(MEGEMB, meg_embedded_device)

#endif

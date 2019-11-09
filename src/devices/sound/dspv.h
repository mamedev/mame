// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha DSPV, dsp used for acoustic simulation

#ifndef DEVICES_SOUND_DSPV_H
#define DEVICES_SOUND_DSPV_H

#pragma once

#include "dspvd.h"

class dspv_device : public cpu_device, public device_sound_interface
{
public:
	dspv_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 22579200);

	void map(address_map &map);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual uint32_t execute_min_cycles() const noexcept override;
	virtual uint32_t execute_max_cycles() const noexcept override;
	virtual uint32_t execute_input_lines() const noexcept override;
	virtual void execute_run() override;
	virtual space_config_vector memory_space_config() const override;
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	address_space_config m_program_config, m_data_config;
	address_space *m_program, *m_data;

	u32 m_pc;
	int m_icount;

	u32 m_table_adr;
	u16 m_prg_adr;
	u16 m_status;

	// Table ram access
	void table_adrh_w(u16 data);
	void table_adrl_w(u16 data);
	void table_data_w(u16 data);
	void table_zero_w(u16 data);

	// Program ram access
	void prg_adr_w(u16 data);
	void prg_data_w(offs_t offset, u16 data);

	// Registers
	u16 status_r();

	// Generic catch-all
	u16 snd_r(offs_t offset);
	void snd_w(offs_t offset, u16 data);

	void prg_map(address_map &map);
	void data_map(address_map &map);
};

DECLARE_DEVICE_TYPE(DSPV, dspv_device)

#endif

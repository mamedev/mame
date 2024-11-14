// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha DSPV, dsp used for acoustic simulation

#ifndef MAME_SOUND_DSPV_H
#define MAME_SOUND_DSPV_H

#pragma once

#include "dspvd.h"

class dspv_device : public cpu_device, public device_sound_interface
{
public:
	dspv_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 22579200);

	void map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual uint32_t execute_min_cycles() const noexcept override;
	virtual uint32_t execute_max_cycles() const noexcept override;
	virtual void execute_run() override;
	virtual space_config_vector memory_space_config() const override;
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	address_space_config m_prg1_config, m_prg2_config, m_data_config;
	address_space *m_data;

	required_shared_ptr<u64> m_prg1, m_prg2;

	std::array<u16, 0x20> m_buffer;

	u32 m_pc;
	int m_icount;

	u32 m_data_adr;
	u16 m_status;

	// Data ram access
	void data_adrh_w(u16 data);
	void data_adrl_w(u16 data);
	void data_data_w(u16 data);
	void data_zero_w(u16 data);

	// Program ram access
	void prg_adr_w(u16 data);
	void prg_data_w(offs_t offset, u16 data);
	u16 prg_data_r(offs_t offset);

	// Registers
	u16 status_r();

	// Generic catch-all
	u16 snd_r(offs_t offset);
	void snd_w(offs_t offset, u16 data);

	void prg1_map(address_map &map) ATTR_COLD;
	void prg2_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(DSPV, dspv_device)

#endif // MAME_SOUND_DSPV_H

// license:GPL-2.0+
// copyright-holders:Kevin Thacker
/*****************************************************************************
 *
 * includes/spec128.h
 *
 ****************************************************************************/

#ifndef MAME_SINCLAIR_SPEC128_H
#define MAME_SINCLAIR_SPEC128_H

#pragma once

#include "spectrum.h"


class spectrum_128_state : public spectrum_state
{
public:
	spectrum_128_state(const machine_config &mconfig, device_type type, const char *tag) :
		spectrum_state(mconfig, type, tag),
		m_bank_rom(*this, "bank_rom%u", 0U),
		m_bank_ram(*this, "bank_ram%u", 0U)
	{ }

	void spectrum_128(machine_config &config);

protected:
	memory_bank_array_creator<1> m_bank_rom;
	memory_bank_array_creator<4> m_bank_ram;

	virtual void video_start() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void spectrum_128_update_memory() override;
	virtual rectangle get_screen_area() override;

	virtual bool is_contended(offs_t offset) override;
	virtual bool is_vram_write(offs_t offset) override;

	template <u8 Bank>
	void spectrum_128_ram_w(offs_t offset, u8 data);
	template <u8 Bank>
	u8 spectrum_128_ram_r(offs_t offset);

private:
	u8 spectrum_128_pre_opcode_fetch_r(offs_t offset);
	void spectrum_128_rom_w(offs_t offset, u8 data);
	u8 spectrum_128_rom_r(offs_t offset);
	void spectrum_128_port_7ffd_w(offs_t offset, u8 data);
	virtual uint8_t spectrum_port_r(offs_t offset) override;
	//uint8_t spectrum_128_ula_r();

	void spectrum_128_io(address_map &map) ATTR_COLD;
	void spectrum_128_mem(address_map &map) ATTR_COLD;
	void spectrum_128_fetch(address_map &map) ATTR_COLD;
};

#define X1_128_AMSTRAD  35'469'000       // Main clock (Amstrad 128K model, +2A?)
#define X1_128_SINCLAIR 35.469_MHz_XTAL  // Main clock (Sinclair 128K model)

/* 128K machines take an extra 4 cycles per scan line - add this to retrace */
#define SPEC128_UNSEEN_LINES    15
#define SPEC128_RETRACE_CYCLES  52
#define SPEC128_CYCLES_PER_LINE 228

#endif // MAME_SINCLAIR_SPEC128_H

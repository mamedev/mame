// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Nigel Barnes
/*****************************************************************************
 *
 * Acorn Electron
 *
 * Driver by Wilbert Pol
 *
 ****************************************************************************/
#ifndef MAME_ACORN_ELECTRON_H
#define MAME_ACORN_ELECTRON_H

#pragma once

#include "electron_ula.h"

#include "bus/bbc/userport/userport.h"
#include "bus/electron/exp.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "imagedev/cassette.h"
#include "machine/6522via.h"
#include "machine/input_merger.h"
#include "machine/ram.h"

#include "screen.h"


class electron_state : public driver_device
{
public:
	electron_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_irqs(*this, "irqs")
		, m_screen(*this, "screen")
		, m_cassette(*this, "cassette")
		, m_ula(*this, "ula")
		, m_region_mos(*this, "mos")
		, m_keybd(*this, "LINE.%u", 0)
		, m_exp(*this, "exp")
		, m_ram(*this, RAM_TAG)
		, m_mrb(*this, "MRB")
		, m_capslock_led(*this, "capslock_led")
	{ }

	void electron(machine_config &config);
	void btm2105(machine_config &config);

	void electron64(machine_config &config);

	static void plus3_default(device_t* device);

	DECLARE_INPUT_CHANGED_MEMBER( trigger_reset );

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t fetch_r(offs_t offset);
	uint8_t ram_r(offs_t offset);
	void ram_w(offs_t offset, uint8_t data);
	virtual uint8_t rom_r(offs_t offset);
	virtual void rom_w(offs_t offset, uint8_t data);
	virtual uint8_t io_r(offs_t offset);
	virtual void io_w(offs_t offset, uint8_t data);
	uint8_t keyboard_r(offs_t offset);

	void mem_map(address_map &map) ATTR_COLD;
	void opcodes_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<input_merger_device> m_irqs;
	required_device<screen_device> m_screen;
	required_device<cassette_image_device> m_cassette;
	required_device<electron_ula_device> m_ula;
	required_memory_region m_region_mos;
	required_ioport_array<14> m_keybd;
	required_device<electron_expansion_slot_device> m_exp;
	required_device<ram_device> m_ram;
	optional_ioport m_mrb;
	output_finder<> m_capslock_led;

	bool m_mrb_mapped = false;
	bool m_vdu_drivers = false;
};


class electronsp_state : public electron_state
{
public:
	electronsp_state(const machine_config &mconfig, device_type type, const char *tag)
		: electron_state(mconfig, type, tag)
		, m_region_sp64(*this, "sp64")
		, m_via(*this, "via6522")
		, m_userport(*this, "userport")
		, m_romi(*this, "romi%u", 1)
		, m_rompages(*this, "ROMPAGES")
	{ }

	void electronsp(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	virtual uint8_t rom_r(offs_t offset) override;
	virtual void rom_w(offs_t offset, uint8_t data) override;
	virtual uint8_t io_r(offs_t offset) override;
	virtual void io_w(offs_t offset, uint8_t data) override;

private:
	required_memory_region m_region_sp64;
	required_device<via6522_device> m_via;
	required_device<bbc_userport_slot_device> m_userport;
	required_device_array<generic_slot_device, 2> m_romi;
	required_ioport m_rompages;

	uint8_t m_rompage = 0;
	uint8_t m_sp64_bank = 0;
	std::unique_ptr<uint8_t[]> m_sp64_ram;

	std::pair<std::error_condition, std::string> load_rom(device_image_interface &image, generic_slot_device *slot);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom1_load) { return load_rom(image, m_romi[0]); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom2_load) { return load_rom(image, m_romi[1]); }
};

#endif // MAME_ACORN_ELECTRON_H

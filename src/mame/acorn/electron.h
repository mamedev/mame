// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Nigel Barnes
/*****************************************************************************
 *
 * includes/electron.h
 *
 * Acorn Electron
 *
 * Driver by Wilbert Pol
 *
 ****************************************************************************/
#ifndef MAME_ACORN_ELECTRON_H
#define MAME_ACORN_ELECTRON_H

#pragma once

#include "machine/ram.h"
#include "machine/6522via.h"
#include "machine/input_merger.h"
#include "imagedev/cassette.h"
#include "sound/beep.h"
#include "emupal.h"

#include "bus/electron/exp.h"
#include "bus/bbc/userport/userport.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

/* Interrupts */
#define INT_HIGH_TONE       0x40
#define INT_TRANSMIT_EMPTY  0x20
#define INT_RECEIVE_FULL    0x10
#define INT_RTC             0x08
#define INT_DISPLAY_END     0x04
#define INT_SET             0x100
#define INT_CLEAR           0x200

class electron_state : public driver_device
{
public:
	electron_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_irqs(*this, "irqs")
		, m_screen(*this, "screen")
		, m_cassette(*this, "cassette")
		, m_beeper(*this, "beeper")
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
	emu_timer *m_tape_timer = nullptr;
	emu_timer *m_beep_timer = nullptr;
	int m_map4[256]{};
	int m_map16[256]{};
	emu_timer *m_scanline_timer = nullptr;
	uint8_t electron64_fetch_r(offs_t offset);
	uint8_t electron_mem_r(offs_t offset);
	void electron_mem_w(offs_t offset, uint8_t data);
	virtual uint8_t electron_paged_r(offs_t offset);
	virtual void electron_paged_w(offs_t offset, uint8_t data);
	uint8_t electron_mos_r(offs_t offset);
	void electron_mos_w(offs_t offset, uint8_t data);
	virtual uint8_t electron_fred_r(offs_t offset);
	virtual void electron_fred_w(offs_t offset, uint8_t data);
	uint8_t electron_jim_r(offs_t offset);
	void electron_jim_w(offs_t offset, uint8_t data);
	uint8_t electron_sheila_r(offs_t offset);
	void electron_sheila_w(offs_t offset, uint8_t data);

	void electron_colours(palette_device &palette) const;
	uint32_t screen_update_electron(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(electron_tape_timer_handler);
	TIMER_CALLBACK_MEMBER(setup_beep);
	TIMER_CALLBACK_MEMBER(electron_scanline_interrupt);

	inline uint8_t read_vram( uint16_t addr );
	inline void electron_plot_pixel(bitmap_ind16 &bitmap, int x, int y, uint32_t color);
	void electron_interrupt_handler(int mode, int interrupt);

	void electron_mem(address_map &map) ATTR_COLD;

	void electron64_opcodes(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<input_merger_device> m_irqs;
	required_device<screen_device> m_screen;
	required_device<cassette_image_device> m_cassette;
	required_device<beep_device> m_beeper;
	required_memory_region m_region_mos;
	required_ioport_array<14> m_keybd;
	required_device<electron_expansion_slot_device> m_exp;
	required_device<ram_device> m_ram;
	optional_ioport m_mrb;
	output_finder<> m_capslock_led;

	void waitforramsync();
	void electron_tape_start();
	void electron_tape_stop();

	/* ULA context */
	struct ULA
	{
		uint8_t interrupt_status = 0;
		uint8_t interrupt_control = 0;
		uint8_t rompage = 0;
		uint16_t screen_start = 0;
		uint16_t screen_base = 0;
		uint16_t screen_size = 0;
		uint16_t screen_addr = 0;
		int screen_dispend = 0;
		int current_pal[16]{};
		int communication_mode = 0;
		int screen_mode = 0;
		int cassette_motor_mode = 0;
		int capslock_mode = 0;
		/* tape reading related */
		uint32_t tape_value = 0;
		int tape_steps = 0;
		int bit_count = 0;
		int high_tone_set = 0;
		int start_bit = 0;
		int stop_bit = 0;
		int tape_running = 0;
		uint8_t tape_byte = 0;
	};

	ULA m_ula;
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
	virtual uint8_t electron_paged_r(offs_t offset) override;
	virtual void electron_paged_w(offs_t offset, uint8_t data) override;
	virtual uint8_t electron_fred_r(offs_t offset) override;
	virtual void electron_fred_w(offs_t offset, uint8_t data) override;

	virtual void machine_start() override ATTR_COLD;

private:
	required_memory_region m_region_sp64;
	required_device<via6522_device> m_via;
	required_device<bbc_userport_slot_device> m_userport;
	required_device_array<generic_slot_device, 2> m_romi;
	required_ioport m_rompages;

	uint8_t m_sp64_bank = 0;
	std::unique_ptr<uint8_t[]> m_sp64_ram;

	std::pair<std::error_condition, std::string> load_rom(device_image_interface &image, generic_slot_device *slot);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom1_load) { return load_rom(image, m_romi[0]); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom2_load) { return load_rom(image, m_romi[1]); }
};


#endif // MAME_ACORN_ELECTRON_H

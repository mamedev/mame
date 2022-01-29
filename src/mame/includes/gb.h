// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*****************************************************************************
 *
 * includes/gb.h
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_GB_H
#define MAME_INCLUDES_GB_H

#pragma once

#include "sound/gb.h"
#include "cpu/lr35902/lr35902.h"
#include "bus/gameboy/gb_slot.h"
#include "machine/ram.h"
#include "video/gb_lcd.h"
#include "emupal.h"


class gb_state : public driver_device
{
public:
	gb_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_cartslot(*this, "gbslot"),
		m_maincpu(*this, "maincpu"),
		m_apu(*this, "apu"),
		m_region_maincpu(*this, "maincpu"),
		m_rambank(*this, "cgb_ram"),
		m_inputs(*this, "INPUTS"),
		m_bios_hack(*this, "SKIP_CHECK"),
		m_ram(*this, RAM_TAG),
		m_ppu(*this, "ppu"),
		m_palette(*this, "palette"),
		m_cart_low(*this, "cartlow"),
		m_cart_high(*this, "carthigh")
	{ }

	uint8_t       m_gb_io[0x10];

	/* Timer related */
	uint16_t      m_divcount;
	uint8_t       m_shift;
	uint16_t      m_shift_cycles;
	uint8_t       m_triggering_irq;
	uint8_t       m_reloading;

	/* Serial I/O related */
	uint16_t      m_internal_serial_clock;
	uint16_t      m_internal_serial_frequency;
	uint32_t      m_sio_count;             /* Serial I/O counter */

	/* SGB variables */
	int8_t m_sgb_packets;
	uint8_t m_sgb_bitcount;
	uint8_t m_sgb_bytecount;
	uint8_t m_sgb_start;
	uint8_t m_sgb_rest;
	uint8_t m_sgb_controller_no;
	uint8_t m_sgb_controller_mode;
	uint8_t m_sgb_data[0x100];

	/* CGB variables */
	uint8_t       *m_gbc_rammap[8];           /* (CGB) Addresses of internal RAM banks */
	uint8_t       m_gbc_rambank;          /* (CGB) Current CGB RAM bank */

	void gb_io_w(offs_t offset, uint8_t data);
	void gb_io2_w(offs_t offset, uint8_t data);
	void sgb_io_w(offs_t offset, uint8_t data);
	uint8_t gb_ie_r();
	void gb_ie_w(uint8_t data);
	uint8_t gb_io_r(offs_t offset);
	void gbc_io_w(offs_t offset, uint8_t data);
	void gbc_io2_w(offs_t offset, uint8_t data);
	uint8_t gbc_io2_r(offs_t offset);
	void gb_palette(palette_device &palette) const;
	DECLARE_MACHINE_START(sgb);
	DECLARE_MACHINE_RESET(sgb);
	void sgb_palette(palette_device &palette) const;
	void gbp_palette(palette_device &palette) const;
	DECLARE_MACHINE_START(gbc);
	DECLARE_MACHINE_RESET(gbc);
	void gbc_palette(palette_device &palette) const;
	void gb_timer_callback(uint8_t data);

	uint8_t gb_bios_r(offs_t offset);
	optional_device<gb_cart_slot_device> m_cartslot;

	void supergb(machine_config &config);
	void supergb2(machine_config &config);
	void gbcolor(machine_config &config);
	void gbpocket(machine_config &config);
	void gameboy(machine_config &config);
	void gameboy_map(address_map &map);
	void gbc_map(address_map &map);
	void sgb_map(address_map &map);

protected:
	enum {
		SIO_ENABLED = 0x80,
		SIO_FAST_CLOCK = 0x02,
		SIO_INTERNAL_CLOCK = 0x01
	};
	static constexpr u8 NO_CART = 0x00;
	static constexpr u8 BIOS_ENABLED = 0x00;
	static constexpr u8 CART_PRESENT = 0x01;
	static constexpr u8 BIOS_DISABLED = 0x02;

	required_device<lr35902_cpu_device> m_maincpu;
	required_device<gameboy_sound_device> m_apu;
	required_memory_region m_region_maincpu;
	optional_memory_bank m_rambank;   // cgb
	required_ioport m_inputs;
	required_ioport m_bios_hack;
	optional_device<ram_device> m_ram;
	required_device<dmg_ppu_device> m_ppu;
	required_device<palette_device> m_palette;
	memory_view m_cart_low;
	memory_view m_cart_high;

	void gb_timer_increment();
	void gb_timer_check_irq();
	void gb_init();
	void gb_init_regs();
	void gb_serial_timer_tick();

	void save_gb_base();
	void save_gbc_only();
	void save_sgb_only();

	virtual void machine_start() override;
	virtual void machine_reset() override;
};


class megaduck_state : public gb_state
{
public:
	megaduck_state(const machine_config &mconfig, device_type type, const char *tag) :
		gb_state(mconfig, type, tag),
		m_cartslot(*this, "duckslot")
	{ }

	void megaduck(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	uint8_t megaduck_video_r(offs_t offset);
	void megaduck_video_w(offs_t offset, uint8_t data);
	void megaduck_sound_w1(offs_t offset, uint8_t data);
	uint8_t megaduck_sound_r1(offs_t offset);
	void megaduck_sound_w2(offs_t offset, uint8_t data);
	uint8_t megaduck_sound_r2(offs_t offset);
	void megaduck_palette(palette_device &palette) const;
	void megaduck_map(address_map &map);

	required_device<megaduck_cart_slot_device> m_cartslot;
};

#endif // MAME_INCLUDES_GB_H

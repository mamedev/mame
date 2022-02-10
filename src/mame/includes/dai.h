// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha, Nathan Woods
/*****************************************************************************
 *
 * includes/dai.h
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_DAI_H
#define MAME_INCLUDES_DAI_H

#include "audio/dai_snd.h"

#include "cpu/i8085/i8085.h"
#include "imagedev/cassette.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/timer.h"
#include "machine/tms5501.h"

#include "emupal.h"


class dai_state : public driver_device
{
public:
	dai_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pit(*this, "pit")
		, m_tms5501(*this, "tms5501")
		, m_sound(*this, "custom")
		, m_cassette(*this, "cassette")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		, m_palette(*this, "palette")
		, m_io_keyboard(*this, "IN%u", 0U)
		, m_tms_timer(*this, "tms_timer")
	{ }

	void dai(machine_config &config);

private:
	u8 m_paddle_select;
	u8 m_paddle_enable;
	u8 m_cassette_motor[2];
	u8 m_keyboard_scan_mask;
	u8 m_4_colours_palette[4];
	void stack_interrupt_circuit_w(u8 data);
	u8 io_discrete_devices_r(offs_t offset);
	void io_discrete_devices_w(offs_t offset, u8 data);
	u8 amd9511_r();
	void amd9511_w(offs_t offset, u8 data);
	u8 pit_r(offs_t offset);
	void pit_w(offs_t offset, u8 data);
	u8 keyboard_r();
	void keyboard_w(u8 data);
	void dai_palette(palette_device &palette) const;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	IRQ_CALLBACK_MEMBER(int_ack);
	TIMER_DEVICE_CALLBACK_MEMBER(tms_timer);

	void mem_map(address_map &map);

	static const rgb_t s_palette[16];

	virtual void machine_start() override;
	virtual void machine_reset() override;

	memory_passthrough_handler m_rom_shadow_tap;
	required_device<cpu_device> m_maincpu;
	required_device<pit8253_device> m_pit;
	required_device<tms5501_device> m_tms5501;
	required_device<dai_sound_device> m_sound;
	required_device<cassette_image_device> m_cassette;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
	required_device<palette_device> m_palette;
	required_ioport_array<9> m_io_keyboard;
	required_device<timer_device> m_tms_timer;
};


#endif // MAME_INCLUDES_DAI_H

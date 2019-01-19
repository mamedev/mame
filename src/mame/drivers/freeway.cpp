// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    FreeWay (c) 1999 NVC Electronica Ltd.

    Skeleton driver.

*********************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "video/mc6845.h"
#include "screen.h"

class freeway_state : public driver_device
{
public:
	freeway_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pic(*this, "pic")
	{
	}

	void freeway(machine_config &config);

private:
	MC6845_UPDATE_ROW(update_row);

	void mem_map(address_map &map);
	void io_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic;
};

MC6845_UPDATE_ROW(freeway_state::update_row)
{
}

void freeway_state::mem_map(address_map &map)
{
	map(0x00000, 0x07fff).ram();
	map(0xa0000, 0xa0fff).ram(); // video RAM?
	map(0xa4000, 0xa4fff).ram(); // video RAM?
	map(0xf0000, 0xfffff).rom().region("program", 0);
}

void freeway_state::io_map(address_map &map)
{
	map(0x0020, 0x0021).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x0030, 0x0033).w("pit", FUNC(pit8254_device::write));
	map(0x03d0, 0x03d0).w("crtc", FUNC(mc6845_device::address_w));
	map(0x03d1, 0x03d1).w("crtc", FUNC(mc6845_device::register_w));
}

static INPUT_PORTS_START(freeway)
INPUT_PORTS_END

void freeway_state::freeway(machine_config &config)
{
	I8088(config, m_maincpu, 10_MHz_XTAL / 2); // divider unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &freeway_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &freeway_state::io_map);
	m_maincpu->set_irq_acknowledge_callback("pic", FUNC(pic8259_device::inta_cb));

	PIC8259(config, m_pic);
	m_pic->out_int_callback().set_inputline(m_maincpu, 0);

	PIT8254(config, "pit");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(10_MHz_XTAL / 2, 320, 0, 256, 312, 0, 256);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	mc6845_device &crtc(MC6845(config, "crtc", 10_MHz_XTAL / 16));
	crtc.set_char_width(8);
	crtc.set_show_border_area(false);
	crtc.set_screen("screen");
	crtc.set_update_row_callback(FUNC(freeway_state::update_row), this);
}

// 8088 CPU
// Intel 8254 Programmable Interval Timer
// Intel 8259
// 2x 8k SRAM
// 1x 32k SRAM
// 6845 video chip
// 5 roms
// Oscillator 10 MHz

ROM_START(freeway)
	ROM_REGION(0x10000, "program", 0)
	ROM_LOAD("vip88.bin", 0x00000, 0x10000, CRC(aeba6d5e) SHA1(bb84f7040bf1b6976cb2c50b1ffdc59ae88df223))

	ROM_REGION(0x20000, "gfx", 0) // loading might be wrong
	ROM_LOAD("sb_51.bin",  0x00000, 0x8000, CRC(d25bd328) SHA1(b8c692298f6dc5fd5ae2f9e7701e14b0436a95bb)) // xxx0xxxxxxxxxxx = 0xFF
	ROM_LOAD("sb_52.bin",  0x08000, 0x8000, CRC(f2b33acd) SHA1(e4786b4f00871d771aadacd9d6ec767691f4d939))
	ROM_LOAD("sb_53.bin",  0x10000, 0x8000, CRC(50407ae6) SHA1(2c6c4803905bed5f27c6783f99a24f8dee62c19b))
	ROM_LOAD("sb_cor.bin", 0x18000, 0x8000, CRC(5f86a160) SHA1(f21b7e0e6a407371c252d6fde6fcb32a2682824c)) // 00000xxxxxxxxxx = 0xFF
ROM_END


GAME(1999, freeway, 0, freeway, freeway, freeway_state, empty_init, ROT0, "NVC Electronica", "FreeWay (V5.12)", MACHINE_IS_SKELETON)

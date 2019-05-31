// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for Systel System 1/System 100 computer.

    This minimalistic luggable computer has a green-screen monitor and
    5¼-inch floppy drive (Shugart SA455) built into the unit. The detached
    keyboard may be replaced with a standard electric typewriter. There is
    a TYPE/WP switch on the front of the unit.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/clock.h"
#include "machine/input_merger.h"
#include "machine/i8251.h"
#include "machine/i8257.h"
#include "machine/wd_fdc.h"
#include "video/i8275.h"
#include "screen.h"

class systel1_state : public driver_device
{
public:
	systel1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dmac(*this, "dmac")
		, m_chargen(*this, "chargen")
	{
	}

	void systel1(machine_config &config);

private:
	DECLARE_WRITE_LINE_MEMBER(hrq_w);
	u8 memory_r(offs_t offset);
	void memory_w(offs_t offset, u8 data);

	I8275_DRAW_CHARACTER_MEMBER(draw_character);

	void mem_map(address_map &map);
	void io_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<i8257_device> m_dmac;
	required_region_ptr<u8> m_chargen;
};

WRITE_LINE_MEMBER(systel1_state::hrq_w)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state);
	m_dmac->hlda_w(state);
}

u8 systel1_state::memory_r(offs_t offset)
{
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

void systel1_state::memory_w(offs_t offset, u8 data)
{
	m_maincpu->space(AS_PROGRAM).write_byte(offset, data);
}

I8275_DRAW_CHARACTER_MEMBER(systel1_state::draw_character)
{
}

void systel1_state::mem_map(address_map &map)
{
	map(0x0000, 0x07ff).rom().region("program", 0);
	map(0x4000, 0xffff).ram();
}

void systel1_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("JUMPERS");
	map(0x10, 0x11).rw("usart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x20, 0x29).rw(m_dmac, FUNC(i8257_device::read), FUNC(i8257_device::write));
	map(0x40, 0x41).rw("crtc", FUNC(i8276_device::read), FUNC(i8276_device::write));
}

static INPUT_PORTS_START(systel1)
	PORT_START("JUMPERS") // probably at J6
	PORT_DIPNAME(0x02, 0x00, "Screen Refresh")
	PORT_DIPSETTING(0x02, "50 Hz")
	PORT_DIPSETTING(0x00, "60 Hz")
INPUT_PORTS_END

void systel1_state::systel1(machine_config &config)
{
	Z80(config, m_maincpu, 2_MHz_XTAL); // Z8400A; clock not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &systel1_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &systel1_state::io_map);

	input_merger_device &mainint(INPUT_MERGER_ANY_HIGH(config, "mainint"));
	mainint.output_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	I8257(config, m_dmac, 2_MHz_XTAL); // P8257-5
	m_dmac->out_hrq_cb().set(FUNC(systel1_state::hrq_w));
	m_dmac->in_memr_cb().set(FUNC(systel1_state::memory_r));
	m_dmac->out_memw_cb().set(FUNC(systel1_state::memory_w));
	m_dmac->out_iow_cb<2>().set("crtc", FUNC(i8276_device::dack_w));

	i8251_device &usart(I8251(config, "usart", 2_MHz_XTAL)); // AMD P8251A
	usart.rxrdy_handler().set("mainint", FUNC(input_merger_device::in_w<0>));

	clock_device &baudclock(CLOCK(config, "baudclock", 2_MHz_XTAL / 13)); // rate not verified, but also probably fixed
	baudclock.signal_handler().set("usart", FUNC(i8251_device::write_rxc));
	baudclock.signal_handler().append("usart", FUNC(i8251_device::write_txc));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green());
	screen.set_raw(10.8864_MHz_XTAL, 672, 0, 560, 270, 0, 250);
	screen.set_screen_update("crtc", FUNC(i8276_device::screen_update));

	i8276_device &crtc(I8276(config, "crtc", 10.8864_MHz_XTAL / 7)); // WD8276PL-00
	crtc.set_character_width(7);
	crtc.set_display_callback(FUNC(systel1_state::draw_character));
	crtc.drq_wr_callback().set(m_dmac, FUNC(i8257_device::dreq2_w));
	crtc.irq_wr_callback().set("mainint", FUNC(input_merger_device::in_w<1>));

	FD1797(config, "fdc", 2_MHz_XTAL);
}

// XTALs: 10.8864 (Y1), 2.000 (Y2)
// Silkscreened on PCB: "SYSTEL / SYSTEM 1 / 1031-3101-00 REV H"
ROM_START(systel100)
	ROM_REGION(0x800, "program", 0) // TMS2716JL-45
	ROM_LOAD("u11.bin", 0x000, 0x800, CRC(29f1414a) SHA1(c87adfaaec45d0e5f4cf5946d2f04de0332b3413))

	ROM_REGION(0x800, "chargen", 0) // TMS2516JL-45
	ROM_LOAD("u16.bin", 0x000, 0x800, CRC(61a8d742) SHA1(69dada638a17353f91bff34a1e2319a35d8a3ebf))
ROM_END

COMP(198?, systel100, 0, 0, systel1, systel1, systel1_state, empty_init, "Systel Computer", "System 100", MACHINE_IS_SKELETON)

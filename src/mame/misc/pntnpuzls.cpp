// license:BSD-3-Clause
// copyright-holders:

/*
Paint 'N Puzzle Super (or Super Paint 'N Puzzle)
VIDEO PUZZLE V1.0 PCB
(C) 1994 Green Concepts International

Main components:
N80286-12 CPU
3.579545 XTAL (near CPU and empty socket at u11)
27.50000 MHz XTAL
4x GM76C256ALL-70 RAM
GM68B45S CRTC
HM86171-80 RAMDAC
KS82C54 PIT
W82C59 PIC
W86C450 UART
1.8432 MHz XTAL (near UART)
Card connector
Ticket connector
*/

#include "emu.h"

#include "cpu/i86/i286.h"
#include "machine/ins8250.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "video/mc6845.h"
#include "video/ramdac.h"

#include "screen.h"


namespace {

class pntnpuzls_state : public driver_device
{
public:
	pntnpuzls_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu")
	{ }

	void pntnpuzls(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


uint32_t pntnpuzls_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	return 0;
}


void pntnpuzls_state::program_map(address_map &map)
{
	map(0xff8000, 0xffffff).rom().region("maincpu", 0);
}


// no DIP switches on PCB
static INPUT_PORTS_START( pntnpuzls )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// TODO: touchscreen
INPUT_PORTS_END

void pntnpuzls_state::pntnpuzls(machine_config &config)
{
	I80286(config, m_maincpu, 27'500'000 / 2); // clock / divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &pntnpuzls_state::program_map);

	PIC8259(config, "pic");

	PIT8254(config, "pit");

	INS8250(config, "uart", 1.8432_MHz_XTAL);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: everything
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(pntnpuzls_state::screen_update));

	PALETTE(config, "palette").set_entries(0x100); // TODO

	RAMDAC(config, "ramdac", 0, "palette");

	mc6845_device &crtc(MC6845(config, "crtc", 27'500'000 / 30)); // clock / divider not verified
	crtc.set_char_width(8);
	crtc.set_show_border_area(false);
	crtc.set_screen("screen");

	// TODO: sound? missing chip at u11?
}

ROM_START( pntnpuzls )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "u8.u8", 0x0000, 0x8000, CRC(269c2aed) SHA1(466da89a1a4c69668d2e0d6e49f3e20ec5b03d29) )
	ROM_LOAD16_BYTE( "u7.u7", 0x0001, 0x8000, CRC(39ca5f74) SHA1(b9256e296ff248ddc1d5c76fc5a1748a6a86ac80) )

	ROM_REGION( 0x80000, "font", 0 )
	ROM_LOAD16_BYTE( "pnp_1.1d_font_even.u6", 0x00000, 0x40000, CRC(c6af5a61) SHA1(737bf2b2e4e42124bcde60e7a00be42f1b7f32d2) )
	ROM_LOAD16_BYTE( "pnp_1.1d_font_odd.u5",  0x00001, 0x40000, CRC(6c0aa161) SHA1(190ff150d149101ab26b41640acdac023c3f11b1) )
ROM_END

} // anonymous namespace


GAME( 1994, pntnpuzls, 0, pntnpuzls, pntnpuzls, pntnpuzls_state, empty_init, ROT90, "Century Vending / Green Concepts International", "Paint 'N Puzzle Super", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

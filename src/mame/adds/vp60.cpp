// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for ADDS Viewpoint 60 terminal.
No significant progress can be made until the 8051 has its internal ROM dumped.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/mcs51/mcs51.h"
//#include "machine/er2055.h"
#include "video/i8275.h"
#include "screen.h"


namespace {

class vp60_state : public driver_device
{
public:
	vp60_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_crtc(*this, "crtc")
		, m_p_chargen(*this, "chargen")
	{ }

	void vp60(machine_config &config);

private:
	I8275_DRAW_CHARACTER_MEMBER(draw_character);
	u8 crtc_r(offs_t offset);
	void crtc_w(offs_t offset, u8 data);

	void io_map(address_map &map) ATTR_COLD;
	void kbd_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<i8275_device> m_crtc;
	required_region_ptr<u8> m_p_chargen;
};

I8275_DRAW_CHARACTER_MEMBER(vp60_state::draw_character)
{
}

u8 vp60_state::crtc_r(offs_t offset)
{
	return m_crtc->read(offset >> 8);
}

void vp60_state::crtc_w(offs_t offset, u8 data)
{
	m_crtc->write(offset >> 8, data);
}

void vp60_state::mem_map(address_map &map)
{
	map(0x0000, 0x2fff).rom().region("maincpu", 0);
}

void vp60_state::io_map(address_map &map)
{
	map(0x8000, 0x87ff).ram();
	map(0xc000, 0xc000).select(0x100).mirror(0xff).rw(FUNC(vp60_state::crtc_r), FUNC(vp60_state::crtc_w));
}

void vp60_state::kbd_map(address_map &map)
{
	map(0x000, 0x3ff).rom().region("keyboard", 0);
}

static INPUT_PORTS_START( vp60 )
INPUT_PORTS_END

void vp60_state::vp60(machine_config &config)
{
	I8051(config, m_maincpu, 10.92_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &vp60_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &vp60_state::io_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.92_MHz_XTAL, 1600, 0, 1280, 270, 0, 250);
	//screen.set_raw(25.92_MHz_XTAL, 1632, 0, 1280, 319, 0, 275);
	screen.set_screen_update("crtc", FUNC(i8275_device::screen_update));

	I8275(config, m_crtc, 25.92_MHz_XTAL / 16);
	m_crtc->set_character_width(16);
	m_crtc->set_display_callback(FUNC(vp60_state::draw_character));
	m_crtc->set_screen("screen");

	i8035_device &kbdcpu(I8035(config, "kbdcpu", 3.579545_MHz_XTAL)); // 48-300-010 XTAL
	kbdcpu.set_addrmap(AS_PROGRAM, &vp60_state::kbd_map);
}


/**************************************************************************************************************

ADDS Viewpoint 60.
Chips: P8051, P8275, EAROM ER-2055, HM6116P-4
Crystals: 25.92, 10.920
Keyboard: INS8035N-6, crystal marked 48-300-010.

***************************************************************************************************************/

ROM_START( vp60 )
	ROM_REGION(0x4000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD( "p8051.ub1",  0x0000, 0x1000, NO_DUMP ) // internal ROM not dumped
	ROM_LOAD( "pgm.uc1",    0x2000, 0x1000, CRC(714ca569) SHA1(405424369fd5458e02c845c104b2cb386bd857d2) )
	ROM_CONTINUE(           0x1000, 0x1000 )
	// Stubs filling in for missing code
	ROM_FILL( 0x0000, 1, 0x02 )
	ROM_FILL( 0x0001, 1, 0x10 )
	ROM_FILL( 0x0002, 1, 0x09 )
	ROM_FILL( 0x005d, 1, 0x02 )
	ROM_FILL( 0x005e, 1, 0x27 )
	ROM_FILL( 0x005f, 1, 0x2e )
	ROM_FILL( 0x0100, 1, 0x22 )
	ROM_FILL( 0x0500, 1, 0x22 )
	ROM_FILL( 0x0600, 1, 0x22 )
	ROM_FILL( 0x0800, 1, 0x22 )

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "font.uc4",   0x0000, 0x1000, CRC(3c4d39c0) SHA1(9503c0d5a76e8073c94c86be57bcb312641f6cc4) )

	ROM_REGION(0x400, "keyboard", 0)
	ROM_LOAD( "195.kbd",    0x0000, 0x0400, CRC(14885da3) SHA1(3b06f658af1a62b28e62d8b3a557b74169917a12) )
ROM_END

} // anonymous namespace


COMP( 1982, vp60, 0, 0, vp60, vp60, vp60_state, empty_init, "ADDS", "Viewpoint 60", MACHINE_IS_SKELETON )

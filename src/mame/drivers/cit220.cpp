// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for VT220-compatible terminals by C. Itoh/CIE Terminals.

The CIT-220+ Video Terminal was introduced as a direct competitor to DEC's VT220. It copied the design of the VT220 closely
enough to provoke a lawsuit, which led to its eventual withdrawal in favor of its successor, the CIT224.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/i8251.h"
#include "machine/mc68681.h"
#include "video/scn2674.h"
#include "screen.h"


class cit220_state : public driver_device
{
public:
	cit220_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		//, m_p_chargen(*this, "chargen")
	{ }

	void cit220p(machine_config &config);
private:
	virtual void machine_start() override;

	DECLARE_WRITE_LINE_MEMBER(sod_w);
	SCN2674_DRAW_CHARACTER_MEMBER(draw_character);

	void io_map(address_map &map);
	void mem_map(address_map &map);
	void char_map(address_map &map);
	void attr_map(address_map &map);
	void keyboard_map(address_map &map);
	void kbd_io_map(address_map &map);

	required_device<i8085a_cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	//required_region_ptr<u8> m_p_chargen;
};


void cit220_state::machine_start()
{
	subdevice<i8251_device>("usart")->write_cts(0);
}


WRITE_LINE_MEMBER(cit220_state::sod_w)
{
	// probably asserts PBREQ on SCN2674 to access memory at Exxx
}

void cit220_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0x87ff).ram();
	map(0xa000, 0xa1ff).rom().region("eeprom", 0x800);
	map(0xe000, 0xe7ff).ram(); // ???
}

void cit220_state::io_map(address_map &map)
{
	map(0x00, 0x0f).rw("duart", FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0x10, 0x11).rw("usart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x20, 0x27).rw("avdc", FUNC(scn2674_device::read), FUNC(scn2674_device::write));
	map(0xa0, 0xa0).rw("avdc", FUNC(scn2674_device::attr_buffer_r), FUNC(scn2674_device::attr_buffer_w));
	map(0xc0, 0xc0).rw("avdc", FUNC(scn2674_device::buffer_r), FUNC(scn2674_device::buffer_w));
}


SCN2674_DRAW_CHARACTER_MEMBER(cit220_state::draw_character)
{
}

void cit220_state::char_map(address_map &map)
{
	map(0x0000, 0x2fff).ram();
}

void cit220_state::attr_map(address_map &map)
{
	map(0x0000, 0x2fff).ram();
}

void cit220_state::keyboard_map(address_map &map)
{
	map(0x000, 0xfff).rom().region("keyboard", 0);
}

void cit220_state::kbd_io_map(address_map &map)
{
}


static INPUT_PORTS_START( cit220p )
INPUT_PORTS_END


void cit220_state::cit220p(machine_config &config)
{
	I8085A(config, m_maincpu, 6000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &cit220_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &cit220_state::io_map);
	m_maincpu->out_sod_func().set(FUNC(cit220_state::sod_w));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(24553200, 1580, 0, 1320, 259, 0, 240); // dot clock guessed
	m_screen->set_screen_update("avdc", FUNC(scn2674_device::screen_update));

	scn2674_device &avdc(SCN2674(config, "avdc", 24553200 / 10));
	avdc.intr_callback().set_inputline(m_maincpu, I8085_RST65_LINE);
	avdc.set_character_width(10);
	avdc.set_display_callback(FUNC(cit220_state::draw_character));
	avdc.set_addrmap(0, &cit220_state::char_map);
	avdc.set_addrmap(1, &cit220_state::attr_map);
	avdc.set_screen(m_screen);

	scn2681_device &duart(SCN2681(config, "duart", 3686400));
	duart.irq_cb().set_inputline("maincpu", I8085_RST55_LINE);
	duart.outport_cb().set("usart", FUNC(i8251_device::write_txc)).bit(3); // 9600 baud?
	duart.outport_cb().append("usart", FUNC(i8251_device::write_rxc)).bit(3);

	I8251(config, "usart", 3000000);

	mcs48_cpu_device &kbdmcu(I8035(config, "kbdmcu", 4608000));
	kbdmcu.set_addrmap(AS_PROGRAM, &cit220_state::keyboard_map);
	kbdmcu.set_addrmap(AS_IO, &cit220_state::kbd_io_map);
}


ROM_START( cit220p )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "v17_001a.ic23", 0x0000, 0x4000, CRC(2cc43026) SHA1(366f57292c6e44571368c29e3258203779847356) )
	ROM_LOAD( "v17_001b.ic24", 0x4000, 0x4000, CRC(a56b16f1) SHA1(c68f26b35453153f7defcf1cf2b7ad7fe36cc9e7) )

	ROM_REGION(0x1000, "eeprom", 0)
	ROM_LOAD( "eeprom.bin",    0x0000, 0x1000, CRC(7b24878a) SHA1(20086fb792a24339b65abe627aefbcf48e2abcf4) ) // don't know where this fits in

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "v20_cg.ic17",   0x0000, 0x1000, CRC(76ef7ca9) SHA1(6e7799ca0a41350fbc369bbbd4ab581150f37b10) )

	ROM_REGION(0x1000, "keyboard", 0)
	ROM_LOAD( "v00_kbd.bin",   0x0000, 0x1000, CRC(f9d24190) SHA1(c4e9ef8188afb18de373f2a537ca9b7a315bfb76) )
ROM_END

COMP( 1984, cit220p, 0, 0, cit220p, cit220p, cit220_state, empty_init, "C. Itoh Electronics", "CIT-220+ Video Terminal", MACHINE_IS_SKELETON )

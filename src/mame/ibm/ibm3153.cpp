// license:BSD-3-Clause
// copyright-holders: Robbbert
/***************************************************************************

IBM 3153 Terminal.

2016-05-04 Skeleton driver.

A green-screen terminal with a beeper.
Chip complement:
U1    K6T0808C10-DB70 (32k static ram)
U2    D-80C32-16 (cpu)
U3    DM74LS373N
U5    LM339N
U6    DM74LS125AN
U7    K6T0808C10-DB70 (ram)
U8    K6T0808C10-DB70 (ram)
U9    598-0013040 6491 3.19 (boot rom)
U10   DS1488N
U11   74LS377N
U12   DS1489AN
U13   LSI VICTOR 006-9802760 REV B WDB36003 Y9936 (video processor)
U14   74F00PC
U16   DM74LS125AN
U17   DS1488N
U18   DS1489AN
U25   SN74F04N
U100  74F07N
Crystals:
Y1    16.000 MHz
Y2    65.089 MHz
Y3    44.976 MHz


ToDo:
- Everything!

****************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "emupal.h"
#include "screen.h"


namespace {

class ibm3153_state : public driver_device
{
public:
	ibm3153_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
		{ }

	void ibm3153(machine_config &config);

private:
	virtual void machine_reset() override ATTR_COLD;
	void ibm3153_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
};


void ibm3153_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x0ffff).rom().region("user1", 0);
}

void ibm3153_state::io_map(address_map &map)
{
	map(0x0000, 0xffff).ram();
	//map.unmap_value_high();
	//map.global_mask(0xff);
}


/* Input ports */
static INPUT_PORTS_START( ibm3153 )
INPUT_PORTS_END

uint32_t ibm3153_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void ibm3153_state::ibm3153_palette(palette_device &palette) const
{
	palette.set_pen_color(0, 0,   0, 0); // Black
	palette.set_pen_color(1, 0, 255, 0); // Full
	palette.set_pen_color(2, 0, 128, 0); // Dimmed
}

void ibm3153_state::machine_reset()
{
}

void ibm3153_state::ibm3153(machine_config &config)
{
	/* basic machine hardware */
	I80C32(config, m_maincpu, XTAL(16'000'000)); // no idea of clock
	m_maincpu->set_addrmap(AS_PROGRAM, &ibm3153_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &ibm3153_state::io_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(ibm3153_state::screen_update));
	screen.set_size(640, 240);
	screen.set_visarea(0, 639, 0, 239);
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(ibm3153_state::ibm3153_palette), 3);
}

/* ROM definition */
ROM_START( ibm3153 )
	ROM_REGION( 0x40000, "user1", 0 )
	ROM_LOAD("598-0013040_6491_3.19.u9", 0x0000, 0x040000, CRC(7092d690) SHA1(a23a5bd5eae90e9b31fa32ef4be1258612eaaa0a) )

	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "char.bin", 0x0000, 0x2000, NO_DUMP ) // probably inside the video processor
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR   NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY  FULLNAME             FLAGS
COMP( 1999?, ibm3153, 0,      0,      ibm3153, ibm3153, ibm3153_state, empty_init, "IBM",   "IBM 3153 Terminal", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)

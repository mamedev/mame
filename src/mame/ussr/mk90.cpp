// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

Elektronika MK-90

2009-05-12 Skeleton driver.


    http://www.pisi.com.pl/piotr433/index.htm#mk90
    http://www.taswegian.com/MOSCOW/mk-90.html

This is a Soviet computer-calculator, very similar in looks to the Sharp.
It has a LCD display. It cost about 1500 roubles, which is the wages for 6
months for an average citizen.

Status: Starts in the weeds.

****************************************************************************/

#include "emu.h"
#include "cpu/t11/t11.h"
#include "emupal.h"
#include "screen.h"


namespace {

class mk90_state : public driver_device
{
public:
	mk90_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void mk90(machine_config &config);

private:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

	void mem_map(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<k1801vm2_device> m_maincpu;
};


void mk90_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).ram(); // RAM
	map(0x4000, 0x7fff).rom(); // Extension ROM
	map(0x8000, 0xffff).rom(); // Main ROM
//  map(0xe800, 0xe801) LCD address
//  map(0xe802, 0xe803) LCD data
//  map(0xe810, 0xe810) serial bus controller data
//  map(0xe812, 0xe813) serial bus controller transfer rate
//  map(0xe814, 0xe814) serial bus controller control/status
//  map(0xe816, 0xe816) serial bus controller command
//  map(0xea00, 0xea7e) RTC
}

/* Input ports */
static INPUT_PORTS_START( mk90 )
INPUT_PORTS_END


void mk90_state::machine_reset()
{
}

void mk90_state::machine_start()
{
}

uint32_t mk90_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void mk90_state::mk90(machine_config &config)
{
	/* basic machine hardware */
	K1801VM2(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_initial_mode(0x8000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mk90_state::mem_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(mk90_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);
}

/* ROM definition */
ROM_START( mk90 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "bas1", "Basic 1")
	ROMX_LOAD( "mk90ro10.bin",  0x8000, 0x8000, CRC(fac18038) SHA1(639f09a1be5f781f897603d0f799f7c6efd1b67f), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "bas2", "Basic 2")
	ROMX_LOAD( "mk90ro20.bin",  0x8000, 0x8000, CRC(d8b3a5f5) SHA1(8f7ab2d97c7466392b6354c0ea7017531c2133ae), ROM_BIOS(1))
	ROMX_LOAD( "mk90ro20t.bin", 0x4000, 0x4000, CRC(0f4b9434) SHA1(c74bbde6d201913c9e67ef8e2abe14b784187f8d), ROM_BIOS(1))  // Expansion ROM
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY        FULLNAME  FLAGS */
COMP( 1988, mk90, 0,      0,      mk90,    mk90,  mk90_state, empty_init, "Elektronika", "MK-90",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )

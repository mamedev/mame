// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Elektronika MK-85

        12/05/2009 Skeleton driver.

    http://www.taswegian.com/MOSCOW/mk-85.html

This is a Soviet computer-calculator, very similar in looks to the Sharp.
It has a LCD display.

Models:
    MK-85:  2K of RAM
    MK-85M: 6K of RAM
    MK-85C: Military cryptographic device. Typing text into it produces
            a string of numbers.

****************************************************************************/

#include "emu.h"
#include "cpu/t11/t11.h"
#include "emupal.h"
#include "screen.h"


namespace {

class mk85_state : public driver_device
{
public:
	mk85_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void mk85(machine_config &config);

private:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

	void mem_map(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<k1801vm2_device> m_maincpu;
};


void mk85_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).rom().mirror(0x4000);
	map(0x8000, 0xffff).ram();
}

/* Input ports */
static INPUT_PORTS_START( mk85 )
INPUT_PORTS_END


void mk85_state::machine_reset()
{
}

void mk85_state::machine_start()
{
}

uint32_t mk85_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void mk85_state::mk85(machine_config &config)
{
	/* basic machine hardware */
	K1801VM2(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_initial_mode(0);
	m_maincpu->set_addrmap(AS_PROGRAM, &mk85_state::mem_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(mk85_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);
}

/* ROM definition */
ROM_START( mk85 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "mk85.rom", 0x0000, 0x4000, CRC(398e4fd1) SHA1(5e2f877d0f451b46840f01190004552bad5248c8))
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY        FULLNAME  FLAGS */
COMP( 1986, mk85, 0,      0,      mk85,    mk85,  mk85_state, empty_init, "Elektronika", "MK-85",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )

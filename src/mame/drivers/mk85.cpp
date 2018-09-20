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
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_mk85(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	void mk85_mem(address_map &map);
};


void mk85_state::mk85_mem(address_map &map)
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

void mk85_state::video_start()
{
}

uint32_t mk85_state::screen_update_mk85(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

MACHINE_CONFIG_START(mk85_state::mk85)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", K1801VM2, XTAL(4'000'000))
	MCFG_T11_INITIAL_MODE(0)
	MCFG_DEVICE_PROGRAM_MAP(mk85_mem)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DRIVER(mk85_state, screen_update_mk85)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_MONOCHROME("palette")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( mk85 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "mk85.rom", 0x0000, 0x4000, CRC(398e4fd1) SHA1(5e2f877d0f451b46840f01190004552bad5248c8))
ROM_END

/* Driver */

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY        FULLNAME  FLAGS */
COMP( 1986, mk85, 0,      0,      mk85,    mk85,  mk85_state, empty_init, "Elektronika", "MK-85",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND)

// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        UKNC

        12/05/2009 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/t11/t11.h"
#include "emupal.h"
#include "screen.h"


class uknc_state : public driver_device
{
public:
	uknc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu") { }

	void uknc(machine_config &config);

private:
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_uknc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;

	void uknc_mem(address_map &map);
	void uknc_sub_mem(address_map &map);
};


void uknc_state::uknc_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).ram();  // RAM
	map(0x8000, 0xffff).rom();  // ROM
}

void uknc_state::uknc_sub_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).ram();  // RAM
	map(0x8000, 0xffff).rom();  // ROM
}

/* Input ports */
static INPUT_PORTS_START( uknc )
INPUT_PORTS_END


void uknc_state::machine_reset()
{
}

void uknc_state::video_start()
{
}

uint32_t uknc_state::screen_update_uknc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

MACHINE_CONFIG_START(uknc_state::uknc)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", K1801VM2, 8000000)
	MCFG_T11_INITIAL_MODE(0x8000)
	MCFG_DEVICE_PROGRAM_MAP(uknc_mem)

	MCFG_DEVICE_ADD("subcpu",  K1801VM2, 6000000)
	MCFG_T11_INITIAL_MODE(0x8000)
	MCFG_DEVICE_PROGRAM_MAP(uknc_sub_mem)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DRIVER(uknc_state, screen_update_uknc)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_MONOCHROME("palette")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( uknc )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "uknc.rom", 0x8000, 0x8000, CRC(a1536994) SHA1(b3c7c678c41ffa9b37f654fbf20fef7d19e6407b))

	ROM_REGION( 0x10000, "subcpu", ROMREGION_ERASEFF )
ROM_END

/* Driver */

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  STATE       INIT        COMPANY      FULLNAME  FLAGS */
COMP( 1987, uknc, 0,      0,      uknc,    uknc,  uknc_state, empty_init, "<unknown>", "UKNC",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND)

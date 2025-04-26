// license:BSD-3-Clause
// copyright-holders:
/*******************************************************************************

Skeleton driver for IBM InfoWindow 3477 terminal.

                      ______________          ________
   ___________________|             |_________|       |_______________________
  |                   |_____________|         |_______|                      |
  |  BATTERY            ______     _____                                     |
  |                    |_____|    |____|                                     |
  |                             ____________    ___   ___   ___   ___        |
  |         _________          |38F5768    |   | <-SN74LS74AN |  |  |        |
  |        CXK58257M-10LL      |TC110G17AF |   |  |  |  <-SN75112N  |        |
  |                            |           |   |  |  |  |  |  |  | <-SN7510BN|
  |    ____   ____             |           |   |__|  |__|  |__|  |__|        |
  |   |___|  |___|             |___________|              _________     ____ |
  |____                                                CXK58257M-10LL  |___| |
      |        ______   ______   _________          ____________             |
      |  ___  |     |  |     |  |AMD     |         |38F5769    |  Xtal       |
      | |  |  |EPROM|  |EPROM|  |N8088-2 |         |TC110G26AF |  59.8185 MHz|
      | |  |  |     |  |     |  |        |         |           |             |
      | |  |  |     |  |     |  |________|         |           |             |
   ___| |__|  |     |  |     |                     |___________|             |
 _|__         |_____|  |_____|                                   ____        |
|   |                                                      10124N-> |        |
|___|         ||||||||||||||||                                   |  |        |
  |                                                              |__|        |
  |                            _________________    ___               ___    |
  |___________________________|                |_|_|  |               |  |___|
                                                      |_|_|_|_|_|_|_|_|

ToDo:
- Everything!

********************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "emupal.h"
#include "screen.h"


namespace {

class ibm3477_state : public driver_device
{
public:
	ibm3477_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void ibm3477(machine_config &config);

private:
	virtual void machine_reset() override ATTR_COLD;
	void palette_init(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<cpu_device> m_maincpu;
};


// Input ports
static INPUT_PORTS_START( ibm3477 )
INPUT_PORTS_END

uint32_t ibm3477_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void ibm3477_state::palette_init(palette_device &palette) const
{
	palette.set_pen_color(0, 0,   0, 0); // Black
	palette.set_pen_color(1, 0, 255, 0); // Full
	palette.set_pen_color(2, 0, 128, 0); // Dimmed
}

void ibm3477_state::machine_reset()
{
}

void ibm3477_state::ibm3477(machine_config &config)
{
	// Basic machine hardware
	I8088(config, m_maincpu, 59'818'500 / 10); // Unknown clock

	// Video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // Not accurate
	screen.set_screen_update(FUNC(ibm3477_state::screen_update));
	screen.set_size(640, 240);
	screen.set_visarea(0, 639, 0, 239);
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(ibm3477_state::palette_init), 3);
}

// ROM definition
ROM_START( ibm3477 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD("pn_38f5843_ibm_30-10-89.zm12", 0x00000, 0x20000, CRC(b07efb93) SHA1(84d245d17828bc4e096e0cae2d066f2209f56a8f) )
	ROM_LOAD("pn_38f5844_ibm_20-10-89.zm13", 0x20000, 0x20000, CRC(05f3c0f6) SHA1(f99096a2f3e97fe1eb540db3f3e620ddccf71bbc) )
ROM_END

} // anonymous namespace


//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY  FULLNAME           FLAGS
COMP( 1998, ibm3477, 0,      0,      ibm3477, ibm3477, ibm3477_state, empty_init, "IBM",   "InfoWindow 3477", MACHINE_NO_SOUND | MACHINE_NOT_WORKING) // Spanish ROMs?

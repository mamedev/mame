// license:BSD-3-Clause
// copyright-holders:

/*****************************************************************************

Skeleton driver for "Flash VGA 2" hardware (from TourVisión?) for video-slots.
                               _________
     _________________________|  VGA   |_________________________________
    |           ······        |________|        _______________         |
    |    ____      _____    ______   ______    |K6R4016C10-JC10         |
    |  TDA2030A   SN7407   74HC5740 74HC5740   |______________|         |
 ___|                                                               ··  |
|___         _____    ______                     _________          ··  |
|___       TLC7524C  |LM358|                    |LATTICE |          ··  |
|___                                            |ISPLSI  |          ··  |
|___       _________     Xtal                   |1032EA  |          ··  |
|___     SCC2692AC1A44  3.6864 MHz              |C424AH03|              |
|___      |Philips |                                _____________       |
|___      |        |           _____   _________   |Intel       | Xtal  |
|___      |________|         74HC04D  |LATTICE |   |N80C186XL25 |50 MHz |
|___   __________                     |ISPLSI  |   |            |       |
|___  |ULN2803A |  ______    ______   |2032A   |   |            |       |
|___   __________ 74HC273D  74HC273D  |80LJ44__|   |____________|       |
|___  |ULN2803A |  ______    ______  ______   ______   ______   ______  |
|___              74HC273D  |HC138| 74HC245D  HC573A  74HC245D  HC573A  |
|___               ______     ____     :::::::::::::::::::::::::::::    |
|___              74HC245D   |NE555          ______         ______      |
|___               ______    ______         |FLASH|        |FLASH|      |
    |             74HC245D  ADM691AARW      M29F032D       M29F032D     |
    |                                       |_____|        |_____|      |
    |  __________          ________   ___    __________     __________  |
    | OMRON G5V-2         |_ST232C|  24256B |CY62256LL|    |CY62256LL|  |
    |                                        ______________             |
    |    ·········                   CR2032 |M48T18-150PC1|   Switch    |
    |               _________         BATT  |_____________|    (o)      |
    |______________|  DB9   |___________________________________________|
                   |________|

*****************************************************************************/

#include "emu.h"

#include "cpu/i86/i186.h"
#include "machine/mc68681.h"
#include "machine/timekpr.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class flashvga2_state : public driver_device
{
public:
	flashvga2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	void flashvga2(machine_config &config);

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// devices
	required_device<cpu_device> m_maincpu;
};

static INPUT_PORTS_START(ruletamag)
INPUT_PORTS_END

uint32_t flashvga2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static GFXDECODE_START( gfx_flashvga2 )
GFXDECODE_END

void flashvga2_state::flashvga2(machine_config &config)
{
	I80186(config, m_maincpu, 50_MHz_XTAL/2);

	SCN2681(config, "uart", 3.6864_MHz_XTAL); // Philips SCC2692AC1A44

	M48T02(config, "m48t18", 0); // ST M48T18-150PC1

	// Video hardware (probably wrong values)
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_screen_update(FUNC(flashvga2_state::screen_update));
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_palette("palette");

	PALETTE(config, "palette").set_entries(512);

	// Sound hardware
	SPEAKER(config, "mono").front_center();
}

/* Ruleta Mágica from Codere. Same hardware as Ruleta Mágica Mini, the only difference is the cabinet.
   VGA screen, plus a big 7 digits 7-segments (plus dot) display.
   The manual can be downloaded from: https://www.recreativas.org/manuales/tragaperras
   Video of the actual machine booting: https://youtu.be/xUARqw1_N_A  */
ROM_START( ruletamag )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD( "m29f032d.u100", 0x000000, 0x400000, CRC(04bf20c2) SHA1(fc4be2c22dc266d6a460aeca257b449be5ab630f) )
	ROM_LOAD( "m29f032d.u101", 0x400000, 0x400000, CRC(2bd85284) SHA1(36f4b918d1d9b57bf382fa940180b3a1aac9780f) )

	ROM_REGION( 0x8000, "seeprom", 0 )
	ROM_LOAD( "m24256bf.u31",  0x000000, 0x008000, CRC(af9adcae) SHA1(ac6274edc4240d5cf397455868009263264ffc6e) )

	/* With an unintialized NVRAM/timekeeper, the machine won't work and Will output just a "ERROR EN RELOJ" message.
	       With the included dump (corrupted), it will output the message "ERROR EN MODULO", but still won't boot.
	       Maybe there's a way to initialize the NVRAM, but there's nothing about it on the manual. */
	ROM_REGION( 0x2000, "nvram", 0 )
	ROM_LOAD( "m48t18.u38",    0x000000, 0x002000, BAD_DUMP CRC(025fb8c2) SHA1(61c90ecad8565cfd20674034a5917b0225edbfe5) ) // Corrupted
ROM_END


} // Anonymous namespace

//    YEAR  NAME       PARENT  MACHINE    INPUT      CLASS            INIT        ROT   COMPANY   FULLNAME           FLAGS
GAME( 2005, ruletamag, 0,      flashvga2, ruletamag, flashvga2_state, empty_init, ROT0, "Codere", u8"Ruleta Mágica", MACHINE_IS_SKELETON )

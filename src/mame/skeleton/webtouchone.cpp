// license:BSD-3-Clause
// copyright-holders:

/*
  Skeleton driver for Alcatel Web Touch One.
  More information and technical manuals: https://www.minitel-alcatel.fr/gamme_webphone.html
  The phone asks for an adminstrative password for performing a factory reset.

  Hardware info for model 2840:
   -CPU: Motorola PowerPC 823
   -Operating system: JavaOS over ChorusOS (Sun Microsystems)
   -Screen: Hitachi SX19V001-ZZA (7,6'' touch, color, 640x480)
   -Keyboard: N860-1428-T021
   -Other: Smartcard reader, 33.6 kbps modem (Conexant RVC3366ACFW), 8MB DRAM, etc.

  CPU PCB
    ____________________________________________________________________
   |        Xtal                                                       |
   |     25.416 MHz                                                    |
   |    _____________                         _______                  |
   |   | CONEXANT   |                        |MC3403|   ___            |
   |   | RVC3366ACFW|                                  |  |            |
   |   | R6749-24   |   ________  ________   74HC4053->|  |            |
   |   |            |  |74HC4053||74HC4053|            |__|            |
   |   |____________|                    ___                   _______ |
   |  _______                    34119->|  |                  |MC3403| |
   | 74AHCT574	                        |__|                           |
   |  ____________   ____________                                      |
   | |KM416S4030CT| |KM416S4030CT|                                     |
   | |____________| |____________|                                     |
   |       ____________                                     __________ |
   |      |KM29U64000T|             _____________          |74HCT541 | |
   |      |___________|            | Motorola   |          |_________| |
   |                               | XPC823ZT66A|           __________ |
   |                    Xtal       |            |          |74HCT541 | |
   |                 32.768 MHz    |            |          |_________| |
   |                               |____________|                      |
   |                                                                   | 
   |                                          _______                  |
   | 3BN62121AAAF KAZZA 01                   |ST 324|                  |
   |___________________________________________________________________|


  Keyboard PCB
   _____________________________________________________________________________
  |                                   ___________                              |
  |             CONN                 |TDA8002CT/8|                             |
  |                                  |___________|                             |
  |__                         _________                 CONN                 __|
    |                        | NEC    |  Xtal                                |
   _|   CONN                D78F0034AGC  7.15909 MHz                         |_
  |                          |________|                                        |
  |____________________________________________________________________________|


  Main PCB (ALCATEL CPU-I)
   __________________________________________________________
  | :::::::::::::::    CONN             CONN                |
  |   _______________                ____                 C |
  |  | M29W800AT    |                LC08A                O |
  |  |______________|                                     N |
  |   _______________                                     N |
  |  | M29W800AT    |                                       |
  |  |______________|  ____                               C |
  |                    LC32A                              O |
  | _______________   _______________                     N |
  || KM416S4030CT |  | KM416S4030CT |                     N |
  ||______________|  |______________|                       |
  | _______           _______   _____                       |
  ||LVC4245          |_HB574|  LVC14A   ________     ___    |
  |                                     74HC4035   ST072C C |
  | ___________     _____               ________          O |
  ||STM27C1001|    |    |<-SDT 71256    74HC4035          N |
  ||          |    |    |                                 N |
  ||          |    |    |           ____________     ___    |
  ||__________|    |____|          | MC34118DW |   MC3403D  |
  |_________________________________________________________|

*/

#include "emu.h"
#include "cpu/powerpc/ppc.h"
#include "screen.h"
#include "speaker.h"

namespace {

class webtouchone_state : public driver_device
{
public:
	webtouchone_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen")
	{ }

	void webtouchone(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	uint32_t screen_update_webtouchone(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

uint32_t webtouchone_state::screen_update_webtouchone(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void webtouchone_state::machine_start()
{
}

void webtouchone_state::machine_reset()
{
}

static INPUT_PORTS_START( webtouchone )
INPUT_PORTS_END

void webtouchone_state::webtouchone(machine_config &config)
{
	MPC8240(config, m_maincpu, XTAL(32'768'000)); // Actually a Motorola XPC823ZT66A

	SCREEN(config, m_screen, SCREEN_TYPE_LCD); // Hitachi SX19V001-ZZA (7,6'' touch, color, 640x480)
	m_screen->set_refresh_hz(60); // Guess
	m_screen->set_size(640, 480);
	m_screen->set_visarea(0, 640-1, 0, 480-1);
	m_screen->set_screen_update(FUNC(webtouchone_state::screen_update_webtouchone));

	SPEAKER(config, "mono").front_center();
}

// Spanish ROM for Terra (now Telefónica)
ROM_START( wto2840sp )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "3bn64108aaaa-02hy-m29w800at.bin", 0x000000, 0x100000, CRC(ebc91205) SHA1(a0609936cf5e99de18f7e94efe7ecfdeabf40d34) )
	ROM_LOAD( "3bn64108aaba-04fw-m29w800at.bin", 0x100000, 0x100000, CRC(ca1967c1) SHA1(8a49a255029c5098335d58909718ef558452294c) )

	ROM_REGION( 0x020000, "modem", 0 )
	ROM_LOAD( "3bn64078_aabe_stm27c1001.bin",    0x000000, 0x020000, CRC(25c0cb47) SHA1(67b337e05204a68b54f6e33d64ac876012ee9eb6) )

	ROM_REGION( 0x840000, "user", 0 )
	ROM_LOAD( "km29u64000t.bin",                 0x000000, 0x840000, BAD_DUMP CRC(869a07d1) SHA1(a4199c8babd723584c17a913bf4c43c02b90ffad) ) // Contains user data (addessses, accounts, etc.; needs a factory reset)

	ROM_REGION( 0x008000, "keyboard", 0 )
	ROM_LOAD( "upd78f0034agc.bin",               0x000000, 0x008000, NO_DUMP )
ROM_END

} // anonymous namespace


//    YEAR  NAME       PARENT  COMPAT  MACHINE      INPUT        CLASS              INIT        COMPANY    FULLNAME                                      FLAGS
COMP( 1999, wto2840sp, 0,      0,      webtouchone, webtouchone, webtouchone_state, empty_init, "Alcatel", "Web Touch One (model 2840, Terra, Spanish)", MACHINE_IS_SKELETON )

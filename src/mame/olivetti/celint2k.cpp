// license: BSD-3-Clause
// copyright-holders:

/********************************************************************************************

Skeleton driver for Olivetti Celint 2000 phone with Videotext terminal.
In Spain, Banco Santander distributed it as the Superfono Santander (with a custom ROM) as 
part of a "bank at home" pilot program.

Main PCB:
    ____________________________________________________________________________________
   |                          |           _________________                            |
   |                          |          |  GoldStar      |  ___________  ___________  |
   |   POWER SUPPLY           |   BATT   |  GM76C256L-85  | |SN74HC245N| |_SN74HC74_|  |
   |                          |          |________________|               ___________  |
   |                          |           Xtal                           |M74HC00B1_|  |
   |__________________________|        4.4334 MHz         ____________                 |
   |                                                      |           |                |
   |                                     Xtal 32.768 kHz  | OKI M6255 |                |
   |                 ___________          ___________     |___________|                |
   |                |SN74HC08N_|         |_PCF8573P_|   _________________________      |
   |                 ___________                       | GoldStar               |      |
   |                |M74HC374B1|                       | GM76C8128ALL-85        |      |
   |                                                   |________________________|      |
   |                 _________________________          _________________________      |
   |                | GolsStar               |         |                        |      |
   | SMARTCARD      | GM76C88L-15            |         | EPROM                  |      |
   |  READER        |________________________|         |________________________|      |
   |                 ______________________________                                    |
   |                | Zilog                       |    Xtal           ___________      |
   |                | Z84C0006PEC Z80 CPU         |   11.0573 MHz    |SN74LS145N|      |
   |                |_____________________________|         ________________           |
   |                                     ___________       |               |           |
   |                                    |GAL16V8-20|       | Zilog         |           |
   |                                                       | Z84C9008VSC   |           |
   |                                                       | Z80 KIO       |           |
   |                     _____________________             |_______________|           |
   |                    |                    |                                         |
   |                    | 73K322L-IP         |  _____        ___________      :        |
   |                    |____________________| |TL7705ACP   |HCF4094BE_|      ·        |
   |                                            ___________                            |
   |      _______                              |HCF4066BE_|                            |
   |     |7805CT|             ___________       _____                                  |
 __|_                        |_TL084CN__|      MC34119P                                |
|    |                                                         _____________________   |
|DB25|                                                        |                    |   |
|RS232                                                        | MC34118P           |   |
|____|    ___________                                         |____________________|   |
   |     |MC145406P_|                                                                  |
 __|_                                  ___________                                     |
|    |                                |_GD4053B__|                                     |
|P/T |                                     _______                         ___________ |
|____|                                    |H11D1_|                        |__M3541B__| |
   |                                                                                   |
 __|_                                                  ___________                     |
|    |                                                |_KA8501A__|                     |
|LINE|                                                                                 |
|____|                                                                                 |
  _|_                                                                    SPEAKER       |
 | CD/MF switch                                                                        |
 |___|                                                           _______               |
   |                                           _____            |LS1240A               |
   |__________________________________________|    |___________________________________|
                                              |____|

Video screen is driven by 9 Hitachi HD61105A chips (separate PCB).

********************************************************************************************/

#include "emu.h"


#include "cpu/z80/z80.h"

#include "machine/pcf8573.h"
#include "machine/z80ctc.h"

#include "video/msm6255.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {


class celint2k_state : public driver_device
{
public:
	celint2k_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdc(*this, "msm6255")
		, m_rtc(*this, "rtc")
	{ }

	void celint2k(machine_config &config) ATTR_COLD;

private:

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void celint2k_palette(palette_device &palette) const;

	required_device<cpu_device> m_maincpu;
	required_device<msm6255_device> m_lcdc;
	required_device<pcf8573_device> m_rtc;
};

void celint2k_state::celint2k_palette(palette_device &palette) const
{
}

void celint2k_state::machine_start()
{
}

void celint2k_state::machine_reset()
{
}

INPUT_PORTS_START( celint2k )
INPUT_PORTS_END

void celint2k_state::celint2k(machine_config &config)
{
	Z80(config, m_maincpu, 11'057'300 / 2); // Z84C0006PEC, verified divisor

	Z80CTC(config, "ctc", 11'057'300 / 2); // Z84C9008VSC

	PCF8573(config, m_rtc, 32.768_kHz_XTAL);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD)); // TECDIS C425901 backlight 320x240 gLCD
	screen.set_refresh_hz(60); // Guess
	screen.set_screen_update("msm6255", FUNC(msm6255_device::screen_update));
	screen.set_size(320, 240);
	screen.set_visarea(0, 320-1, 0, 240-1);
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(celint2k_state::celint2k_palette), 2);

	MSM6255(config, m_lcdc, 4'433'400 );
	m_lcdc->set_screen("screen");

	SPEAKER(config, "mono").front_center();
}

ROM_START( celint2kss )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "sa21-b_eae5_27c020.ic19", 0x00000, 0x40000, CRC(0f5fd110) SHA1(9d1abc90db5eb5efbcde1da4b8a7ef6438723664) )

	ROM_REGION( 0x0117, "pld", 0 )
	ROM_LOAD( "gal16v8-20lnc.ic15", 0x0000, 0x0117, CRC(45724282) SHA1(09c1029af68ef6f8bd1d17d19dbce7a691f80171) )
ROM_END

} // anonymous namespace

COMP( 1995, celint2kss, 0, 0, celint2k, celint2k, celint2k_state, empty_init, "Olivetti", "Celint 2000 (Superfono Santander edition)", MACHINE_NOT_WORKING ) // Labeled as model "MULTIMEDIA - T"

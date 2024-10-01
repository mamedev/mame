// license:BSD-3-Clause
// copyright-holders:
/*
  Skeleton driver for Bally/Sente "Cashline" slot machine.
  Six reels, three up, three down.
  Large 6 digits 7 segments (plus dot) display up, a smaller 12 digits 7 segments display down,
  plus another two digits 7 sements display as credits counter.

  There are no dip switches for hoppers, instead you can insert "KEY" mini PCBs on a small 
  socket on the drivers PCB that shorts contacts TS4, TS5, TS6, TS7, TZ4, TZ5, TZ6 and TZ7.
  On the dumped machine, there was a KEY inserted named "KEY 3" that configures:
  -Hopper 1 = 0.10 €
  -Hopper 2 = 1.00 €
  -Double/Nothing = NO
  -Coin acceptor = 0.10 = yes, 0.20 = yes, 0.50 = yes, 1.00 = yes, 2.00 = yes

  
  Drivers PCB                 _________
                             | KEY    |
                             |Mini PCB|
   ______|||||||___||||______|        |___|||||||________________|||||||_______________
  |      |||||||   ||||      |        |   |||||||                |||||||              |
 ---                         |________|                                               |
 ---                    ______                                                        |
 ---                   L7805CV                                                        |
  |    __________  __________                                                         |
  |   |M74HC245B1 |M74HC165B1                                                         |
 ---               __________                                                         |
 ---              |74HC4051N|                                                         |
 ---   ______   __________                                                            |
  |  TC426CPA  |M74HC123B1                                                            |
 ---                       __________                                                 |
 ---                      |ULN2803A_|      __________  __________  __________         |
 ---  __________  __________              M74HC4094B1 M74HC4094B1 |ULN2803A_|         |
 --- M74HC4094B1 |M74HC107B1                                                          |
  |___________________________________________________________________________________|


  Reels control PCB
   _______________________________________________________________________________________
  |                                                                                      |
  |   ··                               ··                                       ··       |
 ---  ··                               ··                                       ··       |
 ---  ··     ____________              ··      ____________  ____________       ··       |
 ---  ··    |___L6219___|              ··     |___L6219___| |___L6219___|       ··       |
  |   ··                               ··                                       ··       |
 ---  ··                               ··                                       ··       |
 ---                                             ___________  ___________                |
 ---                                            |AT89C2051_| |M74HC165B1|                |
  |                                               Xtal 11.050 MHz                        |
  |  __________  __________  __________  __________  __________  __________  __________  |
  | |M74HC245B1 |M74HC123B1 |M74HC107B1 |M74HC165B1 M74HC4094B1 |ULN2804A_| |74HC4051N|  |
  | ______                                                       __________              |
  |L780SCV                         5163/010101/BN               M74HC4094B1              |
  |______________________________________________________________________________________|


  Sound PCB
   ______________________________________________________________________________________________________
  |                                     ____   __________                                               |
 ---                ___________        |___|  |M74HC02B1|                                               |
 ---               |__________|        __________________                                              _|_
  |                 ___________  ···  | YM2149F         |  __________  __________  ···     __________  _|_
 ---               |M74HC574B1|  ···  |_________________| |M74HC393B1 |74HC4015N|  ···    |MX7224KN_|   |
 ---   ___________  ___________  ···   __________________    ________  __________  ···   _____         _|_
 ---  |__L4974A__| |__________|  ···  | 68C681CP        |   |SG531P_| |_________|  ···  |____|         _|_
 ---                                  |_________________|   3.6864 MHz __________                      _|_
  |                                                       |M74HC32B1| |CD74AC138E                       |
 ---                                    _____                              _____  _____  _____         _|_
 ---                                   TC428CPA                           |____| |____| |____|         _|_
  |_____|||||__||||||__········__·····_____LED_LED___||||__·····__||||||||_____···________|||||__|||||__|
        |||||  ||||||                                ||||         ||||||||                |||||  |||||


  CPU PCB
   __________________________________________________________________________________
  |  ___________   ______________      ______________                               |
  | |M74HC245B1|  | EPROM 1      |    | KM681000CLP |                    BATT       |
  |  ___________  |______________|    |_____________|                   3 VOLTS     |
  | |M74HC245B1|  ______________       ______________                               |
  |         ...   | EPROM 2      |    | KM681000CLP |        ...      ___________   |
  |         ...   |______________|    |_____________|        ...     |_RTC62421_|   |
  | ______  ...   _____________  ___________  ___________    ...      ___________   |
  ||SG531P  ...  |MC68EC000FN12||M74HC32B1_| |M74HC245B1|    ...     |MAX691CPE_|   |
  | 12 MHz  ...  |             | ___________  ___________    ...        ___________ |
  |         ...  |             ||CD74AC138E| |M74HC573B1|    ...       |M74HC32B1_| |
  |  ___________ |             | ___________  ___________  ___________  ___________ |
  | |M74HC573B1| |_____________||M74HC08B1_| |M74HC10B1_| |M74HC32B1_| |M74HC21B1_| |
  |_________________________________________________________________________________|


  Coin acceptor PCB (Azkoyen L60K)
   ___________________________________________
  |   ____    ______           ···   ____    |
  |  HCF4069 | Xtal|  ___________   | DIPSx4 |
  |          |6 MHz| |MHS       |   |___|    |
  |     ___          S-80C51CCMA-12   ____   |
  |  ADC0804LCN      |          |    |   |   |
  |    |  |          |__________|    |___|   |
  |    |  |   _________________              |
  |___ |__|  | EPROM          |         ____ |
  ||HC00A    |________________|      L7805CV |
  ||__|                       _____          |
  |                         MC14099B         |
  |                                          |
  |                                          |
  |                               :::::      |
  |__________________________________________|
*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/msm6242.h"
#include "machine/mc68681.h"
#include "sound/ay8910.h"
#include "speaker.h"


namespace {

class cashline_state : public driver_device
{
public:
	cashline_state( const machine_config &mconfig, device_type type, const char *tag ) :
		driver_device( mconfig, type, tag ),
		m_maincpu( *this, "maincpu" ),
		m_coincpu( *this, "coincpu" )
	{ }

	void cashline( machine_config &config );

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_device<cpu_device> m_maincpu;
	required_device<i80c51_device> m_coincpu;
};

void cashline_state::machine_start()
{
}

void cashline_state::machine_reset()
{
}

static INPUT_PORTS_START( cashline )
INPUT_PORTS_END

void cashline_state::cashline( machine_config &config )
{
	M68000( config, m_maincpu, 12_MHz_XTAL ); // MC68EC000FN12

	RTC62421(config, "rtc", 32.768_kHz_XTAL);

	// Sound hardware
	SPEAKER( config, "mono" ).front_center();

	YM2149( config, "ym", 3.6864_MHz_XTAL / 12).add_route(ALL_OUTPUTS, "mono", 0.3 );

	XR68C681( config, "duart", 3.6864_MHz_XTAL );

	// Coin acceptor
	I80C51(config, m_coincpu, 6_MHz_XTAL); // MHS S-80C51CCMA-12
}

ROM_START( cashline )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "cash_line_t2_2.10_i_m-164_b-1981.bin",  0x00000, 0x80000, CRC(780e96b2) SHA1(778e1bdb1087e628c158ee0aa9fa119bc6304bc7) )
	ROM_LOAD( "cash_line_t2_2.10_ii_m-164_b-1981.bin", 0x80000, 0x80000, CRC(61665c58) SHA1(62e813dc8967233bfd4ff64236081cc57fc502f1) )

	ROM_REGION( 0x000800, "reels", 0 )
	ROM_LOAD( "m2_at89c2051_reels.u12",                0x00000, 0x00800, NO_DUMP ) // Protected

	ROM_REGION( 0x010000, "coincpu", 0 )
	ROM_LOAD( "497a_azkoyen_l60k_27c512.bin",          0x00000, 0x10000, CRC(6ef809c7) SHA1(3ba632bc1f33e1f22a50a70e56cac7cfdae391c2) )
ROM_END


} // anonymous namespace


//    YEAR  NAME      PARENT  MACHINE   INPUT     CLASS           INIT        ROTATION  COMPANY        FULLNAME    FLAGS
GAME( 1981, cashline, 0,      cashline, cashline, cashline_state, empty_init, ROT0,     "Bally/Sente", "Cashline", MACHINE_IS_SKELETON_MECHANICAL )

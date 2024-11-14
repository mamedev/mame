// license:BSD-3-Clause
// copyright-holders:AJR
/*************************************************************************

    Skeleton driver for Compumatic dart board with LED display.

**************************************************************************
_______________________________
|__  ________________________  |
||C| |_CN____________________| |
||N|    __   __   __   __   __ |
|| |    |IC  |IC  |IC  |IC  |CN|
||_|    |19  |20  |18  |21  |_||
| __    |_|  |_|  |_|  |_|  __ |
| |CN   __   ______________ |_| CN
| |_|   |IC  |IC17 REF0032 || ||
| :..   |25  |___9817K0552_||_||
| __    |_| ______ ________ |_||
| |CN   __  |IC23_||IC15   || ||
| | |   |IC ______ |_______||_||
| | |   |24 |IC22_|         |_||
| | |   |_| ______   BATT   __ |
| | |   __  |IC12_|         | | CN
| | |   IC8 ______________  | ||
| |_|   __  |IC1  REF34VA | | ||
| __    IC7 |______9818h-_|XT1||
| |CN   ______  __________  | ||
| | |   |IC5__||IC4_______| | ||
| | |   ______ ___________  |_||
| | |   |IC6__||IC3       | __ |
| | |          |__________| |CN|
| | |          _______         |
| | |          |IC2___|        |
| |_|          _______         |
| __           |IC10__|        |
| |CN          _______         |
| | |          |IC9___|        |
| | |      __  _______       : |
| |_|     IC14 |IC13__|        |
|______________________________|

IC19, IC20, IC18, IC21 = Toshiba TD62703AP
IC25, IC24 = ST ULN2803A
IC17 = REF0032 9817K0552 (40-pin DIP: display controller?)
IC23, IC22, IC12 = Hitachi HD74HC273P
IC15 = Natsemi CD4514BCN
IC8 = Microchip 24LC16B
IC7 = TI TL7705ACP
IC1 = Philips REF34VA 9818h- (40-pin DIP, confirmed to be a 80C51)
XT1 = 20.000 MHz
IC5 = Hitachi HD74HC08P
IC6 = Hitachi HD74HC138P
IC4 = LGS GM76C88ALK-15
IC3 = Winbond W29EE011-15
IC2 = Hitachi HD74HC373P
IC2 = Hitachi HD74HC373P
IC10 = Hitachi HD74HC244P

**************************************************************************

Known machines using this hardware:
_____________________________________________________________________________________________________________________________________________________
|Dumped | Name                | Manufacturer      | Notes                                                               | Machine type               |
|-------|---------------------|-------------------|---------------------------------------------------------------------|----------------------------|
|  NO   | King Dart 2         | CIC Play          | Slightly different PCB layout. Regular MCU instead of REF34VA       | Darts                      |
|  NO   | Sagitario           | CIC Play          | CPU silkscreened "REF 0034 9115S", without manufacturer logos       | Darts                      |
|  YES  | Far West            | Compumatic        | Standard Microdar SPD with Philips REF34VA                          | Electromechanical shooting |
|  YES  | Minidart            | Compumatic        | Compumatic Microdard-V6 PCB. Atmel AT89S51 instead of the REF34 MCU | Darts                      |
|  NO   | Party Darts         | Compumatic        | More info: http://www.recreativas.org/party-darts-4906-compumatic   | Darts                      |
|  NO   | Compudar            | Compumatic        | Standard Microdar with Philips REF34VA                              | Darts                      |
|  YES  | Diana Bifuca        | Compumatic/Bifuca | Standard Microdar SPD with Philips REF34VA. "Bifuca" string on ROM  | Darts                      |
|  YES  | Diana Olakoa        | Compumatic/Olaoka | Compumatic Microdard-V5 PCB (REF0034 + REF0032 + REF0096)           | Darts                      |
|  YES  | Covidarts           | Covielsa          | Not from Compumatic, but similar hardware. 80C31 (ROMless MCU)      | Darts                      |
|  NO   | Top Darts           | unknown           | Large PCB that combines a Microdar PCB with the LED control PCB     | Darts                      |
|  NO   | Western Darts       | unknown           | Standard Microdar with Philips REF34VA                              | Darts                      |
|  NO   | Diamant             | unknown           | Newer PCB with Philips REF34VA and additional Compumatic custom ICs | Darts                      |
|  NO   | Tiger Dart          | unknown           | Standard Microdar SPD with Philips REF34VA                          | Darts                      |
|  NO   | Champion            | unknown           | ProSPDP PCB. https://www.recreativas.org/champion-6137-compumatic   | Darts                      |
|  NO   | Paradise Dart Sport | unknown           | Standard Microdar SPD with RE34VA (without Philips logo)            | Darts                      |
|_______|_____________________|___________________|_____________________________________________________________________|____________________________|

*/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/i2cmem.h"
#include "machine/nvram.h"


namespace {

class microdar_state : public driver_device
{
public:
	microdar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_eeprom(*this, "eeprom")
	{
	}

	void microdar(machine_config &config);
	void prospdp(machine_config &config);
	void microdv5(machine_config &config);
	void covidarts(machine_config &config);

private:
	void prog_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;

	required_device<mcs51_cpu_device> m_maincpu;
	required_device<i2cmem_device> m_eeprom;
};

void microdar_state::prog_map(address_map &map)
{
	map(0x1000, 0xffff).rom().region("program", 0x1000);
}

void microdar_state::ext_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram");
}

static INPUT_PORTS_START(microdar)
INPUT_PORTS_END

void microdar_state::microdar(machine_config &config)
{
	I80C51(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &microdar_state::prog_map);
	m_maincpu->set_addrmap(AS_IO, &microdar_state::ext_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // GM76C88ALK-15 + battery
	I2C_24C16(config, m_eeprom);

	// Code also references some sort of serial RTC?
}

void microdar_state::prospdp(machine_config &config)
{
	microdar(config);
	m_maincpu->set_clock(24_MHz_XTAL);
}

void microdar_state::microdv5(machine_config &config)
{
	microdar(config);
	m_maincpu->set_clock(16_MHz_XTAL);
}

void microdar_state::covidarts(machine_config &config)
{
	I80C31(config, m_maincpu, 16'000'000); // Philips PCB80C31BH-3-16P (Internal MCU osc.)

	//NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // HM6264ALP-12 + battery
	I2C_24C04(config, m_eeprom); // Microchip 24LC04B
}

#define PHILIPS_REF34VA \
	ROM_REGION(0x1000, "maincpu", ROMREGION_ERASE00) \
	ROM_LOAD("ref34va.ic1", 0x0000, 0x1000, NO_DUMP) \
	ROM_FILL(0x0000, 1, 0x02) /* temporary LJMP to external init code */ \
	ROM_FILL(0x0001, 1, 0x10) \
	ROM_FILL(0x0002, 1, 0x1b) \
	ROM_FILL(0x000b, 1, 0x02) /* temporary LJMP to interrupt handler */ \
	ROM_FILL(0x000c, 1, 0x10) \
	ROM_FILL(0x000d, 1, 0x15) \
	ROM_FILL(0x001b, 1, 0x02) /* temporary LJMP to interrupt handler */ \
	ROM_FILL(0x001c, 1, 0x10) \
	ROM_FILL(0x001d, 1, 0x18) \
	ROM_FILL(0x0058, 1, 0x32) /* RETI stubs */ \
	ROM_FILL(0x00af, 1, 0x32) \
	ROM_FILL(0x00c9, 1, 0x02) /* temporary LJMP to end of interrupt handler */ \
	ROM_FILL(0x00ca, 1, 0x12) \
	ROM_FILL(0x00cb, 1, 0xd1) \
	ROM_FILL(0x0135, 1, 0x22) /* RET stubs */ \
	ROM_FILL(0x0163, 1, 0x22) \
	ROM_FILL(0x0185, 1, 0x22) \
	ROM_FILL(0x01cf, 1, 0x22) \
	ROM_FILL(0x02de, 1, 0x22) \
	ROM_FILL(0x02fa, 1, 0x22) \
	ROM_FILL(0x0308, 1, 0x22) \
	ROM_FILL(0x0313, 1, 0x22) \
	ROM_FILL(0x037d, 1, 0x22) \
	ROM_FILL(0x03be, 1, 0x22) \
	ROM_FILL(0x0496, 1, 0x22) \
	ROM_FILL(0x04ce, 1, 0x22) \
	ROM_FILL(0x0514, 1, 0x22) \
	ROM_FILL(0x0520, 1, 0x22) \
	ROM_FILL(0x0550, 1, 0x22) \
	ROM_FILL(0x0574, 1, 0x22) \
	ROM_FILL(0x05b4, 1, 0x22) \
	ROM_FILL(0x05bb, 1, 0x22) \
	ROM_FILL(0x05ca, 1, 0x22) \
	ROM_FILL(0x05d8, 1, 0x22) \
	ROM_FILL(0x0605, 1, 0x22) \
	ROM_FILL(0x0638, 1, 0x22) \
	ROM_FILL(0x068a, 1, 0x22) \
	ROM_FILL(0x06a2, 1, 0x22) \
	ROM_FILL(0x06bd, 1, 0x22) \
	ROM_FILL(0x06db, 1, 0x22) \
	ROM_FILL(0x0708, 1, 0x22) \
	ROM_FILL(0x0715, 1, 0x22) \
	ROM_FILL(0x072d, 1, 0x22) \
	ROM_FILL(0x0744, 1, 0x22) \
	ROM_FILL(0x0751, 1, 0x22) \
	ROM_FILL(0x0767, 1, 0x22) \
	ROM_FILL(0x0772, 1, 0x22) \
	ROM_FILL(0x077d, 1, 0x22) \
	ROM_FILL(0x07a4, 1, 0x22) \
	ROM_FILL(0x07c2, 1, 0x22) \
	ROM_FILL(0x0802, 1, 0x22) \
	ROM_FILL(0x0836, 1, 0x22) \
	ROM_FILL(0x087d, 1, 0x22) \
	ROM_FILL(0x0893, 1, 0x22) \
	ROM_FILL(0x0930, 1, 0x22) \
	ROM_FILL(0x094a, 1, 0x22) \
	ROM_FILL(0x095a, 1, 0x22) \
	ROM_FILL(0x096a, 1, 0x22) \
	ROM_FILL(0x097b, 1, 0x22) \
	ROM_FILL(0x098f, 1, 0x22) \
	ROM_FILL(0x09a3, 1, 0x22) \
	ROM_FILL(0x09c0, 1, 0x22) \
	ROM_FILL(0x0a21, 1, 0x22) \
	ROM_FILL(0x0a54, 1, 0x22) \
	ROM_FILL(0x0a63, 1, 0x22) \
	ROM_FILL(0x0a73, 1, 0x22) \
	ROM_FILL(0x0a90, 1, 0x22) \
	ROM_FILL(0x0ae8, 1, 0x22) \
	ROM_FILL(0x0abf, 1, 0x22) \
	ROM_FILL(0x0ac7, 1, 0x22) \
	ROM_FILL(0x0b11, 1, 0x22) \
	ROM_FILL(0x0b7f, 1, 0x22) \
	ROM_FILL(0x0bf1, 1, 0x22) \
	ROM_FILL(0x0bf6, 1, 0x22) \
	ROM_FILL(0x0c4a, 1, 0x22) \
	ROM_FILL(0x0c59, 1, 0x22) \
	ROM_FILL(0x0c64, 1, 0x22) \
	ROM_FILL(0x0c99, 1, 0x22) \
	ROM_FILL(0x0ca8, 1, 0x22) \
	ROM_FILL(0x0cbd, 1, 0x22) \
	ROM_FILL(0x0dac, 1, 0x22) \
	ROM_FILL(0x0dca, 1, 0x22) \
	ROM_FILL(0x0e6a, 1, 0x22)

ROM_START(dibifuca)
	PHILIPS_REF34VA

	ROM_REGION(0x20000, "program", 0)
	ROM_LOAD("compumatic_925.ic3", 0x00000, 0x20000, CRC(0d49be4b) SHA1(685a35b6040ca2ca1a92068e890d4a5abbcbab0a)) // TMS27C010A

	ROM_REGION(0x800, "eeprom", 0)
	ROM_LOAD("24lc16b.ic8", 0x000, 0x800, NO_DUMP)
ROM_END

ROM_START(dibif743)
	PHILIPS_REF34VA

	ROM_REGION(0x20000, "program", 0)
	ROM_LOAD("compumatic_743.ic3", 0x00000, 0x20000, CRC(ece15715) SHA1(f6cbc420e5d77ff753ceb1e9d3e6119e9a3f83d9)) // W29C011A

	ROM_REGION(0x800, "eeprom", 0)
	ROM_LOAD("24lc16b.ic8", 0x000, 0x800, NO_DUMP)
ROM_END

ROM_START(dibif727)
	// Philips REF34VA K7V5534 9818h
	PHILIPS_REF34VA

	ROM_REGION(0x20000, "program", 0)
	ROM_LOAD("compumatic_727.ic3", 0x00000, 0x20000, CRC(ccf973b6) SHA1(ab67e466849b3bbd8f24be041c979c3f833a32a8)) // W29C011A

	ROM_REGION(0x800, "eeprom", 0)
	ROM_LOAD("24lc16b.ic8", 0x000, 0x800, CRC(1cae70db) SHA1(575d4c787fd65950417e85fdb34d2961fc327c74))
ROM_END

/* Diana Bifuca (unknown version) on Compumatic ProSPDP-V3 PCB
  ____________________________________________________________________________________________________
__|_  ________ ___ ___ ___ __________ ____________ ________ __________ ____ ________ __________       |
|   ||_CN105_| 110 123 119 |__CN125_| |__CN124___| |_CN121| |__CN112_|CN127 |_CN128| |__CN103_|   ___ |
| C |                                               _______ __________                          : |C ||
| N |                                               |CN116| |__CN115_|  BATTERY 3.6V              |N ||
| 1 |   ______               ______   ______       ______                                         |1 ||
| 1 | CD4514BCWM             MM74HC   MM74HC     CD4514BCWM           ______       ___________    |0 ||
| 3 |  ___________  _____   _____       _____________        ______   MM74HC       |COMPUMATIC|   |6 ||
|   |  |UDN2981A_| 74HC00D 74HC00D   :  GM76C88ALK-15        MM74HC   ______       | REF0096  |Xtal__||
|   |  ___________        ______      _______________  ___   ______   |HC367       |__________|   |C ||
|   |: |TD62683AP|        MM74HC      |  28SF040A   | 25C16N TLC77051P               Xtal 16.000  |N ||
|   |: ___________                    |_____________|           __________  ____                  |1 ||
|   |: |TD62683AP|    _____           _________________      ATF16V8B-15PC LM358                  |0 ||
|   |: ___________    |REF |          |PHILIPS REF34VA| Xtal     _________                TEST    |9 ||
|   |  |TD62683AP|    |8032|   ____   |_______________| 24.000   |ULN2803A|              SWITCH   |__||
|___|                 |____|   CN122                                                             CN102|
  | ________       _________   ____  _______________  ___________________ _____ _________  ________   |
  | |_CN101_|      |__CN111_|  CN126 |___CN108______| |_____CN107________|CN114 |_CN118_| |_CN120_|   |
  |___________________________________________________________________________________________________|

CN101 = DISPLAY INFO
CN102 = SPEAKER
CN103 = CARD/NOTE
CN105 = 6 pin: GND, RX, TX, TEL, CTL, 12V
CN106 = EXPANSION
CN107 = POWER SUPPLY
CN108 = BUTTON PRO
CN109 = PAN-LED
CN110 = 2 pin: GND / KEY
CN111 = ACCEPTOR
CN112 = TARGET A
CN113 = DISPLAY
CN114 = 2 pin: GND, DISP
CN116 = 3 pin: BI, SS, SM
CN118 = GUN A
CN120 = GUN B
CN122 = ICC (3 pin: T, GND, X)
CN123 = CNT, +12
CN124 = INTA PRO
CN125 = LED-RING
CN126 = COIN
CN127 = FDD (4 pin: +5, FDD, GND, +12)
CN128 = TROB (5 pin: BUL, OUL, REL, THL, +12)
*/
ROM_START(dibifpspdp)
	// REF34VA K0V951 Phr0038 F
	PHILIPS_REF34VA

	ROM_REGION(0x80000, "program", 0)
	ROM_LOAD("28sf040a.ic3", 0x00000, 0x80000, CRC(f5727a08) SHA1(f4185afc62c1d1f6cb6c772ea40062ced9b2130a)) // COMPUMATIC RESEARCH S.L. (c) 1997

	ROM_REGION(0x800, "eeprom", 0)
	ROM_LOAD("25c16n.ic8", 0x000, 0x800, CRC(a89a5016) SHA1(84cb29477b1917225e972c2a25e396567c145719)) // Atmel 25C16N

	ROM_REGION(0x117, "plds", 0)
	ROM_LOAD("atf16v8b.ic7", 0x000, 0x117, CRC(85e98105) SHA1(9b3389eedd62b3e599559a03e9664ed1e374d60b))
ROM_END

/* Info about "Far West":
 The sound contains shooting samples and a small sample of the Rawhide main theme.
 Background layout (four shooting targets as food cans with led circles), see https://youtu.be/YVxThMwhvKQ

                            o o o o
                    o o o o         o o o o          <- 16 LEDs
                o o                         o o         first three from the left red, the rest yellow


              o                                  o
           o     o                            o     o      <- Outer circle: 20 blue LEDs
        o     o     o                      o     o     o      Middle circle: 20 green LEDS
      o     o   o     o                  o     o   o     o    Inner circle: 7 red LEDs
    o     o       o     o              o     o       o     o
  o     o           o     o          o     o           o     o
      o      o o      o                  o      o o      o
 o   o      o o o      o   o        o   o      o o o      o   o
      o      o o      o                  o      o o      o
   o    o           o    o            o    o           o    o
          o       o                          o       o
      o     o   o    o                   o     o   o    o
              o                                  o
           o     o                            o     o
              o                                  o

 |PLAYER 1|  |PLAYER 2|  |PLAYER 3|  |PLAYER 4|     |SHOOTS |   <- Labels.
  __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __ __      The display shows scrolling text all across the 20 digits
 |_||_||_||_||_||_||_||_||_||_||_||_||_||_||_||_||_||_||_||_|   <- 20 x 7-segments display
 |_||_||_||_||_||_||_||_||_||_||_||_||_||_||_||_||_||_||_||_|

              o                                  o
           o     o                            o     o
        o     o     o                      o     o     o
      o     o   o     o                  o     o   o     o
    o     o       o     o              o     o       o     o
  o     o           o     o          o     o           o     o
      o      o o      o                  o      o o      o
 o   o      o o o      o   o        o   o      o o o      o   o
      o      o o      o                  o      o o      o
   o    o           o    o            o    o           o    o
          o       o                          o       o
      o     o   o    o                   o     o   o    o
              o                                  o
           o     o                            o     o
              o                                  o

  ________
  | START | <- Button with light
  | BUTTON|
  |_______|
*/
ROM_START(cfarwest)
	// Philips REF34VA K8V2873 Phr9920 0
	PHILIPS_REF34VA

	ROM_REGION(0x20000, "program", 0)
	ROM_LOAD("farwest_pistola.ic3", 0x00000, 0x20000, CRC(ad68a0e8) SHA1(157a6a84f31e05d289e2fc67099fcff2887a84b9))

	// No EEPROM on this PCB
ROM_END


/** Compumatic Minidart.
 Microdar V6 PCB. 24MHz xtal. Atmel AT89S51 intead of the usual REF34VA and a custom chip labeled "Toronto 2707"
 (but referred as REF0097 on the manual) instead of the REF0096.
*/
ROM_START(minidart)
	// Not REF34VA, but a regular Atmel AT89S51
	ROM_REGION(0x1000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD("at89s51.ic1", 0x0000, 0x1000, NO_DUMP)

	ROM_REGION(0x40000, "program", 0)
	ROM_LOAD("minidart_v0.00.ic3", 0x00000, 0x40000, CRC(5ab9f755) SHA1(e80d5d0e8fc8bc246dcf9ef82c7a656f8a1b86ff)) // MINIDART 0.00 COMPUMATIC S.L. 2009

	ROM_REGION(0x800, "eeprom", 0)
	ROM_LOAD("24lc16b.ic6", 0x000, 0x800, NO_DUMP)

	ROM_REGION(0x117, "plds", 0)
	ROM_LOAD("atf16v8b.ic4", 0x000, 0x117, NO_DUMP)
ROM_END


/* Compumatic Microdard-V5 PCB
   _______________________________________________________________________________
  | ______    _____________________________  _________  TEST  ___  _______     __|
  ||_CN__|   |__________CN________________| |___CN___| SWITCH|CN| |__CN__|    |CN|
 _|___                        ________________________   XT1                  ___|_
|    |   ________________    |REF0034 91515S         | 16.000  BATT           |C  -
| CN |  |CD4514BE       |    |                       |   MHz                  |N  -
|    |  |_______________|    |_______________________|                        |____
|    |   ____________                           ___________________            __|
|    |  |__UDN2981A_|   _____  _____________   |27C10000C-15      |  ____     |C||
|    |   ____________  |24C16 |_TD62083AP__|   |__________________| |___|     |N||
|    |  |__UDN2981A_|   __________  __________  ____________        ________  | ||
|    |   ____________  |SN74HC00N| |PALCE16V8| |_GM76C88ALK|       |REF0096|  |_||
|    |  |__UDN2981A_|   _______________________ ____________       |COMPUMA|   __|
|    |   ____________  |REF0032 9616T         ||_HD74HC373P|       |TIC    |  |C||
|    |  |__UDN2981A_|  |______________________|   XT3 16.0MHz  XT2 |_______|  |N||
|____|     _____ __ __ __ __ __ __  __________________________   ____        ::  |
  |       |_CN_| CN CN CN CN CN CN |________CN_______________|  |_CN|            |
  |______________________________________________________________________________|
*/
ROM_START(diolakoa)
	PHILIPS_REF34VA

	ROM_REGION(0x20000, "program", 0)
	ROM_LOAD("mt_plus_8_38.ic3", 0x00000, 0x20000, CRC(5d58bbbc) SHA1(b47adf99c2a443792b99d3881602ffdbdccfd3b9))

	ROM_REGION(0x800, "eeprom", 0)
	ROM_LOAD("24c16.ic6", 0x000, 0x800, NO_DUMP)

	ROM_REGION(0x117, "plds", 0)
	ROM_LOAD("palce16v8h.ic8", 0x000, 0x117, NO_DUMP)
ROM_END

ROM_START(diola827)
	// REF 0034 9515S (without Philips logos)
	PHILIPS_REF34VA

	ROM_REGION(0x20000, "program", 0)
	ROM_LOAD("mt_plus_8_27.ic3", 0x00000, 0x20000, CRC(ada2ce10) SHA1(30fd0bd4eae282467dcec2ee1fe2dab47f4ea4d6))

	ROM_REGION(0x800, "eeprom", 0)
	ROM_LOAD("24c16.ic6", 0x000, 0x800, NO_DUMP) // Atmel 24C16

	ROM_REGION(0x117, "plds", 0)
	ROM_LOAD("palce16v8h.ic8", 0x000, 0x117, NO_DUMP)
ROM_END


/* Covielsa "Covidart".
   Very similar to Compumatic hardware, but using a MCU without internal ROM and a slightly different PCB.

 PCB labeled as "0095" (same text as on EPROM label).
  ______________________________________________________________________________
 |  ooooo oooooo oo OOOOOO  ····  ooooooooooooooo                             _|_
 |                     __   __   __ <- H606014                               |   |
 |     PALCE16V8H-25->| |  |_|  |_|  ________  ________   ________ ________  |   |
 |                    | | 24LC04B   |ULN2803A |ULN2803A  HEF4094BP TD62783AP |   |
74HC273N    74HC273N  |_|     _____  __  __  __           ________ ________  |   |
 | __  __  __  __  ____      |    | | | | | | |          HEF4094BP TD62783AP |   |
 || | | | | | | | |   | _____80C31| | | | | | |           ________ ________  |   |
 || | | | | | | | EPROM|    ||    | |_| |_| |_|          HEF4094BP TD62783AP |   |
 ||_| |_| |_| |_| |   ||    ||    | 3 x 74HC273N          ________ ________  |   |
 |  2 x 74HC244N  |   ||    ||    |      ______          HEF4094BP TD62783AP |   |
 |                |   ||    ||    |     | BATT|        ___________           |   |
 |                |___||____||____|     |_____|       |MC14514BCP|           |   |
 |                   HM6264ALP-12                     |__________|    0095   |   |
 |  oooooooooo ooooooooo ooooooooooooooooooo           OO      OOO           |___|
 |_____CN7________CN8_________CN9_____________________CN10_____CN11_____________|

  MCU: Philips PCB80C31BH-3-16P
*/
ROM_START(covidarts)
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("eprom_095.ic3", 0x00000, 0x20000, CRC(e42172f8) SHA1(667e2a79517af4e8344f8dccf12b83f7788841e3))

	ROM_REGION(0x200, "eeprom", 0)
	ROM_LOAD("24lc04.ic6", 0x000, 0x200, NO_DUMP) // Microchip 24LC04B

	ROM_REGION(0x117, "plds", 0)
	ROM_LOAD("palce16v8h-25.ic7", 0x000, 0x117, BAD_DUMP CRC(3a35a751) SHA1(e39fc8784d94ff09e0ff814f469ce23e52bb35fd)) // Bruteforced and verified OK
ROM_END

} // anonymous namespace


GAME(199?, dibifuca,   0,        microdar,  microdar, microdar_state, empty_init, ROT0, "Compumatic / Bifuca", "Diana Bifuca (v9.25)",                          MACHINE_IS_SKELETON_MECHANICAL)
GAME(199?, dibif743,   dibifuca, microdar,  microdar, microdar_state, empty_init, ROT0, "Compumatic / Bifuca", "Diana Bifuca (v7.43)",                          MACHINE_IS_SKELETON_MECHANICAL)
GAME(199?, dibif727,   dibifuca, microdar,  microdar, microdar_state, empty_init, ROT0, "Compumatic / Bifuca", "Diana Bifuca (v7.27)",                          MACHINE_IS_SKELETON_MECHANICAL)
GAME(1997, dibifpspdp, dibifuca, prospdp,   microdar, microdar_state, empty_init, ROT0, "Compumatic / Bifuca", "Diana Bifuca (unknown version, ProSPDP based)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1997, cfarwest,   0,        microdar,  microdar, microdar_state, empty_init, ROT0, "Compumatic",          "Far West (Compumatic)",                         MACHINE_IS_SKELETON_MECHANICAL)
GAME(1997, diolakoa,   0,        microdv5,  microdar, microdar_state, empty_init, ROT0, "Compumatic / Olakoa", "Diana Olakoa (v8.38)",                          MACHINE_IS_SKELETON_MECHANICAL)
GAME(1997, diola827,   diolakoa, microdv5,  microdar, microdar_state, empty_init, ROT0, "Compumatic / Olakoa", "Diana Olakoa (v8.27)",                          MACHINE_IS_SKELETON_MECHANICAL)
GAME(1997, covidarts,  0,        covidarts, microdar, microdar_state, empty_init, ROT0, "Covielsa",            "Covidarts",                                     MACHINE_IS_SKELETON_MECHANICAL)
GAME(2009, minidart,   0,        prospdp,   microdar, microdar_state, empty_init, ROT0, "Compumatic",          "Minidart",                                      MACHINE_IS_SKELETON_MECHANICAL)

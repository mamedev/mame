// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************
Skeleton driver for IBM ThinkPad 750
The IBM ThinkPad 750 (750/750C/750Cs/750P/750Ce) is a laptop series from the ThinkPad line
that was manufactured by IBM. It is based on the i486 processor.

Hardware for the 750Cs model (released November 1993):

* Motherboard
              _____________________________                 ______     ____________          ______
  ___________|____________________________|________________|_____|____|___________|_________|_____|_______
 |                                                                                                       |   _________
 |            __________                                                                                 |__|  _____ |
 |           |         |   ________   _____                      _____________  ______________   ______       |HC132 |
 |__         | 53G9032 |  |       |  |    |       _____________ |            | |             |  |75LV4735            |
   |         |         |HD6473378F10 |    |      |            | | Intel      | | Intel       |  _____  _____       __|
   |         |_________|  |_______| TC514260BJLL-60           | | SC82360SL  | | KU80486SL-33| |    | |    |      |
   |          _____________  ______  |    |      | WDC        | |            | |             | 2 x M5M44900ATP    |
   |         |            | |     |  |    |      | WD90C24A-ZZ  |            | |             | |    | |    |    __|
   |         | Intel      | 59G3315  |____|      |            | |____________| |             | |____| |____|   |
   | ______  | SE82365SL  | |_____|              |____________|   _______      |_____________|    ________     |
   ||     |  |            |  ______                              |AV9154-10CS16       _____      |       |     |_
   ||EPROM|  |            |DS1267S-010         _____                         __      |8L05|     PC8477BVF-1      |
   ||_____|  |____________| |_____|        ___|    |________________________| |                  |       |       |_
   |                           ___________|                                  _|                  |_______|         |
   |__________________________|                                             |                       _____  ______  |_
                                                                            |  ___________________ |HC244 |HC244     |
                                                                            |_|__________________|_____________      |
                                                                                                               |_____|
                                                                                                                ______
                                                                             __________________________________|     |
                                                                            |                                        |
                                                                            |                                       _|
    ___________________________                                             |    ___________________________       |
   | ________________________ |____________                                 |_   |||||||||||||||||||||||||||      _|
   | ||||||||||||||||||||||||             |___      _________________________ |  |||||||||||||||||||||||||||     |
   |                                       __ |____|              ______    |_|                                 _|
   |                       _________       -|       _________    59G3482                ______  ______  ______ |
   |          ____  ____  | IBM    |       -|      M5M44260ATP   _________ ___________ |     | |     | |     | |
   |          HT04A M811P | 06G9999|       -|      |________|   | NEC    | ||||||||||| |___6 x KM49C512ALJ-7_| |___
   | ____  ____           |        |       -|                   | 53G9031|              ______  ______  ______    |
   | HT08A HC4040A        |________|       -| ____   _________  |        |             |     | |     | |     |    |
   | ____   ____   ____ ____ ____          -|HC126A 74HC4067D|  |________|  Xtal       |_____| |_____| |_____|    |___
   |HC4066 HC4066 HT32A 957B HC132         -| ____  |________|      ______  32.768                      _____  _____ |
  _|                                       -| LM385                |74HC595              _____________  HC132  HC74A |
 |                                                    ____________________________       |||||||||||||   __          |
 |            _____________________________           ||||||||||||||||||||||||||||                      |  |_________|
 |           |                            |                 ______     ____________          ______     |
 |___________|                            |________________|     |____|           |_________|     |_____|
             |____________________________|                |_____|    |___________|         |_____|

 Hitachi HD6473378F10
 Intel SE82365SL (PC Card Controller)
 Dallas DS1267S-010
 Toshiba TC514260BJLL-60 DRAM
 Western Digital WD90C24A-ZZ (VGA)
 Intel SC82360SL
 Intel KU80486SL-33 CPU
 Avasem AV9154-10CS16 clock generator
 National Semiconductor PC8477BVF-1 floppy disc controller
 OKI M811P SEEPROM
 2 x Mitsubishi M5M44900ATP,
 Mitsubishi M5M44260ATP
 6 x Samsung KM49C512ALJ-7

 Custom chips:
 - 53G9032 from Matsushita Electronics / IBM
 - 59G3315 from Signetics / IBM (© 1992 IBM, © 1980 Intel, © 1988 Signetics)
 - 06G9999 from IBM
 - 59G3482 from IBM (xtal?)
 - 53G9031 from NEC / IBM


 * Sound board

 Crystal CS4248-KL ISA 16-bit sound
 16.934 xtal
 24.576 xtal
 2 x Dallas DS1669S
 2 x Motorola MC33274AD
 National Semiconductor LP339M


 Existing BIOS versions:
                                                            355/355C/355Cs
                                                            360/360C/360Cs
        750/750C/750Cs                                        360CE/360CSE
        750P              750Ce             755C/755Cs        360P/360PE
      ----------------  ----------------  ----------------  ----------------  -------------------------------------------------------
Ver.    P/N     Date      P/N     Date      P/N     Date      P/N     Date    Feature
----  ------- --------  ------- --------  ------- --------  ------- --------  -------------------------------------------------------
1.00  59G3849 10-14-93    N/A     N/A       N/A     N/A       N/A     N/A     Supports 750Cs
1.01  59G3966 11-19-93    N/A     N/A       N/A     N/A       N/A     N/A     Supports A/D Video Converter
1.02  59G3968 11-25-93    N/A     N/A       N/A     N/A       N/A     N/A     Corrective service
1.03  59G3970 12-20-93  89G4293 11-22-93    N/A     N/A       N/A     N/A     Supports 750Ce
1.04  59G3980 02-28-94  89G4576 02-15-94    N/A     N/A       N/A     N/A     Supports TV Tuner
1.04a 59G3985 03-01-94   ditto   ditto      N/A     N/A       N/A     N/A     Corrective service
1.05  59G3983 04-04-94  59G4063 03-18-94    N/A     N/A       N/A     N/A     Supports hard disk drive more than 512MB and CDPD
1.06  59G3987 05-25-94  40G3482 04-27-94  40G3483 04-27-94  89G5074 05-23-94  Supports 755 and 355/360 families
1.07   ditto   ditto     ditto   ditto    89G5118 06-03-94  89G5119 06-03-94  Corrective service
1.07a  ditto   ditto     ditto   ditto     ditto   ditto    89G5213 06-14-94  Corrective service
1.12  89G5075 07-07-94  89G5076 07-07-94  89G5078 07-20-94  89G5077 07-20-94  Supports Docking Station II
1.13   ditto   ditto     ditto   ditto     ditto   ditto    89G5704 09-02-94  Corrective service
1.15  91G0362 10-12-94  91G0363 10-14-94  91G0365 10-14-94  91G0375 10-14-94  Supports 360CE and 360CSE
1.16   ditto   ditto     ditto   ditto     ditto   ditto     ditto   ditto    Adds a PATCH diskette for Madge TRN ISA card
1.20  91G0716 03-31-95  91G0720 04-26-95  91G0724 06-30-95  91G0723 06-30-95  Supports Windows 95 (beta) plug and play function
1.40   ditto   ditto     ditto   ditto    91G1218 09-13-95  91G1217 09-13-95  Supports Windows 95
1.41   ditto   ditto     ditto   ditto     ditto   ditto     ditto   ditto    Fixes installation problem of 750 family (except 750Ce)
1.42   ditto   ditto     ditto   ditto    91G1533 02-15-96  91G1534 02-21-96  VGA chip new version support and corrective service
1.43  91G1719 03-13-96  91G1720 03-13-96   ditto   ditto     ditto   ditto    New function support

***********************************************************************************************************/

#include "emu.h"

#include "cpu/h8/h8325.h"
#include "cpu/i386/i386.h"

#include "speaker.h"


namespace {

class thinkpad750_state : public driver_device
{
public:
	thinkpad750_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void thinkpad750(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	void main_map(address_map &map) ATTR_COLD;
};


void thinkpad750_state::main_map(address_map &map)
{
	map(0x0000'0000, 0x0009'ffff).ram();
	map(0x000e'0000, 0x000f'ffff).rom().region("maincpu", 0x20000);
	map(0xfffc'0000, 0xffff'ffff).rom().region("maincpu", 0);
}


void thinkpad750_state::thinkpad750(machine_config &config)
{
	I486(config, m_maincpu, 33.333_MHz_XTAL); // Intel i486SL-33
	m_maincpu->set_addrmap(AS_PROGRAM, &thinkpad750_state::main_map);

	H8325(config, "mcu", 10'000'000); // Actually an H8/337 (HD6473378), unknown clock

	SPEAKER(config, "speaker", 2).front();
}


ROM_START(tpad750cs)
	ROM_DEFAULT_BIOS("v143")
	ROM_SYSTEM_BIOS( 0, "v143", "v1.43 (91G1719, 03-13-96)" )
	ROM_SYSTEM_BIOS( 1, "v103", "v1.03 (59G3970, 12-20-93)" )

	ROM_REGION( 0x40000, "maincpu", 0 )
	ROMX_LOAD( "91g1719_000e000.fl1", 0x00000, 0x40000, CRC(b70abec7) SHA1(9480e0a2c5b9c2d6285f7b13807429604580dc51), ROM_BIOS(0) )
	ROMX_LOAD( "59g3968_n28f020.u43", 0x00000, 0x40000, CRC(f90a8f8c) SHA1(c70a678291a5963594ea55a3d691459271c99b8c), ROM_BIOS(1) )  // Chip labeled 59G3968, but contains 59G3970, probably got updated

	ROM_REGION(0x8000, "mcu", 0)
	ROM_LOAD( "59g3812_hd6473378f10.u34", 0x0000, 0x8000, CRC(29a0dad0) SHA1(7f3158ae93fda0f329eba69650d1e47e64712c58) )

	ROM_REGION(0x80, "seeprom", 0)
	ROM_LOAD( "m811p.u26", 0x00, 0x80, CRC(92560c80) SHA1(47beaa8c86de39ac2f7b92642861e835731ef9ea) ) // 59G3850
ROM_END

} // anonymous namespace

//    YEAR, NAME,      PARENT, COMPAT, MACHINE,     INPUT, CLASS,             INIT,       COMPANY, FULLNAME,         FLAGS
COMP( 1993, tpad750cs, 0,      0,      thinkpad750, 0,     thinkpad750_state, empty_init, "IBM",   "ThinkPad 750Cs", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

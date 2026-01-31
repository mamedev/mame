// license:BSD-3-Clause
// copyright-holders:flama12333
/*************************************************************************
Basic Hardware info: (Wip)
At front PCB Labeled SC031028A1-1 CY-123
U23 U25 U24 hd74ls138p
U6  U27 U28 U29? hd74ls245p
U2 W78E052C40DL                       - MCU (8kb) 8052 Microcontroller with internal rom.
U7 HM6264LP-12                        - RAM (8kb) 
U17 U6295                             - ADPCM Clone of MSM6295
U18 UM3567                            - OPLL Clone of ym2413
U22 ATF1508AS                         - EEPLD
U49 D8255AC-2                         - PPI1 for led
U50 M5L8255AP-5                       - PPI2 for led
EEPROM:
m27c801-100f1                         - ROM (1024kb) adpcm rom.

xtal:
X1 3.579MHZ            - ???
X2 10.738M             - mcu
X3 3.57mhz             - OPLL 

At Back PCB 
FEITALIN GAME SQUARES 20/03/08 #959 - Sticker
NM27C010 or TMS27C010A At back of pcb - ROM (128kb) program rom. first 8kb empty.
3.6v 60mah - Varta Battery

Port:
CN8 DB37
J1 CON12
J2 CON11
J3 6PIN PW
J4 CON2
J5 2.5 NH 9P
J6 2.5 NH 10P
J7 CON8
J8 CON30A

Todo:
Loop at 378 ljmp loop. waiting for timer?. 
Check if the eeprom are encrypted?.

*/

#include "emu.h"
#include "cpu/mcs51/i80c52.h"
#include "machine/i8255.h"
#include "sound/ymopl.h"
#include "sound/okim6295.h"

#include "speaker.h"

namespace {

class ymball_state : public driver_device
{
public:
	ymball_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
			, m_oki(*this, "oki")

	{ }

	void ymball(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void data_map(address_map &map) ATTR_COLD;
	void program_map(address_map &map) ATTR_COLD;
	optional_device<okim6295_device> m_oki;

};

static INPUT_PORTS_START( ymball )
	
INPUT_PORTS_END

void ymball_state::program_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("maincpu", 0); // mcu
	map(0x2000, 0xffff).rom().region("eeprom",  0x2000); // eeprom

}

void ymball_state::data_map(address_map &map)
{
	
}

void ymball_state::machine_start()
{
	
}

void ymball_state::ymball(machine_config &config)
{
    // basic machine hardware
	i80c52_device &maincpu(I80C52(config, "maincpu", XTAL(10'738'635))); // Actual cpu is W78E052C40DL
	maincpu.set_addrmap(AS_PROGRAM, &ymball_state::program_map);
	maincpu.set_addrmap(AS_DATA, &ymball_state::data_map);
	
    // Programmable Peripheral Interface
	I8255A(config, "ppi1");
	I8255A(config, "ppi2");

  	// sound hardware
	SPEAKER(config, "mono").front_center();
	
	ym2413_device &opll(YM2413(config, "opll", 3.579545_MHz_XTAL));
	opll.add_route(ALL_OUTPUTS, "mono", 1.0);
	OKIM6295(config, m_oki,  XTAL(10'738'635) / 4, okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "mono", 1.00);  // Clock frequency & pin 7 not verified

}

ROM_START( ymball )
	ROM_REGION( 0x02000, "maincpu", 0 )
   	ROM_LOAD( "w78e052c.u2", 0x00000, 0x02000, CRC(b950c825) SHA1(822cd26c2df0c4966ab0bbed5a87af1f98cfd7a3) ) // Unprotected
	
	ROM_REGION( 0x20000, "eeprom", 0 )
	ROM_LOAD( "nm27c010.u8", 0x00000, 0x20000,  CRC(fc9b528d) SHA1(4d47945e6aab4a85ffdc37027707377dd6640108) ) // Hex FF filled at 0x0000-0x1fff  

    ROM_REGION( 0x100000, "oki", 0 )
    ROM_LOAD( "m27c801.u19", 0x00000, 0x100000,  CRC(311e2451) SHA1(772b42d7231c6852c0226c23e4b49c1d7a6a397f) ) // adpcm
    ROM_END

} // anonymous namespace


//    YEAR  NAME        PARENT   MACHINE     INPUT    STATE             INIT        ROT   COMPANY       FULLNAME                       FLAGS
GAME( 2003, ymball,     0,       ymball,     ymball,  ymball_state,     empty_init, ROT0, "Feitalin",   "Feitalin Game Square",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND  | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK  )

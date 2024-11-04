// license:BSD-3-Clause
// copyright-holders:

/*

Stella 8085 based hardware
Electromechanical fruit machines
Lots of lamps and 4 7-segment LEDs

Main components:
Siemens SAB 8085AH-2-P (CPU)
Sharp LH5164D-10L (SRAM)
Siemens SAB 8256 A 2 P (MUART)
NEC D8279C-2 (keyboard & display interface)
RTC 62421A

Is there a sound board or is it discrete?

Game reference: https://www.youtube.com/watch?v=NlB06dMxjME
*/


#include "emu.h"

#include "cpu/i8085/i8085.h"
//#include "machine/i8256.h"
#include "machine/i8279.h"
#include "machine/msm6242.h"
//#include "sound/???.h"

#include "speaker.h"


namespace {

class stella8085_state : public driver_device
{
public:
	stella8085_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void dicemstr(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


void stella8085_state::program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xffff).rom();
}

void stella8085_state::io_map(address_map &map)
{
}


static INPUT_PORTS_START( dicemstr )
	PORT_START("INPUTS")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void stella8085_state::dicemstr(machine_config &config)
{
	I8085A(config, m_maincpu, 10.240_MHz_XTAL / 2); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &stella8085_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &stella8085_state::io_map);

	//I8256(config, "muart1", 10.240_MHz_XTAL / 2); // divider not verified

	I8279(config, "kdc", 10.240_MHz_XTAL / 2); // divider not verified

	RTC62421(config, "rtc", 32.768_kHz_XTAL);

	SPEAKER(config, "mono").front_center();
}


ROM_START( dicemstr ) // curiously hand-written stickers say F3 but strings in ROM are F2
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "stella_dice_master_f3_i.ice6",  0x0000, 0x8000, CRC(9897fb87) SHA1(bfb18c1370d9bd12ec61622c0ebbad5c0138e1d8) )
	ROM_LOAD( "stella_dice_master_f3_ii.icd6", 0x8000, 0x8000, CRC(9484cf3b) SHA1(e1104882eaba860ab984c1a37e2f97d4bed08829) ) // 0x0000 - 0x1fff is 0xff filled
ROM_END

} // anonymous namespace


// 'STELLA DICE MASTER F2' and 'COPYRIGHT BY ADP LUEBBECKE GERMANY 1993' in ROM
GAME( 1993, dicemstr, 0, dicemstr, dicemstr, stella8085_state, empty_init, ROT0, "Stella", "Dice Master", MACHINE_IS_SKELETON_MECHANICAL )

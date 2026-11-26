// license:BSD-3-Clause
// copyright-holders:
/***************************************************************************

 Skeleton driver for "Futsala" electromechanical coinop from Bifuca.
 More info and PCB pics at: https://www.recreativas.org/futsala-6645-bifuca-sl

 Hardware:
 - ST62T15B6-HWD near a 3.6864 MHz crystal
 - PIC16C61
 - Bank of 5 DIP switches
 - Small subboard with an EPROM and a OKI M6376

***************************************************************************/

#include "emu.h"

#include "cpu/pic16_mid/pic16_mid.h"
#include "cpu/st62xx/st62xx.h"
#include "sound/okim6376.h"

#include "speaker.h"

namespace
{

class futsala_state : public driver_device
{
public:
	futsala_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_soundcpu(*this, "soundcpu")
		, m_okim6376(*this, "oki")
	{
	}

	void futsala(machine_config &config) ATTR_COLD;

private:
	required_device<st6228_device> m_maincpu;
	required_device<pic16f84_device> m_soundcpu;
	required_device<okim6376_device> m_okim6376;
};

// 1 x 5 DIP switches bank
INPUT_PORTS_START(futsala)
	PORT_START("DSW")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
INPUT_PORTS_END

void futsala_state::futsala(machine_config &config)
{
	ST6228(config, m_maincpu, 3.6864_MHz_XTAL); // Actually a ST62T15B6-HWD

	PIC16F84(config, m_soundcpu, 4'000'000); // Actually a PIC16C61, real clock unknown

	OKIM6376(config, m_okim6376, 128'000).add_route(ALL_OUTPUTS, "mono", 1.0); // Real clock unknown

	SPEAKER(config, "mono").front_center();
}

ROM_START(futsala)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("procesador_nor._bifuca_31_cip_030.bin", 0x0000, 0x1000, NO_DUMP) // Protected

	ROM_REGION16_LE(0x4280, "soundcpu", 0)
	ROM_LOAD("pic16c61-04_p.bin", 0x0000, 0x4280, NO_DUMP) // Protected

	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("espanol_1.bin", 0x00000, 0x80000, CRC(8d7eac9a) SHA1(2e1940bfbd80dd4809beaa641da6e6ce21807dfe))
ROM_END

} // anonymous namespace


//    YEAR  NAME     PARENT  MACHINE  INPUT    CLASS          INIT        ROT   COMPANY   FULLNAME   FLAGS
GAME( 2000, futsala, 0,      futsala, futsala, futsala_state, empty_init, ROT0, "Bifuca", "Futsala", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

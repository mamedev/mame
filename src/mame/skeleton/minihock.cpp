// license:BSD-3-Clause
// copyright-holders:
/***************************************************************************

 Skeleton driver for "Mini Hockey" electromechanical coinop from Rumatic.
 More info and PCB pics at: https://www.recreativas.org/hockey-4077-rumatic


 Rumatic Mini Hockey (1991)
  _____________________________________________________________
 |                                                __________  |
 ||:                                             |GD74LS373|  |
 |                                          ________________  |
 ||:                                       | EPROM         |  |
 ||:                                       |_______________|  |
 ||:                            ____                          |
 ||:                           |LM358                         |
 |                                       ___________________  |
 ||:                                    | NEC D7759C       |  |
 ||:                                    |__________________|  |
 ||:                                                     640B <- Resonator
 ||:                                     ___________________  |
 |                                      | MC68705U3S       |  |
 ||:                                    |__________________|  |
 ||:                                        Xtal              |
 ||:                                     4.000 MHz            |
 ||:                        __________   __________           |
 |                         |SN74LS03N|  |SN74LS03N|           |
 ||:                                                          |
 ||:                        __________   __________           |
 ||:                       |SN74LS03N|  |SN74LS03N|           |
 ||:                                                          |
 |                          __________        _____           |
 ||:                       |_DIPSx8__|        NE555           |
 |                                                            |
 ||:   ______________   ............    ............          |
 |    |:::::::::::::|   _____________   _____________         |
 |____________________________________________________________|

***************************************************************************/

#include "emu.h"
#include "cpu/m6805/m68705.h"
#include "sound/upd7759.h"
#include "speaker.h"

namespace
{

class minihockey_state : public driver_device
{
public:
	minihockey_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_upd7759(*this, "samples")
	{
	}

	void minihockey(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<upd7759_device> m_upd7759;
};

// 1 x 8-dips bank
INPUT_PORTS_START(minihockey)
	PORT_START("DSW")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")
INPUT_PORTS_END

void minihockey_state::minihockey(machine_config &config)
{
	M68705U3(config, m_maincpu, 4_MHz_XTAL); // MC68705U3S

	UPD7759(config, m_upd7759, 640_kHz_XTAL); // NEC D7759C

	SPEAKER(config, "mono").front_center();
}

ROM_START(minihockey)
	ROM_REGION(0x7600, "maincpu", 0)
	ROM_LOAD("hockey_05.u6", 0x0000, 0x7600, NO_DUMP) // Protected, 3776 bytes of ROM

	ROM_REGION(0x20000, "samples", 0)
	ROM_LOAD("hky_v20.u8", 0x00000, 0x20000, CRC(f8859af2) SHA1(9024822c512767d404c72f20c3ff613a3ff5bc57))
ROM_END

} // anonymous namespace


//    YEAR  NAME        PARENT  MACHINE     INPUT       CLASS             INIT        ROT   COMPANY    FULLNAME       FLAGS
GAME( 1991, minihockey, 0,      minihockey, minihockey, minihockey_state, empty_init, ROT0, "Rumatic", "Mini Hockey", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )

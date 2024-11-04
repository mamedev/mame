// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

 Skeleton driver for slot game "Mad Money 2", from Picmatic (hardware by Novatronic).
_____________________________________________________________________________
|   ________________  ________________                                       |
|  |···············| |···············|                                       |
|                                                                            |
|    __________       __________            NOVATRONIC S.A.                  |
|   |MC74HC540|      |UCN5801A_|                                          __ |
|                                                                          ·||
|                               _____________    _____________             ·||
|    __________    __________   | MC68705P3S |   | EPROM      | ___  ___   ·||
|   |74LS373N_|   |_PLS153N_|   |____________|   |____________| |  |<-ULN2803A
|                                                 _____________ |  | |  |  ·||
|    __________                                  | EPROM      | |__| |__|<-MC14015BCP
|   |__EMPTY__|                                  |____________|           __ |
|    __________  __________           __________   ____________ ___  ___   ·||
|   |__EMPTY__| |_________|          |_________|  |KM6816AL-15| |  |<-ULN2803A
|                                                 |___________| |  | |  |  ·||
|    __________  __________  __________  __________  __________ |__| |__|<-MC14015BCP
|   |_________| |CD4028BCN| |_UCN5801A| |SN74LS02N| |_________|            ·||
|                                                                          ·||
|    __________     ________________     ________________       ___  ___   ·||
|   |DM74LS373N    | NSC810AN-3I   |    | NSC810AN-3I   |       |  |<-ULN2803A
|    __________    |_______________|    |_______________|       |  | |  |  ·||
|   |DM74LS74AN                           Xtal                  |__| |__|<-MC14015BCP
|    __________     ________________     ________________                 __ |
|   |_NE556N__|    | AY-3-8910     |    | AY-3-8910     |                  ·||
|                  |_______________|    |_______________|                  ·||
|                          _________                            _______    ·||
|    __________           |_DIPSx8_|                            | BATT |   ·||
|   |MC74HC540|                                                 |______|   ·||
|   ________________  ________________  ________________  ________________ ·||
|  |···············| |···············| |···············| |···············| ·||
|____________________________________________________________________________|

*/

#include "emu.h"
#include "cpu/m6805/m68705.h"
#include "machine/nsc810.h"
#include "sound/ay8910.h"
#include "speaker.h"

namespace {

class madmoney2_state : public driver_device
{
public:
	madmoney2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ay8910_1(*this, "ay8910_1")
		, m_ay8910_2(*this, "ay8910_2")
	{
	}

	void madmoney2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<m68705p3_device> m_maincpu;
	required_device<ay8910_device> m_ay8910_1;
	required_device<ay8910_device> m_ay8910_2;
};

void madmoney2_state::machine_start()
{
}

static INPUT_PORTS_START(madmoney2)
	PORT_START("DSW0")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW0:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW0:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW0:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW0:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW0:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW0:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW0:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW0:8")
INPUT_PORTS_END

void madmoney2_state::madmoney2(machine_config &config)
{
	M68705P3(config, m_maincpu, 3.579'545_MHz_XTAL); // Motorola MC68705P3S

	// Sound hardware

	NSC810(config, "iotimer1", 0, 3.579'545_MHz_XTAL, 3.579'545_MHz_XTAL); // NSC810AN-3I
	NSC810(config, "iotimer2", 0, 3.579'545_MHz_XTAL, 3.579'545_MHz_XTAL); // NSC810AN-3I

	SPEAKER(config, "mono").front_center();

	AY8910(config, m_ay8910_1, 3.579'545_MHz_XTAL); // Guess
	m_ay8910_1->port_a_read_callback().set_ioport("DSW0");
	m_ay8910_1->add_route(ALL_OUTPUTS, "mono", 0.25);

	AY8910(config, m_ay8910_2, 3.579'545_MHz_XTAL); // Guess
	m_ay8910_2->add_route(ALL_OUTPUTS, "mono", 0.25);
}

ROM_START(madmoney2)
	ROM_REGION(0x0800, "maincpu", 0)
	ROM_LOAD("p3_mm.bin",   0x0000, 0x0800, NO_DUMP) // MC68705P3S

	ROM_REGION(0x8000, "roms", 0)
	ROM_LOAD("mad2a.bin",   0x0000, 0x4000, CRC(81d4c727) SHA1(a58c5be0bc7604b7fed095fceab9976aa2125b69))
	ROM_LOAD("mad2b.bin",   0x4000, 0x4000, CRC(7368c04c) SHA1(a5770e4f0c8278a970dc4837e922fe857337817b))

	ROM_REGION(0x0117, "plds", 0)
	ROM_LOAD("pls153n.bin", 0x0000, 0x0117, NO_DUMP) // Signetics PLS153N
ROM_END

} // anonymous namespace

GAME(1988, madmoney2, 0, madmoney2, madmoney2, madmoney2_state, empty_init, ROT0, "Picmatic", "Mad Money 2", MACHINE_IS_SKELETON_MECHANICAL) // String "(C) PICMATIC S.A. 1988 BY B.MEITINER VER 0.0 09.09.88" on ROM

// license:BSD-3-Clause
// copyright-holders:
/********************************************************************************************

Skeleton driver for electromechanical air hockey hardware from the Spanish company Inconel SL

  ____________________________________________
 | ______________                            |
 || ::::::::::: |                            |
 | ______________         ________           |
 || ::::::::::: |        |SN7406N|         |:|
 | ________  ________                      |:|
 |SN74HC245 |SN7407N|     ________         |:|
 | __________________    |ULN2803A         |:|
 || NEC D8155HC     |                      |:|
 ||_________________|    CARAMBALL           |
 | ________              PLACA CPU         |:|
 ||PALCE16V8             011292            |:|
 | __________________     _____            |:|
 || OKI M8085A      |    MAX690CPA           |
 ||_________________|                      |:|
 | Xtal                                    |:|
 | 6.144 MHz ________                      |:|
 |  __      |SN74HC373N                    |:|
 | (__)<-SWITCH                            |:|
 | _______________     __________________  |:|
 || EPROM        |    | AY38910A/P      |    |
 ||______________|    |_________________|  |:|
 | _______________               ________  |:|
 || MK48Z02B-20  |     ________ |_DIPSx8|    |
 ||______________|    4116R-001      ____    |
 |                                  |||_|<-SWITCH
 |___________________________________________|

 More info at: https://www.recreativas.org/caramball-10956-inconel

********************************************************************************************/

#include "emu.h"

#include "cpu/i8085/i8085.h"
#include "machine/i8155.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"

#include "speaker.h"

namespace {

class caramball_state : public driver_device
{
public:
	caramball_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void caramball(machine_config &config);

private:
	void i8155_pa_w(u8 data);
	void i8155_pb_w(u8 data);
	void i8155_pc_w(u8 data);
	void f000_w(u8 data);
	void mem_map(address_map &map) ATTR_COLD;

	required_device<i8085a_cpu_device> m_maincpu;
};

void caramball_state::i8155_pa_w(u8 data)
{
	logerror("%s: 8155 Port A <- %02X\n", machine().describe_context(), data);
}

void caramball_state::i8155_pb_w(u8 data)
{
	logerror("%s: 8155 Port B <- %02X\n", machine().describe_context(), data);
}

void caramball_state::i8155_pc_w(u8 data)
{
	logerror("%s: 8155 Port C <- %02X\n", machine().describe_context(), data);
}

void caramball_state::f000_w(u8 data)
{
	logerror("%s: F000 <- %02X\n", machine().describe_context(), data);
}

void caramball_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0x87ff).ram().share("nvram");
	map(0xa000, 0xa0ff).rw("i8155", FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
	map(0xa100, 0xa107).rw("i8155", FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0xc000, 0xc000).rw("psg", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0xd000, 0xd000).w("psg", FUNC(ay8910_device::address_w));
	map(0xf000, 0xf000).w(FUNC(caramball_state::f000_w));
}

// 1 x 8-dips bank
INPUT_PORTS_START(caramball)
	PORT_START("DSW")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

	PORT_START("UNK")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)
INPUT_PORTS_END

void caramball_state::caramball(machine_config &config)
{
	I8085A(config, m_maincpu, 6.144_MHz_XTAL); // OKI M80C85A
	m_maincpu->set_addrmap(AS_PROGRAM, &caramball_state::mem_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // MK48Z02B-20

	i8155_device &i8155(I8155(config, "i8155", 6.144_MHz_XTAL / 2)); // NEC D8155HC
	i8155.out_pa_callback().set(FUNC(caramball_state::i8155_pa_w));
	i8155.out_pb_callback().set(FUNC(caramball_state::i8155_pb_w));
	i8155.out_pc_callback().set(FUNC(caramball_state::i8155_pc_w));
	i8155.out_to_callback().set_inputline(m_maincpu, I8085_RST65_LINE).invert();

	SPEAKER(config, "mono").front_center();

	ay8910_device &psg(AY8910(config, "psg", 6.144_MHz_XTAL / 4)); // Divider unknown
	psg.port_a_read_callback().set_ioport("DSW");
	psg.port_b_read_callback().set_ioport("UNK");
	psg.add_route(ALL_OUTPUTS, "mono", 1.0); // Guess
}

ROM_START(caramball)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("crmbal_4.0.i7", 0x0000, 0x8000, CRC(6ff5eca9) SHA1(8a926509c6143aced4d2d3a024965d8e33ef1074))

	ROM_REGION(0x800, "nvram", 0)
	ROM_LOAD("mk48z02b-20.i8", 0x000, 0x800, CRC(8125cb9d) SHA1(ac76f81edb092e8f16fae577a0702a1ec4ab4503))

	ROM_REGION(0x117, "plds", 0)
	ROM_LOAD("palce16v8h.i9", 0x000, 0x117, NO_DUMP) // Protected and registered
ROM_END

} // anonymous namespace

//   YEAR  NAME       PARENT  MACHINE    INPUT      CLASS            INIT        ROT   COMPANY    FULLNAME     FLAGS
GAME(1992, caramball, 0,      caramball, caramball, caramball_state, empty_init, ROT0, "Inconel", "Caramball", MACHINE_IS_SKELETON_MECHANICAL)

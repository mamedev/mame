// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Skeleton driver for Nintendo Computer Mahjong

****************************************************************************/

#include "emu.h"
#include "cpu/lh5801/lh5801.h"

class compmahj_state : public driver_device
{
public:
	compmahj_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void compmahj(machine_config &config);

private:
	void main_map(address_map &map);
	void io_map(address_map &map);

	required_device<lh5801_cpu_device> m_maincpu;
};

void compmahj_state::main_map(address_map &map)
{
	map(0x8000, 0x80ff).ram();
	map(0xe000, 0xffff).rom().region("maincpu", 0);
}

void compmahj_state::io_map(address_map &map)
{
}

static INPUT_PORTS_START( compmahj )
INPUT_PORTS_END

void compmahj_state::compmahj(machine_config &config)
{
	LH5801(config, m_maincpu, 1000000); // total guess
	m_maincpu->set_addrmap(AS_PROGRAM, &compmahj_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &compmahj_state::io_map);
}

ROM_START( compmahj )
	ROM_REGION( 0x2000, "maincpu", 0)
	ROM_LOAD( "mj02.bin", 0x0000, 0x2000, CRC(56d51944) SHA1(5923d72753642314f1e64bd30a6bd69da3985ce9) )
ROM_END

/*    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY     FULLNAME            FLAGS */
CONS( 1987, compmahj, 0,      0,      compmahj, compmahj, compmahj_state, empty_init, "Nintendo", "Computer Mahjong", MACHINE_IS_SKELETON )

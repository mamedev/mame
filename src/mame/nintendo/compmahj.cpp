// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
// thanks-to:Sean Riddle
/***************************************************************************

Skeleton driver for Nintendo Computer Mahjong

2 PCBs, MJ-CPU and MJ-DR. Each chip is custom labeled.
MJ-CPU: MJ-01, MJ-02, MJ-04, 2*MJ-03
MJ-DR: 2*MJ-06, MJ-05

MJ-01 is definitely a Sharp LH5801 or LH5803.  I'm not sure of the
difference; the pinouts are the same.  The PC-1600 Technical Reference says
LH-5803 is an "upper" version of LH-5801 that supports most of the same
machine language instructions except SDP, RDP and OFF are NOPs.
MJ-02 is an LH5367 8Kx8 ROM.  It is edge-enabled, so one of the CE lines has
to toggle for the byte at the address to be output.  This is different from
most modern ROMs that you can dump by simply tying /CE low and setting
different addresses.  It's mapped at $E000 - $FFFF.
MJ-03 is LH5101 256x4 SRAM.  There are 2 of them to provide 256x8 RAM.  They
are mapped to $8000-$80FF.
MJ-04 is a parallel I/O controller, like LH5081 or LH5810/LH5811, but it's
in a QFP48, whereas LH5081 is QFP44 and LH5810/11 is QFP60.  The pinout
doesn't match either of those either, so it's not just one of those in a
different package.

MJ-05 is QFP44 and is likely LH5031 LCD dot matrix common driver.
MJ-06 is QFP96 and is likely LH5035A/LH5036A LCD dot matrix segment driver.

LCD display: 1st and 3rd row is 20 8*11-pixel characters. 2nd row is 19
8*10-pixel characters, and a couple of custom segments to the right.

Piezo speaker, 20 buttons, 3-pos switch. There's a 12-pin connector for
linking 2 Computer Mahjongs together, using MJ 8002 link cable.

****************************************************************************/

#include "emu.h"
#include "cpu/lh5801/lh5801.h"


namespace {

class compmahj_state : public driver_device
{
public:
	compmahj_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void compmahj(machine_config &config);

private:
	required_device<lh5801_cpu_device> m_maincpu;

	void main_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	u8 ita_r();
};

u8 compmahj_state::ita_r()
{
	// f75e/f760
	return 0;
}

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
	LH5801(config, m_maincpu, 2.5_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &compmahj_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &compmahj_state::io_map);
	m_maincpu->in_func().set(FUNC(compmahj_state::ita_r));
}

ROM_START( compmahj )
	ROM_REGION( 0x2000, "maincpu", 0)
	ROM_LOAD( "mj-02", 0x0000, 0x2000, CRC(56d51944) SHA1(5923d72753642314f1e64bd30a6bd69da3985ce9) )
ROM_END

} // anonymous namespace


/*    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY     FULLNAME                     FLAGS */
CONS( 1983, compmahj, 0,      0,      compmahj, compmahj, compmahj_state, empty_init, "Nintendo", "Computer Mah-jong Yakuman", MACHINE_IS_SKELETON )

// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/********************************************************************************

    Pinball
    Bally Kiss 8035 prototype

*********************************************************************************/


#include "emu.h"
#include "cpu/mcs48/mcs48.h"


namespace {

class kissp_state : public driver_device
{
public:
	kissp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	void kissp(machine_config &config);

	void init_kissp();

private:
	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset() override ATTR_COLD;
};


void kissp_state::mem_map(address_map &map)
{
	map(0x0000, 0x07ff).rom().region("maincpu", 0);
}

void kissp_state::io_map(address_map &map)
{
}

static INPUT_PORTS_START( kissp )
INPUT_PORTS_END

void kissp_state::machine_reset()
{
}

void kissp_state::init_kissp()
{
}

void kissp_state::kissp(machine_config &config)
{
	/* basic machine hardware */
	I8035(config, m_maincpu, 6000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &kissp_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &kissp_state::io_map);
}

ROM_START(kissp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "kiss8755.bin", 0x4000, 0x0800, CRC(894c1052) SHA1(579ce3c8ec374f2cd17928ab92311f035ecee341))
	ROM_RELOAD( 0x0000, 0x0800)
	ROM_LOAD( "kissprot.u5", 0x1000, 0x1000, CRC(38a2ef5a) SHA1(4ffdb2e9aa30417d506af3bc4b6835ba1dc80e4f))
	ROM_LOAD( "kissprot.u6", 0x2000, 0x1000, CRC(bcdfaf1d) SHA1(d21bebbf702b400eb71f8c88be50a180a5ac260a))
	ROM_LOAD( "kissprot.u7", 0x3000, 0x0800, CRC(d97da1d3) SHA1(da771a08969a12105c7adc9f9e3cbd1677971e79))
	ROM_RELOAD( 0x4800, 0x0800)
ROM_END

ROM_START(kissp2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "8755u8.dat", 0x4000, 0x0800, CRC(d2d04100) SHA1(fe81f3667cb5802c9780761a359660bad83862c2))
	ROM_RELOAD( 0x0000, 0x0800)
	ROM_LOAD( "kissprot.u5", 0x1000, 0x1000, CRC(38a2ef5a) SHA1(4ffdb2e9aa30417d506af3bc4b6835ba1dc80e4f))
	ROM_LOAD( "kissprot.u6", 0x2000, 0x1000, CRC(bcdfaf1d) SHA1(d21bebbf702b400eb71f8c88be50a180a5ac260a))
	ROM_LOAD( "u7.dat", 0x3000, 0x0800, CRC(e224a9b0) SHA1(2a0e3afad8c566432ebe690ff1ce6fa92b68816f))
	ROM_RELOAD( 0x4800, 0x0800)
ROM_END

} // anonymous namespace


GAME( 1979, kissp,  kiss, kissp, kissp, kissp_state, init_kissp, ROT0, "Bally", "Kiss (prototype)",     MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1979, kissp2, kiss, kissp, kissp, kissp_state, init_kissp, ROT0, "Bally", "Kiss (prototype v.2)", MACHINE_IS_SKELETON_MECHANICAL )

// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-10-28 Skeleton

Three Rivers / ICL - PERQ 1A. CPU is discrete components including 74S181 bit-slice ALUs, and AM2910 microcode sequencer.
A Z80 handles most of the I/O. Display is 768x1024. B&W (black on white).

ROMS came from PERQemu by Josh Dersch.

************************************************************************************************************************************/

#include "emu.h"


namespace {

class perq_state : public driver_device
{
public:
	perq_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		//      , m_maincpu(*this, "maincpu")
	{ }

	void perq(machine_config &config);
private:
	//  required_device<cpu_device> m_maincpu;
};

static INPUT_PORTS_START( perq )
INPUT_PORTS_END

void perq_state::perq(machine_config &config)
{
}

ROM_START( perq )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rti02.rom",     0x0000, 0x0200, CRC(cad04f75) SHA1(7a789c6ab1f7627f14953b28231aec0ea49f7945) )
	ROM_LOAD( "t1bootrom.rom", 0x0000, 0x0c00, CRC(cd75f925) SHA1(eb8d5381d87c366052ba1f01f803403e313dc29a) )
	ROM_LOAD( "boot.bin",      0x0000, 0x0d98, CRC(a2b9b7ea) SHA1(75b4b7743e4e65fc14b9f3dbb08421c16cde0f11) )
	ROM_LOAD( "rds00.rom",     0x0000, 0x0400, CRC(77650e0a) SHA1(e507cbc0a1fa56054ce178f7600004d3669961ce) )
	ROM_LOAD( "rsc03.rom",     0x0000, 0x0100, CRC(d66f1f1f) SHA1(5ccccb68dc59dbcabab99adf8a57af0af545bfc5) )
	ROM_LOAD( "rsh00.rom",     0x0000, 0x0400, CRC(815d92bf) SHA1(b87bdea13de391e5615c474ba96af4b28b7f8f38) )
ROM_END

} // anonymous namespace


COMP( 1979, perq, 0, 0, perq, perq, perq_state, empty_init, "Three Rivers Company Corporation", "PERQ 1A", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

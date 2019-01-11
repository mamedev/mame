// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-10-28 Skeleton

Advanced Computer Design computer. CPU is WD9000. Some details at bitsavers.

************************************************************************************************************************************/

#include "emu.h"

class acd_state : public driver_device
{
public:
	acd_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		//      , m_maincpu(*this, "maincpu")
	{ }

	void acd(machine_config &config);
private:
	//  required_device<cpu_device> m_maincpu;
};

static INPUT_PORTS_START( acd )
INPUT_PORTS_END

MACHINE_CONFIG_START(acd_state::acd)
MACHINE_CONFIG_END

ROM_START( acd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d9_cpu_c4_rev_2.4.2.bin", 0x000000, 0x000200, CRC(481b8d65) SHA1(b766b22282cd321c707c18137ddb4c133fd0ede4) )
	ROM_LOAD( "d9_cpu_c5.bin", 0x000000, 0x000200, CRC(481b8d65) SHA1(b766b22282cd321c707c18137ddb4c133fd0ede4) )
	ROM_LOAD( "dr.bin",       0x000000, 0x000020, CRC(a0baab49) SHA1(50b91bf00390c7923b8e03975c1c6fea3c153cd7) )
	ROM_LOAD( "btwph.bin",    0x000000, 0x000020, CRC(a48a80d1) SHA1(60490d1dbd01441ebb9a9860a98131d628d5e3f9) )
	ROM_LOAD( "btwpl.bin",    0x000000, 0x000020, CRC(0f6fb759) SHA1(7ab376865faf57841960bfcfd3e038219b5b66b6) )
	ROM_LOAD( "d7_cpu_b14_rev_1.2.bin", 0x000000, 0x000200, CRC(51b2bb18) SHA1(533ef263146c1e03e5ef30e792a669293f31a1d3) )
	ROM_LOAD( "d7_cpu_c4_rev_2.4.2.bin", 0x000000, 0x000200, CRC(b1b59b5f) SHA1(5c199836a59da78ab404de663d306d0cc10aaf83) )
	ROM_LOAD( "d7_cpu_c5.bin", 0x000000, 0x000200, CRC(7db9ecca) SHA1(535a34608e59c72d13434e3fc2892db9841a0f60) )
	ROM_LOAD( "d9_cpu_b14_rev_1.2.bin", 0x000000, 0x000200, CRC(5572ff8b) SHA1(9e8158c338b2798f7fb1d9c4fa6dd99592d2fae3) )
ROM_END

COMP( 198?, acd, 0, 0, acd, acd, acd_state, empty_init, "Advanced Computer Design", "unknown ACD computer", MACHINE_IS_SKELETON )

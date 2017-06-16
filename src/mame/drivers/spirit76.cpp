// license:BSD-3-Clause
// copyright-holders:Ivan Vangelista
// PINBALL
// Skeleton driver for Mirco's Spirit of 76, one of the first if not the first commercial solid-state pinball game.
// Hardware listing and ROM definitions from PinMAME.

/*
   Hardware:
CPU:   1 x M6800
IO:    1x PIA 6821
SOUND: Chimes
*/

#include "emu.h"
#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"

class spirit76_state : public genpin_class
{
public:
	spirit76_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu") { }

private:
	required_device<cpu_device> m_maincpu;
};

static ADDRESS_MAP_START( maincpu_map, AS_PROGRAM, 8, spirit76_state )
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END


static INPUT_PORTS_START( spirit76 )
INPUT_PORTS_END

static MACHINE_CONFIG_START( spirit76 )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, 500000)
	MCFG_CPU_PROGRAM_MAP(maincpu_map)

	/* video hardware */
	//MCFG_DEFAULT_LAYOUT()

	//6821pia

	/* sound hardware */
	MCFG_FRAGMENT_ADD( genpin_audio )
MACHINE_CONFIG_END


ROM_START(spirit76)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD_NIB_LOW("1g.bin",  0x0600, 0x0200, CRC(57d7213c) SHA1(0897876f5c662b2518a680bcbfe282bb3a19a161))
	ROM_LOAD_NIB_HIGH("5g.bin", 0x0600, 0x0200, CRC(90e22786) SHA1(da9e0eae1e8576c6c8ac734a9557784d9e59c141))
	ROM_LOAD_NIB_LOW("2c.bin",  0x0800, 0x0200, CRC(4b996a52) SHA1(c73378e61598f84e20c1022b811780e300b01cd1))
	ROM_LOAD_NIB_HIGH("3c.bin", 0x0800, 0x0200, CRC(448626fa) SHA1(658b9589ba60ef62ff692192f743038d622776ba))
	ROM_LOAD_NIB_LOW("2e.bin",  0x0a00, 0x0200, CRC(faaa907e) SHA1(ee9227944911a7c068216dd7b1b8dec284f90e3b))
	ROM_LOAD_NIB_HIGH("3e.bin", 0x0a00, 0x0200, CRC(3463168e) SHA1(d98643179eac5ecbf1a559df59da620ea544bdee))
	ROM_LOAD_NIB_LOW("2f.bin",  0x0c00, 0x0200, CRC(4d1a71ec) SHA1(6d3aa8fc4f7cec27d7fae2ecc73425388f8d9d52))
	ROM_LOAD_NIB_HIGH("3f.bin", 0x0c00, 0x0200, CRC(bf23f0fd) SHA1(62e2ef7df0c057f25685a99e57cf95aae2e75cdb))
	ROM_LOAD_NIB_LOW("2g.bin",  0x0e00, 0x0200, CRC(6236f053) SHA1(6183c8fa7dbd32ec40c4668cab8010b5e8c49949))
	ROM_RELOAD(0xfe00, 0x0200)
	ROM_LOAD_NIB_HIGH("3g.bin", 0x0e00, 0x0200, CRC(ae7192cd) SHA1(9ba76e81b8603163c22f47f1a99da310b4325e84))
	ROM_RELOAD(0xfe00, 0x0200)
ROM_END


GAME( 1975, spirit76, 0, spirit76, spirit76, spirit76_state, 0, ROT0, "Mirco", "Spirit of 76", MACHINE_IS_SKELETON_MECHANICAL )

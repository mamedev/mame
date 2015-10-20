// license:BSD-3-Clause
// copyright-holders:David Haywood
/* BGT Fruit Machines
  BGT (British Gaming Technology) were a small Spanish company

  x86 based, not sure exactly what CPU tho

*/


#include "emu.h"
#include "cpu/nec/nec.h"
#include "cpu/i86/i86.h"


class bgt_state : public driver_device
{
public:
	bgt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;
};


static ADDRESS_MAP_START( bgt_map, AS_PROGRAM, 16, bgt_state )
	AM_RANGE(0x00000, 0x7ffff) AM_ROM
	AM_RANGE(0xf8000, 0xfffff) AM_ROM
ADDRESS_MAP_END



static ADDRESS_MAP_START( bgt_io, AS_IO, 16, bgt_state )
ADDRESS_MAP_END



static INPUT_PORTS_START( bgt )
INPUT_PORTS_END


static MACHINE_CONFIG_START( bgt, bgt_state )
	MCFG_CPU_ADD("maincpu", V30, 12000000 ) // ? unknown CPU.. definitely x86 based tho
	MCFG_CPU_PROGRAM_MAP(bgt_map)
	MCFG_CPU_IO_MAP(bgt_io)
MACHINE_CONFIG_END

ROM_START( bg_ddb )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "nkyky_0-15_5", 0x0000, 0x080000, CRC(ac4a5094) SHA1(db4eab0be63e5daddca603af290debd8e929757e) )
	ROM_RELOAD(0x80000,0x80000)

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 )
	/* there were sound roms in the 'CoinWorld Ding Dong Bells' set which might belong here, otherwise
	   roms are probably missing */
ROM_END

ROM_START( bg_barmy )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "barmyarmy_ndp16", 0x0000, 0x080000, CRC(ae488f48) SHA1(c417a3d1a79a0ca54ade2d9a4f6d70467e6c5cb4) )
	ROM_RELOAD(0x80000,0x80000)

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "barmyarmy_sound1", 0x000000, 0x080000, CRC(3530d77c) SHA1(c7a42f698090fcd9644f9929b92935cf85183d23) )
	ROM_LOAD( "barmyarmy_sound2", 0x080000, 0x080000, CRC(48d4c2f3) SHA1(71e64e3e76a55275484a7c72ce2a17232b27a4eb) )
ROM_END


ROM_START( bg_max )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "max_a_million_v014", 0x0000, 0x080000, CRC(32fe9c3b) SHA1(77519657e6e478b3cd1bf2ad2aecc6e191abe554) )
	ROM_RELOAD(0x80000,0x80000)

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 )
	/* probably missing */
ROM_END

ROM_START( bg_maxa )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "max_a_million_v114", 0x0000, 0x080000, CRC(a66851e9) SHA1(733ec52fa01615e740ebd40fba4a88efe9d9f24f) )
	ROM_RELOAD(0x80000,0x80000)

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 )
	/* probably missing */
ROM_END


GAME( 199?, bg_ddb,    0,       bgt,  bgt, driver_device,  0,  ROT0,  "BGT",    "Ding Dong Bells (BGT)",          MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, bg_barmy,  0,       bgt,  bgt, driver_device,  0,  ROT0,  "BGT",    "Barmy Army (BGT)",               MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, bg_max,    0,       bgt,  bgt, driver_device,  0,  ROT0,  "BGT",    "Max A Million (BGT) (set 1)",    MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, bg_maxa,   bg_max,  bgt,  bgt, driver_device,  0,  ROT0,  "BGT",    "Max A Million (BGT) (set 2)",    MACHINE_IS_SKELETON_MECHANICAL )

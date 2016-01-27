// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Hazel Grove Fruit Machine HW
 unknown platform! z80 based..

*/


#include "emu.h"
#include "cpu/z80/z80.h"

class haze_state : public driver_device
{
public:
	haze_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;
};



static ADDRESS_MAP_START( haze_map, AS_PROGRAM, 8, haze_state )
	AM_RANGE(0x0000, 0x17ff) AM_ROM
	AM_RANGE(0x9000, 0x9fff) AM_RAM
ADDRESS_MAP_END


static INPUT_PORTS_START( haze )
INPUT_PORTS_END


static MACHINE_CONFIG_START( haze, haze_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,2000000)         /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(haze_map)
MACHINE_CONFIG_END

ROM_START( hg_frd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fd v 3-2 a.bin", 0x0000, 0x0800, CRC(d8c276e6) SHA1(9902554d40fb1f24ea5e43f8bbfb508b3a96e90b) )
	ROM_LOAD( "fd v 3-2 b.bin", 0x0800, 0x0800, CRC(c8654bdf) SHA1(342fa389b80fb9519e3fad488cea2063e88b30fa) )
	ROM_LOAD( "fd v 3-2 c.bin", 0x1000, 0x0800, CRC(77bb8d8c) SHA1(65b7dd8024747175c3bd5bc341e2e1a92699f1c6) )
ROM_END


GAME( 198?,  hg_frd,  0,  haze,  haze, driver_device,  0,  ROT0,  "Hazel Grove",    "Fruit Deuce (Hazel Grove)",     MACHINE_IS_SKELETON_MECHANICAL)

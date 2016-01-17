// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Early Maygay HW
 I believe this is 'Triple M' or 'MMM' hardware

 Z80 based Fruit Machine
*/


#include "emu.h"
#include "cpu/z80/z80.h"

class mmm_state : public driver_device
{
public:
	mmm_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;
};



static ADDRESS_MAP_START( mmm_map, AS_PROGRAM, 8, mmm_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( mmm )
INPUT_PORTS_END


static MACHINE_CONFIG_START( mmm, mmm_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,2000000)         /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(mmm_map)
MACHINE_CONFIG_END


ROM_START( mmm_ldip )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ld1.bin", 0x0000, 0x001000, CRC(5a3c2402) SHA1(4972d309e6aabef4f9277ea851e45981d0cb3dbb) )
	ROM_LOAD( "ld2.bin", 0x1000, 0x001000, CRC(ff82643b) SHA1(0e47cdc9c0eb6f05a420d2ffeb2ebf22acbda15b) )
	ROM_LOAD( "ld3.bin", 0x2000, 0x001000, CRC(9e7158ae) SHA1(7f3b8730add127ed0608365875be3042fb2e3e7a) )
	ROM_LOAD( "ld4.bin", 0x3000, 0x001000, CRC(970b749f) SHA1(fe6da7abc699db69c0761304f588b5bed899c674) )
ROM_END


GAME( 198?,  mmm_ldip,  0,  mmm,  mmm, driver_device,  0,  ROT0,  "Maygay",    "Lucky Dip (Maygay)",    MACHINE_IS_SKELETON_MECHANICAL)

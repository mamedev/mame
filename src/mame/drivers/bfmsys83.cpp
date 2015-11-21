// license:BSD-3-Clause
// copyright-holders:David Haywood
/* BFM System 83 */

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "sound/ay8910.h"
#include "machine/bfm_comn.h"


class bfmsys83_state : public driver_device
{
public:
	bfmsys83_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu") { }

	UINT8 m_codec_data[256];
	required_device<cpu_device> m_maincpu;
};



static ADDRESS_MAP_START( memmap, AS_PROGRAM, 8, bfmsys83_state )
	AM_RANGE(0x4000, 0xffff) AM_ROM                     // 32K ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( bfmsys83 )
INPUT_PORTS_END



static MACHINE_CONFIG_START( bfmsys83, bfmsys83_state )
	MCFG_CPU_ADD("maincpu", M6802, 40000000/4)
	MCFG_CPU_PROGRAM_MAP(memmap)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd",AY8912, 40000000/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

/*
DRIVER_INIT_MEMBER(bfmsys83_state,decode)
{
    bfm_decode_mainrom(machine(),"maincpu", m_codec_data);
}
*/

ROM_START( b83catms )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "catmouse1.bin", 0xe000, 0x2000, CRC(fa2f26a1) SHA1(a85cfde6e2f14d49f627fd8c0bf2c34b331a24b5) )
	ROM_LOAD( "catmouse2.bin", 0xc000, 0x2000, CRC(51f1ad0a) SHA1(a21196553bb41a025d26fe91ead6282dfc61afe5) )
	ROM_LOAD( "catmouse3.bin", 0xa000, 0x2000, CRC(5eb5e699) SHA1(ca78a29b607ecf2367d7213e37d5894973fe2a09) )
	ROM_LOAD( "catmouse4.bin", 0x8000, 0x2000, CRC(e96f1ea7) SHA1(d7c6d0f5852e0ee56e3316840fed034ddd7bf242) )
ROM_END

ROM_START( b83cops ) // was marked as sys85, but I think this is the sys83 set?
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cops.p1", 0xe000, 0x2000, CRC(865014f3) SHA1(f1661500edf3167cd06c2248effe506bf34cb59b) )
	ROM_LOAD( "cops.p2", 0xc000, 0x2000, CRC(ab4f14fb) SHA1(56bc6aa45aeb680fbd17a0c6d1bc73523cf70588) )
	ROM_LOAD( "cops.p3", 0xa000, 0x2000, CRC(981d76e4) SHA1(18600f98d20c0d501609bafc78af8cbc366b02c7) )
	ROM_LOAD( "cops.p4", 0x8000, 0x2000, CRC(ce573b35) SHA1(f2ba22f0d55f882dd91b37e80e4bb14effd9113a) )
ROM_END

GAME( 198?, b83catms    , 0         ,  bfmsys83     , bfmsys83  , driver_device, 0  , 0,         "BFM",      "Cat & Mouse (Bellfruit) (System 83)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 198?, b83cops     , 0         ,  bfmsys83     , bfmsys83  , driver_device, 0  , 0,         "BFM",      "Cops & Robbers (Bellfruit) (System 83)", MACHINE_IS_SKELETON_MECHANICAL)

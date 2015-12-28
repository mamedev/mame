// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Awesome tossem u21 = 27c512
               u7  = 27c512  cpu
                u10-u7 and u11-u14 = 27c512 sound board

 probably http://www.highwaygames.com/arcade-machines/awesome-toss-em-7115/
*/

#include "emu.h"
#include "sound/okim6295.h"


class awetoss_state : public driver_device
{
public:
	awetoss_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	//	,m_maincpu(*this, "maincpu")
	{ }

	virtual void machine_start() override;
	virtual void machine_reset() override;

//	required_device<mcs51_cpu_device> m_maincpu;
};

static INPUT_PORTS_START( awetoss )
INPUT_PORTS_END



void awetoss_state::machine_start()
{
}

void awetoss_state::machine_reset()
{
}


static MACHINE_CONFIG_START( awetoss, awetoss_state )

	/* basic machine hardware */
//	MCFG_CPU_ADD("maincpu", ??, 8000000) // unknown
//	MCFG_CPU_PROGRAM_MAP(awetoss_map)
//	MCFG_CPU_IO_MAP(awetoss_io)
//  MCFG_CPU_VBLANK_INT_DRIVER("screen", awetoss_state,  irq0_line_hold)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH) // maybe
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



ROM_START( awetoss )
	// based on the IC positions differing I don't think this is 2 different sets?
	// both program roms look similar tho
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "AWSMTOSS.U7", 0x00000, 0x10000, CRC(2c48469c) SHA1(5ccca03d6b9cbcaddd73c8a95425f55d9e6af238) )

	ROM_REGION( 0xc0000, "oki", 0 )
	ROM_LOAD( "AWSMTOSS.U10", 0x00000, 0x10000, CRC(84c8a6b9) SHA1(26dc8c0f2098c9b0ef0e06e5dd69c897a9af69a2) )
	ROM_LOAD( "AWSMTOSS.U9S", 0x10000, 0x10000, CRC(5c7bbbd9) SHA1(89713058d03f982647217e4c6cbe37969f2537a5) )
	ROM_LOAD( "AWSMTOSS.U8S", 0x20000, 0x10000, CRC(9852e0bd) SHA1(930cca65e3f7774334dd0513a261f874f94886ac) )
	ROM_LOAD( "AWSMTOSS.U7S", 0x30000, 0x10000, CRC(32fa11f5) SHA1(70914eac64f53bcb07c0eb9fcc1b4fbeab2fc453) )

	ROM_REGION( 0x10000, "maincpu2", 0 )
	ROM_LOAD( "AWSMTOSS.U21", 0x00000, 0x10000, CRC(2b66d952) SHA1(b95f019d007cbd1f0325c33ffd1208f2afa6b996) )

	ROM_REGION( 0xc0000, "oki2", 0 )
	ROM_LOAD( "AWSMTOSS.U14", 0x00000, 0x10000, CRC(6217daaf) SHA1(3036e7f941f787374ef130d3ae6d57813d9e9aac) )
	ROM_LOAD( "AWSMTOSS.U13", 0x10000, 0x10000, CRC(4ed3c827) SHA1(761d2796d4f40deeb2caa61c4a9c56ced156084b) )
	ROM_LOAD( "AWSMTOSS.U12", 0x20000, 0x10000, CRC(9ddf6dd9) SHA1(c115828ab261ae6d83cb500057313c3a5570b4b0) )
	ROM_LOAD( "AWSMTOSS.U11", 0x30000, 0x10000, CRC(8ae9d4f0) SHA1(58d1d8972c8e4c9a7c63e9d63e267ea81515d22a) )	
ROM_END

GAME( 19??, awetoss,  0,    awetoss, awetoss, driver_device,  0, ROT0, "Lazer-tron", "Awesome Toss'em (Lazer-tron)", MACHINE_IS_SKELETON_MECHANICAL )


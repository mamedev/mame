/* Unknown Gambling game with 186 CPU */

/*

80186xl20   Xtal 40Mhz
At89c52 (not dumped) with external 32K ram?? Xtal 11.xxx18Mhz
Cirrus Logic CL-GD5428-80QC-A video chip with 416c256 ram near it Xtal 14.x818 Mhaz

U9-U10 programm rom common to both pcb
u11-u12 Program rom/GFX
U13-U14 256Kramx8 (32Kbyte x16)

U23 Nec D71055C
U28 Nec D71055C
U22 Nec D71055C
U42 Nec D71051C
U500 Nec D71051C (not present on board "1")

U3 Max691cpe

U300 Nec D7759GC (10Mhz xtal near it)


code doesn't make much sense, wrong mapping? bad?

*/



#include "emu.h"
#include "cpu/i86/i186.h"


class gambl186_state : public driver_device
{
public:
	gambl186_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu") { }

	UINT32 screen_update_gambl186(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
	{
		return 0;
	}
	required_device<cpu_device> m_maincpu;
};



static ADDRESS_MAP_START( gambl186_map, AS_PROGRAM, 16, gambl186_state )
	AM_RANGE(0x80000, 0xfffff) AM_ROM AM_REGION("user1",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( gambl186_io, AS_IO, 16, gambl186_state )
ADDRESS_MAP_END



static INPUT_PORTS_START( gambl186 )
INPUT_PORTS_END



static MACHINE_CONFIG_START( gambl186, gambl186_state )
	MCFG_CPU_ADD("maincpu", I80186, XTAL_40MHz/2)
	MCFG_CPU_PROGRAM_MAP(gambl186_map)
	MCFG_CPU_IO_MAP(gambl186_io)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 511, 0, 255)
	MCFG_SCREEN_UPDATE_DRIVER(gambl186_state, screen_update_gambl186)

	MCFG_PALETTE_ADD("palette", 0x100)

MACHINE_CONFIG_END




ROM_START( gambl186 )
	ROM_REGION( 0x100000, "user1", 0 )
	ROM_LOAD16_BYTE( "ie398.u11", 0x00000, 0x80000, CRC(86ad7cab) SHA1(b701c3701db630d218a9b1700f216f795a1b1272) )
	ROM_LOAD16_BYTE( "io398.u12", 0x00001, 0x80000, CRC(0a036f34) SHA1(63d0b87c7d4c902413f28c0b55d78e5fda511f4f) )

	ROM_REGION( 0x40000, "vidbios", 0 )
	ROM_LOAD16_BYTE( "se403p.u9",  0x00000, 0x20000, CRC(1021cc20) SHA1(d9bb67676b05458ff813d608431ff06946ab7721) )
	ROM_LOAD16_BYTE( "so403p.u10", 0x00001, 0x20000, CRC(af9746c9) SHA1(3f1ab8110cc5eadec661181779799693ad695e21) )

	ROM_REGION( 0x200000, "snd", 0 )
	ROM_LOAD( "347.u302", 0x00000, 0x20000, CRC(7ce8f490) SHA1(2f856e31d189e9d46ba6b322133d99133e0b52ac) )
ROM_END

ROM_START( gambl186a )
	ROM_REGION( 0x100000, "user1", 0 )
	ROM_LOAD16_BYTE( "ie399.u11", 0x00000, 0x80000, CRC(2a7bce20) SHA1(fbabaaa0d72b5dfccd33f5194d13009bdc44b5a7) )
	ROM_LOAD16_BYTE( "io399.u12", 0x00001, 0x80000, CRC(9212f52b) SHA1(d970c59c1e0f5f7e94c1b632398bcfae278c143d) )

	ROM_REGION( 0x40000, "vidbios", 0 )
	ROM_LOAD16_BYTE( "se403p.u9",  0x00000, 0x20000, CRC(1021cc20) SHA1(d9bb67676b05458ff813d608431ff06946ab7721) )
	ROM_LOAD16_BYTE( "so403p.u10", 0x00001, 0x20000, CRC(af9746c9) SHA1(3f1ab8110cc5eadec661181779799693ad695e21) )

	ROM_REGION( 0x200000, "snd", 0 )
	ROM_LOAD( "347.u302", 0x00000, 0x20000, CRC(7ce8f490) SHA1(2f856e31d189e9d46ba6b322133d99133e0b52ac) ) // xxx.u302
ROM_END


GAME( 199?, gambl186,  0,        gambl186,   gambl186, driver_device,   0,       ROT0,  "<unknown>", "unknown 186 based gambling game (V398)",         GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 199?, gambl186a, gambl186, gambl186,   gambl186, driver_device,   0,       ROT0,  "<unknown>", "unknown 186 based gambling game (V399)",         GAME_NOT_WORKING | GAME_NO_SOUND )

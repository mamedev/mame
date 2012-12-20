/************************************************************************************************************

   Bowling Try

   (c)200? Atlus


   ATLUS PCB  BT-208001
   ------------------------

   At U12 the chip is Toshiba TA8428FG
 
   At U1 the chip is H8/3008
 
   At X1 on the crystal it is printed S753
 
   big gfx chip marked

   YAMAHA JAPAN
   YGV631-B
   0806LU004

************************************************************************************************************/


#include "emu.h"
#include "cpu/h83002/h8.h"


class bowltry_state : public driver_device
{
public:
	bowltry_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

	UINT32 screen_update_bowltry(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	required_device<cpu_device> m_maincpu;
public:
};

static ADDRESS_MAP_START( bowltry_map, AS_PROGRAM, 16, bowltry_state )
	AM_RANGE( 0x000000, 0x07ffff ) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static INPUT_PORTS_START( bowltry )
INPUT_PORTS_END

UINT32 bowltry_state::screen_update_bowltry(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}



static MACHINE_CONFIG_START( bowltry, bowltry_state )
	MCFG_CPU_ADD("maincpu", H83002, 16000000 ) // H83008 (!)
	MCFG_CPU_PROGRAM_MAP( bowltry_map )

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(bowltry_state, screen_update_bowltry)

	MCFG_PALETTE_LENGTH(0x200)

	/* tt5665 sound */

MACHINE_CONFIG_END

ROM_START( bowltry )
	ROM_REGION( 0x080000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "u30_v1.00.u30", 0x000000, 0x080000, CRC(2bd47419) SHA1(8fc975340e47ddeedf96e454a6c5372328f28b72) )	
	
	ROM_REGION( 0x800000, "gfx", 0 )
	ROM_LOAD16_BYTE( "u27_v1.00.u27", 0x000000, 0x400000, CRC(80f51c25) SHA1(53c21325e7796197c26ca0cf4f8e51bf1e0bdcd3) )
	ROM_LOAD16_BYTE( "u28_v1.00.u28", 0x000001, 0x400000, CRC(9cc8b577) SHA1(6ef5cbb83860f88c9c83d4410034c5b528b2138b) )
	
	ROM_REGION( 0x400000, "tt5665", 0 )
	ROM_LOAD( "u24_v1.00.u24", 0x000000, 0x400000, CRC(4e082d58) SHA1(d2eb58bc3d8ade2ea556960013d580f0fb952090) )	
ROM_END


GAME( 200?, bowltry,	0,			bowltry,  bowltry, driver_device,  0, ROT0, "Atlus",        "Bowling Try",GAME_IS_SKELETON )

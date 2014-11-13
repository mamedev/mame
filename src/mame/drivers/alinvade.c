/*

 tiny bartop b&w Space Invaders type game with colour overlay
 
 does it use any off-the shelf chips in addition to the 6502?


*/

#include "emu.h"
#include "cpu/m6502/m6502.h"

class alinvade_state : public driver_device
{
public:
	alinvade_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }


public:
	virtual void machine_start();
	virtual void machine_reset();
	UINT32 screen_update_alinvade(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};



static ADDRESS_MAP_START( alinvade_map, AS_PROGRAM, 8, alinvade_state )
	AM_RANGE(0x0000, 0x0fff) AM_RAM	
	AM_RANGE(0xe000, 0xe3ff) AM_ROM
	AM_RANGE(0xe800, 0xebff) AM_RAM	
	AM_RANGE(0xec00, 0xffff) AM_ROM


ADDRESS_MAP_END


static INPUT_PORTS_START( alinvade )
INPUT_PORTS_END



void alinvade_state::machine_start()
{
}

void alinvade_state::machine_reset()
{
}

UINT32 alinvade_state::screen_update_alinvade(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}


static MACHINE_CONFIG_START( alinvade, alinvade_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502,2000000)         /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(alinvade_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", alinvade_state,  irq0_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 16, 256-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(alinvade_state, screen_update_alinvade)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
MACHINE_CONFIG_END



ROM_START( alinvade )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "alien28.708", 0xe000, 0x0400, CRC(de376295) SHA1(e8eddbb1be1f8661c6b5b39c0d78a65bded65db2) )
	ROM_LOAD( "alien29.708", 0xec00, 0x0400, CRC(20212977) SHA1(9d24a6b403d968267079fa6241545bd5a01afebb) )
	ROM_LOAD( "alien30.708", 0xf000, 0x0400, CRC(734b691c) SHA1(9e562159061eecf4b1dee4ea0ee4752c901a54aa) )
	ROM_LOAD( "alien31.708", 0xf400, 0x0400, CRC(5a70535c) SHA1(2827e7d4bffca78bd035da04481e1e972ee2da39) )
	ROM_LOAD( "alien32.708", 0xf800, 0x0400, CRC(332dd234) SHA1(9974668344a2a351868a9e7757d1c3a497dc5621) )
	ROM_LOAD( "alien33.708", 0xfc00, 0x0400, CRC(e0d57fc7) SHA1(7b8ddcb4a86811592d2d0bbc61b2f19e5caa9ccc) )
ROM_END


GAME( 198?, alinvade,  0,    alinvade, alinvade, driver_device,  0, ROT0, "Forbes?", "Alien Invaders", GAME_NOT_WORKING )

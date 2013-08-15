/***************************************************************************

	Sharp Zaurus PDA skeleton driver (SL, ARM/Linux based, 4th generation)

	TODO:
	- PXA-255 ID opcode fails on this
	- ARM TLB look-up errors
	- Dumps are questionable to say the least

***************************************************************************/


#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "machine/pxa255.h"

#define MAIN_CLOCK XTAL_8MHz

class zaurus_state : public driver_device
{
public:
	zaurus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_ram(*this, "ram")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT32> m_ram;

	// screen updates
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
	virtual void palette_init();
};

void zaurus_state::video_start()
{
}

UINT32 zaurus_state::screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{


	return 0;
}

static ADDRESS_MAP_START( zaurus_map, AS_PROGRAM, 32, zaurus_state )
	AM_RANGE(0x00000000, 0x001fffff) AM_RAM AM_REGION("firmware", 0)
	AM_RANGE(0xa0000000, 0xa07fffff) AM_RAM AM_SHARE("ram")
ADDRESS_MAP_END


static INPUT_PORTS_START( zaurus )
INPUT_PORTS_END



void zaurus_state::machine_start()
{
}

void zaurus_state::machine_reset()
{
}


void zaurus_state::palette_init()
{
}

// TODO: main CPU differs greatly between versions!
static MACHINE_CONFIG_START( zaurus, zaurus_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",PXA255,MAIN_CLOCK)
	MCFG_CPU_PROGRAM_MAP(zaurus_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DRIVER(zaurus_state, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)

	MCFG_PALETTE_LENGTH(8)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
//  MCFG_SOUND_ADD("aysnd", AY8910, MAIN_CLOCK/4)
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

/* was labeled SL-C500 */
ROM_START( zsl5500 )
	ROM_REGION32_LE( 0x200000, "firmware", ROMREGION_ERASE00 )
    ROM_LOAD( "sl-c500 v1.20 (zimage).bin", 0x000000, 0x13c000, BAD_DUMP CRC(dc1c259f) SHA1(8150744196a72821ae792462d0381182274c2ce0) )
ROM_END

ROM_START( zsl5600 )
	ROM_REGION32_LE( 0x200000, "firmware", ROMREGION_ERASE00 )
	ROM_LOAD( "zaurus sl-b500 - 5600 (zimage).bin", 0x000000, 0x11b6b0, BAD_DUMP CRC(779c70a1) SHA1(26824e3dc563b681f195029f220dfaa405613f9e) )
ROM_END

ROM_START( zslc750 )
	ROM_REGION32_LE( 0x200000, "firmware", ROMREGION_ERASE00 )
    ROM_LOAD( "zaurus sl-c750 (zimage).bin", 0x000000, 0x121544, BAD_DUMP CRC(56353f4d) SHA1(8e1fff6e93d560bd6572c5c163bbd81378693f68) )
ROM_END

ROM_START( zslc760 )
	ROM_REGION32_LE( 0x200000, "firmware", ROMREGION_ERASE00 )
	ROM_LOAD( "zaurus sl-c760 (zimage).bin", 0x000000, 0x120b44, BAD_DUMP CRC(feedcba3) SHA1(1821ad0fc03a8c3832ad5fe2221c21c1ca277508) )
ROM_END

ROM_START( zslc3000 )
	ROM_REGION32_LE( 0x200000, "firmware", ROMREGION_ERASE00 )
    ROM_LOAD( "openzaurus 3.5.3 - zimage-sharp sl-c3000-20050428091110.bin", 0x000000, 0x12828c, BAD_DUMP CRC(fd94510d) SHA1(901f8154b4228a448f5551f0c9f21c2153e1e3a1) )
ROM_END

ROM_START( zslc1000 )
	ROM_REGION32_LE( 0x200000, "firmware", ROMREGION_ERASE00 )
    ROM_LOAD( "openzaurus 3.5.3 - zimage-sharp sl-c1000-20050427214434.bin", 0x000000, 0x128980, BAD_DUMP  CRC(1e1a9279) SHA1(909ac3f00385eced55822d6a155b79d9d25f43b3) )
ROM_END

GAME( 2002, zsl5500,  0,   zaurus,  zaurus, driver_device,  0,       ROT0, "Sharp",      "Zaurus SL-5500 \"Collie\"", GAME_IS_SKELETON )
GAME( 2002, zsl5600,  0,   zaurus,  zaurus, driver_device,  0,       ROT0, "Sharp",      "Zaurus SL-5600 / SL-B500 \"Poodle\"", GAME_IS_SKELETON )
GAME( 2003, zslc750,  0,   zaurus,  zaurus, driver_device,  0,       ROT0, "Sharp",      "Zaurus SL-C750 \"Shepherd\" (Japan)", GAME_IS_SKELETON )
GAME( 2004, zslc760,  0,   zaurus,  zaurus, driver_device,  0,       ROT0, "Sharp",      "Zaurus SL-C760 \"Husky\" (Japan)", GAME_IS_SKELETON )
GAME( 200?, zslc3000, 0,   zaurus,  zaurus, driver_device,  0,       ROT0, "Sharp",      "Zaurus SL-C3000 \"Spitz\" (Japan)", GAME_IS_SKELETON )
GAME( 200?, zslc1000, 0,   zaurus,  zaurus, driver_device,  0,       ROT0, "Sharp",      "Zaurus SL-C3000 \"Akita\" (Japan)", GAME_IS_SKELETON )

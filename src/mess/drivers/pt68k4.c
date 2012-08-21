/***************************************************************************

        Peripheral Technology PT68K4

        03/01/2011 Skeleton driver.

This has the appearance of a PC, including pc power supply, slots, etc
on a conventional pc-like motherboard and case.

Some pics: http://www.wormfood.net/old_computers/

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"

class pt68k4_state : public driver_device
{
public:
	pt68k4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_p_ram(*this, "p_ram"){ }

	required_shared_ptr<UINT16> m_p_ram;
};

static ADDRESS_MAP_START(pt68k4_mem, AS_PROGRAM, 16, pt68k4_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x0007ffff) AM_RAM AM_SHARE("p_ram") // 512 KB RAM / ROM at boot
	AM_RANGE(0x00f00000, 0x00f0ffff) AM_ROM AM_MIRROR(0xf0000) AM_REGION("user1", 0)
	AM_RANGE(0x00ff0000, 0x00ffffff) AM_RAM
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( pt68k4 )
INPUT_PORTS_END


static MACHINE_RESET(pt68k4)
{
	pt68k4_state *state = machine.driver_data<pt68k4_state>();
	UINT8* user1 = state->memregion("user1")->base();

	memcpy((UINT8*)state->m_p_ram.target(), user1, 8);

	machine.device("maincpu")->reset();
}

static VIDEO_START( pt68k4 )
{
}

static SCREEN_UPDATE_IND16( pt68k4 )
{
	return 0;
}

static MACHINE_CONFIG_START( pt68k4, pt68k4_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M68000, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(pt68k4_mem)

	MCFG_MACHINE_RESET(pt68k4)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_VIDEO_START(pt68k4)
	MCFG_SCREEN_UPDATE_STATIC(pt68k4)
	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(black_and_white)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( pt68k4 )
	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "humbug", "Humbug" )
	ROMX_LOAD( "humpta40.bin", 0x0001, 0x8000, CRC(af67ff64) SHA1(da9fa31338c6847bb0e66118679b1ec01f6dc30b), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "humpta41.bin", 0x0000, 0x8000, CRC(a8b16e27) SHA1(218802f6e20d14cff736bb7423f06ce2f66e074c), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "monk", "Monk" )
	ROMX_LOAD( "monk_0.bin", 0x0001, 0x8000, CRC(420d6a4b) SHA1(fca8c53c9c3c8ebd09370499cf34f4cc75ed9463), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "monk_1.bin", 0x0000, 0x8000, CRC(fc495e82) SHA1(f7b720d87db4d72a23e6c42d2cdd03216db04b60), ROM_SKIP(1) | ROM_BIOS(2))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY             FULLNAME       FLAGS */
COMP( 1990, pt68k4,  0,       0,     pt68k4,    pt68k4, driver_device,  0,  "Peripheral Technology", "PT68K4", GAME_NOT_WORKING | GAME_NO_SOUND)

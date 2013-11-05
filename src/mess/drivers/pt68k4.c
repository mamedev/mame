// license:MAME
// copyright-holders:Robbbert
/***************************************************************************

    Peripheral Technology PT68K4

    2011-01-03 Skeleton driver.
    2013-09-30 Connected to a terminal

This has the appearance of a PC, including pc power supply, slots, etc
on a conventional pc-like motherboard and case.

Some pics: http://www.wormfood.net/old_computers/

Note: bios 0 works with the terminal. When first started, press Enter
      to get the logo. Enter HE to get a list of commands. Terminate
      numeric entries with a Space (not Enter!).

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/terminal.h"

class pt68k4_state : public driver_device
{
public:
	pt68k4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_p_base(*this, "rambase")
		, m_p_upper(*this, "upper_ram")
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, TERMINAL_TAG)
	{ }

	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_READ16_MEMBER(status_r);
	DECLARE_READ16_MEMBER(nop_r);
	//virtual void video_start();
	//UINT32 screen_update_pt68k4(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
private:
	virtual void machine_reset();
	required_shared_ptr<UINT16> m_p_base;
	required_shared_ptr<UINT16> m_p_upper;
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
};

static ADDRESS_MAP_START(pt68k4_mem, AS_PROGRAM, 16, pt68k4_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x07ffff) AM_RAM AM_SHARE("rambase") // 512 KB RAM / ROM at boot
	AM_RANGE(0xf80000, 0xf8ffff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0xfe0002, 0xfe0003) AM_READ(status_r)
	AM_RANGE(0xfe0006, 0xfe0007) AM_DEVWRITE8(TERMINAL_TAG, generic_terminal_device, write, 0x00ff)
	AM_RANGE(0xfe0012, 0xfe0013) AM_READ(nop_r)
	AM_RANGE(0xff0000, 0xffffff) AM_RAM AM_SHARE("upper_ram")
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( pt68k4 )
INPUT_PORTS_END


void pt68k4_state::machine_reset()
{
	UINT8* user1 = memregion("roms")->base();
	memcpy((UINT8*)m_p_base.target(), user1, 8);

	m_maincpu->reset();
}

//void pt68k4_state::video_start()
//{
//}

//UINT32 pt68k4_state::screen_update_pt68k4(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
//{
//  return 0;
//}

READ16_MEMBER( pt68k4_state::status_r )
{
	return 0x8c;
}

READ16_MEMBER( pt68k4_state::nop_r )
{
	return 0;
}

WRITE8_MEMBER( pt68k4_state::kbd_put )
{
	m_p_upper[0x64f] = 0x100 | data;
}

static GENERIC_TERMINAL_INTERFACE( terminal_intf )
{
	DEVCB_DRIVER_MEMBER(pt68k4_state, kbd_put)
};

static MACHINE_CONFIG_START( pt68k4, pt68k4_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M68000, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(pt68k4_mem)

	/* video hardware */
	MCFG_GENERIC_TERMINAL_ADD(TERMINAL_TAG, terminal_intf)
	//MCFG_SCREEN_ADD("screen", RASTER)
	//MCFG_SCREEN_REFRESH_RATE(50)
	//MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	//MCFG_SCREEN_SIZE(640, 480)
	//MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	//MCFG_SCREEN_UPDATE_DRIVER(pt68k4_state, screen_update_pt68k4)
	//MCFG_PALETTE_LENGTH(2)
	//MCFG_PALETTE_INIT_OVERRIDE(driver_device, black_and_white)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( pt68k4 )
	ROM_REGION16_BE( 0x10000, "roms", 0 )
	ROM_SYSTEM_BIOS( 0, "humbug", "Humbug" )
	ROMX_LOAD( "humpta40.bin", 0x0000, 0x8000, CRC(af67ff64) SHA1(da9fa31338c6847bb0e66118679b1ec01f6dc30b), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "humpta41.bin", 0x0001, 0x8000, CRC(a8b16e27) SHA1(218802f6e20d14cff736bb7423f06ce2f66e074c), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "monk", "Monk" )
	ROMX_LOAD( "monk_0.bin", 0x0000, 0x8000, CRC(420d6a4b) SHA1(fca8c53c9c3c8ebd09370499cf34f4cc75ed9463), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "monk_1.bin", 0x0001, 0x8000, CRC(fc495e82) SHA1(f7b720d87db4d72a23e6c42d2cdd03216db04b60), ROM_SKIP(1) | ROM_BIOS(2))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT   CLASS          INIT     COMPANY             FULLNAME       FLAGS */
COMP( 1990, pt68k4,  0,       0,     pt68k4,    pt68k4, driver_device,  0,  "Peripheral Technology", "PT68K4", GAME_NOT_WORKING | GAME_NO_SOUND)

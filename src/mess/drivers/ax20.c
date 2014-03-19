// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    Axel AX-20

    27/12/2011 Skeleton driver.

    Hardware description:
    - CPU: I8088
    - FDC: I8272
    - PIT: I8253
    - PIC: I8259

    Also marketed under the Matra brand as MAX-20 ("M" for Matra ?)

****************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "imagedev/flopdrv.h"
#include "formats/basicdsk.h"

class ax20_state : public driver_device
{
public:
	ax20_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_p_vram(*this, "p_vram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	required_device<cpu_device> m_maincpu;

	virtual void machine_start();
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_shared_ptr<UINT8> m_p_vram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


UINT32 ax20_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for ( int y = 0; y < 24; y++ )
	{
		for ( int x = 0; x < 80; x++ )
		{
			UINT16 tile = m_p_vram[24 +  y * 128 + x ] & 0x7f;

			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect, tile, 0, 0, 0, x*8, y*12);
		}
	}

	return 0;
}


static ADDRESS_MAP_START(ax20_map, AS_PROGRAM, 8, ax20_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000,0x1ffff) AM_RAM
	AM_RANGE(0x20000,0x3ffff) AM_RAM //optional RAM
	AM_RANGE(0xf0400,0xf0fff) AM_RAM AM_SHARE("p_vram")
	AM_RANGE(0xff800,0xfffff) AM_ROM AM_REGION("ipl", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(ax20_io, AS_IO, 8, ax20_state)
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( ax20 )
INPUT_PORTS_END


void ax20_state::machine_start()
{
}

static const gfx_layout ax20_charlayout =
{
	8, 12,
	128,
	1,
	{ 0 },
	{ 4*8+7, 4*8+6, 4*8+5, 4*8+4, 4*8+3, 4*8+2, 4*8+1, 4*8+0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 8*9, 8*10, 8*11 },
	8*16
};

static GFXDECODE_START( ax20 )
	GFXDECODE_ENTRY( "chargen", 0x0000, ax20_charlayout, 0, 1 )
GFXDECODE_END

static const floppy_interface ax20_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSDD_40, // TODO
	LEGACY_FLOPPY_OPTIONS_NAME(default),
	NULL,
	NULL
};

static MACHINE_CONFIG_START( ax20, ax20_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8088, XTAL_14_31818MHz/3)
	MCFG_CPU_PROGRAM_MAP(ax20_map)
	MCFG_CPU_IO_MAP(ax20_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(ax20_state, screen_update)
	MCFG_SCREEN_SIZE(80*8, 24*12)
	MCFG_SCREEN_VISIBLE_AREA(0, 80*8-1, 0, 24*12-1)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ax20)
	MCFG_PALETTE_ADD_MONOCHROME_GREEN("palette")

	/* Devices */
	MCFG_LEGACY_FLOPPY_DRIVE_ADD(FLOPPY_0, ax20_floppy_interface)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( ax20 )
	ROM_REGION( 0x0800, "ipl", 0 )
	ROM_LOAD( "ax20-s.rom", 0x0000, 0x0800, CRC(f11f95b9) SHA1(59949332dd431fcf8211c2d556e1f49351e90750))

	ROM_REGION( 0x4000, "chargen", 0 )
	ROM_LOAD( "ax20-g.rom", 0x0000, 0x0800, CRC(90bcef80) SHA1(922067fd7316de9e69b9600c793ada5c87197eeb))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY           FULLNAME       FLAGS */
COMP( 1982, ax20,  0,      0,       ax20,     ax20, driver_device,    0,     "Axel",   "AX-20", GAME_NOT_WORKING | GAME_NO_SOUND)

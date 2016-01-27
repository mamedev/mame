// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    Jonos Escort

    2013-09-12 Skeleton driver

    This computer is some sort of portable Z80-based system.

    Haven't found any info.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"


class jonos_state : public driver_device
{
public:
	jonos_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_p_videoram(*this, "videoram")
		, m_maincpu(*this, "maincpu")
	{ }

	DECLARE_DRIVER_INIT(jonos);
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_shared_ptr<UINT8> m_p_videoram;
private:
	const UINT8 *m_p_chargen;
	virtual void machine_reset() override;
	virtual void video_start() override;
	required_device<cpu_device> m_maincpu;
};



static ADDRESS_MAP_START(jonos_mem, AS_PROGRAM, 8, jonos_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0x1000, 0x17ff) AM_RAM
	AM_RANGE(0x1800, 0x1fff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x2000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( jonos_io, AS_IO, 8, jonos_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( jonos )
INPUT_PORTS_END

void jonos_state::machine_reset()
{
}

void jonos_state::video_start()
{
	m_p_chargen = memregion("chargen")->base();
}

UINT32 jonos_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 y,ra,chr,gfx;
	UINT16 sy=0,ma=0,x;

	for (y = 0; y < 25; y++)
	{
		for (ra = 0; ra < 12; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 80; x++)
			{
				chr = m_p_videoram[x];

				if (ra < 8)
					gfx = m_p_chargen[(chr<<3) | ra ];
				else
					gfx = m_p_chargen[(chr<<3) | (ra&7) | 0x400];

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 0);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 7);
			}
		}
		ma+=80;
	}
	return 0;
}

/* F4 Character Displayer */
static const gfx_layout jonos_charlayout =
{
	8, 12,                   /* 8 x 12 characters */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 1024*8, 1025*8, 1026*8, 1027*8 },
	8*8                    /* every char takes 8 bytes */
};

static GFXDECODE_START( jonos )
	GFXDECODE_ENTRY( "chargen", 0x0000, jonos_charlayout, 0, 1 )
GFXDECODE_END


static MACHINE_CONFIG_START( jonos, jonos_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_16MHz / 4)
	MCFG_CPU_PROGRAM_MAP(jonos_mem)
	MCFG_CPU_IO_MAP(jonos_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(jonos_state, screen_update)
	MCFG_SCREEN_SIZE(640, 300)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 299)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", jonos)
	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")
MACHINE_CONFIG_END

DRIVER_INIT_MEMBER(jonos_state,jonos)
{
}

/* ROM definition */
ROM_START( jonos )
	ROM_REGION( 0x1000, "roms", 0 )
	ROM_LOAD( "jocrts04.rom", 0x0000, 0x1000, CRC(6a3d4048) SHA1(bdb0bc2c8c4e54261376e4ea3c2827d00d3d89bc) )

	ROM_REGION( 0x0800, "user1", 0 ) // Z80 code for serial & parallel ports
	ROM_LOAD( "joz80.rom",    0x0000, 0x0800, CRC(de1e8998) SHA1(270df08caf30cc8f18e740ef05dc8727a925a5da) )

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "jochset0.rom", 0x0000, 0x0800, CRC(1d8e9640) SHA1(74f3604acc71f9bc1e1f9479f6438feda79293a2) )
ROM_END

/* Driver */

/*   YEAR   NAME    PARENT  COMPAT   MACHINE  INPUT  CLASS            INIT       COMPANY   FULLNAME       FLAGS */
COMP( 198?, jonos,  0,      0,       jonos,   jonos, jonos_state,    jonos,     "Jonos", "Escort", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)

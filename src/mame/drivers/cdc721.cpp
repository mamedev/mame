// license:BSD-3-Clause
// copyright-holders:Robbbert
/************************************************************************************************************

Control Data Corporation CDC 721 Terminal (Viking)

2013-08-13 Skeleton


*************************************************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"

class cdc721_state : public driver_device
{
public:
	cdc721_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	, m_p_videoram(*this, "videoram")
	, m_maincpu(*this, "maincpu")
	{ }

public:
	virtual void machine_reset() override;
	DECLARE_PALETTE_INIT(cdc721);
	const UINT8 *m_p_chargen;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_shared_ptr<UINT8> m_p_videoram;
private:
	required_device<cpu_device> m_maincpu;


};

static ADDRESS_MAP_START( cdc721_mem, AS_PROGRAM, 8, cdc721_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x4000, 0xf097) AM_RAM
	AM_RANGE(0xf098, 0xffff) AM_RAM AM_SHARE("videoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( cdc721_io, AS_IO, 8, cdc721_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

static INPUT_PORTS_START( cdc721 )
INPUT_PORTS_END

void cdc721_state::machine_reset()
{
	m_p_chargen = memregion("chargen")->base();
}

/* F4 Character Displayer */
static const gfx_layout cdc721_charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	/* y offsets */
	{ 0*8, 0x100*8, 0x200*8, 0x300*8, 0x400*8, 0x500*8, 0x600*8, 0x700*8,
	0x800*8, 0x900*8, 0xa00*8, 0xb00*8, 0xc00*8, 0xd00*8, 0xe00*8, 0xf00*8 },
	8                   /* every char takes 16 x 1 bytes */
};

static GFXDECODE_START( cdc721 )
	GFXDECODE_ENTRY( "chargen", 0x0000, cdc721_charlayout, 0, 1 )
GFXDECODE_END

PALETTE_INIT_MEMBER( cdc721_state, cdc721 )
{
	palette.set_pen_color(0, 0, 0, 0 ); /* Black */
	palette.set_pen_color(1, 0, 255, 0 );   /* Full */
	palette.set_pen_color(2, 0, 128, 0 );   /* Dimmed */
}

UINT32 cdc721_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 y,ra,chr,gfx;
	UINT16 sy=0,ma=0,x;

	for (y = 0; y < 24; y++)
	{
		for (ra = 3; ra < 13; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

			for (x = 0; x < 160; x+=2)
			{
				chr = m_p_videoram[x+ma];

				gfx = m_p_chargen[chr | (ra << 8) ];

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
		ma+=160;
	}
	return 0;
}

static MACHINE_CONFIG_START( cdc721, cdc721_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(cdc721_mem)
	MCFG_CPU_IO_MAP(cdc721_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(cdc721_state, screen_update)
	MCFG_SCREEN_SIZE(640, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 239)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_PALETTE_ADD("palette", 3)
	MCFG_PALETTE_INIT_OWNER(cdc721_state, cdc721)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", cdc721)
MACHINE_CONFIG_END

ROM_START( cdc721 )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "66315359", 0x0000, 0x2000, CRC(20ff3eb4) SHA1(5f15cb14893d75a46dc66d3042356bb054d632c2) )
	ROM_LOAD( "66315361", 0x2000, 0x2000, CRC(21d59d09) SHA1(9c087537d68c600ddf1eb9b009cf458231c279f4) )
	ROM_LOAD( "66315360", 0x4000, 0x1000, CRC(feaa0fc5) SHA1(f06196553a1f10c07b2f7e495823daf7ea26edee) ) // rom @ 4000
	ROM_LOAD( "66307828", 0x5000, 0x1000, CRC(ac97136f) SHA1(0d280e1aa4b9502bd390d260f83af19bf24905cd) ) // keyboard

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "66315039", 0x0000, 0x1000, CRC(5c9aa968) SHA1(3ec7c5f25562579e6ed3fda7562428ff5e6b9550) ) // chargen
ROM_END

COMP( 1981, cdc721, 0, 0, cdc721, cdc721, driver_device, 0, "Control Data Corporation",  "CDC721 Terminal", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )

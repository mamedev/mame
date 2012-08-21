/***************************************************************************

        VTA-2000 Terminal

            board images : http://fotki.yandex.ru/users/lodedome/album/93699?p=0

        BDP-15 board only

        29/11/2010 Skeleton driver.

Better known on the net as BTA2000-15m.
It is a green-screen terminal, using RS232, and supposedly VT100 compatible.
The top line is a status line.

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"

class vta2000_state : public driver_device
{
public:
	vta2000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu")
	{ }

	required_device<cpu_device> m_maincpu;
	const UINT8 *m_p_chargen;
	const UINT8 *m_p_videoram;
};

static ADDRESS_MAP_START(vta2000_mem, AS_PROGRAM, 8, vta2000_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x5fff ) AM_ROM
	AM_RANGE( 0x8000, 0xffff ) AM_RAM AM_REGION("maincpu", 0x8000)
ADDRESS_MAP_END

static ADDRESS_MAP_START(vta2000_io, AS_IO, 8, vta2000_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( vta2000 )
INPUT_PORTS_END


static MACHINE_RESET(vta2000)
{
}

static VIDEO_START( vta2000 )
{
	vta2000_state *state = machine.driver_data<vta2000_state>();
	state->m_p_chargen = machine.root_device().memregion("chargen")->base();
	state->m_p_videoram = state->memregion("maincpu")->base()+0x8000;
}

static SCREEN_UPDATE_IND16( vta2000 )
/* Cursor is missing. */
{
	vta2000_state *state = screen.machine().driver_data<vta2000_state>();
	static UINT8 framecnt=0;
	UINT8 y,ra,gfx,attr,fg,bg;
	UINT16 sy=0,ma=0,x,xx=0,chr;

	framecnt++;

	for (y = 0; y < 25; y++)
	{
		for (ra = 0; ra < 12; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

			xx = ma << 1;
			for (x = ma; x < ma + 80; x++)
			{
				chr = state->m_p_videoram[xx++];
				attr = state->m_p_videoram[xx++];

				if ((chr & 0x60)==0x60)
					chr+=256;

				gfx = state->m_p_chargen[(chr<<4) | ra ];
				bg = 0;

				/* Process attributes */
				if (BIT(attr, 4))
				{
					gfx ^= 0xff; // reverse video
					bg = 2;
				}
				if (BIT(attr, 0))
					fg = 2; // highlight
				else
					fg = 1;
				if ((BIT(attr, 1)) && (BIT(framecnt, 5)))
					gfx = 0; // blink
				if ((BIT(attr, 5)) && (ra == 10))
				{
					gfx = 0xff; // underline
					fg = 2;
				}

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7) ? fg : bg;
				*p++ = BIT(gfx, 6) ? fg : bg;
				*p++ = BIT(gfx, 5) ? fg : bg;
				*p++ = BIT(gfx, 4) ? fg : bg;
				*p++ = BIT(gfx, 3) ? fg : bg;
				*p++ = BIT(gfx, 2) ? fg : bg;
				*p++ = BIT(gfx, 1) ? fg : bg;
				*p++ = BIT(gfx, 0) ? fg : bg;
			}
		}
		if (y)
			ma+=132;
		else
			ma+=80;
	}
	return 0;
}


/* F4 Character Displayer */
static const gfx_layout vta2000_charlayout =
{
	8, 16,					/* 8 x 16 characters */
	512,					/* 128 characters */
	1,					/* 1 bits per pixel */
	{ 0 },					/* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16					/* every char takes 16 bytes */
};

static GFXDECODE_START( vta2000 )
	GFXDECODE_ENTRY( "chargen", 0x0000, vta2000_charlayout, 0, 1 )
GFXDECODE_END

static PALETTE_INIT( vta2000 )
{
	palette_set_color(machine, 0, RGB_BLACK); // black
	palette_set_color_rgb(machine, 1, 0x00, 0xc0, 0x00); // green
	palette_set_color_rgb(machine, 2, 0x00, 0xff, 0x00); // highlight
}

static MACHINE_CONFIG_START( vta2000, vta2000_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8080, XTAL_4MHz / 4)
	MCFG_CPU_PROGRAM_MAP(vta2000_mem)
	MCFG_CPU_IO_MAP(vta2000_io)

	MCFG_MACHINE_RESET(vta2000)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(80*8, 25*12)
	MCFG_SCREEN_VISIBLE_AREA(0, 80*8-1, 0, 25*12-1)
	MCFG_VIDEO_START(vta2000)
	MCFG_SCREEN_UPDATE_STATIC(vta2000)
	MCFG_PALETTE_LENGTH(3)
	MCFG_PALETTE_INIT(vta2000)
	MCFG_GFXDECODE(vta2000)
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( vta2000 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bdp-15_11.rom", 0x4000, 0x2000, CRC(d4abe3e9) SHA1(ab1973306e263b0f66f2e1ede50cb5230f8d69d5) )
	ROM_LOAD( "bdp-15_12.rom", 0x2000, 0x2000, CRC(4a5fe332) SHA1(f1401c26687236184fec0558cc890e796d7d5c77) )
	ROM_LOAD( "bdp-15_13.rom", 0x0000, 0x2000, CRC(b6b89d90) SHA1(0356d7ba77013b8a79986689fb22ef4107ef885b) )

	ROM_REGION(0x2000, "chargen", ROMREGION_INVERT )
	ROM_LOAD( "bdp-15_14.rom", 0x0000, 0x2000, CRC(a1dc4f8e) SHA1(873fd211f44713b713d73163de2d8b5db83d2143) )
ROM_END

/* Driver */

/*    YEAR  NAME      PARENT  COMPAT   MACHINE    INPUT    INIT   COMPANY     FULLNAME       FLAGS */
COMP( ????, vta2000,  0,      0,       vta2000,   vta2000, driver_device, 0,   "<unknown>", "VTA-2000", GAME_NOT_WORKING | GAME_NO_SOUND)

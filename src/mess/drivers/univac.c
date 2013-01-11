/***************************************************************************

    Univac Terminals

    The terminals are models UTS20, UTS30, UTS40, UTS50 and SVT1120,
    however only the UTS20 is dumped (program roms only).

    25/05/2009 Skeleton driver

    The terminal has 2 screens selectable by the operator with the Fn + 1-2
    buttons. Thus the user can have two sessions open at once, to different
    mainframes or applications.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"


class univac_state : public driver_device
{
public:
	univac_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu")
	{ }

	required_device<cpu_device> m_maincpu;
	DECLARE_READ8_MEMBER( uts20_vram_r );
	DECLARE_WRITE8_MEMBER( uts20_vram_w );
	DECLARE_WRITE8_MEMBER( uts20_43_w );
	const UINT8 *m_p_chargen;
	bool m_screen_num;
	UINT8 m_framecnt;
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};



WRITE8_MEMBER( univac_state::uts20_43_w )
{
	m_screen_num = data & 1;
}

READ8_MEMBER( univac_state::uts20_vram_r )
{
	UINT8 *RAM = memregion("maincpu")->base();
	return RAM[offset | ((m_screen_num) ? 0xe000 : 0xc000)];
}

WRITE8_MEMBER( univac_state::uts20_vram_w )
{
	UINT8 *RAM = memregion("maincpu")->base();
	RAM[offset | ((m_screen_num) ? 0xe000 : 0xc000)] = data;
}


static ADDRESS_MAP_START(uts20_mem, AS_PROGRAM, 8, univac_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x4fff ) AM_ROM
	AM_RANGE( 0x5000, 0x7fff ) AM_RAM AM_REGION("maincpu", 0x5000)
	AM_RANGE( 0x8000, 0x9fff ) AM_READWRITE(uts20_vram_r,uts20_vram_w)
	AM_RANGE( 0xa000, 0xffff ) AM_RAM AM_REGION("maincpu", 0xa000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( uts20_io, AS_IO, 8, univac_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x43, 0x43 ) AM_WRITE(uts20_43_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( uts20 )
INPUT_PORTS_END


void univac_state::machine_reset()
{
	m_screen_num = 0;
}

void univac_state::video_start()
{
	m_p_chargen = memregion("chargen")->base();
}

UINT32 univac_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 y,ra,chr,gfx;
	UINT16 sy=0,ma=0,x;
	UINT8 *videoram = memregion("maincpu")->base()+((m_screen_num) ? 0xe000 : 0xc000);

	m_framecnt++;

	for (y = 0; y < 25; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 80; x++)
			{
				gfx = 0;

				if (ra < 9)
				{
					chr = videoram[x];

					/* Take care of flashing characters */
					if ((chr & 0x80) && (m_framecnt & 8))
						chr = 0x20;

					chr &= 0x7f;

					gfx = m_p_chargen[(chr<<4) | ra ];
				}

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma+=80;
	}
	return 0;
}

static MACHINE_CONFIG_START( uts20, univac_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(uts20_mem)
	MCFG_CPU_IO_MAP(uts20_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(univac_state, screen_update)
	MCFG_SCREEN_SIZE(640, 250)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 249)
	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(black_and_white)
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( uts20 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "uts20a.rom", 0x0000, 0x1000, CRC(1a7b4b4e) SHA1(c3732e25b4b7c7a80172e3fe55c77b923cf511eb) )
	ROM_LOAD( "uts20b.rom", 0x1000, 0x1000, CRC(7f8de87b) SHA1(a85f404ad9d560df831cc3e651a4b45e4ed30130) )
	ROM_LOAD( "uts20c.rom", 0x2000, 0x1000, CRC(4e334705) SHA1(ff1a730551b42f29d20af8ecc4495fd30567d35b) )
	ROM_LOAD( "uts20d.rom", 0x3000, 0x1000, CRC(76757cf7) SHA1(b0509d9a35366b21955f83ec3685163844c4dbf1) )
	ROM_LOAD( "uts20e.rom", 0x4000, 0x1000, CRC(0dfc8062) SHA1(cd681020bfb4829d4cebaf1b5bf618e67b55bda3) )

	/* character generator not dumped, using the one from 'c10' for now */
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD("c10_char.bin", 0x0000, 0x2000, BAD_DUMP CRC(cb530b6f) SHA1(95590bbb433db9c4317f535723b29516b9b9fcbf) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY   FULLNAME       FLAGS */
COMP( 198?, uts20,  0,       0,     uts20,  uts20, driver_device,    0,      "Sperry Univac",   "UTS-20", GAME_NOT_WORKING | GAME_NO_SOUND)

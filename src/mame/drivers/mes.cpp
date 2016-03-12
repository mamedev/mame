// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        Schleicher MES

        30/08/2010 Skeleton driver

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"


class mes_state : public driver_device
{
public:
	mes_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_p_videoram(*this, "p_videoram"){ }

	required_device<cpu_device> m_maincpu;
	const UINT8 *m_p_chargen;
	required_shared_ptr<UINT8> m_p_videoram;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};



static ADDRESS_MAP_START(mes_mem, AS_PROGRAM, 8, mes_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x1000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_RAM AM_SHARE("p_videoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( mes_io, AS_IO, 8, mes_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( mes )
INPUT_PORTS_END

void mes_state::machine_reset()
{
}

void mes_state::video_start()
{
	m_p_chargen = memregion("chargen")->base();
}

/* This system appears to have 2 screens. Not implemented.
    Also the screen dimensions are a guess. */
UINT32 mes_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//static UINT8 framecnt=0;
	UINT8 y,ra,chr,gfx;
	UINT16 sy=0,ma=0,x,xx;

	//framecnt++;

	for (y = 0; y < 25; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

			xx = ma;
			for (x = ma; x < ma + 80; x++)
			{
				gfx = 0;
				if (ra < 9)
				{
					chr = m_p_videoram[xx++];

				//  /* Take care of flashing characters */
				//  if ((chr < 0x80) && (framecnt & 0x08))
				//      chr |= 0x80;

					if (chr & 0x80)  // ignore attribute bytes
						x--;
					else
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

static MACHINE_CONFIG_START( mes, mes_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_16MHz / 4)
	MCFG_CPU_PROGRAM_MAP(mes_mem)
	MCFG_CPU_IO_MAP(mes_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(mes_state, screen_update)
	MCFG_SCREEN_SIZE(640, 250)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 249)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_MONOCHROME("palette")
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( mes )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "mescpu.bin",   0x0000, 0x1000, CRC(b6d90cf4) SHA1(19e608af5bdaabb00a134e1106b151b00e2a0b04))

	ROM_REGION( 0x10000, "xebec", ROMREGION_ERASEFF )
	ROM_LOAD( "mesxebec.bin", 0x0000, 0x2000, CRC(061b7212) SHA1(c5d600116fb7563c69ebd909eb9613269b2ada0f))

	/* character generator not dumped, using the one from 'c10' for now */
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "c10_char.bin", 0x0000, 0x2000, BAD_DUMP CRC(cb530b6f) SHA1(95590bbb433db9c4317f535723b29516b9b9fcbf))
ROM_END

/* Driver */

/*   YEAR   NAME    PARENT  COMPAT   MACHINE  INPUT  INIT        COMPANY     FULLNAME       FLAGS */
COMP( 198?, mes,    0,      0,       mes,     mes, driver_device,   0,       "Schleicher",   "MES", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)

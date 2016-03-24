// license:BSD-3-Clause
// copyright-holders: Robbbert
/***************************************************************************

        Homebrew Z80-based TV Game computer by Mr. Isizu

        http://w01.tp1.jp/~a571632211/z80tvgame/index.html

        2015-06-12 Driver by Robbbert

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/speaker.h"


class tvgame_state : public driver_device
{
public:
	tvgame_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_speaker(*this, "speaker")
		, m_p_videoram(*this, "videoram")
	{ }

	DECLARE_WRITE8_MEMBER(speaker_w);
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
private:
	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_shared_ptr<UINT8> m_p_videoram;
};

static ADDRESS_MAP_START( tvgame_mem, AS_PROGRAM, 8, tvgame_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x7fff ) AM_ROM
	AM_RANGE( 0x8000, 0xbfff ) AM_RAM
	AM_RANGE( 0xc000, 0xdfff ) AM_RAM AM_SHARE("videoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( tvgame_io, AS_IO, 8, tvgame_state )
	ADDRESS_MAP_GLOBAL_MASK(3)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x0003) AM_DEVREADWRITE("ppi", i8255_device, read, write)
ADDRESS_MAP_END

/* Input ports */
INPUT_PORTS_START( tvgame )
	PORT_START("LINE0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

WRITE8_MEMBER( tvgame_state::speaker_w )
{
	m_speaker->level_w(BIT(data, 0));
}

UINT32 tvgame_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 y,gfx;
	UINT16 sy=0,ma=241,x;

	for (y = 0; y < 213; y++)
	{
		UINT16 *p = &bitmap.pix16(sy++);
		for (x = ma; x < ma+27; x++)
		{
			gfx = m_p_videoram[x];

			/* Display a scanline of a character (8 pixels) */
			*p++ = BIT(gfx, 0);
			*p++ = BIT(gfx, 1);
			*p++ = BIT(gfx, 2);
			*p++ = BIT(gfx, 3);
			*p++ = BIT(gfx, 4);
			*p++ = BIT(gfx, 5);
			*p++ = BIT(gfx, 6);
			*p++ = BIT(gfx, 7);
		}
		ma+=30;
	}
	return 0;
}

static MACHINE_CONFIG_START( tvgame, tvgame_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(tvgame_mem)
	MCFG_CPU_IO_MAP(tvgame_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(tvgame_state, screen_update)
	MCFG_SCREEN_SIZE(216, 213)
	MCFG_SCREEN_VISIBLE_AREA(0, 215, 0, 212)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_PALETTE_ADD_MONOCHROME("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	// Devices
	MCFG_DEVICE_ADD("ppi", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("LINE0"))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(tvgame_state, speaker_w))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( tvgame )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "tvgame_all.ppi",  0x0000, 0x8000, CRC(b61d17bd) SHA1(bb92d6679370fc31d67cc334807d88d7288c7cfa) )

	ROM_REGION(0x1000, "proms", 0)
	ROM_LOAD( "video32.bin", 0x0000, 0x1000, CRC(516006e3) SHA1(942b31acccf833cd722cbcb739eb87673dc633d7) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    CLASS         INIT    COMPANY   FULLNAME       FLAGS */
CONS( 2011, tvgame, 0,      0,       tvgame,    tvgame,  driver_device, 0,    "Mr. Isizu", "Z80 TV Game System", 0 )

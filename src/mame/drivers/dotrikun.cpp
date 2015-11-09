// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/***************************************************************************

Dottori Kun (Head On's mini game)
(c)1990 SEGA

Driver by Takahiro Nogi (nogi@kt.rim.or.jp) 1999/12/15 -


CPU   : Z-80 (4MHz)
SOUND : (none)

14479.MPR  ; PRG (FIRST VER)
14479A.MPR ; PRG (NEW VER)

* This game is only for the test of cabinet
* BackRaster = WHITE on the FIRST version.
* BackRaster = BLACK on the NEW version.
* On the NEW version, push COIN-SW as TEST MODE.
* 0000-3FFF:ROM 8000-85FF:VRAM(128x96) 8600-87FF:WORK-RAM

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"


class dotrikun_state : public driver_device
{
public:
	dotrikun_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_dotrikun_bitmap(*this, "dotrikun_bitmap"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_dotrikun_bitmap;

	/* video-related */
	UINT8          m_color;
	DECLARE_WRITE8_MEMBER(dotrikun_color_w);
	virtual void machine_start();
	virtual void machine_reset();
	UINT32 screen_update_dotrikun(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
};


/*************************************
 *
 *  Video emulation
 *
 *************************************/

WRITE8_MEMBER(dotrikun_state::dotrikun_color_w)
{
	/*
	x--- ---- screen color swap?
	---- -x-- B
	---- --x- G
	---- ---x R
	*/

	m_color = data;
	m_screen->update_partial(m_screen->vpos());
}


UINT32 dotrikun_state::screen_update_dotrikun(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int x,y,i;

	pen_t back_pen = rgb_t(pal1bit(m_color >> 3), pal1bit(m_color >> 4), pal1bit(m_color >> 5));
	pen_t fore_pen = rgb_t(pal1bit(m_color >> 0), pal1bit(m_color >> 1), pal1bit(m_color >> 2));

	for(y = (cliprect.min_y & ~1); y < cliprect.max_y; y+=2)
	{
		for (x = 0; x < 256; x+=16)
		{
			UINT8 data = m_dotrikun_bitmap[((x/16)+((y/2)*16))];

			for (i = 0; i < 8; i++)
			{
				pen_t pen = ((data >> (7 - i)) & 1) ? fore_pen : back_pen;

				/* I think the video hardware doubles pixels, screen would be too small otherwise */
				bitmap.pix32(y + 0, (x + 0) + i*2) = pen;
				bitmap.pix32(y + 0, (x + 1) + i*2) = pen;
				bitmap.pix32(y + 1, (x + 0) + i*2) = pen;
				bitmap.pix32(y + 1, (x + 1) + i*2) = pen;
			}
		}
	}

	return 0;
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( dotrikun_map, AS_PROGRAM, 8, dotrikun_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x8000, 0x85ff) AM_RAM AM_SHARE("dotrikun_bitmap")
	AM_RANGE(0x8600, 0x87ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, dotrikun_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("INPUTS") AM_WRITE(dotrikun_color_w)
ADDRESS_MAP_END


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( dotrikun )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void dotrikun_state::machine_start()
{
	save_item(NAME(m_color));
}

void dotrikun_state::machine_reset()
{
	m_color = 0;
}

#define MASTER_CLOCK XTAL_4MHz

static MACHINE_CONFIG_START( dotrikun, dotrikun_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK)       /* 4 MHz */
	MCFG_CPU_PROGRAM_MAP(dotrikun_map)
	MCFG_CPU_IO_MAP(io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", dotrikun_state,  irq0_line_hold)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK, 256+32, 0, 256, 192+32, 0, 192) // FIXME: h/v params of this are completely inaccurate, shows it especially under the "CRT test"
	MCFG_SCREEN_UPDATE_DRIVER(dotrikun_state, screen_update_dotrikun)

	/* sound hardware */
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( dotrikun )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "14479a.mpr", 0x0000, 0x4000, CRC(b77a50db) SHA1(2a5d812d39f0f58f5c3e1b46f80aca75aa225115) )
ROM_END

ROM_START( dotrikun2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "14479.mpr",  0x0000, 0x4000, CRC(a6aa7fa5) SHA1(4dbea33fb3541fdacf2195355751078a33bb30d5) )
ROM_END


GAME( 1990, dotrikun, 0,        dotrikun, dotrikun, driver_device, 0, ROT0, "Sega", "Dottori Kun (new version)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1990, dotrikun2,dotrikun, dotrikun, dotrikun, driver_device, 0, ROT0, "Sega", "Dottori Kun (old version)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW | MACHINE_IMPERFECT_GRAPHICS )

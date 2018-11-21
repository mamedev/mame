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
#include "sound/spkrdev.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class tvgame_state : public driver_device
{
public:
	tvgame_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_speaker(*this, "speaker")
		, m_p_videoram(*this, "videoram")
	{ }

	void tvgame(machine_config &config);

private:
	DECLARE_WRITE8_MEMBER(speaker_w);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void io_map(address_map &map);
	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_shared_ptr<uint8_t> m_p_videoram;
};

void tvgame_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).ram();
	map(0xc000, 0xdfff).ram().share("videoram");
}

void tvgame_state::io_map(address_map &map)
{
	map.global_mask(3);
	map.unmap_value_high();
	map(0x0000, 0x0003).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
}

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

uint32_t tvgame_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t y,gfx;
	uint16_t sy=0,ma=241,x;

	for (y = 0; y < 213; y++)
	{
		uint16_t *p = &bitmap.pix16(sy++);
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

MACHINE_CONFIG_START(tvgame_state::tvgame)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, XTAL(4'000'000))
	MCFG_DEVICE_PROGRAM_MAP(mem_map)
	MCFG_DEVICE_IO_MAP(io_map)

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
	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD("speaker", SPEAKER_SOUND)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	// Devices
	i8255_device &ppi(I8255(config, "ppi"));
	ppi.in_pa_callback().set_ioport("LINE0");
	ppi.out_pc_callback().set(FUNC(tvgame_state::speaker_w));
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( tvgame )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "tvgame_all.ppi",  0x0000, 0x8000, CRC(b61d17bd) SHA1(bb92d6679370fc31d67cc334807d88d7288c7cfa) )

	ROM_REGION(0x1000, "proms", 0)
	ROM_LOAD( "video32.bin", 0x0000, 0x1000, CRC(516006e3) SHA1(942b31acccf833cd722cbcb739eb87673dc633d7) )
ROM_END

/* Driver */

//    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    CLASS         INIT        COMPANY      FULLNAME              FLAGS
CONS( 2011, tvgame, 0,      0,       tvgame,    tvgame,  tvgame_state, empty_init, "Mr. Isizu", "Z80 TV Game System", 0 )

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

namespace {

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
	void speaker_w(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

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

void tvgame_state::speaker_w(uint8_t data)
{
	m_speaker->level_w(BIT(data, 0));
}

uint32_t tvgame_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t sy=0,ma=241;

	for (uint8_t y = 0; y < 213; y++)
	{
		uint16_t *p = &bitmap.pix(sy++);
		for (uint16_t x = ma; x < ma+27; x++)
		{
			uint8_t gfx = m_p_videoram[x];

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

void tvgame_state::tvgame(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &tvgame_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &tvgame_state::io_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(tvgame_state::screen_update));
	screen.set_size(216, 213);
	screen.set_visarea(0, 215, 0, 212);
	screen.set_palette("palette");
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	// Devices
	i8255_device &ppi(I8255(config, "ppi"));
	ppi.in_pa_callback().set_ioport("LINE0");
	ppi.out_pc_callback().set(FUNC(tvgame_state::speaker_w));
}

/* ROM definition */
ROM_START( tvgame )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "tvgame_all.ppi",  0x0000, 0x8000, CRC(b61d17bd) SHA1(bb92d6679370fc31d67cc334807d88d7288c7cfa) )

	ROM_REGION(0x1000, "proms", 0)
	ROM_LOAD( "video32.bin", 0x0000, 0x1000, CRC(516006e3) SHA1(942b31acccf833cd722cbcb739eb87673dc633d7) )
ROM_END

} // Anonymous namespace

//    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    CLASS         INIT        COMPANY      FULLNAME              FLAGS
CONS( 2011, tvgame, 0,      0,       tvgame,    tvgame,  tvgame_state, empty_init, "Mr. Isizu", "Z80 TV Game System", MACHINE_SUPPORTS_SAVE )

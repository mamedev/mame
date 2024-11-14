// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/***************************************************************************

Mini Vaders (Space Invaders's mini game)
(c)1990 Taito Corporation

Driver by Takahiro Nogi 1999/12/19 -

This is a test board sold together with the cabinet (as required by law in
Japan). It has no sound.

PCB Layout
----------

K11X0622A
MINI VADERS
|-------------------------|
|MB3771 24MHz             |
|LS32   74F74             |
|LS139                    |
|D26_01.IC7 LS244        J|
|Z80        LS244        A|
|LS86  LS08 LS373        M|
|LS157 LS157 LS161 LS161 M|
|LS157 LS157 LS161 LS161 A|
|6116        LS157 LS08   |
|LS74  LS74  LS74  LS157  |
|-------------------------|
Notes: (all ICs shown)
       Z80  - Clock 4MHz [24/6]
       6116 - 2Kbx8 SRAM
 D26_01.IC7 - 27C64 8Kbx8 EPROM

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "screen.h"


namespace {

class minivadr_state : public driver_device
{
public:
	minivadr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu") { }

	void minivadr(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	uint32_t screen_update_minivadr(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	void minivadr_map(address_map &map) ATTR_COLD;
};

/*************************************
 *
 *  Video update
 *
 *************************************/

uint32_t minivadr_state::screen_update_minivadr(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (offs_t offs = 0; offs < m_videoram.bytes(); offs++)
	{
		uint8_t x = offs << 3;
		const int y = offs >> 5;
		uint8_t data = m_videoram[offs];

		for (int i = 0; i < 8; i++)
		{
			pen_t pen = (data & 0x80) ? rgb_t::white() : rgb_t::black();
			bitmap.pix(y, x) = pen;

			data <<= 1;
			x++;
		}
	}

	return 0;
}


void minivadr_state::minivadr_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0xa000, 0xbfff).ram().share("videoram");
	map(0xe008, 0xe008).portr("INPUTS").nopw();     // W - ???
}


static INPUT_PORTS_START( minivadr )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void minivadr_state::minivadr(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(24'000'000) / 6);
	m_maincpu->set_addrmap(AS_PROGRAM, &minivadr_state::minivadr_map);
	m_maincpu->set_vblank_int("screen", FUNC(minivadr_state::irq0_line_hold));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 16, 240-1);
	screen.set_screen_update(FUNC(minivadr_state::screen_update_minivadr));

	/* the board has no sound hardware */
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( minivadr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d26-01.ic7", 0x0000, 0x2000, CRC(a96c823d) SHA1(aa9969ff80e94b0fff0f3530863f6b300510162e) )
ROM_END

} // anonymous namespace


GAME( 1990, minivadr, 0, minivadr, minivadr, minivadr_state, empty_init, ROT0, "Taito Corporation", "Mini Vaders", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )

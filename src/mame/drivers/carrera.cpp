// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

This is a simple 'Pairs' game called
Carrera or Bomberman by BS Electronics



PCB Layout
----------

|----------------------------------------------|
|            22.1184MHz  Z80                   |
|                                     ROM.IC1  |
|_  BATTERY              ROM.IC22              |
  |                                            |
 _|                      6116         ROM.IC2  |
|                                              |
|                                              |
|J  AY-3-8910                         ROM.IC3  |
|A                       DSW1(8)               |
|M                                             |
|M                       DSW2(8)      ROM.IC4  |
|A                                             |
|                        DSW3(8)               |
|                                     ROM.IC5  |
|_  PROM.IC39            DSW4(8)               |
  |                                    6116    |
 _|                                            |
|                   HD6845             6116    |
|----------------------------------------------|
Notes:
      Z80 @ 3.6864MHz [22.1184/6]
      AY-3-8910 @ 1.8432MHz [22.1184/12]


Emulation Notes:
 Corrupt Tile on the first R in Carrera? (unlikely to be a bug, HW is very simple..)

TODO:
- Are colors 100% correct? Needs a reference to be sure.
- There are reel gfxs on the roms (near the end), left-over or there's a way to enable it?
  Update: if you trigger a normal irq 0 instead of a NMI the game will change into a proper 8 liner game without inputs. Investigate on it...

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#define MASTER_CLOCK    XTAL(22'118'400)


class carrera_state : public driver_device
{
public:
	carrera_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_tileram(*this, "tileram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void carrera(machine_config &config);

private:
	DECLARE_READ8_MEMBER(unknown_r);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void prg_map(address_map &map);
	void io_map(address_map &map);

	required_shared_ptr<uint8_t> m_tileram;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


void carrera_state::prg_map(address_map &map)
{
	map(0x0000, 0x4fff).rom();
	map(0xe000, 0xe7ff).ram();
	map(0xe800, 0xe800).w("crtc", FUNC(mc6845_device::address_w));
	map(0xe801, 0xe801).w("crtc", FUNC(mc6845_device::register_w));
	map(0xf000, 0xffff).ram().share(m_tileram);
}

void carrera_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("IN0");
	map(0x01, 0x01).portr("IN1");
	map(0x02, 0x02).portr("IN2");
	map(0x03, 0x03).portr("IN3");
	map(0x04, 0x04).portr("IN4");
	map(0x05, 0x05).portr("IN5");
	map(0x06, 0x06).nopw(); // ?
	map(0x08, 0x09).w("aysnd", FUNC(ay8910_device::address_data_w));
}

static INPUT_PORTS_START( carrera )
	PORT_START("IN0")   /* Port 0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1 )
	/* unused / unknown inputs, not dips */
	PORT_DIPNAME( 0x20, 0x20, "0" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_SERVICE1 ) PORT_NAME("Master Reset")
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")   /* Port 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	/* unused / unknown inputs, not dips */
	PORT_DIPNAME( 0x04, 0x04, "1" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	/* I suspect the 4 below are the 4xDSWs */
	PORT_START("IN2")   /* Port 2 */
	PORT_DIPNAME( 0x01, 0x01, "2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN3")   /* Port 3 */
	PORT_DIPNAME( 0x01, 0x01, "3" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN4")   /* Port 4 */
	PORT_DIPNAME( 0x01, 0x01, "4" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN5")   /* Port 5 */
	PORT_DIPNAME( 0x01, 0x01, "5" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Playing Graphics" )
	PORT_DIPSETTING(    0x08, "Bricks" )
	PORT_DIPSETTING(    0x00, "Fruits" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Debug?" ) // displays numbers over the game area
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,5),
	5,
	{ RGN_FRAC(0,5), RGN_FRAC(1,5),RGN_FRAC(2,5),RGN_FRAC(3,5),RGN_FRAC(4,5) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_carrera )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 1 )
GFXDECODE_END

uint32_t carrera_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int count = 0;

	for (int y = 0; y < 32; y++)
	{
		for (int x = 0; x < 64; x++)
		{
			int tile = m_tileram[count&0x7ff] | m_tileram[(count&0x7ff)+0x800]<<8;

			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect,tile,0,0,0,x*8,y*8);
			count++;
		}
	}
	return 0;
}

READ8_MEMBER(carrera_state::unknown_r)
{
	return machine().rand();
}

void carrera_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	for (int i = 0; i < 0x20; ++i)
	{
		int bit0, bit1;
		int const br_bit0 = BIT(color_prom[i], 6);
		int const br_bit1 = BIT(color_prom[i], 7);

		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 3);
		int const b = 0x0e * br_bit0 + 0x1f * br_bit1 + 0x43 * bit0 + 0x8f * bit1;
		bit0 = BIT(color_prom[i], 1);
		bit1 = BIT(color_prom[i], 4);
		int const g = 0x0e * br_bit0 + 0x1f * br_bit1 + 0x43 * bit0 + 0x8f * bit1;
		bit0 = BIT(color_prom[i], 2);
		bit1 = BIT(color_prom[i], 5);
		int const r = 0x0e * br_bit0 + 0x1f * br_bit1 + 0x43 * bit0 + 0x8f * bit1;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


void carrera_state::carrera(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK / 6);
	m_maincpu->set_addrmap(AS_PROGRAM, &carrera_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &carrera_state::io_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(carrera_state::screen_update));

	mc6845_device &crtc(MC6845(config, "crtc", MASTER_CLOCK / 16));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.out_vsync_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_carrera);
	PALETTE(config, m_palette, FUNC(carrera_state::palette), 32);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", MASTER_CLOCK/12));
	/* these are set as input, but I have no idea which input port it uses is for the AY */
	aysnd.port_a_read_callback().set(FUNC(carrera_state::unknown_r));
	aysnd.port_b_read_callback().set(FUNC(carrera_state::unknown_r));
	aysnd.add_route(ALL_OUTPUTS, "mono", 1.00);
}


ROM_START( carrera )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27512.ic22", 0x00000, 0x10000, CRC(2385b9c8) SHA1(12d4397779e074096fbb23b114985f104366b79c) )

	ROM_REGION( 0x50000, "gfx1", 0 )
	ROM_LOAD( "27512.ic1", 0x00000, 0x10000, CRC(a16e914e) SHA1(09f2271f193a7bffd62ef6e428ecbf9aa1154860) )
	ROM_LOAD( "27512.ic2", 0x10000, 0x10000, CRC(147036a5) SHA1(34b4818fe61c5b13220b0a2001987b68b655b2cb) )
	ROM_LOAD( "27512.ic3", 0x20000, 0x10000, CRC(920eee0e) SHA1(85e6d5292b751c57c64d17858bd00292356599e3) )
	ROM_LOAD( "27512.ic4", 0x30000, 0x10000, CRC(97433f36) SHA1(39f3c6b76ad540693682832aba6e4fc400ca3753) )
	ROM_LOAD( "27512.ic5", 0x40000, 0x10000, CRC(ffa75920) SHA1(aa5619f5aabcdfa250bb24bcad101a8c512a1776) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "82s123.ic39", 0x00, 0x20, CRC(af16359f) SHA1(1ff5c9d7807e52be09c0ded56fb68a47e41b3fcf) )
ROM_END

ROM_START( bsebman )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic22", 0x00000, 0x10000, CRC(294a205f) SHA1(b088617354b6a37520060f19f77d841ad8ee1538) )

	ROM_REGION( 0x50000, "gfx1", 0 )
	ROM_LOAD( "ic1", 0x00000, 0x10000, CRC(bf4868e1) SHA1(ca91343dbdb9f43d9b981b9b3f958edb17bf188d) )
	ROM_LOAD( "ic2", 0x10000, 0x10000, CRC(26e1c17e) SHA1(ea0669e87207104eeaa3eae6a6708dbdf94e3c3c) )
	ROM_LOAD( "ic3", 0x20000, 0x10000, CRC(8401248d) SHA1(f8c8ed93d76709ead0b262ab3039df6febe3c005) )
	ROM_LOAD( "ic4", 0x30000, 0x10000, CRC(6b569989) SHA1(e00263fae310094ad5119e3a9673fb342f643ddc) )
	ROM_LOAD( "ic5", 0x40000, 0x10000, CRC(21635791) SHA1(514078694269582c33fb7dddd6171089f9e21ee2) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "82s123.ic39", 0x00, 0x20, BAD_DUMP CRC(af16359f) SHA1(1ff5c9d7807e52be09c0ded56fb68a47e41b3fcf) ) // BPROM had broken legs, not dumped for now, probably same as carrera
ROM_END

GAME( 19??, carrera, 0,       carrera, carrera, carrera_state, empty_init, ROT0, "BS Electronics", "Carrera (Version 6.7)",   MACHINE_SUPPORTS_SAVE )
GAME( 19??, bsebman, carrera, carrera, carrera, carrera_state, empty_init, ROT0, "BS Electronics", "Bomberman (Version 6.6)", MACHINE_SUPPORTS_SAVE )

// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Go 2000 - Korean Card game

Newer PCB, very sparse with newer surface mounted CPUs

MC68EC000FU10
Z84C0006FEC
TM29F550ZX
OSC: 32.000MHz
2 8-way Dipswitch banks
Ram:
 2 UM61256FK-15 (near 3 & 4 (68k program roms))
 3 Windbond W24257AK-15 (near TM29F550ZX)
 2 UM61256AK-15 (near Z80)

P1, P2 & P3 4-pin connectors (unknown purpose)

2008-08:
Added Dips and Dip locations based on Service Mode.

TODO:
- Merge this driver with SunA16 driver.

Notes:
- Maybe SA stands for SunA? The z80 memory map matches the one seen in Ultra Balloon,
  and the only difference stands in the DAC used. And the sprite chip is the same as
  the one used in SunA16 driver as well.

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/dac.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class go2000_state : public driver_device
{
public:
	go2000_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram%u", 0U),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundbank(*this, "soundbank")
	{ }

	void go2000(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// memory pointers
	required_shared_ptr_array<uint16_t, 2> m_videoram;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_memory_bank m_soundbank;

	void pcm_1_bankswitch_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map);
	void sound_io(address_map &map);
	void sound_map(address_map &map);
};


void go2000_state::main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x200000, 0x203fff).ram();
	map(0x600000, 0x60ffff).ram().share(m_videoram[0]);
	map(0x610000, 0x61ffff).ram().share(m_videoram[1]);
	map(0x800000, 0x800fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xa00000, 0xa00001).portr("INPUTS");
	map(0xa00002, 0xa00003).portr("DSW");
	map(0x620003, 0x620003).w("soundlatch", FUNC(generic_latch_8_device::write));
//  map(0xe00000, 0xe00001).nopw();
//  map(0xe00010, 0xe00011).nopw();
//  map(0xe00020, 0xe00021).nopw();
}

void go2000_state::pcm_1_bankswitch_w(uint8_t data)
{
	m_soundbank->set_entry(data & 0x07);
}

void go2000_state::sound_map(address_map &map)
{
	map(0x0000, 0x03ff).rom();
	map(0x0400, 0xffff).bankr(m_soundbank);
}

void go2000_state::sound_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x00, 0x00).w("dac", FUNC(dac_byte_interface::data_w));
	map(0x03, 0x03).w(FUNC(go2000_state::pcm_1_bankswitch_w));
}


static INPUT_PORTS_START( go2000 )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) // continue
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON2 ) // Korean symbol
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON3 ) // out
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) // high
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) // low
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED) // unused?
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED) // unused?
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED) // unused?
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED) // unused?
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 M1")   // m1
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P1 M2")   // m2
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P1 M3")   // m3
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED) // unused?
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN2 ) // coin2

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, "Coin / Credits" ) PORT_DIPLOCATION("SW-1:1,2")
	PORT_DIPSETTING(      0x0000, "1 Coin / 50 Credits" )
	PORT_DIPSETTING(      0x0003, "1 Coin / 100 Credits" )
	PORT_DIPSETTING(      0x0002, "1 Coin / 125 Credits" )
	PORT_DIPSETTING(      0x0001, "1 Coin / 150 Credits" )
	PORT_DIPNAME( 0x000c, 0x000c, "Minimum Coin" ) PORT_DIPLOCATION("SW-1:3,4")
	PORT_DIPSETTING(      0x000c, "1" )
	PORT_DIPSETTING(      0x0008, "2" )
	PORT_DIPSETTING(      0x0004, "3" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0010, IP_ACTIVE_LOW, "SW-1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0020, IP_ACTIVE_LOW, "SW-1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0040, IP_ACTIVE_LOW, "SW-1:7" )
	PORT_SERVICE_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW-1:8" )
	PORT_DIPNAME( 0x0700, 0x0700, "Difficult-1" ) PORT_DIPLOCATION("SW-2:1,2,3")
	PORT_DIPSETTING(      0x0700, "1" )
	PORT_DIPSETTING(      0x0600, "2" )
	PORT_DIPSETTING(      0x0500, "3" )
	PORT_DIPSETTING(      0x0400, "4" )
	PORT_DIPSETTING(      0x0300, "5" )
	PORT_DIPSETTING(      0x0200, "6" )
	PORT_DIPSETTING(      0x0100, "7" )
	PORT_DIPSETTING(      0x0000, "8" )
	PORT_DIPNAME( 0x1800, 0x1800, "M1 value" ) PORT_DIPLOCATION("SW-2:4,5")
	PORT_DIPSETTING(      0x0800, "3000" ) PORT_CONDITION("DSW", 0x0003, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0000, "3500" ) PORT_CONDITION("DSW", 0x0003, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x1800, "4000" ) PORT_CONDITION("DSW", 0x0003, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x1000, "4500" ) PORT_CONDITION("DSW", 0x0003, EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0800, "6000" ) PORT_CONDITION("DSW", 0x0003, NOTEQUALS, 0x0000)
	PORT_DIPSETTING(      0x0000, "7000" ) PORT_CONDITION("DSW", 0x0003, NOTEQUALS, 0x0000)
	PORT_DIPSETTING(      0x1800, "8000" ) PORT_CONDITION("DSW", 0x0003, NOTEQUALS, 0x0000)
	PORT_DIPSETTING(      0x1000, "9000" ) PORT_CONDITION("DSW", 0x0003, NOTEQUALS, 0x0000)
	PORT_DIPUNKNOWN_DIPLOC( 0x2000, IP_ACTIVE_LOW, "SW-2:6" )
	PORT_DIPNAME( 0xc000, 0xc000, "Difficult-2" ) PORT_DIPLOCATION("SW-2:7,8")
	PORT_DIPSETTING(      0xc000, "1" )
	PORT_DIPSETTING(      0x8000, "2" )
	PORT_DIPSETTING(      0x4000, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
INPUT_PORTS_END


static const gfx_layout go2000_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 8,12,0,4 },
	{ 3,2,1,0, 19,18,17,16 },
	{ 0*32, 1*32, 2*32, 3*32,4*32,5*32,6*32,7*32 },
	8*32
};

static GFXDECODE_START( gfx_go2000 )
	GFXDECODE_ENTRY( "tiles", 0, go2000_layout,   0x0, 0x80 )
GFXDECODE_END


uint32_t go2000_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int count = 0;

	// 0x600000 - 0x601fff / 0x610000 - 0x611fff
	for (int x = 0; x < 64; x++)
	{
		for (int y = 0; y < 32; y++)
		{
			int tile = m_videoram[0][count];
			int attr = m_videoram[1][count];
			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect, tile, attr, 0, 0, x * 8, y * 8);
			count++;
		}
	}

	// 0x602000 - 0x603fff / 0x612000 - 0x613fff
	for (int x = 0; x < 64; x++)
	{
		for (int y = 0; y < 32; y++)
		{
			int tile = m_videoram[0][count];
			int attr = m_videoram[1][count];
			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect, tile, attr, 0, 0, x * 8, y * 8, 0xf);
			count++;
		}
	}

	// Sprite RAM code actually copied from video/suna16.cpp with minor modifications.
	int max_x = m_screen->width() - 8;
	int max_y = m_screen->height() - 8;

	for (int offs = 0xf800 / 2; offs < 0x10000 / 2 ; offs += 4/2)
	{
		int dimx, dimy;
		int tile_xinc, tile_xstart;
		int flipx, y0;

		int y = m_videoram[0][offs + 0 + 0x00000 / 2];
		int x = m_videoram[0][offs + 1 + 0x00000 / 2];
		int dim = m_videoram[1][offs + 0 + 0x00000 / 2];

		int bank    =   (x >> 12) & 0xf;

		int srcpg = ((y & 0xf000) >> 12) + ((x & 0x0200) >> 5); // src page
		int srcx = ((y >> 8) & 0xf) * 2;                    // src col
		int srcy = ((dim >> 0) & 0xf) * 2;                  // src row

		switch ((dim >> 4) & 0xc)
		{
			case 0x0:   dimx = 2;   dimy = 2;   y0 = 0x100; break;
			case 0x4:   dimx = 4;   dimy = 4;   y0 = 0x100; break;
			case 0x8:   dimx = 2;   dimy = 32;  y0 = 0x130; break;
			default:
			case 0xc:   dimx = 4;   dimy = 32;  y0 = 0x120; break;
		}

		if (dimx == 4)
		{
			flipx = srcx & 2;
			srcx &= ~2;
		}
		else
			flipx = 0;

		x = (x & 0xff) - (x & 0x100);
		y = (y0 - (y & 0xff) - dimy * 8) & 0xff;

		if (flipx)
		{
			tile_xstart = dimx - 1;
			tile_xinc = -1;
		}
		else
		{
			tile_xstart = 0;
			tile_xinc = +1;
		}

		int tile_y = 0;
		int tile_yinc = +1;

		for (int dy = 0; dy < dimy * 8; dy += 8)
		{
			int tile_x = tile_xstart;

			for (int dx = 0; dx < dimx * 8; dx += 8)
			{
				int addr = (srcpg * 0x20 * 0x20) + ((srcx + tile_x) & 0x1f) * 0x20 + ((srcy + tile_y) & 0x1f);
				int tile = m_videoram[0][addr + 0x00000 / 2];
				int attr = m_videoram[1][addr + 0x00000 / 2];

				int sx = x + dx;
				int sy = (y + dy) & 0xff;

				int tile_flipx = tile & 0x4000;
				int tile_flipy = tile & 0x8000;

				if (flipx)
					tile_flipx = !tile_flipx;

				if (flip_screen())
				{
					sx = max_x - sx;
					sy = max_y - sy;
					tile_flipx = !tile_flipx;
					tile_flipy = !tile_flipy;
				}

				m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
							(tile & 0x1fff) + bank * 0x4000,
							attr,
							tile_flipx, tile_flipy,
							sx, sy, 15 );

				tile_x += tile_xinc;
			}

			tile_y += tile_yinc;
		}
	}

	return 0;
}


void go2000_state::machine_start()
{
	uint8_t *rom = memregion("soundcpu")->base();

	m_soundbank->configure_entries(0, 8, &rom[0x00400], 0x10000);
	m_soundbank->set_entry(0);
}

void go2000_state::go2000(machine_config &config)
{
	M68000(config, m_maincpu, 32_MHz_XTAL / 4); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &go2000_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(go2000_state::irq1_line_hold));

	Z80(config, m_soundcpu, 32_MHz_XTAL / 8); // divider not verified
	m_soundcpu->set_addrmap(AS_PROGRAM, &go2000_state::sound_map);
	m_soundcpu->set_addrmap(AS_IO, &go2000_state::sound_io);


	GFXDECODE(config, m_gfxdecode, m_palette, gfx_go2000);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 48*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(go2000_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x800);


	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline(m_soundcpu, 0);

	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_R2R(config, "dac", 0).add_route(0, "speaker", 0.25); // unknown DAC
}

ROM_START( go2000 )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "3.bin", 0x00000, 0x20000, CRC(fe1fb269) SHA1(266b8acddfcfd960b8e44f8606bf0873da42b9f8) )
	ROM_LOAD16_BYTE( "4.bin", 0x00001, 0x20000, CRC(d6246ae3) SHA1(f2618dcabaa0c0a6e377e4acd1cdec8bea90bea8) )

	ROM_REGION( 0x080000, "soundcpu", 0 ) // Z80
	ROM_LOAD( "5.bin", 0x00000, 0x80000, CRC(a32676ee) SHA1(2dab73497c0818fce479be21ed589985db51560b) )

	ROM_REGION( 0x40000, "tiles", ROMREGION_INVERT )
	ROM_LOAD16_BYTE( "1.bin", 0x00000, 0x20000, CRC(96e50aba) SHA1(caa1aadab855c3a758378dc8c48eec859e8110a4) )
	ROM_LOAD16_BYTE( "2.bin", 0x00001, 0x20000, CRC(b0adf1cb) SHA1(2afb30691182dbf46be709f0d5b03b0f8ff52790) )
ROM_END

} // Anonymous namespace


GAME( 2000, go2000, 0, go2000, go2000, go2000_state, empty_init, ROT0, "SunA", "Go 2000", MACHINE_SUPPORTS_SAVE ) // manufacturer confirmed from https://web.archive.org/web/20160624224827/http://www.grb.or.kr/download/GameImage/2000/2000-A0055.jpg

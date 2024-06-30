// license:BSD-3-Clause
// copyright-holders:

/*
Alone Shettle Crew
1984
Copyright New Digimatic but almost surely developed in Japan.

2-PCB stack
main PCB is marked: "PC-082A" on component side
sub PCB is marked: "MADE IN JAPAN", "SCO-102B(C)1983 GRC" on component side
main PCB is labelled: "NEW DIGIMATIC GARANZIA 6 MESI DATA OTTOBRE 1984" on component side

Hardware has similarities with that of various Nichibutsu games of the same era.

TODO:
- what is the 24-pin chip marked Z4? Same is present on Clash Road. Maybe some kind of protection?
  Reads area $6000-$61ff on player life loss. Game seems to be working but left as MUP since it isn't
  currently known what is affected, if anything.
- input reading isn't correct (see weird coinage DIPs);
- colors need checking on real hardware (available pics are possibly taken with wrong RGB hookup);
- cocktail mode sprite positioning isn't totally correct.
*/


#include "emu.h"

#include "wiping_a.h"

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class shettle_state : public driver_device
{
public:
	shettle_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_io_port(*this, { "IN0", "IN1", "DSW1", "DSW2" })
	{ }

	void shettle(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<u8> m_videoram;
	required_shared_ptr<u8> m_colorram;
	required_shared_ptr<u8> m_spriteram;

	required_ioport_array<4> m_io_port;

	u8 m_main_irq_mask = 0;
	u8 m_sound_irq_mask = 0;

	tilemap_t *m_tilemap = nullptr;

	u8 ports_r(offs_t offset);
	void main_irq_mask_w(int state);
	void sound_irq_mask_w(int state);

	void palette_init(palette_device &palette) const;

	void vram_w(offs_t offset, u8 data);
	void attr_w(offs_t offset, u8 data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	TILEMAP_MAPPER_MEMBER(tilemap_scan_rows_extra);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(vblank_irq);
	INTERRUPT_GEN_MEMBER(sound_timer_irq);

	void main_map(address_map &map);
	void sound_map(address_map &map);
};


void shettle_state::palette_init(palette_device &palette) const
{
	const u8 *color_prom = memregion("proms")->base();
	static constexpr int resistances_rg[3] = { 1000, 470, 220 };
	static constexpr int resistances_b [2] = { 470, 220 };

	// compute the color output resistor weights
	double rweights[3], gweights[3], bweights[2];
	compute_resistor_weights(0, 255, -1.0,
			3, &resistances_rg[0], rweights, 470, 0,
			3, &resistances_rg[0], gweights, 470, 0,
			2, &resistances_b[0],  bweights, 470, 0);

	// create a lookup table for the palette
	for (int i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = combine_weights(rweights, bit0, bit1, bit2);

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = combine_weights(gweights, bit0, bit1, bit2);

		// blue component
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const b = combine_weights(bweights, bit0, bit1);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x20;

	// chars and sprites use colors 0-15
	for (int i = 0; i < 0x200; i++)
	{
		u8 const ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}
}

TILE_GET_INFO_MEMBER(shettle_state::get_tile_info)
{
	const u8 tile = m_videoram[tile_index];
	const u8 attr = m_colorram[tile_index];
	// unused by the game, ported from wiping.cpp
	tileinfo.group = (attr >> 7) & 1;
	tileinfo.set(0, tile, attr & 0x3f, 0);
}

// same as clshroad.cpp:
// flips scanning per-column instead of per-row on left/right 16 pixels, for a 288 layout
TILEMAP_MAPPER_MEMBER(shettle_state::tilemap_scan_rows_extra)
{
	if (col <= 0x01)    return row + (col + 0x1e) * 0x20;
	if (col >= 0x22)    return row + (col - 0x22) * 0x20;

	if (row <= 0x01)    return 0;
	if (row >= 0x1e)    return 0;

	return (col - 2) + row * 0x20;
}


void shettle_state::vram_w(offs_t offset, u8 data)
{
	m_videoram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

void shettle_state::attr_w(offs_t offset, u8 data)
{
	m_colorram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

void shettle_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(shettle_state::get_tile_info)), tilemap_mapper_delegate(*this, FUNC(shettle_state::tilemap_scan_rows_extra)), 8, 8, 36, 32);
	m_tilemap->set_scrolldy(-16, -16);
}

u32 shettle_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);

	// top to bottom, ensure bridges goes behind player/enemy sprites
	for (int offs = 0; offs < m_spriteram.bytes(); offs += 8)
	{
		int x = m_spriteram[offs + 3] + 0x0b;
		int y = 224 - m_spriteram[offs + 2];

		u8 color = m_spriteram[offs + 1] & 0x3f;

		// color entry 0 seems fully transparent
		// (would otherwise "stick" bonus points after using yoyo, transmask?)
		if (!color)
			continue;

		u16 code = (m_spriteram[offs + 0] & 0x3f) | (m_spriteram[offs + 4] << 6);
		int flipx = m_spriteram[offs + 0] & 0x80;
		int flipy = m_spriteram[offs + 0] & 0x40;

		if (flip_screen())
		{
			y = 209 - y;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, code, color, flipx, flipy, x, y, 3);
	}

	m_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);

	return 0;
}


void shettle_state::machine_start()
{
	save_item(NAME(m_main_irq_mask));
	save_item(NAME(m_sound_irq_mask));
}

u8 shettle_state::ports_r(offs_t offset)
{
	int res = 0;

	for (int i = 0; i < 4; i++)
		res |= BIT(m_io_port[i]->read(), offset) << i;

	return res | 0xf0;
}

// irq / reset controls like in clshroad.cpp

void shettle_state::main_irq_mask_w(int state)
{
	m_main_irq_mask = state;
}

void shettle_state::sound_irq_mask_w(int state)
{
	m_sound_irq_mask = state;
}

INTERRUPT_GEN_MEMBER(shettle_state::vblank_irq)
{
	if (m_main_irq_mask)
		device.execute().set_input_line(0, HOLD_LINE);
}

INTERRUPT_GEN_MEMBER(shettle_state::sound_timer_irq)
{
	if (m_sound_irq_mask)
		device.execute().set_input_line(0, HOLD_LINE);
}


void shettle_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom(); // TODO: some reads after 0x5fff. Interactions with the Z4 chip?
	map(0x8000, 0x83ff).ram().w(FUNC(shettle_state::vram_w)).share(m_videoram);
	map(0x8400, 0x87ff).ram().w(FUNC(shettle_state::attr_w)).share(m_colorram);
	map(0x8e00, 0x8fff).ram().share(m_spriteram);
	map(0x9000, 0x93ff).ram();
	map(0x9600, 0x97ff).ram().share("main_sound");
	map(0xa000, 0xa007).w("mainlatch", FUNC(ls259_device::write_d0));
	map(0xa100, 0xa107).r(FUNC(shettle_state::ports_r));
}

void shettle_state::sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x4000, 0x7fff).w("wiping", FUNC(wiping_sound_device::sound_w));
	map(0x9600, 0x97ff).ram().share("main_sound");
	map(0xa000, 0xa007).w("mainlatch", FUNC(ls259_device::write_d0));
}


static INPUT_PORTS_START( shettle )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Infinite Lives (Cheat)" )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x05, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW2:1,3")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME (0x02, 0x02, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(0,1), STEP4(8,1) },
	{ STEP8(0,8*2) },
	8*8*2
};

static const gfx_layout spritelayout =
{
	16,16,  // 16*16 sprites
	128,    // 128 sprites
	2,  // 2 bits per pixel
	{ 0, 4 },   // the two bitplanes are packed in one byte
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 17*8+0, 17*8+1, 17*8+2, 17*8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			16*16, 17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16 },
	64*8    // every sprite takes 64 consecutive bytes
};

static GFXDECODE_START( gfx_wiping )
	GFXDECODE_ENTRY( "chars",   0, charlayout,       0, 64 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout, 0x100, 64 )
GFXDECODE_END


void shettle_state::shettle(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 18.432_MHz_XTAL / 6); // 3.072 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &shettle_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(shettle_state::vblank_irq));

	Z80(config, m_audiocpu, 18.432_MHz_XTAL / 6); // 3.072 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &shettle_state::sound_map);
	m_audiocpu->set_periodic_int(FUNC(shettle_state::sound_timer_irq), attotime::from_hz(120));    // periodic interrupt, don't know about the frequency

	config.set_maximum_quantum(attotime::from_hz(18.432_MHz_XTAL / 6 / 512)); // 6000 Hz

	ls259_device &mainlatch(LS259(config, "mainlatch")); // 5A
	mainlatch.q_out_cb<0>().set_inputline(m_audiocpu, INPUT_LINE_RESET).invert();
	mainlatch.q_out_cb<1>().set(FUNC(shettle_state::main_irq_mask_w));
	mainlatch.q_out_cb<3>().set(FUNC(shettle_state::sound_irq_mask_w));
	mainlatch.q_out_cb<4>().set(FUNC(shettle_state::flip_screen_set));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(18.432_MHz_XTAL / 3, 384, 0, 288, 264, 0, 224); // unknown, single XTAL on PCB & 288x224 suggests 60.606060 Hz like Galaxian HW
	screen.set_screen_update(FUNC(shettle_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_wiping);
	PALETTE(config, m_palette, FUNC(shettle_state::palette_init), 256 + 256, 32);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	WIPING_CUSTOM(config, "wiping", 96'000 / 2).add_route(ALL_OUTPUTS, "mono", 1.0); // 48000 Hz?
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( shettle )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "1.bin", 0x00000, 0x2000, CRC(e2b25df4) SHA1(781e09ca2ef03ded468b768261074f6e4a25720b) )
	ROM_LOAD( "2.bin", 0x02000, 0x2000, CRC(a24bf4ad) SHA1(fbe00dfb6ce2306c59e459440f403c3c5f49bdd3) )
	ROM_LOAD( "3.bin", 0x04000, 0x2000, CRC(b88e8213) SHA1(e3745ad1c25eaf8019dd9d46e3480f2ca8c5a7cf) )
	ROM_FILL(          0x06000, 0x2000, 0x00 ) // Accessed, could this range map to the Z4 device ?!

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "d.bin", 0x0000, 0x2000, CRC(1e2e7365) SHA1(ad6d0c94d5cb172d3a29523706ccd901a72e90be) )

	ROM_REGION( 0x1000, "chars", ROMREGION_INVERT )
	ROM_LOAD( "0.5d", 0x0000, 0x1000, CRC(fa6261da) SHA1(e7ab7eb2ab2ba2497d06606861a804d317d306ff) )

	ROM_REGION( 0x2000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "e.bin", 0x0000, 0x2000, CRC(a3cef381) SHA1(ed511f5b695f0abdbaea8414d9de260f696f5318) )

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "prom-4.2f",  0x0000, 0x0020, CRC(befab139) SHA1(748c49437067d2d0a99b359bb5d53841a22b4760) ) // MMI 6331 - palette
	ROM_LOAD( "prom-6.4h",  0x0020, 0x0100, CRC(1abbc864) SHA1(a28d35cb2492f74f847858475aef669c38c3574a) ) // char lookup table (near 0.5d ROM)
	ROM_LOAD( "prom-5.3r",  0x0120, 0x0100, CRC(0f64edb9) SHA1(e1bc4acc0778ca13a3a2b8caa653bbf54a3507f9) ) // sprite lookup table (next to e.4r ROM)
	ROM_LOAD( "prom-7.7b",  0x0220, 0x0100, CRC(9e824f74) SHA1(03fcde2546b87286038ef93a6939c1c325f74998) ) // unknown (almost identical to clshroad.g10 in clshroad.cpp)
	ROM_LOAD( "prom-1.bin", 0x0320, 0x0020, CRC(1afc04f0) SHA1(38207cf3e15bac7034ac06469b95708d22b57da4) ) // MMI 6331 - timing? (same as clashrd.g4 in clshroad.cpp)

	ROM_REGION( 0x2000, "wiping:samples", 0 )
	ROM_LOAD( "4.bin", 0x0000, 0x2000, CRC(c9da4245) SHA1(961c3b52b7608a35493d753a3b482713198fd2eb) )

	ROM_REGION( 0x0200, "wiping:soundproms", 0 ) // 4bit->8bit sample expansion PROMs
	ROM_LOAD( "prom-2.bin", 0x0000, 0x0100, CRC(bd2c080b) SHA1(9782bb5001e96db56bc29df398187f700bce4f8e) ) // low 4 bits, same as wiping, clshroad, firebatl
	ROM_LOAD( "prom-3.bin", 0x0100, 0x0100, CRC(4017a2a6) SHA1(dadef2de7a1119758c8e6d397aa42815b0218889) ) // high 4 bits, same as wiping, clshroad, firebatl
ROM_END

} // anonymous namespace


GAME( 1984, shettle, 0,      shettle, shettle, shettle_state, empty_init, ROT90, "New Digimatic", "Alone Shettle Crew", MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

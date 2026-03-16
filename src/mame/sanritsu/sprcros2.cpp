// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/*

Super Cross II 『スーパークロスⅡ』 (c) 1986 GM Shoji

driver by Angelo Salese, based on "wiped off due to not anymore licenseable"
driver by insideoutboy.

TODO:
- Verify screen timing, it's either 320*264, or if bankp is accurate, then it's
  330*256. Visible width is probably correct.
- Verify Z80 frequency, it runs too laggy at 2.5MHz which isn't evident on PCB
  video, and Z80B is rated 6MHz. Maybe there are waitstates though?
  Also note that if it's clocked at 2.5MHz, it will sometimes write wrong tiles
  to bg_vram, see MT 09395.

BTANB:
- occasional 1-frame sprite glitches at the edge of the screen

The hardware has similarities with Sanritsu's Bank Panic, PCB label is similar
as well: C2-00170-A for Bank Panic, C2-00171-D/C2-00172-D for Super Cross II.
There's also an unused SEGA logo in the gfx0 region.

GM Shoji is not a game developer. It's probably an old Sanritsu game that was
canceled/rejected by Sega. Sanritsu's involvement is also mentioned in a
preview in the Japanese magazine GAME FREAK Vol 21.

===================================

Super Cross II (JPN Ver.)
(c)1986 GM Shoji

C2-00172-D
CPU  :Z80B
Sound:SN76489 x3

SCS-24.4E
SCS-25.4C
SCS-26.4B
SCS-27.5K
SCS-28.5J
SCS-29.5H
SCS-30.5F

SC-62.3A
SC-63.3B
SC-64.6A

C2-00171-D
CPU  :Z80B
OSC  :10.000MHz

SCM-00.10L
SCM-01.10K
SCM-02.10J
SCM-03.10G
SCM-20.5K
SCM-21.5G
SCM-22.5E
SCM-23.5B

SC-60.4K
SC-61.5A

*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/timer.h"
#include "sound/sn76496.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class sprcros2_state : public driver_device
{
public:
	sprcros2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
		, m_screen(*this, "screen")
		, m_gfxdecode(*this, "gfxdecode")
		, m_mainbank(*this, "mainbank")
		, m_subbank(*this, "subbank")
		, m_fg_vram(*this, "fg_vram")
		, m_fg_attr(*this, "fg_attr")
		, m_bg_vram(*this, "bg_vram")
		, m_bg_attr(*this, "bg_attr")
		, m_spriteram(*this, "spriteram")
	{ }

	void sprcros2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void video_start() override ATTR_COLD;

private:
	// devices
	required_device<z80_device> m_maincpu;
	required_device<z80_device> m_subcpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;

	required_memory_bank m_mainbank;
	required_memory_bank m_subbank;

	required_shared_ptr<uint8_t> m_fg_vram;
	required_shared_ptr<uint8_t> m_fg_attr;
	required_shared_ptr<uint8_t> m_bg_vram;
	required_shared_ptr<uint8_t> m_bg_attr;
	required_shared_ptr<uint8_t> m_spriteram;

	// misc
	bool m_main_nmi_enable = false;
	bool m_main_irq_enable = false;
	bool m_sub_nmi_enable = false;
	bool m_screen_enable = false;
	uint8_t m_bg_scrollx = 0;
	uint8_t m_bg_scrolly = 0;
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;

	// handlers
	void palette(palette_device &palette) const;
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(main_vblank_irq);
	INTERRUPT_GEN_MEMBER(sub_vblank_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	void bg_vram_w(offs_t offset, uint8_t data);
	void bg_attr_w(offs_t offset, uint8_t data);
	void fg_vram_w(offs_t offset, uint8_t data);
	void fg_attr_w(offs_t offset, uint8_t data);
	void bg_scrollx_w(uint8_t data);
	void bg_scrolly_w(uint8_t data);
	void main_output_w(uint8_t data);
	void sub_output_w(uint8_t data);

	void main_io(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void sub_io(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;
};


/*******************************************************************************

  Machine initialization

*******************************************************************************/

void sprcros2_state::machine_start()
{
	m_mainbank->configure_entries(0, 2, memregion("maincpu")->base() + 0xc000, 0x2000);
	m_subbank->configure_entries(0, 2, memregion("subcpu")->base() + 0xc000, 0x2000);

	save_item(NAME(m_main_nmi_enable));
	save_item(NAME(m_main_irq_enable));
	save_item(NAME(m_sub_nmi_enable));
	save_item(NAME(m_screen_enable));
	save_item(NAME(m_bg_scrollx));
	save_item(NAME(m_bg_scrolly));
}

void sprcros2_state::machine_reset()
{
	main_output_w(0);
	sub_output_w(0);
}


/*******************************************************************************

  Video initialization

*******************************************************************************/

void sprcros2_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	// create a lookup table for the palette
	for (int i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// blue component
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const b = 0x47 * bit0 + 0xb8 * bit1;

		palette.set_pen_color(i, rgb_t(r, g, b));
		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x20;

	// bg
	for (int i = 0; i < 0x100; i++)
	{
		uint8_t const ctabentry = (color_prom[i] & 0x0f) | ((color_prom[i + 0x100] & 0x0f) << 4);
		palette.set_pen_indirect(i, ctabentry);
	}

	// sprites & fg
	for (int i = 0x100; i < 0x300; i++)
	{
		uint8_t ctabentry = color_prom[i + 0x100];
		palette.set_pen_indirect(i, ctabentry);
	}
}

TILE_GET_INFO_MEMBER(sprcros2_state::get_bg_tile_info)
{
	const int code = m_bg_vram[tile_index] | (m_bg_attr[tile_index] & 0x07) << 8;
	const int color = m_bg_attr[tile_index] >> 4 & 0x0f;
	const int flags = (m_bg_attr[tile_index] & 0x08) ? TILE_FLIPX : 0;

	tileinfo.set(0, code, color, flags);
}

TILE_GET_INFO_MEMBER(sprcros2_state::get_fg_tile_info)
{
	const int code = m_fg_vram[tile_index] | (m_fg_attr[tile_index] & 0x03) << 8;
	const int color = m_fg_attr[tile_index] >> 2 & 0x3f;

	tileinfo.set(2, code, color, 0);
	tileinfo.group = color;
}

void sprcros2_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sprcros2_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sprcros2_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_tilemap->configure_groups(*m_gfxdecode->gfx(2), 0);
}


/*******************************************************************************

  Display refresh

*******************************************************************************/

void sprcros2_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int i = m_spriteram.bytes() - 4; i >= 0; i -= 4)
	{
		int y = ((224 - m_spriteram[i + 2]) & 0xff) + 1;
		int x = m_spriteram[i + 3];
		int code = m_spriteram[i + 0] & 0x7f;
		int flipx = BIT(m_spriteram[i + 1], 1);
		int flipy = 0;
		int color = (m_spriteram[i + 1] & 0x38) >> 3;

		if (flip_screen())
		{
			x = 224 - x;
			y = 224 - y;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->transpen(
				bitmap, cliprect,
				code, color,
				flipx, flipy,
				x, y, 0);
	}
}

uint32_t sprcros2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!m_screen_enable)
	{
		bitmap.fill(0, cliprect);
		return 0;
	}

	m_bg_tilemap->set_scrollx(0, flip_screen() ? 256 - m_bg_scrollx : m_bg_scrollx);
	m_bg_tilemap->set_scrolly(0, m_bg_scrolly);

	m_bg_tilemap->draw(screen, bitmap, cliprect);
	draw_sprites(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect);

	return 0;
}


/*******************************************************************************

  Interrupts

*******************************************************************************/

INTERRUPT_GEN_MEMBER(sprcros2_state::main_vblank_irq)
{
	if (m_main_nmi_enable)
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

INTERRUPT_GEN_MEMBER(sprcros2_state::sub_vblank_irq)
{
	if (m_sub_nmi_enable)
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

TIMER_DEVICE_CALLBACK_MEMBER(sprcros2_state::scanline)
{
	const int scanline = param;

	// 2 sound interrupts per frame
	if ((scanline & 0x7f) == 0x40 && m_main_irq_enable)
		m_maincpu->set_input_line(0, HOLD_LINE);
}


/*******************************************************************************

  Memory & I/O handlers

*******************************************************************************/

void sprcros2_state::bg_vram_w(offs_t offset, uint8_t data)
{
	m_bg_vram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void sprcros2_state::bg_attr_w(offs_t offset, uint8_t data)
{
	m_bg_attr[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void sprcros2_state::fg_vram_w(offs_t offset, uint8_t data)
{
	m_fg_vram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void sprcros2_state::fg_attr_w(offs_t offset, uint8_t data)
{
	m_fg_attr[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void sprcros2_state::bg_scrollx_w(uint8_t data)
{
	m_bg_scrollx = data;
}

void sprcros2_state::bg_scrolly_w(uint8_t data)
{
	m_bg_scrolly = data;
}


void sprcros2_state::main_output_w(uint8_t data)
{
	// bit 0: enable vblank NMI
	m_main_nmi_enable = BIT(data, 0);

	// bit 1: flip screen
	flip_screen_set(BIT(data, 1));

	// bit 2: enable display
	m_screen_enable = BIT(data, 2);

	// bit 3: enable IRQ
	m_main_irq_enable = BIT(data, 3);

	// bit 6: bankswitch
	m_mainbank->set_entry(BIT(data, 6));

	// other: unused
	//if (data & 0xb0) printf("main 07 -> %02x\n",data);
}

void sprcros2_state::sub_output_w(uint8_t data)
{
	// bit 0: enable vblank NMI
	m_sub_nmi_enable = BIT(data, 0);

	// bit 3: bankswitch
	m_subbank->set_entry(BIT(data, 3));

	// other: unused
	//if (data & 0xf6) printf("sub 03 -> %02x\n",data);
}


/*******************************************************************************

  Address maps

*******************************************************************************/

void sprcros2_state::main_map(address_map &map)
{
	map(0x0000, 0xbfff).rom().region("maincpu", 0);
	map(0xc000, 0xdfff).bankr(m_mainbank);
	map(0xe000, 0xe3ff).ram().w(FUNC(sprcros2_state::fg_vram_w)).share(m_fg_vram);
	map(0xe400, 0xe7ff).ram().w(FUNC(sprcros2_state::fg_attr_w)).share(m_fg_attr);
	map(0xe800, 0xe83f).ram().share(m_spriteram);
	map(0xe840, 0xf7ff).ram();
	map(0xf800, 0xffff).ram().share("shared_ram");
}

void sprcros2_state::main_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("P1").w("sn1", FUNC(sn76489_device::write));
	map(0x01, 0x01).portr("P2").w("sn2", FUNC(sn76489_device::write));
	map(0x02, 0x02).portr("EXTRA").w("sn3", FUNC(sn76489_device::write));
	map(0x04, 0x04).portr("DSW1");
	map(0x05, 0x05).portr("DSW2");
	map(0x07, 0x07).w(FUNC(sprcros2_state::main_output_w));
}

void sprcros2_state::sub_map(address_map &map)
{
	map(0x0000, 0xbfff).rom().region("subcpu", 0);
	map(0xc000, 0xdfff).bankr(m_subbank);
	map(0xe000, 0xe3ff).ram().w(FUNC(sprcros2_state::bg_vram_w)).share(m_bg_vram);
	map(0xe400, 0xe7ff).ram().w(FUNC(sprcros2_state::bg_attr_w)).share(m_bg_attr);
	map(0xe800, 0xf7ff).ram();
	map(0xf800, 0xffff).ram().share("shared_ram");
}

void sprcros2_state::sub_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(sprcros2_state::bg_scrollx_w));
	map(0x01, 0x01).w(FUNC(sprcros2_state::bg_scrolly_w));
	map(0x03, 0x03).w(FUNC(sprcros2_state::sub_output_w));
}


/*******************************************************************************

  Input ports

*******************************************************************************/

static INPUT_PORTS_START( sprcros2 )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("EXTRA")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(    0x07, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x00, "SW1:!4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x00, "SW1:!5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "SW1:!6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW1:!7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "SW1:!8" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:!1")
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x00, "SW2:!2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x00, "SW2:!3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x00, "SW2:!4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x00, "SW2:!5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "SW2:!6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW2:!7" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


/*******************************************************************************

  GFX layouts

*******************************************************************************/

static const gfx_layout sprite_layout =
{
	32,32,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ STEP8(0,1), STEP8(256,1), STEP8(512,1), STEP8(768,1) },
	{ STEP16(0,8), STEP16(128,8) },
	32*32
};

static const gfx_layout fgtiles_layout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(64,1), STEP4(0,1) },
	{ STEP8(0,8) },
	8*8*2
};

static GFXDECODE_START( gfx_sprcros2 )
	GFXDECODE_ENTRY( "bgtiles", 0, gfx_8x8x3_planar, 0,   16 )
	GFXDECODE_ENTRY( "sprites", 0, sprite_layout,    256, 8 )
	GFXDECODE_ENTRY( "fgtiles", 0, fgtiles_layout,   512, 64 )
GFXDECODE_END


/*******************************************************************************

  Machine configs

*******************************************************************************/

void sprcros2_state::sprcros2(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 10_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &sprcros2_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &sprcros2_state::main_io);
	m_maincpu->set_vblank_int("screen", FUNC(sprcros2_state::main_vblank_irq));

	TIMER(config, "scantimer").configure_scanline(FUNC(sprcros2_state::scanline), "screen", 0, 1);

	Z80(config, m_subcpu, 10_MHz_XTAL / 2);
	m_subcpu->set_addrmap(AS_PROGRAM, &sprcros2_state::sub_map);
	m_subcpu->set_addrmap(AS_IO, &sprcros2_state::sub_io);
	m_subcpu->set_vblank_int("screen", FUNC(sprcros2_state::sub_vblank_irq));

	config.set_maximum_quantum(attotime::from_hz(m_maincpu->clock() / 4));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(sprcros2_state::screen_update));
	m_screen->set_raw(10_MHz_XTAL / 2, 320, 8, 256-8, 264, 16, 240); // measured ~59.2Hz
	m_screen->set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_sprcros2);

	PALETTE(config, "palette", FUNC(sprcros2_state::palette), 768, 32);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	SN76489(config, "sn1", 10_MHz_XTAL / 4).add_route(ALL_OUTPUTS, "mono", 0.50);
	SN76489(config, "sn2", 10_MHz_XTAL / 4).add_route(ALL_OUTPUTS, "mono", 0.50);
	SN76489(config, "sn3", 10_MHz_XTAL / 4).add_route(ALL_OUTPUTS, "mono", 0.50);
}


/*******************************************************************************

  ROM definitions

*******************************************************************************/

ROM_START( sprcros2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "scm-03.10g", 0x0000, 0x4000, CRC(b9757908) SHA1(d59cb2aac1b6268fc766306850f5711d4a12d897) )
	ROM_LOAD( "scm-02.10j", 0x4000, 0x4000, CRC(849c5c87) SHA1(0e02c4990e371d6a290efa53301818e769648945) )
	ROM_LOAD( "scm-01.10k", 0x8000, 0x4000, CRC(385a62de) SHA1(847bf9d97ab3fa8949d9198e4e509948a940d6aa) )
	ROM_LOAD( "scm-00.10l", 0xc000, 0x4000, CRC(13fa3684) SHA1(611b7a237e394f285dcc5beb027dacdbdd58a7a0) )

	ROM_REGION( 0x10000, "subcpu", 0 )
	ROM_LOAD( "scs-30.5f",  0x0000, 0x4000, CRC(c0a40e41) SHA1(e74131b353855749258dffa45091c825ccdbf05a) )
	ROM_LOAD( "scs-29.5h",  0x4000, 0x4000, CRC(83d49fa5) SHA1(7112110df2f382bbc0e651adcec975054a485b9b) )
	ROM_LOAD( "scs-28.5j",  0x8000, 0x4000, CRC(480d351f) SHA1(d1b86f441ae0e58b30e0f089ab25de219d5f30e3) )
	ROM_LOAD( "scs-27.5k",  0xc000, 0x4000, CRC(2cf720cb) SHA1(a95c5b8c88371cf597bb7d80afeca6a48c7b74e6) )

	ROM_REGION( 0xc000, "bgtiles", 0 )
	ROM_LOAD( "scs-26.4b",  0x8000, 0x4000, CRC(f958b56d) SHA1(a1973179d336d2ba57294155550515f2b8a33a09) )
	ROM_LOAD( "scs-25.4c",  0x4000, 0x4000, CRC(d6fd7ba5) SHA1(1c26c4c1655b2be9cb6103e75386cc2f0cf27fc5) )
	ROM_LOAD( "scs-24.4e",  0x0000, 0x4000, CRC(87783c36) SHA1(7102be795afcddd76b4d41823e95c65fe1ffbca0) )

	ROM_REGION( 0xc000, "sprites", 0 )
	ROM_LOAD( "scm-23.5b",  0x0000, 0x4000, CRC(ab42f8e3) SHA1(8c2213b7c47a48e223fc3f7d323d16c0e4cd0457) )
	ROM_LOAD( "scm-22.5e",  0x4000, 0x4000, CRC(0cad254c) SHA1(36e30e30b652b3a388a3c4a82251196f79368f59) )
	ROM_LOAD( "scm-21.5g",  0x8000, 0x4000, CRC(b6b68998) SHA1(cc3c6d996beeedcc7e5199f10d65c5b1d3c6e666) )

	ROM_REGION( 0x4000, "fgtiles", 0 )
	ROM_LOAD( "scm-20.5k",  0x0000, 0x4000, CRC(67a099a6) SHA1(43981abdcaa0ff36183027a3c691ce2df7f06ec7) )

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "sc-64.6a",   0x0000, 0x0020, CRC(336dd1c0) SHA1(f0a0d2c13617fd84ee55c0cb96643761a8735147) ) // palette
	ROM_LOAD( "sc-63.3b",   0x0020, 0x0100, CRC(9034a059) SHA1(1801965b4f0f3e04ca4b3faf0ba3a27dbb008474) ) // bg clut lo nibble
	ROM_LOAD( "sc-62.3a",   0x0120, 0x0100, CRC(3c78a14f) SHA1(8f9c196a3e18bdce2d4855bc285bd5bde534bf09) ) // bg clut hi nibble
	ROM_LOAD( "sc-61.5a",   0x0220, 0x0100, CRC(2f71185d) SHA1(974fbb52285f01f4353e9acb1992dcd6fdefedcb) ) // sprite clut
	ROM_LOAD( "sc-60.4k",   0x0320, 0x0100, CRC(d7a4e57d) SHA1(6db02ec6aa55b05422cb505e63c71e36b4b11b4a) ) // fg clut
ROM_END

ROM_START( sprcros2a ) // bootleg PCB, but ROMs look untampered, just an earlier revision
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "15.bin",     0x0000, 0x4000, CRC(b9d02558) SHA1(775404c6c7648d9dab02b496541739ea700cd481) )
	ROM_LOAD( "scm-02.10j", 0x4000, 0x4000, CRC(849c5c87) SHA1(0e02c4990e371d6a290efa53301818e769648945) )
	ROM_LOAD( "scm-01.10k", 0x8000, 0x4000, CRC(385a62de) SHA1(847bf9d97ab3fa8949d9198e4e509948a940d6aa) )
	ROM_LOAD( "scm-00.10l", 0xc000, 0x4000, CRC(13fa3684) SHA1(611b7a237e394f285dcc5beb027dacdbdd58a7a0) )

	ROM_REGION( 0x10000, "subcpu", 0 )
	ROM_LOAD( "scs-30.5f",  0x0000, 0x4000, CRC(c0a40e41) SHA1(e74131b353855749258dffa45091c825ccdbf05a) )
	ROM_LOAD( "scs-29.5h",  0x4000, 0x4000, CRC(83d49fa5) SHA1(7112110df2f382bbc0e651adcec975054a485b9b) )
	ROM_LOAD( "scs-28.5j",  0x8000, 0x4000, CRC(480d351f) SHA1(d1b86f441ae0e58b30e0f089ab25de219d5f30e3) )
	ROM_LOAD( "scs-27.5k",  0xc000, 0x4000, CRC(2cf720cb) SHA1(a95c5b8c88371cf597bb7d80afeca6a48c7b74e6) )

	ROM_REGION( 0xc000, "bgtiles", 0 )
	ROM_LOAD( "scs-26.4b",  0x8000, 0x4000, CRC(f958b56d) SHA1(a1973179d336d2ba57294155550515f2b8a33a09) )
	ROM_LOAD( "scs-25.4c",  0x4000, 0x4000, CRC(d6fd7ba5) SHA1(1c26c4c1655b2be9cb6103e75386cc2f0cf27fc5) )
	ROM_LOAD( "scs-24.4e",  0x0000, 0x4000, CRC(87783c36) SHA1(7102be795afcddd76b4d41823e95c65fe1ffbca0) )

	ROM_REGION( 0xc000, "sprites", 0 )
	ROM_LOAD( "scm-23.5b",  0x0000, 0x4000, CRC(ab42f8e3) SHA1(8c2213b7c47a48e223fc3f7d323d16c0e4cd0457) )
	ROM_LOAD( "scm-22.5e",  0x4000, 0x4000, CRC(0cad254c) SHA1(36e30e30b652b3a388a3c4a82251196f79368f59) )
	ROM_LOAD( "scm-21.5g",  0x8000, 0x4000, CRC(b6b68998) SHA1(cc3c6d996beeedcc7e5199f10d65c5b1d3c6e666) )

	ROM_REGION( 0x4000, "fgtiles", 0 )
	ROM_LOAD( "scm-20.5k",  0x0000, 0x4000, CRC(67a099a6) SHA1(43981abdcaa0ff36183027a3c691ce2df7f06ec7) )

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "sc-64.6a",   0x0000, 0x0020, CRC(336dd1c0) SHA1(f0a0d2c13617fd84ee55c0cb96643761a8735147) ) // palette
	ROM_LOAD( "sc-63.3b",   0x0020, 0x0100, CRC(9034a059) SHA1(1801965b4f0f3e04ca4b3faf0ba3a27dbb008474) ) // bg clut lo nibble
	ROM_LOAD( "sc-62.3a",   0x0120, 0x0100, CRC(3c78a14f) SHA1(8f9c196a3e18bdce2d4855bc285bd5bde534bf09) ) // bg clut hi nibble
	ROM_LOAD( "sc-61.5a",   0x0220, 0x0100, CRC(2f71185d) SHA1(974fbb52285f01f4353e9acb1992dcd6fdefedcb) ) // sprite clut
	ROM_LOAD( "sc-60.4k",   0x0320, 0x0100, CRC(d7a4e57d) SHA1(6db02ec6aa55b05422cb505e63c71e36b4b11b4a) ) // fg clut
ROM_END

} // anonymous namespace


/*******************************************************************************

  Game drivers

*******************************************************************************/

GAME( 1986, sprcros2,  0,        sprcros2, sprcros2, sprcros2_state, empty_init, ROT0, "Sanritsu / GM Shoji", "Super Cross II (Japan, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, sprcros2a, sprcros2, sprcros2, sprcros2, sprcros2_state, empty_init, ROT0, "Sanritsu / GM Shoji", "Super Cross II (Japan, set 2)", MACHINE_SUPPORTS_SAVE )

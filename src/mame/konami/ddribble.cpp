// license:BSD-3-Clause
// copyright-holders: Manuel Abadia
/***************************************************************************

    Double Dribble (GX690) (c) Konami 1986

    Driver by Manuel Abadia <emumanu+mame@gmail.com>

    2008-08
    Dip locations and suggested settings verified with US manual.

    TODO: using a debug build, the cmd prompt is filled with sound_assert: u32(start) < samples()

***************************************************************************/

#include "emu.h"

#include "konamipt.h"

#include "cpu/m6809/m6809.h"
#include "machine/watchdog.h"
#include "sound/flt_rc.h"
#include "sound/vlm5030.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class ddribble_state : public driver_device
{
public:
	ddribble_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_fg_videoram(*this, "fg_videoram"),
		m_spriteram(*this, "spriteram_%u", 1U),
		m_bg_videoram(*this, "bg_videoram"),
		m_mainbank(*this, "mainbank"),
		m_vlmbank(*this, "vlmbank"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_vlm(*this, "vlm"),
		m_filter(*this, "filter%u", 1U),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void ddribble(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_fg_videoram;
	required_shared_ptr_array<uint8_t, 2> m_spriteram;
	required_shared_ptr<uint8_t> m_bg_videoram;
	required_memory_bank m_mainbank;
	required_memory_bank m_vlmbank;

	// video-related
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_vregs[2][5]{};
	uint8_t m_charbank[2]{};

	// misc
	uint8_t  m_int_enable[2]{};

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<vlm5030_device> m_vlm;
	required_device_array<filter_rc_device, 3> m_filter;
	required_device<gfxdecode_device> m_gfxdecode;

	void bankswitch_w(uint8_t data);
	void coin_counter_w(uint8_t data);
	template <uint8_t Which> void k005885_w(offs_t offset, uint8_t data);
	void k005885_1_w(offs_t offset, uint8_t data);
	void fg_videoram_w(offs_t offset, uint8_t data);
	void bg_videoram_w(offs_t offset, uint8_t data);
	uint8_t vlm5030_busy_r();
	void vlm5030_ctrl_w(uint8_t data);
	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(int state);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t* source, int lenght, int gfxset, int flipscreen);
	void maincpu_map(address_map &map) ATTR_COLD;
	void subcpu_map(address_map &map) ATTR_COLD;
	void audiocpu_map(address_map &map) ATTR_COLD;
	void vlm_map(address_map &map) ATTR_COLD;
};


void ddribble_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0x10; i < 0x40; i++)
		palette.set_pen_indirect(i, i);

	// sprite #2 uses pens 0x00-0x0f
	for (int i = 0x0; i < 0x100; i++)
	{
		uint8_t const ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i + 0x40, ctabentry);
	}
}


template <uint8_t Which>
void ddribble_state::k005885_w(offs_t offset, uint8_t data) // TODO: K005885 should be device-ified
{
	switch (offset)
	{
		case 0x03:  // char bank selection
			if ((data & 0x03) != m_charbank[Which])
			{
				m_charbank[Which] = data & 0x03;
				Which ? m_bg_tilemap->mark_all_dirty() : m_fg_tilemap->mark_all_dirty();
			}
			break;
		case 0x04:  // IRQ control, flipscreen
			m_int_enable[Which] = data & 0x02;
			break;
	}
	m_vregs[Which][offset] = data;
}

/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

TILEMAP_MAPPER_MEMBER(ddribble_state::tilemap_scan)
{
	// logical (col,row) -> memory offset
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x20) << 6);    // skip 0x400
}

TILE_GET_INFO_MEMBER(ddribble_state::get_fg_tile_info)
{
	uint8_t const attr = m_fg_videoram[tile_index];
	int const num = m_fg_videoram[tile_index + 0x400] + ((attr & 0xc0) << 2) + ((attr & 0x20) << 5) + ((m_charbank[0] & 2) << 10);
	tileinfo.set(0,
			num,
			0,
			TILE_FLIPYX((attr & 0x30) >> 4));
}

TILE_GET_INFO_MEMBER(ddribble_state::get_bg_tile_info)
{
	uint8_t const attr = m_bg_videoram[tile_index];
	int const num = m_bg_videoram[tile_index + 0x400] + ((attr & 0xc0) << 2) + ((attr & 0x20) << 5) + (m_charbank[1] << 11);
	tileinfo.set(1,
			num,
			0,
			TILE_FLIPYX((attr & 0x30) >> 4));
}

/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void ddribble_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ddribble_state::get_fg_tile_info)), tilemap_mapper_delegate(*this, FUNC(ddribble_state::tilemap_scan)), 8, 8, 64, 32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ddribble_state::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(ddribble_state::tilemap_scan)), 8, 8, 64, 32);

	m_fg_tilemap->set_transparent_pen(0);
}

/***************************************************************************

    Memory handlers

***************************************************************************/

void ddribble_state::fg_videoram_w(offs_t offset, uint8_t data)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0xbff);
}

void ddribble_state::bg_videoram_w(offs_t offset, uint8_t data)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0xbff);
}

/***************************************************************************

    Double Dribble sprites

Each sprite has 5 bytes:
byte #0:    sprite number
byte #1:
    bits 0..2:  sprite bank #
    bit 3:      not used?
    bits 4..7:  sprite color
byte #2:    y position
byte #3:    x position
byte #4:    attributes
    bit 0:      x position (high bit)
    bit 1:      ???
    bits 2..4:  sprite size
    bit 5:      flip x
    bit 6:      flip y
    bit 7:      unused?

***************************************************************************/

void ddribble_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t* source, int lenght, int gfxset, int flipscreen)
{
	gfx_element *gfx = m_gfxdecode->gfx(gfxset);
	const uint8_t *finish = source + lenght;

	while (source < finish)
	{
		int number = source[0] | ((source[1] & 0x07) << 8);       // sprite number
		int const attr = source[4];                               // attributes
		int sx = source[3] | ((attr & 0x01) << 8);                // vertical position
		int sy = source[2];                                       // horizontal position
		int flipx = attr & 0x20;                                  // flip x
		int flipy = attr & 0x40;                                  // flip y
		int const color = (source[1] & 0xf0) >> 4;                // color
		int width, height;

		if (flipscreen)
		{
			flipx = !flipx;
			flipy = !flipy;
			sx = 240 - sx;
			sy = 240 - sy;

			if ((attr & 0x1c) == 0x10)
			{   // ???. needed for some sprites in flipped mode
				sx -= 0x10;
				sy -= 0x10;
			}
		}

		switch (attr & 0x1c)
		{
			case 0x10:  // 32x32
				width = height = 2; number &= (~3); break;
			case 0x08:  // 16x32
				width = 1; height = 2; number &= (~2); break;
			case 0x04:  // 32x16
				width = 2; height = 1; number &= (~1); break;
			// the hardware allows more sprite sizes, but ddribble doesn't use them
			default:    // 16x16
				width = height = 1; break;
		}

		{
			static const int x_offset[2] = { 0x00, 0x01 };
			static const int y_offset[2] = { 0x00, 0x02 };

			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					int const ex = flipx ? (width - 1 - x) : x;
					int const ey = flipy ? (height - 1 - y) : y;


						gfx->transpen(bitmap, cliprect,
						(number) + x_offset[ex] + y_offset[ey],
						color,
						flipx, flipy,
						sx + x * 16, sy + y * 16, 0);
				}
			}
		}
		source += 5;
	}
}

/***************************************************************************

    Display Refresh

***************************************************************************/

uint32_t ddribble_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_fg_tilemap->set_flip((m_vregs[0][4] & 0x08) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	m_bg_tilemap->set_flip((m_vregs[1][4] & 0x08) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	// set scroll registers
	m_fg_tilemap->set_scrollx(0, m_vregs[0][1] | ((m_vregs[0][2] & 0x01) << 8));
	m_bg_tilemap->set_scrollx(0, m_vregs[1][1] | ((m_vregs[1][2] & 0x01) << 8));
	m_fg_tilemap->set_scrolly(0, m_vregs[0][0]);
	m_bg_tilemap->set_scrolly(0, m_vregs[1][0]);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect, m_spriteram[0], 0x07d, 2, m_vregs[0][4] & 0x08);
	draw_sprites(bitmap, cliprect, m_spriteram[1], 0x140, 3, m_vregs[1][4] & 0x08);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


void ddribble_state::vblank_irq(int state)
{
	if (state && m_int_enable[0])
		m_maincpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);

	if (state && m_int_enable[1])
		m_subcpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
}


void ddribble_state::bankswitch_w(uint8_t data)
{
	m_mainbank->set_entry(data & 0x07);
}


void ddribble_state::coin_counter_w(uint8_t data)
{
	// b4-b7: unused
	// b2-b3: unknown
	// b1: coin counter 2
	// b0: coin counter 1
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));
}

uint8_t ddribble_state::vlm5030_busy_r()
{
	return machine().rand(); // patch
	// FIXME: remove ?
#if 0
	if (m_vlm->bsy()) return 1;
	else return 0;
#endif
}

void ddribble_state::vlm5030_ctrl_w(uint8_t data)
{
	// b7 : vlm data bus OE

	// b6 : VLM5030-RST
	m_vlm->rst(BIT(data, 6));

	// b5 : VLM5030-ST
	m_vlm->st(BIT(data, 5));

	// b4 : VLM5300-VCU
	m_vlm->vcu(BIT(data, 4));

	// b3 : ROM bank select
	m_vlmbank->set_entry(BIT(data, 3));

	// b2 : SSG-C rc filter enable
	m_filter[2]->filter_rc_set_RC(filter_rc_device::LOWPASS_3R, 1000, 2200, 1000, data & 0x04 ? CAP_N(150) : 0); // YM2203-SSG-C

	// b1 : SSG-B rc filter enable
	m_filter[1]->filter_rc_set_RC(filter_rc_device::LOWPASS_3R, 1000, 2200, 1000, data & 0x02 ? CAP_N(150) : 0); // YM2203-SSG-B

	// b0 : SSG-A rc filter enable
	m_filter[0]->filter_rc_set_RC(filter_rc_device::LOWPASS_3R, 1000, 2200, 1000, data & 0x01 ? CAP_N(150) : 0); // YM2203-SSG-A
}


void ddribble_state::maincpu_map(address_map &map)
{
	map(0x0000, 0x0004).w(FUNC(ddribble_state::k005885_w<0>));                                              // video registers (005885 #1)
	map(0x0800, 0x0804).w(FUNC(ddribble_state::k005885_w<1>));                                              // video registers (005885 #2)
	map(0x1800, 0x187f).ram().w("palette", FUNC(palette_device::write_indirect)).share("palette");
	map(0x2000, 0x2fff).ram().w(FUNC(ddribble_state::fg_videoram_w)).share(m_fg_videoram);   // Video RAM 1
	map(0x3000, 0x3fff).ram().share(m_spriteram[0]);                             // Object RAM 1
	map(0x4000, 0x5fff).ram().share("sharedram");                                   // shared RAM with CPU #1
	map(0x6000, 0x6fff).ram().w(FUNC(ddribble_state::bg_videoram_w)).share(m_bg_videoram);   // Video RAM 2
	map(0x7000, 0x7fff).ram().share(m_spriteram[1]);                             // Object RAM 2
	map(0x8000, 0x8000).w(FUNC(ddribble_state::bankswitch_w));
	map(0x8000, 0x9fff).bankr(m_mainbank);
	map(0xa000, 0xffff).rom().region("maincpu", 0xa000);
}

void ddribble_state::subcpu_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("sharedram");       // shared RAM with CPU #0
	map(0x2000, 0x27ff).ram().share("snd_sharedram");   // shared RAM with CPU #2
	map(0x2800, 0x2800).portr("DSW1");
	map(0x2801, 0x2801).portr("P1");
	map(0x2802, 0x2802).portr("P2");
	map(0x2803, 0x2803).portr("SYSTEM");                                         // coinsw & start
	map(0x2c00, 0x2c00).portr("DSW2");
	map(0x3000, 0x3000).portr("DSW3");
	map(0x3400, 0x3400).w(FUNC(ddribble_state::coin_counter_w));
	map(0x3c00, 0x3c00).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x8000, 0xffff).rom().region("subcpu", 0);
}

void ddribble_state::audiocpu_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("snd_sharedram");       // shared RAM with CPU #1
	map(0x1000, 0x1001).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x3000, 0x3000).w(m_vlm, FUNC(vlm5030_device::data_w));
	map(0x8000, 0xffff).rom().region("audiocpu", 0);
}

void ddribble_state::vlm_map(address_map &map)
{
	map(0x0000, 0xffff).bankr(m_vlmbank);
}

static INPUT_PORTS_START( ddribble )
	PORT_START("P1")
	KONAMI8_B132(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	KONAMI8_B132(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	KONAMI8_SYSTEM_UNK

	PORT_START("DSW1")
	KONAMI_COINAGE_ALT_LOC(SW1)

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "SW2:1" )   // Manual says it's Unused
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "SW2:2" )   // Manual says it's Unused
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "SW2:4" )   // Manual says it's Unused
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW2:5" )   // Manual says it's Unused
	PORT_DIPNAME( 0x60, 0x20, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "SW3:2" )   // Manual says it's Unused
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPNAME( 0x08, 0x08, "Allow vs match with 1 Credit" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4,
			32*8+0*4, 32*8+1*4, 32*8+2*4, 32*8+3*4, 32*8+4*4, 32*8+5*4, 32*8+6*4, 32*8+7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32 },
	32*32
};

static GFXDECODE_START( gfx_ddribble )
	GFXDECODE_ENTRY( "gfx1", 0x00000, charlayout,    48,  1 )   // colors 48-63
	GFXDECODE_ENTRY( "gfx2", 0x00000, charlayout,    16,  1 )   // colors 16-31
	GFXDECODE_ENTRY( "gfx1", 0x20000, spritelayout,  32,  1 )   // colors 32-47
	GFXDECODE_ENTRY( "gfx2", 0x40000, spritelayout,  64, 16 )   // colors  0-15 but using lookup table
GFXDECODE_END


void ddribble_state::machine_start()
{
	m_mainbank->configure_entries(0, 8, memregion("maincpu")->base(), 0x2000);
	m_vlmbank->configure_entries(0, 2, memregion("vlm")->base(), 0x10000);

	save_item(NAME(m_int_enable));
	save_item(NAME(m_vregs));
	save_item(NAME(m_charbank));
}

void ddribble_state::machine_reset()
{
	for (int i = 0; i < 5; i++)
	{
		m_vregs[0][i] = 0;
		m_vregs[1][i] = 0;
	}

	m_int_enable[0] = 0;
	m_int_enable[1] = 0;
	m_charbank[0] = 0;
	m_charbank[1] = 0;
}

void ddribble_state::ddribble(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, XTAL(18'432'000) / 12);  // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &ddribble_state::maincpu_map);

	MC6809E(config, m_subcpu, XTAL(18'432'000) / 12);  // verified on PCB
	m_subcpu->set_addrmap(AS_PROGRAM, &ddribble_state::subcpu_map);

	mc6809e_device &audiocpu(MC6809E(config, "audiocpu", XTAL(18'432'000) / 12));  // verified on PCB
	audiocpu.set_addrmap(AS_PROGRAM, &ddribble_state::audiocpu_map);

	config.set_maximum_quantum(attotime::from_hz(6000));  // we need heavy synch

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
/*  screen.set_size(64*8, 32*8);
    screen.set_visarea(0*8, 64*8-1, 2*8, 30*8-1); */
	screen.set_screen_update(FUNC(ddribble_state::screen_update));
	screen.set_palette("palette");
	screen.screen_vblank().set(FUNC(ddribble_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, "palette", gfx_ddribble);
	PALETTE(config, "palette", FUNC(ddribble_state::palette)).set_format(palette_device::xBGR_555, 64 + 256, 64);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2203_device &ymsnd(YM2203(config, "ymsnd", XTAL(3'579'545))); // verified on PCB
	ymsnd.port_b_read_callback().set(FUNC(ddribble_state::vlm5030_busy_r));
	ymsnd.port_a_write_callback().set(FUNC(ddribble_state::vlm5030_ctrl_w));
	ymsnd.add_route(0, "filter1", 0.25);
	ymsnd.add_route(1, "filter2", 0.25);
	ymsnd.add_route(2, "filter3", 0.25);
	ymsnd.add_route(3, "mono", 0.25);

	VLM5030(config, m_vlm, XTAL(3'579'545)); // verified on PCB
	m_vlm->add_route(ALL_OUTPUTS, "mono", 1.0);
	m_vlm->set_addrmap(0, &ddribble_state::vlm_map);

	FILTER_RC(config, m_filter[0]).add_route(ALL_OUTPUTS, "mono", 1.0);

	FILTER_RC(config, m_filter[1]).add_route(ALL_OUTPUTS, "mono", 1.0);

	FILTER_RC(config, m_filter[2]).add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START( ddribble )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "690c03.bin", 0x00000, 0x10000, CRC(07975a58) SHA1(96fd1b2348bbdf560067d8ee3cd4c0514e263d7a) )

	ROM_REGION( 0x8000, "subcpu", 0 )
	ROM_LOAD( "690c02.bin", 0x0000, 0x8000, CRC(f07c030a) SHA1(db96a10f8bb657bf285266db9e775fa6af82f38c) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "690b01.bin", 0x0000, 0x8000, CRC(806b8453) SHA1(3184772c5e5181438a17ac72129070bf164b2965) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "690a05.bin",  0x00000, 0x20000, CRC(6a816d0d) SHA1(73f2527d5f2b9d51b784be36e07e0d0c566a28d9) )    // characters & objects
	ROM_LOAD16_BYTE( "690a06.bin",  0x00001, 0x20000, CRC(46300cd0) SHA1(07197a546fff452a41575fcd481da64ac6bf601e) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "690a10.bin", 0x00000, 0x20000, CRC(61efa222) SHA1(bd7b993ad1c06d8f6ac29fbc07c4a987abe1ab42) ) // characters
	ROM_LOAD16_BYTE( "690a09.bin", 0x00001, 0x20000, CRC(ab682186) SHA1(a28982835042a07354557e1539b097cdf93fc466) )
	ROM_LOAD16_BYTE( "690a08.bin", 0x40000, 0x20000, CRC(9a889944) SHA1(ca96815aefb1e336bd2288841b00a5c21cacf90f) ) // objects
	ROM_LOAD16_BYTE( "690a07.bin", 0x40001, 0x20000, CRC(faf81b3f) SHA1(0bd647b4cdd3f2209472e303fd22eedd5533d1b1) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "690a11.i15", 0x0000, 0x0100, CRC(f34617ad) SHA1(79ceba6fe204472a5a659641ac4f14bb1f0ee3f6) )  // sprite lookup table

	ROM_REGION( 0x20000, "vlm", 0 )
	ROM_LOAD( "690a04.bin", 0x00000, 0x20000, CRC(1bfeb763) SHA1(f3e9acb2a7a9b4c8dee6838c1344a7a65c27ff77) )

	ROM_REGION( 0x0100, "plds", 0 )
	ROM_LOAD( "pal10l8-007553.bin", 0x0000, 0x002c, CRC(0ae5a161) SHA1(87571addf434b332019ea0e22372eb24b4fd0197) )
ROM_END

ROM_START( ddribblep )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ebs_11-19.c19",  0x00000, 0x10000, CRC(0a81c926) SHA1(1ecd30f0d352cf6c96d246bb443b5a6738624b9b) )

	ROM_REGION( 0x8000, "subcpu", 0 )
	ROM_LOAD( "eb_11-19.c12", 0x0000, 0x8000, CRC(22130292) SHA1(a5f9bf3f63ff85d171f096867433513419458b0e) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "master_sound.a6", 0x0000, 0x8000, CRC(090e3a31) SHA1(4c645b55d52abb859354ea2ea401e4ab99f5d493) )

	ROM_REGION( 0x40000, "gfx1", 0 ) // same content as parent
	ROM_LOAD16_BYTE( "v1a.e12", 0x00000, 0x10000, CRC(53724765) SHA1(55a45ab71f7bf55ed805d4dc2345cadc4171f323) )    // characters & objects
	ROM_LOAD16_BYTE( "01a.e11", 0x20000, 0x10000, CRC(1ae5d725) SHA1(d8dd41cc1872c6d218cc425d1cd03f8d8eefe3e3) )    // characters & objects
	ROM_LOAD16_BYTE( "v1b.e13", 0x00001, 0x10000, CRC(d9dc6f1a) SHA1(f50169525c5109ba65acdccbb01dddb92926462a) )
	ROM_LOAD16_BYTE( "01b.d14", 0x20001, 0x10000, CRC(054c5242) SHA1(411389e36d33fd27e13ffc6a7d4b295a42f08869) )

	ROM_REGION( 0x80000, "gfx2", 0 ) // same content as parent
	ROM_LOAD16_BYTE( "v2a00.i13",         0x00000, 0x10000, CRC(a33f7d6d) SHA1(c2b9a9a66e4712785250cad69a5e43338af60a82) )  // characters
	ROM_LOAD16_BYTE( "v2a10.h13",         0x20000, 0x10000, CRC(8fbc7454) SHA1(93782d148afe64b14fa46deb4d227ef167030c94) )  // characters
	ROM_LOAD16_BYTE( "v2b00.i12",         0x00001, 0x10000, CRC(e63759bb) SHA1(df7e94f40266aa8995509346cdfdce08a885de16) )
	ROM_LOAD16_BYTE( "v2b10.h12",         0x20001, 0x10000, CRC(8a7d4062) SHA1(5b5eb4edc765f0e13e22f9de62ddae7380ba3790) )
	ROM_LOAD16_BYTE( "02a00.i11",         0x40000, 0x10000, CRC(6751a942) SHA1(a71c9cbbf1fba92664144d571d49cf2c15f45408) )  // objects
	ROM_LOAD16_BYTE( "02a10.h11",         0x60000, 0x10000, CRC(bc5ff11c) SHA1(b02296982298e1a659ce05606b291eda9a605cc8) )  // objects
	ROM_LOAD16_BYTE( "02b00_11-4.i8.bin", 0x40001, 0x10000, CRC(460aa7b4) SHA1(9e928d6150e7a91d411c0510198e80d523a88272) )
	ROM_LOAD16_BYTE( "02b10.h8",          0x60001, 0x10000, CRC(2cc7ee28) SHA1(c96890383dbef755953f851a43449cf563e2e1a5) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "6301-1.i15", 0x0000, 0x0100, CRC(f34617ad) SHA1(79ceba6fe204472a5a659641ac4f14bb1f0ee3f6) )  // sprite lookup table

	ROM_REGION( 0x20000, "vlm", 0 )  // same content as parent
	ROM_LOAD( "voice_00.e7", 0x00000, 0x10000, CRC(8bd0fcf7) SHA1(d55644f8b33eff6f960725f00ba842e0253e3b36) )
	ROM_LOAD( "voice_10.d7", 0x10000, 0x10000, CRC(b4c97494) SHA1(93f7c3c93f6f790c3f480e183da0105b5ac3593b) )
ROM_END

} // anonymous namespace


GAME( 1986, ddribble,  0,        ddribble, ddribble, ddribble_state, empty_init, ROT0, "Konami", "Double Dribble",              MACHINE_SUPPORTS_SAVE )
GAME( 1986, ddribblep, ddribble, ddribble, ddribble, ddribble_state, empty_init, ROT0, "Konami", "Double Dribble (prototype?)", MACHINE_SUPPORTS_SAVE )

// license:BSD-3-Clause
// copyright-holders: Pierpaolo Prazzoli

/***************************************************************************

Fighting Roller (c) 1983 Kaneko

Issues:
-sound effects missing

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_D900     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_D900)

#include "logmacro.h"

#define LOGD900(...)     LOGMASKED(LOG_D900,     __VA_ARGS__)


namespace {

class rollrace_state : public driver_device
{
public:
	rollrace_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_roadlay_rom(*this, "road_layout")
	{ }

	void rollace2(machine_config &config);
	void rollace(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_region_ptr<uint8_t> m_roadlay_rom;

	tilemap_t *m_fg_tilemap = nullptr;
	uint8_t m_bkgpage = 0;
	uint8_t m_bkgflip = 0;
	uint8_t m_chrbank = 0;
	uint8_t m_bkgpen = 0;
	uint8_t m_bkgcol = 0;
	uint8_t m_flipy = 0;
	uint8_t m_flipx = 0;
	uint8_t m_spritebank = 0;

	enum
	{
		RA_FGCHAR_BASE = 0,
		RA_BGCHAR_BASE = 4,
		RA_SP_BASE = 5
	};

	uint8_t m_nmi_mask = 0;
	uint8_t m_sound_nmi_mask = 0;

	uint8_t fake_d900_r();
	void fake_d900_w(uint8_t data);
	void nmi_mask_w(int state);
	void sound_nmi_mask_w(uint8_t data);
	template <uint8_t Which> void coin_counter_w(int state);
	void charbank_0_w(int state);
	void charbank_1_w(int state);
	void bkgpen_w(uint8_t data);
	void spritebank_w(int state);
	void backgroundpage_w(uint8_t data);
	void backgroundcolor_w(uint8_t data);
	void flipy_w(uint8_t data);
	void flipx_w(int state);
	void vram_w(offs_t offset, uint8_t data);
	void cram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	void tilemap_refresh_flip();

	void palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void vblank_irq(int state);
	INTERRUPT_GEN_MEMBER(sound_timer_irq);
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


TILE_GET_INFO_MEMBER(rollrace_state::get_fg_tile_info)
{
	int const code = m_videoram[tile_index];
	int const color = m_colorram[(tile_index & 0x1f) * 2 + 1] & 0x1f;

	tileinfo.set(RA_FGCHAR_BASE + m_chrbank,
		code,
		color,
		TILE_FLIPY);
}

void rollrace_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(rollrace_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_scroll_cols(32);
}

void rollrace_state::vram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void rollrace_state::cram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	if (offset & 1)
	{
		// TODO: optimize
		m_fg_tilemap->mark_all_dirty();
		//for(int x = 0; x < 32; x++)
		//  m_fg_tilemap->mark_tile_dirty(x + ((offset >> 1) * 32));
	}
	else
		m_fg_tilemap->set_scrolly(offset >> 1,data);
}

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Stinger has three 256x4 palette PROMs (one per gun).
  The palette PROMs are connected to the RGB output this way:

  bit 3 -- 100 ohm resistor  -- RED/GREEN/BLUE
        -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 1  kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/
void rollrace_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2, bit3;

		bit0 = BIT(color_prom[0], 0);
		bit1 = BIT(color_prom[0], 1);
		bit2 = BIT(color_prom[0], 2);
		bit3 = BIT(color_prom[0], 3);
		int const r = 0x0e * bit0 + 0x1f * bit1 + 0x42 * bit2 + 0x90 * bit3;

		bit0 = BIT(color_prom[palette.entries()], 0);
		bit1 = BIT(color_prom[palette.entries()], 1);
		bit2 = BIT(color_prom[palette.entries()], 2);
		bit3 = BIT(color_prom[palette.entries()], 3);
		int const g = 0x0e * bit0 + 0x1f * bit1 + 0x42 * bit2 + 0x90 * bit3;

		bit0 = BIT(color_prom[2 * palette.entries()], 0);
		bit1 = BIT(color_prom[2 * palette.entries()], 1);
		bit2 = BIT(color_prom[2 * palette.entries()], 2);
		bit3 = BIT(color_prom[2 * palette.entries()], 3);
		int const b = 0x0e * bit0 + 0x1f * bit1 + 0x42 * bit2 + 0x90 * bit3;

		palette.set_pen_color(i, rgb_t(r, g, b));

		color_prom++;
	}
}

void rollrace_state::charbank_0_w(int state)
{
	m_chrbank = state | (m_chrbank & 2);
	m_fg_tilemap->mark_all_dirty();
}

void rollrace_state::charbank_1_w(int state)
{
	m_chrbank = (m_chrbank & 1) | (state << 1);
	m_fg_tilemap->mark_all_dirty();
}

void rollrace_state::bkgpen_w(uint8_t data)
{
	m_bkgpen = data;
}

void rollrace_state::spritebank_w(int state)
{
	m_spritebank = state;
}

void rollrace_state::backgroundpage_w(uint8_t data)
{
	m_bkgpage = data & 0x1f;
	m_bkgflip = (data & 0x80) >> 7;

	// 0x80 flip vertical
}

void rollrace_state::backgroundcolor_w(uint8_t data)
{
	m_bkgcol = data;
}

void rollrace_state::flipy_w(uint8_t data)
{
	m_flipy = data & 0x01;
	// bit 2: cleared at night stage in attract, unknown purpose
}

void rollrace_state::flipx_w(int state)
{
	m_flipx = state;
	m_fg_tilemap->set_flip(m_flipx ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
}

uint32_t rollrace_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// fill in background colour
	bitmap.fill(m_bkgpen, cliprect);

	// draw road
	for (int offs = 0x3ff; offs >= 0; offs--)
	{
		int sy;

		if (!(m_bkgflip))
			sy = (31 - offs / 32);
		else
			sy = (offs / 32);

		int sx = (offs % 32);

		if (m_flipx)
			sx = 31 - sx;

		if (m_flipy)
			sy = 31 - sy;

		m_gfxdecode->gfx(RA_BGCHAR_BASE)->transpen(bitmap,
			cliprect,
			m_roadlay_rom[offs + (m_bkgpage * 1024)]
			+ (((m_roadlay_rom[offs + 0x4000 + (m_bkgpage * 1024)] & 0xc0) >> 6) * 256),
			m_bkgcol,
			m_flipx, (m_bkgflip ^ m_flipy),
			sx * 8, sy * 8, 0);
	}

	// sprites
	for (int offs = 0x80-4; offs >= 0x0; offs -= 4)
	{
		int s_flipy = 0;

		int sy = m_spriteram[offs] - 16;
		int sx = m_spriteram[offs + 3] - 16;

		if (sx && sy)
		{
			if (m_flipx)
				sx = 224 - sx;
			if (m_flipy)
				sy = 224 - sy;

			if (m_spriteram[offs + 1] & 0x80)
				s_flipy = 1;

			int bank = ((m_spriteram[offs + 1] & 0x40 ) >> 6);

			if (bank)
				bank += m_spritebank;

			m_gfxdecode->gfx(RA_SP_BASE + bank)->transpen(bitmap, cliprect,
				m_spriteram[offs + 1] & 0x3f,
				m_spriteram[offs + 2] & 0x1f,
				m_flipx, !(s_flipy ^ m_flipy),
				sx, sy, 0);
		}
	}

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


void rollrace_state::machine_start()
{
	save_item(NAME(m_bkgpage));
	save_item(NAME(m_bkgflip));
	save_item(NAME(m_chrbank));
	save_item(NAME(m_bkgpen));
	save_item(NAME(m_bkgcol));
	save_item(NAME(m_flipy));
	save_item(NAME(m_flipx));
	save_item(NAME(m_spritebank));
	save_item(NAME(m_nmi_mask));
	save_item(NAME(m_sound_nmi_mask));
}

uint8_t rollrace_state::fake_d900_r()
{
	return 0x51;
}

void rollrace_state::fake_d900_w(uint8_t data)
{
	LOGD900("d900: %02X\n", data);
}

void rollrace_state::nmi_mask_w(int state)
{
	m_nmi_mask = state;
	if (!m_nmi_mask)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void rollrace_state::sound_nmi_mask_w(uint8_t data)
{
	m_sound_nmi_mask = data & 1;
}

template <uint8_t Which>
void rollrace_state::coin_counter_w(int state)
{
	machine().bookkeeping().coin_counter_w(Which, state);
}

void rollrace_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).rom(); // only rollace2
	map(0xc000, 0xcfff).ram();
	map(0xd806, 0xd806).nopr(); // looks like a watchdog, bit 4 checked
	map(0xd900, 0xd900).rw(FUNC(rollrace_state::fake_d900_r), FUNC(rollrace_state::fake_d900_w)); // protection ??
	map(0xe000, 0xe3ff).ram().w(FUNC(rollrace_state::vram_w)).share(m_videoram);
	map(0xe400, 0xe47f).ram().w(FUNC(rollrace_state::cram_w)).share(m_colorram);
	map(0xe800, 0xe800).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xec00, 0xec0f).noprw(); // Analog sound effects ?? ec00 sound enable ?
	map(0xf000, 0xf0ff).ram().share(m_spriteram);
	map(0xf400, 0xf400).w(FUNC(rollrace_state::backgroundcolor_w));
	map(0xf800, 0xf800).portr("P1");
	map(0xf801, 0xf801).portr("P2").w(FUNC(rollrace_state::bkgpen_w));
	map(0xf802, 0xf802).portr("SYSTEM").w(FUNC(rollrace_state::backgroundpage_w));
	map(0xf803, 0xf803).w(FUNC(rollrace_state::flipy_w));
	map(0xf804, 0xf804).portr("DSW1");
	map(0xf805, 0xf805).portr("DSW2");
	map(0xfc00, 0xfc07).w("mainlatch", FUNC(ls259_device::write_d0));
}


void rollrace_state::sound_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x2000, 0x2fff).ram();
	map(0x3000, 0x3000).r("soundlatch", FUNC(generic_latch_8_device::read)).w(FUNC(rollrace_state::sound_nmi_mask_w));
	map(0x4000, 0x4001).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x5000, 0x5001).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0x6000, 0x6001).w("ay3", FUNC(ay8910_device::address_data_w));
}


static INPUT_PORTS_START( rollace )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) // Jump
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) // Punch
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL // Jump
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL // Punch
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH,IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH,IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH,IPT_SERVICE1 )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_SERVICE( 0x40, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )

	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )

//  PORT_BIT( 0x40, IP_ACTIVE_HIGH , IPT_CUSTOM ) PORT_VBLANK("screen")  freezes frame, could be vblank ?
	PORT_DIPNAME( 0x40, 0x00, "Freeze" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
//  PORT_DIPNAME( 0x80, 0x00, "Free Run" )
	PORT_DIPNAME( 0x80, 0x00, "Invulnerability (Cheat)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )  // test mode, you are invulnerable
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )   // to 'static' objects

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "7" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x04, "20000" )
	PORT_DIPSETTING(    0x08, "50000" )
	PORT_DIPSETTING(    0x0c, "100000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, "A" )
	PORT_DIPSETTING(    0x10, "B" )
	PORT_DIPSETTING(    0x20, "C" )
	PORT_DIPSETTING(    0x30, "D" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR(Cabinet) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x80, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,    // 8*8 characters
	256,    // 256 characters
	3,    // 3 bits per pixel
	{ 0,1024*8*8, 2*1024*8*8 }, // the two bitplanes are separated
	{ 0,1,2,3,4,5,6,7 },
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	8*8 // every char takes 8 consecutive bytes
};
static const gfx_layout charlayout2 =
{
	8,8,    // 8*8 characters
	1024,   // 1024 characters
	3,    // 3 bits per pixel
	{ 0,1024*8*8, 2*1024*8*8 }, // the two bitplanes are separated
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
//  { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 // every char takes 8 consecutive bytes
};

static const gfx_layout spritelayout =
{
	32,32,  // 32*32 sprites
	64, // 64 sprites
	3,    // 3 bits per pixel
	{ 0x4000*8, 0x2000*8, 0 }, // the three bitplanes are separated
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7,16*8,16*8+1,16*8+2,16*8+3,16*8+4,16*8+5,16*8+6,16*8+7,24*8,24*8+1,24*8+2,24*8+3,24*8+4,24*8+5,24*8+6,24*8+7},

	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8,
			64*8,65*8,66*8,67*8,68*8,69*8,70*8,71*8, 96*8,97*8,98*8,99*8,100*8,101*8,102*8,103*8 },
	32*32    // every sprite takes 128 consecutive bytes
};

static GFXDECODE_START( gfx_rollace )
	GFXDECODE_ENTRY( "chars",     0x0000, charlayout,    0,  32 )
	GFXDECODE_ENTRY( "chars",     0x0800, charlayout,    0,  32 )
	GFXDECODE_ENTRY( "chars",     0x1000, charlayout,    0,  32 )
	GFXDECODE_ENTRY( "chars",     0x1800, charlayout,    0,  32 )
	GFXDECODE_ENTRY( "road",      0x0000, charlayout2,   0,  32 )
	GFXDECODE_ENTRY( "sprites_0", 0x0000, spritelayout,  0,  32 )
	GFXDECODE_ENTRY( "sprites_1", 0x0000, spritelayout,  0,  32 )
	GFXDECODE_ENTRY( "sprites_2", 0x0000, spritelayout,  0,  32 )
GFXDECODE_END

void rollrace_state::vblank_irq(int state)
{
	if (state && m_nmi_mask)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

INTERRUPT_GEN_MEMBER(rollrace_state::sound_timer_irq)
{
	if (m_sound_nmi_mask)
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void rollrace_state::rollace(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(24'000'000) / 8); // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &rollrace_state::main_map);

	Z80(config, m_audiocpu, XTAL(24'000'000) / 16); // verified on PCB
	m_audiocpu->set_addrmap(AS_PROGRAM, &rollrace_state::sound_map);
	m_audiocpu->set_periodic_int(FUNC(rollrace_state::sound_timer_irq), attotime::from_hz(4 * 60));

	ls259_device &mainlatch(LS259(config, "mainlatch"));
	mainlatch.q_out_cb<0>().set(FUNC(rollrace_state::flipx_w));
	mainlatch.q_out_cb<1>().set(FUNC(rollrace_state::nmi_mask_w));
	mainlatch.q_out_cb<2>().set(FUNC(rollrace_state::coin_counter_w<0>));
	mainlatch.q_out_cb<3>().set(FUNC(rollrace_state::coin_counter_w<1>));
	mainlatch.q_out_cb<4>().set(FUNC(rollrace_state::charbank_0_w));
	mainlatch.q_out_cb<5>().set(FUNC(rollrace_state::charbank_1_w));
	mainlatch.q_out_cb<6>().set(FUNC(rollrace_state::spritebank_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 16, 255-16);
	screen.set_screen_update(FUNC(rollrace_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(rollrace_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_rollace);
	PALETTE(config, m_palette, FUNC(rollrace_state::palette), 256);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, "soundlatch");

	AY8910(config, "ay1", XTAL(24'000'000) / 16).add_route(ALL_OUTPUTS, "rspeaker", 0.10); // verified on PCB
	AY8910(config, "ay2", XTAL(24'000'000) / 16).add_route(ALL_OUTPUTS, "rspeaker", 0.10); // verified on PCB
	AY8910(config, "ay3", XTAL(24'000'000) / 16).add_route(ALL_OUTPUTS, "lspeaker", 0.10); // verified on PCB
}

void rollrace_state::rollace2(machine_config &config)
{
	rollace(config);

	// basic machine hardware

//  subdevice<screen_device>("screen")->set_visarea(0, 256-1, 16, 255-16);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( fightrol )
	ROM_REGION( 0xa000, "maincpu", 0 )
	ROM_LOAD( "4.8k", 0x0000, 0x2000, CRC(efa2f430) SHA1(6aeb2a41e4fba97a0ac1b24fe5437e25b6c6b6c5) )
	ROM_LOAD( "5.8h", 0x2000, 0x2000, CRC(2497d9f6) SHA1(4f4cfed47efc603bf057dd24b761beecf5b929f4) )
	ROM_LOAD( "6.8f", 0x4000, 0x2000, CRC(f39727b9) SHA1(08a1300172b4100cb80c9a5d8942408255d8e330) )
	ROM_LOAD( "7.8d", 0x6000, 0x2000, CRC(ee65b728) SHA1(871918d505ad8bab60c55bbb95fe37556a204dc9) )

	ROM_REGION( 0x6000, "chars", 0 )
	ROM_LOAD( "3.7m", 0x0000, 0x2000, CRC(ca4f353c) SHA1(754838c6ad6886052a018966d55f40a7ed4b684d) )
	ROM_LOAD( "2.8m", 0x2000, 0x2000, CRC(93786171) SHA1(3928aad8bc43adeaad5e53c1d4e9df64f1d23704) )
	ROM_LOAD( "1.9m", 0x4000, 0x2000, CRC(dc072be1) SHA1(94d379a4c5a53050a18cd572cc82edb337182f3b) )

	ROM_REGION( 0x6000, "road", 0 )
	ROM_LOAD ( "6.20k", 0x0000, 0x2000, CRC(003d7515) SHA1(d8d84d690478cad16101f2ef9a1ae1ae74d01c88) )
	ROM_LOAD ( "7.18k", 0x2000, 0x2000, CRC(27843afa) SHA1(81d3031a2c06086461110696a0ee11d32992ecac) )
	ROM_LOAD ( "5.20f", 0x4000, 0x2000, CRC(51dd0108) SHA1(138c0aba6c952204e794216193def17b390c4ba2) )

	ROM_REGION( 0x6000, "sprites_0", 0 )
	ROM_LOAD ( "8.17m",  0x0000, 0x2000, CRC(08ad783e) SHA1(fea91e41916cfc7b29c5f9a578e2c82a54f66829) )
	ROM_LOAD ( "9.17r",  0x2000, 0x2000, CRC(69b23461) SHA1(73eca5e721425f37df311454bd5b4e632b096eba) )
	ROM_LOAD ( "10.17t", 0x4000, 0x2000, CRC(ba6ccd8c) SHA1(29a13e3161aba4db080434685869f8b79ad7997c) )

	ROM_REGION( 0x6000, "sprites_1", 0 )
	ROM_LOAD ( "11.18m", 0x0000, 0x2000, CRC(06a5d849) SHA1(b9f604edf4fdc053b738041493aef91dd730fe6b) )
	ROM_LOAD ( "12.18r", 0x2000, 0x2000, CRC(569815ef) SHA1(db261799892f60b2274b73fb25cde58219bb44db) )
	ROM_LOAD ( "13.18t", 0x4000, 0x2000, CRC(4f8af872) SHA1(6c07ff0733b8d8440309c9ae0db0876587b740a6) )

	ROM_REGION( 0x6000, "sprites_2", 0 )
	ROM_LOAD ( "14.19m", 0x0000, 0x2000, CRC(93f3c649) SHA1(38d6bb4b6108a67b135ae1a145532f4a0c2568b8) )
	ROM_LOAD ( "15.19r", 0x2000, 0x2000, CRC(5b3d87e4) SHA1(e47f7b62bf7101afba8d5e181f4bd8f8eb6eeb08) )
	ROM_LOAD ( "16.19u", 0x4000, 0x2000, CRC(a2c24b64) SHA1(e76558785ea337ab902fb6f94dc1a4bdfcd6335e) )

	ROM_REGION( 0x8000, "road_layout", 0 )
	ROM_LOAD ( "1.17a", 0x0000, 0x2000, CRC(f0fa72fc) SHA1(b73e794df635630f29a79adfe2951dc8f1d17e20) )
	ROM_LOAD ( "3.18b", 0x2000, 0x2000, CRC(954268f7) SHA1(07057296e0281f90b18dfe4223aad18bff7cfa6e) )
	ROM_LOAD ( "2.17d", 0x4000, 0x2000, CRC(2e38bb0e) SHA1(684f14a06ff957e40780be21c0ad5f10088a55ed) )
	ROM_LOAD ( "4.18d", 0x6000, 0x2000, CRC(3d9e16ab) SHA1(e99628ffc54e3ff4818313a287ca111617120910) )

	ROM_REGION( 0x300, "proms", 0 )  // colour
	ROM_LOAD( "tbp24s10.7u", 0x0000, 0x0100, CRC(9d199d33) SHA1(b8982f7da2b85f10d117177e4e73cbb486931cf5) )
	ROM_LOAD( "tbp24s10.7t", 0x0100, 0x0100, CRC(c0426582) SHA1(8e3e4d1e76243cce272aa099d2d6ad4fa6c99f7c) )
	ROM_LOAD( "tbp24s10.6t", 0x0200, 0x0100, CRC(c096e05c) SHA1(cb5b509e6124453f381a683ba446f8f4493d4610) )

	ROM_REGION( 0x1000, "audiocpu", 0 )
	ROM_LOAD( "8.6f", 0x0000, 0x1000, CRC(6ec3c545) SHA1(1a2477b9e1563734195b0743f5dbbb005e06022e) )
ROM_END

ROM_START( rollace )
	ROM_REGION( 0xa000, "maincpu", 0 )
	ROM_LOAD( "w1.8k", 0x0000, 0x2000, CRC(c0bd3cf3) SHA1(a44d69b8c3249b5093261a32d0e0404992fa7f7a) )
	ROM_LOAD( "w2.8h", 0x2000, 0x2000, CRC(c1900a75) SHA1(f7ec968b6bcb6ee6db98628cdf566ae0a501edba) )
	ROM_LOAD( "w3.8f", 0x4000, 0x2000, CRC(16ceced6) SHA1(241119959ffdf26780258bcc5651eca0c6a6128f) )
	ROM_LOAD( "w4.8d", 0x6000, 0x2000, CRC(ae826a96) SHA1(47979343c9fa7629ba6d62630c7c3fdfa2c8c28a) )

	ROM_REGION( 0x6000, "chars", 0 )
	ROM_LOAD( "w3.7m", 0x0000, 0x2000, CRC(f9970aae) SHA1(ccb806cab3d3817c779e048f995d1f6fbe163679) )
	ROM_LOAD( "w2.8m", 0x2000, 0x2000, CRC(80573091) SHA1(ea352abebc428db9e89eda5f369a3b1086aa8970) )
	ROM_LOAD( "w1.9m", 0x4000, 0x2000, CRC(b37effd8) SHA1(d77d56d734834812b8d9b3c156577dbbcb2deac8) )

	ROM_REGION( 0x6000, "road", 0 )
	ROM_LOAD ( "6.20k", 0x0000, 0x2000, CRC(003d7515) SHA1(d8d84d690478cad16101f2ef9a1ae1ae74d01c88) )
	ROM_LOAD ( "7.18k", 0x2000, 0x2000, CRC(27843afa) SHA1(81d3031a2c06086461110696a0ee11d32992ecac) )
	ROM_LOAD ( "5.20f", 0x4000, 0x2000, CRC(51dd0108) SHA1(138c0aba6c952204e794216193def17b390c4ba2) )

	ROM_REGION( 0x6000, "sprites_0", 0 )
	ROM_LOAD ( "w8.17m",  0x0000, 0x2000, CRC(e2afe3a3) SHA1(a83a12c0c6c62e45add916a6993f0ad06840c4d9) )
	ROM_LOAD ( "w9.17p",  0x2000, 0x2000, CRC(8a8e6b62) SHA1(6e7d4a84b7c78e009bce0641e357f74c8ac9e5ac) )
	ROM_LOAD ( "w10.17t", 0x4000, 0x2000, CRC(70bf7b23) SHA1(6774eceb0bfea66156ecd837f9d0adbdf8dec8ee) )

	ROM_REGION( 0x6000, "sprites_1", 0 )
	ROM_LOAD ( "11.18m", 0x0000, 0x2000, CRC(06a5d849) SHA1(b9f604edf4fdc053b738041493aef91dd730fe6b) )
	ROM_LOAD ( "12.18r", 0x2000, 0x2000, CRC(569815ef) SHA1(db261799892f60b2274b73fb25cde58219bb44db) )
	ROM_LOAD ( "13.18t", 0x4000, 0x2000, CRC(4f8af872) SHA1(6c07ff0733b8d8440309c9ae0db0876587b740a6) )

	ROM_REGION( 0x6000, "sprites_2", 0 )
	ROM_LOAD ( "14.19m", 0x0000, 0x2000, CRC(93f3c649) SHA1(38d6bb4b6108a67b135ae1a145532f4a0c2568b8) )
	ROM_LOAD ( "15.19r", 0x2000, 0x2000, CRC(5b3d87e4) SHA1(e47f7b62bf7101afba8d5e181f4bd8f8eb6eeb08) )
	ROM_LOAD ( "16.19u", 0x4000, 0x2000, CRC(a2c24b64) SHA1(e76558785ea337ab902fb6f94dc1a4bdfcd6335e) )

	ROM_REGION( 0x8000, "road_layout", 0 )
	ROM_LOAD ( "1.17a", 0x0000, 0x2000, CRC(f0fa72fc) SHA1(b73e794df635630f29a79adfe2951dc8f1d17e20) )
	ROM_LOAD ( "3.18b", 0x2000, 0x2000, CRC(954268f7) SHA1(07057296e0281f90b18dfe4223aad18bff7cfa6e) )
	ROM_LOAD ( "2.17d", 0x4000, 0x2000, CRC(2e38bb0e) SHA1(684f14a06ff957e40780be21c0ad5f10088a55ed) )
	ROM_LOAD ( "4.18d", 0x6000, 0x2000, CRC(3d9e16ab) SHA1(e99628ffc54e3ff4818313a287ca111617120910) )

	ROM_REGION( 0x300, "proms", 0 )  // colour
	ROM_LOAD( "tbp24s10.7u", 0x0000, 0x0100, CRC(9d199d33) SHA1(b8982f7da2b85f10d117177e4e73cbb486931cf5) )
	ROM_LOAD( "tbp24s10.7t", 0x0100, 0x0100, CRC(c0426582) SHA1(8e3e4d1e76243cce272aa099d2d6ad4fa6c99f7c) )
	ROM_LOAD( "tbp24s10.6t", 0x0200, 0x0100, CRC(c096e05c) SHA1(cb5b509e6124453f381a683ba446f8f4493d4610) )

	ROM_REGION( 0x1000, "audiocpu", 0 )
	ROM_LOAD( "8.6f", 0x0000, 0x1000, CRC(6ec3c545) SHA1(1a2477b9e1563734195b0743f5dbbb005e06022e) )
ROM_END

ROM_START( rollace2 )
	ROM_REGION( 0xa000, "maincpu", 0 )
	ROM_LOAD( "8k.764", 0x0000, 0x2000, CRC(a7abff82) SHA1(d49635f98b28b2b5e2833d25b0961addac2c3e6f) )
	ROM_LOAD( "8h.764", 0x2000, 0x2000, CRC(9716ba03) SHA1(8a7bfc1dce3b1b0c634690e0637e0a30776c0334) )
	ROM_LOAD( "8f.764", 0x4000, 0x2000, CRC(3eadb0e8) SHA1(6ff5b76360597f3a6a9718e505295c8557e569ae) )
	ROM_LOAD( "8d.764", 0x6000, 0x2000, CRC(baac14db) SHA1(9707b59a6506eb11c0a6b88364a784469ccdbb96) )
	ROM_LOAD( "8c.764", 0x8000, 0x2000, CRC(b418ce84) SHA1(876be297a671328138a9238d42871f22bb568cda) )

	ROM_REGION( 0x6000, "chars", 0 )
	ROM_LOAD( "7m.764", 0x0000, 0x2000, CRC(8b9b27af) SHA1(a52894adb739f14a5949b6d15dd7b03ce5716d9a) )
	ROM_LOAD( "8m.764", 0x2000, 0x2000, CRC(2dfc38f2) SHA1(c0ad3a7d1f5249c159c355d709cc3039fbb7a3b2) )
	ROM_LOAD( "9m.764", 0x4000, 0x2000, CRC(2e3a825b) SHA1(d0d25d9a0fe31d46cb6cc999da3d9fc14f23251f) )

	ROM_REGION( 0x6000, "road", 0 )
	ROM_LOAD ( "6.20k", 0x0000, 0x2000, CRC(003d7515) SHA1(d8d84d690478cad16101f2ef9a1ae1ae74d01c88) )
	ROM_LOAD ( "7.18k", 0x2000, 0x2000, CRC(27843afa) SHA1(81d3031a2c06086461110696a0ee11d32992ecac) )
	ROM_LOAD ( "5.20f", 0x4000, 0x2000, CRC(51dd0108) SHA1(138c0aba6c952204e794216193def17b390c4ba2) )

	ROM_REGION( 0x6000, "sprites_0", 0 )
	ROM_LOAD ( "17n.764", 0x0000, 0x2000, CRC(3365703c) SHA1(7cf374ba25f4fd163a66c0aea74ddfd3003c7992) )
	ROM_LOAD ( "9.17r",   0x2000, 0x2000, CRC(69b23461) SHA1(73eca5e721425f37df311454bd5b4e632b096eba) )
	ROM_LOAD ( "17t.764", 0x4000, 0x2000, CRC(5e84cc9b) SHA1(33cdf7b756ade8c0dd1dcdad583af4de02cd51eb) )

	ROM_REGION( 0x6000, "sprites_1", 0 )
	ROM_LOAD ( "11.18m", 0x0000, 0x2000, CRC(06a5d849) SHA1(b9f604edf4fdc053b738041493aef91dd730fe6b) )
	ROM_LOAD ( "12.18r", 0x2000, 0x2000, CRC(569815ef) SHA1(db261799892f60b2274b73fb25cde58219bb44db) )
	ROM_LOAD ( "13.18t", 0x4000, 0x2000, CRC(4f8af872) SHA1(6c07ff0733b8d8440309c9ae0db0876587b740a6) )

	ROM_REGION( 0x6000, "sprites_2", 0 )
	ROM_LOAD ( "14.19m", 0x0000, 0x2000, CRC(93f3c649) SHA1(38d6bb4b6108a67b135ae1a145532f4a0c2568b8) )
	ROM_LOAD ( "15.19r", 0x2000, 0x2000, CRC(5b3d87e4) SHA1(e47f7b62bf7101afba8d5e181f4bd8f8eb6eeb08) )
	ROM_LOAD ( "16.19u", 0x4000, 0x2000, CRC(a2c24b64) SHA1(e76558785ea337ab902fb6f94dc1a4bdfcd6335e) )

	ROM_REGION( 0x8000, "road_layout", 0 )
	ROM_LOAD ( "1.17a",   0x0000, 0x2000, CRC(f0fa72fc) SHA1(b73e794df635630f29a79adfe2951dc8f1d17e20) )
	ROM_LOAD ( "3.18b",   0x2000, 0x2000, CRC(954268f7) SHA1(07057296e0281f90b18dfe4223aad18bff7cfa6e) )
	ROM_LOAD ( "17d.764", 0x4000, 0x2000, CRC(32e69320) SHA1(d399a8c3b0319178d75f68f1a9b65b3efd91e00a) )
	ROM_LOAD ( "4.18d",   0x6000, 0x2000, CRC(3d9e16ab) SHA1(e99628ffc54e3ff4818313a287ca111617120910) )

	ROM_REGION( 0x300, "proms", 0 )  // colour
	ROM_LOAD( "tbp24s10.7u", 0x0000, 0x0100, CRC(9d199d33) SHA1(b8982f7da2b85f10d117177e4e73cbb486931cf5) )
	ROM_LOAD( "tbp24s10.7t", 0x0100, 0x0100, CRC(c0426582) SHA1(8e3e4d1e76243cce272aa099d2d6ad4fa6c99f7c) )
	ROM_LOAD( "tbp24s10.6t", 0x0200, 0x0100, CRC(c096e05c) SHA1(cb5b509e6124453f381a683ba446f8f4493d4610) )

	ROM_REGION( 0x1000, "audiocpu", 0 )
	ROM_LOAD( "8.6f", 0x0000, 0x1000, CRC(6ec3c545) SHA1(1a2477b9e1563734195b0743f5dbbb005e06022e) )
ROM_END

} // anonymous namespace

//    YEAR  NAME      PARENT    MACHINE   INPUT    CLASS          INIT        SCREEN  COMPANY, FULLNAME, FLAGS
GAME( 1983, fightrol, 0,        rollace,  rollace, rollrace_state, empty_init, ROT270, "Kaneko (Taito license)", "Fighting Roller", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1983, rollace,  fightrol, rollace,  rollace, rollrace_state, empty_init, ROT270, "Kaneko (Williams license)", "Roller Aces (set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1983, rollace2, fightrol, rollace2, rollace, rollrace_state, empty_init, ROT90,  "Kaneko (Williams license)", "Roller Aces (set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

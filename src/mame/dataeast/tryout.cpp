// license:BSD-3-Clause
// copyright-holders: Pierpaolo Prazzoli, Bryan McPhail

/***************************************************************

 Pro Baseball Skill Tryout (JPN Ver.)
 (c) 1985 Data East

 Driver by Pierpaolo Prazzoli and Bryan McPhail

=================================================================
Debug cheats:

*tryout
Pitching stage (3)
$201 remaining ball counter
$208 strikes count
(note: put a wpset 20x,1,w and modify the value AFTER that the
       game modifies it)

****************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "machine/gen_latch.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_GFXCTRL (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_GFXCTRL)

#include "logmacro.h"

#define LOGGFXCTRL(...) LOGMASKED(LOG_GFXCTRL, __VA_ARGS__)


namespace {

class tryout_state : public driver_device
{
public:
	tryout_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram%u", 1U),
		m_gfx_control(*this, "gfx_control"),
		m_vram(*this, "vram", 8 * 0x800, ENDIANNESS_LITTLE),
		m_vram_gfx(*this, "vram_gfx", 0x6000, ENDIANNESS_LITTLE),
		m_rombank(*this, "rombank")
	{ }

	void tryout(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr_array<uint8_t, 2> m_spriteram;
	required_shared_ptr<uint8_t> m_gfx_control;
	memory_share_creator<uint8_t> m_vram;
	memory_share_creator<uint8_t> m_vram_gfx;

	required_memory_bank m_rombank;

	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_vram_bank = 0;

	void nmi_ack_w(uint8_t data);
	void sound_irq_ack_w(uint8_t data);
	void bankswitch_w(uint8_t data);
	uint8_t vram_r(offs_t offset);
	void videoram_w(offs_t offset, uint8_t data);
	void vram_w(offs_t offset, uint8_t data);
	void vram_bankswitch_w(uint8_t data);
	void flipscreen_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILEMAP_MAPPER_MEMBER(get_fg_memory_offset);
	TILEMAP_MAPPER_MEMBER(get_bg_memory_offset);

	void palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);

	void main_cpu(address_map &map) ATTR_COLD;
	void sound_cpu(address_map &map) ATTR_COLD;
};


void tryout_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
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
		bit0 = 0;
		bit1 = BIT(color_prom[i], 6);
		bit2 = BIT(color_prom[i], 7);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

TILE_GET_INFO_MEMBER(tryout_state::get_fg_tile_info)
{
	int code = m_videoram[tile_index];
	int const attr = m_videoram[tile_index + 0x400];
	code |= ((attr & 0x03) << 8);
	int const color = ((attr & 0x4) >> 2) + 6;

	tileinfo.set(0, code, color, 0);
}

TILE_GET_INFO_MEMBER(tryout_state::get_bg_tile_info)
{
	tileinfo.set(2, m_vram[tile_index] & 0x7f, 2, 0);
}

uint8_t tryout_state::vram_r(offs_t offset)
{
	return m_vram[offset]; // debug only
}

void tryout_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

void tryout_state::vram_w(offs_t offset, uint8_t data)
{
	/*  There are eight banks of vram - in bank 0 the first 0x400 bytes
	are reserved for the tilemap.  In banks 2, 4 and 6 the game never
	writes to the first 0x400 bytes - I suspect it's either
	unused, or it actually mirrors the tilemap ram from the first bank.

	The rest of the vram is tile data which has the bitplanes arranged
	in a very strange format.  For MAME's sake we reformat this on
	the fly for easier gfx decode.

	Bit 0 of the bank register seems special - it's kept low when uploading
	gfx data and then set high from that point onwards.

	*/
	const uint8_t bank = (m_vram_bank >> 1) & 0x7;


	if ((bank == 0 || bank == 2 || bank == 4 || bank == 6) && (offset & 0x7ff) < 0x400)
	{
		int const newoff = offset & 0x3ff;

		m_vram[newoff] = data;
		m_bg_tilemap->mark_tile_dirty(newoff);
		return;
	}

	/*
	    Bit planes for tiles are arranged as follows within vram (split into high/low nibbles):
	        0x0400 (0) + 0x0400 (4) + 0x0800(0) - tiles 0x00 to 0x0f
	        0x0800 (4) + 0x0c00 (0) + 0x0c00(4) - tiles 0x10 to 0x1f
	        0x1400 (0) + 0x1400 (4) + 0x1800(0) - tiles 0x20 to 0x2f
	        0x1800 (4) + 0x1c00 (0) + 0x1c00(4) - tiles 0x30 to 0x3f
	        etc.
	*/

	offset = (offset & 0x7ff) | (bank << 11);
	m_vram[offset] = data;

	switch (offset & 0x1c00)
	{
	case 0x0400:
		m_vram_gfx[(offset & 0x3ff) + 0x0000 + ((offset & 0x2000) >> 1)] = (~data & 0xf);
		m_vram_gfx[(offset & 0x3ff) + 0x2000 + ((offset & 0x2000) >> 1)] = (~data & 0xf0) >> 4;
		break;
	case 0x0800:
		m_vram_gfx[(offset & 0x3ff) + 0x4000 + ((offset & 0x2000) >> 1)] = (~data & 0xf);
		m_vram_gfx[(offset & 0x3ff) + 0x4400 + ((offset & 0x2000) >> 1)] = (~data & 0xf0) >> 4;
		break;
	case 0x0c00:
		m_vram_gfx[(offset & 0x3ff) + 0x0400 + ((offset & 0x2000) >> 1)] = (~data & 0xf);
		m_vram_gfx[(offset & 0x3ff) + 0x2400 + ((offset & 0x2000) >> 1)] = (~data & 0xf0) >> 4;
		break;
	case 0x1400:
		m_vram_gfx[(offset & 0x3ff) + 0x0800 + ((offset & 0x2000) >> 1)] = (~data & 0xf);
		m_vram_gfx[(offset & 0x3ff) + 0x2800 + ((offset & 0x2000) >> 1)] = (~data & 0xf0) >> 4;
		break;
	case 0x1800:
		m_vram_gfx[(offset & 0x3ff) + 0x4800 + ((offset & 0x2000) >> 1)] = (~data & 0xf);
		m_vram_gfx[(offset & 0x3ff) + 0x4c00 + ((offset & 0x2000) >> 1)] = (~data & 0xf0) >> 4;
		break;
	case 0x1c00:
		m_vram_gfx[(offset & 0x3ff) + 0x0c00 + ((offset & 0x2000) >> 1)] = (~data & 0xf);
		m_vram_gfx[(offset & 0x3ff) + 0x2c00 + ((offset & 0x2000) >> 1)] = (~data & 0xf0) >> 4;
		break;
	}

	m_gfxdecode->gfx(2)->mark_dirty((offset - 0x400 / 64) & 0x7f);
}

void tryout_state::vram_bankswitch_w(uint8_t data)
{
	m_vram_bank = data;
}

void tryout_state::flipscreen_w(uint8_t data)
{
	flip_screen_set(data & 1);
}

TILEMAP_MAPPER_MEMBER(tryout_state::get_fg_memory_offset)
{
	return (row ^ 0x1f) + (col << 5);
}

TILEMAP_MAPPER_MEMBER(tryout_state::get_bg_memory_offset)
{
	int a;
//  if (col&0x20)
//      a= (7 - (row & 7)) + ((0x8 - (row & 0x8)) << 4) + ((col & 0xf) << 3) + (( (  0x10 - (col & 0x10) ) ) << 4) + ((( (col & 0x20))) << 4);
//  else
		a= (7 - (row & 7)) + ((0x8 - (row & 0x8)) << 4) + ((col & 0xf) << 3) + ((col & 0x10) << 4) + ((col & 0x20) << 4);

//  osd_printf_debug("%d %d -> %d\n", col, row, a);
	return a;
}

void tryout_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tryout_state::get_fg_tile_info)), tilemap_mapper_delegate(*this, FUNC(tryout_state::get_fg_memory_offset)),  8, 8, 32,32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tryout_state::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(tryout_state::get_bg_memory_offset)), 16,16, 64,16);

	m_fg_tilemap->set_transparent_pen(0);

	save_item(NAME(m_vram_bank));
}

void tryout_state::draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	for (int offs = 0; offs < 0x7f; offs += 4)
	{
		if (!(m_spriteram[0][offs] & 1))
			continue;

		int const sprite = m_spriteram[0][offs + 1] + ((m_spriteram[1][offs] & 7) << 8);
		int x = m_spriteram[0][offs + 3] - 3;
		int y = m_spriteram[0][offs + 2];
		int const color = 0;//(m_spriteram[0][offs] & 8) >> 3;
		int fx = (m_spriteram[0][offs] & 8) >> 3;
		int fy = 0;
		int inc = 16;

		if (flip_screen())
		{
			x = 240 - x;
			fx = !fx;

			y = 240 - y;
			fy = !fy;

			inc = -inc;
		}

		// Double Height
		if (m_spriteram[0][offs] & 0x10)
		{
			m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
				sprite,
				color, fx, fy, x, y + inc, 0);

			m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
				sprite + 1,
				color, fx, fy, x, y, 0);
		}
		else
		{
			m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
				sprite,
				color, fx, fy, x, y, 0);
		}
	}
}

uint32_t tryout_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!flip_screen())
		m_fg_tilemap->set_scrollx(0, 16); // Assumed hard-wired
	else
		m_fg_tilemap->set_scrollx(0, -8); // Assumed hard-wired

	int scrollx = m_gfx_control[1] + ((m_gfx_control[0] & 1) << 8) + ((m_gfx_control[0] & 4) << 7) - ((m_gfx_control[0] & 2) ? 0 : 0x100);

	// wrap-around
	if (m_gfx_control[1] == 0) { scrollx += 0x100; }

	m_bg_tilemap->set_scrollx(0, scrollx + 2); // why +2? hard-wired?
	m_bg_tilemap->set_scrolly(0, -m_gfx_control[2]);

	if (!(m_gfx_control[0] & 0x8)) // screen disable
	{
		// TODO: Color might be different, needs a video from an original PCB.
		bitmap.fill(m_palette->pen(0x10), cliprect);
	}
	else
	{
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		draw_sprites(bitmap, cliprect);
	}

	LOGGFXCTRL("%02x %02x %02x %02x", m_gfx_control[0], m_gfx_control[1], m_gfx_control[2], scrollx);
	return 0;
}


void tryout_state::nmi_ack_w(uint8_t data)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

// this is actually irq/nmi mask, polls only four values at start up (81->01->81->01) and then stays on this state.
void tryout_state::sound_irq_ack_w(uint8_t data)
{
//  m_audiocpu->set_input_line(0, CLEAR_LINE);
}

void tryout_state::machine_start()
{
	m_rombank->configure_entries(0, 2, memregion("maincpu")->base() + 0x10000, 0x2000);
}

void tryout_state::bankswitch_w(uint8_t data)
{
	m_rombank->set_entry(data & 0x01);
}

void tryout_state::main_cpu(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x1000, 0x17ff).ram().w(FUNC(tryout_state::videoram_w)).share(m_videoram);
	map(0x2000, 0x3fff).bankr(m_rombank);
	map(0x4000, 0xbfff).rom();
	map(0xc800, 0xc87f).ram().share(m_spriteram[0]);
	map(0xcc00, 0xcc7f).ram().share(m_spriteram[1]);
	map(0xd000, 0xd7ff).rw(FUNC(tryout_state::vram_r), FUNC(tryout_state::vram_w));
	map(0xe000, 0xe000).portr("DSW");
	map(0xe001, 0xe001).portr("P1");
	map(0xe002, 0xe002).portr("P2");
	map(0xe003, 0xe003).portr("SYSTEM");
	map(0xe301, 0xe301).w(FUNC(tryout_state::flipscreen_w));
	map(0xe302, 0xe302).w(FUNC(tryout_state::bankswitch_w));
	map(0xe401, 0xe401).w(FUNC(tryout_state::vram_bankswitch_w));
	map(0xe402, 0xe404).writeonly().share(m_gfx_control);
	map(0xe414, 0xe414).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xe417, 0xe417).w(FUNC(tryout_state::nmi_ack_w));
	map(0xfff0, 0xffff).rom().region("maincpu", 0xbff0); // reset vectors
}

void tryout_state::sound_cpu(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x4000, 0x4001).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xa000, 0xa000).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0xd000, 0xd000).w(FUNC(tryout_state::sound_irq_ack_w));
	map(0xc000, 0xffff).rom();
}

INPUT_CHANGED_MEMBER(tryout_state::coin_inserted)
{
	if (oldval)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

static INPUT_PORTS_START( tryout )
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, tryout_state, coin_inserted, 0)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, tryout_state, coin_inserted, 0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,    // 8*8 characters
	RGN_FRAC(1,2),
	2,  // 2 bits per pixel
	{ 0, 4 },   // the two bitplanes for 4 pixels are packed into one byte
	{ 3, 2, 1, 0, RGN_FRAC(1,2)+3, RGN_FRAC(1,2)+2, RGN_FRAC(1,2)+1, RGN_FRAC(1,2)+0 },
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	8*8 // every char takes 8 consecutive bytes
};

static const gfx_layout vramlayout =
{
	16, 16,
	128,
	3,
	{ 0x0000 * 8, 0x2000 * 8, 0x4000 * 8 },
	{ 7, 6, 5, 4, 128+7, 128+6, 128+5, 128+4, 256+7, 256+6, 256+5, 256+4, 384+7, 384+6, 384+5, 384+4  },
	{ 15*8, 14*8, 13*8, 12*8, 11*8, 10*8, 9*8, 8*8,
		7*8, 6*8,5*8,4*8,3*8,2*8,1*8,0*8 },
	64*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 7, 6, 5, 4, 3, 2, 1, 0,
			16*8+7, 16*8+6, 16*8+5, 16*8+4, 16*8+3, 16*8+2, 16*8+1, 16*8+0 },
	{ 15*8, 14*8, 13*8, 12*8, 11*8, 10*8, 9*8, 8*8,
			7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	32*8
};

static GFXDECODE_START( gfx_tryout )
	GFXDECODE_ENTRY( "chars",    0, charlayout,   0, 8 )
	GFXDECODE_ENTRY( "sprites",  0, spritelayout, 0, 4 )
	GFXDECODE_RAM(   "vram_gfx", 0, vramlayout,   0, 4 )
GFXDECODE_END

void tryout_state::tryout(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, 2'000'000);     // ?
	m_maincpu->set_addrmap(AS_PROGRAM, &tryout_state::main_cpu);

	M6502(config, m_audiocpu, 1'500'000);    // ?
	m_audiocpu->set_addrmap(AS_PROGRAM, &tryout_state::sound_cpu);
	m_audiocpu->set_periodic_int(FUNC(tryout_state::nmi_line_pulse), attotime::from_hz(1000)); // controls BGM tempo, 1000 is an hand-tuned value to match a side-by-side video

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(256, 256);
	screen.set_visarea(0*8, 32*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(tryout_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tryout);
	PALETTE(config, m_palette, FUNC(tryout_state::palette), 0x20);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	generic_latch_8_device &soundlatch(GENERIC_LATCH_8(config, "soundlatch"));
	soundlatch.data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_IRQ0);

	YM2203(config, "ymsnd", 1'500'000).add_route(ALL_OUTPUTS, "mono", 0.50);
}

ROM_START( tryout )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "ch10-1.bin",   0x04000, 0x4000, CRC(d046231b) SHA1(145f9e9b0707824f7ae6d1587754b28c17907807) )
	ROM_LOAD( "ch11.bin",     0x08000, 0x4000, CRC(4d00b6f0) SHA1(cc1e700b8547672d7dd1d262c6181a5c321fbf72) )
	ROM_LOAD( "ch12.bin",     0x10000, 0x4000, CRC(bcd221be) SHA1(69869de8b5d56a97e2cd15fa275527aa767f1e44) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ch00-1.bin",   0x0c000, 0x4000, CRC(8b33d968) SHA1(cf44529e5577d09978b87dc2bbe1415babbf36a0) )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "ch13.bin",     0x00000, 0x4000, CRC(a9619c58) SHA1(92528b1c4afc95394ac8cad5b37f23da0c6a5310) )

	ROM_REGION( 0x24000, "sprites", 0 )
	ROM_LOAD( "ch09.bin",     0x00000, 0x4000, CRC(9c5e275b) SHA1(83b29996573d85c73bb4b63086c7a624fad19bde) )
	ROM_LOAD( "ch08.bin",     0x04000, 0x4000, CRC(88396abb) SHA1(2865a265ddfb91c2ad2770da5e0d84a544f3c419) )
	ROM_LOAD( "ch07.bin",     0x08000, 0x4000, CRC(901b5f5e) SHA1(f749b5ec0c51c66655798e8a37c887870370991e) )
	ROM_LOAD( "ch06.bin",     0x0c000, 0x4000, CRC(d937e326) SHA1(5870a82b02438f2fdae089f6d1b8e9ce13d213a6) )
	ROM_LOAD( "ch05.bin",     0x10000, 0x4000, CRC(27f0e7be) SHA1(5fa2bd666d012addfb836d009f962f89e4a00b2d) )
	ROM_LOAD( "ch04.bin",     0x14000, 0x4000, CRC(019e0b75) SHA1(4bfd7cd6c28ec6dfaf8e9bf009716e92759f06c2) )
	ROM_LOAD( "ch03.bin",     0x18000, 0x4000, CRC(b87e2464) SHA1(0089c0ff421929345a1d21951789a6374e0019ff) )
	ROM_LOAD( "ch02.bin",     0x1c000, 0x4000, CRC(62369772) SHA1(89f360003e916bee76d74b7e046bf08349726fda) )
	ROM_LOAD( "ch01.bin",     0x20000, 0x4000, CRC(ee6d57b5) SHA1(7dd2f3b962f088fcbc40fcb74c0a56783857fb7b) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "ch14.bpr",     0x00000, 0x0020, CRC(8ce19925) SHA1(12f8f6022f1148b6ba1d019a34247452637063a7) )
ROM_END

} // anonymous namespace


GAME( 1985, tryout, 0, tryout, tryout, tryout_state, empty_init, ROT90, "Data East Corporation", "Pro Baseball Skill Tryout (Japan)", MACHINE_SUPPORTS_SAVE )

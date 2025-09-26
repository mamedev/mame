// license:BSD-3-Clause
// copyright-holders: Curt Coder
// thanks-to: Kenneth Lin (original driver author)

/***************************************************************************

  jackal.cpp
  Konami GX631 PCB

Notes:
- This game uses two 005885 gfx chip in parallel. The unique thing about it is
  that the two 4bpp tilemaps from the two chips are merged to form a single
  8bpp tilemap.
- topgunbl is derived from a completely different version, which supports gun
  turret rotation. The copyright year is also different, but this doesn't
  necessarily mean anything.

TODO:
- create a K005885 device shared with other drivers.


Memory Map
----------

MAIN CPU:

Address range 00xxxxxxxxxx---- is handled by the 007343 custom so layout is
inferred by program behaviour. Note that address lines A8 and A9 are ORed
together and go to the single A8.9 input of the 007343.

Address          Dir Data     Description
---------------- --- -------- -----------------------
00000000000000xx R/W xxxxxxxx 005885 registers
0000000000000100 R/W xxxxxxxx 005885 registers
0000000000010000 R   xxxxxxxx DIPSW1
0000000000010001 R   xxxxxxxx P1 inputs + DIPSW3.4
0000000000010010 R   xxxxxxxx P2 inputs
0000000000010011 R   xxxxxxxx Coin inputs + DIPSW3.1-3
00000000000101-0 R   xxxxxxxx P1 extra inputs (only used by the bootleg for the rotary control)
00000000000101-1 R   xxxxxxxx P2 extra inputs (only used by the bootleg for the rotary control)
00000000000110-0 R   xxxxxxxx DIPSW2
00000000000111-0   W -------x Coin Counter 1 (to 005924 OUT1 input)
                   W ------x- Coin Counter 2 (to 005924 OUT2 input)
                   W -----x-- unknown ("END", to connector SVCN4P pin 4)
                   W ----x--- sprite RAM bank (to 007343 OBJB input)
                   W ---x---- 005885 select (to 007343 GATEB input)
                   W --x----- ROM bank
00000000000111-1 R/W -------- Watchdog reset (to 005924 AFR input)
00000000001xxxxx R/W xxxxxxxx scroll RAM (005885)
00000000010xxxxx R/W xxxxxxxx Z RAM (005885)
000xxxxxxxxxxxxx R/W xxxxxxxx RAM (shared with sound CPU--note that addresses 0000-005F are handled above so they are excluded)
0010xxxxxxxxxxxx R/W xxxxxxxx video RAM (005885)
0011xxxxxxxxxxxx R/W xxxxxxxx sprite RAM (005885)
01xxxxxxxxxxxxxx R   xxxxxxxx ROM (banked)
10xxxxxxxxxxxxxx R   xxxxxxxx ROM (banked)
11xxxxxxxxxxxxxx R   xxxxxxxx ROM


SOUND CPU:

Address          Dir Data     Description
---------------- --- -------- -----------------------
000-------------              n.c.
001------------x R/W xxxxxxxx YM2151
010-xxxxxxxxxxxx R/W xxxxxxxx 007327 (palette)
011xxxxxxxxxxxxx R/W xxxxxxxx RAM (shared with main CPU)
1xxxxxxxxxxxxxxx R   xxxxxxxx ROM

***************************************************************************/

#include "emu.h"

#include "konamipt.h"

#include "cpu/m6809/m6809.h"
#include "machine/watchdog.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_RAMBANK     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_RAMBANK)

#include "logmacro.h"

#define LOGRAMBANK(...)     LOGMASKED(LOG_RAMBANK,     __VA_ARGS__)


namespace {

class jackal_state : public driver_device
{
public:
	jackal_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoctrl(*this, "videoctrl"),
		m_videoram(*this, "videoram%u", 0U),
		m_scrollram(*this, "scrollram%u", 0U, 0x40U, ENDIANNESS_BIG),
		m_mainbank(*this, "mainbank"),
		m_videoview(*this, "videoview"),
		m_spritebank(*this, "spritebank"),
		m_spriteram(*this, "spriteram%u", 0U, 0x1000U, ENDIANNESS_BIG),
		m_scrollbank(*this, "scrollbank"),
		m_dials(*this, "DIAL%u", 0U),
		m_maincpu(*this, "maincpu"),
		m_slavecpu(*this, "slave"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void jackal(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_videoctrl;
	required_shared_ptr_array<uint8_t, 2> m_videoram;
	memory_share_array_creator<uint8_t, 2> m_scrollram;
	required_memory_bank m_mainbank;
	memory_view m_videoview;
	required_memory_bank m_spritebank;
	memory_share_array_creator<uint8_t, 2> m_spriteram;
	required_memory_bank m_scrollbank;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;

	// misc
	uint8_t m_irq_enable = 0;
	optional_ioport_array<2> m_dials;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_slavecpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	uint8_t rotary_r(offs_t offset);
	void flipscreen_w(uint8_t data);
	void rambank_w(uint8_t data);
	template <uint8_t Which> void voram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(int state);
	void draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites_region(bitmap_ind16 &bitmap, const rectangle &cliprect, const uint8_t *sram, int length, int bank);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map) ATTR_COLD;
	void slave_map(address_map &map) ATTR_COLD;
};


void jackal_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < 0x100; i++)
	{
		uint16_t const ctabentry = i | 0x100;
		palette.set_pen_indirect(i, ctabentry);
	}

	for (int i = 0x100; i < 0x200; i++)
	{
		uint16_t const ctabentry = color_prom[i - 0x100] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}

	for (int i = 0x200; i < 0x300; i++)
	{
		uint16_t const ctabentry = (color_prom[i - 0x100] & 0x0f) | 0x10;
		palette.set_pen_indirect(i, ctabentry);
	}
}

TILE_GET_INFO_MEMBER(jackal_state::get_bg_tile_info)
{
	int const attr = m_videoram[0][tile_index];
	int const code = m_videoram[0][0x400 + tile_index] + ((attr & 0xc0) << 2) + ((attr & 0x30) << 6);
	int const color = 0; //attr & 0x0f;
	int const flags = ((attr & 0x10) ? TILE_FLIPX : 0) | ((attr & 0x20) ? TILE_FLIPY : 0);

	tileinfo.set(0, code, color, flags);
}

void jackal_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(jackal_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_spritebank->configure_entry(0, m_spriteram[0]);
	m_spritebank->configure_entry(1, m_spriteram[1]);

	m_scrollbank->configure_entry(0, m_scrollram[0]);
	m_scrollbank->configure_entry(1, m_scrollram[1]);
}

void jackal_state::draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scroll_rows(1);
	m_bg_tilemap->set_scroll_cols(1);

	m_bg_tilemap->set_scrolly(0, m_videoctrl[0]);
	m_bg_tilemap->set_scrollx(0, m_videoctrl[1]);

	if (m_videoctrl[2] & 0x02)
	{
		if (m_videoctrl[2] & 0x08)
		{
			m_bg_tilemap->set_scroll_rows(32);

			for (int i = 0; i < 32; i++)
				m_bg_tilemap->set_scrollx(i, m_scrollram[0][i]);
		}

		if (m_videoctrl[2] & 0x04)
		{
			m_bg_tilemap->set_scroll_cols(32);

			for (int i = 0; i < 32; i++)
				m_bg_tilemap->set_scrolly(i, m_scrollram[0][i]);
		}
	}

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
}

#define DRAW_SPRITE(bank, code, sx, sy)  m_gfxdecode->gfx(bank)->transpen(bitmap, cliprect, code, color, flipx, flipy, sx, sy, 0);

void jackal_state::draw_sprites_region(bitmap_ind16 &bitmap, const rectangle &cliprect, const uint8_t *sram, int length, int bank)
{
	for (int offs = 0; offs < length; offs += 5)
	{
		int const sn1 = sram[offs];
		int const sn2 = sram[offs + 1];
		int sy  = sram[offs + 2];
		int sx  = sram[offs + 3];
		int const attr = sram[offs + 4];
		int flipx = attr & 0x20;
		int flipy = attr & 0x40;
		int const color = ((sn2 & 0xf0) >> 4);

		if (attr & 0x01)
			sx = sx - 256;
		if (sy > 0xf0)
			sy = sy - 256;

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (attr & 0xc)    // half-size sprite
		{
			int const spritenum = sn1 * 4 + ((sn2 & (8 + 4)) >> 2) + ((sn2 & (2 + 1)) << 10);
			int mod = -8;

			if (flip_screen())
			{
				sx += 8;
				sy -= 8;
				mod = 8;
			}

			if ((attr & 0x0c) == 0x0c)
			{
				if (flip_screen()) sy += 16;
				DRAW_SPRITE(bank + 1, spritenum, sx, sy)
			}

			if ((attr & 0x0c) == 0x08)
			{
				sy += 8;
				DRAW_SPRITE(bank + 1, spritenum, sx, sy)
				DRAW_SPRITE(bank + 1, spritenum - 2, sx, sy + mod)
			}

			if ((attr & 0x0c) == 0x04)
			{
				DRAW_SPRITE(bank + 1, spritenum, sx, sy)
				DRAW_SPRITE(bank + 1, spritenum + 1, sx + mod, sy)
			}
		}
		else
		{
			int const spritenum = sn1 + ((sn2 & 0x03) << 8);

			if (attr & 0x10)
			{
				if (flip_screen())
				{
					sx -= 16;
					sy -= 16;
				}

				DRAW_SPRITE(bank, spritenum, flipx ? sx + 16 : sx, flipy ? sy + 16 : sy)
				DRAW_SPRITE(bank, spritenum + 1, flipx ? sx : sx + 16, flipy ? sy + 16 : sy)
				DRAW_SPRITE(bank, spritenum + 2, flipx ? sx + 16 : sx, flipy ? sy : sy + 16)
				DRAW_SPRITE(bank, spritenum + 3, flipx ? sx : sx + 16, flipy ? sy : sy + 16)
			}
			else
			{
				DRAW_SPRITE(bank, spritenum, sx, sy)
			}
		}
	}
}

void jackal_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_sprites_region(bitmap, cliprect, &m_spriteram[1][BIT(m_videoctrl[0x03], 3) * 0x800], 0x0f5, 3);
	draw_sprites_region(bitmap, cliprect, &m_spriteram[0][BIT(m_videoctrl[0x03], 3) * 0x800], 0x500, 1);
}

uint32_t jackal_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_background(screen, bitmap, cliprect);
	draw_sprites(bitmap, cliprect);
	return 0;
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

uint8_t jackal_state::rotary_r(offs_t offset)
{
	return (1 << m_dials[offset].read_safe(0x00)) ^ 0xff;
}

void jackal_state::flipscreen_w(uint8_t data)
{
	m_irq_enable = BIT(data, 1);
	flip_screen_set(BIT(data, 3));
}

void jackal_state::rambank_w(uint8_t data)
{
	if (data & 0x04)
		LOGRAMBANK("rambank_w %02x", data);

	// all revisions flips the coin counter bit between 1 -> 0 five times, causing the bookkeeping to report 5 coins inserted.
	// most likely solution in HW is a f/f that disables coin counters when any of the other bits are enabled.
	if ((data & 0xfc) == 0)
	{
		machine().bookkeeping().coin_counter_w(0, data & 0x01);
		machine().bookkeeping().coin_counter_w(1, data & 0x02);
	}

	m_spritebank->set_entry(BIT(data, 3));
	m_scrollbank->set_entry(BIT(data, 4));
	m_videoview.select(BIT(data, 4));
	m_mainbank->set_entry(BIT(data, 5));
}

template <uint8_t Which>
void jackal_state::voram_w(offs_t offset, uint8_t data)
{
	if ((offset & 0xf800) == 0)
		m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);

	m_videoram[Which][offset] = data;
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

void jackal_state::main_map(address_map &map)
{
	map(0x0000, 0x0003).ram().share(m_videoctrl);   // scroll + other things
	map(0x0004, 0x0004).w(FUNC(jackal_state::flipscreen_w));
	map(0x0010, 0x0010).portr("DSW1");
	map(0x0011, 0x0011).portr("IN1");
	map(0x0012, 0x0012).portr("IN2");
	map(0x0013, 0x0013).portr("IN0");
	map(0x0014, 0x0015).r(FUNC(jackal_state::rotary_r));
	map(0x0018, 0x0018).portr("DSW2");
	map(0x0019, 0x0019).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x001c, 0x001c).w(FUNC(jackal_state::rambank_w));
	map(0x0020, 0x005f).bankrw(m_scrollbank);      // MAIN   Z RAM,SUB    Z RAM
	map(0x0060, 0x1fff).ram().share("mainsub");    // M COMMON RAM,S COMMON RAM
	map(0x2000, 0x2fff).view(m_videoview);
	m_videoview[0](0x2000, 0x2fff).ram().share(m_videoram[0]).w(FUNC(jackal_state::voram_w<0>));   // MAIN V O RAM,SUB  V O RAM
	m_videoview[1](0x2000, 0x2fff).ram().share(m_videoram[1]).w(FUNC(jackal_state::voram_w<1>));   // MAIN V O RAM,SUB  V O RAM
	map(0x3000, 0x3fff).bankrw(m_spritebank);      // MAIN V O RAM,SUB  V O RAM
	map(0x4000, 0xbfff).bankr(m_mainbank);
	map(0xc000, 0xffff).rom().region("maincpu", 0);
}

void jackal_state::slave_map(address_map &map)
{
	map(0x2000, 0x2001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x4000, 0x43ff).ram().w(m_palette, FUNC(palette_device::write_indirect)).share("palette");  // self test only checks 0x4000-0x423f, 007327 should actually go up to 4fff
	map(0x6000, 0x605f).ram();                     // SOUND RAM (Self test check 0x6000-605f, 0x7c00-0x7fff)
	map(0x6060, 0x7fff).ram().share("mainsub");
	map(0x8000, 0xffff).rom().region("slave", 0);
}

/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( jackal )
	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	// "No Coin B" = coins produce sound, but no effect on coin counter

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) ) PORT_DIPLOCATION( "SW2:1,2" )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) ) PORT_DIPLOCATION( "SW2:3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION( "SW2:4,5" )
	PORT_DIPSETTING(    0x18, "30K 150K" )
	PORT_DIPSETTING(    0x10, "50K 200K" )
	PORT_DIPSETTING(    0x08, "30K" )
	PORT_DIPSETTING(    0x00, "50K" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) ) PORT_DIPLOCATION( "SW2:6,7" )
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION( "SW2:8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN0")
	KONAMI8_SYSTEM_10
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION( "SW3:1" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Sound Adjustment" ) PORT_DIPLOCATION( "SW3:2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, "Sound Mode" ) PORT_DIPLOCATION( "SW3:3" )
	PORT_DIPSETTING(    0x80, DEF_STR( Mono ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Stereo ) )

	PORT_START("IN1")
	KONAMI8_B12(1)
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) ) PORT_DIPLOCATION( "SW3:4" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	KONAMI8_B12_UNK(2)
INPUT_PORTS_END

static INPUT_PORTS_START( jackalr )
	PORT_INCLUDE(jackal)

	PORT_MODIFY("IN0")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DIAL0") // player 1 8-way rotary control - converted in rotary_r()
	PORT_BIT( 0xff, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(8) PORT_WRAPS PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X) PORT_FULL_TURN_COUNT(8)

	PORT_START("DIAL1") // player 2 8-way rotary control - converted in rotary_r()
	PORT_BIT( 0xff, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(8) PORT_WRAPS PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_M) PORT_PLAYER(2) PORT_FULL_TURN_COUNT(8)
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8, 8,
	RGN_FRAC(1,4),
	8,  // 8 bits per pixel (!)
	{ 0, 1, 2, 3, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+1, RGN_FRAC(1,2)+2, RGN_FRAC(1,2)+3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout spritelayout =
{
	16, 16,
	RGN_FRAC(1,4),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4,
			32*8+0*4, 32*8+1*4, 32*8+2*4, 32*8+3*4, 32*8+4*4, 32*8+5*4, 32*8+6*4, 32*8+7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32 },
	32*32
};

static const gfx_layout spritelayout8 =
{
	8, 8,
	RGN_FRAC(1,4),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static GFXDECODE_START( gfx_jackal )
	GFXDECODE_ENTRY( "gfx", 0x00000, charlayout,        0,  1 )    // colors 256-511 without lookup
	GFXDECODE_ENTRY( "gfx", 0x20000, spritelayout,  0x100, 16 )    // colors   0- 15 with lookup
	GFXDECODE_ENTRY( "gfx", 0x20000, spritelayout8, 0x100, 16 )    // to handle 8x8 sprites
	GFXDECODE_ENTRY( "gfx", 0x60000, spritelayout,  0x200, 16 )    // colors  16- 31 with lookup
	GFXDECODE_ENTRY( "gfx", 0x60000, spritelayout8, 0x200, 16 )    // to handle 8x8 sprites
GFXDECODE_END

/*************************************
 *
 *  Interrupt generator
 *
 *************************************/

void jackal_state::vblank_irq(int state)
{
	if (state && m_irq_enable)
	{
		m_maincpu->set_input_line(0, HOLD_LINE);
		m_slavecpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}
}


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void jackal_state::machine_start()
{
	m_mainbank->configure_entries(0, 2, memregion("maincpu")->base() + 0x4000, 0x8000);
	m_mainbank->set_entry(0);

	save_item(NAME(m_irq_enable));
}

void jackal_state::machine_reset()
{
	m_irq_enable = 0;
}

void jackal_state::jackal(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, 18.432_MHz_XTAL / 12); // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &jackal_state::main_map);

	MC6809E(config, m_slavecpu, 18.432_MHz_XTAL / 12); // verified on PCB
	m_slavecpu->set_addrmap(AS_PROGRAM, &jackal_state::slave_map);

	config.set_maximum_quantum(attotime::from_hz(6000));

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(1*8, 31*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(jackal_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(jackal_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_jackal);
	PALETTE(config, m_palette, FUNC(jackal_state::palette));
	m_palette->set_format(palette_device::xBGR_555, 0x300, 0x200);
	m_palette->set_endianness(ENDIANNESS_LITTLE);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	YM2151(config, "ymsnd", 3.579545_MHz_XTAL).add_route(0, "speaker", 0.50, 0).add_route(1, "speaker", 0.50, 1); // verified on PCB
}

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( jackal ) // 8-Way Joystick: You can only shoot in one direction regardless of travel - up the screen
	ROM_REGION( 0x14000, "maincpu", 0 ) // Banked 64k for 1st CPU
	ROM_LOAD( "631_v03.16d", 0x00000, 0x04000, CRC(3e0dfb83) SHA1(5ba7073751eee33180e51143b348256597909516) )
	ROM_LOAD( "631_v02.15d", 0x04000, 0x10000, CRC(0b7e0584) SHA1(e4019463345a4c020d5a004c9a400aca4bdae07b) )

	ROM_REGION( 0x8000, "slave", 0 ) // 64k for 2nd cpu (Graphics & Sound)
	ROM_LOAD( "631_t01.11d", 0x0000, 0x8000, CRC(b189af6a) SHA1(f7df996c394fdd6f2ce128a8df38d7838f7ec6d6) )

	ROM_REGION( 0x80000, "gfx", 0 )
	ROM_LOAD16_BYTE( "631t04.7h",  0x00000, 0x20000, CRC(457f42f0) SHA1(08413a13d128875dddcf4f6ad302363096bf1d41) ) // Silkscreened MASK1M
	ROM_LOAD16_BYTE( "631t05.8h",  0x00001, 0x20000, CRC(732b3fc1) SHA1(7e89650b9e5e2b7ae82f8c55ac9995740f6fdfe1) ) // Silkscreened MASK1M
	ROM_LOAD16_BYTE( "631t06.12h", 0x40000, 0x20000, CRC(2d10e56e) SHA1(447b464ea725fb9ef87da067a41bcf463b427cce) ) // Silkscreened MASK1M
	ROM_LOAD16_BYTE( "631t07.13h", 0x40001, 0x20000, CRC(4961c397) SHA1(b430df58fc3bb722d6fb23bed7d04afdb7e5d9c1) ) // Silkscreened MASK1M

	ROM_REGION( 0x0200, "proms", 0 ) // color lookup tables
	ROM_LOAD( "631r08.9h",  0x0000, 0x0100, CRC(7553a172) SHA1(eadf1b4157f62c3af4602da764268df954aa0018) ) // MMI 63S141AN or compatible (silkscreened 6301)
	ROM_LOAD( "631r09.14h", 0x0100, 0x0100, CRC(a74dd86c) SHA1(571f606f8fc0fd3d98d26761de79ccb4cc9ab044) ) // MMI 63S141AN or compatible (silkscreened 6301)
ROM_END

ROM_START( jackalr ) // Rotary Joystick: Shot direction is controlled via the rotary function of the joystick
	ROM_REGION( 0x14000, "maincpu", 0 ) // Banked 64k for 1st CPU
	ROM_LOAD( "631_q03.16d", 0x00000, 0x04000, CRC(b9d34836) SHA1(af23a0c844fb9e60a757511ca898d73eef4c2e51) )
	ROM_LOAD( "631_q02.15d", 0x04000, 0x10000, CRC(ed2a7d66) SHA1(3d9b31fa8b31e509880d617feb0dd4bd9790d2d5) )

	ROM_REGION( 0x8000, "slave", 0 ) // 64k for 2nd cpu (Graphics & Sound)
	ROM_LOAD( "631_q01.11d", 0x0000, 0x8000, CRC(54aa2d29) SHA1(ebc6b3a5db5120cc33d62e3213d0e881f658282d) )

	ROM_REGION( 0x80000, "gfx", 0 ) // Paired ROMs are on tiny riser boards instead of larger single MASK1M type ROMs per socket
	ROM_LOAD16_BYTE( "631_q05.7h",  0x00000, 0x10000, CRC(bcf5d0a8) SHA1(c406d6b3eaf251d9505b809b9ef927ecd6d672c0) ) // ROM located over socket on riser for 7H socket == 631t04 1/2
	ROM_LOAD16_BYTE( "631_q06.8h",  0x00001, 0x10000, CRC(4cb5df22) SHA1(f4f6346459f4ddf6ddb34d143d77836d10710769) ) // ROM located over socket on riser for 8H socket == 631t05 1/2
	ROM_LOAD16_BYTE( "631_q04.7h",  0x20000, 0x10000, CRC(e1e9aa42) SHA1(5b12537b7263663ee7ab52f98328f2334066216e) ) // ROM located left edge of riser for 7H socket == 631t04 2/2
	ROM_LOAD16_BYTE( "631_q07.8h",  0x20001, 0x10000, CRC(cc68c5b8) SHA1(2fa475bd261532ded42240db14dcda4359bb94a3) ) // ROM located right edge of riser for 8H socket == 631t05 2/2
	ROM_LOAD16_BYTE( "631_q09.12h", 0x40000, 0x10000, CRC(55ea6852) SHA1(2027ff887ecc6b7a96f24b14bd6ac89197fd963f) ) // ROM located over socket on riser for 12H socket == 631t06 1/2
	ROM_LOAD16_BYTE( "631_q10.13h", 0x40001, 0x10000, CRC(fe93e217) SHA1(7b43ec09dd819fe9a9bdaffed0211e0359d17980) ) // ROM located over socket on riser for 13H socket == 631t07 1/2
	ROM_LOAD16_BYTE( "631_q08.12h", 0x60000, 0x10000, CRC(d2492b8b) SHA1(a2563d73f7e03b144367354a5c207303fd91f670) ) // ROM located left edge of riser for 12H socket == 621t06 2/2
	ROM_LOAD16_BYTE( "631_q11.13h", 0x60001, 0x10000, CRC(563ae24c) SHA1(224435bcbc543fe735521ec5a5a8bbd124d0a88a) ) // ROM located right edge of riser for 13H socket == 631t07 2/2

	ROM_REGION( 0x0200, "proms", 0 ) // color lookup tables
	ROM_LOAD( "631r08.9h",  0x0000, 0x0100, CRC(7553a172) SHA1(eadf1b4157f62c3af4602da764268df954aa0018) ) // MMI 63S141AN or compatible (silkscreened 6301)
	ROM_LOAD( "631r09.14h", 0x0100, 0x0100, CRC(a74dd86c) SHA1(571f606f8fc0fd3d98d26761de79ccb4cc9ab044) ) // MMI 63S141AN or compatible (silkscreened 6301)
ROM_END

ROM_START( topgunr ) // 8-Way Joystick: You can only shoot in one direction regardless of travel - up the screen
	ROM_REGION( 0x14000, "maincpu", 0 ) // Banked 64k for 1st CPU
	ROM_LOAD( "631_u03.16d", 0x00000, 0x04000, CRC(c086844e) SHA1(4d6f27ac3aabb4b2d673aa619e407e417ad89337) )
	ROM_LOAD( "631_u02.15d", 0x04000, 0x10000, CRC(f7e28426) SHA1(db2d5f252a574b8aa4d8406a8e93b423fd2a7fef) )

	ROM_REGION( 0x8000, "slave", 0 ) // 64k for 2nd cpu (Graphics & Sound)
	ROM_LOAD( "631_t01.11d", 0x0000, 0x8000, CRC(b189af6a) SHA1(f7df996c394fdd6f2ce128a8df38d7838f7ec6d6) )

	ROM_REGION( 0x80000, "gfx", 0 )
	ROM_LOAD16_BYTE( "631u04.7h",  0x00000, 0x20000, CRC(50122a12) SHA1(c9e0132a3a40d9d28685c867c70231947d8a9cb7) ) // Silkscreened MASK1M
	ROM_LOAD16_BYTE( "631u05.8h",  0x00001, 0x20000, CRC(6943b1a4) SHA1(40de2b434600ea4c8fb42e6b21be2c3705a55d67) ) // Silkscreened MASK1M
	ROM_LOAD16_BYTE( "631u06.12h", 0x40000, 0x20000, CRC(37dbbdb0) SHA1(f94db780d69e7dd40231a75629af79469d957378) ) // Silkscreened MASK1M
	ROM_LOAD16_BYTE( "631u07.13h", 0x40001, 0x20000, CRC(22effcc8) SHA1(4d174b0ce64def32050f87343c4b1424e0fef6f7) ) // Silkscreened MASK1M

	ROM_REGION( 0x0200, "proms", 0 ) // color lookup tables
	ROM_LOAD( "631r08.9h",  0x0000, 0x0100, CRC(7553a172) SHA1(eadf1b4157f62c3af4602da764268df954aa0018) ) // MMI 63S141AN or compatible (silkscreened 6301)
	ROM_LOAD( "631r09.14h", 0x0100, 0x0100, CRC(a74dd86c) SHA1(571f606f8fc0fd3d98d26761de79ccb4cc9ab044) ) // MMI 63S141AN or compatible (silkscreened 6301)
ROM_END

ROM_START( jackalj ) // 8-Way Joystick: You can only shoot in the direction you're traveling
	ROM_REGION( 0x14000, "maincpu", 0 ) // Banked 64k for 1st CPU
	ROM_LOAD( "631_t03.16d", 0x00000, 0x04000, CRC(fd5f9624) SHA1(2520c1ff54410ef498ecbf52877f011900baed4c) )
	ROM_LOAD( "631_t02.15d", 0x04000, 0x10000, CRC(14db6b1a) SHA1(b469ea50aa94a2bda3bd0442300aa1272e5f30c4) )

	ROM_REGION( 0x8000, "slave", 0 ) // 64k for 2nd cpu (Graphics & Sound)
	ROM_LOAD( "631_t01.11d", 0x0000, 0x8000, CRC(b189af6a) SHA1(f7df996c394fdd6f2ce128a8df38d7838f7ec6d6) )

	ROM_REGION( 0x80000, "gfx", 0 )
	ROM_LOAD16_BYTE( "631t04.7h",  0x00000, 0x20000, CRC(457f42f0) SHA1(08413a13d128875dddcf4f6ad302363096bf1d41) ) // Silkscreened MASK1M
	ROM_LOAD16_BYTE( "631t05.8h",  0x00001, 0x20000, CRC(732b3fc1) SHA1(7e89650b9e5e2b7ae82f8c55ac9995740f6fdfe1) ) // Silkscreened MASK1M
	ROM_LOAD16_BYTE( "631t06.12h", 0x40000, 0x20000, CRC(2d10e56e) SHA1(447b464ea725fb9ef87da067a41bcf463b427cce) ) // Silkscreened MASK1M
	ROM_LOAD16_BYTE( "631t07.13h", 0x40001, 0x20000, CRC(4961c397) SHA1(b430df58fc3bb722d6fb23bed7d04afdb7e5d9c1) ) // Silkscreened MASK1M

	ROM_REGION( 0x0200, "proms", 0 ) // color lookup tables
	ROM_LOAD( "631r08.9h",  0x0000, 0x0100, CRC(7553a172) SHA1(eadf1b4157f62c3af4602da764268df954aa0018) ) // MMI 63S141AN or compatible (silkscreened 6301)
	ROM_LOAD( "631r09.14h", 0x0100, 0x0100, CRC(a74dd86c) SHA1(571f606f8fc0fd3d98d26761de79ccb4cc9ab044) ) // MMI 63S141AN or compatible (silkscreened 6301)
ROM_END

ROM_START( jackalbl ) // This is based on jackalr. Was dumped from 2 different PCBs.
	ROM_REGION( 0x14000, "maincpu", 0 ) // Banked 64k for 1st CPU
	ROM_LOAD( "epr-a-2.bin", 0x00000, 0x04000, CRC(ae2a290a) SHA1(e9bee75a02aef5cf330dccb9e7a45b0171a8c1d7) ) // also found labeled "2.20"
	ROM_LOAD( "epr-a-3.bin", 0x04000, 0x08000, CRC(5fffee27) SHA1(224d5fd26dd1e0f15a3c99fd2fffbb76f641416e) ) // also found labeled "3.17"
	ROM_LOAD( "epr-a-4.bin", 0x0c000, 0x08000, CRC(976c8431) SHA1(c199f57c25380d741aec85b0e0bfb6acf383e6a6) ) // also found labeled "4.18"

	ROM_REGION( 0x8000, "slave", 0 ) // 64k for 2nd cpu (Graphics & Sound)
	ROM_LOAD( "epr-a-1.bin", 0x0000, 0x8000, CRC(54aa2d29) SHA1(ebc6b3a5db5120cc33d62e3213d0e881f658282d) ) // also found labeled "1.19"

	ROM_REGION( 0x80000, "gfx", 0 ) // same data, different layout
	ROM_LOAD16_WORD_SWAP( "epr-a-17.bin", 0x00000, 0x08000, CRC(a96720b6) SHA1(d3c2a1848fa9d9d1232e58e412bdd69032fe2c83) ) // also found labeled "17.5"
	ROM_LOAD16_WORD_SWAP( "epr-a-18.bin", 0x08000, 0x08000, CRC(932d0ecb) SHA1(20bf789f45c5b3ba90012e1a945523236578a014) ) // also found labeled "18.6"
	ROM_LOAD16_WORD_SWAP( "epr-a-19.bin", 0x10000, 0x08000, CRC(1e3412e7) SHA1(dc0be23d6c89b7b131c3bd5cd117123e5f9d971c) ) // also found labeled "19.7"
	ROM_LOAD16_WORD_SWAP( "epr-a-20.bin", 0x18000, 0x08000, CRC(4b0d15be) SHA1(657c861357b5881e4ff356b1f27345b11e6c0696) ) // also found labeled "20.8"
	ROM_LOAD16_WORD_SWAP( "epr-a-6.bin",  0x20000, 0x08000, CRC(ec7141ad) SHA1(eb631ca58364827659fba1cb3dca326c2e5bf5b7) ) // also found labeled "6.9"
	ROM_LOAD16_WORD_SWAP( "epr-a-5.bin",  0x28000, 0x08000, CRC(c6375c74) SHA1(55090485307c6581556632fadbf704431734b145) ) // also found labeled "5.10"
	ROM_LOAD16_WORD_SWAP( "epr-a-7.bin",  0x30000, 0x08000, CRC(03e1de04) SHA1(c5f17633f4d5907310effb490488053861a55f6c) ) // also found labeled "7.11"
	ROM_LOAD16_WORD_SWAP( "epr-a-8.bin",  0x38000, 0x08000, CRC(f946ada7) SHA1(fd9a0786436cbdb4c844f71342232e4e6645d98f) ) // also found labeled "8.12"
	ROM_LOAD16_WORD_SWAP( "epr-a-13.bin", 0x40000, 0x08000, CRC(7c29c59e) SHA1(c2764f99ab4b39e7c0e43f58f69d6c353d0357aa) ) // also found labeled "13.1"
	ROM_LOAD16_WORD_SWAP( "epr-a-14.bin", 0x48000, 0x08000, CRC(f2bbff39) SHA1(f39dfdb3301f9b01d58071ffee41c467757d99d9) ) // also found labeled "14.2"
	ROM_LOAD16_WORD_SWAP( "epr-a-15.bin", 0x50000, 0x08000, CRC(594dbaaf) SHA1(e3f05acbdba8e8644dbabb065832476c1cb20569) ) // also found labeled "15.3"
	ROM_LOAD16_WORD_SWAP( "epr-a-16.bin", 0x58000, 0x08000, CRC(069bf945) SHA1(93ecc7dd779d16d825680bbc4986a312758db52f) ) // also found labeled "16.4"
	ROM_LOAD16_WORD_SWAP( "epr-a-9.bin",  0x60000, 0x08000, CRC(c00cef79) SHA1(94af2f4a67d1425fbb64b58ed7e618c8b38df203) ) // also found labeled "9.13"
	ROM_LOAD16_WORD_SWAP( "epr-a-10.bin", 0x68000, 0x08000, CRC(0aed6cd7) SHA1(88f81be5d8b2679349bf5cf2d4e02aff25c5118b) ) // also found labeled "10.14"
	ROM_LOAD16_WORD_SWAP( "epr-a-11.bin", 0x70000, 0x08000, CRC(a48e9f60) SHA1(6d5af16c16b40fb092fdba6dce852b94ac4767f4) ) // also found labeled "11.15"
	ROM_LOAD16_WORD_SWAP( "epr-a-12.bin", 0x78000, 0x08000, CRC(79b7c71c) SHA1(8510226114ab9098ec48e02840465fc8b69b5262) ) // also found labeled "12.16"

	ROM_REGION( 0x0200, "proms", 0 ) // color lookup tables
	ROM_LOAD( "n82s129n.prom2", 0x0000, 0x0100, CRC(7553a172) SHA1(eadf1b4157f62c3af4602da764268df954aa0018) )
	ROM_LOAD( "n82s129n.prom1", 0x0100, 0x0100, CRC(a74dd86c) SHA1(571f606f8fc0fd3d98d26761de79ccb4cc9ab044) )

	ROM_REGION( 0x1000, "pals", 0 ) // currently not used by the emulation
	ROM_LOAD( "pal16r6cn.pal1",     0x0000, 0x0104, CRC(9bba948f) SHA1(5f42568489f16f8b3719eb2ec178e7c61d7ce25f) )
	ROM_LOAD( "ampal16l8pc.pal2",   0x0200, 0x0104, CRC(17c9de2f) SHA1(2db42618f9ca1174bdcdbf92ea91ebc1a79bc6d2) )
	ROM_LOAD( "ampal16r4pc.pal3",   0x0400, 0x0104, CRC(e54cd288) SHA1(5b8ae5a2a4a9ec3fab603b063fd18c96dd1fd0cf) )
	ROM_LOAD( "pal16r8acn.pal4",    0x0600, 0x0104, CRC(5cc45e00) SHA1(cbd871addbac6310c8593fe8e8d5a962c2b45b2c) )
	ROM_LOAD( "pal20l8a-2cns.pal5", 0x0800, 0x0144, NO_DUMP ) // read protected
	ROM_LOAD( "pal20l8acns.pal6",   0x0a00, 0x0144, NO_DUMP ) // read protected
	ROM_LOAD( "pal16l8pc.pal7",     0x0c00, 0x0104, CRC(e8cdc259) SHA1(6917ef8f4f09aa099a48b8ae2d10bcd10961fb43) )
	ROM_LOAD( "d5c121.ep1200",      0x0e00, 0x0200, NO_DUMP ) // not dumped yet
ROM_END

ROM_START( topgunbl ) // Rotary Joystick: Shot direction is controlled via the Rotary function of the joystick
	ROM_REGION( 0x14000, "maincpu", 0 ) // Banked 64k for 1st CPU
	ROM_LOAD( "t-2.c6", 0x00000, 0x04000, CRC(d53172e5) SHA1(44b7f180c17f9a121a2f06f2d3471920a8989e21) )
	ROM_LOAD( "t-3.c5", 0x04000, 0x08000, CRC(7826ad38) SHA1(875e87867924905b9b83bc203eb7ffe81cf72233) )
	ROM_LOAD( "t-4.c4", 0x0c000, 0x08000, CRC(976c8431) SHA1(c199f57c25380d741aec85b0e0bfb6acf383e6a6) ) // == 2nd half of 631_q02.15d

	ROM_REGION( 0x8000, "slave", 0 ) // 64k for 2nd cpu (Graphics & Sound)
	ROM_LOAD( "t-1.c14", 0x0000, 0x8000, CRC(54aa2d29) SHA1(ebc6b3a5db5120cc33d62e3213d0e881f658282d) ) // == 631_q01.11d

	ROM_REGION( 0x80000, "gfx", 0 ) // same data, different layout
	ROM_LOAD16_WORD_SWAP( "t-17.n12", 0x00000, 0x08000, CRC(e8875110) SHA1(73f4c47ab039dce8c285bf222253084c860c95bf) )
	ROM_LOAD16_WORD_SWAP( "t-18.n13", 0x08000, 0x08000, CRC(cf14471d) SHA1(896aa8d7c93f837f6661d30bd0d6e19d16669107) )
	ROM_LOAD16_WORD_SWAP( "t-19.n14", 0x10000, 0x08000, CRC(46ee5dd2) SHA1(1a910984a197af341f13b4683babee857aafb245) )
	ROM_LOAD16_WORD_SWAP( "t-20.n15", 0x18000, 0x08000, CRC(3f472344) SHA1(49b9da8741b8e474d25726a706cf3008096ab2dc) )
	ROM_LOAD16_WORD_SWAP( "t-6.n1",   0x20000, 0x08000, CRC(539cc48c) SHA1(476ff5fe239e5acb61ede4d745d327f6bc3709f3) )
	ROM_LOAD16_WORD_SWAP( "t-5.m1",   0x28000, 0x08000, CRC(dbc26afe) SHA1(faab1feae91a9c22c008555955596c55d77b70c7) )
	ROM_LOAD16_WORD_SWAP( "t-7.n2",   0x30000, 0x08000, CRC(0ecd31b1) SHA1(06d77159ed55c1e288f2a194cdb09d29542e06d6) )
	ROM_LOAD16_WORD_SWAP( "t-8.n3",   0x38000, 0x08000, CRC(f946ada7) SHA1(fd9a0786436cbdb4c844f71342232e4e6645d98f) )
	ROM_LOAD16_WORD_SWAP( "t-13.n8",  0x40000, 0x08000, CRC(5d669abb) SHA1(faba6d7b47caae2ecdf15fb3527824bdb22e3d6b) )
	ROM_LOAD16_WORD_SWAP( "t-14.n9",  0x48000, 0x08000, CRC(f349369b) SHA1(f19238ef5feb1c89ef58c17a2506cc96ed8054e1) )
	ROM_LOAD16_WORD_SWAP( "t-15.n10", 0x50000, 0x08000, CRC(7c5a91dd) SHA1(85a1d76efc385e8e971a65e225de7f5d100bfbc7) )
	ROM_LOAD16_WORD_SWAP( "t-16.n11", 0x58000, 0x08000, CRC(5ec46d8e) SHA1(350e983b56a9f7d95e98429ee9a5fa6d3af36db4) )
	ROM_LOAD16_WORD_SWAP( "t-9.n4",   0x60000, 0x08000, CRC(8269caca) SHA1(8b80b7bad966d5b61a5c22d2ced625b5645f2ce2) )
	ROM_LOAD16_WORD_SWAP( "t-10.n5",  0x68000, 0x08000, CRC(25393e4f) SHA1(f6d7995b51d5bbbc3e325d6949dbc435446b5cf9) )
	ROM_LOAD16_WORD_SWAP( "t-11.n6",  0x70000, 0x08000, CRC(7895c22d) SHA1(c81ae51116fb32ac99d37eb7c2000c990d089b8d) )
	ROM_LOAD16_WORD_SWAP( "t-12.n7",  0x78000, 0x08000, CRC(15606dfc) SHA1(829492da49dbe70f81d15237803c5203aa011957) )

	ROM_REGION( 0x0200, "proms", 0 ) // color lookup tables
	ROM_LOAD( "631r08.bpr", 0x0000, 0x0100, CRC(7553a172) SHA1(eadf1b4157f62c3af4602da764268df954aa0018) )
	ROM_LOAD( "631r09.bpr", 0x0100, 0x0100, CRC(a74dd86c) SHA1(571f606f8fc0fd3d98d26761de79ccb4cc9ab044) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1986, jackal,   0,      jackal, jackal,  jackal_state, empty_init, ROT90, "Konami",  "Jackal (World, 8-way Joystick)",               MACHINE_SUPPORTS_SAVE )
GAME( 1986, jackalr,  jackal, jackal, jackalr, jackal_state, empty_init, ROT90, "Konami",  "Jackal (World, Rotary Joystick)",              MACHINE_SUPPORTS_SAVE )
GAME( 1986, topgunr,  jackal, jackal, jackal,  jackal_state, empty_init, ROT90, "Konami",  "Top Gunner (US, 8-way Joystick)",              MACHINE_SUPPORTS_SAVE )
GAME( 1986, jackalj,  jackal, jackal, jackal,  jackal_state, empty_init, ROT90, "Konami",  "Tokushu Butai Jackal (Japan, 8-way Joystick)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, jackalbl, jackal, jackal, jackalr, jackal_state, empty_init, ROT90, "bootleg", "Jackal (bootleg, Rotary Joystick)",            MACHINE_SUPPORTS_SAVE )
GAME( 1986, topgunbl, jackal, jackal, jackalr, jackal_state, empty_init, ROT90, "bootleg", "Top Gunner (bootleg, Rotary Joystick)",        MACHINE_SUPPORTS_SAVE )

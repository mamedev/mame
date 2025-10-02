// license:GPL-2.0+
// copyright-holders:Jarek Burczynski
/***************************************************************************

Tank Busters
6009-A + 6009-B PCBs

driver by Jarek Burczynski


Note:
    To enter the test mode:
    reset the game and keep start1 and start2 buttons pressed.

To do:
    - verify colors: PROM to output mapping is unknown, resistor values are guess
    - remove the 'some_changing_input' hack (see below)
    - from time to time the game just hangs

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_SCROLL     (1U << 1)
#define LOG_SOUND      (1U << 2)
#define LOG_E0XX       (1U << 3)

//#define VERBOSE (LOG_GENERAL | LOG_SCROLL | LOG_SOUND | LOG_E0XX)

#include "logmacro.h"

#define LOGSCROLL(...)    LOGMASKED(LOG_SCROLL,    __VA_ARGS__)
#define LOGSOUND(...)     LOGMASKED(LOG_SOUND,     __VA_ARGS__)
#define LOGE0XX(...)      LOGMASKED(LOG_E0XX,      __VA_ARGS__)


namespace {

class tankbust_state : public driver_device
{
public:
	tankbust_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_txtram(*this, "txtram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_mainbank(*this, "mainbank%u", 1U)
	{ }

	void tankbust(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_txtram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_memory_bank_array<2> m_mainbank;

	int32_t m_latch = 0;
	uint32_t m_timer1 = 0;
	uint8_t m_e0xx_data[8];
	uint8_t m_variable_data = 0;
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_txt_tilemap = nullptr;
	uint8_t m_xscroll[2];
	uint8_t m_yscroll[2];
	uint8_t m_irq_mask = 0;

	void soundlatch_w(uint8_t data);
	void e0xx_w(offs_t offset, uint8_t data);
	uint8_t debug_output_area_r(offs_t offset);
	uint8_t some_changing_input();
	void background_videoram_w(offs_t offset, uint8_t data);
	void background_colorram_w(offs_t offset, uint8_t data);
	void txtram_w(offs_t offset, uint8_t data);
	void xscroll_w(offs_t offset, uint8_t data);
	void yscroll_w(offs_t offset, uint8_t data);
	uint8_t soundlatch_r();
	uint8_t soundtimer_r();
	[[maybe_unused]] uint8_t read_from_unmapped_memory();

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_txt_tile_info);

	void palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(vblank_irq);
	TIMER_CALLBACK_MEMBER(soundlatch_callback);
	TIMER_CALLBACK_MEMBER(soundirqline_callback);

	void main_map(address_map &map) ATTR_COLD;
	void map_cpu2(address_map &map) ATTR_COLD;
	void port_map_cpu2(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/* background tilemap :

0xc000-0xc7ff:  xxxx xxxx tile code: 8 LSB bits

0xc800-0xcfff:  .... .xxx tile code: 3 MSB bits
                .... x... tile priority ON TOP of sprites (roofs and part of the rails)
                .xxx .... tile color code
                x... .... ?? set on *all* roofs (a different bg/sprite priority ?)

note:
 - seems that the only way to get color test right is to swap bit 1 and bit 0 of color code

*/

TILE_GET_INFO_MEMBER(tankbust_state::get_bg_tile_info)
{
	int code = m_videoram[tile_index];
	int attr = m_colorram[tile_index];

	int color = ((attr >> 4) & 0x07);

	code |= (attr & 0x07) * 256;


#if 0
	if (attr & 0x08)  // priority bg/sprites (1 = this bg tile on top of sprites)
	{
		color = ((int) machine().rand()) & 0x0f;
	}
	if (attr&0x80)  // all the roofs of all buildings have this bit set. What's this ???
	{
		color = ((int) machine().rand()) & 0x0f;
	}
#endif

	// priority bg/sprites (1 = this bg tile on top of sprites)
	tileinfo.category = (attr & 0x08) >> 3;

	tileinfo.set(1,
			code,
			(color & 4) | ((color & 2) >> 1) | ((color & 1) << 1),
			0);
}

TILE_GET_INFO_MEMBER(tankbust_state::get_txt_tile_info)
{
	int code = m_txtram[tile_index];
	int color = ((code >> 6) & 0x03);

	tileinfo.set(2,
			code & 0x3f,
			((color & 2) >> 1) | ((color & 1) << 1),
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void tankbust_state::video_start()
{
	// not scrollable
	m_txt_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tankbust_state::get_txt_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	// scrollable
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tankbust_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_txt_tilemap->set_transparent_pen(0);

	save_item(NAME(m_xscroll));
	save_item(NAME(m_yscroll));
}


/***************************************************************************

  Memory handlers

***************************************************************************/

void tankbust_state::background_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void tankbust_state::background_colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void tankbust_state::txtram_w(offs_t offset, uint8_t data)
{
	m_txtram[offset] = data;
	m_txt_tilemap->mark_tile_dirty(offset);
}

void tankbust_state::xscroll_w(offs_t offset, uint8_t data)
{
	if (m_xscroll[offset] != data)
	{
		m_xscroll[offset] = data;

		int x = m_xscroll[0] + 256 * (m_xscroll[1] & 1);
		if (x >= 0x100) x -= 0x200;
		m_bg_tilemap->set_scrollx(0, x);
	}

	LOGSCROLL("x = %02x %02x", m_xscroll[0], m_xscroll[1]);
}


void tankbust_state::yscroll_w(offs_t offset, uint8_t data)
{
	if (m_yscroll[offset] != data)
	{
		m_yscroll[offset] = data;
		int y = m_yscroll[0];
		if (y >= 0x80) y -= 0x100;
		m_bg_tilemap->set_scrolly(0, y);
	}

	LOGSCROLL("y = %02x %02x", m_yscroll[0], m_yscroll[1]);
}

/***************************************************************************

  Display refresh

***************************************************************************/
/*
spriteram format (4 bytes per sprite):

    offset  0   x.......    flip X
    offset  0   .x......    flip Y
    offset  0   ..xxxxxx    gfx code (6 bits)

    offset  1   xxxxxxxx    y position

    offset  2   ??????..    not used ?
    offset  2   ......?.    used but unknown ??? (color code ? or x ?)
    offset  2   .......x    x position (1 MSB bit)

    offset  3   xxxxxxxx    x position (8 LSB bits)
*/

void tankbust_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		int code  = m_spriteram[offs + 0] & 0x3f;
		int flipy = m_spriteram[offs + 0] & 0x40;
		int flipx = m_spriteram[offs + 0] & 0x80;

		int sy = (240 - m_spriteram[offs + 1]) - 14;
		int sx = (m_spriteram[offs + 2] & 0x01) * 256 + m_spriteram[offs + 3] - 7;

		int color = 0;

		// 0x02 - don't know (most of the time this bit is set in tank sprite and others but not all and not always)
		// 0x04 - not used
		// 0x08 - not used
		// 0x10 - not used
		// 0x20 - not used
		// 0x40 - not used
		// 0x80 - not used
#if 0
		if ((m_spriteram[offs + 2] & 0x02))
		{
			code = ((int) machine().rand()) & 63;
		}
#endif

		if ((m_spriteram[offs + 1] != 4)) // otherwise - ghost sprites
		{
			m_gfxdecode->gfx(0)->transpen(bitmap, cliprect,
				code, color,
				flipx, flipy,
				sx, sy, 0);
		}
	}
}


uint32_t tankbust_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
#if 0
	for (int i = 0; i < 0x800; i++)
	{
		int tile_attrib = m_colorram[i];

		if ((tile_attrib & 8) || (tile_attrib & 0x80))
		{
			m_bg_tilemap->mark_tile_dirty(i);
		}
	}
#endif

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 1, 0);

	m_txt_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


void tankbust_state::machine_start()
{
	m_mainbank[0]->configure_entries(0, 2, memregion("maincpu")->base() + 0x10000, 0x4000);
	m_mainbank[1]->configure_entries(0, 2, memregion("maincpu")->base() + 0x18000, 0x2000);

	save_item(NAME(m_latch));
	save_item(NAME(m_timer1));
	save_item(NAME(m_e0xx_data));
	save_item(NAME(m_variable_data));
	save_item(NAME(m_irq_mask));
}

// port A of ay8910#0

TIMER_CALLBACK_MEMBER(tankbust_state::soundlatch_callback)
{
	m_latch = param;
}

void tankbust_state::soundlatch_w(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(tankbust_state::soundlatch_callback), this), data);
}

uint8_t tankbust_state::soundlatch_r()
{
	return m_latch;
}

// port B of ay8910#0
uint8_t tankbust_state::soundtimer_r()
{
	m_timer1++;
	int ret = m_timer1;
	return ret;
}

TIMER_CALLBACK_MEMBER(tankbust_state::soundirqline_callback)
{
	LOGSOUND("sound_irq_line write = %2x (after CPUs synced) \n", param);

	if ((param & 1) == 0)
		m_subcpu->set_input_line(0, HOLD_LINE);
}



void tankbust_state::e0xx_w(offs_t offset, uint8_t data)
{
	m_e0xx_data[offset] = data;

	LOGE0XX("e0: %x %x (%x cnt) %x %x %x %x",
		m_e0xx_data[0], m_e0xx_data[1],
		m_e0xx_data[2], m_e0xx_data[3],
		m_e0xx_data[4], m_e0xx_data[5],
		m_e0xx_data[6]);

	switch (offset)
	{
	case 0: // 0xe000 interrupt enable
		m_irq_mask = data & 1;
		break;

	case 1: // 0xe001 (value 0 then 1) written right after the soundlatch_byte_w
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(tankbust_state::soundirqline_callback), this), data);
		break;

	case 2: // 0xe002 coin counter
		machine().bookkeeping().coin_counter_w(0, data & 1);
		break;

	case 6: // 0xe006 screen disable ?? or disable screen update
		/* program sets this to 0,
		   clears screen memory,
		   and sets this to 1 */

		// ????
		break;

	case 7: // 0xe007 bankswitch
		// bank 1 at 0x6000-9fff = from 0x10000 when bit0 = 0 else from 0x14000 */
		m_mainbank[0]->set_entry(data & 1);

		// bank 2 at 0xa000-bfff = from 0x18000 when bit0 = 0 else from 0x1a000
		m_mainbank[1]->set_entry(data & 1); // verified (the game will reset after the "game over" otherwise)
		break;
	}
}

uint8_t tankbust_state::debug_output_area_r(offs_t offset)
{
	return m_e0xx_data[offset];
}




void tankbust_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	for (int i = 0; i < 128; i++)
	{
		// 7 6   5 4 3   2 1 0
		// bb    r r r   g g g - bad (for sure - no green for tank)
		// bb    g g g   r r r - bad (for sure - no yellow, no red)
		// gg    r r r   b b b - bad
		// gg    b b b   r r r - bad
		// rr    b b b   g g g - bad

		// rr    g g g   b b b - very close (green,yellow,red present)

		// rr    r g g   g b b - bad
		// rr    r g g   b b b - bad
		// rr    g g g   b b r - bad

		// rr    g g b   b x x - bad (x: unused)
		// rr    g g x   x b b - bad but still close
		// rr    g g r   g b b - bad but still close
		// rr    g g g   r b b - bad but still close

		// blue component
		int bit0 = BIT(color_prom[i], 0);
		int bit1 = BIT(color_prom[i], 1);
		int bit2 = BIT(color_prom[i], 2);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// red component
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const r = 0x55 * bit0 + 0xaa * bit1;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

uint8_t tankbust_state::read_from_unmapped_memory()
{
	return 0xff;
}

uint8_t tankbust_state::some_changing_input()
{
	m_variable_data += 8;
	return m_variable_data;
}

void tankbust_state::main_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x9fff).bankr(m_mainbank[0]);
	map(0xa000, 0xbfff).bankr(m_mainbank[1]);
	map(0xc000, 0xc7ff).ram().w(FUNC(tankbust_state::background_videoram_w)).share(m_videoram);
	map(0xc800, 0xcfff).ram().w(FUNC(tankbust_state::background_colorram_w)).share(m_colorram);
	map(0xd000, 0xd7ff).ram().w(FUNC(tankbust_state::txtram_w)).share(m_txtram);
	map(0xd800, 0xd8ff).ram().share(m_spriteram);
	map(0xe000, 0xe007).rw(FUNC(tankbust_state::debug_output_area_r), FUNC(tankbust_state::e0xx_w));
	map(0xe800, 0xe800).portr("INPUTS").w(FUNC(tankbust_state::yscroll_w));
	map(0xe801, 0xe801).portr("SYSTEM");
	map(0xe802, 0xe802).portr("DSW");
	map(0xe801, 0xe802).w(FUNC(tankbust_state::xscroll_w));
	map(0xe803, 0xe803).rw(FUNC(tankbust_state::some_changing_input), FUNC(tankbust_state::soundlatch_w));   // unknown. Game expects this to change so this is not player input
	map(0xe804, 0xe804).nopw();    // watchdog ? ; written in long-lasting loops
	map(0xf000, 0xf7ff).ram();
	//map(0xf800, 0xffff).r(FUNC(tankbust_state::read_from_unmapped_memory));   // a bug in game code ?
}

void tankbust_state::port_map_cpu2(address_map &map)
{
	map.global_mask(0xff);
	map(0x10, 0x10).w("ay2", FUNC(ay8910_device::data_w));
	map(0x30, 0x30).rw("ay2", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w));
	map(0x40, 0x40).w("ay1", FUNC(ay8910_device::data_w));
	map(0xc0, 0xc0).rw("ay1", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w));
}


void tankbust_state::map_cpu2(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x3fff).nopw();    // garbage, written in initialization loop
	// 0x4000 and 0x4040-0x4045 seem to be used (referenced in the code)
	map(0x4000, 0x7fff).nopw();    // garbage, written in initialization loop
	map(0x8000, 0x87ff).ram();
}


static INPUT_PORTS_START( tankbust )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x08, DEF_STR( English ) )
	PORT_DIPSETTING(    0x00, DEF_STR( French ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x10, "No Bonus" )
	PORT_DIPSETTING(    0x00, "60000" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x20, "1C/1C 1C/2C 1C/6C 1C/14C" )
	PORT_DIPSETTING(    0x00, "2C/1C 1C/1C 1C/3C 1C/7C" )
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x00, "4" )
INPUT_PORTS_END

static const gfx_layout spritelayout =
{
	32,32,  // 32*32 pixels
	64,     // 64 sprites
	4,      // 4 bits per pixel
	{ 0, 8192*8*1, 8192*8*2, 8192*8*3 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7,
		32*8+0, 32*8+1, 32*8+2, 32*8+3, 32*8+4, 32*8+5, 32*8+6, 32*8+7,
		40*8+0, 40*8+1, 40*8+2, 40*8+3, 40*8+4, 40*8+5, 40*8+6, 40*8+7 },
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8,
		23*8, 22*8, 21*8, 20*8, 19*8, 18*8, 17*8, 16*8,
		71*8, 70*8, 69*8, 68*8, 67*8, 66*8, 65*8, 64*8,
		87*8, 86*8, 85*8, 84*8, 83*8, 82*8, 81*8, 80*8 },
	128*8   // every sprite takes 128 consecutive bytes
};

static const gfx_layout charlayout =
{
	8,8,    // 8*8 pixels
	2048,   // 2048 characters
	3,      // 3 bits per pixel
	{ 0, 16384*8*1, 16384*8*2 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	8*8     // every char takes 8 consecutive bytes
};

static const gfx_layout charlayout2 =
{
	8,8,    // 8*8 pixels
	256,    // 256 characters
	1,      // 1 bit per pixel - the data repeats 4 times within one ROM
	{ 0 }, /* , 2048*8*1, 2048*8*2, 2048*8*3 },*/
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8     // every char takes 8 consecutive bytes
};

static GFXDECODE_START( gfx_tankbust )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout, 0x00, 2 )   // 32x32  (2 * 16 colors)
	GFXDECODE_ENTRY( "bgtiles", 0, charlayout,   0x20, 8 )
	GFXDECODE_ENTRY( "txtiles", 0, charlayout2,  0x60, 16 )
GFXDECODE_END

void tankbust_state::machine_reset()
{
	m_variable_data = 0x11;
}

INTERRUPT_GEN_MEMBER(tankbust_state::vblank_irq)
{
	if (m_irq_mask)
		device.execute().set_input_line(0, HOLD_LINE);
}

void tankbust_state::tankbust(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(14'318'181) / 2);    // Verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &tankbust_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(tankbust_state::vblank_irq));

	Z80(config, m_subcpu, XTAL(14'318'181) / 4);        // Verified on PCB
//  Z80(config, m_subcpu, XTAL(14'318'181) / 3);        // Accurate to audio recording, but apparently incorrect clock
	m_subcpu->set_addrmap(AS_PROGRAM, &tankbust_state::map_cpu2);
	m_subcpu->set_addrmap(AS_IO, &tankbust_state::port_map_cpu2);

	config.set_maximum_quantum(attotime::from_hz(6000));


	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size( 64*8, 32*8);
	screen.set_visarea( 16*8, 56*8-1, 1*8, 31*8-1);
//  screen.set_visarea(  0*8, 64*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(tankbust_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tankbust);
	PALETTE(config, m_palette, FUNC(tankbust_state::palette), 128);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ay8910_device &ay1(AY8910(config, "ay1", XTAL(14'318'181) / 16));  // Verified on PCB
	ay1.port_a_read_callback().set(FUNC(tankbust_state::soundlatch_r));
	ay1.port_b_read_callback().set(FUNC(tankbust_state::soundtimer_r));
	ay1.add_route(ALL_OUTPUTS, "mono", 0.10);

	AY8910(config, "ay2", XTAL(14'318'181) / 16).add_route(ALL_OUTPUTS, "mono", 0.10);  // Verified on PCB
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( tankbust )
	ROM_REGION( 0x1c000, "maincpu", 0 )
	ROM_LOAD( "a-s4-6.bin",     0x00000, 0x4000, CRC(8ebe7317) SHA1(bc45d530ad6335312c9c3efdcedf7acd2cdeeb55) )
	ROM_LOAD( "a-s7-9.bin",     0x04000, 0x2000, CRC(047aee33) SHA1(62ee776c403b228e065baa9218f32597951ca935) )

	ROM_LOAD( "a-s5_7.bin",     0x12000, 0x2000, CRC(dd4800ca) SHA1(73a6caa029c27fb45217f9372d9541c6fe206f08) ) // banked at 0x6000-0x9fff
	ROM_CONTINUE(               0x10000, 0x2000)

	ROM_LOAD( "a-s6-8.bin",     0x16000, 0x2000, CRC(f8801238) SHA1(fd3abe18542660a8c31dc316012a99d48c9bb5aa) ) // banked at 0x6000-0x9fff
	ROM_CONTINUE(               0x14000, 0x2000)

//  ROM_LOAD( "a-s5_7.bin",     0x10000, 0x4000, CRC(dd4800ca) SHA1(73a6caa029c27fb45217f9372d9541c6fe206f08) ) // banked at 0x6000-0x9fff
//  ROM_LOAD( "a-s6-8.bin",     0x14000, 0x4000, CRC(f8801238) SHA1(fd3abe18542660a8c31dc316012a99d48c9bb5aa) ) // banked at 0x6000-0x9fff

	ROM_LOAD( "a-s8-10.bin",    0x18000, 0x4000, CRC(9e826faa) SHA1(6a252428c69133d3e9d7a9938140d5ae37fb0c7d) ) // banked at 0xa000-0xbfff

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "a-b3-1.bin",     0x0000, 0x2000, CRC(b0f56102) SHA1(4f427c3bd6131b7cba42a0e24a69bd1b6a1b0a3c) )

	ROM_REGION( 0x8000, "sprites", 0 ) // 32x32
	ROM_LOAD( "a-d5-2.bin",     0x0000, 0x2000, CRC(0bbf3fdb) SHA1(035c2db6eca701be690042e006c0d07c90d752f1) )
	ROM_LOAD( "a-d6-3.bin",     0x2000, 0x2000, CRC(4398dc21) SHA1(3b23433d0c9daa554ad6615af2fdec715e4e3794) )
	ROM_LOAD( "a-d7-4.bin",     0x4000, 0x2000, CRC(aca197fc) SHA1(03ecd94b84a31389539074079ed7f2a500e588ab) )
	ROM_LOAD( "a-d8-5.bin",     0x6000, 0x2000, CRC(1e6edc17) SHA1(4dbc91938c999348bcbd5f960fc3bb49f3174059) )

	ROM_REGION( 0xc000, "bgtiles", ROMREGION_INVERT ) // 8x8
	ROM_LOAD( "b-m4-11.bin",    0x0000, 0x4000, CRC(eb88ee1f) SHA1(60ec2d77186c196a27278b0639cbfa838986e2e2) )
	ROM_LOAD( "b-m5-12.bin",    0x4000, 0x4000, CRC(4c65f399) SHA1(72db15884f346c001d3b86cb33e3f6d339eedb56) )
	ROM_LOAD( "b-m6-13.bin",    0x8000, 0x4000, CRC(a5baa413) SHA1(dc772042706c3a92594ee8422aafed77375c0632) )

	ROM_REGION( 0x2000, "txtiles", 0 ) // 8x8
	ROM_LOAD( "b-r3-14.bin",    0x0000, 0x2000, CRC(4310a815) SHA1(bf58a7a8d3f82fcaa0c46d9ebb13cac1231b80ad) )

	ROM_REGION( 0x0080, "proms", 0 )
	ROM_LOAD( "tb-prom.1s8",    0x0000, 0x0020, CRC(dfaa086c) SHA1(f534aedddd18addd0833a3a28a4297689c4a46ac) ) // sprites
	ROM_LOAD( "tb-prom.2r8",    0x0020, 0x0020, CRC(ec50d674) SHA1(64c8961eca33b23e14b7383eb7e64fcac8772ee7) ) // background
	ROM_LOAD( "tb-prom.3p8",    0x0040, 0x0020, CRC(3e70eafd) SHA1(b200350a3f6c166228706734419dd3ef1207eeef) ) // background palette 2 ??
	ROM_LOAD( "tb-prom.4k8",    0x0060, 0x0020, CRC(624f40d2) SHA1(8421f1d774afc72e0817d41edae74a2837021a5f) ) // text
ROM_END

} // anonymous namespace


GAME( 1985, tankbust, 0, tankbust, tankbust, tankbust_state, empty_init, ROT90, "Valadon Automation", "Tank Busters", MACHINE_SUPPORTS_SAVE )

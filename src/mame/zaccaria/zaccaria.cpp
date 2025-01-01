// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Zaccaria Z80µP hardware

driver by Nicola Salmoria
thanks to Andrea Babich for the manual.

TODO:

- implement discrete filters for analog signals 1 to 5 and attenuation control
  for signal 5 (74LS156)

- The 8910 outputs go through some analog circuitry to make them sound more like
  real instruments.
  #0 Ch. A = "rullante"/"cassa" (drum roll/bass drum) (selected by bits 3&4 of port A)
  #0 Ch. B = "basso" (bass)
  #0 Ch. C = straight out through an optional filter
  #1 Ch. A = "piano"
  #1 Ch. B = "tromba" (trumpet) (level selected by bit 0 of port A)
  #1 Ch. C = disabled (there's an open jumper, otherwise would go out through a filter)

- some minor color issues (see video)

- testing dips in service mode, they don't match what MAME's UI shows
  (i.e. dip 5 in the UI causes dip 8 to change in service mode display).
  A dip listing is available online.

- get rid of (&machine().system() == &GAME_NAME(monymony)) and check and see if the
  protection handler should also apply to monymony2 that was added later


Notes:
- The protection device at 1A on the ROM board (1B11147) is unidentified on the
  schematics but appears to be a PAL16L8 or PAL16R4. It sits on bits 4-7 of the
  data bus, and is read from locations where only bits 0-3 are connected to regular
  devices (6400-6407 has 4-bit RAM, while 6c00-6c07 has a 4-bit input port).

- The 6802 driving the TMS5220 has a push button connected to the NMI line. On
  Zaccaria pinballs, when pressed, this causes the speech 6802 and the slave
  sound 6802 to play a few speech effects and sound effects;
  This currently doesn't work in MAME, and tracing the code it seems the code is
  possibly broken, made up of nonfunctional leftovers of some of the pinball test
  mode code.

***************************************************************************/

#include "emu.h"
#include "zaccaria_a.h"

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/i8255.h"
#include "machine/watchdog.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

GAME_EXTERN(monymony);


namespace {

class zaccaria_state : public driver_device
{
public:
	zaccaria_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_audiopcb(*this, "audiopcb")
		, m_videoram(*this, "videoram")
		, m_attributesram(*this, "attributesram")
		, m_spriteram(*this, "spriteram%u", 1U)
		, m_dsw_port(*this, "DSW.%u", 0)
		, m_coins(*this, "COINS")
	{ }

	void zaccaria(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	uint8_t dsw_r();
	uint8_t prot1_r(offs_t offset);
	uint8_t prot2_r(offs_t offset);
	void coin_w(int state);
	void nmi_mask_w(int state);
	void videoram_w(offs_t offset, uint8_t data);
	void attributes_w(offs_t offset, uint8_t data);
	uint8_t read_attr(offs_t offset, int which);
	void update_colscroll();
	void flip_screen_x_w(int state);
	void dsw_sel_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(int state);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t *spriteram, int color, int section);

	void main_map(address_map &map) ATTR_COLD;

	required_device<cpu_device>                 m_maincpu;
	required_device<gfxdecode_device>           m_gfxdecode;
	required_device<palette_device>             m_palette;
	required_device<zac1b11142_audio_device>    m_audiopcb;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_attributesram;
	required_shared_ptr_array<uint8_t, 2> m_spriteram;

	required_ioport_array<3> m_dsw_port;
	required_ioport m_coins;

	uint8_t m_dsw_sel = 0;
	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_nmi_mask = 0;
};


void zaccaria_state::machine_start()
{
	save_item(NAME(m_dsw_sel));
	save_item(NAME(m_nmi_mask));
}

void zaccaria_state::machine_reset()
{
	m_dsw_sel = 0;
}



/***************************************************************************

  Convert the color PROMs into a more useable format.


Here's the hookup from the proms (82s131) to the r-g-b-outputs

     Prom 9F        74LS374
    -----------   ____________
       12         |  3   2   |---680 ohm----| blue out
       11         |  4   5   |---1k ohm-----| (+ 470 ohm pulldown)
       10         |  7   6   |---820 ohm-------|
        9         |  8   9   |---1k ohm--------| green out
     Prom 9G      |          |                 | (+ 390 ohm pulldown)
       12         |  13  12  |---1.2k ohm------|
       11         |  14  15  |---820 ohm----------|
       10         |  17  16  |---1k ohm-----------| red out
        9         |  18  19  |---1.2k ohm---------| (+ 390 ohm pulldown)
                  |__________|


***************************************************************************/

void zaccaria_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	static constexpr int resistances_rg[] = { 1200, 1000, 820 };
	static constexpr int resistances_b[]  = { 1000, 820 };

	double weights_rg[3], weights_b[2];
	compute_resistor_weights(0, 0xff, -1.0,
			3, resistances_rg, weights_rg, 390, 0,
			2, resistances_b,  weights_b,  470, 0,
			0, nullptr, nullptr, 0, 0);

	for (int i = 0; i < 0x200; i++)
	{
		/*
		  TODO: I'm not sure, but I think that pen 0 must always be black, otherwise
		  there's some junk brown background in Jack Rabbit.
		  From the schematics it seems that the background color can be changed, but
		  I'm not sure where it would be taken from; I think the high bits of
		  attributesram, but they are always 0 in these games so they would turn out
		  black anyway.
		 */
		if (!(i & 0x038))
			palette.set_indirect_color(i, rgb_t::black());
		else
		{
			int bit0, bit1, bit2;

			// red component
			bit0 = BIT(color_prom[i + 0x000], 3);
			bit1 = BIT(color_prom[i + 0x000], 2);
			bit2 = BIT(color_prom[i + 0x000], 1);
			int const r = combine_weights(weights_rg, bit0, bit1, bit2);

			// green component
			bit0 = BIT(color_prom[i + 0x000], 0);
			bit1 = BIT(color_prom[i + 0x200], 3);
			bit2 = BIT(color_prom[i + 0x200], 2);
			int const g = combine_weights(weights_rg, bit0, bit1, bit2);

			// blue component
			bit0 = BIT(color_prom[i + 0x200], 1);
			bit1 = BIT(color_prom[i + 0x200], 0);
			int const b = combine_weights(weights_b, bit0, bit1);

			palette.set_indirect_color(i, rgb_t(r, g, b));
		}
	}

	/* There are 512 unique colors, which seem to be organized in 8 blocks */
	/* of 64. In each block, colors are not in the usual sequential order */
	/* but in interleaved order, like Phoenix. Additionally, colors for */
	/* background and sprites are interleaved. */
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 4; j++)
			for (int k = 0; k < 8; k++)
				// swap j and k to make the colors sequential
				palette.set_pen_indirect(0 + 32 * i + 8 * j + k, 64 * i + 8 * k + 2 * j);

	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 4; j++)
			for (int k = 0; k < 8; k++)
				// swap j and k to make the colors sequential
				palette.set_pen_indirect(256 + 32 * i + 8 * j + k, 64 * i + 8 * k + 2 * j + 1);
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(zaccaria_state::get_tile_info)
{
	uint8_t attr = m_videoram[tile_index | 0x400];
	uint16_t code = m_videoram[tile_index] | ((attr & 0x03) << 8);
	attr = (attr & 0x0c) >> 2 | (read_attr(tile_index, 1) & 0x07) << 2;

	tileinfo.set(0, code, attr, 0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void zaccaria_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(zaccaria_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_bg_tilemap->set_scroll_cols(32);
}



/***************************************************************************

  Display refresh

***************************************************************************/

/* sprite format:

  76543210
0 xxxxxxxx x
1 x....... flipy
  .x...... flipx
  ..xxxxxx code low
2 xx...... code high
  ..xxx... ?
  .....xxx color
3 xxxxxxxx y

offsets 1 and 2 are swapped if accessed from spriteram2

*/
void zaccaria_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t *spriteram, int color, int section)
{
	int o1 = 1 + section;
	int o2 = 2 - section;

	for (int offs = 0; offs < 0x20; offs += 4)
	{
		int sx = spriteram[offs + 3] + 1;
		int sy = 242 - spriteram[offs];
		int flipx = spriteram[offs + o1] & 0x40;
		int flipy = spriteram[offs + o1] & 0x80;

		if (sx == 1) continue;

		if (flip_screen_x())
		{
			sx = 240 - sx;
			flipx = !flipx;
		}
		if (flip_screen_y())
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
				(spriteram[offs + o1] & 0x3f) + (spriteram[offs + o2] & 0xc0),
				((spriteram[offs + o2] & 0x07) << 2) | color,
				flipx, flipy, sx, sy, 0);
	}
}

uint32_t zaccaria_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect);

	// 3 layers of sprites, each with their own palette and priorities
	// Not perfect yet, does spriteram(1) layer have a priority bit somewhere?
	draw_sprites(bitmap, cliprect, m_spriteram[1], 2, 1);
	draw_sprites(bitmap, cliprect, m_spriteram[0], 1, 0);
	draw_sprites(bitmap, cliprect, m_spriteram[1] + 0x20, 0, 1);

	return 0;
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void zaccaria_state::vblank_irq(int state)
{
	if (state && m_nmi_mask)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

void zaccaria_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

void zaccaria_state::attributes_w(offs_t offset, uint8_t data)
{
	uint8_t prev = m_attributesram[offset];
	m_attributesram[offset] = data;

	if (prev != data)
	{
		if (offset & 1)
		{
			uint8_t mask = flip_screen_x() ? 0x1f : 0;
			for (int i = offset / 2; i < 0x400; i += 0x20)
				m_bg_tilemap->mark_tile_dirty(i ^ mask);
		}
		else
			update_colscroll();
	}
}

uint8_t zaccaria_state::read_attr(offs_t offset, int which)
{
	if (flip_screen_x()) offset ^= 0x1f;
	return m_attributesram[(offset << 1 & 0x3f) | (which & 1)];
}

void zaccaria_state::update_colscroll()
{
	for (int i = 0; i < 0x20; i++)
		m_bg_tilemap->set_scrolly(i, read_attr(i, 0));
}

void zaccaria_state::flip_screen_x_w(int state)
{
	flip_screen_x_set(state);
	update_colscroll();
}


void zaccaria_state::dsw_sel_w(uint8_t data)
{
	switch (data & 0xf0)
	{
	case 0xe0:
		m_dsw_sel = 0;
		break;

	case 0xd0:
		m_dsw_sel = 1;
		break;

	case 0xb0:
		m_dsw_sel = 2;
		break;

	default:
		logerror("%s: portsel = %02x\n", machine().describe_context(), data);
		break;
	}
}

uint8_t zaccaria_state::dsw_r()
{
	return m_dsw_port[m_dsw_sel]->read();
}


uint8_t zaccaria_state::prot1_r(offs_t offset)
{
	switch (offset)
	{
		case 0:
			return 0x50;    // Money Money

		case 4:
			return 0x40;    // Jack Rabbit

		case 6:
			if (&machine().system() == &GAME_NAME(monymony))
				return 0x70;    // Money Money
			return 0xa0;    // Jack Rabbit

		default:
			return 0;
	}
}

uint8_t zaccaria_state::prot2_r(offs_t offset)
{
	switch (offset)
	{
		case 0:
			return m_coins->read();   // bits 4 and 5 must be 0 in Jack Rabbit

		case 2:
			return 0x10;    // Jack Rabbit

		case 4:
			return 0x80;    // Money Money

		case 6:
			return 0x00;    // Money Money

		default:
			return 0;
	}
}


void zaccaria_state::coin_w(int state)
{
	machine().bookkeeping().coin_counter_w(0, state);
}

void zaccaria_state::nmi_mask_w(int state)
{
	m_nmi_mask = state;
	if (!m_nmi_mask)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void zaccaria_state::main_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x67ff).ram().w(FUNC(zaccaria_state::videoram_w)).share(m_videoram); // 6400-67ff is 4 bits wide
	map(0x6400, 0x6407).r(FUNC(zaccaria_state::prot1_r));
	map(0x6800, 0x683f).w(FUNC(zaccaria_state::attributes_w)).share(m_attributesram);
	map(0x6840, 0x685f).ram().share(m_spriteram[0]);
	map(0x6881, 0x68c0).ram().share(m_spriteram[1]);
	map(0x6c00, 0x6c07).mirror(0x81f8).r(FUNC(zaccaria_state::prot2_r)).w("mainlatch", FUNC(ls259_device::write_d0));
	map(0x6e00, 0x6e00).mirror(0x81f8).r(FUNC(zaccaria_state::dsw_r)).w(m_audiopcb, FUNC(zac1b11142_audio_device::hs_w));
	map(0x7000, 0x77ff).ram();
	map(0x7800, 0x7803).rw("ppi8255", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x7c00, 0x7c00).r("watchdog", FUNC(watchdog_timer_device::reset_r));
	map(0x8000, 0xdfff).rom();
}



/***************************************************************************

  Input ports

***************************************************************************/

static INPUT_PORTS_START( monymony )
	PORT_START("DSW.0")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW 5I:1,2")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x04, 0x00, "Infinite Lives (Cheat)")     PORT_DIPLOCATION("SW 5I:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW 5I:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW 5I:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x00, "Freeze" )                    PORT_DIPLOCATION("SW 5I:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Cross Hatch Pattern" )       PORT_DIPLOCATION("SW 5I:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW 5I:8") // random high scores?
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW.1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW 4I:1,2")
	PORT_DIPSETTING(    0x01, "200000" )
	PORT_DIPSETTING(    0x02, "300000" )
	PORT_DIPSETTING(    0x03, "400000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x04, 0x00, "Table Title" )               PORT_DIPLOCATION("SW 4I:3")
	PORT_DIPSETTING(    0x00, "Todays High Scores" )
	PORT_DIPSETTING(    0x04, "High Scores" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )           PORT_DIPLOCATION("SW 4I:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) )           PORT_DIPLOCATION("SW 4I:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) )           PORT_DIPLOCATION("SW 4I:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )           PORT_DIPLOCATION("SW 4I:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_HIGH )                    PORT_DIPLOCATION("SW 4I:8")

	PORT_START("DSW.2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW 3I:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x8c, 0x84, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW 3I:3,4,5")
	PORT_DIPSETTING(    0x8c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x88, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x84, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x70, 0x50, "Coin C" )                    PORT_DIPLOCATION("SW 3I:6,7,8")
	PORT_DIPSETTING(    0x70, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_7C ) )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	// other bits are outputs

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("audiopcb", FUNC(zac1b11142_audio_device::acs_r))
	// other bits come from a protection device
INPUT_PORTS_END

static INPUT_PORTS_START( jackrabt )
	PORT_INCLUDE( monymony )

	PORT_MODIFY("DSW.0")
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW 5I:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW.1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW 4I:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW 4I:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Table Title" )               PORT_DIPLOCATION("SW 4I:3")
	PORT_DIPSETTING(    0x00, "Todays High Scores" )
	PORT_DIPSETTING(    0x04, "High Scores" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW 4I:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW 4I:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW 4I:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW 4I:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



/***************************************************************************

  GFX layouts

***************************************************************************/

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};

static GFXDECODE_START( gfx_zaccaria )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x3_planar, 0, 32 )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout, 32*8, 32 )
GFXDECODE_END



/***************************************************************************

  Machine config

***************************************************************************/

void zaccaria_state::zaccaria(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 18.432_MHz_XTAL / 6);   // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &zaccaria_state::main_map);

//  config.set_maximum_quantum(attotime::from_hz(1000000));

	WATCHDOG_TIMER(config, "watchdog");

	ls259_device &mainlatch(LS259(config, "mainlatch")); // 3G on 1B1141 I/O (Z80) board
	mainlatch.q_out_cb<0>().set(FUNC(zaccaria_state::flip_screen_x_w)); // VCMA
	mainlatch.q_out_cb<1>().set(FUNC(zaccaria_state::flip_screen_y_set)); // HCMA
	mainlatch.q_out_cb<2>().set("audiopcb", FUNC(zac1b11142_audio_device::ressound_w)); // RESSOUND
	mainlatch.q_out_cb<6>().set(FUNC(zaccaria_state::coin_w)); // COUNT
	mainlatch.q_out_cb<7>().set(FUNC(zaccaria_state::nmi_mask_w)); // INTST

	i8255_device &ppi(I8255A(config, "ppi8255"));
	ppi.in_pa_callback().set_ioport("P1");
	ppi.in_pb_callback().set_ioport("P2");
	ppi.in_pc_callback().set_ioport("SYSTEM");
	ppi.out_pc_callback().set(FUNC(zaccaria_state::dsw_sel_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(18.432_MHz_XTAL / 3, 384, 0, 256, 264, 16, 240); // verified from schematics
	screen.set_screen_update(FUNC(zaccaria_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(zaccaria_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_zaccaria);
	PALETTE(config, m_palette, FUNC(zaccaria_state::palette), 32*8 + 32*8, 512);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	ZACCARIA_1B11142(config, "audiopcb").add_route(ALL_OUTPUTS, "speaker", 1.0);
}



/***************************************************************************

  ROM definitions

***************************************************************************/

ROM_START( monymony )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cpu1.1a",      0x0000, 0x1000, CRC(13c227ca) SHA1(be305d112917904dd130b08f6b5186e3fbcb858a) )
	ROM_CONTINUE(             0x8000, 0x1000 )
	ROM_LOAD( "cpu2.1b",      0x1000, 0x1000, CRC(87372545) SHA1(04618d007a93b3f6706f56b10bdf39727d7d748d) )
	ROM_CONTINUE(             0x9000, 0x1000 )
	ROM_LOAD( "cpu3.1c",      0x2000, 0x1000, CRC(6aea9c01) SHA1(36a57f4dfae52d674dcf55d2b93dbacf734866b1) )
	ROM_CONTINUE(             0xa000, 0x1000 )
	ROM_LOAD( "cpu4.1d",      0x3000, 0x1000, CRC(5fdec451) SHA1(0f955c907e0a61a725a951018fdf5cc321139863) )
	ROM_CONTINUE(             0xb000, 0x1000 )
	ROM_LOAD( "cpu5.2a",      0x4000, 0x1000, CRC(af830e3c) SHA1(bed57c341ae3500f147efe31bcf01f81466ec1c0) )
	ROM_CONTINUE(             0xc000, 0x1000 )
	ROM_LOAD( "cpu6.2c",      0x5000, 0x1000, CRC(31da62b1) SHA1(486f07087244f8537510afacb64ddd59eb512a4d) )
	ROM_CONTINUE(             0xd000, 0x1000 )

	ROM_REGION( 0x10000, "audiopcb:melodycpu", 0 ) // 64k for first 6802
	ROM_LOAD( "snd13.2g",     0x8000, 0x2000, CRC(78b01b98) SHA1(2aabed56cdae9463deb513c0c5021f6c8dfd271e) )
	ROM_LOAD( "snd9.1i",      0xc000, 0x2000, CRC(94e3858b) SHA1(04961f67b95798b530bd83355dec612389f22255) )

	ROM_REGION( 0x10000, "audiopcb:audiocpu", 0 ) // 64k for second 6802
	ROM_LOAD( "snd8.1h",      0x2000, 0x1000, CRC(aad76193) SHA1(e08fc184efced392ee902c4cc9daaaf3310cdfe2) )
	ROM_CONTINUE(             0x6000, 0x1000 )
	ROM_LOAD( "snd7.1g",      0x3000, 0x1000, CRC(1e8ffe3e) SHA1(858ee7abe88d5801237e519cae2b50ae4bf33a58) )
	ROM_CONTINUE(             0x7000, 0x1000 )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "bg1.2d",       0x0000, 0x2000, CRC(82ab4d1a) SHA1(5aaf42a508df236f2e7c844d377132d73053907b) )
	ROM_LOAD( "bg2.1f",       0x2000, 0x2000, CRC(40d4e4d1) SHA1(79cbade30f1c9269e70ddb9c4332cfe1e8dc50a9) )
	ROM_LOAD( "bg3.1e",       0x4000, 0x2000, CRC(36980455) SHA1(4140b0cd4137c8f209124b12d9c0eb3b04f91991) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "9g",  0x0000, 0x0200, CRC(fc9a0f21) SHA1(2a93d684645ee1b70315386127223151582ab370) )
	ROM_LOAD( "9f",  0x0200, 0x0200, CRC(93106704) SHA1(d3b8281c87d253a2ed40ff400438e879ca40c2b7) )
ROM_END

ROM_START( monymony2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cpu1.1a",      0x0000, 0x1000, CRC(907225b2) SHA1(88955d21deee8364e391413c8e59361ca5f7e534) )
	ROM_CONTINUE(             0x8000, 0x1000 )
	ROM_LOAD( "cpu2.1b",      0x1000, 0x1000, CRC(87372545) SHA1(04618d007a93b3f6706f56b10bdf39727d7d748d) )
	ROM_CONTINUE(             0x9000, 0x1000 )
	ROM_LOAD( "cpu3.1c",      0x2000, 0x1000, CRC(3c874c16) SHA1(5607475638c3c313a8150aaa0e3b653226c2442a) )
	ROM_CONTINUE(             0xa000, 0x1000 )
	ROM_LOAD( "cpu4.1d",      0x3000, 0x1000, CRC(5fdec451) SHA1(0f955c907e0a61a725a951018fdf5cc321139863) )
	ROM_CONTINUE(             0xb000, 0x1000 )
	ROM_LOAD( "cpu5.2a",      0x4000, 0x1000, CRC(af830e3c) SHA1(bed57c341ae3500f147efe31bcf01f81466ec1c0) )
	ROM_CONTINUE(             0xc000, 0x1000 )
	ROM_LOAD( "cpu6.2c",      0x5000, 0x1000, CRC(31da62b1) SHA1(486f07087244f8537510afacb64ddd59eb512a4d) )
	ROM_CONTINUE(             0xd000, 0x1000 )

	ROM_REGION( 0x10000, "audiopcb:melodycpu", 0 ) // 64k for first 6802
	ROM_LOAD( "snd13.2g",     0x8000, 0x2000, CRC(78b01b98) SHA1(2aabed56cdae9463deb513c0c5021f6c8dfd271e) )
	ROM_LOAD( "snd9.1i",      0xc000, 0x2000, CRC(94e3858b) SHA1(04961f67b95798b530bd83355dec612389f22255) )

	ROM_REGION( 0x10000, "audiopcb:audiocpu", 0 ) // 64k for second 6802
	ROM_LOAD( "snd8.1h",      0x2000, 0x1000, CRC(aad76193) SHA1(e08fc184efced392ee902c4cc9daaaf3310cdfe2) )
	ROM_CONTINUE(             0x6000, 0x1000 )
	ROM_LOAD( "snd7.1g",      0x3000, 0x1000, CRC(1e8ffe3e) SHA1(858ee7abe88d5801237e519cae2b50ae4bf33a58) )
	ROM_CONTINUE(             0x7000, 0x1000 )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "bg1.2d",       0x0000, 0x2000, CRC(82ab4d1a) SHA1(5aaf42a508df236f2e7c844d377132d73053907b) )
	ROM_LOAD( "bg2.1f",       0x2000, 0x2000, CRC(40d4e4d1) SHA1(79cbade30f1c9269e70ddb9c4332cfe1e8dc50a9) )
	ROM_LOAD( "bg3.1e",       0x4000, 0x2000, CRC(36980455) SHA1(4140b0cd4137c8f209124b12d9c0eb3b04f91991) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "9g",  0x0000, 0x0200, CRC(fc9a0f21) SHA1(2a93d684645ee1b70315386127223151582ab370) )
	ROM_LOAD( "9f",  0x0200, 0x0200, CRC(93106704) SHA1(d3b8281c87d253a2ed40ff400438e879ca40c2b7) )
ROM_END

ROM_START( jackrabt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cpu-01.1a",    0x0000, 0x1000, CRC(499efe97) SHA1(f0efc910a5343001b27637779e1d4de218d44a4e) )
	ROM_CONTINUE(             0x8000, 0x1000 )
	ROM_LOAD( "cpu-01.2l",    0x1000, 0x1000, CRC(4772e557) SHA1(71c1eb49c978799294e732e65a77eba330d8da9b) )
	ROM_LOAD( "cpu-01.3l",    0x2000, 0x1000, CRC(1e844228) SHA1(0525fe95a0f90c50b54c0bf618eb083ccf20e6c4) )
	ROM_LOAD( "cpu-01.4l",    0x3000, 0x1000, CRC(ebffcc38) SHA1(abaf0e96d92f9c828a95446af6d5301053416f3d) )
	ROM_LOAD( "cpu-01.5l",    0x4000, 0x1000, CRC(275e0ed6) SHA1(c0789007a4de1aa848b7e5d26cf9fe847cc5d8a4) )
	ROM_LOAD( "cpu-01.6l",    0x5000, 0x1000, CRC(8a20977a) SHA1(ba15f4c62f600372390e56c2067b4a8ab1f2dba9) )
	ROM_LOAD( "cpu-01.2h",    0x9000, 0x1000, CRC(21f2be2a) SHA1(7d10489ca7325eebfa309ae4ffd4962a4310c403) )
	ROM_LOAD( "cpu-01.3h",    0xa000, 0x1000, CRC(59077027) SHA1(d6c2e68b4b2f1dce8a2141ec259812e732c1c69c) )
	ROM_LOAD( "cpu-01.4h",    0xb000, 0x1000, CRC(0b9db007) SHA1(836f8cacf2a097fd80d5c045bdc49b3a3174b89e) )
	ROM_LOAD( "cpu-01.5h",    0xc000, 0x1000, CRC(785e1a01) SHA1(a748d300be9455cad4f912e01c2279bb8465edfe) )
	ROM_LOAD( "cpu-01.6h",    0xd000, 0x1000, CRC(dd5979cf) SHA1(e9afe7002b2258a1c3132bdd951c6e20d473fb6a) )
/* This set was also found with bigger program ROMs (but for the first which matches)
    ROM_LOAD( "cpu-01-2.1b",  0x1000, 0x1000, CRC(1af79299) SHA1(8e68606a31aa7a7940ff90a7059f56ca7db0ac7c) )
    ROM_CONTINUE(             0x9000, 0x1000)
    ROM_LOAD( "cpu-01-3.1c",  0x2000, 0x1000, CRC(a02d5bc7) SHA1(1cb0ad29e7895b80053212bdf9aab9efe334326a) )
    ROM_CONTINUE(             0xa000, 0x1000)
    ROM_LOAD( "cpu-01-4.1d",  0x3000, 0x1000) CRC(8e7fbbb3) SHA1(3d57ddf6a47d5f28e4fd24002e948287803a2438) )
    ROM_CONTINUE(             0xb000, 0x1000)
    ROM_LOAD( "cpu-01-5.2a",  0x4000, 0x1000, CRC(2f3aa2a4) SHA1(4256894f178980abf187bb5424f9d738fcae2623) )
    ROM_CONTINUE(             0xc000, 0x1000)
    ROM_LOAD( "cpu-01-6.2c",  0x5000, 0x1000, CRC(c38228c0) SHA1(1ccc720b0d64b16c268a2bcfb8990f3c71b65913) )
    ROM_CONTINUE(             0xd000, 0x1000)
*/

	ROM_REGION( 0x10000, "audiopcb:melodycpu", 0 ) // 64k for first 6802
	ROM_LOAD( "13snd.2g",     0x8000, 0x2000, CRC(fc05654e) SHA1(ed9c66672fe89c41e320e1d27b53f5efa92dce9c) )
	ROM_LOAD( "9snd.1i",      0xc000, 0x2000, CRC(3dab977f) SHA1(3e79c06d2e70b050f01b7ac58be5127ba87904b0) )

	ROM_REGION( 0x10000, "audiopcb:audiocpu", 0 ) // 64k for second 6802
	ROM_LOAD( "8snd.1h",      0x2000, 0x1000, CRC(f4507111) SHA1(0513f0831b94aeda84aa4f3b4a7c60dfc5113b2d) )
	ROM_CONTINUE(             0x6000, 0x1000 )
	ROM_LOAD( "7snd.1g",      0x3000, 0x1000, CRC(c722eff8) SHA1(d8d1c091ab80ea2d6616e4dc030adc9905c0a496) )
	ROM_CONTINUE(             0x7000, 0x1000 )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "1bg.2d",       0x0000, 0x2000, CRC(9f880ef5) SHA1(0ee20fb7c794f6dafdaf2c9ee8456221c9d668c5) )
	ROM_LOAD( "2bg.1f",       0x2000, 0x2000, CRC(afc04cd7) SHA1(f4349e86b9caee71c9bf9faf68b86603417d9a2b) )
	ROM_LOAD( "3bg.1e",       0x4000, 0x2000, CRC(14f23cdd) SHA1(e5f3dac52288c56f2fd2940b397bb6c896131a26) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "jr-ic9g",      0x0000, 0x0200, CRC(85577107) SHA1(76575fa68b66130b18dfe7374d1a03740963cc73) )
	ROM_LOAD( "jr-ic9f",      0x0200, 0x0200, CRC(085914d1) SHA1(3d6f9318f5a9f08ce89e4184e3efb9881f671fa7) )

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "jr-pal16l8.6j",   0x0000, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "jr-pal16l8.6k",   0x0200, 0x0104, NO_DUMP ) // PAL is read protected
ROM_END

ROM_START( jackrabt2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1cpu2.1a",     0x0000, 0x1000, CRC(f9374113) SHA1(521f293f1894bcaf21e44bc7841a20ae29232da3) )
	ROM_CONTINUE(             0x8000, 0x1000 )
	ROM_LOAD( "2cpu2.1b",     0x1000, 0x1000, CRC(0a0eea4a) SHA1(4dfd9b2511d480bb5cc918f7d91013205911d377) )
	ROM_CONTINUE(             0x9000, 0x1000 )
	ROM_LOAD( "3cpu2.1c",     0x2000, 0x1000, CRC(291f5772) SHA1(958c2601d43de3c95ed5e3d79737199703263a6a) )
	ROM_CONTINUE(             0xa000, 0x1000 )
	ROM_LOAD( "4cpu2.1d",     0x3000, 0x1000, CRC(10972cfb) SHA1(30dd473b3416ee37f887d930ba0017b5b694398e) )
	ROM_CONTINUE(             0xb000, 0x1000 )
	ROM_LOAD( "5cpu2.2a",     0x4000, 0x1000, CRC(aa95d06d) SHA1(2216effe6cacd02a5320e71a85842087dda5f85a) )
	ROM_CONTINUE(             0xc000, 0x1000 )
	ROM_LOAD( "6cpu2.2c",     0x5000, 0x1000, CRC(404496eb) SHA1(44381e27e540fe9d8cacab4c3b1fe9a4f20d26a8) )
	ROM_CONTINUE(             0xd000, 0x1000 )

	ROM_REGION( 0x10000, "audiopcb:melodycpu", 0 ) // 64k for first 6802
	ROM_LOAD( "13snd.2g",     0x8000, 0x2000, CRC(fc05654e) SHA1(ed9c66672fe89c41e320e1d27b53f5efa92dce9c) )
	ROM_LOAD( "9snd.1i",      0xc000, 0x2000, CRC(3dab977f) SHA1(3e79c06d2e70b050f01b7ac58be5127ba87904b0) )

	ROM_REGION( 0x10000, "audiopcb:audiocpu", 0 ) // 64k for second 6802
	ROM_LOAD( "8snd.1h",      0x2000, 0x1000, CRC(f4507111) SHA1(0513f0831b94aeda84aa4f3b4a7c60dfc5113b2d) )
	ROM_CONTINUE(             0x6000, 0x1000 )
	ROM_LOAD( "7snd.1g",      0x3000, 0x1000, CRC(c722eff8) SHA1(d8d1c091ab80ea2d6616e4dc030adc9905c0a496) )
	ROM_CONTINUE(             0x7000, 0x1000 )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "1bg.2d",       0x0000, 0x2000, CRC(9f880ef5) SHA1(0ee20fb7c794f6dafdaf2c9ee8456221c9d668c5) )
	ROM_LOAD( "2bg.1f",       0x2000, 0x2000, CRC(afc04cd7) SHA1(f4349e86b9caee71c9bf9faf68b86603417d9a2b) )
	ROM_LOAD( "3bg.1e",       0x4000, 0x2000, CRC(14f23cdd) SHA1(e5f3dac52288c56f2fd2940b397bb6c896131a26) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "jr-ic9g",      0x0000, 0x0200, CRC(85577107) SHA1(76575fa68b66130b18dfe7374d1a03740963cc73) )
	ROM_LOAD( "jr-ic9f",      0x0200, 0x0200, CRC(085914d1) SHA1(3d6f9318f5a9f08ce89e4184e3efb9881f671fa7) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16l8.6j",   0x0000, 0x0104, CRC(a88e52d6) SHA1(32efecb91843d5d1bdace86cbcc94ebacf1b9389) )
	ROM_LOAD( "pal16l8.6k",   0x0200, 0x0104, NO_DUMP )
	ROM_LOAD( "82s100.8c",    0x0400, 0x00f5, CRC(70ddfa6d) SHA1(904347cc63e88413c393f14b5f1260a57ab72677) )
	ROM_LOAD( "82s100.8n",    0x0500, 0x00f5, CRC(e00625ee) SHA1(88bbd020be67355dc0eb58b79f7deb77cbe505bb) )
ROM_END

ROM_START( jackrabts )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1cpu.1a",      0x0000, 0x1000, CRC(6698dc65) SHA1(33e3518846e88dc34f4b6c4e9ca9f8999c0460c8) )
	ROM_CONTINUE(             0x8000, 0x1000 )
	ROM_LOAD( "2cpu.1b",      0x1000, 0x1000, CRC(42b32929) SHA1(5b400d434ce903c74f58780a422a8c2594af90be) )
	ROM_CONTINUE(             0x9000, 0x1000 )
	ROM_LOAD( "3cpu.1c",      0x2000, 0x1000, CRC(89b50c9a) SHA1(5ab56247de013b5196c1c5765ead4361a5df53e0) )
	ROM_CONTINUE(             0xa000, 0x1000 )
	ROM_LOAD( "4cpu.1d",      0x3000, 0x1000, CRC(d5520665) SHA1(69b34d87d50e6d6e8d365ba0479405380ba3cf11) )
	ROM_CONTINUE(             0xb000, 0x1000 )
	ROM_LOAD( "5cpu.2a",      0x4000, 0x1000, CRC(0f9a093c) SHA1(7fba0d2b8d5d4d1597decec96ed93b997c721d99) )
	ROM_CONTINUE(             0xc000, 0x1000 )
	ROM_LOAD( "6cpu.2c",      0x5000, 0x1000, CRC(f53d6356) SHA1(9b167edca59cf81a2468368a372bab132f15e2ea) )
	ROM_CONTINUE(             0xd000, 0x1000 )

	ROM_REGION( 0x10000, "audiopcb:melodycpu", 0 ) // 64k for first 6802
	ROM_LOAD( "13snd.2g",     0x8000, 0x2000, CRC(fc05654e) SHA1(ed9c66672fe89c41e320e1d27b53f5efa92dce9c) )
	ROM_LOAD( "9snd.1i",      0xc000, 0x2000, CRC(3dab977f) SHA1(3e79c06d2e70b050f01b7ac58be5127ba87904b0) )

	ROM_REGION( 0x10000, "audiopcb:audiocpu", 0 ) // 64k for second 6802
	ROM_LOAD( "8snd.1h",      0x2000, 0x1000, CRC(f4507111) SHA1(0513f0831b94aeda84aa4f3b4a7c60dfc5113b2d) )
	ROM_CONTINUE(             0x6000, 0x1000 )
	ROM_LOAD( "7snd.1g",      0x3000, 0x1000, CRC(c722eff8) SHA1(d8d1c091ab80ea2d6616e4dc030adc9905c0a496) )
	ROM_CONTINUE(             0x7000, 0x1000 )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "1bg.2d",       0x0000, 0x2000, CRC(9f880ef5) SHA1(0ee20fb7c794f6dafdaf2c9ee8456221c9d668c5) )
	ROM_LOAD( "2bg.1f",       0x2000, 0x2000, CRC(afc04cd7) SHA1(f4349e86b9caee71c9bf9faf68b86603417d9a2b) )
	ROM_LOAD( "3bg.1e",       0x4000, 0x2000, CRC(14f23cdd) SHA1(e5f3dac52288c56f2fd2940b397bb6c896131a26) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "jr-ic9g",      0x0000, 0x0200, CRC(85577107) SHA1(76575fa68b66130b18dfe7374d1a03740963cc73) )
	ROM_LOAD( "jr-ic9f",      0x0200, 0x0200, CRC(085914d1) SHA1(3d6f9318f5a9f08ce89e4184e3efb9881f671fa7) )
ROM_END

} // anonymous namespace



/***************************************************************************

  Game drivers

***************************************************************************/

GAME( 1983, monymony,  0,        zaccaria, monymony, zaccaria_state, empty_init, ROT90, "Zaccaria", "Money Money (set 1)",   MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1983, monymony2, monymony, zaccaria, monymony, zaccaria_state, empty_init, ROT90, "Zaccaria", "Money Money (set 2)",   MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, jackrabt,  0,        zaccaria, jackrabt, zaccaria_state, empty_init, ROT90, "Zaccaria", "Jack Rabbit (set 1)",   MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, jackrabt2, jackrabt, zaccaria, jackrabt, zaccaria_state, empty_init, ROT90, "Zaccaria", "Jack Rabbit (set 2)",   MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, jackrabts, jackrabt, zaccaria, jackrabt, zaccaria_state, empty_init, ROT90, "Zaccaria", "Jack Rabbit (special)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

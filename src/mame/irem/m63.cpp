// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Irem M63 hardware

****************************************************************************

Wily Tower              (c) 1984 Irem
Fighting Basketball     (c) 1984 Paradise Co. Ltd.

driver by Nicola Salmoria


PCB Layout (based on Atomic Boy PCB)
------------------------------------

Both boards has etched NANAO logo.
They (Eizo Nanao Corporation) own Irem Software Engineering.


M63-A-A (TOP)

       1         2       3       4       5       6       7
|---------------------------------------------------------------|
|              DSW8    74373                                    |   A
|                                                               |
|-|   74368    DSW8    74373    AY-3-8910     MSM80C39R6       |-|  B
  |                                                            | |
  |   74368            74373    AY-3-8910   74373              | |  C
  |                                                            | |
  |   74368    7408    74373   wt_a-4d         wt_a-6d        C| |  D
  |                                                           N| |
  |   74368    7400    7404    wt_a-4e 7404    7474    74???  1| |  E
  |2                                                           | |
  |2                   74138           74367   7474    74367   | |  F
  |W                                                           | |
  |A           7432    7427    wt_a-4h D780C                   | |  H
  |Y                                                           |-|
  |   74368    74299   wt_a-3j wt_a-4j 74367   74245            |   J
  |                                                            |-|
  |            74367   wt_a-3k wt_a-4k 74138   7432            | |  K
  |                                                            | |
  |            74299                   74139   7432            | |  L
  |                                                            | |
|-|   74368    74299   wt_a-3m wt_a-4m 74139   M53202  7474   C| |  M
|                                                             N| |
|     7400     74299   wt_a-3n wt_a-4n 74273   74283   7420   2| |  N
|                                                              | |
|              74157   wt_a-3p M58725 WT_A-5P  74283   74273   | |  P
|                                                              | |
|              74299                  WT_A-5R  7432    74157   |-|  R
|     AMP                                                       |
|              74299   wt_a-3s M58725 WT_A-5S  74273   74157    |   S
|---------------------------------------------------------------|

M63-B-A (BOTTOM)

      1        2       3       4       5       6       7       8       9
|------------------------------------------------------------------------------|
|   74244     74244      74244    wt_b-5a     2128   74244   74244   74245     |   A
|                                                                              -
|   74373     74273      74299    wt_b-5b     2128   74161   74161   74161    | |  B
|                                                                             | |
|   74244     74273      74299                       74157   74157   74157    | |  C
|                                                                             | |
|   74273     74244      74299    wt_b-5d     2128   74157   74157   74157   C| |  D
|                                                                            N| |
|   2148      74283      74299    wt_b-5e     2128   74157   74157   74157   1| |  E
|                                                                             | |
|   2148      74283      74299    wt_b-5f     2128   74157   74157   74157    | |  F
|                                                                             | |
|   74157     7420       74157                2128   2148    7486    7486     |-|  H
|                                                                              |
|   74157     7430   74367   7474    74273           2148    74367   74367    |-|  J
|                                                                             | |
|   74161     74139  7420    74161   7486    74139   7474    74161   74161    | |  K
|                                                                             | |
|   74157     7432   7474    74161   7486    7404    7474    74175   wt_b-9l  | |  L
|                                                                            C| |
|   74161     7432   74368   74???   74175   7408    7400    7432            N| |  M
|                                                                            2| |
|             7400   7404    7420    74175   7474    74273   74377   74175    | |  N
|                                                                             | |
|                    7414    M53202  74163   7486    2148    74241   7427     | |  P
|                                                                             |-|
|             12MHz  7404            74163   7486    2148    74241   74373     |   R
|------------------------------------------------------------------------------|




Notes:
- Unless there is some special logic related to NMI enable, the game doesn't
  rely on vblank for timing. It all seems to be controlled by the CPU clock.
  The NMI handler just handles the "Stop Mode" dip switch.

TS 2008.06.14:
- Added sound emulation - atomboy and fghtbskt req different interrupt (T1)
  timing than wilytowr, otherwise music/fx tempo is too fast.
  Music tempo and pitch verified on real pcb.
- Extra space in atomboy 2764 eproms is filled with garbage z80 code
  (taken from one of code roms, but from different offset)
- Fghtbskt has one AY, but every frame writes 0 to 2nd AY regs - probably
  leftover from Wily Tower sound driver/code
- I'm not sure about sound_status write - maybe it's something else or
  different data (p1?) is used as status

TODO:
- Sprite positioning is wacky. The electric 'bands' that go along the pipes
  are drawn 2 pixels off in x/y directions. If you fix that, then the player
  sprite doesn't slide in the middle of the pipes when climbing...
- Clocks

Dip locations verified for:
- atomboy (manual)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"
#include "sound/samples.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

namespace {

class m63_state : public driver_device
{
public:
	m63_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_scrollram(*this, "scrollram"),
		m_videoram2(*this, "videoram2"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_ay1(*this, "ay1"),
		m_ay2(*this, "ay2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundcpu(*this, "soundcpu"),
		m_samples(*this, "samples"),
		m_soundlatch(*this, "soundlatch")
	{
	}

	void atomboy(machine_config &config);
	void m63(machine_config &config);
	void fghtbskt(machine_config &config);

	void init_wilytowr();
	void init_fghtbskt();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_scrollram;
	required_shared_ptr<uint8_t> m_videoram2;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;

	uint8_t    m_nmi_mask = 0;

	/* video-related */
	tilemap_t  *m_bg_tilemap = nullptr;
	tilemap_t  *m_fg_tilemap = nullptr;
	int      m_pal_bank = 0;
	int      m_fg_flag = 0;
	int      m_sy_offset = 0;

	/* sound-related */
	uint8_t    m_sound_irq = 0;
	int      m_sound_status = 0;
	int      m_p1 = 0;
	int      m_p2 = 0;
	std::unique_ptr<int16_t[]>    m_samplebuf;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<ay8910_device> m_ay1;
	optional_device<ay8910_device> m_ay2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<i8039_device> m_soundcpu;
	optional_device<samples_device> m_samples;
	required_device<generic_latch_8_device> m_soundlatch;

	void m63_videoram_w(offs_t offset, uint8_t data);
	void m63_colorram_w(offs_t offset, uint8_t data);
	void m63_videoram2_w(offs_t offset, uint8_t data);
	void pal_bank_w(int state);
	void fghtbskt_flipscreen_w(int state);
	void coin1_w(int state);
	void coin2_w(int state);
	void snd_irq_w(uint8_t data);
	void snddata_w(offs_t offset, uint8_t data);
	void p1_w(uint8_t data);
	void p2_w(uint8_t data);
	uint8_t snd_status_r();
	int irq_r();
	uint8_t snddata_r(offs_t offset);
	void fghtbskt_samples_w(uint8_t data);
	SAMPLES_START_CB_MEMBER(fghtbskt_sh_start);
	void nmi_mask_w(int state);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	void m63_palette(palette_device &palette) const;
	uint32_t screen_update_m63(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(snd_irq);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void fghtbskt_map(address_map &map) ATTR_COLD;
	void i8039_map(address_map &map) ATTR_COLD;
	void i8039_port_map(address_map &map) ATTR_COLD;
	void m63_map(address_map &map) ATTR_COLD;
};


void m63_state::m63_palette(palette_device &palette) const
{
	uint8_t const *color_prom = memregion("proms")->base();

	for (int i = 0; i < 256; i++)
	{
		int bit0, bit1, bit2, bit3;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		bit3 = BIT(color_prom[i], 3);
		int const r =  0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		// green component
		bit0 = BIT(color_prom[i + 256], 0);
		bit1 = BIT(color_prom[i + 256], 1);
		bit2 = BIT(color_prom[i + 256], 2);
		bit3 = BIT(color_prom[i + 256], 3);
		int const g =  0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		// blue component
		bit0 = BIT(color_prom[i + 2*256], 0);
		bit1 = BIT(color_prom[i + 2*256], 1);
		bit2 = BIT(color_prom[i + 2*256], 2);
		bit3 = BIT(color_prom[i + 2*256], 3);
		int const b =  0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}

	color_prom += 3 * 256;

	for (int i = 0; i < 4; i++)
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
		int const b = 0x4f * bit0 + 0xa8 * bit1;

		palette.set_pen_color(i + 256, rgb_t(r, g, b));
	}
}

void m63_state::m63_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void m63_state::m63_colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void m63_state::m63_videoram2_w(offs_t offset, uint8_t data)
{
	m_videoram2[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void m63_state::pal_bank_w(int state)
{
	m_pal_bank = state;
	m_bg_tilemap->mark_all_dirty();
}

void m63_state::fghtbskt_flipscreen_w(int state)
{
	flip_screen_set(state);
	m_fg_flag = flip_screen() ? TILE_FLIPX : 0;
}


TILE_GET_INFO_MEMBER(m63_state::get_bg_tile_info)
{
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index] | ((attr & 0x30) << 4);
	int color = (attr & 0x0f) + (m_pal_bank << 4);

	tileinfo.set(1, code, color, 0);
}

TILE_GET_INFO_MEMBER(m63_state::get_fg_tile_info)
{
	int code = m_videoram2[tile_index];

	tileinfo.set(0, code, 0, m_fg_flag);
}

void m63_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(m63_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(m63_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_bg_tilemap->set_scroll_cols(32);
	m_fg_tilemap->set_transparent_pen(0);
}

void m63_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;

	for (offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		int code = m_spriteram[offs + 1] | ((m_spriteram[offs + 2] & 0x10) << 4);
		int color = (m_spriteram[offs + 2] & 0x0f) + (m_pal_bank << 4);
		int flipx = m_spriteram[offs + 2] & 0x20;
		int flipy = 0;
		int sx = m_spriteram[offs + 3];
		int sy = m_sy_offset - m_spriteram[offs];

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = m_sy_offset - sy;
			flipx = !flipx;
			flipy = !flipy;
		}


			m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
			code, color,
			flipx, flipy,
			sx, sy, 0);

		/* sprite wrapping - verified on real hardware*/
		if (sx > 0xf0)
		{
			m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
			code, color,
			flipx, flipy,
			sx - 0x100, sy, 0);
		}

	}
}

uint32_t m63_state::screen_update_m63(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int col;

	for (col = 0; col < 32; col++)
		m_bg_tilemap->set_scrolly(col, m_scrollram[col * 8]);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


void m63_state::coin1_w(int state)
{
	machine().bookkeeping().coin_counter_w(0, state);
}

void m63_state::coin2_w(int state)
{
	machine().bookkeeping().coin_counter_w(1, state);
}

void m63_state::snd_irq_w(uint8_t data)
{
	m_soundcpu->set_input_line(0, ASSERT_LINE);
	machine().scheduler().synchronize();
}

void m63_state::snddata_w(offs_t offset, uint8_t data)
{
	if ((m_p2 & 0xf0) == 0xe0)
		m_ay1->address_w(offset);
	else if ((m_p2 & 0xf0) == 0xa0)
		m_ay1->data_w(offset);
	else if (m_ay2 != nullptr && (m_p1 & 0xe0) == 0x60)
		m_ay2->address_w(offset);
	else if (m_ay2 != nullptr && (m_p1 & 0xe0) == 0x40)
			m_ay2->data_w(offset);
	else if ((m_p2 & 0xf0) == 0x70 )
		m_sound_status = offset;
}

void m63_state::p1_w(uint8_t data)
{
	m_p1 = data;
}

void m63_state::p2_w(uint8_t data)
{
	m_p2 = data;
	if((m_p2 & 0xf0) == 0x50)
	{
		m_soundcpu->set_input_line(0, CLEAR_LINE);
	}
}

uint8_t m63_state::snd_status_r()
{
	return m_sound_status;
}

int m63_state::irq_r()
{
	if (m_sound_irq)
	{
		m_sound_irq = 0;
		return 1;
	}
	return 0;
}

uint8_t m63_state::snddata_r(offs_t offset)
{
	switch (m_p2 & 0xf0)
	{
		case 0x60:  return m_soundlatch->read();
		case 0x70:  return memregion("user1")->base()[((m_p1 & 0x1f) << 8) | offset];
	}
	return 0xff;
}

void m63_state::fghtbskt_samples_w(uint8_t data)
{
	if (data & 1)
		m_samples->start_raw(0, m_samplebuf.get() + ((data & 0xf0) << 8), 0x2000, 8000);
}

void m63_state::nmi_mask_w(int state)
{
	m_nmi_mask = state;
}


void m63_state::m63_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xd000, 0xdfff).ram();
	map(0xe000, 0xe1ff).ram();
	map(0xe200, 0xe2ff).ram().share("spriteram");
	map(0xe300, 0xe3ff).ram().share("scrollram");
	map(0xe400, 0xe7ff).ram().w(FUNC(m63_state::m63_videoram2_w)).share("videoram2");
	map(0xe800, 0xebff).ram().w(FUNC(m63_state::m63_videoram_w)).share("videoram");
	map(0xec00, 0xefff).ram().w(FUNC(m63_state::m63_colorram_w)).share("colorram");
	map(0xf000, 0xf007).w("outlatch", FUNC(ls259_device::write_d0));
	map(0xf800, 0xf800).portr("P1").w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xf801, 0xf801).portr("P2").nopw(); /* continues game when in stop mode (cleared by NMI handler) */
	map(0xf802, 0xf802).portr("DSW1");
	map(0xf803, 0xf803).w(FUNC(m63_state::snd_irq_w));
	map(0xf806, 0xf806).portr("DSW2");
}

void m63_state::fghtbskt_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x8000, 0xbfff).rom();
	map(0xc000, 0xc7ff).ram();
	map(0xd000, 0xd1ff).ram();
	map(0xd200, 0xd2ff).ram().share("spriteram");
	map(0xd300, 0xd3ff).ram().share("scrollram");
	map(0xd400, 0xd7ff).ram().w(FUNC(m63_state::m63_videoram2_w)).share("videoram2");
	map(0xd800, 0xdbff).ram().w(FUNC(m63_state::m63_videoram_w)).share("videoram");
	map(0xdc00, 0xdfff).ram().w(FUNC(m63_state::m63_colorram_w)).share("colorram");
	map(0xf000, 0xf000).r(FUNC(m63_state::snd_status_r));
	map(0xf001, 0xf001).portr("P1");
	map(0xf002, 0xf002).portr("P2");
	map(0xf003, 0xf003).portr("DSW");
	map(0xf000, 0xf000).w(FUNC(m63_state::snd_irq_w));
	map(0xf001, 0xf001).nopw();
	map(0xf002, 0xf002).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xf800, 0xf807).w("outlatch", FUNC(ls259_device::write_d0));
	map(0xf807, 0xf807).w(FUNC(m63_state::fghtbskt_samples_w)); // FIXME
}

void m63_state::i8039_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
}


void m63_state::i8039_port_map(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(m63_state::snddata_r), FUNC(m63_state::snddata_w));
}



static INPUT_PORTS_START( wilytowr )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x0c, 0x00, "Bonus Points Rate" )     PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, "x1.2" )
	PORT_DIPSETTING(    0x08, "x1.4" )
	PORT_DIPSETTING(    0x0c, "x1.6" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:!5,!6") PORT_CONDITION("DSW1",0x04,EQUALS,0x04) /* coin mode 2 */
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Free_Play ) )    /* Not documented */
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:!7,!8") PORT_CONDITION("DSW1",0x04,EQUALS,0x04) /* coin mode 2 */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!5,!6,!7,!8") PORT_CONDITION("DSW1",0x04,EQUALS,0x00) /* coin mode 1 */
	PORT_DIPSETTING(    0x60, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_8C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_9C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( Free_Play ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:!1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:!2")
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	/*  "For cabinets with a single coin selector or 2 coin selectors of the same value, set to Mode 1.
	    For cabinets with coin selectors of two different values, set to Mode 2." */
	PORT_DIPNAME( 0x04, 0x00, "Coin Mode" )             PORT_DIPLOCATION("SW2:!3")
	PORT_DIPSETTING(    0x00, "Mode 1" )
	PORT_DIPSETTING(    0x04, "Mode 2" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x00, "SW2:!4" )       /* Listed as "Unused" */
	/* In stop mode, press 1 to stop and 2 to restart */
	PORT_DIPNAME( 0x10, 0x00, "Stop Mode (Cheat)" )     PORT_DIPLOCATION("SW2:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "SW2:!6" )       /* Listed as "Unused" */
	PORT_DIPNAME( 0x40, 0x00, "Invulnerability (Cheat)" ) PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(0x80, IP_ACTIVE_HIGH, "SW2:!8" )
INPUT_PORTS_END

static INPUT_PORTS_START( fghtbskt )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0c, "99 Credits / Sound Test" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Time Count Down" )
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x20, "Too Fast" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,6),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			RGN_FRAC(1,6)+0, RGN_FRAC(1,6)+1, RGN_FRAC(1,6)+2, RGN_FRAC(1,6)+3,
			RGN_FRAC(1,6)+4, RGN_FRAC(1,6)+5, RGN_FRAC(1,6)+6, RGN_FRAC(1,6)+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*8
};

static GFXDECODE_START( gfx_m63 )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x2_planar, 256, 1  )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_8x8x3_planar,   0, 32 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout,       0, 32 )
GFXDECODE_END

static GFXDECODE_START( gfx_fghtbskt )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x2_planar,  16, 1  )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_8x8x3_planar,   0, 32 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout,       0, 32 )
GFXDECODE_END


SAMPLES_START_CB_MEMBER(m63_state::fghtbskt_sh_start)
{
	int i, len = memregion("samples")->bytes();
	uint8_t *ROM = memregion("samples")->base();

	m_samplebuf = std::make_unique<int16_t[]>(len);
	save_pointer(NAME(m_samplebuf), len);

	for(i = 0; i < len; i++)
		m_samplebuf[i] = ((int8_t)(ROM[i] ^ 0x80)) * 256;
}

INTERRUPT_GEN_MEMBER(m63_state::snd_irq)
{
	m_sound_irq = 1;
}

void m63_state::machine_start()
{
	save_item(NAME(m_pal_bank));
	save_item(NAME(m_fg_flag));
	save_item(NAME(m_sy_offset));

	/* sound-related */
	save_item(NAME(m_sound_irq));
	save_item(NAME(m_sound_status));
	save_item(NAME(m_p1));
	save_item(NAME(m_p2));
}

void m63_state::machine_reset()
{
	m_pal_bank = 0;
	m_fg_flag = 0;
	m_sound_irq = 0;
	m_sound_status = 0;
	m_p1 = 0;
	m_p2 = 0;
}


INTERRUPT_GEN_MEMBER(m63_state::vblank_irq)
{
	if(m_nmi_mask)
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void m63_state::m63(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(12'000'000)/4); /* 3 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &m63_state::m63_map);
	m_maincpu->set_vblank_int("screen", FUNC(m63_state::vblank_irq));

	ls259_device &outlatch(LS259(config, "outlatch")); // probably chip at E7 obscured by pulldown resistor
	outlatch.q_out_cb<0>().set(FUNC(m63_state::nmi_mask_w));
	outlatch.q_out_cb<2>().set(FUNC(m63_state::flip_screen_set)).invert();
	outlatch.q_out_cb<3>().set(FUNC(m63_state::pal_bank_w));
	outlatch.q_out_cb<6>().set(FUNC(m63_state::coin1_w));
	outlatch.q_out_cb<7>().set(FUNC(m63_state::coin2_w));

	I8039(config, m_soundcpu, XTAL(12'000'000)/4); /* ????? */
	m_soundcpu->set_addrmap(AS_PROGRAM, &m63_state::i8039_map);
	m_soundcpu->set_addrmap(AS_IO, &m63_state::i8039_port_map);
	m_soundcpu->p1_out_cb().set(FUNC(m63_state::p1_w));
	m_soundcpu->p2_out_cb().set(FUNC(m63_state::p2_w));
	m_soundcpu->t1_in_cb().set(FUNC(m63_state::irq_r));
	m_soundcpu->set_periodic_int(FUNC(m63_state::snd_irq), attotime::from_hz(60));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(m63_state::screen_update_m63));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_m63);
	PALETTE(config, m_palette, FUNC(m63_state::m63_palette), 256+4);

	/* sound hardware */
	SPEAKER(config, "mono").front_center(); /* ????? */

	GENERIC_LATCH_8(config, m_soundlatch);

	AY8910(config, m_ay1, XTAL(12'000'000)/8).add_route(ALL_OUTPUTS, "mono", 0.25);
	AY8910(config, m_ay2, XTAL(12'000'000)/8).add_route(ALL_OUTPUTS, "mono", 1.00); /* ????? */
}

void m63_state::atomboy(machine_config &config)
{
	m63(config);
	m_soundcpu->set_periodic_int(FUNC(m63_state::snd_irq), attotime::from_hz(60/2));
}

void m63_state::fghtbskt(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(12'000'000)/4); /* 3 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &m63_state::fghtbskt_map);
	m_maincpu->set_vblank_int("screen", FUNC(m63_state::vblank_irq));

	ls259_device &outlatch(LS259(config, "outlatch"));
	outlatch.q_out_cb<1>().set(FUNC(m63_state::nmi_mask_w));
	outlatch.q_out_cb<2>().set(FUNC(m63_state::fghtbskt_flipscreen_w));
	//outlatch.q_out_cb<7>().set(FUNC(m63_state::fghtbskt_samples_w));

	I8039(config, m_soundcpu, XTAL(12'000'000)/4); /* ????? */
	m_soundcpu->set_addrmap(AS_PROGRAM, &m63_state::i8039_map);
	m_soundcpu->set_addrmap(AS_IO, &m63_state::i8039_port_map);
	m_soundcpu->p1_out_cb().set(FUNC(m63_state::p1_w));
	m_soundcpu->p2_out_cb().set(FUNC(m63_state::p2_w));
	m_soundcpu->t1_in_cb().set(FUNC(m63_state::irq_r));
	m_soundcpu->set_periodic_int(FUNC(m63_state::snd_irq), attotime::from_hz(60/2));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(m63_state::screen_update_m63));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_fghtbskt);
	PALETTE(config, m_palette, palette_device::RGB_444_PROMS, "proms", 256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	AY8910(config, m_ay1, XTAL(12'000'000)/8).add_route(ALL_OUTPUTS, "mono", 1.0);

	SAMPLES(config, m_samples);
	m_samples->set_channels(1);
	m_samples->set_samples_start_callback(FUNC(m63_state::fghtbskt_sh_start));
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.50);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( wilytowr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wt4e.bin",     0x0000, 0x2000, CRC(a38e4b8a) SHA1(e296ba1764d3e8e2a5cc43bdde7f30a522b437ff) )
	ROM_LOAD( "wt4h.bin",     0x2000, 0x2000, CRC(c1405ceb) SHA1(c11dd4cd180bc9576e8042e1f56074620ea00f53) )
	ROM_LOAD( "wt4j.bin",     0x4000, 0x2000, CRC(379fb1c3) SHA1(677e4077f6d2140e4fb5c3d86bc7081d3b6cc028) )
	ROM_LOAD( "wt4k.bin",     0x6000, 0x2000, CRC(2dd6f9c7) SHA1(88ba58a1ddd25403211b7f920ba7006ed80c13eb) )
	ROM_LOAD( "wt_a-4m.bin",  0x8000, 0x2000, CRC(c1f8a7d5) SHA1(4307e7604aec728a1f5b0e6a0d6c9f4d37084da3) )
	ROM_LOAD( "wt_a-4n.bin",  0xa000, 0x2000, CRC(b212f7d2) SHA1(dd1c35559982e8bbcb0e778c733a3afb5b6611df) )

	ROM_REGION( 0x1000, "soundcpu", 0 ) /* 8039 */
	ROM_LOAD( "wt4d.bin",     0x0000, 0x1000, CRC(25a171bf) SHA1(7465dbfa8858d0f5822eb748b96d99753d58d243) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	/* '3' character is bad, but ROMs have been verified on four boards */
	ROM_LOAD( "wt_b-5e.bin",  0x0000, 0x1000, CRC(fe45df43) SHA1(9586a5728069e0c293bd17d4663305ce5758ca01) )
	ROM_LOAD( "wt_b-5f.bin",  0x1000, 0x1000, CRC(87a17eff) SHA1(cee2ba2889baf08dc6ee1c8e9150bd277f343be9) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "wtb5a.bin",    0x0000, 0x2000, CRC(efc1cbfa) SHA1(9a2ea29e64360ef7b143ac1b6a1ba3e672be4a42) )
	ROM_LOAD( "wtb5b.bin",    0x2000, 0x2000, CRC(ab4bfd07) SHA1(1d5010413989895c09d8e5ee903d665506836f94) )
	ROM_LOAD( "wtb5d.bin",    0x4000, 0x2000, CRC(40f23e1d) SHA1(abff583021e2cf2d2ec83adbbd4f2e96bfa3e04f) )

	ROM_REGION( 0x6000, "gfx3", 0 )
	/* there are horizontal lines in some tiles, but ROMs have been verified on four boards */
	ROM_LOAD( "wt2j.bin",     0x0000, 0x1000, CRC(d1bf0670) SHA1(8d07bce354bb4538948c358fd696304a8e0640b8) )
	ROM_LOAD( "wt3k.bin",     0x1000, 0x1000, CRC(83c39a0e) SHA1(da98f887ac5c3d52281eece3d760c41fb9ecfd5c) )
	ROM_LOAD( "wt_a-3m.bin",  0x2000, 0x1000, CRC(e7e468ae) SHA1(17448191b440b668714d83730075938aaaf34b5a) )
	ROM_LOAD( "wt_a-3n.bin",  0x3000, 0x1000, CRC(0741d1a9) SHA1(51f5ee03db8a3f7afbf944b9e3e4ae12b2520269) )
	ROM_LOAD( "wt_a-3p.bin",  0x4000, 0x1000, CRC(7299f362) SHA1(5ba309d789df8432c08d67e4f9e8bf6c447fc425) )
	ROM_LOAD( "wt_a-3s.bin",  0x5000, 0x1000, CRC(9b37d50d) SHA1(a08d4a7654b815cb652be66dbaa097011327f5d5) )

	ROM_REGION( 0x2000, "user1", 0 )
	ROM_LOAD( "wt_a-6d.bin",  0x0000, 0x1000, CRC(a5dde29b) SHA1(8f7545d2022da7c98d47112179dce717f6c3c5e2) )


	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "wt_a-5s-.bpr", 0x0000, 0x0100, CRC(041950e7) SHA1(8276068bec3f4c5013c773033fca3cd3ed9e82ef) )    /* red */
	ROM_LOAD( "wt_a-5r-.bpr", 0x0100, 0x0100, CRC(bc04bf25) SHA1(37d0e89296760f51df5a0d434dca390fb60bb052) )    /* green */
	ROM_LOAD( "wt_a-5p-.bpr", 0x0200, 0x0100, CRC(ed819a19) SHA1(76f13dcf1674f136375738756e175ceec469d545) )    /* blue */
	ROM_LOAD( "wt_b-9l-.bpr", 0x0300, 0x0020, CRC(d2728744) SHA1(e6b1a570854ca90326414874432ab03ec85b9c8e) )    /* char palette */
ROM_END

ROM_START( atomboy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wt_a-4e.bin",  0x0000, 0x2000, CRC(f7978185) SHA1(6a108d1e9b1a81cedf865aba3998748dcf1d55ef) )
	ROM_LOAD( "wt_a-4h.bin",  0x2000, 0x2000, CRC(0ca9950b) SHA1(d6583fcdf17d16a8884932695caa9c5587a20795) )
	ROM_LOAD( "wt_a-4j.bin",  0x4000, 0x2000, CRC(1badbc65) SHA1(e0768f2cd7bbe8908fd68ff6d54dbef84cc7de4c) )
	ROM_LOAD( "wt_a-4k.bin",  0x6000, 0x2000, CRC(5a341f75) SHA1(9e1a180e37aaa0afbf8ff45219be40d3f75fe60a) )
	ROM_LOAD( "wt_a-4m.bin",  0x8000, 0x2000, CRC(c1f8a7d5) SHA1(4307e7604aec728a1f5b0e6a0d6c9f4d37084da3) )
	ROM_LOAD( "wt_a-4n.bin",  0xa000, 0x2000, CRC(b212f7d2) SHA1(dd1c35559982e8bbcb0e778c733a3afb5b6611df) )

	ROM_REGION( 0x2000, "soundcpu", 0 ) /* 8039 */
	ROM_LOAD( "wt_a-4d-b.bin",  0x0000, 0x2000, CRC(793ea53f) SHA1(9dbff5e011a1f1f48aad68f8e5b02bcdb86c182a) ) /* 2764 ROM, Also had a red dot on label */

	ROM_REGION( 0x2000, "gfx1", 0 )
	/* '3' character is bad, but ROMs have been verified on four boards */
	ROM_LOAD( "wt_b-5e.bin",  0x0000, 0x1000, CRC(fe45df43) SHA1(9586a5728069e0c293bd17d4663305ce5758ca01) )
	ROM_LOAD( "wt_b-5f.bin",  0x1000, 0x1000, CRC(87a17eff) SHA1(cee2ba2889baf08dc6ee1c8e9150bd277f343be9) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "wt_b-5a.bin",  0x0000, 0x2000, CRC(da22c452) SHA1(bd921baa12087e996d07625e05eda00981608655) )
	ROM_LOAD( "wt_b-5b.bin",  0x2000, 0x2000, CRC(4fb25a1f) SHA1(0f90fb3b373760c33ba9be3b56b917eca92c9700) )
	ROM_LOAD( "wt_b-5d.bin",  0x4000, 0x2000, CRC(75be2604) SHA1(fe1f110e188aa34a04a9f43412a8308240391fcf) )

	ROM_REGION( 0xc000, "gfx3", 0 )
	ROM_LOAD( "wt_a-3j-b.bin",  0x0000, 0x2000, CRC(996470f1) SHA1(c0c787a73535917d1314bb2e1e9056aea9859205) ) /* All these ROMs are 2764 type/size */
	ROM_LOAD( "wt_a-3k-b.bin",  0x2000, 0x2000, CRC(8f4ec45c) SHA1(525393e0555e1aa24df74e8095da216f02fe3c65) )
	ROM_LOAD( "wt_a-3m-b.bin",  0x4000, 0x2000, CRC(4ac40358) SHA1(c71bd62ef1e8d008abd468c193e67b278599a5f3) )
	ROM_LOAD( "wt_a-3n-b.bin",  0x6000, 0x2000, CRC(709eef5b) SHA1(95beadcf876a2549836329521f1293634413e983) )
	ROM_LOAD( "wt_a-3p-b.bin",  0x8000, 0x2000, CRC(3018b840) SHA1(77df9d4f1c8d76d30c435d03d51ef9e7509fab9c) )
	ROM_LOAD( "wt_a-3s-b.bin",  0xa000, 0x2000, CRC(05a251d4) SHA1(1cd9102871507ab988d5fe799024d63b93807448) )

	ROM_REGION( 0x2000, "user1", 0 )
	ROM_LOAD( "wt_a-6d.bin",  0x0000, 0x1000, CRC(a5dde29b) SHA1(8f7545d2022da7c98d47112179dce717f6c3c5e2) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "wt_a-5s-b.bpr", 0x0000, 0x0100, CRC(991e2a04) SHA1(a70525948ad85ad898e0d8a25fb6d1639a4ec133) )   /* red    TBP24S10 (read as 82s129) */
	ROM_LOAD( "wt_a-5r-b.bpr", 0x0100, 0x0100, CRC(fb3822b7) SHA1(bb1ecdd0156acc16bef3c9072e496e4f544b5d9d) )   /* green  TBP24S10 (read as 82s129) */
	ROM_LOAD( "wt_a-5p-b.bpr", 0x0200, 0x0100, CRC(95849f7d) SHA1(ad031d6035045b19c1cd65ac6a78c5aa4b647cd6) )   /* blue   TBP24S10 (read as 82s129) */
	ROM_LOAD( "wt_b-9l-.bpr",  0x0300, 0x0020, CRC(d2728744) SHA1(e6b1a570854ca90326414874432ab03ec85b9c8e) )   /* char palette */
ROM_END

ROM_START( atomboya )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wt_a-4e.bin",  0x0000, 0x2000, CRC(f7978185) SHA1(6a108d1e9b1a81cedf865aba3998748dcf1d55ef) )
	ROM_LOAD( "wt_a-4h.bin",  0x2000, 0x2000, CRC(0ca9950b) SHA1(d6583fcdf17d16a8884932695caa9c5587a20795) )
	ROM_LOAD( "wt_a-4j.bin",  0x4000, 0x2000, CRC(1badbc65) SHA1(e0768f2cd7bbe8908fd68ff6d54dbef84cc7de4c) )
	ROM_LOAD( "wt_a-4k.bin",  0x6000, 0x2000, CRC(5a341f75) SHA1(9e1a180e37aaa0afbf8ff45219be40d3f75fe60a) )
	ROM_LOAD( "wt_a-4m.bin",  0x8000, 0x2000, CRC(c1f8a7d5) SHA1(4307e7604aec728a1f5b0e6a0d6c9f4d37084da3) )
	ROM_LOAD( "wt_a-4n.bin",  0xa000, 0x2000, CRC(b212f7d2) SHA1(dd1c35559982e8bbcb0e778c733a3afb5b6611df) )

	ROM_REGION( 0x1000, "soundcpu", 0 ) /* 8039 */
	ROM_LOAD( "wt_a-4d.bin",  0x0000, 0x1000, CRC(3d43361e) SHA1(2977df9f90d9d214909c56ab44c40ab45fd90675) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	/* '3' character is bad, but ROMs have been verified on four boards */
	ROM_LOAD( "wt_b-5e.bin",  0x0000, 0x1000, CRC(fe45df43) SHA1(9586a5728069e0c293bd17d4663305ce5758ca01) )
	ROM_LOAD( "wt_b-5f.bin",  0x1000, 0x1000, CRC(87a17eff) SHA1(cee2ba2889baf08dc6ee1c8e9150bd277f343be9) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "wt_b-5a.bin",  0x0000, 0x2000, CRC(da22c452) SHA1(bd921baa12087e996d07625e05eda00981608655) )
	ROM_LOAD( "wt_b-5b.bin",  0x2000, 0x2000, CRC(4fb25a1f) SHA1(0f90fb3b373760c33ba9be3b56b917eca92c9700) )
	ROM_LOAD( "wt_b-5d.bin",  0x4000, 0x2000, CRC(75be2604) SHA1(fe1f110e188aa34a04a9f43412a8308240391fcf) )

	ROM_REGION( 0x6000, "gfx3", 0 )
	/* there are horizontal lines in some tiles, but ROMs have been verified on four boards */
	ROM_LOAD( "wt_a-3j.bin",  0x0000, 0x1000, CRC(b30ca38f) SHA1(885743893461b8617180a9723f6fcef160a2f05d) )
	ROM_LOAD( "wt_a-3k.bin",  0x1000, 0x1000, CRC(9a77eb73) SHA1(2564a3b3744b0be147b41c521fc7efde53bdfea7) )
	ROM_LOAD( "wt_a-3m.bin",  0x2000, 0x1000, CRC(e7e468ae) SHA1(17448191b440b668714d83730075938aaaf34b5a) )
	ROM_LOAD( "wt_a-3n.bin",  0x3000, 0x1000, CRC(0741d1a9) SHA1(51f5ee03db8a3f7afbf944b9e3e4ae12b2520269) )
	ROM_LOAD( "wt_a-3p.bin",  0x4000, 0x1000, CRC(7299f362) SHA1(5ba309d789df8432c08d67e4f9e8bf6c447fc425) )
	ROM_LOAD( "wt_a-3s.bin",  0x5000, 0x1000, CRC(9b37d50d) SHA1(a08d4a7654b815cb652be66dbaa097011327f5d5) )

	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD( "wt_a-6d.bin",  0x0000, 0x1000, CRC(a5dde29b) SHA1(8f7545d2022da7c98d47112179dce717f6c3c5e2) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "wt_a-5s-.bpr", 0x0000, 0x0100, CRC(041950e7) SHA1(8276068bec3f4c5013c773033fca3cd3ed9e82ef) )    /* red */
	ROM_LOAD( "wt_a-5r-.bpr", 0x0100, 0x0100, CRC(bc04bf25) SHA1(37d0e89296760f51df5a0d434dca390fb60bb052) )    /* green */
	ROM_LOAD( "wt_a-5p-.bpr", 0x0200, 0x0100, CRC(ed819a19) SHA1(76f13dcf1674f136375738756e175ceec469d545) )    /* blue */
	ROM_LOAD( "wt_b-9l-.bpr", 0x0300, 0x0020, CRC(d2728744) SHA1(e6b1a570854ca90326414874432ab03ec85b9c8e) )    /* char palette */
ROM_END

ROM_START( fghtbskt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fb14.0f",      0x0000, 0x2000, CRC(82032853) SHA1(e103ace4cac6df3a429b785f9789b302ae8cdade) )
	ROM_LOAD( "fb13.2f",      0x2000, 0x2000, CRC(5306df0f) SHA1(11be226e7167703bb08e48510a113b2d43b211a4) )
	ROM_LOAD( "fb12.3f",      0x4000, 0x2000, CRC(ee9210d4) SHA1(c63d036314d635f65a2b5bb192ceb312a587db6e) )
	ROM_LOAD( "fb10.6f",      0x8000, 0x2000, CRC(6b47efba) SHA1(cb55c7a9d5afe748c1c88f87dd1909e106932798) )
	ROM_LOAD( "fb09.7f",      0xa000, 0x2000, CRC(be69e087) SHA1(be95ecafa494cb0787ee18eb3ecea4ad545a6ae3) )

	ROM_REGION( 0x1000, "soundcpu", 0 ) /* 8039 */
	ROM_LOAD( "fb07.0b",      0x0000, 0x1000, CRC(50432dbd) SHA1(35a2218ed243bde47dbe06b5a11a65502ba734ea) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "fb08.12f",     0x0000, 0x1000, CRC(271cd7b8) SHA1(00cfeb6ba429cf6cc59d6542dea8de2ca79155ed) )
	ROM_FILL(                 0x1000, 0x1000, 0x00 )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "fb21.25e",     0x0000, 0x2000, CRC(02843591) SHA1(e38ccc97dcbd642d0ac768837f7baf1573fdb91f) )
	ROM_LOAD( "fb22.23e",     0x2000, 0x2000, CRC(cd51d8e7) SHA1(16d55d13b47dddb7c7e6b28b1512540938a4a596) )
	ROM_LOAD( "fb23.22e",     0x4000, 0x2000, CRC(62bcac87) SHA1(dd2272d8c7e46bd0a742b4490c9e960b2bfe14c3) )

	ROM_REGION( 0xc000, "gfx3", 0 )
	ROM_LOAD( "fb16.35a",     0x0000, 0x2000, CRC(a5df1652) SHA1(76d1443c523851aa418574c6a879f4a8e46dc887) )
	ROM_LOAD( "fb15.37a",     0x2000, 0x2000, CRC(59c4de06) SHA1(594411f10d6bb3577c649c66133b90c6423184d7) )
	ROM_LOAD( "fb18.32a",     0x4000, 0x2000, CRC(c23ddcd7) SHA1(f73d142ac0baae519ed633a923e132eb1836adbb) )
	ROM_LOAD( "fb17.34a",     0x6000, 0x2000, CRC(7db28013) SHA1(305e6a6254f69625c81ae107f4420fd76f9a24ba) )
	ROM_LOAD( "fb20.29a",     0x8000, 0x2000, CRC(1a1b48f8) SHA1(62f7774807aea86f73f0b9380bb1c237d55bf451) )
	ROM_LOAD( "fb19.31a",     0xa000, 0x2000, CRC(7ff7e321) SHA1(4fe4eee9c6260599950080c600187ce8e9dab7d2) )

	ROM_REGION( 0xa000, "samples", 0 ) /* Samples */
	ROM_LOAD( "fb01.42a",     0x0000, 0x2000, CRC(1200b220) SHA1(8a5f896441c6a6507e72b9b302a8183cc361d118) )
	ROM_LOAD( "fb02.41a",     0x2000, 0x2000, CRC(0b67aa82) SHA1(59b6cf733150eab0bd807beeeb1d2f784ccb6f58) )
	ROM_LOAD( "fb03.40a",     0x4000, 0x2000, CRC(c71269ed) SHA1(71cc6f43877b28d50beb744587c189dabbbaa067) )
	ROM_LOAD( "fb04.39a",     0x6000, 0x2000, CRC(02ddc42d) SHA1(9d40967071f674592c174b5a5470db56a5f99adf) )
	ROM_LOAD( "fb05.38a",     0x8000, 0x2000, CRC(72ea6b49) SHA1(e081a1cad5abf373a2489169b5c86ee63dcf5823) )

	ROM_REGION( 0x2000, "user1", 0 )
	ROM_LOAD( "fb06.12a",     0x0000, 0x2000, CRC(bea3df99) SHA1(18b795f8626b22f6a1620e04c23f4967c3122c89) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "fb_r.9e",      0x0000, 0x0100, CRC(c5cdc8ba) SHA1(3fcef3ebe0dda72dfa35e042ff611758c345d749) )
	ROM_LOAD( "fb_g.10e",     0x0100, 0x0100, CRC(1460c936) SHA1(f99a544c83931de098a6cfac391f63ae43f5cdd0) )
	ROM_LOAD( "fb_b.11e",     0x0200, 0x0100, CRC(fca5bf0e) SHA1(5846f43aa2906cac58e300fdab197b99f896e3ef) )
ROM_END

void m63_state::init_wilytowr()
{
	m_sy_offset = 238;
}

void m63_state::init_fghtbskt()
{
	m_sy_offset = 240;
}

} // Anonymous namespace


GAME( 1984, wilytowr, 0,        m63,      wilytowr, m63_state, init_wilytowr, ROT180, "Irem",                    "Wily Tower", MACHINE_SUPPORTS_SAVE )
GAME( 1985, atomboy,  wilytowr, atomboy,  wilytowr, m63_state, init_wilytowr, ROT180, "Irem (Memetron license)", "Atomic Boy (revision B)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, atomboya, wilytowr, atomboy,  wilytowr, m63_state, init_wilytowr, ROT180, "Irem (Memetron license)", "Atomic Boy (revision A)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, fghtbskt, 0,        fghtbskt, fghtbskt, m63_state, init_fghtbskt, ROT0,   "Paradise Co. Ltd.",       "Fighting Basketball", MACHINE_SUPPORTS_SAVE )

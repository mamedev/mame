// license:BSD-3-Clause
// copyright-holders: David Haywood

/***************************************************************************

    Success Joe / Ashita no Joe [Wave]

driver by David Haywood and bits from Pierpaolo Prazzoli


Upper board marked: W9011

Sticker on the upper pcb of Success Joe (World) labelled:
M6100506A
SUCCESS JOE
900110149

Two sub-boards:

Upper:

 Program
  ROMS 1 to 8 (ST M27512)
  Standard Motorola MC68000P8
  8.0000 MHz osc.
  PALs W9011A (AMD PALCE16V8H) + W9011B (MMI PAL 16L88CN)

 Sound
  ROM 9 (ST M27256)
  Standard Zilog Z80 (Z0840004PSC)
  Yamaha YM2203C (horrible music, that is not an emulation bug)

 GFX?
  Mask ROMs 401, 402 & 403 (Hitachi HN62414 Mask ROMs)

 Note: the pcb has a place for a battery circuit but the components are not soldered.

Lower:

 GFX?
  Mask ROMs 404, 405, 406, 407, 408 & 409 (Hitachi HN62414 Mask ROMs)
  PAL W90120R2 (MMI PAL 16L88CN)
  EPL (Ricoh EPL16P8BP, not dumped)
  13.3330 MHz osc.

Dips:

 Two banks (* = default)
  A
                                    1   2   3   4   5   6   7   8
   Game Style      * Table          ON                      OFF OFF
                   Upright          OFF                     OFF OFF
   Screen Reverse  * Usual              OFF                 OFF OFF
                   Reverse              ON                  OFF OFF
   Test Mode       * Game mode              OFF             OFF OFF
                   Test Mode                ON              OFF OFF
   Demo Sound      * Yes                        OFF         OFF OFF
                   No                           ON          OFF OFF
   Play Fee - Coin * 1 Coin 1 Play                  OFF OFF OFF OFF
                   1 Coin 2 Play                    ON  OFF OFF OFF
                   2 Coin 1 Play                    OFF ON  OFF OFF
                   2 Coin 3 Play                    ON  ON  OFF OFF

  B
                                    1   2   3   4   5   6   7   8
   Difficulty      * Rank B         OFF OFF OFF OFF OFF OFF OFF OFF
                   Rank A           ON  OFF OFF OFF OFF OFF OFF OFF
                   Rank C           OFF ON  OFF OFF OFF OFF OFF OFF
                   Rank D           ON  ON  OFF OFF OFF OFF OFF OFF

   Easy (A) -> Difficult (D)

Game is controlled with 4-direction lever and two buttons
Coin B is not used

*************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class ashnojoe_state : public driver_device
{
public:
	ashnojoe_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_tileram(*this, "tileram_%u", 1U),
		m_tilemap_reg(*this, "tilemap_reg"),
		m_audiobank(*this, "audiobank"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void ashnojoe(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr_array<u16, 7> m_tileram;
	required_shared_ptr<u16> m_tilemap_reg;
	required_memory_bank m_audiobank;

	// video-related
	tilemap_t *m_tilemap[7]{};

	// sound-related
	u8 m_adpcm_byte = 0;
	u8 m_msm5205_vclk_toggle = 0;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<generic_latch_8_device> m_soundlatch;

	u16 fake_4a00a_r();
	void adpcm_w(u8 data);
	u8 sound_latch_status_r();
	template<unsigned Which> void tileram_8x8_w(offs_t offset, u16 data);
	template<unsigned Which> void tileram_16x16_w(offs_t offset, u16 data);
	void tilemaps_xscroll_w(offs_t offset, u16 data);
	void tilemaps_yscroll_w(offs_t offset, u16 data);
	void tilemap_regs_w(offs_t offset, u16 data, u16 mem_mask);
	void ym2203_write_a(u8 data);
	void ym2203_write_b(u8 data);
	TILE_GET_INFO_MEMBER(get_tile_info_highest);
	TILE_GET_INFO_MEMBER(get_tile_info_midlow);
	TILE_GET_INFO_MEMBER(get_tile_info_high);
	TILE_GET_INFO_MEMBER(get_tile_info_low);
	TILE_GET_INFO_MEMBER(get_tile_info_midhigh);
	TILE_GET_INFO_MEMBER(get_tile_info_lowest);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vclk_cb(int state);
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void sound_portmap(address_map &map) ATTR_COLD;
};


TILE_GET_INFO_MEMBER(ashnojoe_state::get_tile_info_highest)
{
	const int code = m_tileram[0][tile_index];

	tileinfo.set(2,
			code & 0xfff,
			((code >> 12) & 0x0f),
			0);
}

TILE_GET_INFO_MEMBER(ashnojoe_state::get_tile_info_midlow)
{
	const int code = m_tileram[1][tile_index * 2];
	const int attr = m_tileram[1][tile_index * 2 + 1];

	tileinfo.set(4,
			(code & 0x7fff),
			((attr >> 8) & 0x1f) + 0x40,
			0);
}

TILE_GET_INFO_MEMBER(ashnojoe_state::get_tile_info_high)
{
	const int code = m_tileram[2][tile_index];

	tileinfo.set(0,
			code & 0xfff,
			((code >> 12) & 0x0f) + 0x10,
			0);
}

TILE_GET_INFO_MEMBER(ashnojoe_state::get_tile_info_low)
{
	const int code = m_tileram[3][tile_index];

	tileinfo.set(1,
			code & 0xfff,
			((code >> 12) & 0x0f) + 0x60,
			0);
}

TILE_GET_INFO_MEMBER(ashnojoe_state::get_tile_info_midhigh)
{
	const int code = m_tileram[4][tile_index * 2];
	const int attr = m_tileram[4][tile_index * 2 + 1];

	tileinfo.set(4,
			(code & 0x7fff),
			((attr >> 8) & 0x1f) + 0x20,
			0);
}

TILE_GET_INFO_MEMBER(ashnojoe_state::get_tile_info_lowest)
{
	const int buffer = (m_tilemap_reg[0] & 0x02) >> 1;
	int const code = m_tileram[5 + buffer][tile_index * 2];
	int const attr = m_tileram[5 + buffer][tile_index * 2 + 1];

	tileinfo.set(3,
			(code & 0x1fff),
			((attr >> 8) & 0x1f) + 0x70,
			0);
}


void ashnojoe_state::tilemaps_xscroll_w(offs_t offset, u16 data)
{
	switch (offset)
	{
	case 0:
		m_tilemap[2]->set_scrollx(0, data);
		break;
	case 1:
		m_tilemap[4]->set_scrollx(0, data);
		break;
	case 2:
		m_tilemap[1]->set_scrollx(0, data);
		break;
	case 3:
		m_tilemap[3]->set_scrollx(0, data);
		break;
	case 4:
		m_tilemap[5]->set_scrollx(0, data);
		break;
	}
}

void ashnojoe_state::tilemaps_yscroll_w(offs_t offset, u16 data)
{
	switch (offset)
	{
	case 0:
		m_tilemap[2]->set_scrolly(0, data);
		break;
	case 1:
		m_tilemap[4]->set_scrolly(0, data);
		break;
	case 2:
		m_tilemap[1]->set_scrolly(0, data);
		break;
	case 3:
		m_tilemap[3]->set_scrolly(0, data);
		break;
	case 4:
		m_tilemap[5]->set_scrolly(0, data);
		break;
	}
}

void ashnojoe_state::tilemap_regs_w(offs_t offset, u16 data, u16 mem_mask)
{
	const u16 old = m_tilemap_reg[offset];
	data = COMBINE_DATA(&m_tilemap_reg[offset]);
	if (old != data)
	{
		if (offset == 0)
		{
			if ((old ^ data) & 0x02)
				m_tilemap[5]->mark_all_dirty();
		}
	}
}

void ashnojoe_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ashnojoe_state::get_tile_info_highest)), TILEMAP_SCAN_ROWS,  8,  8, 64, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ashnojoe_state::get_tile_info_midlow)),  TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ashnojoe_state::get_tile_info_high)),    TILEMAP_SCAN_ROWS,  8,  8, 64, 64);
	m_tilemap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ashnojoe_state::get_tile_info_low)),     TILEMAP_SCAN_ROWS,  8,  8, 64, 64);
	m_tilemap[4] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ashnojoe_state::get_tile_info_midhigh)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap[5] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ashnojoe_state::get_tile_info_lowest)),  TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_tilemap[0]->set_transparent_pen(15);
	m_tilemap[1]->set_transparent_pen(15);
	m_tilemap[2]->set_transparent_pen(15);
	m_tilemap[3]->set_transparent_pen(15);
	m_tilemap[4]->set_transparent_pen(15);
}

u32 ashnojoe_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//m_tilemap_reg[0] & 0x10 // ?? on coin insertion

	flip_screen_set(m_tilemap_reg[0] & 1);

	m_tilemap[5]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[3]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[4]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[2]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


u16 ashnojoe_state::fake_4a00a_r()
{
	/* If it returns 1 there's no sound. Is it used to sync the game and sound?
	or just a debug enable/disable register? */
	return 0;
}

template<unsigned Which>
void ashnojoe_state::tileram_8x8_w(offs_t offset, u16 data)
{
	m_tileram[Which][offset] = data;
	m_tilemap[Which]->mark_tile_dirty(offset);
}

template<unsigned Which>
void ashnojoe_state::tileram_16x16_w(offs_t offset, u16 data)
{
	const int buffer = (m_tilemap_reg[0] & 0x02);
	m_tileram[Which][offset] = data;
	if (((Which == 5 && !buffer) || (Which == 6 && buffer)) || (Which < 5))
		m_tilemap[(Which < 5) ? Which : 5]->mark_tile_dirty(offset / 2);
}

void ashnojoe_state::main_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x040000, 0x041fff).ram().w(FUNC(ashnojoe_state::tileram_8x8_w<2>)).share(m_tileram[2]);
	map(0x042000, 0x043fff).ram().w(FUNC(ashnojoe_state::tileram_8x8_w<3>)).share(m_tileram[3]);
	map(0x044000, 0x044fff).ram().w(FUNC(ashnojoe_state::tileram_16x16_w<4>)).share(m_tileram[4]);
	map(0x045000, 0x045fff).ram().w(FUNC(ashnojoe_state::tileram_16x16_w<1>)).share(m_tileram[1]);
	map(0x046000, 0x046fff).ram().w(FUNC(ashnojoe_state::tileram_16x16_w<5>)).share(m_tileram[5]);
	map(0x047000, 0x047fff).ram().w(FUNC(ashnojoe_state::tileram_16x16_w<6>)).share(m_tileram[6]);
	map(0x048000, 0x048fff).ram().w(FUNC(ashnojoe_state::tileram_8x8_w<0>)).share(m_tileram[0]);
	map(0x049000, 0x049fff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x04a000, 0x04a001).portr("P1");
	map(0x04a002, 0x04a003).portr("P2");
	map(0x04a004, 0x04a005).portr("DSW");
	map(0x04a006, 0x04a007).w(FUNC(ashnojoe_state::tilemap_regs_w)).share(m_tilemap_reg);
	map(0x04a009, 0x04a009).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x04a00a, 0x04a00b).r(FUNC(ashnojoe_state::fake_4a00a_r));  // ??
	map(0x04a010, 0x04a019).w(FUNC(ashnojoe_state::tilemaps_xscroll_w));
	map(0x04a020, 0x04a029).w(FUNC(ashnojoe_state::tilemaps_yscroll_w));
	map(0x04c000, 0x04ffff).ram();
	map(0x080000, 0x0bffff).rom();
}


void ashnojoe_state::adpcm_w(u8 data)
{
	m_adpcm_byte = data;
}

u8 ashnojoe_state::sound_latch_status_r()
{
	return m_soundlatch->pending_r();
}

void ashnojoe_state::sound_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x7fff).ram();
	map(0x8000, 0xffff).bankr(m_audiobank);
}

void ashnojoe_state::sound_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x02, 0x02).w(FUNC(ashnojoe_state::adpcm_w));
	map(0x04, 0x04).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x06, 0x06).r(FUNC(ashnojoe_state::sound_latch_status_r));
}


static INPUT_PORTS_START( ashnojoe )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )  // anything else and the controls don't work
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )  // anything else and the controls don't work
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	/* unused ? */
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_SERVICE( 0x0004, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0030, 0x0000, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0400, 0x0000, "Number of controller" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0400, "1" )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )
INPUT_PORTS_END


static GFXDECODE_START( gfx_ashnojoe )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x4_packed_msb,   0, 0x100 )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_8x8x4_packed_msb,   0, 0x100 )
	GFXDECODE_ENTRY( "gfx3", 0, gfx_8x8x4_packed_msb,   0, 0x100 )
	GFXDECODE_ENTRY( "gfx4", 0, gfx_16x16x4_packed_msb, 0, 0x100 )
	GFXDECODE_ENTRY( "gfx5", 0, gfx_16x16x4_packed_msb, 0, 0x100 )
GFXDECODE_END

void ashnojoe_state::ym2203_write_a(u8 data)
{
	// HACK: This gets called at 8910 startup with 0xff before the 5205 exists, causing a crash
	if (data == 0xff)
		return;

	m_msm->reset_w(!(data & 0x01));
}

void ashnojoe_state::ym2203_write_b(u8 data)
{
	m_audiobank->set_entry(data & 0x0f);
}

void ashnojoe_state::vclk_cb(int state)
{
	if (m_msm5205_vclk_toggle == 0)
	{
		m_msm->data_w(m_adpcm_byte >> 4);
	}
	else
	{
		m_msm->data_w(m_adpcm_byte & 0xf);
		m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}

	m_msm5205_vclk_toggle ^= 1;
}

void ashnojoe_state::machine_start()
{
	u8 *ROM = memregion("audiocpu")->base();
	m_audiobank->configure_entries(0, 16, &ROM[0x10000], 0x8000);
	m_audiobank->set_entry(0);

	save_item(NAME(m_adpcm_byte));
	save_item(NAME(m_msm5205_vclk_toggle));
}

void ashnojoe_state::machine_reset()
{
	// start the sound section with a known state
	// (would otherwise playback the full ADPCM bank on soft resets)
	m_adpcm_byte = 0;
	m_msm5205_vclk_toggle = 0;
	m_msm->reset_w(1);
	m_audiobank->set_entry(0);
}


void ashnojoe_state::ashnojoe(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &ashnojoe_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(ashnojoe_state::irq1_line_hold));

	Z80(config, m_audiocpu, 8_MHz_XTAL / 2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &ashnojoe_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &ashnojoe_state::sound_portmap);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 512);
	screen.set_visarea(14*8, 50*8-1, 3*8, 29*8-1);
	screen.set_screen_update(FUNC(ashnojoe_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_ashnojoe);
	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 0x1000 / 2);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	ym2203_device &ymsnd(YM2203(config, "ymsnd", 8_MHz_XTAL / 2));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.port_a_write_callback().set(FUNC(ashnojoe_state::ym2203_write_a));
	ymsnd.port_b_write_callback().set(FUNC(ashnojoe_state::ym2203_write_b));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.1);

	MSM5205(config, m_msm, 384'000);
	m_msm->vck_legacy_callback().set(FUNC(ashnojoe_state::vclk_cb));
	m_msm->set_prescaler_selector(msm5205_device::S48_4B);
	m_msm->add_route(ALL_OUTPUTS, "mono", 1.0);
}

ROM_START( scessjoe )
	ROM_REGION( 0xc0000, "maincpu", 0 )     // 68000
	ROM_LOAD16_BYTE( "5.4q", 0x00000, 0x10000, CRC(c805f9e7) SHA1(e1e85701bde496b1fd64211b94bfb0def597ae51) )
	ROM_LOAD16_BYTE( "6.4s", 0x00001, 0x10000, CRC(eda7a537) SHA1(3bb19fbdfb6c8af4e2078958fa445ac1f4434d0d) )
	ROM_LOAD16_WORD_SWAP( "sj201-nw.6m", 0x80000, 0x40000, CRC(5a64ca42) SHA1(660b8bca21ef3c2230adce7cb7e7d1f018714f23) )

	ROM_REGION( 0x90000, "audiocpu", 0 )     // Z80
	ROM_LOAD( "9.8q",         0x00000, 0x08000, CRC(8767e212) SHA1(13bf927febedff9d7d164fbf0da7fb3a588c2a94) )
	ROM_LOAD( "sj401-nw.10r", 0x10000, 0x80000, CRC(25dfab59) SHA1(7d50159204ba05323a2442778f35192e66117dda) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "8.5e", 0x00000, 0x10000, CRC(9bcb160e) SHA1(1677048e5ce26562ff7ba36fcc2d0ed5a652b91e) )
	ROM_LOAD( "7.5c", 0x10000, 0x10000, CRC(b250c69d) SHA1(594b1bb94a162b07944a971b7fedddca5c37f2cb) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "4.4e", 0x00000, 0x10000, CRC(aa6336d3) SHA1(43f70cc3223f11d7929dd44b0edf0a31f5fe41c3) )
	ROM_LOAD( "3.4c", 0x10000, 0x10000, CRC(7e2d86b5) SHA1(8b8d1b9240a700e29afc109eddf6e58a0a7666a4) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "2.3m", 0x00000, 0x10000, CRC(c3254938) SHA1(fd57163f740cd4fdecca94cced91314c289741ae) )
	ROM_LOAD( "1.1m", 0x10000, 0x10000, CRC(5d16a6fa) SHA1(2af907b0fcb9ff93340de3301da4b10e945455e5) )

	ROM_REGION( 0x100000, "gfx4", 0 )
	ROM_LOAD16_WORD_SWAP( "sj402-nw.8e", 0x000000, 0x80000, CRC(b6d33d06) SHA1(688ccf467a5112ec522811894e2626ab5f155903) )
	ROM_LOAD16_WORD_SWAP( "sj403-nw.7e", 0x080000, 0x80000, CRC(07143f56) SHA1(1b953c8826d3993a486eed6b9d94d37145fd2e79) )

	ROM_REGION( 0x300000, "gfx5", 0 )
	ROM_LOAD16_WORD_SWAP( "sj404-nw.7a", 0x000000, 0x80000, CRC(8f134128) SHA1(026a6076d54cd5f1d06b29c51031cb79a6b2c11d) )
	ROM_LOAD16_WORD_SWAP( "sj405-nw.7c", 0x080000, 0x80000, CRC(6fd81699) SHA1(8a4f9e47dd39b4b0213c3682da2221ca53bba658) )
	ROM_LOAD16_WORD_SWAP( "sj406-nw.7d", 0x100000, 0x80000, CRC(634e33e6) SHA1(1d6a72a4ca80cd1c1fd6ce9359c304b45091cdfe) )
	ROM_LOAD16_WORD_SWAP( "sj407-nw.7f", 0x180000, 0x80000, CRC(5c66ff06) SHA1(9923ba00679e1b47b5da63c1a13e0f8dd4c78bb5) )
	ROM_LOAD16_WORD_SWAP( "sj408-nw.7g", 0x200000, 0x80000, CRC(6a3b1ea1) SHA1(e39a6e52d930f291bf237cf9db3d4b3d2fad53e0) )
	ROM_LOAD16_WORD_SWAP( "sj409-nw.7j", 0x280000, 0x80000, CRC(d8764213) SHA1(89eadefb956863216c8e3d0380394aba35e8c856) )
ROM_END

ROM_START( ashnojoe )
	ROM_REGION( 0xc0000, "maincpu", 0 )     // 68000
	ROM_LOAD16_BYTE( "5.bin", 0x00000, 0x10000, CRC(c61e1569) SHA1(422c18f5810539b5a9e3a9bd4e3b4d70bde8d1d5) )
	ROM_LOAD16_BYTE( "6.bin", 0x00001, 0x10000, CRC(c0a16338) SHA1(fb127b9d38f2c9807b6e23ff71935fc8a22a2e8f) )
	ROM_LOAD16_WORD_SWAP( "sj201-nw.6m", 0x80000, 0x40000, CRC(5a64ca42) SHA1(660b8bca21ef3c2230adce7cb7e7d1f018714f23) )

	ROM_REGION( 0x90000, "audiocpu", 0 )     // Z80
	ROM_LOAD( "9.8q",         0x00000, 0x08000, CRC(8767e212) SHA1(13bf927febedff9d7d164fbf0da7fb3a588c2a94) )
	ROM_LOAD( "sj401-nw.10r", 0x10000, 0x80000, CRC(25dfab59) SHA1(7d50159204ba05323a2442778f35192e66117dda) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "8.5e",  0x00000, 0x10000, CRC(9bcb160e) SHA1(1677048e5ce26562ff7ba36fcc2d0ed5a652b91e) )
	ROM_LOAD( "7.bin", 0x10000, 0x10000, CRC(7e1efc42) SHA1(e3c282072fdaa0b98c2a1bf25fd02c680d9ca4d7) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "4.4e", 0x00000, 0x10000, CRC(aa6336d3) SHA1(43f70cc3223f11d7929dd44b0edf0a31f5fe41c3) )
	ROM_LOAD( "3.4c", 0x10000, 0x10000, CRC(7e2d86b5) SHA1(8b8d1b9240a700e29afc109eddf6e58a0a7666a4) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "2.3m",  0x00000, 0x10000, CRC(c3254938) SHA1(fd57163f740cd4fdecca94cced91314c289741ae) )
	ROM_LOAD( "1.bin", 0x10000, 0x10000, CRC(1bf585f0) SHA1(4003941636e7fded95e880109c3c9dd1d8f28b07) )

	ROM_REGION( 0x100000, "gfx4", 0 )
	ROM_LOAD16_WORD_SWAP( "sj402-nw.8e", 0x000000, 0x80000, CRC(b6d33d06) SHA1(688ccf467a5112ec522811894e2626ab5f155903) )
	ROM_LOAD16_WORD_SWAP( "sj403-nw.7e", 0x080000, 0x80000, CRC(07143f56) SHA1(1b953c8826d3993a486eed6b9d94d37145fd2e79) )

	ROM_REGION( 0x300000, "gfx5", 0 )
	ROM_LOAD16_WORD_SWAP( "sj404-nw.7a", 0x000000, 0x80000, CRC(8f134128) SHA1(026a6076d54cd5f1d06b29c51031cb79a6b2c11d) )
	ROM_LOAD16_WORD_SWAP( "sj405-nw.7c", 0x080000, 0x80000, CRC(6fd81699) SHA1(8a4f9e47dd39b4b0213c3682da2221ca53bba658) )
	ROM_LOAD16_WORD_SWAP( "sj406-nw.7d", 0x100000, 0x80000, CRC(634e33e6) SHA1(1d6a72a4ca80cd1c1fd6ce9359c304b45091cdfe) )
	ROM_LOAD16_WORD_SWAP( "sj407-nw.7f", 0x180000, 0x80000, CRC(5c66ff06) SHA1(9923ba00679e1b47b5da63c1a13e0f8dd4c78bb5) )
	ROM_LOAD16_WORD_SWAP( "sj408-nw.7g", 0x200000, 0x80000, CRC(6a3b1ea1) SHA1(e39a6e52d930f291bf237cf9db3d4b3d2fad53e0) )
	ROM_LOAD16_WORD_SWAP( "sj409-nw.7j", 0x280000, 0x80000, CRC(d8764213) SHA1(89eadefb956863216c8e3d0380394aba35e8c856) )
ROM_END

} // anonymous namespace


GAME( 1990, scessjoe, 0,        ashnojoe, ashnojoe, ashnojoe_state, empty_init, ROT0, "Taito Corporation / Wave", "Success Joe (World)",   MACHINE_SUPPORTS_SAVE )
GAME( 1990, ashnojoe, scessjoe, ashnojoe, ashnojoe, ashnojoe_state, empty_init, ROT0, "Taito Corporation / Wave", "Ashita no Joe (Japan)", MACHINE_SUPPORTS_SAVE )

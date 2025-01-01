// license:BSD-3-Clause
// copyright-holders: Nicola Salmoria

/***************************************************************************

Mahjong Kyou Jidai (麻雀狂時代)     (c)1986 Sanritsu

CPU: Z80
I/O: NEC D8255AC*2
Sound: SN76489*2 CUSTOM
OSC: 10MHz ??MHz

driver by Nicola Salmoria

TODO:
- Complete dip switches.

- Several imperfections with sprites rendering:
  - some sprites are misplaced by 1pixel vertically
  - during the tile distribution at the beginning of a match, there's something
    wrong with the stacks moved around, they are misaligned and something is
    missing.

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "sound/msm5205.h"
#include "sound/sn76496.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_CTRL     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_CTRL)

#include "logmacro.h"

#define LOGCTRL(...)     LOGMASKED(LOG_CTRL,     __VA_ARGS__)


namespace {

class mjkjidai_state : public driver_device
{
public:
	mjkjidai_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_msm(*this, "msm"),
		m_nvram(*this, "nvram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_adpcmrom(*this, "adpcm"),
		m_videoram(*this, "videoram"),
		m_mainbank(*this, "mainbank"),
		m_row(*this, "ROW.%u", 0)
	{ }

	void mjkjidai(machine_config &config);

	ioport_value keyboard_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<msm5205_device> m_msm;
	required_device<nvram_device> m_nvram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_region_ptr<uint8_t> m_adpcmrom;
	required_shared_ptr<uint8_t> m_videoram;
	required_memory_bank m_mainbank;

	required_ioport_array<12> m_row;

	uint16_t m_adpcm_pos = 0;
	uint32_t m_adpcm_end = 0;
	uint16_t m_keyb = 0;
	bool m_nmi_enable = false;
	bool m_display_enable = false;
	tilemap_t *m_bg_tilemap = nullptr;

	void keyboard_select_lo_w(uint8_t data);
	void keyboard_select_hi_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	void ctrl_w(uint8_t data);
	void adpcm_w(uint8_t data);
	void adpcm_int(int state);
	TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(int state);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void io_map(address_map &map) ATTR_COLD;
	void prg_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(mjkjidai_state::get_tile_info)
{
	int attr = m_videoram[tile_index + 0x800];
	int code = m_videoram[tile_index] + ((attr & 0x1f) << 8);
	int color = m_videoram[tile_index + 0x1000];
	tileinfo.set(0, code, color >> 3, 0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void mjkjidai_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mjkjidai_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void mjkjidai_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x7ff);
}

void mjkjidai_state::ctrl_w(uint8_t data)
{
	LOGCTRL("%s: port c0 = %02x\n", m_maincpu->pc(), data);

	// bit 0 = NMI enable
	m_nmi_enable = data & 1;
	if (!m_nmi_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	// bit 1 = flip screen
	flip_screen_set(data & 0x02);

	// bit 2 = display enable
	m_display_enable = data & 0x04;

	// bit 5 = coin counter
	machine().bookkeeping().coin_counter_w(0, data & 0x20);

	// bits 6-7 select ROM bank
	m_mainbank->set_entry(data >> 6);
}



/***************************************************************************

  Display refresh

***************************************************************************/

void mjkjidai_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t *spriteram = &m_videoram[0];
	uint8_t *spriteram_2 = &m_videoram[0x800];
	uint8_t *spriteram_3 = &m_videoram[0x1000];

	for (int offs = 0x20 - 2; offs >= 0; offs -= 2)
	{
		int code = spriteram[offs] + ((spriteram_2[offs] & 0x1f) << 8);
		int color = (spriteram_3[offs] & 0x78) >> 3;
		int sx = 2 * spriteram_2[offs + 1];
		int sy = 240 - spriteram[offs + 1];
		int flipx = code & 1;
		int flipy = code & 2;

		code >>= 2;

		sx += (spriteram_2[offs] & 0x20) >> 5;  // not sure about this

		if (flip_screen())
		{
			sx = 496 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		sx += 16;
		sy += 1;

		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
				code,
				color,
				flipx, flipy,
				sx, sy, 0);
	}
}



uint32_t mjkjidai_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!m_display_enable)
		bitmap.fill(m_palette->black_pen(), cliprect);
	else
	{
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		draw_sprites(bitmap, cliprect);
	}
	return 0;
}


void mjkjidai_state::adpcm_w(uint8_t data)
{
	m_adpcm_pos = (data & 0x07) * 0x1000 * 2;
	m_adpcm_end = m_adpcm_pos + 0x1000 * 2;
	m_msm->reset_w(0);
}

void mjkjidai_state::adpcm_int(int state)
{
	if (m_adpcm_pos >= m_adpcm_end)
	{
		m_msm->reset_w(1);
	}
	else
	{
		uint8_t const data = m_adpcmrom[m_adpcm_pos / 2];
		m_msm->data_w(m_adpcm_pos & 1 ? data & 0xf : data >> 4);
		m_adpcm_pos++;
	}
}

ioport_value mjkjidai_state::keyboard_r()
{
	int res = 0x3f;

	for (int i = 0; i < 12; i++)
	{
		if (~m_keyb & (0x800 >> i))
		{
			res = m_row[i]->read();
			break;
		}
	}

	return res;
}

void mjkjidai_state::keyboard_select_lo_w(uint8_t data)
{
	m_keyb = (m_keyb & 0xff00) | (data);
}

void mjkjidai_state::keyboard_select_hi_w(uint8_t data)
{
	m_keyb = (m_keyb & 0x00ff) | (data << 8);
}


void mjkjidai_state::prg_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_mainbank);
	map(0xc000, 0xcfff).ram();
	map(0xd000, 0xdfff).ram().share("nvram");   // cleared and initialized on startup if bit 6 of port 00 is 0
	map(0xe000, 0xf7ff).ram().w(FUNC(mjkjidai_state::videoram_w)).share(m_videoram);
}

void mjkjidai_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x10, 0x13).rw("ppi2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x20, 0x20).w("sn1", FUNC(sn76489_device::write));
	map(0x30, 0x30).w("sn2", FUNC(sn76489_device::write));
	map(0x40, 0x40).w(FUNC(mjkjidai_state::adpcm_w));
}


static INPUT_PORTS_START( mjkjidai )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Test ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x20, 0x20, "Statistics" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEYBOARD")
	PORT_BIT( 0x3f, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CUSTOM_MEMBER(FUNC(mjkjidai_state::keyboard_r))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MEMORY_RESET )   // reinitialize NVRAM and reset the game
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("ROW.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW.4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x38, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW.5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x3e, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW.6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW.7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW.8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW.9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW.10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x38, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW.11")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x3e, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};

static GFXDECODE_START( gfx_mjkjidai )
	GFXDECODE_ENTRY( "gfx", 0, charlayout,   0, 32 )
	GFXDECODE_ENTRY( "gfx", 0, spritelayout, 0, 16 )
GFXDECODE_END

void mjkjidai_state::vblank_irq(int state)
{
	if (state && m_nmi_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

void mjkjidai_state::machine_start()
{
	m_mainbank->configure_entries(0, 4, memregion("maincpu")->base() + 0x8000, 0x4000);

	save_item(NAME(m_adpcm_pos));
	save_item(NAME(m_adpcm_end));
	save_item(NAME(m_keyb));
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_display_enable));
}

void mjkjidai_state::machine_reset()
{
	m_adpcm_pos = m_adpcm_end = 0;
}

void mjkjidai_state::mjkjidai(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 10_MHz_XTAL / 2); // 5 MHz ??? divider unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &mjkjidai_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &mjkjidai_state::io_map);

	NVRAM(config, m_nvram, nvram_device::DEFAULT_NONE);

	i8255_device &ppi1(I8255A(config, "ppi1"));
	ppi1.in_pa_callback().set_ioport("KEYBOARD");
	ppi1.out_pb_callback().set(FUNC(mjkjidai_state::keyboard_select_lo_w));
	ppi1.out_pc_callback().set(FUNC(mjkjidai_state::keyboard_select_hi_w));
	ppi1.in_pc_callback().set_ioport("IN2");

	i8255_device &ppi2(I8255A(config, "ppi2"));
	ppi2.out_pa_callback().set(FUNC(mjkjidai_state::ctrl_w));  // ROM bank, coin counter, flip screen etc
	ppi2.in_pb_callback().set_ioport("DSW1");
	ppi2.in_pc_callback().set_ioport("DSW2");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(3*8, 61*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(mjkjidai_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(mjkjidai_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_mjkjidai);
	PALETTE(config, m_palette, palette_device::RGB_444_PROMS, "proms", 0x100);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	SN76489(config, "sn1", 10_MHz_XTAL / 4).add_route(ALL_OUTPUTS, "mono", 0.50);
	SN76489(config, "sn2", 10_MHz_XTAL / 4).add_route(ALL_OUTPUTS, "mono", 0.50);

	MSM5205(config, m_msm, 384000);
	m_msm->vck_legacy_callback().set(FUNC(mjkjidai_state::adpcm_int));
	m_msm->set_prescaler_selector(msm5205_device::S64_4B);  // 6kHz
	m_msm->add_route(ALL_OUTPUTS, "mono", 1.0);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( mjkjidai )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "mkj-00.14g",   0x00000, 0x8000, CRC(188a27e9) SHA1(2306ad112aaf8d9ac77a89d0e4c3a17f36945130) )
	ROM_LOAD( "mkj-01.15g",   0x08000, 0x8000, CRC(a6a5e9c7) SHA1(974f4343f4347a0065f833c1fdcc47e96d42932d) )
	ROM_LOAD( "mkj-02.16g",   0x10000, 0x8000, CRC(fb312927) SHA1(b71db72ba881474f9c2523d0617757889af9f28e) )

	ROM_REGION( 0x30000, "gfx", 0 )
	ROM_LOAD( "mkj-20.4e",    0x00000, 0x8000, CRC(8fc66bce) SHA1(4f1006bc5168e39eb7a1f6a4b3c3f5aaa3c1c7dd) )
	ROM_LOAD( "mkj-21.5e",    0x08000, 0x8000, CRC(4dd41a9b) SHA1(780f9e5bbf9dc47e931cebd67d89122209f573a2) )
	ROM_LOAD( "mkj-22.6e",    0x10000, 0x8000, CRC(70ac2bd7) SHA1(8ddb00a24f2b49b9eb1a70ae95fcd6bb0820be50) )
	ROM_LOAD( "mkj-23.7e",    0x18000, 0x8000, CRC(f9313dde) SHA1(787577ccdc7e7030439159c194ca6719df80ad2f) )
	ROM_LOAD( "mkj-24.8e",    0x20000, 0x8000, CRC(aa5130d0) SHA1(1dbaf2ba9ed97c22dc74d12471fc54b0f7ce2f25) )
	ROM_LOAD( "mkj-25.9e",    0x28000, 0x8000, CRC(c12c3fe0) SHA1(0acd3f8e8d849a09b187cd83852593a64aa87451) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "mkj-60.13a",   0x0000, 0x0100, CRC(5dfaba60) SHA1(7c821a5e951ccf9d86d98aa8dc75d847ab579496) )
	ROM_LOAD( "mkj-61.14a",   0x0100, 0x0100, CRC(e9e90d55) SHA1(a14177df3bab59e0f9ce41094e03ef3593329149) )
	ROM_LOAD( "mkj-62.15a",   0x0200, 0x0100, CRC(934f1d53) SHA1(2b3b2dc77789b814810b25cda3f5adcfd7e0e57e) )

	ROM_REGION( 0x8000, "adpcm", 0 )
	ROM_LOAD( "mkj-40.14c",   0x0000, 0x8000, CRC(4d8fcc4a) SHA1(24c2b8031367035c89c6649a084bce0714f3e8d4) )

	ROM_REGION( 0x1000, "nvram", 0 )    // preformatted NVRAM
	ROM_LOAD( "default.nv",   0x0000, 0x1000, CRC(eccc0263) SHA1(679010f096536e8bb572551e9d0776cad72145e2) )
ROM_END

} // anonymous namespace


GAME( 1986, mjkjidai, 0, mjkjidai, mjkjidai, mjkjidai_state, empty_init, ROT0, "Sanritsu", "Mahjong Kyou Jidai (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

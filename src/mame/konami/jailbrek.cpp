// license:BSD-3-Clause
// copyright-holders: Ernesto Corvi

/***************************************************************************

Jailbreak - (c) 1986 Konami

Ernesto Corvi
ernesto@imagina.com


Konami designated Jail Break with the label of GX507.  (There is also
the label PWB 300394A silk screened onto the board.)


Board Parts:

    Konami A082 @ 11a (Encrypted 6809 CPU)
    VLM5030 @ 6a
    Konami 005849 @ 8e
    18.432000MHz @ OSC
    3.579545MHz @ XTAL
    SN76489AN @ 6d
    6301 @ 6f and 7f (PROM's)
    6331 @ 1f and 2f (PROM's)

Jail Break
Konami 1986

PCB Layout
----------

GX507
PWB 300394A
|----------------------------------------|
| KONAMI-1   507P03.11D    6264     4416 |
|            507P02.9D    |-------| 4416 |
|                         |KONAMI | 4416 |
|       507L01.8C         |005849 | 4416 |
|                         |-------|      |
| VLM5030                       507J13.7F|
|                    SN76489    507J12.6F|
| 3.579545MHz        18.432MHz           |
| DSW3                          507J09.5F|
|                               507L08.4F|
| DSW2                          507J07.3F|
|           UPC324                       |
| DSW1                          507J11.2F|
|      LA4460                   507J10.1F|
|             005273 005273              |
|             005273 005273              |
|     VOL           18-WAY        CN1    |
|----------------------------------------|
Notes:
      KONAMI1   - Custom encrypted 6809 CPU, clock 1.536MHz [18.432/12] (DIP42)
      VLM5030   - Sanyo VLM5030 speech chip, clock 3.579545MHz (DIP40)
      SN76489   - Texas Instruments SN76489 noise generator IC, clock 1.536MHz [18.432/12] (DIP16)
      UPC324    - NEC UPC324 op amp. Also compatible with LM324 and Mitsubishi M5224P (DIP14)
      LA4460    - Power amp IC
      DSW1/2    - 8-position dip switches
      DSW3      - 4-position dip switch
      CN1       - 4-pin connector for monitors that require separate syncs. Pin 1 is negative vertical sync output.
                  Horizontal sync is taken from the regular composite sync on the PCB on pin B14.
      18-WAY    - 18-way edge connector. Pinout matches standard Konami 18-way pinout (Scramble/Frogger/Yie-Ar Kung Fu etc)
      6264      - 8kx8 SRAM (DIP28)
      4416      - 16kx4 DRAM (DIP18)
      005849    - Konami custom graphics generator (PGA179)
      005273    - Konami custom resistor array (SIL10)
      507J12/13 - MMI 63S141 256x4 Bipolar PROM (= 82S129 & 6301 etc)
      507J10/11 - MMI 63S081 32x8 Bipolar PROM (= 82S123 & 6331 etc)
      All ROMs type 27C128. Pin 26 of 8C is tied high, the lower half is empty.

      Measurements
      ------------
      Xtal  - 3.57867MHz
      OSC   - 18.43199MHz
      HSync - 15.5185kHz
      VSync - 60.6059Hz


****************************************************************************

    TODO:

    - various unknown writes (NOPed out in the memory map)

***************************************************************************/

#include "emu.h"

#include "konami1.h"
#include "konamipt.h"

#include "cpu/m6809/m6809.h"
#include "machine/watchdog.h"
#include "sound/sn76496.h"
#include "sound/vlm5030.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class jailbrek_state : public driver_device
{
public:
	jailbrek_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_scroll_x(*this, "scroll_x"),
		m_scroll_dir(*this, "scroll_dir"),
		m_maincpu(*this, "maincpu"),
		m_vlm(*this, "vlm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void jailbrek(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_scroll_x;
	required_shared_ptr<uint8_t> m_scroll_dir;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<vlm5030_device> m_vlm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;

	// misc
	uint8_t m_irq_enable = 0U;
	uint8_t m_nmi_enable = 0U;

	void ctrl_w(uint8_t data);
	void coin_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	uint8_t speech_r();
	void speech_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(int state);
	INTERRUPT_GEN_MEMBER(interrupt_nmi);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void prg_map(address_map &map) ATTR_COLD;
	void vlm_map(address_map &map) ATTR_COLD;
};


void jailbrek_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	// create a lookup table for the palette
	for (int i = 0; i < 0x20; i++)
	{
		int const r = pal4bit(color_prom[i + 0x00] >> 0);
		int const g = pal4bit(color_prom[i + 0x00] >> 4);
		int const b = pal4bit(color_prom[i + 0x20] >> 0);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x40;

	for (int i = 0; i < 0x100; i++)
	{
		uint8_t const ctabentry = (color_prom[i] & 0x0f) | 0x10;
		palette.set_pen_indirect(i, ctabentry);
	}

	for (int i = 0x100; i < 0x200; i++)
	{
		uint8_t const ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}
}

void jailbrek_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void jailbrek_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(jailbrek_state::get_bg_tile_info)
{
	int const attr = m_colorram[tile_index];
	int const code = m_videoram[tile_index] + ((attr & 0xc0) << 2);
	int const color = attr & 0x0f;

	tileinfo.set(0, code, color, 0);
}

void jailbrek_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(jailbrek_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
}

void jailbrek_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int i = 0; i < m_spriteram.bytes(); i += 4)
	{
		int const attr = m_spriteram[i + 1];    // attributes = ?tyxcccc
		int const code = m_spriteram[i] + ((attr & 0x40) << 2);
		int const color = attr & 0x0f;
		int flipx = attr & 0x10;
		int flipy = attr & 0x20;
		int sx = m_spriteram[i + 2] - ((attr & 0x80) << 1);
		int sy = m_spriteram[i + 3];

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->transmask(bitmap, cliprect, code, color, flipx, flipy,
			sx, sy,
			m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 0));
	}
}

uint32_t jailbrek_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// added support for vertical scrolling (credits).  23/1/2002  -BR
	// bit 2 appears to be horizontal/vertical scroll control
	if (m_scroll_dir[0] & 0x04)
	{
		m_bg_tilemap->set_scroll_cols(32);
		m_bg_tilemap->set_scroll_rows(1);
		m_bg_tilemap->set_scrollx(0, 0);

		for (int i = 0; i < 32; i++)
			m_bg_tilemap->set_scrolly(i, ((m_scroll_x[i + 32] << 8) + m_scroll_x[i]));
	}
	else
	{
		m_bg_tilemap->set_scroll_rows(32);
		m_bg_tilemap->set_scroll_cols(1);
		m_bg_tilemap->set_scrolly(0, 0);

		for (int i = 0; i < 32; i++)
			m_bg_tilemap->set_scrollx(i, ((m_scroll_x[i + 32] << 8) + m_scroll_x[i]));
	}

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}


void jailbrek_state::ctrl_w(uint8_t data)
{
	m_nmi_enable = data & 0x01;
	m_irq_enable = data & 0x02;
	flip_screen_set(data & 0x08);
}

void jailbrek_state::coin_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	machine().bookkeeping().coin_counter_w(1, data & 0x02);
}

void jailbrek_state::vblank_irq(int state)
{
	if (state && m_irq_enable)
		m_maincpu->set_input_line(0, HOLD_LINE);
}

INTERRUPT_GEN_MEMBER(jailbrek_state::interrupt_nmi)
{
	if (m_nmi_enable)
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


uint8_t jailbrek_state::speech_r()
{
	return (m_vlm->bsy() ? 1 : 0);
}

void jailbrek_state::speech_w(uint8_t data)
{
	// bit 0 is latch direction like in yiear
	m_vlm->st((data >> 1) & 1);
	m_vlm->rst((data >> 2) & 1);
}

void jailbrek_state::prg_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().w(FUNC(jailbrek_state::colorram_w)).share(m_colorram);
	map(0x0800, 0x0fff).ram().w(FUNC(jailbrek_state::videoram_w)).share(m_videoram);
	map(0x1000, 0x10bf).ram().share(m_spriteram);
	map(0x10c0, 0x14ff).ram(); // ???
	map(0x1500, 0x1fff).ram(); // work RAM
	map(0x2000, 0x203f).ram().share(m_scroll_x);
	map(0x2040, 0x2040).nopw(); // ???
	map(0x2041, 0x2041).nopw(); // ???
	map(0x2042, 0x2042).ram().share(m_scroll_dir); // bit 2 = scroll direction
	map(0x2043, 0x2043).nopw(); // ???
	map(0x2044, 0x2044).w(FUNC(jailbrek_state::ctrl_w)); // irq, nmi enable, screen flip
	map(0x3000, 0x3000).w(FUNC(jailbrek_state::coin_w));
	map(0x3100, 0x3100).portr("DSW2").w("snsnd", FUNC(sn76489a_device::write));
	map(0x3200, 0x3200).portr("DSW3").nopw(); // mirror of the previous?
	map(0x3300, 0x3300).portr("SYSTEM").w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x3301, 0x3301).portr("P1");
	map(0x3302, 0x3302).portr("P2");
	map(0x3303, 0x3303).portr("DSW1");
	map(0x4000, 0x4000).w(FUNC(jailbrek_state::speech_w)); // speech pins
	map(0x5000, 0x5000).w(m_vlm, FUNC(vlm5030_device::data_w)); // speech data
	map(0x6000, 0x6000).r(FUNC(jailbrek_state::speech_r));
	map(0x8000, 0xffff).rom();
}

void jailbrek_state::vlm_map(address_map &map)
{
	map.global_mask(0x1fff);
	map(0x0000, 0x1fff).rom();
}



static INPUT_PORTS_START( jailbrek )
	PORT_START("SYSTEM")    // $3300
	KONAMI8_SYSTEM_10
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")        // $3301
	KONAMI8_B12_UNK(1)  // button1 = shoot, button2 = select

	PORT_START("P2")        // $3302
	KONAMI8_B12_UNK(2)

	PORT_START("DSW1")      // $3303
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "Invalid", SW1)
	// "Invalid" = both coin slots disabled

	PORT_START("DSW2")      // $3100
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )       PORT_DIPLOCATION( "SW2:1,2" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )     PORT_DIPLOCATION( "SW2:3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )  PORT_DIPLOCATION( "SW2:4" )
	PORT_DIPSETTING(    0x08, "30K 70K+" )
	PORT_DIPSETTING(    0x00, "40K 80K+" )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Difficulty ) )  PORT_DIPLOCATION( "SW2:5,6" )
	PORT_DIPSETTING(    0x30, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )        PORT_DIPLOCATION( "SW2:7" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION( "SW2:8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")      // $3200
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION( "SW3:1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Upright Controls" )     PORT_DIPLOCATION( "SW3:2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )        PORT_DIPLOCATION( "SW3:3" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )        PORT_DIPLOCATION( "SW3:4" )
INPUT_PORTS_END


static GFXDECODE_START( gfx_jailbrek )
	GFXDECODE_ENTRY( "tiles",   0, gfx_8x8x4_packed_msb,                   0, 16 )
	GFXDECODE_ENTRY( "sprites", 0, gfx_8x8x4_row_2x2_group_packed_msb, 16*16, 16 )
GFXDECODE_END


void jailbrek_state::machine_start()
{
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_nmi_enable));
}

void jailbrek_state::machine_reset()
{
	m_irq_enable = 0;
	m_nmi_enable = 0;
}

void jailbrek_state::jailbrek(machine_config &config)
{
	static constexpr XTAL MASTER_CLOCK = XTAL(18'432'000);
	static constexpr XTAL VOICE_CLOCK = XTAL(3'579'545);

	// basic machine hardware
	KONAMI1(config, m_maincpu, MASTER_CLOCK / 12); // the bootleg uses a standard M6809 with separate decryption logic
	m_maincpu->set_addrmap(AS_PROGRAM, &jailbrek_state::prg_map);
	m_maincpu->set_periodic_int(FUNC(jailbrek_state::interrupt_nmi), attotime::from_hz(500)); // ?

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_jailbrek);
	PALETTE(config, m_palette, FUNC(jailbrek_state::palette), 512, 32);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(MASTER_CLOCK / 3, 396, 8, 248, 256, 16, 240);
	screen.set_screen_update(FUNC(jailbrek_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(jailbrek_state::vblank_irq));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	SN76489A(config, "snsnd", MASTER_CLOCK / 12).add_route(ALL_OUTPUTS, "mono", 1.0);

	VLM5030(config, m_vlm, VOICE_CLOCK);
	m_vlm->add_route(ALL_OUTPUTS, "mono", 1.0);
	m_vlm->set_addrmap(0, &jailbrek_state::vlm_map);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

	/*
	   Check if the ROM used for the speech is not a 2764, but a 27128.  If a
	   27128 is used then the data is stored in the upper half of the EPROM.
	   (The schematics and board refer to a 2764, but all the boards I have seen
	   use a 27128.  According to the schematics pin 26 is tied high so if a 2764
	   is used then the pin is ignored, but if a 27128 is used then pin 26
	   represents address line A13.)
	*/

ROM_START( jailbrek )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "507p03.11d", 0x8000, 0x4000, CRC(a0b88dfd) SHA1(f999e382b9d3b812fca41f4d0da3ea692fef6b19) )
	ROM_LOAD( "507p02.9d",  0xc000, 0x4000, CRC(444b7d8e) SHA1(c708b67c2d249448dae9a3d10c24d13ba6849597) )

	ROM_REGION( 0x08000, "tiles", 0 )
	ROM_LOAD( "507l08.4f",  0x0000, 0x4000, CRC(e3b7a226) SHA1(c19a02a2def65648bf198fccec98ebbd2fc7c0fb) )
	ROM_LOAD( "507j09.5f",  0x4000, 0x4000, CRC(504f0912) SHA1(b51a45dd5506bccdf0061dd6edd7f49ac86ed0f8) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "507j04.3e",  0x0000, 0x4000, CRC(0d269524) SHA1(a10ddb405e884bfec521a3c7a29d22f63e535b59) )
	ROM_LOAD( "507j05.4e",  0x4000, 0x4000, CRC(27d4f6f4) SHA1(c42c064dbd7c5cf0b1d99651367e0bee1728a5b0) )
	ROM_LOAD( "507j06.5e",  0x8000, 0x4000, CRC(717485cb) SHA1(22609489186dcb3d7cd49b7ddfdc6f04d0739354) )
	ROM_LOAD( "507j07.3f",  0xc000, 0x4000, CRC(e933086f) SHA1(c0fd1e8d23c0f7e14c0b75f629448034420cf8ef) )

	ROM_REGION( 0x0240, "proms", 0 )
	ROM_LOAD( "507j10.1f",  0x0000, 0x0020, CRC(f1909605) SHA1(91eaa865375b3bc052897732b64b1ff7df3f78f6) ) // red & green
	ROM_LOAD( "507j11.2f",  0x0020, 0x0020, CRC(f70bb122) SHA1(bf77990260e8346faa3d3481718cbe46a4a27150) ) // blue
	ROM_LOAD( "507j13.7f",  0x0040, 0x0100, CRC(d4fe5c97) SHA1(972e9dab6c53722545dd3a43e3ada7921e88708b) ) // char lookup
	ROM_LOAD( "507j12.6f",  0x0140, 0x0100, CRC(0266c7db) SHA1(a8f21e86e6d974c9bfd92a147689d0e7316d66e2) ) // sprites lookup

	ROM_REGION( 0x4000, "vlm", 0 ) // speech
	ROM_LOAD( "507l01.8c",  0x0000, 0x4000, CRC(0c8a3605) SHA1(d886b66d3861c3a90a1825ccf5bf0011831ca366) ) // same data in both halves
ROM_END

ROM_START( manhatan )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "507n03.11d", 0x8000, 0x4000, CRC(e5039f7e) SHA1(0f12484ed40444d978e0405c27bdd027ae2e2a0b) )
	ROM_LOAD( "507n02.9d",  0xc000, 0x4000, CRC(143cc62c) SHA1(9520dbb1b6f1fa439e03d4caa9bed96ef8f805f2) )

	ROM_REGION( 0x08000, "tiles", 0 )
	ROM_LOAD( "507j08.4f",  0x0000, 0x4000, CRC(175e1b49) SHA1(4cfe982cdf7729bd05c6da803480571876320bf6) )
	ROM_LOAD( "507j09.5f",  0x4000, 0x4000, CRC(504f0912) SHA1(b51a45dd5506bccdf0061dd6edd7f49ac86ed0f8) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "507j04.3e",  0x0000, 0x4000, CRC(0d269524) SHA1(a10ddb405e884bfec521a3c7a29d22f63e535b59) )
	ROM_LOAD( "507j05.4e",  0x4000, 0x4000, CRC(27d4f6f4) SHA1(c42c064dbd7c5cf0b1d99651367e0bee1728a5b0) )
	ROM_LOAD( "507j06.5e",  0x8000, 0x4000, CRC(717485cb) SHA1(22609489186dcb3d7cd49b7ddfdc6f04d0739354) )
	ROM_LOAD( "507j07.3f",  0xc000, 0x4000, CRC(e933086f) SHA1(c0fd1e8d23c0f7e14c0b75f629448034420cf8ef) )

	ROM_REGION( 0x0240, "proms", 0 )
	ROM_LOAD( "507j10.1f",  0x0000, 0x0020, CRC(f1909605) SHA1(91eaa865375b3bc052897732b64b1ff7df3f78f6) ) // red & green
	ROM_LOAD( "507j11.2f",  0x0020, 0x0020, CRC(f70bb122) SHA1(bf77990260e8346faa3d3481718cbe46a4a27150) ) // blue
	ROM_LOAD( "507j13.7f",  0x0040, 0x0100, CRC(d4fe5c97) SHA1(972e9dab6c53722545dd3a43e3ada7921e88708b) ) // char lookup
	ROM_LOAD( "507j12.6f",  0x0140, 0x0100, CRC(0266c7db) SHA1(a8f21e86e6d974c9bfd92a147689d0e7316d66e2) ) // sprites lookup

	ROM_REGION( 0x4000, "vlm", 0 ) // speech
	ROM_LOAD( "507p01.8c",  0x2000, 0x2000, CRC(973fa351) SHA1(ac360d05ed4d03334e00c80e70d5ae939d93af5f) ) // top half is blank
	ROM_CONTINUE( 0x0000, 0x2000 )
ROM_END


/***************************************************************************

Jail Break Bootleg
Hardware Info by Guru

PCB Number: A3001
Note none of the Jail Break bootleg PCBs have numbers/letters along the edge of the PCB.
These have been added so that chip locations can be documented.
  |--------------------------------------------------------------------------------------------------------------|
A |VOL    LM324     LS32    LS04    LS157          PAL16R4  LS669   LS669    LS32    LS86    LS04   LS109        |
  |                                                                                                              |
B |C1182H                   LS32    LS374   LS670   LS374   LS86    LS109    LS08    LS10    LS74   LS04    18MHz|
  |                                                                                                              |
C |                 LS00    LS283   LS174   LS670   LS374   LS86    LS374    LS92    LS11    LS107  LS74    LS32 |
  |                                                                                                              |
D |                 LS07    LS138   DSW3  82S129.D6 LS257   LS244   LS374    LS86    LS161   LS161          LS04 |
  |                                                                                                              |
E |1      LS253     DSW1    LS174   LS367 82S129.E6 LS257   LS374   LS374    LS85    LS161   LS257  6264    LS161|
  |8                                                                                                             |
F |W      LS253     DSW2    LS244   LS14     5.F6           LS374   LS374    LS374   LS161   LS257  6264    LS161|
  |A                       |--------------|                                                                      |
G |Y      LS253     LS08   |    VLM5030   |  4.G6           6264   PAL16R6   LS374   LS161   LS257  6264    LS08 |
  |                        |--------------|                                                                      |
H |       LS253     76489 3.579545  LS175    3.H6   LS10    LS367   LS32     LS283   LS161   LS257  6264    LS32 |
  |                          MHz                                                                                 |
I |      82S123.I2  LS374   LS367   LS139    2.I6           LS244   LS367    LS283   LS32    LS257  LS273   LS257|
  |                                                                                                              |
J |      82S123.J2  LS373           LS138   DIP28           LS244   LS367    LS245   LS04    LS139  LS174   LS257|
  |                                                                                                              |
K | LS74   LS86     LS244  PAL16L8  LS245    1.K6          PAL16L8  LS367    LS273   LS74    LS367  LS157   LS04 |
  |                                |-------------|                                                               |
L | LS74   LS04     LS245   LS273  |   MC6809    |  LS74    LS367   6116     LS245   LS174   LS125  LS175   LS21 |
  |                                |-------------|                                                               |
  |--------------------------------------------------------------------------------------------------------------|
     1       2       3       4       5       6       7       8       9       10      11      12      13      14

Notes: (All ICs shown)
      6809 - Clock input 1.5000MHz [18/12]
   VLM5030 - Sanyo VLM5030 Speech IC. Clock input 3.579545MHz
     76489 - Texas Instruments SN76489A Digital Complex Sound Generator. Clock input 1.5000MHz [18/12]
     HSync - 15.6609kHz
     VSync - 57.5772Hz
      1.K6 - 27C256 (main program)
      2.I6 - 27C128 (speech data for VLM5030)
      3.H6 - 27C256 (background character data)
      4.G6 - 27C256 (sprite data)
      5.F6 - 27C256 (sprite data)
 82S123.I2 - 32x8-bit bipolar PROM (blue color PROM)
 82S123.J2 - 32x8-bit bipolar PROM (red and green color PROM)
 82S129.D6 - 256x4-bit bipolar PROM (character lookup table)
 82S129.E6 - 256x4-bit bipolar PROM (sprite lookup table)
      6264 - 8kBx8-bit SRAM
      6116 - 2kBx8-bit SRAM
    C1182H - NEC uPC1182H Audio Power Amplifier
     LM324 - LM324 Quad Operational Amplifier
     DIP28 - Unpopulated DIP28 position (no socket)
PAL16R6.G9 - sprites not shown when removed
PAL16R4.A7 - black screen when removed, coin/start works and game plays blind
PAL16L8.K8 - no boot when removed, only static startup garbage shown
PAL16L8.K4 - no boot when removed, only static startup garbage shown

***************************************************************************/

ROM_START( jailbrekb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.k6",    0x8000, 0x8000, CRC(df0e8fc7) SHA1(62e59dbb3941ed8af365e96906315318d9aee060) )

	ROM_REGION( 0x08000, "tiles", 0 )
	ROM_LOAD( "3.h6",    0x0000, 0x8000, CRC(bf67a8ff) SHA1(9aca8de7e2c2cc0ff9fe3f316a9300574df4ff06) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "5.f6",    0x0000, 0x8000, CRC(081d2eea) SHA1(dae66b2607d1a56e72e9cb456bdb3c0c21337d6c) )
	ROM_LOAD( "4.g6",    0x8000, 0x8000, CRC(e34b93b8) SHA1(fb6ed12ab017ac1e5006165f435cf0ed95a49c17) )

	ROM_REGION( 0x0240, "proms", 0 )
	ROM_LOAD( "prom.j2", 0x0000, 0x0020, CRC(f1909605) SHA1(91eaa865375b3bc052897732b64b1ff7df3f78f6) ) // red & green
	ROM_LOAD( "prom.i2", 0x0020, 0x0020, CRC(f70bb122) SHA1(bf77990260e8346faa3d3481718cbe46a4a27150) ) // blue
	ROM_LOAD( "prom.d6", 0x0040, 0x0100, CRC(d4fe5c97) SHA1(972e9dab6c53722545dd3a43e3ada7921e88708b) ) // char lookup
	ROM_LOAD( "prom.e6", 0x0140, 0x0100, CRC(0266c7db) SHA1(a8f21e86e6d974c9bfd92a147689d0e7316d66e2) ) // sprites lookup

	ROM_REGION( 0x2000, "vlm", 0 ) // speech
	ROM_LOAD( "2.i6",    0x0000, 0x2000, CRC(d91d15e3) SHA1(475fe50aafbf8f2fb79880ef0e2c25158eda5270) )

	ROM_REGION( 0x800, "plds", 0 )
	ROM_LOAD( "pal16l8.k4", 0x000, 0x104, CRC(96e993c6) SHA1(bd6fd8a039fbf8c890c5ce6cea0fcc7649719b51) )
	ROM_LOAD( "pal16r4.a7", 0x200, 0x104, CRC(3d074375) SHA1(8b2c8143e3540e265213a2d521e350ab71e1b26b) )
	ROM_LOAD( "pal16r6.g9", 0x400, 0x104, CRC(b6c4f22d) SHA1(d445b1c806dd1bcbdd07c9fa8c5483e0d03496aa) )
	ROM_LOAD( "pal16l8.k8", 0x600, 0x104, CRC(38783f49) SHA1(101621b378bb9b5faad7d8e3acdbaa42b5045d45) )
ROM_END

} // anonymous namespace


GAME( 1986, jailbrek,  0,        jailbrek, jailbrek, jailbrek_state, empty_init, ROT0, "Konami",  "Jail Break",                  MACHINE_SUPPORTS_SAVE )
GAME( 1986, jailbrekb, jailbrek, jailbrek, jailbrek, jailbrek_state, empty_init, ROT0, "bootleg", "Jail Break (bootleg)",        MACHINE_SUPPORTS_SAVE )
GAME( 1986, manhatan,  jailbrek, jailbrek, jailbrek, jailbrek_state, empty_init, ROT0, "Konami",  "Manhattan 24 Bunsyo (Japan)", MACHINE_SUPPORTS_SAVE )

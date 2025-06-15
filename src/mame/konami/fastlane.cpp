// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

    Fast Lane (GX752) (c) 1987 Konami

    Driver by Manuel Abadia <emumanu+mame@gmail.com>

    TODO:
        - verify that sound is correct (volume and bank switching)

***************************************************************************/

#include "emu.h"

#include "konamipt.h"
#include "k007121.h"
#include "k051733.h"

#include "cpu/m6809/hd6309.h"
#include "machine/watchdog.h"
#include "sound/k007232.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class fastlane_state : public driver_device
{
public:
	fastlane_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_videoram(*this, "videoram%u", 1U),
		m_spriteram(*this, "spriteram"),
		m_prgbank(*this, "prgbank"),
		m_k007232(*this, "k007232_%u", 1U),
		m_k007121(*this, "k007121"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	void fastlane(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	// memory pointers
	required_shared_ptr_array<uint8_t, 2> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_memory_bank m_prgbank;

	// video-related
	tilemap_t *m_tilemap[2];

	// devices
	required_device_array<k007232_device, 2> m_k007232;
	required_device<k007121_device> m_k007121;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	void palette(palette_device &palette) const;

	template <uint8_t Which> TILE_GET_INFO_MEMBER(get_tile_info);
	template <uint8_t Which> void vram_w(offs_t offset, uint8_t data);
	void flipscreen_w(int state) { machine().tilemap().set_flip_all(state ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0); }
	void dirtytiles() { machine().tilemap().mark_all_dirty(); }

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void bankswitch_w(uint8_t data);
	template <uint8_t Which> uint8_t k007232_r(offs_t offset);
	template <uint8_t Which> void k007232_w(offs_t offset, uint8_t data);
	template <uint8_t Which> void volume_callback(uint8_t data);

	void prg_map(address_map &map) ATTR_COLD;
};


void fastlane_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	for (int pal = 0; pal < 0x10; pal++)
	{
		for (int i = 0; i < 0x400; i++)
		{
			uint8_t const ctabentry = (i & 0x3f0) | color_prom[(pal << 4) | (i & 0x0f)];
			palette.set_pen_indirect((pal << 10) | i, ctabentry);
		}
	}
}


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

template <uint8_t Which>
TILE_GET_INFO_MEMBER(fastlane_state::get_tile_info)
{
	uint8_t ctrl_3 = m_k007121->ctrl_r(3);
	uint8_t ctrl_4 = m_k007121->ctrl_r(4);
	uint8_t ctrl_5 = m_k007121->ctrl_r(5);
	int attr = m_videoram[Which][tile_index];
	int code = m_videoram[Which][tile_index + 0x400];
	int bit0 = (ctrl_5 >> 0) & 0x03;
	int bit1 = (ctrl_5 >> 2) & 0x03;
	int bit2 = (ctrl_5 >> 4) & 0x03;
	int bit3 = (ctrl_5 >> 6) & 0x03;
	int bank = ((attr >> (bit0 + 3)) & 0x01) |
			((attr >> (bit1 + 2)) & 0x02) |
			((attr >> (bit2 + 1)) & 0x04) |
			((attr >> (bit3 + 0)) & 0x08);
	int mask = (ctrl_4 & 0xf0) >> 4;
	bank = (bank & ~mask) | (ctrl_4 & mask);
	bank = ((attr & 0x80) >> 7) | (bank << 1) | ((ctrl_3 & 0x01) << 5);

	tileinfo.set(0,
			code + bank * 256,
			1 + 64 * (attr & 0x0f),
			0);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void fastlane_state::video_start()
{
	m_k007121->set_spriteram(m_spriteram);

	m_tilemap[0] = &machine().tilemap().create(*m_k007121, tilemap_get_info_delegate(*this, FUNC(fastlane_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_k007121, tilemap_get_info_delegate(*this, FUNC(fastlane_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_tilemap[1]->set_transparent_pen(0);
}


/***************************************************************************

  Memory Handlers

***************************************************************************/

template <uint8_t Which>
void fastlane_state::vram_w(offs_t offset, uint8_t data)
{
	m_videoram[Which][offset] = data;
	m_tilemap[Which]->mark_tile_dirty(offset & 0x3ff);
}


/***************************************************************************

  Screen Refresh

***************************************************************************/

uint32_t fastlane_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x10, cliprect);

	// compute clipping
	rectangle clip[2];
	clip[0] = clip[1] = screen.visible_area();

	if (m_k007121->flipscreen())
	{
		clip[0].max_x -= 40;
		clip[1].min_x = clip[1].max_x - 39;
	}
	else
	{
		clip[0].min_x += 40;
		clip[1].max_x = 39;
	}

	clip[0] &= cliprect;
	clip[1] &= cliprect;

	// set scroll registers
	m_tilemap[0]->set_scrollx(0, m_k007121->ctrl_r(0) - 40);
	m_tilemap[0]->set_scrolly(0, m_k007121->ctrl_r(2));

	// draw the graphics
	m_tilemap[0]->draw(screen, bitmap, clip[0], 0, 0);
	m_k007121->sprites_draw(bitmap, clip[0], 0, m_k007121->flipscreen() ? 16 : 40, 0, screen.priority(), (uint32_t)-1);
	m_tilemap[1]->draw(screen, bitmap, clip[1], 0, 0);

	return 0;
}


void fastlane_state::bankswitch_w(uint8_t data)
{
	// bits 0 & 1 coin counters
	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	machine().bookkeeping().coin_counter_w(1, data & 0x02);

	// bits 2 & 3 = bank number
	m_prgbank->set_entry((data & 0x0c) >> 2);

	// bit 4: bank # for the 007232 (chip 2)
	m_k007232[1]->set_bank(0 + ((data & 0x10) >> 4), 2 + ((data & 0x10) >> 4));

	// other bits seems to be unused
}

// Read and write handlers for one K007232 chip: even and odd register are mapped swapped

template <uint8_t Which>
uint8_t fastlane_state::k007232_r(offs_t offset)
{
	return m_k007232[Which]->read(offset ^ 1);
}

template <uint8_t Which>
void fastlane_state::k007232_w(offs_t offset, uint8_t data)
{
	m_k007232[Which]->write(offset ^ 1, data);
}

void fastlane_state::prg_map(address_map &map)
{
	map(0x0000, 0x0007).w(m_k007121, FUNC(k007121_device::ctrl_w));
	map(0x0020, 0x005f).rw(m_k007121, FUNC(k007121_device::scroll_r), FUNC(k007121_device::scroll_w));
	map(0x0800, 0x0800).portr("DSW3");
	map(0x0801, 0x0801).portr("P2");
	map(0x0802, 0x0802).portr("P1");
	map(0x0803, 0x0803).portr("SYSTEM");
	map(0x0900, 0x0900).portr("DSW1");
	map(0x0901, 0x0901).portr("DSW2");
	map(0x0b00, 0x0b00).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x0c00, 0x0c00).w(FUNC(fastlane_state::bankswitch_w));
	map(0x0d00, 0x0d0d).rw(FUNC(fastlane_state::k007232_r<0>), FUNC(fastlane_state::k007232_w<0>));
	map(0x0e00, 0x0e0d).rw(FUNC(fastlane_state::k007232_r<1>), FUNC(fastlane_state::k007232_w<1>));
	map(0x0f00, 0x0f1f).rw("k051733", FUNC(k051733_device::read), FUNC(k051733_device::write)); // protection
	map(0x1000, 0x17ff).ram().w(m_palette, FUNC(palette_device::write_indirect)).share("palette");
	map(0x1800, 0x1fff).ram(); // Work RAM
	map(0x2000, 0x27ff).ram().w(FUNC(fastlane_state::vram_w<0>)).share(m_videoram[0]);
	map(0x2800, 0x2fff).ram().w(FUNC(fastlane_state::vram_w<1>)).share(m_videoram[1]);
	map(0x3000, 0x3fff).ram().share(m_spriteram);
	map(0x4000, 0x7fff).bankr(m_prgbank);
	map(0x8000, 0xffff).rom().region("maincpu", 0);
}

/***************************************************************************

    Input Ports

***************************************************************************/

// verified from HD6309 code
static INPUT_PORTS_START( fastlane )
	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	// "No Coin B" = coins produce sound, but no effect on coin counter

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	// The bonus life affects the starting high score too, 20000 or 30000
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x18, "20k 100k 200k 400k 800k" )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x10, "30k 150k 300k 600k" )
	PORT_DIPSETTING(    0x08, "20k only" )
	PORT_DIPSETTING(    0x00, "30k only" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:8") // seems it doesn't work (same on PCB)
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Upright Controls" )          PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, "3 Times" )
	PORT_DIPSETTING(    0x00, DEF_STR( Infinite ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	KONAMI8_SYSTEM_UNK

	PORT_START("P1")
	KONAMI8_B12_UNK(1)

	PORT_START("P2")
	KONAMI8_B12_UNK(2)
INPUT_PORTS_END

static GFXDECODE_START( gfx_fastlane )
	GFXDECODE_ENTRY( "gfx", 0, gfx_8x8x4_packed_msb, 0, 64*16 )
GFXDECODE_END

/***************************************************************************

    Machine Driver

***************************************************************************/

template <uint8_t Which>
void fastlane_state::volume_callback(uint8_t data)
{
	m_k007232[Which]->set_volume(0, (data >> 4) * 0x11, 0);
	m_k007232[Which]->set_volume(1, 0, (data & 0x0f) * 0x11);
}

void fastlane_state::machine_start()
{
	uint8_t *ROM = memregion("maincpu")->base();

	m_prgbank->configure_entries(0, 4, &ROM[0x8000], 0x4000);
}

void fastlane_state::fastlane(machine_config &config)
{
	// basic machine hardware
	HD6309E(config, m_maincpu, 24_MHz_XTAL / 8); // HD63C09EP, 3 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &fastlane_state::prg_map);

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(24_MHz_XTAL / 3, 512, 0, 280, 264, 16, 240);
	m_screen->set_screen_update(FUNC(fastlane_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(fastlane_state::palette)).set_format(palette_device::xBGR_555, 1024*16, 0x400);

	K007121(config, m_k007121, 0, gfx_fastlane, m_palette, m_screen);
	m_k007121->set_irq_cb().set_inputline(m_maincpu, HD6309_IRQ_LINE);
	m_k007121->set_nmi_cb().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_k007121->set_flipscreen_cb().set(FUNC(fastlane_state::flipscreen_w));
	m_k007121->set_dirtytiles_cb(FUNC(fastlane_state::dirtytiles));

	K051733(config, "k051733", 0);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	K007232(config, m_k007232[0], 3.579545_MHz_XTAL);
	m_k007232[0]->port_write().set(FUNC(fastlane_state::volume_callback<0>));
	m_k007232[0]->add_route(0, "mono", 0.50);
	m_k007232[0]->add_route(1, "mono", 0.50);

	K007232(config, m_k007232[1], 3.579545_MHz_XTAL);
	m_k007232[1]->port_write().set(FUNC(fastlane_state::volume_callback<1>));
	m_k007232[1]->add_route(0, "mono", 0.50);
	m_k007232[1]->add_route(1, "mono", 0.50);
}


/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( fastlane )
	ROM_REGION( 0x18000, "maincpu", 0 ) // code + banked ROMs
	ROM_LOAD( "752_m02.9h",  0x00000, 0x08000, CRC(e1004489) SHA1(615b608d22abc3611f1620503cd6a8c9a6218db8) )  // fixed ROM
	ROM_LOAD( "752_e01.10h", 0x08000, 0x10000, CRC(ff4d6029) SHA1(b5c5d8654ce728300d268628bd3dd878570ba7b8) )  // banked ROM

	ROM_REGION( 0x80000, "gfx", 0 )
	ROM_LOAD16_WORD_SWAP( "752e04.2i",   0x00000, 0x80000, CRC(a126e82d) SHA1(6663230c2c36dec563969bccad8c62e3d454d240) )  // tiles + sprites

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "752e03.6h",   0x0000, 0x0100, CRC(44300aeb) SHA1(580c6e88cbb3b6d8156ea0b9103834f199ec2747) )

	ROM_REGION( 0x20000, "k007232_1", 0 )
	ROM_LOAD( "752e06.4c",   0x00000, 0x20000, CRC(85d691ed) SHA1(7f8d05562a68c75672141fc80ce7e7acb80588b9) )

	ROM_REGION( 0x80000, "k007232_2", 0 )
	ROM_LOAD( "752e05.12b",  0x00000, 0x80000, CRC(119e9cbf) SHA1(21e3def9ab10b210632df11b6df4699140c473db) )
ROM_END

} // anonymous namespace


GAME( 1987, fastlane, 0, fastlane, fastlane, fastlane_state, empty_init, ROT90, "Konami", "Fast Lane", MACHINE_SUPPORTS_SAVE )

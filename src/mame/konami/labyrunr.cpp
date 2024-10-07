// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Labyrinth Runner (GX771) (c) 1987 Konami

    similar to Fast Lane

    Driver by Nicola Salmoria

***************************************************************************/

#include "emu.h"

#include "konamipt.h"
#include "k007121.h"
#include "k051733.h"

#include "cpu/m6809/hd6309.h"
#include "machine/watchdog.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class labyrunr_state : public driver_device
{
public:
	labyrunr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_k007121(*this, "k007121"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_scrollram(*this, "scrollram"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram%u", 1U),
		m_mainbank(*this, "mainbank")
	{ }

	void labyrunr(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<k007121_device> m_k007121;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	// memory pointers
	required_shared_ptr<uint8_t> m_scrollram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr_array<uint8_t, 2> m_videoram;
	required_memory_bank m_mainbank;

	// video-related
	tilemap_t *m_layer[2]{};
	rectangle m_clip[2]{};

	void bankswitch_w(uint8_t data);
	template <uint8_t Which> void vram_w(offs_t offset, uint8_t data);
	template <uint8_t Which> TILE_GET_INFO_MEMBER(get_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(int state);
	INTERRUPT_GEN_MEMBER(timer_interrupt);
	void prg_map(address_map &map) ATTR_COLD;
};


void labyrunr_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	for (int pal = 0; pal < 8; pal++)
	{
		if (pal & 1)
		{
			// chars, no lookup table
			for (int i = 0; i < 0x100; i++)
				palette.set_pen_indirect((pal << 8) | i, (pal << 4) | (i & 0x0f));
		}
		else
		{
			// sprites
			for (int i = 0; i < 0x100; i++)
			{
				uint8_t const ctabentry = !color_prom[i] ? 0 : ((pal << 4) | (color_prom[i] & 0x0f));

				palette.set_pen_indirect((pal << 8) | i, ctabentry);
			}
		}
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

template <uint8_t Which>
TILE_GET_INFO_MEMBER(labyrunr_state::get_tile_info)
{
	uint8_t ctrl_3 = m_k007121->ctrlram_r(3);
	uint8_t ctrl_4 = m_k007121->ctrlram_r(4);
	uint8_t ctrl_5 = m_k007121->ctrlram_r(5);
	uint8_t ctrl_6 = m_k007121->ctrlram_r(6);
	int attr = m_videoram[Which][tile_index];
	int code = m_videoram[Which][tile_index + 0x400];
	int bit0 = (ctrl_5 >> 0) & 0x03;
	int bit1 = (ctrl_5 >> 2) & 0x03;
	int bit2 = (ctrl_5 >> 4) & 0x03;
	int bit3 = (ctrl_5 >> 6) & 0x03;
	int bank = ((attr & 0x80) >> 7) |
			((attr >> (bit0 + 2)) & 0x02) |
			((attr >> (bit1 + 1)) & 0x04) |
			((attr >> (bit2    )) & 0x08) |
			((attr >> (bit3 - 1)) & 0x10) |
			((ctrl_3 & 0x01) << 5);
	int mask = (ctrl_4 & 0xf0) >> 4;

	bank = (bank & ~(mask << 1)) | ((ctrl_4 & mask) << 1);

	tileinfo.set(0,
			code + bank * 256,
			((ctrl_6 & 0x30) * 2 + 16) + (attr & 7),
			0);
	tileinfo.category = Which ? (attr & 0x40) >> 6 : 0;
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void labyrunr_state::video_start()
{
	m_layer[0] = &machine().tilemap().create(*m_k007121, tilemap_get_info_delegate(*this, FUNC(labyrunr_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_layer[1] = &machine().tilemap().create(*m_k007121, tilemap_get_info_delegate(*this, FUNC(labyrunr_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_layer[0]->set_transparent_pen(0);
	m_layer[1]->set_transparent_pen(0);

	m_clip[0] = m_screen->visible_area();
	m_clip[0].min_x += 40;

	m_clip[1] = m_screen->visible_area();
	m_clip[1].max_x = 39;
	m_clip[1].min_x = 0;

	m_layer[0]->set_scroll_cols(32);
}



/***************************************************************************

  Memory Handlers

***************************************************************************/

template <uint8_t Which>
void labyrunr_state::vram_w(offs_t offset, uint8_t data)
{
	m_videoram[Which][offset] = data;
	m_layer[Which]->mark_tile_dirty(offset & 0x3ff);
}



/***************************************************************************

  Screen Refresh

***************************************************************************/

uint32_t labyrunr_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t ctrl_0 = m_k007121->ctrlram_r(0);
	rectangle finalclip0, finalclip1;

	screen.priority().fill(0, cliprect);
	bitmap.fill(m_palette->black_pen(), cliprect);

	if (~m_k007121->ctrlram_r(3) & 0x20)
	{
		finalclip0 = m_clip[0];
		finalclip1 = m_clip[1];

		finalclip0 &= cliprect;
		finalclip1 &= cliprect;

		m_layer[0]->set_scrollx(0, ctrl_0 - 40);
		m_layer[1]->set_scrollx(0, 0);

		for (int i = 0; i < 32; i++)
		{
			// enable colscroll
			if ((m_k007121->ctrlram_r(1) & 6) == 6) // it's probably just one bit, but it's only used once in the game so I don't know which it's
				m_layer[0]->set_scrolly((i + 2) & 0x1f, m_k007121->ctrlram_r(2) + m_scrollram[i]);
			else
				m_layer[0]->set_scrolly((i + 2) & 0x1f, m_k007121->ctrlram_r(2));
		}

		m_layer[0]->draw(screen, bitmap, finalclip0, TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_CATEGORY(0), 0);
		m_k007121->sprites_draw(bitmap, cliprect, m_spriteram, (m_k007121->ctrlram_r(6) & 0x30) * 2, 40, 0, screen.priority(), (m_k007121->ctrlram_r(3) & 0x40) >> 5);
		m_layer[0]->draw(screen, bitmap, finalclip0, TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_CATEGORY(1), 0);
		// we ignore the transparency because layer1 is drawn only at the top of the screen also covering sprites
		m_layer[1]->draw(screen, bitmap, finalclip1, TILEMAP_DRAW_OPAQUE, 0);
	}
	else
	{
		int use_clip3[2] = { 0, 0 };
		rectangle finalclip3;

		// custom cliprects needed for the weird effect used in the ending sequence to hide and show the needed part of text
		finalclip0.min_y = finalclip1.min_y = cliprect.min_y;
		finalclip0.max_y = finalclip1.max_y = cliprect.max_y;

		if (m_k007121->ctrlram_r(1) & 1)
		{
			finalclip0.min_x = cliprect.max_x - ctrl_0 + 8;
			finalclip0.max_x = cliprect.max_x;

			if (ctrl_0 >= 40)
			{
				finalclip1.min_x = cliprect.min_x;
			}
			else
			{
				use_clip3[0] = 1;

				finalclip1.min_x = 40 - ctrl_0;
			}

			finalclip1.max_x = cliprect.max_x - ctrl_0 + 8;

		}
		else
		{
			if (ctrl_0 >= 40)
			{
				finalclip0.min_x = cliprect.min_x;
			}
			else
			{
				use_clip3[1] = 1;

				finalclip0.min_x = 40 - ctrl_0;
			}

			finalclip0.max_x = cliprect.max_x - ctrl_0 + 8;

			finalclip1.min_x = cliprect.max_x - ctrl_0 + 8;
			finalclip1.max_x = cliprect.max_x;
		}

		if (use_clip3[0] || use_clip3[1])
		{
			finalclip3.min_y = cliprect.min_y;
			finalclip3.max_y = cliprect.max_y;
			finalclip3.min_x = cliprect.min_x;
			finalclip3.max_x = 40 - ctrl_0 - 8;
		}

		m_layer[0]->set_scrollx(0, ctrl_0 - 40);
		m_layer[1]->set_scrollx(0, ctrl_0 - 40);

		m_layer[0]->draw(screen, bitmap, finalclip0, TILEMAP_DRAW_CATEGORY(0), 0);
		if (use_clip3[0])
			m_layer[0]->draw(screen, bitmap, finalclip3, TILEMAP_DRAW_CATEGORY(0), 0);

		m_k007121->sprites_draw(bitmap, cliprect, m_spriteram, (m_k007121->ctrlram_r(6) & 0x30) * 2,40,0,screen.priority(),(m_k007121->ctrlram_r(3) & 0x40) >> 5);

		m_layer[0]->draw(screen, bitmap, finalclip0, TILEMAP_DRAW_CATEGORY(1), 0);
		if (use_clip3[0])
			m_layer[0]->draw(screen, bitmap, finalclip3, TILEMAP_DRAW_CATEGORY(1), 0);

		m_layer[1]->draw(screen, bitmap, finalclip1, 0, 0);
		if (use_clip3[1])
			m_layer[1]->draw(screen, bitmap, finalclip3, 0, 0);

	}
	return 0;
}


void labyrunr_state::vblank_irq(int state)
{
	if (state && (m_k007121->ctrlram_r(7) & 0x02))
		m_maincpu->set_input_line(HD6309_IRQ_LINE, HOLD_LINE);
}

INTERRUPT_GEN_MEMBER(labyrunr_state::timer_interrupt)
{
	if (m_k007121->ctrlram_r(7) & 0x01)
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


void labyrunr_state::bankswitch_w(uint8_t data)
{
	if (data & 0xe0) logerror("bankswitch %02x", data);

	// bits 0-2 = bank number
	m_mainbank->set_entry(data & 0x07);   // shall we check if data & 7 > #banks?

	// bits 3 and 4 are coin counters
	machine().bookkeeping().coin_counter_w(0, data & 0x08);
	machine().bookkeeping().coin_counter_w(1, data & 0x10);
}

void labyrunr_state::prg_map(address_map &map)
{
	map(0x0000, 0x0007).w(m_k007121, FUNC(k007121_device::ctrl_w));
	map(0x0020, 0x005f).ram().share(m_scrollram);
	map(0x0800, 0x0800).rw("ym1", FUNC(ym2203_device::data_r), FUNC(ym2203_device::data_w));
	map(0x0801, 0x0801).rw("ym1", FUNC(ym2203_device::status_r), FUNC(ym2203_device::address_w));
	map(0x0900, 0x0900).rw("ym2", FUNC(ym2203_device::data_r), FUNC(ym2203_device::data_w));
	map(0x0901, 0x0901).rw("ym2", FUNC(ym2203_device::status_r), FUNC(ym2203_device::address_w));
	map(0x0a00, 0x0a00).portr("P2");
	map(0x0a01, 0x0a01).portr("P1");
	map(0x0b00, 0x0b00).portr("SYSTEM");
	map(0x0c00, 0x0c00).w(FUNC(labyrunr_state::bankswitch_w));
	map(0x0d00, 0x0d1f).rw("k051733", FUNC(k051733_device::read), FUNC(k051733_device::write));
	map(0x0e00, 0x0e00).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x1000, 0x10ff).ram().w(m_palette, FUNC(palette_device::write_indirect)).share("palette");
	map(0x1800, 0x1fff).ram();
	map(0x2000, 0x2fff).ram().share(m_spriteram);
	map(0x3000, 0x37ff).ram().w(FUNC(labyrunr_state::vram_w<0>)).share(m_videoram[0]);
	map(0x3800, 0x3fff).ram().w(FUNC(labyrunr_state::vram_w<1>)).share(m_videoram[1]);
	map(0x4000, 0x7fff).bankr(m_mainbank);
	map(0x8000, 0xffff).rom();
}


/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( labyrunr )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	KONAMI8_MONO_B12_START

	PORT_START("P2")
	KONAMI8_COCKTAIL_B12_START

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), DEF_STR( None ), SW1)
	// "None" = coin slot B disabled

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "30000 70000" )
	PORT_DIPSETTING(    0x10, "40000 80000" )
	PORT_DIPSETTING(    0x08, "40000" )
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:8")
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
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, "3 Times" )
	PORT_DIPSETTING(    0x00, "5 Times" )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



static GFXDECODE_START( gfx_labyrunr )
	GFXDECODE_ENTRY( "gfx", 0, gfx_8x8x4_packed_msb, 0, 8*16 )
GFXDECODE_END

/***************************************************************************

    Machine Driver

***************************************************************************/

void labyrunr_state::machine_start()
{
	uint8_t *ROM = memregion("maincpu")->base();

	m_mainbank->configure_entries(0, 6, &ROM[0x10000], 0x4000);
}

void labyrunr_state::labyrunr(machine_config &config)
{
	// basic machine hardware
	HD6309E(config, m_maincpu, 24_MHz_XTAL / 8); // HD63C09EP
	m_maincpu->set_addrmap(AS_PROGRAM, &labyrunr_state::prg_map);
	m_maincpu->set_periodic_int(FUNC(labyrunr_state::timer_interrupt), attotime::from_hz(4 * 60));

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(37*8, 32*8);
	screen.set_visarea(0*8, 35*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(labyrunr_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(labyrunr_state::vblank_irq));

	PALETTE(config, m_palette, FUNC(labyrunr_state::palette));
	m_palette->set_format(palette_device::xBGR_555, 2*8*16*16, 128);

	K007121(config, m_k007121, 0, m_palette, gfx_labyrunr);

	K051733(config, "k051733", 0);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2203_device &ym1(YM2203(config, "ym1", 3000000));
	ym1.port_a_read_callback().set_ioport("DSW1");
	ym1.port_b_read_callback().set_ioport("DSW2");
	ym1.add_route(0, "mono", 0.40);
	ym1.add_route(1, "mono", 0.40);
	ym1.add_route(2, "mono", 0.40);
	ym1.add_route(3, "mono", 0.80);

	ym2203_device &ym2(YM2203(config, "ym2", 3000000));
	ym2.port_b_read_callback().set_ioport("DSW3");
	ym2.add_route(0, "mono", 0.40);
	ym2.add_route(1, "mono", 0.40);
	ym2.add_route(2, "mono", 0.40);
	ym2.add_route(3, "mono", 0.80);
}


/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( tricktrp )
	ROM_REGION( 0x28000, "maincpu", 0 ) // code + banked ROMs
	ROM_LOAD( "771e04",     0x10000, 0x08000, CRC(ba2c7e20) SHA1(713dcc0e65bf9431f2c0df9db1210346a9476a52) )
	ROM_CONTINUE(           0x08000, 0x08000 )
	ROM_LOAD( "771e03",     0x18000, 0x10000, CRC(d0d68036) SHA1(8589ee07e229259341a4cc22bc64de8f06536472) )

	ROM_REGION( 0x40000, "gfx", 0 )
	ROM_LOAD16_BYTE( "771e01a", 0x00001, 0x10000, CRC(103ffa0d) SHA1(1949c49ca3b243e4cfb5fb19ecd3a1e1492cfddd) )    // tiles + sprites
	ROM_LOAD16_BYTE( "771e01c", 0x00000, 0x10000, CRC(cfec5be9) SHA1(2b6a32e2608a70c47d1ec9b4de38b5c3a0898cde) )
	ROM_LOAD16_BYTE( "771d01b", 0x20001, 0x10000, CRC(07f2a71c) SHA1(63c79e75e71539e69d4d9d35e629a6021124f6d0) )
	ROM_LOAD16_BYTE( "771d01d", 0x20000, 0x10000, CRC(f6810a49) SHA1(b40e9f0d0919188a05c1990347da8dc8ff12d65a) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "771d02.08d", 0x0000, 0x0100, CRC(3d34bb5a) SHA1(3f3c845f1197457244e7c7e4f9b2a03c278613e4) )  // sprite lookup table
															// there is no char lookup table
ROM_END

ROM_START( labyrunr )
	ROM_REGION( 0x28000, "maincpu", 0 ) // code + banked ROMs
	ROM_LOAD( "771j04.10f", 0x10000, 0x08000, CRC(354a41d0) SHA1(302e8f5c469ad3f615aeca8005ebde6b6051aaae) )
	ROM_CONTINUE(           0x08000, 0x08000 )
	ROM_LOAD( "771j03.08f", 0x18000, 0x10000, CRC(12b49044) SHA1(e9b22fb093cfb746a9767e94ef5deef98bed5b7a) )

	ROM_REGION( 0x40000, "gfx", 0 )
	ROM_LOAD16_WORD_SWAP( "771d01.14a", 0x00000, 0x40000, CRC(15c8f5f9) SHA1(e4235e1315d0331f3ce5047834a68764ed43aa4b) )    // tiles + sprites

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "771d02.08d", 0x0000, 0x0100, CRC(3d34bb5a) SHA1(3f3c845f1197457244e7c7e4f9b2a03c278613e4) )  // sprite lookup table
															// there is no char lookup table
ROM_END

ROM_START( labyrunrk )
	ROM_REGION( 0x28000, "maincpu", 0 ) // code + banked ROMs
	ROM_LOAD( "771k04.10f", 0x10000, 0x08000, CRC(9816ab35) SHA1(6efb0332f4a62f20889f212682ee7225e4a182a9) )
	ROM_CONTINUE(           0x08000, 0x08000 )
	ROM_LOAD( "771k03.8f",  0x18000, 0x10000, CRC(48d732ae) SHA1(8bc7917397f32cf5f995b3763ae921725e27de05) )

	ROM_REGION( 0x40000, "gfx", 0 )
	ROM_LOAD16_BYTE( "771d01a.13a", 0x00001, 0x10000, CRC(0cd1ed1a) SHA1(eac6c106de28acc54535ae1fb99f778c1ed4013e) )    // tiles + sprites
	ROM_LOAD16_BYTE( "771d01c.13a", 0x00000, 0x10000, CRC(d75521fe) SHA1(72f0c4d9511bc70d77415f50be93293026305bd5) )
	ROM_LOAD16_BYTE( "771d01b",     0x20001, 0x10000, CRC(07f2a71c) SHA1(63c79e75e71539e69d4d9d35e629a6021124f6d0) )
	ROM_LOAD16_BYTE( "771d01d",     0x20000, 0x10000, CRC(f6810a49) SHA1(b40e9f0d0919188a05c1990347da8dc8ff12d65a) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "771d02.08d", 0x0000, 0x0100, CRC(3d34bb5a) SHA1(3f3c845f1197457244e7c7e4f9b2a03c278613e4) )  // sprite lookup table
															// there is no char lookup table
ROM_END

} // anonymous namespace


GAME( 1987, tricktrp,  0,        labyrunr, labyrunr, labyrunr_state, empty_init, ROT90, "Konami", "Trick Trap (World?)",             MACHINE_SUPPORTS_SAVE )
GAME( 1987, labyrunr,  tricktrp, labyrunr, labyrunr, labyrunr_state, empty_init, ROT90, "Konami", "Labyrinth Runner (Japan)",        MACHINE_SUPPORTS_SAVE )
GAME( 1987, labyrunrk, tricktrp, labyrunr, labyrunr, labyrunr_state, empty_init, ROT90, "Konami", "Labyrinth Runner (World Ver. K)", MACHINE_SUPPORTS_SAVE )

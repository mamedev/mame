// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Surprise Attack (Konami GX911) (c) 1990 Konami

    Very similar to Parodius

    driver by Nicola Salmoria

***************************************************************************/

#include "emu.h"

#include "konamipt.h"
#include "k052109.h"
#include "k053244_k053245.h"
#include "k053251.h"
#include "konami_helper.h"

#include "cpu/m6809/konami.h"
#include "machine/bankdev.h"
#include "machine/watchdog.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class surpratk_state : public driver_device
{
public:
	surpratk_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_bank0000(*this, "bank0000"),
		m_k052109(*this, "k052109"),
		m_k053244(*this, "k053244"),
		m_k053251(*this, "k053251"),
		m_palette(*this, "palette"),
		m_mainbank(*this, "mainbank")
	{ }

	void surpratk(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// video-related
	uint8_t m_layer_colorbase[3]{};
	uint8_t m_sprite_colorbase = 0;
	int m_layerpri[3]{};

	// devices
	required_device<konami_cpu_device> m_maincpu;
	required_device<address_map_bank_device> m_bank0000;
	required_device<k052109_device> m_k052109;
	required_device<k05324x_device> m_k053244;
	required_device<k053251_device> m_k053251;
	required_device<palette_device> m_palette;

	required_memory_bank m_mainbank;

	void videobank_w(uint8_t data);
	void _5fc0_w(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	K05324X_CB_MEMBER(sprite_callback);
	K052109_CB_MEMBER(tile_callback);
	void banking_callback(uint8_t data);
	void bank0000_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

K052109_CB_MEMBER(surpratk_state::tile_callback)
{
	*flags = (*color & 0x80) ? TILE_FLIPX : 0;
	*code |= ((*color & 0x03) << 8) | ((*color & 0x10) << 6) | ((*color & 0x0c) << 9) | (bank << 13);
	*color = m_layer_colorbase[layer] + ((*color & 0x60) >> 5);
}

/***************************************************************************

  Callbacks for the K053245

***************************************************************************/

K05324X_CB_MEMBER(surpratk_state::sprite_callback)
{
	int pri = 0x20 | ((*color & 0x60) >> 2);
	if (pri <= m_layerpri[2])
		*priority = 0;
	else if (pri > m_layerpri[2] && pri <= m_layerpri[1])
		*priority = 0xf0;
	else if (pri > m_layerpri[1] && pri <= m_layerpri[0])
		*priority = 0xf0 | 0xcc;
	else
		*priority = 0xf0 | 0xcc | 0xaa;

	*color = m_sprite_colorbase + (*color & 0x1f);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

uint32_t surpratk_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// update color info and refresh tilemaps
	static const int K053251_CI[3] = { k053251_device::CI2, k053251_device::CI4, k053251_device::CI3 };
	int bg_colorbase = m_k053251->get_palette_index(k053251_device::CI0);
	m_sprite_colorbase = m_k053251->get_palette_index(k053251_device::CI1);

	for (int i = 0; i < 3; i++)
	{
		int prev_colorbase = m_layer_colorbase[i];
		m_layer_colorbase[i] = m_k053251->get_palette_index(K053251_CI[i]);

		if (m_layer_colorbase[i] != prev_colorbase)
			m_k052109->mark_tilemap_dirty(i);
	}

	m_k052109->tilemap_update();

	// sort layers and draw
	int layer[3];
	for (int i = 0; i < 3; i++)
	{
		layer[i] = i;
		m_layerpri[i] = m_k053251->get_priority(K053251_CI[i]);
	}

	konami_sortlayers3(layer, m_layerpri);

	screen.priority().fill(0, cliprect);
	bitmap.fill(16 * bg_colorbase, cliprect);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, layer[0], 0, 1);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 2);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 4);

	m_k053244->sprites_draw(bitmap, cliprect, screen.priority());
	return 0;
}


void surpratk_state::videobank_w(uint8_t data)
{
	if (data & 0xf8)
		logerror("%s: videobank = %02x\n", machine().describe_context(), data);

	// bit 0 = select 053245 at 0000-07ff
	// bit 1 = select palette at 0000-07ff
	// bit 2 = select palette bank 0 or 1
	if (BIT(data, 1))
		m_bank0000->set_bank(2 + BIT(data, 2));
	else
		m_bank0000->set_bank(BIT(data, 0));
}

void surpratk_state::_5fc0_w(uint8_t data)
{
	if ((data & 0xf4) != 0x10)
		logerror("%04x: 3fc0 = %02x\n", m_maincpu->pc(), data);

	// bit 0/1 = coin counters
	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	machine().bookkeeping().coin_counter_w(1, data & 0x02);

	// bit 3 = enable char ROM reading through the video RAM
	m_k052109->set_rmrd_line((data & 0x08) ? ASSERT_LINE : CLEAR_LINE);

	// other bits unknown
}


/********************************************/

void surpratk_state::main_map(address_map &map)
{
	map(0x0000, 0x07ff).m(m_bank0000, FUNC(address_map_bank_device::amap8));
	map(0x0800, 0x1fff).ram();
	map(0x2000, 0x3fff).bankr(m_mainbank);
	map(0x4000, 0x7fff).rw(m_k052109, FUNC(k052109_device::read), FUNC(k052109_device::write));
	map(0x5f8c, 0x5f8c).portr("P1");
	map(0x5f8d, 0x5f8d).portr("P2");
	map(0x5f8e, 0x5f8e).portr("DSW3");
	map(0x5f8f, 0x5f8f).portr("DSW1");
	map(0x5f90, 0x5f90).portr("DSW2");
	map(0x5fa0, 0x5faf).rw(m_k053244, FUNC(k05324x_device::k053244_r), FUNC(k05324x_device::k053244_w));
	map(0x5fb0, 0x5fbf).w(m_k053251, FUNC(k053251_device::write));
	map(0x5fc0, 0x5fc0).r("watchdog", FUNC(watchdog_timer_device::reset_r)).w(FUNC(surpratk_state::_5fc0_w));
	map(0x5fc4, 0x5fc4).w(FUNC(surpratk_state::videobank_w));
	map(0x5fd0, 0x5fd1).w("ymsnd", FUNC(ym2151_device::write));
	map(0x8000, 0xffff).rom().region("maincpu", 0x38000);
}

void surpratk_state::bank0000_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x0800, 0x0fff).rw(m_k053244, FUNC(k05324x_device::k053245_r), FUNC(k05324x_device::k053245_w));
	map(0x1000, 0x1fff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
}


/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( surpratk )
	PORT_START("P1")
	KONAMI8_ALT_B12(1)

	PORT_START("P2")
	KONAMI8_ALT_B12(2)

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), DEF_STR( Free_Play ), SW1)

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Upright Controls" )      PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x20, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_SERVICE_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPNAME( 0x80, 0x80, "Bonus Quiz" )            PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
INPUT_PORTS_END


void surpratk_state::machine_start()
{
	m_mainbank->configure_entries(0, 32, memregion("maincpu")->base(), 0x2000);
	m_mainbank->set_entry(0);

	save_item(NAME(m_sprite_colorbase));
	save_item(NAME(m_layer_colorbase));
	save_item(NAME(m_layerpri));
}

void surpratk_state::machine_reset()
{
	m_bank0000->set_bank(0);

	for (int i = 0; i < 3; i++)
	{
		m_layerpri[i] = 0;
		m_layer_colorbase[i] = 0;
	}

	m_sprite_colorbase = 0;
}

void surpratk_state::banking_callback(uint8_t data)
{
//  logerror("%s: setlines %02x\n", machine().describe_context(), data);
	m_mainbank->set_entry(data & 0x1f);
}

void surpratk_state::surpratk(machine_config &config)
{
	// basic machine hardware
	KONAMI(config, m_maincpu, XTAL(24'000'000) / 2); // 053248, the clock input is 12MHz, and internal CPU divider of 4
	m_maincpu->set_addrmap(AS_PROGRAM, &surpratk_state::main_map);
	m_maincpu->line().set(FUNC(surpratk_state::banking_callback));

	ADDRESS_MAP_BANK(config, "bank0000").set_map(&surpratk_state::bank0000_map).set_options(ENDIANNESS_BIG, 8, 13, 0x800);

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(12*8, (64-12)*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(surpratk_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 2048);
	m_palette->enable_shadows();

	K052109(config, m_k052109, 0);
	m_k052109->set_palette(m_palette);
	m_k052109->set_screen("screen");
	m_k052109->set_tile_callback(FUNC(surpratk_state::tile_callback));
	m_k052109->irq_handler().set_inputline(m_maincpu, KONAMI_IRQ_LINE);

	K053244(config, m_k053244, 0);
	m_k053244->set_palette(m_palette);
	m_k053244->set_sprite_callback(FUNC(surpratk_state::sprite_callback));

	K053251(config, m_k053251, 0);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", XTAL(3'579'545)));
	ymsnd.irq_handler().set_inputline(m_maincpu, KONAMI_FIRQ_LINE);
	ymsnd.add_route(0, "speaker", 1.0, 0);
	ymsnd.add_route(1, "speaker", 1.0, 1);
}



/***************************************************************************

  Game ROMs

***************************************************************************/


ROM_START( suratk )
	ROM_REGION( 0x40000, "maincpu", 0 ) // code + banked roms
	ROM_LOAD( "911j01.f5", 0x00000, 0x20000, CRC(1e647881) SHA1(241e421d5599ebd9fcfb8be9c48dfd3b4c671958) )
	ROM_LOAD( "911k02.h5", 0x20000, 0x20000, CRC(ef10e7b6) SHA1(0b41a929c0c579d688653a8d90dd6b40db12cfb3) )

	ROM_REGION( 0x080000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "911d05.bin", 0x000000, 0x040000, CRC(308d2319) SHA1(521d2a72fecb094e2c2f23b535f0b527886b4d3a) )
	ROM_LOAD32_WORD( "911d06.bin", 0x000002, 0x040000, CRC(91cc9b32) SHA1(e05b7bbff30f24fe6f009560410f5e90bb118692) )

	ROM_REGION( 0x080000, "k053244", 0 ) // sprites
	ROM_LOAD32_WORD( "911d03.bin", 0x000000, 0x040000, CRC(e34ff182) SHA1(075ca7a91c843bdac7da21ddfcd43f7a043a09b6) )
	ROM_LOAD32_WORD( "911d04.bin", 0x000002, 0x040000, CRC(20700bd2) SHA1(a2fa4a3ee28c1542cdd798907a9ece249aadff0a) )
ROM_END

ROM_START( suratka )
	ROM_REGION( 0x48000, "maincpu", 0 ) // code + banked roms
	ROM_LOAD( "911j01.f5", 0x00000, 0x20000, CRC(1e647881) SHA1(241e421d5599ebd9fcfb8be9c48dfd3b4c671958) )
	ROM_LOAD( "911l02.h5", 0x20000, 0x20000, CRC(11db8288) SHA1(09fe187855172ebf0c57f561cce7f41e47f53114) )

	ROM_REGION( 0x080000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "911d05.bin", 0x000000, 0x040000, CRC(308d2319) SHA1(521d2a72fecb094e2c2f23b535f0b527886b4d3a) )
	ROM_LOAD32_WORD( "911d06.bin", 0x000002, 0x040000, CRC(91cc9b32) SHA1(e05b7bbff30f24fe6f009560410f5e90bb118692) )

	ROM_REGION( 0x080000, "k053244", 0 ) // sprites
	ROM_LOAD32_WORD( "911d03.bin", 0x000000, 0x040000, CRC(e34ff182) SHA1(075ca7a91c843bdac7da21ddfcd43f7a043a09b6) )
	ROM_LOAD32_WORD( "911d04.bin", 0x000002, 0x040000, CRC(20700bd2) SHA1(a2fa4a3ee28c1542cdd798907a9ece249aadff0a) )
ROM_END

ROM_START( suratkj )
	ROM_REGION( 0x40000, "maincpu", 0 ) // code + banked roms
	ROM_LOAD( "911m01.f5", 0x00000, 0x20000, CRC(ee5b2cc8) SHA1(4b05f7ba4e804a3bccb41fe9d3258cbcfe5324aa) )
	ROM_LOAD( "911m02.h5", 0x20000, 0x20000, CRC(5d4148a8) SHA1(4fa5947db777b4c742775d588dea38758812a916) )

	ROM_REGION( 0x080000, "k052109", 0 )    // tiles
	ROM_LOAD32_WORD( "911d05.bin", 0x000000, 0x040000, CRC(308d2319) SHA1(521d2a72fecb094e2c2f23b535f0b527886b4d3a) )
	ROM_LOAD32_WORD( "911d06.bin", 0x000002, 0x040000, CRC(91cc9b32) SHA1(e05b7bbff30f24fe6f009560410f5e90bb118692) )

	ROM_REGION( 0x080000, "k053244", 0 ) // sprites
	ROM_LOAD32_WORD( "911d03.bin", 0x000000, 0x040000, CRC(e34ff182) SHA1(075ca7a91c843bdac7da21ddfcd43f7a043a09b6) )
	ROM_LOAD32_WORD( "911d04.bin", 0x000002, 0x040000, CRC(20700bd2) SHA1(a2fa4a3ee28c1542cdd798907a9ece249aadff0a) )
ROM_END

} // anonymous namespace


/***************************************************************************

  Game driver(s)

***************************************************************************/

GAME( 1990, suratk,  0,      surpratk, surpratk, surpratk_state, empty_init, ROT0, "Konami", "Surprise Attack (World ver. K)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, suratka, suratk, surpratk, surpratk, surpratk_state, empty_init, ROT0, "Konami", "Surprise Attack (Asia ver. L)",  MACHINE_SUPPORTS_SAVE )
GAME( 1990, suratkj, suratk, surpratk, surpratk, surpratk_state, empty_init, ROT0, "Konami", "Surprise Attack (Japan ver. M)", MACHINE_SUPPORTS_SAVE )

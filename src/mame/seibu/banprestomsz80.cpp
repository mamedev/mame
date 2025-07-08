// license:BSD-3-Clause
// copyright-holders:

/*
Banpresto Z80-based medal games with Seibu customs

SCZ80 PCB

Main components:
- Z84C0006PEC main CPU
- 12.000 MHz XTAL
- 2x LH5168D-80L (near CPU)
- 2x HM6116LK-90 (near a pair of GFX ROMs)
- 2x bank of 8 switches
- SEI0181 9149 ABDB custom
- BP-SC001 TC110G26AF gate array
- SEI0200 TC110G21AF gate array
- HB-52 color DAC
- 2x HM6116LK-90 (near the two gate arrays)
- HM6116LK-90 (near other pair of GFX ROMs)
- OKI M6295

TODO:
- hopper (stops the game from working)
- sprites' priority
- check visible area
- 2 more tilemaps (don't seem used by the dumped game)
- enable NVRAM support once it works
*/

#include "emu.h"

#include "seibu_crtc.h"

#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class banprestomsz80_state : public driver_device
{
public:
	banprestomsz80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_crtc(*this, "crtc"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_hopper(*this, "hopper"),
		m_charram(*this, "charram"),
		m_bgram(*this, "bgram"),
		m_spriteram(*this, "sprite_ram")
	{ }

	void banprestomsz80(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<seibu_crtc_device> m_crtc;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<hopper_device> m_hopper;

	required_shared_ptr<uint8_t> m_charram;
	required_shared_ptr<uint8_t> m_bgram;
	required_shared_ptr<uint8_t> m_spriteram;

	tilemap_t *m_char_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;

	uint8_t m_crtc_upper_data = 0;
	uint16_t m_layer_en = 0;
	uint16_t m_scrollram[6] {};

	void output_w(uint8_t data);
	void output2_w(uint8_t data);
	void output3_w(uint8_t data);

	void charram_w(offs_t offset, uint8_t data);
	void bgram_w(offs_t offset, uint8_t data);

	void crtc_upper_data_w(uint8_t data);
	void crtc_w(offs_t offset, uint8_t data);

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	TILE_GET_INFO_MEMBER(get_char_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


void banprestomsz80_state::video_start()
{
	m_char_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(banprestomsz80_state::get_char_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(banprestomsz80_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_char_tilemap->set_transparent_pen(0x0f);
	m_bg_tilemap->set_transparent_pen(0x0f);

	save_item(NAME(m_crtc_upper_data));
	save_item(NAME(m_layer_en));
	save_item(NAME(m_scrollram));
}

void banprestomsz80_state::charram_w(offs_t offset, uint8_t data)
{
	m_charram[offset] = data;
	m_char_tilemap->mark_tile_dirty(offset / 2);
}

void banprestomsz80_state::bgram_w(offs_t offset, uint8_t data)
{
	m_bgram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

TILE_GET_INFO_MEMBER(banprestomsz80_state::get_char_tile_info)
{
	int const tile = m_charram[2 * tile_index] | ((m_charram[2 * tile_index + 1] & 0x07) << 8);
	int const color = m_charram[2 * tile_index + 1] >> 4;

	tileinfo.set(0, tile, color, 0);
}

TILE_GET_INFO_MEMBER(banprestomsz80_state::get_bg_tile_info)
{
	int const tile = m_bgram[2 * tile_index] | ((m_bgram[2 * tile_index + 1] & 0x07) << 8);
	int const color = m_bgram[2 * tile_index + 1] >> 4;

	tileinfo.set(1, tile, color, 0);
}

void banprestomsz80_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int i = 0x200 - 4; i > -1; i -= 4)
	{
		int const tile = m_spriteram[i] | ((m_spriteram[i + 1] & 0x70) << 4);
		int const color = m_spriteram[i + 1] & 0x0f;
		int const y = m_spriteram[i + 2];
		int const x = m_spriteram[i + 3];

		m_gfxdecode->gfx(2)->transpen(bitmap, cliprect, tile, color, 0, 0, x, y, 0xf);
	}
}

uint32_t banprestomsz80_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	m_bg_tilemap->set_scrollx(0, m_scrollram[4]);
	m_bg_tilemap->set_scrolly(0, m_scrollram[5]);

	// are these two tilemaps used? doesn't seem so
	//if (BIT(~m_layer_en, 0)) { tilemap0->draw(screen, bitmap, cliprect, 0, 1); }
	//if (BIT(~m_layer_en, 1)) { tilemap1->draw(screen, bitmap, cliprect, 0, 2); }

	if (BIT(~m_layer_en, 2)) { m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 4); }
	if (BIT(~m_layer_en, 3)) { m_char_tilemap->draw(screen, bitmap, cliprect, 0, 8); }
	if (BIT(~m_layer_en, 4)) { draw_sprites(bitmap, cliprect); }

	return 0;
}

void banprestomsz80_state::crtc_upper_data_w(uint8_t data)
{
	m_crtc_upper_data = data;
}

void banprestomsz80_state::crtc_w(offs_t offset, uint8_t data)
{
	m_crtc->write(offset, (m_crtc_upper_data & 0xe0) << 3 | data);
}

void banprestomsz80_state::output_w(uint8_t data)
{
	machine().bookkeeping().coin_lockout_w(0, BIT(data, 0)); // 10 Yen
	machine().bookkeeping().coin_lockout_w(1, BIT(data, 1)); // 100 Yen
	machine().bookkeeping().coin_lockout_w(2, BIT(data, 2)); // Medal
	machine().bookkeeping().coin_counter_w(0, BIT(data, 3)); // 10 Yen
	machine().bookkeeping().coin_counter_w(1, BIT(data, 4)); // 100 Yen
	machine().bookkeeping().coin_counter_w(2, BIT(data, 5)); // Medal

	if (data & 0xc0)
		logerror("%s output_w bits 6-7 set: %02x\n", machine().describe_context(), data);
}

void banprestomsz80_state::output2_w(uint8_t data)
{
	// TODO:
	// bit 0: NH-1
	// bit 1: AES-505

	if (data & 0xfc)
		logerror("%s output2_w bits 2-7 set: %02x\n", machine().describe_context(), data);
}

void banprestomsz80_state::output3_w(uint8_t data)
{
	if (data)
		logerror("%s output3_w bits 0-7 set: %02x\n", machine().describe_context(), data);
}

void banprestomsz80_state::program_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc7ff).ram().share("nvram");
	map(0xc800, 0xcfff).ram();
	map(0xd000, 0xdfff).ram();
	map(0xe000, 0xe7ff).ram();
	map(0xe800, 0xefff).ram().w(FUNC(banprestomsz80_state::charram_w)).share(m_charram);
	map(0xf000, 0xf7ff).ram().w(FUNC(banprestomsz80_state::bgram_w)).share(m_bgram);
	map(0xf800, 0xfdff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xfe00, 0xffff).ram().share(m_spriteram);
}

// TODO: various writes
void banprestomsz80_state::io_map(address_map &map)
{
	map.global_mask(0xff);

	map(0x40, 0x40).portr("DSW1");
	map(0x41, 0x41).portr("DSW2");
	map(0x42, 0x42).portr("IN0");
	map(0x43, 0x43).portr("IN1");
	map(0x44, 0x44).portr("IN2");
	map(0x60, 0x60).nopw(); // sprite buffering trigger?
	map(0x61, 0x61).w(FUNC(banprestomsz80_state::output_w));
	map(0x62, 0x62).w(FUNC(banprestomsz80_state::output2_w));
	map(0x63, 0x63).w(FUNC(banprestomsz80_state::output3_w));
	map(0x67, 0x67).w(FUNC(banprestomsz80_state::crtc_upper_data_w));
	map(0x80, 0x80).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xa0, 0xbf).w(FUNC(banprestomsz80_state::crtc_w));
}


static INPUT_PORTS_START( dnjsenso )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) // 10 Yen
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) // 100 Yen
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) // Medal
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 ) // Service sw
	PORT_SERVICE_NO_TOGGLE( 0x10, IP_ACTIVE_LOW ) // also selects in test mode
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH,IPT_BUTTON1 ) PORT_PLAYER(3) // NH1 sensor (hopper stuck?)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3) // 505 HP (hopper line?)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH,IPT_BUTTON3 ) PORT_PLAYER(3) // 505 empty
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(3) // S. Medal out
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(3) // G. Medal out
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4) PORT_NAME("Bet 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4) PORT_NAME("Bet 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4) PORT_NAME("Bet 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4) PORT_NAME("Bet 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(4) PORT_NAME("Bet 5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(4) PORT_NAME("Bet 6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(4) PORT_NAME("Bet 7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in test mode

	// Switch effects taken from test mode. No medal in rate shown, so possibly a couple of the unknown ones are that
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("DSW1:1,2") // 100 Yen
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_10C ) )
	PORT_DIPSETTING(    0x03, "1 Coin/11 Credits" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:3") // no effect in test mode
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:4") // no effect in test mode
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:5") // no effect in test mode
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Voice" ) PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("DSW1:7,8") // 10 Yen
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Payout Rate" ) PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x07, "50%" )
	PORT_DIPSETTING(    0x03, "60%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x01, "70%" )
	PORT_DIPSETTING(    0x06, "75%" )
	PORT_DIPSETTING(    0x02, "80%" )
	PORT_DIPSETTING(    0x04, "85%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x08, 0x08, "Winwave" ) PORT_DIPLOCATION("DSW2:4") // ??
	PORT_DIPSETTING(    0x08, "Small" )
	PORT_DIPSETTING(    0x00, "Big" )
	PORT_DIPNAME( 0x30, 0x30, "G Medal Rate" ) PORT_DIPLOCATION("DSW2:5,6")
	PORT_DIPSETTING(    0x30, "03%" )
	PORT_DIPSETTING(    0x10, "05%" )
	PORT_DIPSETTING(    0x20, "07%" )
	PORT_DIPSETTING(    0x00, "10%" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:7") // no effect shown in test mode
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:8") // no effect shown in test mode
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0, 4) },
	{ STEP4(3, -1), STEP4(4*4+3, -1) },
	{ STEP8(0, 4*8) },
	8*8*4
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0, 4) },
	{ STEP4(3, -1), STEP4(4*4+3, -1), STEP4(4*8*16+3, -1), STEP4(4*8*16+4*4+3, -1) },
	{ STEP16(0, 4*8) },
	16*16*4
};


static GFXDECODE_START( gfx )
	GFXDECODE_ENTRY( "tiles", 0, charlayout, 0x200, 16 )
	GFXDECODE_ENTRY( "tiles", 0, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "sprites", 0, tilelayout, 0x100, 16 )
GFXDECODE_END


void banprestomsz80_state::banprestomsz80(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 12_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &banprestomsz80_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &banprestomsz80_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(banprestomsz80_state::irq0_line_hold));

	//NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	HOPPER(config, m_hopper, attotime::from_msec(100)); // TODO: period is guessed

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0, 32*8-1, 0, 28*8-1);
	screen.set_screen_update(FUNC(banprestomsz80_state::screen_update));
	screen.set_palette(m_palette);

	SEIBU_CRTC(config, m_crtc, 0);
	m_crtc->layer_en_callback().set([this] (uint16_t data) { m_layer_en = data; });
	m_crtc->layer_scroll_callback().set([this] (offs_t offset, uint16_t data, uint16_t mem_mask) { COMBINE_DATA(&m_scrollram[offset]); });

	GFXDECODE(config, m_gfxdecode, m_palette, gfx);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x300);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 12_MHz_XTAL / 12, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.40); // TODO: check frequency and pin 7.
}


// ウルトラマン倶楽部 大地防衛戦 (Club Ultraman - Daichikyū Bōeisen)
ROM_START( dnjsenso )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "s83-c01.u0129", 0x00000, 0x10000, CRC(b9cb8a41) SHA1(4ca9c6dd44691e90ac8373fc7e1e6c704cb2f64c) )

	ROM_REGION( 0x40000, "sprites", 0 )
	ROM_LOAD16_BYTE( "s83-a06.u0510", 0x00000, 0x20000, CRC(d2221ab9) SHA1(168ca7cca0d759e7de9d20f838ed9a303ce7bfa8) )
	ROM_LOAD16_BYTE( "s83-a05.u052",  0x00001, 0x20000, CRC(e7fdc950) SHA1(3fe2839a8a8402aa3ee1e867d249b53a4f5bf7a1) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD16_BYTE( "s83-a03.u049",  0x00000, 0x20000, CRC(dced1091) SHA1(03cb87330bed98b96c3ef790bc4dafb744e2f3c9) )
	ROM_LOAD16_BYTE( "s83-a04.u0410", 0x00001, 0x20000, CRC(7146ab4d) SHA1(f819fbe9cf97ea6cb9b6b545fc2786ba5526fd63) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "s83_a02.u0729", 0x00000, 0x20000, CRC(2de05c7a) SHA1(7df663d8f3274dec2af432eb59fa5e270e51efac) )

	ROM_REGION( 0x400, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "scz01b.u017", 0x000, 0x104, NO_DUMP ) // TIBPAL16L8-25CN
	ROM_LOAD( "scz02.u061",  0x200, 0x155, NO_DUMP ) // 18CV8-PC25
ROM_END

} // anonymous namespace


GAME( 1992, dnjsenso, 0, banprestomsz80, dnjsenso, banprestomsz80_state, empty_init, ROT0, "Banpresto", "Club Ultraman - Daichikyu Boeisen", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )

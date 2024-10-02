// license:BSD-3-Clause
// copyright-holders:Angelo Salese, David Haywood
/*******************************************************************************************

Dream 9 Final (c) 1992 Excellent Systems

TODO:
- What does the ES8712 control? There's definitely no ADPCM chip or sample ROM here;
- lamps;
- Start-up fading looks horrible, video timings? btanb?
- Game looks IGS-esque, is there any correlation?

============================================================================================

PCB: ES-9112

Main Chips: Z80, ES8712, 24Mhz OSC, RTC62421B 9262, YM2413, 4x8DSW

*******************************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/msm6242.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "sound/es8712.h"
#include "sound/ymopl.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class d9final_state : public driver_device
{
public:
	d9final_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_hopper(*this, "hopper")
		, m_lo_vram(*this, "lo_vram")
		, m_hi_vram(*this, "hi_vram")
		, m_cram(*this, "cram")
		, m_mainbank(*this, "mainbank")
	{ }

	void d9final(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<ticket_dispenser_device> m_hopper;

	required_shared_ptr<uint8_t> m_lo_vram;
	required_shared_ptr<uint8_t> m_hi_vram;
	required_shared_ptr<uint8_t> m_cram;
	required_memory_bank m_mainbank;

	tilemap_t *m_sc0_tilemap;

	void bank_w(uint8_t data);
	uint8_t prot_latch_r();

	void sc0_lovram(offs_t offset, uint8_t data);
	void sc0_hivram(offs_t offset, uint8_t data);
	void sc0_cram(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_sc0_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map) ATTR_COLD;
	void prg_map(address_map &map) ATTR_COLD;
};



TILE_GET_INFO_MEMBER(d9final_state::get_sc0_tile_info)
{
	int tile = ((m_hi_vram[tile_index] & 0x3f) << 8) | m_lo_vram[tile_index];
	int color = m_cram[tile_index] & 0x3f;

	tileinfo.set(0,
			tile,
			color,
			0);
}

void d9final_state::video_start()
{
	m_sc0_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(d9final_state::get_sc0_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
}

uint32_t d9final_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_sc0_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

void d9final_state::sc0_lovram(offs_t offset, uint8_t data)
{
	m_lo_vram[offset] = data;
	m_sc0_tilemap->mark_tile_dirty(offset);
}

void d9final_state::sc0_hivram(offs_t offset, uint8_t data)
{
	m_hi_vram[offset] = data;
	m_sc0_tilemap->mark_tile_dirty(offset);
}

void d9final_state::sc0_cram(offs_t offset, uint8_t data)
{
	m_cram[offset] = data;
	m_sc0_tilemap->mark_tile_dirty(offset);
}

void d9final_state::bank_w(uint8_t data)
{
	m_mainbank->set_entry(data & 0x7);
}

// game checks this after three attract cycles, otherwise coin inputs stop to work.
uint8_t d9final_state::prot_latch_r()
{
//  printf("PC=%06x\n",m_maincpu->pc());

	return 0x04;
}


void d9final_state::prg_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_mainbank);
	map(0xc000, 0xc7ff).ram().share("nvram");
	map(0xc800, 0xcbff).ram().w("palette", FUNC(palette_device::write8)).share("palette");
	map(0xcc00, 0xcfff).ram().w("palette", FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0xd000, 0xd7ff).ram().w(FUNC(d9final_state::sc0_lovram)).share(m_lo_vram);
	map(0xd800, 0xdfff).ram().w(FUNC(d9final_state::sc0_hivram)).share(m_hi_vram);
	map(0xe000, 0xe7ff).ram().w(FUNC(d9final_state::sc0_cram)).share(m_cram);
	map(0xf000, 0xf007).r(FUNC(d9final_state::prot_latch_r)); //.rw("essnd", FUNC(es8712_device::read), FUNC(es8712_device::write));
	map(0xf800, 0xf80f).rw("rtc", FUNC(rtc62421_device::read), FUNC(rtc62421_device::write));
}

void d9final_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("DSWA").lw8(
		NAME([this] (offs_t offset, u8 data) {
			machine().bookkeeping().coin_lockout_global_w(!BIT(data, 0));
			// bit 1: bet lamp
			// bit 2-3: take score & double up lamps
			// bit 4: big lamp
			// bit 5: small lamp (lit for instruction sheet in bet phase)
			// bit 6: start lamp
			// bit 7: payout lamp
		})
	);
	map(0x20, 0x20).portr("DSWB").lw8(
		NAME([this] (offs_t offset, u8 data) {
			// NOTE: keyin goes to coin counter 2, coin 2 and 3 to 1 & 2
			for (int i = 0; i < 4; i++)
				machine().bookkeeping().coin_counter_w(i, BIT(data, i));

			m_hopper->motor_w(BIT(data, 7));
		})
	);
	map(0x40, 0x40).portr("DSWC");
	map(0x40, 0x41).w("ymsnd", FUNC(ym2413_device::write));
	map(0x60, 0x60).portr("DSWD").nopw(); // write: irq ack? shadows bank_w writes, twice, at line ~8
	map(0x80, 0x80).portr("IN0");
	map(0xa0, 0xa0).portr("IN1").w(FUNC(d9final_state::bank_w));
	map(0xe0, 0xe0).portr("IN2");
}

static INPUT_PORTS_START( d9final )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )

	PORT_START("IN1")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) //another reset button
	PORT_BIT( 0x0e, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x32, 0x32, "Credit Limit" ) PORT_DIPLOCATION("SW1:2,5,6")
	PORT_DIPSETTING(    0x32, "1000" )
	PORT_DIPSETTING(    0x12, "5000" )
	PORT_DIPSETTING(    0x22, "10000" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPSETTING(    0x10, "20000" )
	PORT_DIPSETTING(    0x30, "20000" )
	PORT_DIPSETTING(    0x02, "50000" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW1:3" )
	PORT_DIPNAME( 0x08, 0x08, "Auto Start" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Hopper Switch" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( High ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
	PORT_DIPNAME( 0x80, 0x80, "Payout" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB") //odd rates / difficulty stuff
	PORT_DIPNAME( 0x07, 0x07, "Win Percentage" ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x00, "60%" )
	PORT_DIPSETTING(    0x04, "65%" )
	PORT_DIPSETTING(    0x02, "70%" )
	PORT_DIPSETTING(    0x06, "75%" )
	PORT_DIPSETTING(    0x01, "80%" )
	PORT_DIPSETTING(    0x05, "85%" )
	PORT_DIPSETTING(    0x03, "90%" )
	PORT_DIPSETTING(    0x07, "95%" )
	PORT_DIPNAME( 0x18, 0x18, "Bet Max" ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "8" )
	PORT_DIPSETTING(    0x08, "16" )
	PORT_DIPSETTING(    0x10, "32" )
	PORT_DIPSETTING(    0x00, "64" )
	PORT_DIPNAME( 0x60, 0x60, "Double-Up Difficulty" ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START("DSWD") //coinage C & D
	PORT_DIPNAME( 0x0f, 0x0e, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW3:1,2,3,4")
	PORT_DIPSETTING(    0x00, "10 Coins / 1 Credit" )
	PORT_DIPSETTING(    0x08, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, "5 Coins / 2 Credits" )
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0d, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin / 20 Credits" )
	PORT_DIPSETTING(    0x0b, "1 Coin / 25 Credits" )
	PORT_DIPSETTING(    0x07, "1 Coin / 50 Credits" )
	PORT_DIPSETTING(    0x0f, "1 Coin / 100 Credits" )
	PORT_DIPNAME( 0x70, 0x30, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW3:5,6,7")
	PORT_DIPSETTING(    0x00, "10 Coins / 1 Credit" )
	PORT_DIPSETTING(    0x40, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x70, "1 Coin / 50 Credits" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW3:8" )

	PORT_START("DSWC") //coinage C & Key In Coinage
	PORT_DIPNAME( 0x07, 0x00, "Coin C" ) PORT_DIPLOCATION("SW4:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x01, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin / 20 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin / 25 Credits" )
	PORT_DIPSETTING(    0x07, "1 Coin / 50 Credits" )
	PORT_DIPNAME( 0x38, 0x00, "Key In Credit" ) PORT_DIPLOCATION("SW4:4,5,6")
	PORT_DIPSETTING(    0x00, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(    0x20, "1 Coin / 20 Credits" )
	PORT_DIPSETTING(    0x10, "1 Coin / 40 Credits" )
	PORT_DIPSETTING(    0x30, "1 Coin / 50 Credits" )
	PORT_DIPSETTING(    0x08, "1 Coin / 100 Credits" )
	PORT_DIPSETTING(    0x28, "1 Coin / 200 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin / 250 Credits" )
	PORT_DIPSETTING(    0x38, "1 Coin / 500 Credits" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW4:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW4:8" )
INPUT_PORTS_END

static const gfx_layout tiles16x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 8, 0, 12, 4, 24, 16, 28, 20 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static GFXDECODE_START( gfx_d9final )
	GFXDECODE_ENTRY( "tiles", 0, tiles16x8_layout, 0, 16 * 4 )
GFXDECODE_END

void d9final_state::machine_start()
{
	m_mainbank->configure_entries(0, 8, memregion("maincpu")->base() + 0x10000, 0x4000);
	m_mainbank->set_entry(0);
}

void d9final_state::d9final(machine_config &config)
{
	Z80(config, m_maincpu, 24000000 / 4); /* ? MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &d9final_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &d9final_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(d9final_state::irq0_line_hold));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // Sharp LH5116D-10 + battery

	HOPPER(config, m_hopper, attotime::from_msec(20));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 16, 256-16-1);
	screen.set_screen_update(FUNC(d9final_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_d9final);
	PALETTE(config, "palette", palette_device::BLACK).set_format(palette_device::xBRG_444, 0x400);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YM2413(config, "ymsnd", XTAL(3'579'545)).add_route(ALL_OUTPUTS, "mono", 0.5);

	//ES8712(config, "essnd", 24000000 / 3).add_route(ALL_OUTPUTS, "mono", 1.0); // clock unknown

	RTC62421(config, "rtc", XTAL(32'768)); // internal oscillator
}


ROM_START( d9final )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "2.4h", 0x00000, 0x8000, CRC(a8d838c8) SHA1(85b2cd1b73569e0e4fc13bfff537cfc2b4d569a1)  )
	ROM_CONTINUE(        0x10000, 0x08000 )
	ROM_COPY( "maincpu", 0x10000, 0x18000, 0x08000 ) //or just 0xff
	ROM_LOAD( "1.2h", 0x20000, 0x10000, CRC(901281ec) SHA1(7b4cae343f1b025d988a507141c0fa8229a0fea1)  )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD16_BYTE( "3.13h", 0x00001, 0x40000, CRC(a2de0cce) SHA1(d510671b75417c10ce479663f6f21367121384b4) )
	ROM_LOAD16_BYTE( "4.15h", 0x00000, 0x40000, CRC(859b7105) SHA1(1b36f84706473afaa50b6546d7373a2ee6602b9a) )
ROM_END

ROM_START( rpanic )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "2.4h",    0x00000, 0x8000, CRC(f3496b10) SHA1(84f33ab519a1f55213d0069d161c07bc99844035)  )
	ROM_CONTINUE(        0x10000, 0x08000 )
	ROM_COPY( "maincpu", 0x10000, 0x18000, 0x08000 ) //or just 0xff
	ROM_LOAD( "1.2h",    0x20000, 0x10000, CRC(901281ec) SHA1(7b4cae343f1b025d988a507141c0fa8229a0fea1)  )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD16_BYTE( "3.13h", 0x00001, 0x40000, CRC(a907fd4a) SHA1(15590b8c9c1a791f5f995b909df9035ba3721446) )
	ROM_LOAD16_BYTE( "4.15h", 0x00000, 0x40000, CRC(c5b4d37f) SHA1(e5dbc0251ff288d52637bef93561705403fbe2e2) )
ROM_END

} // Anonymous namespace


GAME( 1992, d9final, 0,       d9final, d9final, d9final_state, empty_init, ROT0, "Excellent System",          "Dream 9 Final (v2.24)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, rpanic,  d9final, d9final, d9final, d9final_state, empty_init, ROT0, "Excellent System / Jaleco", "Rolling Panic (v2.33)", MACHINE_SUPPORTS_SAVE )

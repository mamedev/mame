// license:BSD-3-Clause
// copyright-holders: 

/*

  Master Boy, Modular System version (Z80, 1986) .

  This is Gaelco's first game. Master Boy was originally developed for Modular System, so this was
  the very first Master Boy.

  The exact hardware setup is unknown. At least four Modular System PCBs for Master Boy:
    - MOD 1: Sound board (unknown exact configuration, probably Z80 + AY-8910).
    - MOD 3: CPU board (unknown exact configuration, probably Z80).
    - MOD 4: Tilemap board.
    - MOD ?: ROM board for questions.

  TODO: Everything. Driver is a copy of 'mastboyo', probably allmost everything is wrong.

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class mastboyo_ms_state : public driver_device
{
public:
	mastboyo_ms_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_fgram(*this, "fgram.%u", 0U),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_bank1(*this, "bank1"),
		m_questionrom(*this, "questions")
	{ }

	void mastboyo_ms(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	tilemap_t *m_fg_tilemap;

	required_shared_ptr_array<uint8_t, 2> m_fgram;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_memory_bank m_bank1;
	required_region_ptr<uint8_t> m_questionrom;

	void mastboyo_ms_map(address_map &map) ATTR_COLD;
	void mastboyo_ms_portmap(address_map &map) ATTR_COLD;

	template<uint8_t Which> void fgram_w(offs_t offset, uint8_t data);
	void rombank_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void palette(palette_device &palette) const;
};


TILE_GET_INFO_MEMBER(mastboyo_ms_state::get_fg_tile_info)
{
	int code = m_fgram[0][tile_index];
	int attr = m_fgram[1][tile_index];
	tileinfo.set(0,
			code,
			attr & 0x0f,
			0);
}

void mastboyo_ms_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mastboyo_ms_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

uint32_t mastboyo_ms_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

template<uint8_t Which>
void mastboyo_ms_state::fgram_w(offs_t offset, uint8_t data)
{
	m_fgram[Which][offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void mastboyo_ms_state::palette(palette_device &palette) const
{
}


void mastboyo_ms_state::rombank_w(uint8_t data)
{
	m_bank1->set_entry(data & 0x0f);
}


void mastboyo_ms_state::mastboyo_ms_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram().share("nvram");
	map(0x5000, 0x53ff).ram().w(FUNC(mastboyo_ms_state::fgram_w<0>)).share(m_fgram[0]);
	map(0x5400, 0x57ff).ram().w(FUNC(mastboyo_ms_state::fgram_w<1>)).share(m_fgram[1]);
	map(0x6000, 0x6000).w(FUNC(mastboyo_ms_state::rombank_w));
//  map(0x7000, 0x7000).portr("UNK"); // possible watchdog? or IRQ ack?
	map(0x8000, 0xffff).bankr(m_bank1);
}

void mastboyo_ms_state::mastboyo_ms_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w("aysnd", FUNC(ym2149_device::address_data_w));
	map(0x00, 0x00).r("aysnd", FUNC(ym2149_device::data_r));
}


static INPUT_PORTS_START( mastboyo_ms )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	// note, Player 2 buttons must be used when entering name, and can be used for answering questions even in single player mode
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Red / Enter Initial")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Green / Delete Initial")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Red / <<")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Green / >>")

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, "Disabled" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x30, 0x20, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x30, "6" )
	PORT_DIPNAME( 0x40, 0x40, "Attract Music" )  // From manual... 'Con Reclamo'
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Test Mode" )      // From manual... 'Test Inicial'
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 24, 28, 0, 4, 8, 12, 16, 20,}, // note, slightly strange order
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};


static GFXDECODE_START( gfx_mastboyo_ms )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END


void mastboyo_ms_state::machine_start()
{
	m_bank1->configure_entries(0, 0x10, &m_questionrom[0x00000], 0x08000);
	m_bank1->set_entry(1);
}


void mastboyo_ms_state::mastboyo_ms(machine_config &config)
{
	// Basic machine hardware
	Z80(config, m_maincpu, 20_MHz_XTAL / 6); // Unknown clock and divisor
	m_maincpu->set_addrmap(AS_PROGRAM, &mastboyo_ms_state::mastboyo_ms_map);
	m_maincpu->set_addrmap(AS_IO, &mastboyo_ms_state::mastboyo_ms_portmap);
	m_maincpu->set_periodic_int(FUNC(mastboyo_ms_state::irq0_line_hold), attotime::from_hz(256.244f));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// Video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 2*8, 256-2*8-1);
	screen.set_screen_update(FUNC(mastboyo_ms_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_mastboyo_ms);

	PALETTE(config, "palette", FUNC(mastboyo_ms_state::palette), 256);

	// Sound hardware
	SPEAKER(config, "mono").front_center();

	Z80(config, m_audiocpu, 20_MHz_XTAL / 5); // Unknown clock and divisor

	ay8910_device &aysnd(AY8910(config, "aysnd", 20_MHz_XTAL / 4)); // Unknown clock and divisor
	aysnd.port_a_read_callback().set_ioport("DSW"); // DSW
	aysnd.port_b_read_callback().set_ioport("IN0"); // player inputs
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);
}


ROM_START( mastboyom )
	ROM_REGION( 0x30000, "maincpu", 0 ) // on MOD-3 board
	ROM_LOAD( "ma_301_27512.bin", 0x00000, 0x10000, CRC(3b67c516) SHA1(bf36b143e0df10fc871a7b079c4aa8a2cbdccca0) )
	ROM_LOAD( "ma_302_27512.bin", 0x10000, 0x10000, CRC(871d4a70) SHA1(20126ca9887d51f488847540d23264bc1c9a8233) )
	ROM_LOAD( "ma_303_27512.bin", 0x20000, 0x10000, CRC(0d48929b) SHA1(eaa8e3752d5d5a051813dd408988d53430d2df1a) ) //  1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x10000, "gfx1", 0 ) // on MOD-4 board
	ROM_LOAD( "ma_0401_27128.bin", 0x00000, 0x02000, CRC(ad9f97d2) SHA1(1b8cde21c5fd5f1970ef6cc20616bff5c36a6b68) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_CONTINUE(                  0x02000, 0x02000)
	ROM_LOAD( "ma_0402_27128.bin", 0x04000, 0x02000, CRC(76d14275) SHA1(6bd920bd58b9b30cd81e2658084639a16314322d) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_CONTINUE(                  0x06000, 0x02000)
	ROM_LOAD( "ma_0403_27128.bin", 0x08000, 0x02000, CRC(58abbe25) SHA1(0747985ceaf74c9f1fd34b6927d2d5dc9e8d8ad9) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_CONTINUE(                  0x0a000, 0x02000)
	ROM_LOAD( "ma_0404_27128.bin", 0x0c000, 0x02000, CRC(58abbe25) SHA1(0747985ceaf74c9f1fd34b6927d2d5dc9e8d8ad9) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_CONTINUE(                  0x0e000, 0x02000)

	// Taken from 'mastboyo', should be the same questions on the same format
	ROM_REGION( 0x80000, "questions", ROMREGION_ERASEFF )
	ROM_LOAD( "mastboy_27256.ic7",  0x40000, 0x08000, CRC(3a214efd) SHA1(752fe28a70dc01b5d4ec38c4751e609c690eed71) )
	ROM_LOAD( "mastboy_27256.ic6",  0x48000, 0x08000, CRC(4d682cfb) SHA1(7939cbc72f20c1e930b0a91ac164c7c8b3d8cb34) )
	ROM_LOAD( "mastboy_27256.ic8",  0x50000, 0x08000, CRC(40b07eeb) SHA1(93f62e0a2a330f7ff1041eaa7cba3ef42121b1a8) )
	ROM_LOAD( "mastboy_27256.ic10", 0x60000, 0x08000, CRC(b92ffd4f) SHA1(34431d6771e58f2ec083756f07e8b2a02bdb0e5a) )
	ROM_LOAD( "mastboy_27256.ic9",  0x68000, 0x08000, CRC(266e7d37) SHA1(39b5e4ff4475393126a97c8b51980bfc1b7b2627) )
	ROM_LOAD( "mastboy_27256.ic12", 0x70000, 0x08000, CRC(efb4b2f9) SHA1(c68f7d83549b554d25f7404a758f48f962622a2d) )
	ROM_LOAD( "mastboy_27256.ic11", 0x78000, 0x08000, CRC(f2611186) SHA1(05860fecc23014c39cb28762763e94bc91412b34) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) // on MOD-1 board
	ROM_LOAD16_BYTE( "ma_0101_n_27c256.bin", 0x00000, 0x08000, CRC(7c1ae820) SHA1(2922b9094289daa2830acb6aab3b72941eab02b8) ) // 1xxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "ma_0101_v_27c256.bin", 0x00001, 0x08000, CRC(e754176b) SHA1(325415ddc6bbe77493a7ee0130c1fe103f771bb0) ) // 1xxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200, "proms", 0 ) // timing or memory mapping?
	ROM_LOAD( "p0314_82s147a.bin", 0x000, 0x200, CRC(aea2bbab) SHA1(506cca2acd8e65e5dbddef3a7e93950d54cf3d5a) )
ROM_END


} // anonymous namespace


GAME( 1986, mastboyom, mastboyo, mastboyo_ms, mastboyo_ms, mastboyo_ms_state, empty_init, ROT0, "Gaelco", "Master Boy (1986, Modular System)", MACHINE_SUPPORTS_SAVE )

// license:BSD-3-Clause
// copyright-holders:

/*

  Master Boy, Modular System version (Z80, 1986).

  This is Gaelco's first game. Master Boy was originally developed for Modular System, so this was
  the very first Master Boy.

  The exact hardware setup is unknown. At least four Modular System PCBs for Master Boy:
    - MOD 1: Sound board (unknown exact configuration, probably Z80 + AY-8910).
    - MOD 3: CPU board (unknown exact configuration, probably Z80).
    - MOD 4: Tilemap board.
    - MOD ?: ROM board for questions.

  TODO:
  - improve colors
  - adjust visible area
  - verify ROM bank once colors are better
  - sound

*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
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
		m_tileram(*this, "tileram.%u", 0U),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_rombank(*this, "rombank")
	{ }

	void mastboyo_ms(machine_config &config) ATTR_COLD;

	void init_mastboyom() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_shared_ptr_array<uint8_t, 2> m_tileram;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_memory_bank m_rombank;

	tilemap_t *m_tilemap = nullptr;

	void main_program_map(address_map &map) ATTR_COLD;
	void sound_program_map(address_map &map) ATTR_COLD;

	template<uint8_t Which> void tileram_w(offs_t offset, uint8_t data);
	void rombank_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


TILE_GET_INFO_MEMBER(mastboyo_ms_state::get_tile_info)
{
	int const code = m_tileram[0][tile_index];
	int const attr = m_tileram[1][tile_index] & 0x0f;

	tileinfo.set(0, code, attr, 0);
}

void mastboyo_ms_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mastboyo_ms_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

uint32_t mastboyo_ms_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

template<uint8_t Which>
void mastboyo_ms_state::tileram_w(offs_t offset, uint8_t data)
{
	m_tileram[Which][offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}


void mastboyo_ms_state::machine_start()
{
	m_rombank->configure_entries(0, 8, memregion("maincpu")->base() + 0x10000, 0x4000);
	m_rombank->set_entry(0);
}

void mastboyo_ms_state::rombank_w(uint8_t data)
{
	m_rombank->set_entry(data & 0x07);
	if (data >= 0x08)
		logerror("unknown rombank entry %02x\n", data);
}


void mastboyo_ms_state::main_program_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram().share("nvram");
	map(0x5000, 0x53ff).ram().w(FUNC(mastboyo_ms_state::tileram_w<0>)).share(m_tileram[0]);
	map(0x5400, 0x57ff).ram().w(FUNC(mastboyo_ms_state::tileram_w<1>)).share(m_tileram[1]);
	map(0x6000, 0x6000).w(FUNC(mastboyo_ms_state::rombank_w));
	map(0x6400, 0x6400).nopw(); // TODO: sound latch
	map(0x6404, 0x6404).portr("DSW");
	map(0x6405, 0x6405).portr("IN0");
	map(0x6800, 0x68ff).ram().w("palette", FUNC(palette_device::write8)).share("palette");
	map(0x6c00, 0x6cff).ram().w("palette", FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0x7000, 0x7000).nopr(); // possible watchdog? or IRQ ack?
	map(0x8000, 0xbfff).bankr(m_rombank);
}

void mastboyo_ms_state::sound_program_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0xc000, 0xcfff).ram();
}


static INPUT_PORTS_START( mastboyo_ms )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	// note, Player 2 buttons must be used when entering name, and can be used for answering questions even in single player mode
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Green / Delete Initial")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Red / Enter Initial")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Green / >>")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Red / <<")

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
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )  // From manual... 'Con Reclamo'
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Test ) )      // From manual... 'Test Inicial'
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static GFXDECODE_START( gfx_mastboyo_ms )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x4_planar, 0, 16 )
GFXDECODE_END


void mastboyo_ms_state::mastboyo_ms(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 20_MHz_XTAL / 6); // Unknown clock and divisor
	m_maincpu->set_addrmap(AS_PROGRAM, &mastboyo_ms_state::main_program_map);
	m_maincpu->set_vblank_int("screen", FUNC(mastboyo_ms_state::irq0_line_hold));

	Z80(config, m_audiocpu, 20_MHz_XTAL / 5); // Unknown clock and divisor
	m_audiocpu->set_addrmap(AS_PROGRAM, &mastboyo_ms_state::sound_program_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	GENERIC_LATCH_8(config, "soundlatch");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 2*8, 256-2*8-1);
	screen.set_screen_update(FUNC(mastboyo_ms_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_mastboyo_ms);

	PALETTE(config, "palette").set_format(palette_device::xBGR_333_nibble, 0x100);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", 20_MHz_XTAL / 4)); // Unknown clock and divisor
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);
}


ROM_START( mastboyom )
	ROM_REGION( 0x30000, "maincpu", 0 ) // on MOD-3 board, also contain questions
	ROM_LOAD( "ma_303_27512.bin", 0x00000, 0x10000, CRC(0d48929b) SHA1(eaa8e3752d5d5a051813dd408988d53430d2df1a) ) //  1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "ma_302_27512.bin", 0x10000, 0x10000, CRC(871d4a70) SHA1(20126ca9887d51f488847540d23264bc1c9a8233) )
	ROM_LOAD( "ma_301_27512.bin", 0x20000, 0x10000, CRC(3b67c516) SHA1(bf36b143e0df10fc871a7b079c4aa8a2cbdccca0) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // on MOD-1 board. 2 bytes different between the 2, preserve both for now.
	ROM_LOAD( "ma_0101_n_27c256.bin", 0x0000, 0x8000, CRC(7c1ae820) SHA1(2922b9094289daa2830acb6aab3b72941eab02b8) ) // 1xxxxxxxxxxxxxx = 0xFF
	ROM_LOAD( "ma_0101_v_27c256.bin", 0x8000, 0x8000, CRC(e754176b) SHA1(325415ddc6bbe77493a7ee0130c1fe103f771bb0) ) // 1xxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x8000, "tiles", ROMREGION_INVERT ) // on MOD-4 board
	ROM_LOAD( "ma_0401_27128.bin", 0x00000, 0x02000, CRC(ad9f97d2) SHA1(1b8cde21c5fd5f1970ef6cc20616bff5c36a6b68) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_CONTINUE(                  0x00000, 0x02000)
	ROM_LOAD( "ma_0402_27128.bin", 0x02000, 0x02000, CRC(76d14275) SHA1(6bd920bd58b9b30cd81e2658084639a16314322d) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_CONTINUE(                  0x02000, 0x02000)
	ROM_LOAD( "ma_0403_27128.bin", 0x04000, 0x02000, CRC(58abbe25) SHA1(0747985ceaf74c9f1fd34b6927d2d5dc9e8d8ad9) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_CONTINUE(                  0x04000, 0x02000)
	ROM_LOAD( "ma_0404_27128.bin", 0x06000, 0x02000, CRC(7e9286c2) SHA1(495b065ee5cf0f5a88dee7eb999a54dae53921bd) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_CONTINUE(                  0x06000, 0x02000)

	ROM_REGION( 0x200, "proms", 0 ) // timing or memory mapping?
	ROM_LOAD( "p0314_82s147a.bin", 0x000, 0x200, CRC(aea2bbab) SHA1(506cca2acd8e65e5dbddef3a7e93950d54cf3d5a) )
ROM_END


void mastboyo_ms_state::init_mastboyom()
{
	uint8_t *const src = memregion("maincpu")->base();
	int const len = memregion("maincpu")->bytes();

	// bitswap data
	for (int i = 0; i < len; i++)
		src[i] = bitswap<8>(src[i], 5, 3, 7, 2, 0, 1, 4, 6);

	// descramble address
	std::vector<uint8_t> buffer(len);
	memcpy(&buffer[0], src, len);

	for (int i = 0; i < len; i++)
		src[i] = buffer[bitswap<24>(i, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 10, 8, 6, 4, 0, 2, 7, 3, 1, 9, 11, 5)];
}

} // anonymous namespace


GAME( 1987, mastboyom, mastboyo, mastboyo_ms, mastboyo_ms, mastboyo_ms_state, init_mastboyom, ROT0, "Gaelco", "Master Boy (1987, Modular System)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_COLORS )

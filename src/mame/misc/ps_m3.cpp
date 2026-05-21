// license:BSD-3-Clause
// copyright-holders:

/*
PS-M3 PCB

TMP68301AF
16 MHz XTAL
24 MHz XTAL
6x TC55257DPL-70L RAM (2 near CPU, 2 near GFX ROMs, 2 near data (?) ROMs)
Oki M6295
84-pin chip stickered PS NO-266 (near GFX ROMs)
160-pin chip stickered PS NO-FS (near data (?) ROMs)
4x bank of 8 switches

TODO:
- tilemap enable / priorities / scroll;
- verify palette (banked?);
- verify 'data scrambler' emulation;
- verify inputs;
- verify Oki ROM banking;
- hopper;
- lamps.

NOTES:
- default password is '19490817A0' (checked from 0x1ce6)
*/

#include "emu.h"

#include "cpu/m68000/tmp68301.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class ps_m3_state : public driver_device
{
public:
	ps_m3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_oki(*this, "oki")
		, m_vram(*this, "vram%u", 0U)
	{
	}

	void ps_m3(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<okim6295_device> m_oki;

	required_shared_ptr_array<uint16_t, 8> m_vram;

	tilemap_t *m_tilemap[8] {};
	uint8_t m_irq_source = 0;
	uint16_t m_scramble_data = 0;
	uint8_t m_hwid_read_count = 0;

	template <uint8_t Which> TILE_GET_INFO_MEMBER(get_tile_info);
	template <uint8_t Which> TILE_GET_INFO_MEMBER(get_alt_tile_info);
	template <uint8_t Which> void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void counters_w(uint8_t data);
	void oki_bank_w(uint8_t data);

	uint16_t scramble_data_r(offs_t offset);
	uint8_t irq_id_r();

	void program_map(address_map &map) ATTR_COLD;

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_cb);
};


void ps_m3_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ps_m3_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ps_m3_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ps_m3_state::get_tile_info<2>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ps_m3_state::get_tile_info<3>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap[4] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ps_m3_state::get_alt_tile_info<4>)), TILEMAP_SCAN_ROWS, 8, 16, 64, 16);
	m_tilemap[5] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ps_m3_state::get_alt_tile_info<5>)), TILEMAP_SCAN_ROWS, 8, 16, 64, 16);
	m_tilemap[6] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ps_m3_state::get_alt_tile_info<6>)), TILEMAP_SCAN_ROWS, 8, 16, 64, 16);
	m_tilemap[7] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ps_m3_state::get_alt_tile_info<7>)), TILEMAP_SCAN_ROWS, 8, 16, 64, 16);

	for (int i = 0; i < 0x08; i++)
		m_tilemap[i]->set_transparent_pen(0);
}

template <uint8_t Which>
TILE_GET_INFO_MEMBER(ps_m3_state::get_tile_info)
{
	int const tile = m_vram[Which][tile_index * 2 + 0];
	int const color = (m_vram[Which][tile_index * 2 + 1] & 0xff00) >> 8;

	tileinfo.set(0, tile, color, 0);
}

template <uint8_t Which>
TILE_GET_INFO_MEMBER(ps_m3_state::get_alt_tile_info)
{
	int const tile = m_vram[Which][tile_index * 2 + 1] & 0x1fff;
	int const color = (m_vram[Which][tile_index * 2 + 1] & 0xe000) >> 13;

	tileinfo.set(1, tile, color, 0);
}

template <uint8_t Which>
void ps_m3_state::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vram[Which][offset]);
	m_tilemap[Which]->mark_tile_dirty(offset >> 1);
}

uint32_t ps_m3_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	m_tilemap[4]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[5]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[6]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[7]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[2]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[3]->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


void ps_m3_state::machine_start()
{
	save_item(NAME(m_irq_source));
	save_item(NAME(m_scramble_data));
	save_item(NAME(m_hwid_read_count));
}

uint16_t ps_m3_state::scramble_data_r(offs_t offset)
{
	return bitswap<16>(m_scramble_data, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15); // TODO: AI-assisted reverse engineering, verify
}

uint8_t ps_m3_state::irq_id_r() // TODO: AI-assisted reverse engineering, verify
{
	// Bit 1: IRQ source (tested by btst #1 in polling loops at $225F6/$22602)
	// Bit 0: hardware ID (read 8 times during init via word read from $88002E, assembles to $81)
	// During init, IRQ source = 0 so only bit 0 matters.
	// During gameplay, only bit 1 is polled so bit 0 doesn't interfere.
	static const uint8_t hwid_pattern[8] = { 1, 0, 0, 0, 0, 0, 0, 1 };
	uint8_t const id_bit = hwid_pattern[m_hwid_read_count % 8];
	m_hwid_read_count++;

	return m_irq_source | id_bit;
}


void ps_m3_state::counters_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0)); // COIN A
	machine().bookkeeping().coin_counter_w(0, BIT(data, 1)); // COIN B
	machine().bookkeeping().coin_counter_w(0, BIT(data, 2)); // COIN C
	machine().bookkeeping().coin_counter_w(0, BIT(data, 3)); // COIN D

	if (data & 0xf0)
		logerror("unknown counters_w bits: %02x\n", data);
}

void ps_m3_state::oki_bank_w(uint8_t data)
{
	m_oki->set_rom_bank((data & 0xc0) >> 6); // maybe

	if (data & 0x3f)
		logerror("unknown oki_bank_w bits: %02x\n", data);
}


void ps_m3_state::program_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x0fffff).rom();
	map(0x400000, 0x40ffff).ram().share("nvram");
	map(0x800000, 0x801fff).ram().w(FUNC(ps_m3_state::vram_w<0>)).share(m_vram[0]);
	map(0x802000, 0x803fff).ram().w(FUNC(ps_m3_state::vram_w<1>)).share(m_vram[1]);
	map(0x804000, 0x805fff).ram().w(FUNC(ps_m3_state::vram_w<2>)).share(m_vram[2]);
	map(0x806000, 0x807fff).ram().w(FUNC(ps_m3_state::vram_w<3>)).share(m_vram[3]);
	map(0x808000, 0x808fff).ram().w(FUNC(ps_m3_state::vram_w<4>)).share(m_vram[4]);
	map(0x809000, 0x809fff).ram().w(FUNC(ps_m3_state::vram_w<5>)).share(m_vram[5]);
	map(0x80a000, 0x80afff).ram().w(FUNC(ps_m3_state::vram_w<6>)).share(m_vram[6]);
	map(0x80b000, 0x80bfff).ram().w(FUNC(ps_m3_state::vram_w<7>)).share(m_vram[7]);
	map(0x80c000, 0x80ffff).ram(); // more possible layers? seem unused by the dumped game
	// map(0x880000, 0x880027).w // seem to be tilemap scroll / enable registers
	map(0x880028, 0x880029).r(FUNC(ps_m3_state::scramble_data_r)).lw16(NAME([this] (uint16_t data) { m_scramble_data = data; }));
	map(0x88002a, 0x88002b).r(FUNC(ps_m3_state::scramble_data_r));
	map(0x88002e, 0x88002e).lr8(NAME([] () -> uint8_t { return 0x00; }));
	map(0x88002f, 0x88002f).r(FUNC(ps_m3_state::irq_id_r));
	map(0xa00000, 0xa03fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xa04000, 0xa1ffff).ram();
	map(0xa80013, 0xa80013).w(FUNC(ps_m3_state::counters_w));
	map(0xa80017, 0xa80017).w(FUNC(ps_m3_state::oki_bank_w));
	map(0xa80020, 0xa80021).portr("DSW2");
	map(0xa80022, 0xa80023).portr("DSW3");
	map(0xa80024, 0xa80025).portr("DSW4");
	map(0xa80026, 0xa80027).portr("DSW5");
	map(0xa80028, 0xa80029).portr("IN0");
	map(0xa8002a, 0xa8002b).portr("IN1");
	map(0xa8002c, 0xa8002d).portr("IN2");
	map(0xa8002e, 0xa8002f).portr("IN3");
	map(0xb00001, 0xb00001).w(m_oki, FUNC(okim6295_device::write));
}


static INPUT_PORTS_START( dreamhld ) // TODO: where's Take?
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_GAMBLE_LOW ) // small, also works as right in pw screen
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_GAMBLE_BET )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME( "Commercial" ) // pressing this before inserting coins switches to "Commercial Game", whatever it is
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(4) // maybe unused, left as button for easier testing
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(4) // maybe unused, left as button for easier testing
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_GAMBLE_HIGH ) // big, also works as left in pw screen
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(4) // maybe unused, left as button for easier testing
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(4) // maybe unused, left as button for easier testing
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_POKER_CANCEL )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_COIN1 ) // Coin A
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_COIN3 ) // Coin C
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(4) // maybe unused, left as button for easier testing
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) // maybe
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_COIN2 ) // Coin B
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_COIN4 ) // Coin D
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(4) // maybe unused, left as button for easier testing
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3) // maybe unused, left as button for easier testing
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(3) // maybe unused, left as button for easier testing
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_SERVICE1 ) // Service coin? registers same as coin A in bookkeeping. Also gets out of the password screen to the game
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(3) // maybe unused, left as button for easier testing
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(3) // maybe unused, left as button for easier testing
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(3) // maybe unused, left as button for easier testing
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(3) // maybe unused, left as button for easier testing
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(3) // maybe unused, left as button for easier testing
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNUSED )

	// in book-keeping DIP banks are shown for 2 to 5, so keep the same designation here
	PORT_START("DSW2")
	PORT_DIPNAME( 0x0007, 0x0000, "Main Game Level" ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0007, "81%" )
	PORT_DIPSETTING(      0x0006, "82%" )
	PORT_DIPSETTING(      0x0005, "83%" )
	PORT_DIPSETTING(      0x0004, "84%" )
	PORT_DIPSETTING(      0x0003, "85%" )
	PORT_DIPSETTING(      0x0002, "86%" )
	PORT_DIPSETTING(      0x0001, "87%" )
	PORT_DIPSETTING(      0x0000, "88%" )
	PORT_DIPNAME( 0x0008, 0x0000, "Dream Chance Up" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, "+ 5%" )
	PORT_DIPNAME( 0x0030, 0x0000, "Double Up Level" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0000, "A" )
	PORT_DIPSETTING(      0x0010, "B" )
	PORT_DIPSETTING(      0x0020, "C" )
	PORT_DIPSETTING(      0x0030, "D" )
	PORT_DIPNAME( 0x00c0, 0x0000, "Double Up Renchan Level" ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0x0000, "A" )
	PORT_DIPSETTING(      0x0040, "B" )
	PORT_DIPSETTING(      0x0080, "C" )
	PORT_DIPSETTING(      0x00c0, "D" )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0001, 0x0000, "Bet Min" ) PORT_DIPLOCATION("SW3:1") // hardcoded to 100
	PORT_DIPSETTING(      0x0000, "100" )
	PORT_DIPSETTING(      0x0001, "100" )
	PORT_DIPNAME( 0x0006, 0x0000, "Bet Max" ) PORT_DIPLOCATION("SW3:2,3")
	PORT_DIPSETTING(      0x0000, "10" )
	PORT_DIPSETTING(      0x0002, "20" )
	PORT_DIPSETTING(      0x0004, "50" )
	PORT_DIPSETTING(      0x0006, "100" )
	PORT_DIPNAME( 0x0008, 0x0000, "Bet Skip" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPSETTING(      0x0008, "10" )
	PORT_DIPNAME( 0x0070, 0x0000, "Limit Over" ) PORT_DIPLOCATION("SW3:5,6,7")
	PORT_DIPSETTING(      0x0000, "500" )
	PORT_DIPSETTING(      0x0010, "1000" )
	PORT_DIPSETTING(      0x0020, "5000" )
	PORT_DIPSETTING(      0x0030, "10000" )
	PORT_DIPSETTING(      0x0040, "20000" )
	PORT_DIPSETTING(      0x0050, "30000" )
	PORT_DIPSETTING(      0x0060, "50000" )
	PORT_DIPSETTING(      0x0070, "90000" )
	PORT_DIPNAME( 0x0080, 0x0000, "Limit Over Type" ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(      0x0000, "A" )
	PORT_DIPSETTING(      0x0080, "B" )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x0007, 0x0000, "Coin A / Sev In" ) PORT_DIPLOCATION("SW4:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_10C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_20C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_25C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_50C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_100C ) )
	PORT_DIPNAME( 0x0038, 0x0000, "Coin B / Key In - Coin C / ??? In" ) PORT_DIPLOCATION("SW4:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, "1 Coin/10 Credits - 1 Coin/5 Credits" )
	PORT_DIPSETTING(      0x0010, "1 Coin/20 Credits - 1 Coin/10 Credits" )
	PORT_DIPSETTING(      0x0018, "1 Coin/50 Credits - 1 Coin/25 Credits" )
	PORT_DIPSETTING(      0x0020, "1 Coin/100 Credits - 1 Coin/50 Credits" )
	PORT_DIPSETTING(      0x0028, "1 Coin/200 Credits - 1 Coin/100 Credits" )
	PORT_DIPSETTING(      0x0030, "1 Coin/500 Credits - 1 Coin/250 Credits" )
	PORT_DIPSETTING(      0x0038, "1 Coin/1000 Credits - 1 Coin/500 Credits" )
	PORT_DIPNAME( 0x00c0, 0x0000, "Coin D / Med In" ) PORT_DIPLOCATION("SW4:7,8")
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_10C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_20C ) )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x0003, 0x0000, "Hopper Medal" ) PORT_DIPLOCATION("SW5:1,2")
	PORT_DIPSETTING(      0x0000, "Medal 1 Off" )
	PORT_DIPSETTING(      0x0001, "Medal 1 On" )
	PORT_DIPSETTING(      0x0002, "Medal 2 Off" )
	PORT_DIPSETTING(      0x0003, "Medal 2 On" )
	PORT_DIPNAME( 0x001c, 0x0000, "Hopper Out Rate" ) PORT_DIPLOCATION("SW5:3,4,5")
	PORT_DIPSETTING(      0x0000, "1" ) PORT_CONDITION("DSW5", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(      0x0004, "2" ) PORT_CONDITION("DSW5", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(      0x0008, "5" ) PORT_CONDITION("DSW5", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(      0x000c, "10" ) PORT_CONDITION("DSW5", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(      0x0010, "20" ) PORT_CONDITION("DSW5", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(      0x0014, "25" ) PORT_CONDITION("DSW5", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(      0x0018, "50" ) PORT_CONDITION("DSW5", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(      0x001c, "100" ) PORT_CONDITION("DSW5", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(      0x0000, "50" ) PORT_CONDITION("DSW5", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(      0x0004, "100" ) PORT_CONDITION("DSW5", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(      0x0008, "200" ) PORT_CONDITION("DSW5", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(      0x000c, "300" ) PORT_CONDITION("DSW5", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(      0x0010, "500" ) PORT_CONDITION("DSW5", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(      0x0014, "1000" ) PORT_CONDITION("DSW5", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(      0x0018, "2000" ) PORT_CONDITION("DSW5", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(      0x001c, "3000" ) PORT_CONDITION("DSW5", 0x02, EQUALS, 0x02)
	PORT_DIPNAME( 0x0060, 0x0000, "Hopper Out Max" ) PORT_DIPLOCATION("SW5:6,7")
	PORT_DIPSETTING(      0x0000, "10" ) PORT_CONDITION("DSW5", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(      0x0020, "20" ) PORT_CONDITION("DSW5", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(      0x0040, "50" ) PORT_CONDITION("DSW5", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(      0x0060, "100" ) PORT_CONDITION("DSW5", 0x02, EQUALS, 0x00)
	PORT_DIPSETTING(      0x0000, "1" ) PORT_CONDITION("DSW5", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(      0x0020, "5" ) PORT_CONDITION("DSW5", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(      0x0040, "10" ) PORT_CONDITION("DSW5", 0x02, EQUALS, 0x02)
	PORT_DIPSETTING(      0x0060, "999" ) PORT_CONDITION("DSW5", 0x02, EQUALS, 0x02)
	PORT_DIPNAME( 0x0080, 0x0000, "Key Out Counter" ) PORT_DIPLOCATION("SW5:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static const gfx_layout gfx_8x16x7 =
{
	8,16,
	RGN_FRAC(1,1),
	7,
	{ 1,2,3,4,5,6,7 },
	{ 0,8,16,24,32,40,48,56},
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64, 8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	8*128
};

static GFXDECODE_START( gfx_ps_m3 )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x4_planar, 0x1000, 0x100) // TODO: probably banked
	GFXDECODE_ENTRY( "tiles2", 0, gfx_8x16x7, 0, 16 )
GFXDECODE_END


TIMER_DEVICE_CALLBACK_MEMBER(ps_m3_state::scanline_cb) // TODO: probably not totally correct
{
	int const scanline = param;

	if (scanline == 240)
	{
		m_maincpu->set_input_line(1, HOLD_LINE);
		m_irq_source = 2;
	}

	if (scanline == 0)
	{
		m_maincpu->set_input_line(0, HOLD_LINE);
		m_irq_source = 0;
	}
}


void ps_m3_state::ps_m3(machine_config &config)
{
	// basic machine hardware
	tmp68301_device &maincpu(TMP68301(config, m_maincpu, 16_MHz_XTAL));
	maincpu.set_addrmap(AS_PROGRAM, &ps_m3_state::program_map);

	TIMER(config, "scantimer").configure_scanline(FUNC(ps_m3_state::scanline_cb), "screen", 0, 1);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(ps_m3_state::screen_update));
	screen.screen_vblank().set_inputline(m_maincpu, 1);
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ps_m3);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x2000);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 24_MHz_XTAL / 24, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START( dreamhld )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fsq100.u2", 0x00000, 0x80000, CRC(06c1c0a2) SHA1(988b1ce596ab897e5464a495a2b1c72379d9ef1e) )
	ROM_LOAD16_BYTE( "fsi100.u1", 0x00001, 0x80000, CRC(3c6e1aa0) SHA1(0fc846445962e184db3beb6595009e670578c227) )

	ROM_REGION( 0x200000, "tiles", 0 )
	ROM_LOAD( "dhc0.u12", 0x000000, 0x80000, CRC(5c270895) SHA1(5b35d75348a1cafa2d787abf9f191404e287f14d) )
	ROM_LOAD( "dhc1.u13", 0x080000, 0x80000, CRC(8fae9189) SHA1(0ef9cb5343c75d716c555ea756abf67e011f332c) )
	ROM_LOAD( "dhc2.u14", 0x100000, 0x80000, CRC(2047b3c7) SHA1(8a978ec50858570414e5caba0e7e2d9454464927) )
	ROM_LOAD( "dhc3.u15", 0x180000, 0x80000, CRC(36686061) SHA1(dab490fafe886d05af1308d8ffd977e1aaf04ada) )

	ROM_REGION( 0x100000, "tiles2", 0 ) // 7bpp background data
	ROM_LOAD( "dhdc0.u16", 0x00000, 0x80000, CRC(f62cfca5) SHA1(76e89baebabe22b48db723d02504036cccc2286c) ) // FIXED BITS (0xxxxxxx) (correct)
	ROM_LOAD( "dhdc1.u17", 0x80000, 0x80000, CRC(1b11fa22) SHA1(57af78e979a50201d89ebdefe076cf538fc1d2dd) )

	ROM_REGION( 0x100000, "oki", 0 ) // TODO: maybe not correct
	ROM_LOAD( "dhs0.u29", 0x00000, 0x80000, CRC(a0dccbd0) SHA1(5771e1729c33bd0851d566ac50de9e9b1ae8ae57) ) // 0xxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD( "dhs1.u28", 0x80000, 0x80000, CRC(4ecb8245) SHA1(55bbd2fb1215d1b4b8d53beb08f901b213922ff8) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x10000, "nvram", 0 ) // to bypass license expiration / password
	ROM_LOAD( "nvram", 0x00000, 0x10000, CRC(b58ef176) SHA1(4fbb867fa1b5422d0ad73c8095534160b37bf981) )
ROM_END

} // anonymous namespace


GAME( 2004, dreamhld, 0, ps_m3, dreamhld, ps_m3_state, empty_init, ROT0, "Able Corporation / Light Corporation / Paradise Electronics", "Dream Hold (Rev-FET100)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

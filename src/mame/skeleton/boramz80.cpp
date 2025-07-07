// license:BSD-3-Clause
// copyright-holders:

/*
Boram Z80-based poker games

The 2 dumped games come from 2 similar PCBs:
PK uses the ATPK-BORAM 0211 PCB, while Turbo PK uses the ATPK-BORAM 0300 III PCB.
Main components are:
Z80A CPU (different variants)
HD46505SP CRT
I8255 PPI
4 MHz XTAL
13 MHz XTAL
AY-8910 sound chip
on 0211 PCB: 2x 8-DIP banks
on 0300 III PCB: 4x 8-DIP banks

Some PCBs have been seen mixing different sets of GFX ROMs with the same program ROM.

TODO:
- black screen after first attract cycle (interrupts?)
- colors seem good if compared to available pics, but maybe some slight adjustment needed
- 0211 PCB only has 2 DIP banks, but the program reads 4?
- outputs
- decryption for tpkborama
- are the tpkg2 GFX ROMs correct for that set?
*/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class boramz80_state : public driver_device
{
public:
	boramz80_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_char_ram(*this, "char_ram"),
		m_tile_ram(*this, "tile_ram"),
		m_in(*this, "IN%u", 0U)
	{ }

	void pk(machine_config &config) ATTR_COLD;

	void init_tpkborama() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint8_t> m_char_ram;
	required_shared_ptr<uint8_t> m_tile_ram;

	required_ioport_array<5> m_in;

	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;

	uint8_t m_input_matrix = 0xff;

	uint8_t input_r();
	void output_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void charram_w(offs_t offset, uint8_t data);
	void tileram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


void boramz80_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(boramz80_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 40, 25);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(boramz80_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 40, 25);

	m_fg_tilemap->set_transparent_pen(0);
}

TILE_GET_INFO_MEMBER(boramz80_state::get_fg_tile_info)
{
	int const tile = m_tile_ram[2 * tile_index] | ((m_tile_ram[2 * tile_index + 1] & 0x1f) << 8);
	int const color = (m_tile_ram[2 * tile_index + 1] & 0xe0) >> 5; // TODO: verify

	tileinfo.set(1, tile, color, 0);
}

TILE_GET_INFO_MEMBER(boramz80_state::get_bg_tile_info)
{
	int const tile = m_char_ram[2 * tile_index] | ((m_char_ram[2 * tile_index + 1] & 0x03) << 8);
	int const color = (m_char_ram[2 * tile_index + 1] & 0x3c) >> 2; // TODO: verify

	tileinfo.set(0, tile, color, 0);
}

void boramz80_state::charram_w(offs_t offset, uint8_t data)
{
	m_char_ram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

void boramz80_state::tileram_w(offs_t offset, uint8_t data)
{
	m_tile_ram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

uint32_t boramz80_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


void boramz80_state::machine_start()
{
	save_item(NAME(m_input_matrix));
}

uint8_t boramz80_state::input_r()
{
	uint8_t res = 0xff;

	for (int i = 0; i < 5; i++)
		if (!BIT(m_input_matrix, i))
			res &= m_in[i]->read();

	return res;
}

void boramz80_state::output_w(uint8_t data)
{
	// bits 0-3 are coin counters
	for (int i = 0; i < 4; i++)
		machine().bookkeeping().coin_counter_w(i, BIT(data, i));

	if (data & 0xf0)
		logerror("%s output_w: %02x\n", machine().describe_context(), data);

	// bit 6 is used often

	// bit 7 is probably hopper motor
}


void boramz80_state::program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8fff).ram(); // TODO: only 0x800 for pkboram
	map(0xa000, 0xa7ff).ram().w(FUNC(boramz80_state::charram_w)).share(m_char_ram);
	map(0xc000, 0xc7ff).ram().w(FUNC(boramz80_state::tileram_w)).share(m_tile_ram);
	map(0xe000, 0xe3ff).ram().w("palette", FUNC(palette_device::write8)).share("palette");
	map(0xf000, 0xf3ff).ram().w("palette", FUNC(palette_device::write8_ext)).share("palette_ext");
}

void boramz80_state::io_map(address_map &map)
{
	map.global_mask(0xff);

	map(0x00, 0x03).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x20, 0x20).portr("COIN");
	map(0x40, 0x40).r(FUNC(boramz80_state::input_r));
	map(0x60, 0x60).portr("DSW4"); // TODO: write?
	map(0x80, 0x81).w("aysnd", FUNC(ay8910_device::data_address_w));
	map(0x81, 0x81).r("aysnd", FUNC(ay8910_device::data_r));
	//map(0xa0, 0xa0).r();
	map(0xc0, 0xc0).w("crtc", FUNC(hd6845s_device::address_w));
	map(0xc1, 0xc1).w("crtc", FUNC(hd6845s_device::register_w));
}


static INPUT_PORTS_START( pkboram )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_PLAYER(1)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_PLAYER(1)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // not shown is input test
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // not shown is input test
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) // not shown is input test
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) // hopper line
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Take" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "Credit" )
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPNAME( 0x02, 0x02, "Sensor" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x04, 0x04, "Hopper" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, "Max Credit" ) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x18, "10000" )
	PORT_DIPSETTING(    0x10, "20000" )
	PORT_DIPSETTING(    0x08, "30000" )
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPNAME( 0x60, 0x60, "Max Bet" ) PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x60, "10" )
	PORT_DIPSETTING(    0x40, "20" )
	PORT_DIPSETTING(    0x20, "30" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPNAME( 0x80, 0x80, "Royal" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, "Not Cut" )
	PORT_DIPSETTING(    0x00, "Cut" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_10C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_10C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_20C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_50C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_100C ) )
	PORT_DIPNAME( 0x30, 0x30, "Coin C" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_10C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_25C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_50C ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Coin D" ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x07, 0x07, "PK Game %" ) PORT_DIPLOCATION("SW3:1,2,3")
	PORT_DIPSETTING(    0x00, "85%" )
	PORT_DIPSETTING(    0x01, "87%" )
	PORT_DIPSETTING(    0x02, "89%" )
	PORT_DIPSETTING(    0x03, "91%" )
	PORT_DIPSETTING(    0x04, "93%" )
	PORT_DIPSETTING(    0x05, "95%" )
	PORT_DIPSETTING(    0x06, "98%" )
	PORT_DIPSETTING(    0x07, "101%" )
	PORT_DIPNAME( 0x08, 0x08, "D-Up Card 7" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, "Even" )
	PORT_DIPSETTING(    0x00, "Up" )
	PORT_DIPNAME( 0x10, 0x10, "D-Up Game" ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Cut" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW3:6") // marked as *ON -don't use- in test mode
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW3:7") // marked as *ON -don't use- in test mode
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW3:8") // marked as *ON -don't use- in test mode
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x03, 0x03, "D-Up Game %" ) PORT_DIPLOCATION("SW4:1,2")
	PORT_DIPSETTING(    0x00, "87%" )
	PORT_DIPSETTING(    0x01, "90%" )
	PORT_DIPSETTING(    0x02, "93%" )
	PORT_DIPSETTING(    0x03, "96%" )
	PORT_DIPNAME( 0x04, 0x04, "Royal Pnt" ) PORT_DIPLOCATION("SW4:3")
	PORT_DIPSETTING(    0x04, "x500" )
	PORT_DIPSETTING(    0x00, "x300" )
	PORT_DIPNAME( 0x08, 0x08, "SF-4K Pnt" ) PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(    0x08, "x120 - x50" )
	PORT_DIPSETTING(    0x00, "x150 - x60" )
	PORT_DIPNAME( 0x10, 0x10, "SF Rate" ) PORT_DIPLOCATION("SW4:5")
	PORT_DIPSETTING(    0x00, "Down" )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x20, 0x20, "Auto Hold" ) PORT_DIPLOCATION("SW4:6")
	PORT_DIPSETTING(    0x00, "Cut" )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x40, 0x40, "Open Speed" ) PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin Cut" ) PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( tpkboram )
	PORT_INCLUDE( pkboram )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x60, 0x60, "Yen / Credit" ) PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x60, "10" )
	PORT_DIPSETTING(    0x40, "20" )
	PORT_DIPSETTING(    0x20, "30" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPNAME( 0x80, 0x80, "Service" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "X-Cut" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "X 350-" )
	PORT_DIPSETTING(    0x02, "X 700-" )
	PORT_DIPSETTING(    0x01, "X 1500-" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Royal" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "0%" )
	PORT_DIPSETTING(    0x04, "25%" )
	PORT_DIPSETTING(    0x08, "50%" )
	PORT_DIPSETTING(    0x0c, "100%" )
	PORT_DIPNAME( 0x10, 0x10, "Straight Flush" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPSETTING(    0x10, "100%" )
	PORT_DIPNAME( 0x60, 0x60, "Start %" ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x00, "80%" )
	PORT_DIPSETTING(    0x20, "100%" )
	PORT_DIPSETTING(    0x40, "120%" )
	PORT_DIPSETTING(    0x60, "150%" )
	PORT_DIPNAME( 0x80, 0x80, "TBO Sound" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Cut" )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x07, 0x07, "PK Game %" ) PORT_DIPLOCATION("SW3:1,2,3")
	PORT_DIPSETTING(    0x00, "80%" )
	PORT_DIPSETTING(    0x01, "83%" )
	PORT_DIPSETTING(    0x02, "86%" )
	PORT_DIPSETTING(    0x03, "89%" )
	PORT_DIPSETTING(    0x04, "92%" )
	PORT_DIPSETTING(    0x05, "95%" )
	PORT_DIPSETTING(    0x06, "97%" )
	PORT_DIPSETTING(    0x07, "99%" )
	PORT_DIPNAME( 0x08, 0x08, "D-Up Chance" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Cut" )

	PORT_MODIFY("DSW4")
	PORT_DIPNAME( 0x03, 0x03, "D-Up Game %" ) PORT_DIPLOCATION("SW4:1,2")
	PORT_DIPSETTING(    0x00, "80%" )
	PORT_DIPSETTING(    0x01, "85%" )
	PORT_DIPSETTING(    0x02, "90%" )
	PORT_DIPSETTING(    0x03, "95%" )
	PORT_DIPNAME( 0x0c, 0x0c, "D-Up Min" ) PORT_DIPLOCATION("SW4:3,4")
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x10, 0x10, "T-Analyze" ) PORT_DIPLOCATION("SW4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Cut" )
INPUT_PORTS_END

static INPUT_PORTS_START( pkrboram )
	PORT_INCLUDE( tpkboram )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x60, 0x60, "Yen / Credit" ) PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x60, "10" )
	PORT_DIPSETTING(    0x40, "20" )
	PORT_DIPSETTING(    0x20, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x80, 0x80, "Max Bet" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPSETTING(    0x80, "50" )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW3:6") // marked as *OFF -don't touch- in test mode
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW3:7") // marked as *OFF -don't touch- in test mode
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW3:8") // marked as *OFF -don't touch- in test mode
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW4")
	PORT_DIPNAME( 0x10, 0x10, "Turbo Bet" ) PORT_DIPLOCATION("SW4:5")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "5" )
INPUT_PORTS_END

static INPUT_PORTS_START( pkts )
	PORT_INCLUDE( tpkboram )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:8") // no definition in test mode
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

const gfx_layout gfx_8x8x8_planar =
{
	8,8,
	RGN_FRAC(1,8),
	8,
	{ RGN_FRAC(7,8), RGN_FRAC(6,8), RGN_FRAC(5,8), RGN_FRAC(4,8), RGN_FRAC(3,8), RGN_FRAC(2,8), RGN_FRAC(1,8), RGN_FRAC(0,8) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};


static GFXDECODE_START( gfx_boram )
	GFXDECODE_ENTRY( "chars", 0, gfx_8x8x2_planar,  0x000, 16 )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x8_planar,  0x200, 16 )
GFXDECODE_END


void boramz80_state::pk(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &boramz80_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &boramz80_state::io_map);

	i8255_device &ppi(I8255A(config, "ppi"));
	ppi.in_pa_callback().set_ioport("DSW1");
	ppi.in_pb_callback().set_ioport("DSW2");
	ppi.in_pc_callback().set([this] () { logerror("%s: PPI port C read\n", machine().describe_context()); return 0; });
	ppi.out_pc_callback().set(FUNC(boramz80_state::output_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(40*8, 25*8);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(boramz80_state::screen_update));

	hd6845s_device &crtc(HD6845S(config, "crtc", 13_MHz_XTAL / 16));  // divisor guessed
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.out_vsync_callback().set_inputline(m_maincpu, 0);

	GFXDECODE(config, "gfxdecode", "palette", gfx_boram);
	PALETTE(config, "palette").set_format(palette_device::xGRB_444, 0x400);

	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", 4_MHz_XTAL / 4)); // not sure, could derive from 13 MHz XTAL
	aysnd.port_a_read_callback().set([this] () { logerror("%s: AY port A read\n", machine().describe_context()); return 0; });
	aysnd.port_b_read_callback().set_ioport("DSW3");
	aysnd.port_a_write_callback().set([this] (uint8_t data) { m_input_matrix = data; });
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);
}


ROM_START( pkboram )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "b.rom", 0x0000, 0x8000, CRC(5f38640d) SHA1(914cbd3c5e0406e2daa9bdad6bd46758498aabb5) )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "1.cg1", 0x0000, 0x2000, CRC(a5c43569) SHA1(17a5d529ee2ef18019dabf9aefcf595d1193c7d0) )
	ROM_LOAD( "2.cg2", 0x2000, 0x2000, CRC(68906bae) SHA1(4ccec70f4d6044a7e23e4e50c98916278fe7dfd0) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "3.pg1",  0x00000, 0x8000, CRC(5bd66a9b) SHA1(52fcda1e818c19910b88956ed28a479b6d5f3385) )
	ROM_LOAD( "4.pg2",  0x08000, 0x8000, CRC(a8832475) SHA1(f0ec6cd74992cd6f27c12e0c6da6aaabdb8b2e52) )
	ROM_LOAD( "5.pg3",  0x10000, 0x8000, CRC(bf00fe98) SHA1(218103db220d96c6f16e685f48df8a63443d24f7) )
	ROM_LOAD( "6.pg4",  0x18000, 0x8000, CRC(5fe7b018) SHA1(26a41f96a4b4722b73dbee08cfa1272aa6d83ca8) )
	ROM_LOAD( "7.pg5",  0x20000, 0x8000, CRC(c19d09bf) SHA1(124169b22e566b2a44ae1d0ae1259cdb188e8769) )
	ROM_LOAD( "8.pg6",  0x28000, 0x8000, CRC(191d2ab3) SHA1(ad8bfc3f28ccf503cf388791634f32f745559c3c) )
	ROM_LOAD( "9.pg7",  0x30000, 0x8000, CRC(fd182a3a) SHA1(0d7e9e905b33fd6925962d6992c595830a35ac26) )
	ROM_LOAD( "10.pg8", 0x38000, 0x8000, CRC(7c2e9f86) SHA1(b82efdd718fa49cb57330fdcf05df6a9e025a822) )
ROM_END

ROM_START( tpkboram )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "pkt-21spko.rom", 0x0000, 0x8000, CRC(a024d82b) SHA1(4d656261747930415807cd084536ef145fbf0f5b) )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "1.cg1", 0x0000, 0x2000, CRC(69f44d04) SHA1(2f98805e4b70ce3426078f35ff260a3bc97fab86) )
	ROM_LOAD( "2.cg2", 0x2000, 0x2000, CRC(c1adf009) SHA1(0d5d8b39d40c807b9b5ed7418ba871c4d683286a) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "3.pg1",  0x00000, 0x8000, CRC(5506285c) SHA1(017095d0c293b8a5ae73e40a4e5f662d8ba01a06) )
	ROM_LOAD( "4.pg2",  0x08000, 0x8000, CRC(6d62a734) SHA1(42716934bc93f3c815af961a6efbae120bec2793) )
	ROM_LOAD( "5.pg3",  0x10000, 0x8000, CRC(644c44a6) SHA1(a407735ccdefc4ff7a6f2f007b9e1c4846202dfe) )
	ROM_LOAD( "6.pg4",  0x18000, 0x8000, CRC(029cc0d1) SHA1(d41ec3fa38c1729fee3026f5c9365175738ecc99) )
	ROM_LOAD( "7.pg5",  0x20000, 0x8000, CRC(c07a3cac) SHA1(19c1f996494cf0b200c7d781ba3ffd4af9bfe73b) )
	ROM_LOAD( "8.pg6",  0x28000, 0x8000, CRC(8f2a8c3e) SHA1(5ec031dc1fa21a09c1a4ebc0b6bb5f899038801a) )
	ROM_LOAD( "9.pg7",  0x30000, 0x8000, CRC(7dbbdeb5) SHA1(4d379b9e0c825174bf151117e3550809948e1763) )
	ROM_LOAD( "10.pg8", 0x38000, 0x8000, CRC(4a293afa) SHA1(be532e6a476f78638e7f558bf8093e1914bc3688) )
ROM_END

// this runs on a newer ATPK-BORAM PK-0500 PCB. Given all GFX match tpkboram, it's probably a newer revision.
// code is encrypted
ROM_START( tpkborama )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "223.rom", 0x0000, 0x8000, CRC(1d776d37) SHA1(6918cddb0b47d28cf8145823f869dfd2296c0eed) )

	ROM_REGION( 0x4000, "chars", 0 ) // these are same as tpkboram
	ROM_LOAD( "1.cg1", 0x0000, 0x2000, CRC(69f44d04) SHA1(2f98805e4b70ce3426078f35ff260a3bc97fab86) )
	ROM_LOAD( "2.cg2", 0x2000, 0x2000, CRC(c1adf009) SHA1(0d5d8b39d40c807b9b5ed7418ba871c4d683286a) )

	ROM_REGION( 0x40000, "tiles", 0 ) // these are all 1st and 2nd half identical, but same as tpkboram if split
	ROM_LOAD( "3.pg1",  0x00000, 0x8000, CRC(612c5b39) SHA1(9682167b1fbbcd34b71c2628641b646a2993f61b) )
	ROM_IGNORE(                  0x8000 )
	ROM_LOAD( "4.pg2",  0x08000, 0x8000, CRC(14ee6437) SHA1(a046b3efb14a400d201f7ce1c3ee0e01badb46a6) )
	ROM_IGNORE(                  0x8000 )
	ROM_LOAD( "5.pg3",  0x10000, 0x8000, CRC(ce87f0c5) SHA1(96379856182bb0c81c805906551ec2e4aa2eb1d5) )
	ROM_IGNORE(                  0x8000 )
	ROM_LOAD( "6.pg4",  0x18000, 0x8000, CRC(0a8a6106) SHA1(ac88f1ef2eb39cd24a236b2f18e85367c0736ae8) )
	ROM_IGNORE(                  0x8000 )
	ROM_LOAD( "7.pg5",  0x20000, 0x8000, CRC(484a0eec) SHA1(6e32da2d4d78fb4c4bae2d2da945a71231051d5f) )
	ROM_IGNORE(                  0x8000 )
	ROM_LOAD( "8.pg6",  0x28000, 0x8000, CRC(772d8996) SHA1(bd0412d0656a26a80b0f00ff5d6bcff2c4adb6c7) )
	ROM_IGNORE(                  0x8000 )
	ROM_LOAD( "9.pg7",  0x30000, 0x8000, CRC(ff052a99) SHA1(7523ab2eeef1e44107710c8a68897daa7bf2ce12) )
	ROM_IGNORE(                  0x8000 )
	ROM_LOAD( "10.pg8", 0x38000, 0x8000, CRC(61a4e0f3) SHA1(8d9f0efd3b691eaf93c933c63ba6aa34ebad71b1) )
	ROM_IGNORE(                  0x8000 )
ROM_END

ROM_START( pkrboram ) // ATPK-BORAM PK-0500 PCB
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "ra.rom", 0x0000, 0x8000, CRC(cce7a355) SHA1(76f1f2960bd8ea2bedae9bb414159452a70855e6) )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "1.cg1", 0x0000, 0x2000, CRC(436a6371) SHA1(ab8b7664c54c169599ec1a016e4b6c56aa2d74a7) )
	ROM_LOAD( "2.cg2", 0x2000, 0x2000, CRC(eb33de7c) SHA1(8a9b9ee2349227cfcd9def2c7e85d598069fbb1f) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "3.pg1",  0x00000, 0x10000, CRC(db3c2a23) SHA1(de0c5f118e646e9339980dba48bac287f644ea09) )
	ROM_LOAD( "4.pg2",  0x10000, 0x10000, CRC(3494f28f) SHA1(0cd221c5475c59e82af63d756ee5ecb4a3fbc710) )
	ROM_LOAD( "5.pg3",  0x20000, 0x10000, CRC(0a892a90) SHA1(194bf0e3b7514cc5a9b05ac9f4c55dc1c5934a76) )
	ROM_LOAD( "6.pg4",  0x30000, 0x10000, CRC(0f8dfc3e) SHA1(b2eecc7dd09d3e486b90732a2097bea1a9d1ab70) )
	ROM_LOAD( "7.pg5",  0x40000, 0x10000, CRC(09a0893f) SHA1(712f04a8091228d679a7bd94f2a9884a76b62596) )
	ROM_LOAD( "8.pg6",  0x50000, 0x10000, CRC(1aadfd49) SHA1(3a453b0dfc47a2b67c778faaec3844983bf8f307) )
	ROM_LOAD( "9.pg7",  0x60000, 0x10000, CRC(1d27ac28) SHA1(c8ebaea66d136425970c2335b9d8e16402433b53) )
	ROM_LOAD( "10.pg8", 0x70000, 0x10000, CRC(2c810312) SHA1(df055c7828da2b49bcbd22721309011b7d2143f9) )
ROM_END

ROM_START( tpkg2 ) // ATPK-BORAM PK-0500 PCB, same GFX as pkrboram
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "g21.rom", 0x0000, 0x8000, CRC(1cc7f79f) SHA1(1813d493f50eb971f460426aa618c1a7b2558fc7) )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "1.cg1", 0x0000, 0x2000, CRC(436a6371) SHA1(ab8b7664c54c169599ec1a016e4b6c56aa2d74a7) )
	ROM_LOAD( "2.cg2", 0x2000, 0x2000, CRC(eb33de7c) SHA1(8a9b9ee2349227cfcd9def2c7e85d598069fbb1f) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "3.pg1",  0x00000, 0x10000, CRC(db3c2a23) SHA1(de0c5f118e646e9339980dba48bac287f644ea09) )
	ROM_LOAD( "4.pg2",  0x10000, 0x10000, CRC(3494f28f) SHA1(0cd221c5475c59e82af63d756ee5ecb4a3fbc710) )
	ROM_LOAD( "5.pg3",  0x20000, 0x10000, CRC(0a892a90) SHA1(194bf0e3b7514cc5a9b05ac9f4c55dc1c5934a76) )
	ROM_LOAD( "6.pg4",  0x30000, 0x10000, CRC(0f8dfc3e) SHA1(b2eecc7dd09d3e486b90732a2097bea1a9d1ab70) )
	ROM_LOAD( "7.pg5",  0x40000, 0x10000, CRC(09a0893f) SHA1(712f04a8091228d679a7bd94f2a9884a76b62596) )
	ROM_LOAD( "8.pg6",  0x50000, 0x10000, CRC(1aadfd49) SHA1(3a453b0dfc47a2b67c778faaec3844983bf8f307) )
	ROM_LOAD( "9.pg7",  0x60000, 0x10000, CRC(1d27ac28) SHA1(c8ebaea66d136425970c2335b9d8e16402433b53) )
	ROM_LOAD( "10.pg8", 0x70000, 0x10000, CRC(2c810312) SHA1(df055c7828da2b49bcbd22721309011b7d2143f9) )
ROM_END

ROM_START( pkts ) // ATPK-BORAM PK-0500 PCB
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "11.rom", 0x0000, 0x8000, CRC(822af541) SHA1(621c88bdc2ddca90127eb7ab10ca4ef6ec0cded0) )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "1.cg1", 0x0000, 0x2000, CRC(aa0614ec) SHA1(33f31683a6bd1268e11ad9159fe70d7eaf5edb15) )
	ROM_LOAD( "2.cg2", 0x2000, 0x2000, CRC(025fa9e1) SHA1(fd6d0ad764968ee80355b3fd5156eae89f1d3da8) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "3.pg1",  0x00000, 0x10000, CRC(24206619) SHA1(862403d89d372e3cbffebc37f141748b6543871a) )
	ROM_LOAD( "4.pg2",  0x10000, 0x10000, CRC(327be2a3) SHA1(69b62f50b83528f616378df1c72f35c74dd59022) )
	ROM_LOAD( "5.pg3",  0x20000, 0x10000, CRC(6866c07c) SHA1(7271bdabbdce2cf08d7c45e1e5556bf05ebe1e61) )
	ROM_LOAD( "6.pg4",  0x30000, 0x10000, CRC(c4a928ae) SHA1(ae59254a7cd5bfc5866d25c16671278b84aeb04f) )
	ROM_LOAD( "7.pg5",  0x40000, 0x10000, CRC(050a1387) SHA1(5a1152d53ed91e24993a662db59a55754d79bfae) )
	ROM_LOAD( "8.pg6",  0x50000, 0x10000, CRC(456e3284) SHA1(f61b3c50bc83949bb12b223e7d2bce7d4daa1bd1) )
	ROM_LOAD( "9.pg7",  0x60000, 0x10000, CRC(fba98b86) SHA1(bcf667c8703aa06410c9d8d310fe479db5b28a81) )
	ROM_LOAD( "10.pg8", 0x70000, 0x10000, CRC(c8648897) SHA1(8f024d76b706fde3774d053be6aa307043ce4c06) )
ROM_END


void boramz80_state::init_tpkborama()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x8000; i++)
	{
		// TODO
		rom[i] = rom[i];
	}
}

} // anonymous namespace


GAME( 1987, pkboram,   0,        pk, pkboram,  boramz80_state, empty_init,     ROT0, "Boram", "PK - New Exciting Poker!",        MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // PK-BORAM 0211 aug.04.1987. BORAM CORP
GAME( 1988, tpkboram,  0,        pk, tpkboram, boramz80_state, empty_init,     ROT0, "Boram", "PK Turbo",                        MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // PK-TURBO jan.29.1988. BORAM CORP.
GAME( 1998, tpkborama, tpkboram, pk, tpkboram, boramz80_state, init_tpkborama, ROT0, "Boram", "PK Turbo (Ver 2.3B2, encrypted)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // dep inctype-23B1998 0519Ver 2.3B2
GAME( 1990, pkrboram,  0,        pk, pkrboram, boramz80_state, empty_init,     ROT0, "Boram", "PK Rainbow (v 1.5)",              MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // PK RAINBOW v1.5 BORAM Corp. 1990.11.06
GAME( 1992, tpkg2,     0,        pk, tpkboram, boramz80_state, empty_init,     ROT0, "Boram", "PK Turbo Great 2",                MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // PK TURBO GREAT2 1992.06.04 BORAM CORP.
GAME( 19??, pkts,      0,        pk, pkts,     boramz80_state, empty_init,     ROT0, "Boram", "PK Turbo Special",                MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // PKS v100 BORAM CORP

// license:BSD-3-Clause
// copyright-holders:

/*
CLE-TOUCH REV. 1 PCB

Video slots by Chain Luck Electronic (CLE).
At least some of the games were distributed in the USA by Lucky Sunshine Enterprises (LSE).

The main components are:
MC68HC000FN12 CPU
Lattice ispLSI 1032E 70LJ
Lattice ispLSI 1016 60LJ
Lattice iM4A5-32/32 10JC-12JI
2x HM6264LP-70 RAM (near ispLSI 1016)
2x HM6264LP-70 RAM (near CPU ROMs)
2x HM86171-80 RAMDAC (near CPU ROMs)
12 MHz XTAL (for M68K)
AT90S4414 MCU (AVR core)
11.0592 MHz XTAL (for AT90?)
U6295 sound chip
6x 8-DIP banks


TODO:
* correct GFX decode (address/data encrypted?);
* colors / attribute RAM;
* complete inputs;
* what role does the AT90S4414 play?
* $1e002x accesses during transitions (MCU flushes attribute/graphic RAM data?)
*/


#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "video/ramdac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class cle68k_state : public driver_device
{
public:
	cle68k_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_ramdac(*this, "ramdac%u", 0U),
		m_videoram(*this, "videoram%u", 0U),
		m_attrram(*this, "attrram%u", 0U)
	{ }

	void cle68k(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device_array<ramdac_device, 2> m_ramdac;
	required_shared_ptr_array<uint16_t, 2> m_videoram;
	required_shared_ptr_array<uint16_t, 2> m_attrram;

	tilemap_t *m_tilemap[2]{};

	template <uint8_t Which> void videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template <uint8_t Which> void attrram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template <uint8_t Which> TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
	template <uint8_t Which> void ramdac_map(address_map &map) ATTR_COLD;
};


template <uint8_t Which>
TILE_GET_INFO_MEMBER(cle68k_state::get_tile_info)
{
	// TODO: out-of-range bit 15 used for gameplay reels (different tile layout?)
	int const tile = m_videoram[Which][tile_index];
	// TODO: 4 bits for tile entry then rowscroll?
	int const color = 0; //(m_attrram[Which][tile_index] >> 0) & 0xf;

	tileinfo.set(Which, tile, color, 0);
}

template <uint8_t Which>
void cle68k_state::videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram[Which][offset]);
	m_tilemap[Which]->mark_tile_dirty(offset);
}

template <uint8_t Which>
void cle68k_state::attrram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_attrram[Which][offset]);
	m_tilemap[Which]->mark_tile_dirty(offset);
}

uint32_t cle68k_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

void cle68k_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cle68k_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cle68k_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_tilemap[1]->set_transparent_pen(0);
}


void cle68k_state::program_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x180000, 0x180fff).ram().w(FUNC(cle68k_state::attrram_w<0>)).share(m_attrram[0]);
	map(0x181000, 0x181fff).ram().w(FUNC(cle68k_state::attrram_w<1>)).share(m_attrram[1]);
	map(0x182000, 0x182fff).ram().w(FUNC(cle68k_state::videoram_w<0>)).share(m_videoram[0]);
	map(0x183000, 0x183fff).ram().w(FUNC(cle68k_state::videoram_w<1>)).share(m_videoram[1]);
	map(0x1e0004, 0x1e0005).portr("IN0");
	map(0x1e0009, 0x1e0009).w("oki", FUNC(okim6295_device::write));
	map(0x1e0010, 0x1e0010).w(m_ramdac[0], FUNC(ramdac_device::index_w));
	map(0x1e0011, 0x1e0011).w(m_ramdac[1], FUNC(ramdac_device::index_w));
	map(0x1e0012, 0x1e0012).w(m_ramdac[0], FUNC(ramdac_device::pal_w));
	map(0x1e0013, 0x1e0013).w(m_ramdac[1], FUNC(ramdac_device::pal_w));
	map(0x1e0014, 0x1e0014).w(m_ramdac[0], FUNC(ramdac_device::mask_w));
	map(0x1e0015, 0x1e0015).w(m_ramdac[1], FUNC(ramdac_device::mask_w));
//  map(0x1e0020, 0x1e0023) 8-bit address/data pair for a device (MCU or RAMDAC)
	map(0x1e0030, 0x1e0031).portr("IN1").nopw(); // TODO: video reg? outputs?
	map(0x1e0032, 0x1e0033).portr("DSW1");
	map(0x1e0034, 0x1e0035).portr("DSW2");
	map(0x1e0036, 0x1e0037).portr("DSW3");
	map(0x1f0000, 0x1fffff).ram();
}

template <uint8_t Which>
void cle68k_state::ramdac_map(address_map &map)
{
	map(0x000, 0x2ff).rw(m_ramdac[Which], FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}


static INPUT_PORTS_START( dmndhrt ) // TODO: complete inputs
	PORT_START("IN0")
	PORT_BIT(               0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_SERVICE_NO_TOGGLE( 0x0002, IP_ACTIVE_LOW )
	PORT_BIT(               0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(               0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(               0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(               0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(               0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(               0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(               0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(               0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(               0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(               0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(               0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(               0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(               0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(               0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT(               0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(               0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(               0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(               0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(               0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(               0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(               0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(               0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(               0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(               0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(               0x0400, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(               0x0800, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Show Odds")
	PORT_BIT(               0x1000, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT(               0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(               0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(               0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// DIP definitions taken from test mode
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR(Coinage) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_8C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_10C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_20C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_25C ) )
	PORT_DIPNAME( 0x0018, 0x0018, "Key In Ratio" ) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(      0x0018, "Coin x1" )
	PORT_DIPSETTING(      0x0010, "Coin x5" )
	PORT_DIPSETTING(      0x0008, "Coin x10" )
	PORT_DIPSETTING(      0x0000, "Coin x50" )
	PORT_DIPNAME( 0x0060, 0x0060, "Coin In Limit" ) PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(      0x0060, "1000" )
	PORT_DIPSETTING(      0x0040, "5000" )
	PORT_DIPSETTING(      0x0020, "10000" )
	PORT_DIPSETTING(      0x0000, "20000" )
	PORT_DIPNAME( 0x0080, 0x0080, "Key Out" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR ( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR ( On ) )
	PORT_DIPNAME( 0x0700, 0x0700, "Bet Max" ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0700, "10" )
	PORT_DIPSETTING(      0x0600, "16" )
	PORT_DIPSETTING(      0x0500, "32" )
	PORT_DIPSETTING(      0x0400, "40" )
	PORT_DIPSETTING(      0x0300, "64" )
	PORT_DIPSETTING(      0x0200, "64 (duplicate)" )
	PORT_DIPSETTING(      0x0100, "64 (duplicate)" )
	PORT_DIPSETTING(      0x0000, "64 (duplicate)" )
	PORT_DIPNAME( 0x3800, 0x3800, "Bet Min" ) PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x3800, "1" )
	PORT_DIPSETTING(      0x3000, "8" )
	PORT_DIPSETTING(      0x2800, "10" )
	PORT_DIPSETTING(      0x2000, "16" )
	PORT_DIPSETTING(      0x1800, "32" )
	PORT_DIPSETTING(      0x1000, "40" )
	PORT_DIPSETTING(      0x0800, "64" )
	PORT_DIPSETTING(      0x0000, "64 (duplicate)" )
	PORT_DIPNAME( 0x4000, 0x4000, "Card Kind" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, "Normal Card" )
	PORT_DIPSETTING(      0x0000, "Symbol Card" )
	PORT_DIPNAME( 0x8000, 0x8000, "Card 7" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, "Lost" )
	PORT_DIPSETTING(      0x0000, "Even" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:2") // effect not shown in test mode
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW3:3") // effect not shown in test mode
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0038, 0x0038, "Credit Limit" ) PORT_DIPLOCATION("SW3:4,5,6")
	PORT_DIPSETTING(      0x0030, "5000" )
	PORT_DIPSETTING(      0x0028, "10000" )
	PORT_DIPSETTING(      0x0020, "20000" )
	PORT_DIPSETTING(      0x0018, "30000" )
	PORT_DIPSETTING(      0x0010, "40000" )
	PORT_DIPSETTING(      0x0008, "50000" )
	PORT_DIPSETTING(      0x0000, "100000" )
	PORT_DIPSETTING(      0x0038, "Unlimited" )
	PORT_DIPNAME( 0x0040, 0x0040, "Ex Bonus" ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Pay Out Device Kind" ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(      0x0080, "Hopper" )
	PORT_DIPSETTING(      0x0000, "Ticket" )
	PORT_DIPNAME( 0x0700, 0x0700, "Payout Ratio" ) PORT_DIPLOCATION("SW4:1,2,3")
	PORT_DIPSETTING(      0x0700, "55%" )
	PORT_DIPSETTING(      0x0600, "60%" )
	PORT_DIPSETTING(      0x0500, "65%" )
	PORT_DIPSETTING(      0x0400, "70%" )
	PORT_DIPSETTING(      0x0300, "75%" )
	PORT_DIPSETTING(      0x0200, "80%" )
	PORT_DIPSETTING(      0x0100, "85%" )
	PORT_DIPSETTING(      0x0000, "90%" )
	PORT_DIPNAME( 0x1800, 0x1800, "Double Probability" ) PORT_DIPLOCATION("SW4:4,5")
	PORT_DIPSETTING(      0x1800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Difficult ) )
	PORT_DIPSETTING(      0x0000, "Difficult (duplicate)" )
	PORT_DIPNAME( 0x2000, 0x2000, "CB" ) PORT_DIPLOCATION("SW4:6") // what is this?
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0xc000, 0xc000, "CB Ratio" ) PORT_DIPLOCATION("SW4:7,8")
	PORT_DIPSETTING(      0xc000, "50" )
	PORT_DIPSETTING(      0x8000, "100" )
	PORT_DIPSETTING(      0x4000, "200" )
	PORT_DIPSETTING(      0x0000, "500" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0001, 0x0001, "Double Up Game" ) PORT_DIPLOCATION("SW5:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Hands Mode" ) PORT_DIPLOCATION("SW5:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Auto Stop" ) PORT_DIPLOCATION("SW5:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Reel Speed" ) PORT_DIPLOCATION("SW5:4")
	PORT_DIPSETTING(      0x0008, "Slow" )
	PORT_DIPSETTING(      0x0000, DEF_STR( High ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Game Count" ) PORT_DIPLOCATION("SW5:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Score" ) PORT_DIPLOCATION("SW5:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Bonus Mode" ) PORT_DIPLOCATION("SW5:7")
	PORT_DIPSETTING(      0x0040, "6 3 1" )
	PORT_DIPSETTING(      0x0000, "9 6 1" )
	PORT_DIPNAME( 0x0080, 0x0080, "Amusement" ) PORT_DIPLOCATION("SW5:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0f00, 0x0f00, "Key Out Value" ) PORT_DIPLOCATION("SW6:1,2,3,4")
	PORT_DIPSETTING(      0x0f00, "Coin" )
	PORT_DIPSETTING(      0x0e00, "1" )
	PORT_DIPSETTING(      0x0d00, "2" )
	PORT_DIPSETTING(      0x0c00, "4" )
	PORT_DIPSETTING(      0x0b00, "5" )
	PORT_DIPSETTING(      0x0a00, "10" )
	PORT_DIPSETTING(      0x0900, "15" )
	PORT_DIPSETTING(      0x0800, "20" )
	PORT_DIPSETTING(      0x0700, "25" )
	PORT_DIPSETTING(      0x0600, "40" )
	PORT_DIPSETTING(      0x0500, "50" )
	PORT_DIPSETTING(      0x0400, "60" )
	PORT_DIPSETTING(      0x0300, "75" )
	PORT_DIPSETTING(      0x0200, "80" )
	PORT_DIPSETTING(      0x0100, "100" )
	PORT_DIPSETTING(      0x0000, "500" )
	PORT_DIPNAME( 0xf000, 0xf000, "Device Value" ) PORT_DIPLOCATION("SW6:5,6,7,8")
	PORT_DIPSETTING(      0xf000, "Coin" )
	PORT_DIPSETTING(      0xe000, "1" )
	PORT_DIPSETTING(      0xd000, "2" )
	PORT_DIPSETTING(      0xc000, "4" )
	PORT_DIPSETTING(      0xb000, "5" )
	PORT_DIPSETTING(      0xa000, "10" )
	PORT_DIPSETTING(      0x9000, "15" )
	PORT_DIPSETTING(      0x8000, "20" )
	PORT_DIPSETTING(      0x7000, "25" )
	PORT_DIPSETTING(      0x6000, "40" )
	PORT_DIPSETTING(      0x5000, "50" )
	PORT_DIPSETTING(      0x4000, "60" )
	PORT_DIPSETTING(      0x3000, "75" )
	PORT_DIPSETTING(      0x2000, "80" )
	PORT_DIPSETTING(      0x1000, "100" )
	PORT_DIPSETTING(      0x0000, "500" )
INPUT_PORTS_END

static INPUT_PORTS_START( dmndhrtn ) // TODO: inputs
	PORT_INCLUDE( dmndhrt )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x0018, 0x0018, "Key In Ratio" ) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(      0x0018, "Coin x5" )
	PORT_DIPSETTING(      0x0010, "Coin x10" )
	PORT_DIPSETTING(      0x0008, "Coin x25" )
	PORT_DIPSETTING(      0x0000, "Coin x50" )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x0040, 0x0040, "Pool Min." ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(      0x0040, "5000" )
	PORT_DIPSETTING(      0x0000, "10000" )
	PORT_DIPNAME( 0x0700, 0x0700, "Payout Ratio" ) PORT_DIPLOCATION("SW4:1,2,3")
	PORT_DIPSETTING(      0x0700, "82%" )
	PORT_DIPSETTING(      0x0600, "84%" )
	PORT_DIPSETTING(      0x0500, "86%" )
	PORT_DIPSETTING(      0x0400, "88%" )
	PORT_DIPSETTING(      0x0300, "90%" )
	PORT_DIPSETTING(      0x0200, "92%" )
	PORT_DIPSETTING(      0x0100, "94%" )
	PORT_DIPSETTING(      0x0000, "96%" )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW6:1") // effect not shown in test mode
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW6:2") // effect not shown in test mode
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c00, 0x1c00, "Key Out Value" ) PORT_DIPLOCATION("SW6:3,4,5")
	PORT_DIPSETTING(      0x1c00, "Coin" )
	PORT_DIPSETTING(      0x1800, "1" )
	PORT_DIPSETTING(      0x1400, "5" )
	PORT_DIPSETTING(      0x1000, "20" )
	PORT_DIPSETTING(      0x0c00, "50" )
	PORT_DIPSETTING(      0x0800, "100" )
	PORT_DIPSETTING(      0x0400, "200" )
	PORT_DIPSETTING(      0x0000, "500" )
	PORT_DIPNAME( 0xe000, 0xe000, "Device Value" ) PORT_DIPLOCATION("SW6:6,7,8")
	PORT_DIPSETTING(      0xe000, "Coin" )
	PORT_DIPSETTING(      0xc000, "1" )
	PORT_DIPSETTING(      0xa000, "5" )
	PORT_DIPSETTING(      0x8000, "20" )
	PORT_DIPSETTING(      0x6000, "50" )
	PORT_DIPSETTING(      0x4000, "100" )
	PORT_DIPSETTING(      0x2000, "200" )
	PORT_DIPSETTING(      0x0000, "500" )
INPUT_PORTS_END

INPUT_PORTS_START( honeybee ) // TODO: inputs
	PORT_INCLUDE( dmndhrt )

	// DIP definitions taken from test mode
	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR(Coinage) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0004, "1 Coin/15 Credits" )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_20C ) )
	PORT_DIPSETTING(      0x0002, "1 Coin/75 Credits" )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_100C ) )
	PORT_DIPSETTING(      0x0000, "1 Coin/500 Credits" )
	PORT_DIPNAME( 0x0038, 0x0038, "Key In Ratio" ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0038, "1" )
	PORT_DIPSETTING(      0x0030, "4" )
	PORT_DIPSETTING(      0x0028, "5" )
	PORT_DIPSETTING(      0x0020, "15" )
	PORT_DIPSETTING(      0x0018, "20" )
	PORT_DIPSETTING(      0x0010, "75" )
	PORT_DIPSETTING(      0x0008, "100" )
	PORT_DIPSETTING(      0x0000, "500" )
	PORT_DIPNAME( 0x00c0, 0x00c0, "Coin In Limit" ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x00c0, "1000" )
	PORT_DIPSETTING(      0x0080, "5000" )
	PORT_DIPSETTING(      0x0040, "10000" )
	PORT_DIPSETTING(      0x0000, "20000" )
	PORT_DIPNAME( 0x0700, 0x0700, "Pay Out Ratio" ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0700, "67%" )
	PORT_DIPSETTING(      0x0600, "70%" )
	PORT_DIPSETTING(      0x0500, "73%" )
	PORT_DIPSETTING(      0x0400, "76%" )
	PORT_DIPSETTING(      0x0300, "79%" )
	PORT_DIPSETTING(      0x0200, "82%" )
	PORT_DIPSETTING(      0x0100, "85%" )
	PORT_DIPSETTING(      0x0000, "88%" )
	PORT_DIPNAME( 0x0800, 0x0800, "Double Up Game" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x3000, 0x3000, "Double Up Ratio" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x3000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Difficult ) )
	PORT_DIPSETTING(      0x0000, "Difficult (duplicate)" )
	PORT_DIPNAME( 0x4000, 0x4000, "Odd Table Show" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x0007, 0x0007, "Play Max." )  PORT_DIPLOCATION("SW3:1,2,3")
	PORT_DIPSETTING(      0x0007, "8" )
	PORT_DIPSETTING(      0x0006, "10" )
	PORT_DIPSETTING(      0x0005, "16" )
	PORT_DIPSETTING(      0x0004, "32" )
	PORT_DIPSETTING(      0x0003, "48" )
	PORT_DIPSETTING(      0x0002, "64" )
	PORT_DIPSETTING(      0x0001, "64 (duplicate)" )
	PORT_DIPSETTING(      0x0000, "64 (duplicate)" )
	PORT_DIPNAME( 0x0038, 0x0038, "Play Min." ) PORT_DIPLOCATION("SW3:4,5,6")
	PORT_DIPSETTING(      0x0038, "1" )
	PORT_DIPSETTING(      0x0030, "8" )
	PORT_DIPSETTING(      0x0028, "10" )
	PORT_DIPSETTING(      0x0020, "16" )
	PORT_DIPSETTING(      0x0018, "32" )
	PORT_DIPSETTING(      0x0010, "40" )
	PORT_DIPSETTING(      0x0008, "64" )
	PORT_DIPSETTING(      0x0000, "64 (duplicate)" )
	PORT_DIPNAME( 0x00c0, 0x00c0, "Credit Limit" ) PORT_DIPLOCATION("SW3:7,8")
	PORT_DIPSETTING(      0x00c0, "10000" )
	PORT_DIPSETTING(      0x0080, "50000" )
	PORT_DIPSETTING(      0x0040, "100000" )
	PORT_DIPSETTING(      0x0000, "500000" )
	PORT_DIPNAME( 0x0700, 0x0700, "Key Out Value" ) PORT_DIPLOCATION("SW4:1,2,3")
	PORT_DIPSETTING(      0x0700, "1" )
	PORT_DIPSETTING(      0x0600, "4" )
	PORT_DIPSETTING(      0x0500, "5" )
	PORT_DIPSETTING(      0x0400, "15" )
	PORT_DIPSETTING(      0x0300, "20" )
	PORT_DIPSETTING(      0x0200, "75" )
	PORT_DIPSETTING(      0x0100, "100" )
	PORT_DIPSETTING(      0x0000, "500" )
	PORT_DIPNAME( 0x0800, 0x0800, "Pay Out Device Kind" ) PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(      0x0800, "Hopper" )
	PORT_DIPSETTING(      0x0000, "Ticket" )
	PORT_DIPNAME( 0x7000, 0x7000, "Ticket / Hopper Out" ) PORT_DIPLOCATION("SW4:5,6,7")
	PORT_DIPSETTING(      0x7000, "1" )
	PORT_DIPSETTING(      0x6000, "4" )
	PORT_DIPSETTING(      0x5000, "5" )
	PORT_DIPSETTING(      0x4000, "15" )
	PORT_DIPSETTING(      0x3000, "20" )
	PORT_DIPSETTING(      0x2000, "75" )
	PORT_DIPSETTING(      0x1000, "100" )
	PORT_DIPSETTING(      0x0000, "500" )
	PORT_DIPNAME( 0x8000, 0x8000, "Auto Play" ) PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x0001, 0x0001, "Non Stop Spin" ) PORT_DIPLOCATION("SW5:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Reel Speed" ) PORT_DIPLOCATION("SW5:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Low ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( High ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Bonus Mode" ) PORT_DIPLOCATION("SW5:3")
	PORT_DIPSETTING(      0x0004, "6 3 1" )
	PORT_DIPSETTING(      0x0000, "9 6 1" )
	PORT_DIPNAME( 0x0008, 0x0008, "All Bee Bonus Min." ) PORT_DIPLOCATION("SW5:4")
	PORT_DIPSETTING(      0x0008, "5000" )
	PORT_DIPSETTING(      0x0000, "10000" )
	PORT_DIPNAME( 0x0010, 0x0010, "All Seven" ) PORT_DIPLOCATION("SW5:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0060, 0x0060, "All Bee Play Min." ) PORT_DIPLOCATION("SW5:6,7")
	PORT_DIPSETTING(      0x0060, "8" )
	PORT_DIPSETTING(      0x0040, "16" )
	PORT_DIPSETTING(      0x0020, "32" )
	PORT_DIPSETTING(      0x0000, "64" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW5:8") // effect not shown in test mode
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "Score" )  PORT_DIPLOCATION("SW6:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "Game Count" )  PORT_DIPLOCATION("SW6:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, "10 Time Feature" )  PORT_DIPLOCATION("SW6:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1800, 0x1800, "Win Mode" )  PORT_DIPLOCATION("SW6:4,5")
	PORT_DIPSETTING(      0x1800, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, "L Win Mode" )
	PORT_DIPSETTING(      0x0800, "W Win Mode" )
	PORT_DIPSETTING(      0x0000, "W Win Mode (duplicate)" )
	PORT_DIPNAME( 0x2000, 0x2000, "Play Score Credit" )  PORT_DIPLOCATION("SW6:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW6:7") // effect not shown in test mode
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW6:8") // effect not shown in test mode
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static GFXDECODE_START( gfx_cle68k ) // TODO: correct decoding
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x4_packed_msb, 0x000, 16 )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x4_packed_msb, 0x100, 16 )
GFXDECODE_END


void cle68k_state::cle68k(machine_config &config)
{
	M68000(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &cle68k_state::program_map);
	m_maincpu->set_vblank_int("screen", FUNC(cle68k_state::irq1_line_hold));

	// AT90S4414 (needs core and dumps)

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0, 64*8-1, 0, 32*8-1);
	screen.set_screen_update(FUNC(cle68k_state::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_cle68k);

	PALETTE(config, "palette").set_entries(0x200); // TODO

	RAMDAC(config, m_ramdac[0], 0, "palette");
	m_ramdac[0]->set_addrmap(0, &cle68k_state::ramdac_map<0>);
	m_ramdac[0]->set_color_base(0);

	RAMDAC(config, m_ramdac[1], 0, "palette");
	m_ramdac[1]->set_addrmap(0, &cle68k_state::ramdac_map<1>);
	m_ramdac[1]->set_color_base(0x100);

	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 12_MHz_XTAL / 12, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // pin 7 and clock not verified
}


ROM_START( dmndhrt )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "diamond_heart_u.s.a_u8.u8", 0x00000, 0x20000, CRC(d1f340ce) SHA1(7567448c8694bb24f7957bb461d3be51d138634a) )
	ROM_LOAD16_BYTE( "diamond_heart_u.s.a_u3.u3", 0x00001, 0x20000, CRC(78885bb8) SHA1(51e360036d32b609b4036be086549c011ab41fe3) )

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "at90s4414.u51", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD16_BYTE( "diamond_heart_u.s.a_u10.u10", 0x00000, 0x80000, CRC(00b691a7) SHA1(8cc530ad204cf9168d59419a01abf338c46a49e1) )
	ROM_LOAD16_BYTE( "diamond_heart_u.s.a_u11.u11", 0x00001, 0x80000, CRC(2c666c44) SHA1(15c8e97900444046adb9455bfa827735c226a727) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "diamond_heart_u.s.a_u33.u33", 0x00000, 0x40000, CRC(63b0bc97) SHA1(12adb70a8283c6fec10e2221f1216a7fbfc99355) )

	ROM_REGION( 0x800, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "palce20v8h.pl4", 0x000, 0x157, NO_DUMP )
	ROM_LOAD( "palce20v8h.pl5", 0x200, 0x157, NO_DUMP )
	ROM_LOAD( "palce20v8h.pl6", 0x400, 0x157, NO_DUMP )
	ROM_LOAD( "palce20v8h.pl7", 0x600, 0x157, NO_DUMP )
ROM_END

ROM_START( dmndhrtp ) // u51 was scratched for this set but believed to be AT90S4414, too
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "8.u8", 0x00000, 0x40000, CRC(6fe1a75a) SHA1(373eab66823a5e06bc00fab0ba541a20c9db9505) )
	ROM_LOAD16_BYTE( "3.u3", 0x00001, 0x40000, CRC(89440d16) SHA1(f81c6188a073794f4dd214e45200d8c60569dc03) )

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "diamond_heart_v20_u51.u51", 0x0000, 0x1000, NO_DUMP ) // tried to read as at90s4414. Programmer said 'Device is secured.(Lock bit 1 and 2)'

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD16_BYTE( "11_13ba.u10", 0x00000, 0x80000, CRC(f664cf59) SHA1(cd10fb43562dfec5481994fe612f1bbef1b168b8) ) // handwritten label
	ROM_LOAD16_BYTE( "11_52b.u11",  0x00001, 0x80000, CRC(c0a28638) SHA1(02b348ccf982f486716e394da7a0b7334c7028c2) ) // handwritten label

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "m27c2001.u33", 0x00000, 0x40000, CRC(63b0bc97) SHA1(12adb70a8283c6fec10e2221f1216a7fbfc99355) ) // no sticker, same contents as dmndhrt

	ROM_REGION( 0x800, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "palce20v8h-15pc-4.pl4", 0x000, 0x157, NO_DUMP )
	ROM_LOAD( "palce20v8h-15pc-4.pl5", 0x200, 0x157, NO_DUMP )
	ROM_LOAD( "palce20v8h-15pc-4.pl6", 0x400, 0x157, NO_DUMP )
	ROM_LOAD( "palce20v8h-25pc-4.pl7", 0x600, 0x157, NO_DUMP )
ROM_END

ROM_START( dmndhrtn ) // u51 was scratched for this set but believed to be AT90S4414, too
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "w27e010.u8", 0x00000, 0x20000, CRC(2a1ba91e) SHA1(af340d9e0aa7874669557067a9e043eecdf5301b) ) // no sticker
	ROM_LOAD16_BYTE( "w27e010.u3", 0x00001, 0x20000, CRC(cdb26ff2) SHA1(33ddda977a5f6436a690fa53763f36c7e6acfb94) ) // no sticker

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "at90s4414.u51", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD16_BYTE( "diamond_heart_new_mon_v20.0_u10.u10", 0x00000, 0x80000, CRC(7525bd95) SHA1(b34ab59bde9ecdfe03489a6eceda2c95afdee6c8) )
	ROM_LOAD16_BYTE( "diamond_heart_new_mon_v20.0_u11.u11", 0x00001, 0x80000, CRC(1ffc66a6) SHA1(fd5bfa9ec01ad7aa3060929dbce417babe241700) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "f29c51002t.u33", 0x00000, 0x40000, CRC(97f774cd) SHA1(7c5a1c4a0e7cfb71e24d174c43a83735abfc59c8) ) // no sticker

	ROM_REGION( 0x800, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "gal20v8b.pl4", 0x000, 0x157, NO_DUMP )
	ROM_LOAD( "gal20v8b.pl5", 0x200, 0x157, NO_DUMP )
	ROM_LOAD( "gal20v8b.pl6", 0x400, 0x157, NO_DUMP )
	ROM_LOAD( "gal20v8b.pl7", 0x600, 0x157, NO_DUMP )
ROM_END

ROM_START( honeybee ) // u51 was scratched for this set but believed to be AT90S4414, too
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "honey_bee_hb_tw_u8.u8", 0x00000, 0x40000, CRC(1e7e53a3) SHA1(30d426cca499adf82338ba6cc1391f754e908a5b) )
	ROM_LOAD16_BYTE( "honey_bee_hb_tw_u3.u3", 0x00001, 0x40000, CRC(0ed5f0cc) SHA1(f64c27f04f74162027070f889daaec6f1847f19e) )

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "at90s4414.u51", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD16_BYTE( "honey_bee_hb_tw_u10.u10", 0x00000, 0x80000, CRC(40526fe1) SHA1(58a3a16c4dc0fa6527571b924f43377657f0cc76) )
	ROM_LOAD16_BYTE( "honey_bee_hb_tw_u11.u11", 0x00001, 0x80000, CRC(3036a082) SHA1(16393fac3ccd5c2fc6ab9fd11f8530aace94e4fc) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "honey_bee_hb_tw_u33.u33", 0x00000, 0x40000, CRC(a85f1bfc) SHA1(c2b83a2570280a43241b89fdb21e87c8cf033409) )

	// PAL locations not readable
	ROM_REGION( 0x800, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "gal20v8b.pl4", 0x000, 0x157, NO_DUMP )
	ROM_LOAD( "gal20v8b.pl5", 0x200, 0x157, NO_DUMP )
	ROM_LOAD( "gal20v8b.pl6", 0x400, 0x157, NO_DUMP )
	ROM_LOAD( "gal20v8b.pl7", 0x600, 0x157, NO_DUMP )
ROM_END

} // anonymous namespace


GAME( 2001, dmndhrt,  0, cle68k, dmndhrt,  cle68k_state, empty_init, ROT0, "LSE", "Diamond Heart (v1.06)",      MACHINE_NOT_WORKING ) // 2001/02/15
GAME( 2001, dmndhrtp, 0, cle68k, dmndhrtn, cle68k_state, empty_init, ROT0, "LSE", "Diamond Heart Plus (v18.0)", MACHINE_NOT_WORKING ) // 2001/02/15
GAME( 2003, dmndhrtn, 0, cle68k, dmndhrtn, cle68k_state, empty_init, ROT0, "CLE", "Diamond Heart New (v20.0)",  MACHINE_NOT_WORKING ) // 2003/04/25
GAME( 2004, honeybee, 0, cle68k, honeybee, cle68k_state, empty_init, ROT0, "LSE", "Honey-Bee (v3.0)",           MACHINE_NOT_WORKING ) // 2004/07/01

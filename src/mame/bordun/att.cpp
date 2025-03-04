// license:BSD-3-Clause
// copyright-holders:

/*
Bordun poker games on ATT / ATT III PCBs


TODO:
* this is basically misc/skylncr.cpp without reels and PPIs and with more advanced sound. Merge?
* outputs
* lianhp3: title screen says 2003TM but PCB is from 2010? Was this really released in 2003?

BTANB:
* sound test for lianhp3 doesn't work. It's still coded to send sound commands to 0xd0, but this games
  doesn't have an audio CPU.
*/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"
#include "video/ramdac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class att_state : public driver_device
{
public:
	att_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram")
	{ }

	void att(machine_config &config) ATTR_COLD;
	void att3(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;

	tilemap_t *m_tilemap = nullptr;

	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
	void att_io_map(address_map &map) ATTR_COLD;
	void att3_io_map(address_map &map) ATTR_COLD;
	void att_audio_program_map(address_map &map) ATTR_COLD;
	void att_audio_io_map(address_map &map) ATTR_COLD;
	void ramdac_map(address_map &map) ATTR_COLD;
};


void att_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

void att_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(att_state::get_tile_info)
{
	uint16_t const code = m_videoram[tile_index] + (m_colorram[tile_index] << 8);
	int const pal = (code & 0x8000) >> 15;
	tileinfo.set(0, code, pal ^ 1, TILE_FLIPYX( 0 ));
}

uint32_t att_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

void att_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(att_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
}


void att_state::program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram().share("nvram");
	map(0x8800, 0x8fff).ram().w(FUNC(att_state::videoram_w)).share(m_videoram);
	map(0x9000, 0x97ff).ram().w(FUNC(att_state::colorram_w)).share(m_colorram);
}

void att_state::att_io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();

	map(0x00, 0x00).portr("IN0");
	map(0x01, 0x01).portr("IN1");
	map(0x02, 0x02).portr("DSW3");
	map(0x03, 0x03).portr("DSW2");
	map(0x10, 0x10).portr("DSW4");
	map(0x11, 0x11).portr("IN2");
	map(0x12, 0x12).portr("DSW1");
	// map(0x20, 0x20) // TODO: coin counter / outputs?
	map(0x40, 0x40).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x41, 0x41).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x42, 0x42).w("ramdac", FUNC(ramdac_device::mask_w));
	map(0x70, 0x70).nopw();  // TODO: NMI clear? NOPed for now as it spams the log
	map(0xd0, 0xd0).w("soundlatch", FUNC(generic_latch_8_device::write));
}

void att_state::att3_io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();

	map(0x00, 0x00).portr("IN0");
	map(0x01, 0x01).portr("IN1");
	map(0x02, 0x02).portr("DSW3");
	map(0x03, 0x03).portr("DSW2");
	map(0x10, 0x10).portr("DSW4");
	map(0x11, 0x11).portr("IN2");
	map(0x12, 0x12).portr("DSW1");
	// map(0x20, 0x20) // TODO: coin counter / outputs?
	map(0x40, 0x40).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x41, 0x41).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x42, 0x42).w("ramdac", FUNC(ramdac_device::mask_w));
	map(0x70, 0x70).nopw();  // TODO: NMI clear? NOPed for now as it spams the log
	map(0xf6, 0xf6).lw8(NAME([this] (uint8_t data) { if (data != 0x00) logerror("Oki bank: %02x\n", data); }));  // TODO: Oki bank select?
	map(0xf7, 0xf7).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

void att_state::att_audio_program_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x41ff).ram();
	map(0xf800, 0xffff).ram();
}

void att_state::att_audio_io_map(address_map &map)
{
	// TODO: lots of reads / writes
	map.global_mask(0xff);

	map(0x84, 0x84).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x90, 0x91).w("ym2413", FUNC(ym2413_device::write));
}


void att_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}


static INPUT_PORTS_START( lianhp2 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // active high or it stops after RAM check
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )

	PORT_START("IN1")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no evident effect
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no evident effect
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no evident effect
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no evident effect
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no evident effect
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no evident effect
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no evident effect

	// SW1 on PCB is the reset button and SW2-5 are the DIP switches
	// test mode shows SW1-4 for the DIPs
	// thus to avoid user confusion let's follow the test mode

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Max Bet" ) PORT_DIPLOCATION("SW1:8,7") // hard-coded to 100?
	PORT_DIPSETTING(    0x03, "100" )
	PORT_DIPSETTING(    0x02, "100 (duplicate)" )
	PORT_DIPSETTING(    0x01, "100 (duplicate)" )
	PORT_DIPSETTING(    0x00, "100 (duplicate)" )
	PORT_DIPNAME( 0x04, 0x04, "Clear Point As" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, "Create Point" )
	PORT_DIPSETTING(    0x00, "Coin" )
	PORT_DIPNAME( 0x08, 0x08, "Bonus Rate" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "32" )
	PORT_DIPSETTING(    0x08, "48" )
	PORT_DIPNAME( 0x10, 0x10, "Straight Double Up" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Double Up" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Card Type" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, "Poker" )
	PORT_DIPSETTING(    0x00, "Animals" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:1") // not shown in test mode
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Begin Bonus" ) PORT_DIPLOCATION("SW2:8,7")
	PORT_DIPSETTING(    0x03, "2000" )
	PORT_DIPSETTING(    0x02, "4000" )
	PORT_DIPSETTING(    0x01, "6000" )
	PORT_DIPSETTING(    0x00, "8000" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6") // not shown in test mode
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5") // not shown in test mode
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4") // not shown in test mode
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(    0xe0, "1" )
	PORT_DIPSETTING(    0xc0, "5" )
	PORT_DIPSETTING(    0xa0, "10" )
	PORT_DIPSETTING(    0x80, "25" )
	PORT_DIPSETTING(    0x60, "50" )
	PORT_DIPSETTING(    0x40, "100" )
	PORT_DIPSETTING(    0x20, "250" )
	PORT_DIPSETTING(    0x00, "500" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Min Bet" ) PORT_DIPLOCATION("SW3:8,7")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x1c, 0x1c, "Double Up Rate" ) PORT_DIPLOCATION("SW3:6,5,4")
	PORT_DIPSETTING(    0x00, "86" )
	PORT_DIPSETTING(    0x04, "87" )
	PORT_DIPSETTING(    0x08, "88" )
	PORT_DIPSETTING(    0x0c, "89" )
	PORT_DIPSETTING(    0x10, "90" )
	PORT_DIPSETTING(    0x14, "91" )
	PORT_DIPSETTING(    0x18, "92" )
	PORT_DIPSETTING(    0x1c, "93" )
	PORT_DIPNAME( 0xe0, 0xe0, "Play Rate" ) PORT_DIPLOCATION("SW3:3,2,1") // hard-coded to 100?
	PORT_DIPSETTING(    0xe0, "100" )
	PORT_DIPSETTING(    0xc0, "100 (duplicate)" )
	PORT_DIPSETTING(    0xa0, "100 (duplicate)" )
	PORT_DIPSETTING(    0x80, "100 (duplicate)" )
	PORT_DIPSETTING(    0x60, "100 (duplicate)" )
	PORT_DIPSETTING(    0x40, "100 (duplicate)" )
	PORT_DIPSETTING(    0x20, "100 (duplicate)" )
	PORT_DIPSETTING(    0x00, "100 (duplicate)" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x03, 0x03, "4 K Rate" ) PORT_DIPLOCATION("SW4:8,7")
	PORT_DIPSETTING(    0x00, "70 (duplicate)" )
	PORT_DIPSETTING(    0x01, "70" )
	PORT_DIPSETTING(    0x02, "80" )
	PORT_DIPSETTING(    0x03, "90" )
	PORT_DIPNAME( 0x0c, 0x0c, "S F Rate" ) PORT_DIPLOCATION("SW4:6,5")
	PORT_DIPSETTING(    0x00, "70 (duplicate)" )
	PORT_DIPSETTING(    0x04, "70" )
	PORT_DIPSETTING(    0x08, "80" )
	PORT_DIPSETTING(    0x0c, "90" )
	PORT_DIPNAME( 0x30, 0x30, "R S Rate" ) PORT_DIPLOCATION("SW4:4,3")
	PORT_DIPSETTING(    0x00, "70 (duplicate)" )
	PORT_DIPSETTING(    0x10, "70" )
	PORT_DIPSETTING(    0x20, "80" )
	PORT_DIPSETTING(    0x30, "90" )
	PORT_DIPNAME( 0xc0, 0xc0, "5 K Rate" ) PORT_DIPLOCATION("SW4:2,1")
	PORT_DIPSETTING(    0x00, "70 (duplicate)" )
	PORT_DIPSETTING(    0x40, "70" )
	PORT_DIPSETTING(    0x80, "80" )
	PORT_DIPSETTING(    0xc0, "90" )
INPUT_PORTS_END

static INPUT_PORTS_START( lianhp3 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )

	PORT_START("IN1")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(4)

	// SW1 on PCB is the reset button and SW2-5 are the DIP switches
	// test mode shows SW1-4 for the DIPs
	// thus to avoid user confusion let's follow the test mode

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Max Bet" ) PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(    0x03, "100" )
	PORT_DIPSETTING(    0x02, "500" )
	PORT_DIPSETTING(    0x01, "1000" )
	PORT_DIPSETTING(    0x00, "1500" )
	PORT_DIPNAME( 0x04, 0x04, "Clear Point As" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, "Coin" )
	PORT_DIPSETTING(    0x00, "Create Point" )
	PORT_DIPNAME( 0x08, 0x08, "Bonus Rate" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "32" )
	PORT_DIPSETTING(    0x08, "48" )
	PORT_DIPNAME( 0x10, 0x10, "Straight Double Up" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Double Up" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Card Type" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, "Poker" )
	PORT_DIPSETTING(    0x00, "Animals" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:1") // not shown in test mode
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Begin Bonus" ) PORT_DIPLOCATION("SW2:8,7")
	PORT_DIPSETTING(    0x03, "2000" )
	PORT_DIPSETTING(    0x02, "4000" )
	PORT_DIPSETTING(    0x01, "6000" )
	PORT_DIPSETTING(    0x00, "8000" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6") // not shown in test mode
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5") // not shown in test mode
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4") // not shown in test mode
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(    0xe0, "10" )
	PORT_DIPSETTING(    0xc0, "50" )
	PORT_DIPSETTING(    0xa0, "100" )
	PORT_DIPSETTING(    0x80, "250" )
	PORT_DIPSETTING(    0x60, "500" )
	PORT_DIPSETTING(    0x40, "1000" )
	PORT_DIPSETTING(    0x20, "2500" )
	PORT_DIPSETTING(    0x00, "5000" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Min Bet" ) PORT_DIPLOCATION("SW3:8,7")
	PORT_DIPSETTING(    0x03, "10" )
	PORT_DIPSETTING(    0x02, "20" )
	PORT_DIPSETTING(    0x01, "40" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPNAME( 0x1c, 0x1c, "Double Up Rate" ) PORT_DIPLOCATION("SW3:6,5,4")
	PORT_DIPSETTING(    0x00, "86" )
	PORT_DIPSETTING(    0x04, "87" )
	PORT_DIPSETTING(    0x08, "88" )
	PORT_DIPSETTING(    0x0c, "89" )
	PORT_DIPSETTING(    0x10, "90" )
	PORT_DIPSETTING(    0x14, "91" )
	PORT_DIPSETTING(    0x18, "92" )
	PORT_DIPSETTING(    0x1c, "93" )
	PORT_DIPNAME( 0xe0, 0xe0, "Play Rate" ) PORT_DIPLOCATION("SW3:3,2,1")
	PORT_DIPSETTING(    0xe0, "0" )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0xa0, "10" )
	PORT_DIPSETTING(    0x80, "11" )
	PORT_DIPSETTING(    0x60, "100" )
	PORT_DIPSETTING(    0x40, "101" )
	PORT_DIPSETTING(    0x20, "110" )
	PORT_DIPSETTING(    0x00, "111" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x03, 0x03, "4 K Rate" ) PORT_DIPLOCATION("SW4:8,7")
	PORT_DIPSETTING(    0x00, "70 (duplicate)" )
	PORT_DIPSETTING(    0x01, "70" )
	PORT_DIPSETTING(    0x02, "80" )
	PORT_DIPSETTING(    0x03, "90" )
	PORT_DIPNAME( 0x0c, 0x0c, "S F Rate" ) PORT_DIPLOCATION("SW4:6,5")
	PORT_DIPSETTING(    0x00, "70 (duplicate)" )
	PORT_DIPSETTING(    0x04, "70" )
	PORT_DIPSETTING(    0x08, "80" )
	PORT_DIPSETTING(    0x0c, "90" )
	PORT_DIPNAME( 0x30, 0x30, "R S Rate" ) PORT_DIPLOCATION("SW4:4,3")
	PORT_DIPSETTING(    0x00, "70 (duplicate)" )
	PORT_DIPSETTING(    0x10, "70" )
	PORT_DIPSETTING(    0x20, "80" )
	PORT_DIPSETTING(    0x30, "90" )
	PORT_DIPNAME( 0xc0, 0xc0, "5 K Rate" ) PORT_DIPLOCATION("SW4:2,1")
	PORT_DIPSETTING(    0x00, "70 (duplicate)" )
	PORT_DIPSETTING(    0x40, "70" )
	PORT_DIPSETTING(    0x80, "80" )
	PORT_DIPSETTING(    0xc0, "90" )
INPUT_PORTS_END


static const gfx_layout layout8x8x8 =
{
	8,8,
	RGN_FRAC(1,2),
	8,
	{ STEP8(0,1) },
	{
		8*0,8*1,
		RGN_FRAC(1,2)+8*0,RGN_FRAC(1,2)+8*1,
		8*2,8*3,
		RGN_FRAC(1,2)+8*2,RGN_FRAC(1,2)+8*3
	},
	{ STEP8(0,8*4) },
	8*8*4
};


static GFXDECODE_START( gfx_att )
	GFXDECODE_ENTRY( "tiles", 0, layout8x8x8, 0, 1 )
GFXDECODE_END

static GFXDECODE_START( gfx_att3 )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x8_raw, 0, 1 )
GFXDECODE_END


void att_state::att(machine_config &config)
{
	Z80(config, m_maincpu, 12_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &att_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &att_state::att_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(att_state::nmi_line_pulse));

	z80_device &audiocpu(Z80(config, "audiocpu", 12_MHz_XTAL / 4));
	audiocpu.set_addrmap(AS_PROGRAM, &att_state::att_audio_program_map);
	audiocpu.set_addrmap(AS_IO, &att_state::att_audio_io_map);
	audiocpu.set_periodic_int(FUNC(att_state::irq0_line_hold), attotime::from_hz(4 * 60)); // TODO: find IRQ source

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_screen_update(FUNC(att_state::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_att);

	PALETTE(config, "palette").set_entries(0x100);

	ramdac_device &ramdac(RAMDAC(config, "ramdac", 0, "palette"));
	ramdac.set_addrmap(0, &att_state::ramdac_map);

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline("audiocpu", INPUT_LINE_NMI);

	SPEAKER(config, "mono").front_center();

	YM2413(config, "ym2413", 3.579545_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void att_state::att3(machine_config &config)
{
	Z80(config, m_maincpu, 12_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &att_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &att_state::att3_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(att_state::nmi_line_pulse));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_screen_update(FUNC(att_state::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_att3);

	PALETTE(config, "palette").set_entries(0x100);

	ramdac_device &ramdac(RAMDAC(config, "ramdac", 0, "palette"));
	ramdac.set_addrmap(0, &att_state::ramdac_map);

	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 12_MHz_XTAL / 12, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);
}


/*
Lian Huan Pao, ATT III, 2010
Hardware Info By Guru
---------------------

ATT
  |--------------------------------|
|-| LM386 VOL   61256          Z80A|
|1  78L05                          |
|0                       GAL  GAL  |
|W  SW4       D32-06               |
|A  SW3                  UM70C171-08
|Y  SW2     3.579545MHz            |
|-| SW1                            |
  |         YM2413         D32-05  |
  | ULN2003S                       |
  |                        D32-04  |
|-|                                |
|2                         D32-03  |
|2                                 |
|W                         D32-02  |
|A      T518B                      |
|Y                            61256|
|        12MHz       EPM7128       |
|-|      D32-01               61256|
  |      BATT                      |
  |BUTTON    6116              Z80B|
  |--------------------------------|
Notes:
       Z80A - Sharp LH0080A Z80A CPU. Clock Input 3.000MHz [12/4]
       Z80B - Sharp LH0080B Z80B CPU. Clock Input 3.000MHz [12/4]
     YM2413 - Yamaha YM2413 OPLL FM Sound Chip. Clock Input 3.579545MHz
      LM386 - LM386 0.5W Audio Power Amplifier
      61256 - 32kB x8-bit SRAM
       6116 - 6116 2kB x8-bit SRAM (Battery-Backed)
      78L05 - 5V Linear Regulator (TO92)
      T518B - Mitsumi T518B Reset Chip
     BUTTON - Push Button Reset and Clear NVRAM
       D32* - ROMs
   ULN2003S - 7-Channel Darlington Transistor Array (SOIC16)
   UM70C171 - UMC UM70C171-80 Color Palette With Triple 6-Bit DAC
      SW1-4 - 8-Position DIP Switch
    EPM7128 - Altera MAX EPM7128 CPLD
       BATT - CR2032 Coin Cell
*/

ROM_START( lianhp2 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "d32-01-.u32", 0x0000, 0x8000, CRC(7f6f1b84) SHA1(daee04a8f77580e8aad99498bbbed14d7b0cde22) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "d32-06-.u12", 0x0000, 0x8000, CRC(988515b1) SHA1(e833077885784c420c96903e46217a4292c9e783) ) // 1xxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD16_BYTE( "d32-02-.u4", 0x00000, 0x20000, CRC(867258ad) SHA1(f4047d3c921263d0df34d4357203b3e15c8c8de2) ) // 1xxxxxxxxxxxxxxxxx = 0xFF
	ROM_IGNORE(                             0x20000 )
	ROM_LOAD16_BYTE( "d32-03-.u5", 0x00001, 0x20000, CRC(93574235) SHA1(0e82b52f4169e146d1b4386a68a1a023d31da6fd) ) // 1xxxxxxxxxxxxxxxxx = 0xFF
	ROM_IGNORE(                             0x20000 )
	ROM_LOAD16_BYTE( "d32-04-.u6", 0x40000, 0x20000, CRC(72915ae7) SHA1(8dedcb4f7c57896bf27743f96afbdcc298e06a47) ) // 1xxxxxxxxxxxxxxxxx = 0xFF
	ROM_IGNORE(                             0x20000 )
	ROM_LOAD16_BYTE( "d32-05-.u7", 0x40001, 0x20000, CRC(aab2bd04) SHA1(18dcb9a2d7338bb98b5399c30cfd6ad8ed750cce) ) // 1xxxxxxxxxxxxxxxxx = 0xFF
	ROM_IGNORE(                             0x20000 )
ROM_END

/*
Lian Huan Pao 3, Bordun, 2003?
Hardware Info By Guru
---------------------

TAIWAN ATT III
2010.12.18
030514
  |--------------------------------|
|-| UPC1241H VOL          ATT_III_1|
|1    C1815    78L05               |
|0                    6295         |
|W  86171                          |
|A                        ATT_III_2|
|Y             A1117_3.3           |
|-|                                |
  |                                |
  |                                |
  |  12MHz             |---------| |
|-|                    |         | |
|2                     |  ACTEL  | |
|2   ULN2003           |A54SX16A | |
|W JC817               |-F QFP208| |
|A JC817               |---------| |
|Y JC817                     SW2   |
|  JC817                     SW3   |
|-|JC817     Z80      24257  SW4   |
  |   SW1         6116       SW5   |
  |    BATT   ATT_III_3    T518B   |
  |--------------------------------|
Notes:
        Z80 - Toshiba TMPZ84C000AP-6 Z80B CPU. Clock Input 6.000MHz [12/2]
       6295 - Oki M6295 ADPCM Sample Player. Clock Input 1.000MHz [12/12]. Pin 7 HIGH
   UPC1241H - NEC uPC1241H 7W Audio Power Amplifier
      24257 - Winbond W24257ak-15 32kB x8-bit SRAM
       6116 - 6116 2kB x8-bit SRAM (Battery-Backed)
      78L05 - 5V Linear Regulator (TO92)
      T518B - Mitsumi T518B Reset Chip
   ATT_III* - ROMs; 3=Z80 Program, 2=Graphics, 1=Oki Samples
    ULN2003 - 7-Channel Darlington Transistor Array
      86171 - HMC HM86171-80 Color Palette With Triple 6-Bit DAC
        SW1 - Push Button Reset and Clear NVRAM
      SW2-5 - 8-Position DIP Switch
      ACTEL - Actel A54SX16A-F QFP208 FPGA
       BATT - 3.6V Nicad Barrel Battery
      C1815 - 2SC1815 General-Purpose NPN Transistor
  A1117_3.3 - 3.3V Linear Regulator for powering FPGA
      JC817 - Kento JC817 Photo-Coupler (equivalent to Sharp PC817X)
*/

ROM_START( lianhp3 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "att-iii_3.bin", 0x0000, 0x8000, CRC(9041bfa9) SHA1(047046f8c533ed00a06e2d3cb6328a517ba1e6c2) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "att-iii_2.bin", 0x00000, 0x80000, CRC(53baa37b) SHA1(548a9d6e01d31d65bd2c35be16778cbc5a315b54) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "att-iii_1.bin", 0x00000, 0x80000, CRC(05b3974c) SHA1(70feb9c9fc8d7619838319696e10d65ef55dadfc) )
ROM_END

} // anonymous namespace


GAME( 200?, lianhp2, 0, att,  lianhp2, att_state, empty_init, ROT0, "Bordun", "Lian Huan Pao - ATT II",  MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // audio IRQ timing
GAME( 2003, lianhp3, 0, att3, lianhp3, att_state, empty_init, ROT0, "Bordun", "Lian Huan Pao - ATT III", MACHINE_SUPPORTS_SAVE )

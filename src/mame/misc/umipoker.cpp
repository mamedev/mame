// license:BSD-3-Clause
// copyright-holders: Angelo Salese, Roberto Fresca

/***************************************************************************

    Umi de Poker (c) 1997 World Station Co.,LTD
    Slot Poker Saiyuki (c) 1998 World Station Co.,LTD

    Undumped games running on the same hardware:
    * Baccara Star (World-One)
    * Baccarat Special (World-One)

    Driver by Angelo Salese.
    Additional work by Roberto Fresca.

    TODO:
    - Verify clocks (XTALs are 14.3181 and 2.000MHz)

    TMP68HC000-16 + z80 + YM3812 + OKI6295

***************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "saiyukip.lh"


namespace {

class umipoker_state : public driver_device
{
public:
	umipoker_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_vram(*this, "vram.%u", 0U)
		, m_z80_wram(*this, "z80_wram")
		, m_z80_rom(*this, "audiocpu")
	{
	}

	void umipoker(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;

	void main_map(address_map &map) ATTR_COLD;

private:
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr_array<uint16_t, 4> m_vram;
	required_shared_ptr<uint8_t> m_z80_wram;
	required_region_ptr<uint8_t> m_z80_rom;

	tilemap_t *m_tilemap[4]{};
	uint16_t m_scrolly[4]{};

	uint8_t z80_rom_readback_r(offs_t offset);
	uint8_t z80_shared_ram_r(offs_t offset);
	void z80_shared_ram_w(offs_t offset, uint8_t data);
	void irq_ack_w(uint16_t data);
	template<uint8_t Which> void scrolly_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template<uint8_t Which> void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void umi_counters_w(uint16_t data);
	template<uint8_t Which> TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void audio_io_map(address_map &map) ATTR_COLD;
	void audio_map(address_map &map) ATTR_COLD;
};

class saiyukip_state : public umipoker_state
{
public:
	saiyukip_state(const machine_config &mconfig, device_type type, const char *tag)
		: umipoker_state(mconfig, type, tag)
		, m_lamps(*this, "lamp%u", 0U)
	{
	}

	void saiyukip(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void lamps_w(uint16_t data);
	void saiyu_counters_w(uint16_t data);

	void main_map(address_map &map) ATTR_COLD;

	output_finder<6> m_lamps;
};

template<uint8_t Which>
TILE_GET_INFO_MEMBER(umipoker_state::get_tile_info)
{
	int const tile = m_vram[Which][tile_index * 2 + 0];
	int const color = m_vram[Which][tile_index * 2 + 1] & 0x3f;

	tileinfo.set(0, tile, color, 0);
}

void umipoker_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(umipoker_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(umipoker_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(umipoker_state::get_tile_info<2>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(umipoker_state::get_tile_info<3>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_tilemap[0]->set_transparent_pen(0);
	m_tilemap[1]->set_transparent_pen(0);
	m_tilemap[2]->set_transparent_pen(0);
	m_tilemap[3]->set_transparent_pen(0);

	save_item(NAME(m_scrolly));
	std::fill(std::begin(m_scrolly), std::end(m_scrolly), 0);
}

uint32_t umipoker_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap[0]->set_scrolly(0, m_scrolly[0]);
	m_tilemap[1]->set_scrolly(0, m_scrolly[1]);
	m_tilemap[2]->set_scrolly(0, m_scrolly[2]);
	m_tilemap[3]->set_scrolly(0, m_scrolly[3]);

	bitmap.fill(m_palette->black_pen(), cliprect);

	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[2]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[3]->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

uint8_t umipoker_state::z80_rom_readback_r(offs_t offset)
{
	return m_z80_rom[offset];
}

uint8_t umipoker_state::z80_shared_ram_r(offs_t offset)
{
	machine().scheduler().synchronize(); // force resync

	return m_z80_wram[offset];
}

void umipoker_state::z80_shared_ram_w(offs_t offset, uint8_t data)
{
	machine().scheduler().synchronize(); // force resync

	m_z80_wram[offset] = data;
}

void umipoker_state::irq_ack_w(uint16_t data)
{
	m_maincpu->set_input_line(6, CLEAR_LINE);

	// shouldn't happen
	if (data)
		popmessage("%04x IRQ ACK, contact MAMEdev", data);
}

template<uint8_t Which>
void umipoker_state::scrolly_w(offs_t offset, uint16_t data, uint16_t mem_mask) { COMBINE_DATA(&m_scrolly[Which]); }

template<uint8_t Which>
void umipoker_state::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vram[Which][offset]);
	m_tilemap[Which]->mark_tile_dirty(offset >> 1);
}

void saiyukip_state::lamps_w(uint16_t data)
{
/*
  Umipoker buttons layout:
  .--------.  .--------.  .--------.  .--------.  .--------.  .--------.
  |        |  |        |  |        |  |        |  | START  |  |  BET   |
  | DOUBLE |  |  TAKE  |  |  BIG   |  | SMALL  |  |        |  |        |
  |        |  |        |  |        |  |        |  |  SKIP  |  | RAISE  |
  '--------'  '--------'  '--------'  '--------'  '--------'  '--------'
   (yellow)    (green)      (red)       (red)      (green)     (yellow)

  Saiyukip buttons layout:
  .--------.  .--------.  .--------.  .--------.  .--------.  .--------.
  |        |  |        |  |  LEFT  |  |        |  |        |  |        |
  | DOUBLE |  |  TAKE  |  |        |  | RIGHT  |  | START  |  |  BET   |
  |        |  |        |  |  STOP  |  |        |  |        |  |        |
  '--------'  '--------'  '--------'  '--------'  '--------'  '--------'
   (yellow)    (green)      (red)       (red)      (green)     (yellow)

  Seems that only saiyukip has programmed lamps.

  0x0000 - Normal State (lamps off).
  0x0020 - RIGHT lamp.
  0x0200 - TAKE lamp.
  0x0400 - D-UP lamp.
  0x0800 - BET lamp.
  0x1000 - LEFT/STOP lamp.
  0x2000 - START lamp.

  - Hbits -/- Lbits -
  7654 3210 7654 3210
  ===================
  xx-- ---x xx-x xxxx  Unknown / Not used.
  ---- ---- --x- ----  RIGHT lamp.
  ---- --x- ---- ----  TAKE lamp.
  ---- -x-- ---- ----  D-UP lamp.
  ---- x--- ---- ----  BET lamp.
  ---x ---- ---- ----  LEFT/STOP lamp.
  --x- ---- ---- ----  START lamp.

*/
	m_lamps[0] = BIT(data, 5);      // Lamp 0 - RIGHT
	m_lamps[1] = BIT(data, 9);      // Lamp 1 - TAKE
	m_lamps[2] = BIT(data, 10);     // Lamp 2 - D-UP
	m_lamps[3] = BIT(data, 11);     // Lamp 3 - BET
	m_lamps[4] = BIT(data, 12);     // Lamp 4 - LEFT/STOP
	m_lamps[5] = BIT(data, 13);     // Lamp 5 - START
}

void umipoker_state::umi_counters_w(uint16_t data)
{/*
  0x0000 - Normal State (lamps off).
  0x0001 - Payout pulse.
  0x0020 - Coin 1.
  0x0040 - Coin 2, 3 and Remote.

  - Hbits -/- Lbits -
  7654 3210 7654 3210
  ===================
  ---- ---- ---- ---x  Payout pulse.
  ---- ---- --x- ----  Coin 1.
  ---- ---- -x-- ----  Coin 2, 3 and Remote.
  xxxx xxxx x--x xxx-  Unknown / Not used.

*/
	machine().bookkeeping().coin_counter_w(0, BIT(data, 5));  // COIN 1
	machine().bookkeeping().coin_counter_w(1, BIT(data, 6));  // COIN 2
	machine().bookkeeping().coin_counter_w(2, BIT(data, 0));  // PAYOUT
}

void saiyukip_state::saiyu_counters_w(uint16_t data)
{
/*
  0x0000 - Normal State (lamps off).
  0x0100 - Payout pulse.
  0x2000 - Coin 1.
  0x4000 - Coin 2, 3 and Remote.

  - Hbits -/- Lbits -
  7654 3210 7654 3210
  ===================
  ---- ---x ---- ----  Payout pulse.
  --x- ---- ---- ----  Coin 1.
  -x-- ---- ---- ----  Coin 2, 3 and Remote.
  x--x xxx- xxxx xxxx  Unknown / Not used.

*/
	machine().bookkeeping().coin_counter_w(0, BIT(data, 13));    // COIN 1
	machine().bookkeeping().coin_counter_w(1, BIT(data, 14));    // COIN 2
	machine().bookkeeping().coin_counter_w(2, BIT(data, 8));     // PAYOUT
}


void umipoker_state::main_map(address_map &map)
{
	map.unmap_value_low();
	map(0x000000, 0x03ffff).rom();
	map(0x400000, 0x403fff).ram().share("nvram");
	map(0x600000, 0x6007ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");    // Palette
	map(0x800000, 0x801fff).ram().w(FUNC(umipoker_state::vram_w<0>)).share(m_vram[0]);
	map(0x802000, 0x803fff).ram().w(FUNC(umipoker_state::vram_w<1>)).share(m_vram[1]);
	map(0x804000, 0x805fff).ram().w(FUNC(umipoker_state::vram_w<2>)).share(m_vram[2]);
	map(0x806000, 0x807fff).ram().w(FUNC(umipoker_state::vram_w<3>)).share(m_vram[3]);
	map(0xc00000, 0xc0ffff).r(FUNC(umipoker_state::z80_rom_readback_r)).umask16(0x00ff);
	map(0xc1f000, 0xc1ffff).rw(FUNC(umipoker_state::z80_shared_ram_r), FUNC(umipoker_state::z80_shared_ram_w)).umask16(0x00ff);
	map(0xe00000, 0xe00001).portr("IN0");
	map(0xe00004, 0xe00005).portr("IN1"); // unused?
	map(0xe00008, 0xe00009).portr("IN2");
	map(0xe00010, 0xe00011).w(FUNC(umipoker_state::umi_counters_w));
//  map(0xe0000c, 0xe0000d).w(FUNC(umipoker_state::lamps_w)); -----> lamps only for saiyukip.
//  map(0xe00010, 0xe00011).w(FUNC(umipoker_state::counters_w)); --> coin counters for both games.
	map(0xe00014, 0xe00015).portr("DSW1-2");
	map(0xe00018, 0xe00019).portr("DSW3-4");
	map(0xe00020, 0xe00021).w(FUNC(umipoker_state::scrolly_w<0>));
	map(0xe00022, 0xe00023).w(FUNC(umipoker_state::irq_ack_w));
	map(0xe00026, 0xe00027).w(FUNC(umipoker_state::scrolly_w<2>));
	map(0xe0002a, 0xe0002b).w(FUNC(umipoker_state::scrolly_w<1>));
	map(0xe0002c, 0xe0002d).nopw(); // unknown meaning, bit 0 goes from 0 -> 1 on IRQ service routine
	map(0xe0002e, 0xe0002f).w(FUNC(umipoker_state::scrolly_w<3>));
}

void saiyukip_state::main_map(address_map &map)
{
	umipoker_state::main_map(map);
	map(0xe0000c, 0xe0000d).w(FUNC(saiyukip_state::lamps_w));
	map(0xe00010, 0xe00011).w(FUNC(saiyukip_state::saiyu_counters_w));
}

void umipoker_state::audio_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xf800, 0xffff).rw(FUNC(umipoker_state::z80_shared_ram_r), FUNC(umipoker_state::z80_shared_ram_w)).share(m_z80_wram);
}

void umipoker_state::audio_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x10, 0x11).rw("ym", FUNC(ym3812_device::read), FUNC(ym3812_device::write));
}


static INPUT_PORTS_START( common )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x4000, IP_ACTIVE_HIGH )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( umipoker )
	PORT_INCLUDE( common )

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_GAMBLE_HIGH ) PORT_NAME("Big")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_GAMBLE_BET )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_GAMBLE_LOW )  PORT_NAME("Small")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

/*  Seems that the default switches are all ON.
    This needs confirmation, since the game title
    by default should be the second one.
*/
	PORT_START("DSW1-2")
	PORT_DIPNAME( 0x0003, 0x0003, "Main Level" )        PORT_DIPLOCATION("DSW1:!1,!2")
	PORT_DIPSETTING(    0x0000, "1" )
	PORT_DIPSETTING(    0x0001, "2" )
	PORT_DIPSETTING(    0x0002, "3" )
	PORT_DIPSETTING(    0x0003, "4" )
	PORT_DIPNAME( 0x001c, 0x001c, "Double-Up Level" )   PORT_DIPLOCATION("DSW1:!3,!4,!5")
	PORT_DIPSETTING(    0x0000, "1" )
	PORT_DIPSETTING(    0x0004, "2" )
	PORT_DIPSETTING(    0x0008, "3" )
	PORT_DIPSETTING(    0x000c, "4" )
	PORT_DIPSETTING(    0x0010, "5" )
	PORT_DIPSETTING(    0x0014, "6" )
	PORT_DIPSETTING(    0x0018, "7" )
	PORT_DIPSETTING(    0x001c, "8" )
	PORT_DIPNAME( 0x0060, 0x0060, "Bet Max" )           PORT_DIPLOCATION("DSW1:!6,!7")
	PORT_DIPSETTING(    0x0000, "10" )
	PORT_DIPSETTING(    0x0020, "20" )
	PORT_DIPSETTING(    0x0040, "30" )
	PORT_DIPSETTING(    0x0060, "50" )
	PORT_DIPNAME( 0x0080, 0x0080, "Magnification" )     PORT_DIPLOCATION("DSW1:!8")
	PORT_DIPSETTING(    0x0000, "A Type" )
	PORT_DIPSETTING(    0x0080, "B Type" )

	PORT_DIPNAME( 0x0700, 0x0700, "Key In - Hopper Out" )   PORT_DIPLOCATION("DSW2:!1,!2,!3")
	PORT_DIPSETTING(    0x0000, "1" )
	PORT_DIPSETTING(    0x0100, "10" )
	PORT_DIPSETTING(    0x0200, "50" )
	PORT_DIPSETTING(    0x0300, "100" )
	PORT_DIPSETTING(    0x0400, "200" )
	PORT_DIPSETTING(    0x0500, "300" )
	PORT_DIPSETTING(    0x0600, "500" )
	PORT_DIPSETTING(    0x0700, "1000" )
	PORT_DIPNAME( 0x3800, 0x3800, "Coin-In" )           PORT_DIPLOCATION("DSW2:!4,!5,!6")
	PORT_DIPSETTING(    0x0000, "1" )
	PORT_DIPSETTING(    0x0800, "5" )
	PORT_DIPSETTING(    0x1000, "10" )
	PORT_DIPSETTING(    0x1800, "20" )
	PORT_DIPSETTING(    0x2000, "25" )
	PORT_DIPSETTING(    0x2800, "50" )
	PORT_DIPSETTING(    0x3000, "100" )
	PORT_DIPSETTING(    0x3800, "250" )
	PORT_DIPNAME( 0xc000, 0xc000, "Service-In" )        PORT_DIPLOCATION("DSW2:!7,!8")
	PORT_DIPSETTING(    0x0000, "1" )
	PORT_DIPSETTING(    0x4000, "5" )
	PORT_DIPSETTING(    0x8000, "10" )
	PORT_DIPSETTING(    0xc000, "100" )

	PORT_START("DSW3-4")
	PORT_DIPNAME( 0x0007, 0x0007, "Coin In Limit" )     PORT_DIPLOCATION("DSW3:!1,!2,!3")
	PORT_DIPSETTING(    0x0001, "1000" )
	PORT_DIPSETTING(    0x0002, "2000" )
	PORT_DIPSETTING(    0x0003, "3000" )
	PORT_DIPSETTING(    0x0004, "5000" )
	PORT_DIPSETTING(    0x0005, "10000" )
	PORT_DIPSETTING(    0x0006, "20000" )
	PORT_DIPSETTING(    0x0007, "30000" )
	PORT_DIPSETTING(    0x0000, "999999" )
	PORT_DIPNAME( 0x0038, 0x0038, "Credit Limit" )      PORT_DIPLOCATION("DSW3:!4,!5,!6")
	PORT_DIPSETTING(    0x0008, "1000" )
	PORT_DIPSETTING(    0x0010, "3000" )
	PORT_DIPSETTING(    0x0018, "5000" )
	PORT_DIPSETTING(    0x0020, "10000" )
	PORT_DIPSETTING(    0x0028, "20000" )
	PORT_DIPSETTING(    0x0030, "30000" )
	PORT_DIPSETTING(    0x0038, "50000" )
	PORT_DIPSETTING(    0x0000, "999999" )
	PORT_DIPNAME( 0x0040, 0x0040, "Out Coin Counter" )  PORT_DIPLOCATION("DSW3:!7")     // Conditional to 'Hopper Sub-Board' (DSW4-3)
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Credit Cut" )        PORT_DIPLOCATION("DSW3:!8")
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0080, DEF_STR( On ) )

	PORT_DIPNAME( 0x0300, 0x0300, "4 Card Fever" )      PORT_DIPLOCATION("DSW4:!1,!2")
	PORT_DIPSETTING(    0x0000, "A Type" )
	PORT_DIPSETTING(    0x0100, "B Type" )
	PORT_DIPSETTING(    0x0200, "C Type" )
	PORT_DIPSETTING(    0x0300, "D Type" )
	PORT_DIPNAME( 0x0400, 0x0400, "Hopper Sub-Board" )  PORT_DIPLOCATION("DSW4:!3")     // When off, allow set the 'Out Coin Counter' (DSW3-7)
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0400, "Use" )
	PORT_DIPNAME( 0x0800, 0x0000, "Title Type" )        PORT_DIPLOCATION("DSW4:!4")
	PORT_DIPSETTING(    0x0000, "A Type: Umi de Poker" )
	PORT_DIPSETTING(    0x0800, "B Type: Marine Paradise" )
	PORT_DIPNAME( 0x3000, 0x3000, "First Bet" )         PORT_DIPLOCATION("DSW4:!5,!6")
	PORT_DIPSETTING(    0x0000, "1" )
	PORT_DIPSETTING(    0x1000, "5" )
	PORT_DIPSETTING(    0x2000, "10" )
	PORT_DIPSETTING(    0x3000, "20" )
	PORT_DIPNAME( 0x4000, 0x4000, "Fever Initialize" )  PORT_DIPLOCATION("DSW4:!7")
	PORT_DIPSETTING(    0x0000, "A Type" )
	PORT_DIPSETTING(    0x4000, "B Type" )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:!8")     // Unmapped?...
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( saiyukip )
	PORT_INCLUDE( common )

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_GAMBLE_HIGH ) PORT_NAME("Left / Stop")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_GAMBLE_BET )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_GAMBLE_LOW )  PORT_NAME("Right")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1-2")
	PORT_DIPNAME( 0x0007, 0x0007, "Main Level" )        PORT_DIPLOCATION("DSW1:!1,!2,!3")
	PORT_DIPSETTING(    0x0000, "1" )
	PORT_DIPSETTING(    0x0001, "2" )
	PORT_DIPSETTING(    0x0002, "3" )
	PORT_DIPSETTING(    0x0003, "4" )
	PORT_DIPSETTING(    0x0004, "5" )
	PORT_DIPSETTING(    0x0005, "6" )
	PORT_DIPSETTING(    0x0006, "7" )
	PORT_DIPSETTING(    0x0007, "8" )
	PORT_DIPNAME( 0x0038, 0x0038, "Double-Up Level" )   PORT_DIPLOCATION("DSW1:!4,!5,!6")
	PORT_DIPSETTING(    0x0000, "1" )
	PORT_DIPSETTING(    0x0008, "2" )
	PORT_DIPSETTING(    0x0010, "3" )
	PORT_DIPSETTING(    0x0018, "4" )
	PORT_DIPSETTING(    0x0020, "5" )
	PORT_DIPSETTING(    0x0028, "6" )
	PORT_DIPSETTING(    0x0030, "7" )
	PORT_DIPSETTING(    0x0038, "8" )
	PORT_DIPNAME( 0x0040, 0x0040, "Percentage Wide" )   PORT_DIPLOCATION("DSW1:!7")
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "One Game Timer" )    PORT_DIPLOCATION("DSW1:!8")
	PORT_DIPSETTING(    0x0000, "20 Sec." )
	PORT_DIPSETTING(    0x0080, "50 Sec." )

	PORT_DIPNAME( 0x0700, 0x0700, "Key In - Hopper Out" )   PORT_DIPLOCATION("DSW2:!1,!2,!3")
	PORT_DIPSETTING(    0x0000, "1" )
	PORT_DIPSETTING(    0x0100, "5" )
	PORT_DIPSETTING(    0x0200, "10" )
	PORT_DIPSETTING(    0x0300, "20" )
	PORT_DIPSETTING(    0x0400, "50" )
	PORT_DIPSETTING(    0x0500, "100" )
	PORT_DIPSETTING(    0x0600, "500" )
	PORT_DIPSETTING(    0x0700, "1000" )
	PORT_DIPNAME( 0x3800, 0x3800, "Coin-In" )           PORT_DIPLOCATION("DSW2:!4,!5,!6")
	PORT_DIPSETTING(    0x0000, "1" )
	PORT_DIPSETTING(    0x0800, "5" )
	PORT_DIPSETTING(    0x1000, "10" )
	PORT_DIPSETTING(    0x1800, "20" )
	PORT_DIPSETTING(    0x2000, "25" )
	PORT_DIPSETTING(    0x2800, "50" )
	PORT_DIPSETTING(    0x3000, "100" )
	PORT_DIPSETTING(    0x3800, "250" )
	PORT_DIPNAME( 0xc000, 0xc000, "Service-In" )        PORT_DIPLOCATION("DSW2:!7,!8")
	PORT_DIPSETTING(    0x0000, "1" )
	PORT_DIPSETTING(    0x4000, "5" )
	PORT_DIPSETTING(    0x8000, "10" )
	PORT_DIPSETTING(    0xc000, "100" )

	PORT_START("DSW3-4")
	PORT_DIPNAME( 0x0007, 0x0007, "Coin In Max" )       PORT_DIPLOCATION("DSW3:!1,!2,!3")
	PORT_DIPSETTING(    0x0001, "1000" )
	PORT_DIPSETTING(    0x0002, "2000" )
	PORT_DIPSETTING(    0x0003, "3000" )
	PORT_DIPSETTING(    0x0004, "5000" )
	PORT_DIPSETTING(    0x0005, "10000" )
	PORT_DIPSETTING(    0x0006, "20000" )
	PORT_DIPSETTING(    0x0007, "30000" )
	PORT_DIPSETTING(    0x0000, "50000" )
	PORT_DIPNAME( 0x0038, 0x0038, "Limit Over" )        PORT_DIPLOCATION("DSW3:!4,!5,!6")
	PORT_DIPSETTING(    0x0008, "1000" )
	PORT_DIPSETTING(    0x0010, "3000" )
	PORT_DIPSETTING(    0x0018, "5000" )
	PORT_DIPSETTING(    0x0020, "10000" )
	PORT_DIPSETTING(    0x0028, "20000" )
	PORT_DIPSETTING(    0x0030, "30000" )
	PORT_DIPSETTING(    0x0038, "50000" )
	PORT_DIPSETTING(    0x0000, "Limit Over & Coin In Max OFF" )
	PORT_DIPNAME( 0x0040, 0x0040, "Demo Sound" )        PORT_DIPLOCATION("DSW3:!7")
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "BGM" )               PORT_DIPLOCATION("DSW3:!8")
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0080, DEF_STR( On ) )

	PORT_DIPNAME( 0x0300, 0x0300, "First Bet" )         PORT_DIPLOCATION("DSW4:!1,!2")
	PORT_DIPSETTING(    0x0000, "1" )
	PORT_DIPSETTING(    0x0100, "5" )
	PORT_DIPSETTING(    0x0200, "10" )
	PORT_DIPSETTING(    0x0300, "15" )
	PORT_DIPNAME( 0x0c00, 0x0c00, "Bet Max" )           PORT_DIPLOCATION("DSW4:!3,!4")
	PORT_DIPSETTING(    0x0000, "1" )
	PORT_DIPSETTING(    0x0400, "10" )
	PORT_DIPSETTING(    0x0800, "30" )
	PORT_DIPSETTING(    0x0c00, "50" )
	PORT_DIPNAME( 0x1000, 0x1000, "Hopper" )            PORT_DIPLOCATION("DSW4:!5")
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Medal Sub-Board" )   PORT_DIPLOCATION("DSW4:!6")     // When off, allow 'Out Counter' to be set
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Out Counter" )       PORT_DIPLOCATION("DSW4:!7")     // Conditional to 'Medal Sub-Board'
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Credit Over Cut" )   PORT_DIPLOCATION("DSW4:!8")
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( On ) )
INPUT_PORTS_END


static GFXDECODE_START( gfx_umipoker )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x4_planar, 0, 0x40)
GFXDECODE_END

void saiyukip_state::machine_start()
{
	umipoker_state::machine_start();

	m_lamps.resolve();
}

// TODO: Verify clocks (XTALs are 14.3181 and 2.000MHz)
void umipoker_state::umipoker(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(14'318'181)); // TMP68HC000-16
	m_maincpu->set_addrmap(AS_PROGRAM, &umipoker_state::main_map);

	z80_device &audiocpu(Z80(config, "audiocpu", XTAL(14'318'181) / 4)); // 3.579545MHz
	audiocpu.set_addrmap(AS_PROGRAM, &umipoker_state::audio_map);
	audiocpu.set_addrmap(AS_IO, &umipoker_state::audio_io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(8*8, 48*8-1, 2*8, 32*8-1);
	screen.set_screen_update(FUNC(umipoker_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, 6, ASSERT_LINE);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_umipoker);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x400);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym3812_device &ym(YM3812(config, "ym", XTAL(14'318'181) / 4)); // 3.579545MHz
	ym.irq_handler().set_inputline("audiocpu", 0);
	ym.add_route(ALL_OUTPUTS, "mono", 1.0);

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(2'000'000), okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 not verified
	oki.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void saiyukip_state::saiyukip(machine_config &config)
{
	umipoker(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &saiyukip_state::main_map);
}


/***************************************************************************

  ROM Loads

***************************************************************************/

ROM_START( umipoker ) // W-ONE 1995-08 PCB
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "p0.u61",      0x000000, 0x020000, CRC(3a0c4a8b) SHA1(009cbbd4df68e31dc683a8cd3425e5f8b986fa92) )
	ROM_LOAD16_BYTE( "p1.u60",      0x000001, 0x020000, CRC(44f475cb) SHA1(0c5044331dbcc617d200d052b4597dd4551fc95c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sz.u8",        0x000000, 0x010000, CRC(d874ba1a) SHA1(13c06f3b67694d5d5194023c4f7b75aea8b57129) ) // second half 1-filled

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "sg3.u39",      0x000000, 0x020000, CRC(ebd5f96d) SHA1(968c107ee17f1e92ffc2835e13803347881862f1) )
	ROM_LOAD( "sg2.u40",      0x020000, 0x020000, CRC(eb31649b) SHA1(c0741d85537827e2396e81a1aa3005871dffad78) )
	ROM_LOAD( "sg1.u41",      0x040000, 0x020000, CRC(7fcbfb17) SHA1(be2f308a8e8f0941c54125950702ddfbd8538733) )
	ROM_LOAD( "sg0.u42",      0x060000, 0x020000, CRC(876f1f4f) SHA1(eca4c397be57812f2c34791736fee7c43925d927) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "sm.u17",       0x000000, 0x040000, CRC(99503aed) SHA1(011404fad01b3ced708a94143908be3e1d0194d3) ) // first half 1-filled
	ROM_CONTINUE(             0x000000, 0x040000 )

	ROM_REGION( 0x400, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "gal16v8b.u11", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8b.u24", 0x200, 0x117, NO_DUMP )
ROM_END

ROM_START( umipokera )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sp0.u61",      0x000000, 0x020000, CRC(866eaa02) SHA1(445afdfe010aad1102219a0dbd3a363a22294b4c) )
	ROM_LOAD16_BYTE( "sp1.u60",      0x000001, 0x020000, CRC(8db08696) SHA1(2854d511a8fd30b023e2a2a00b25413f88205d82) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sz.u8",        0x000000, 0x010000, CRC(d874ba1a) SHA1(13c06f3b67694d5d5194023c4f7b75aea8b57129) ) // second half 1-filled

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "sg3.u39",      0x000000, 0x020000, CRC(ebd5f96d) SHA1(968c107ee17f1e92ffc2835e13803347881862f1) )
	ROM_LOAD( "sg2.u40",      0x020000, 0x020000, CRC(eb31649b) SHA1(c0741d85537827e2396e81a1aa3005871dffad78) )
	ROM_LOAD( "sg1.u41",      0x040000, 0x020000, CRC(7fcbfb17) SHA1(be2f308a8e8f0941c54125950702ddfbd8538733) )
	ROM_LOAD( "sg0.u42",      0x060000, 0x020000, CRC(876f1f4f) SHA1(eca4c397be57812f2c34791736fee7c43925d927) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "sm.u17",       0x000000, 0x040000, CRC(99503aed) SHA1(011404fad01b3ced708a94143908be3e1d0194d3) ) // first half 1-filled
	ROM_CONTINUE(             0x000000, 0x040000 )
ROM_END

ROM_START( saiyukip )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "slp0-spq.u61", 0x000000, 0x020000, CRC(7fc0f201) SHA1(969170d68278e212dd459744373ed9e704976e45) )
	ROM_LOAD16_BYTE( "slp1-spq.u60", 0x000001, 0x020000, CRC(c8e3547c) SHA1(18bb380a64ed36f45a377b86cbbac892efe879bb) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "slz.u8",       0x000000, 0x010000, CRC(4f32ba1c) SHA1(8f1f8c0995bcd05d19120dd3b64b135908caf759) ) // second half 1-filled

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "slg3.u39",     0x000000, 0x020000, CRC(e99b2906) SHA1(77884d2dae2e7f7cf27103aa8bbd0eaa39628993) )
	ROM_LOAD( "slg2.u40",     0x020000, 0x020000, CRC(fe6cd717) SHA1(65e59d88a30efd0cec642cda54e2bc38196f0231) )
	ROM_LOAD( "slg1.u41",     0x040000, 0x020000, CRC(59b5f399) SHA1(2b999cebcc53b3b8fd38e3034a12434d82b6fad3) )
	ROM_LOAD( "slg0.u42",     0x060000, 0x020000, CRC(49ba7ffd) SHA1(3bbb7656eafbd8c91c9054fca056c8fc3002ed13) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "slm.u17",      0x000000, 0x040000, CRC(b50eb70b) SHA1(342fcb307844f4d0a02a85b2c61e73b5e8bacd44) ) // first half 1-filled
	ROM_CONTINUE(             0x000000, 0x040000 )
ROM_END

} // anonymous namespace


/******************************************
*              Game Drivers               *
******************************************/

//     YEAR  NAME       PARENT    MACHINE    INPUT     STATE           INIT        ROT   COMPANY                  FULLNAME                                         FLAGS                   LAYOUT
GAME(  1997, umipoker,  0,        umipoker,  umipoker, umipoker_state, empty_init, ROT0, "World Station Co.,LTD", "Umi de Poker / Marine Paradise (Japan, newer)", MACHINE_SUPPORTS_SAVE )                      // title screen is toggleable thru a dsw
GAME(  1997, umipokera, umipoker, umipoker,  umipoker, umipoker_state, empty_init, ROT0, "World Station Co.,LTD", "Umi de Poker / Marine Paradise (Japan, older)", MACHINE_SUPPORTS_SAVE )                      // title screen is toggleable thru a dsw
GAMEL( 1998, saiyukip,  0,        saiyukip,  saiyukip, saiyukip_state, empty_init, ROT0, "World Station Co.,LTD", "Slot Poker Saiyuki (Japan)",                    MACHINE_SUPPORTS_SAVE,  layout_saiyukip )

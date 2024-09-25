// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Gun Dealer memory map

driver by Nicola Salmoria

The custom graphics chips at locations labeled GA 1, GA 2 and GA 3 (x2) are
surface-scratched on common blue boards. However, a Tecmo-licensed green
PCB reveals them to be none other than NMK-901, NMK-902 and NMK-903.

Yam! Yam!? runs on similar hardware but has a protection device which can
           access RAM at e000. Program writes to e000 and expects a value back
           at e001, then jumps to subroutines at e010 and e020. Also, the
           player and coin inputs appear magically at e004-e006.

Despite not using GFX customs or MCU protection and lacking Dooyong labels,
gundealrbl might be a factory conversion of Yam! Yam!? rather than a
bootleg, as its board has the same NMK-style markings for the ROM and PROM
locations. In place of the MCU here is a small daughterboard with three
SN74LS245N buffers, a resistor array and an Altera EP320PC.

0000-7fff ROM
8000-bfff ROM (banked)
c400-c7ff palette RAM
c800-cfff background video RAM
d000-dfff foreground (scrollable) video RAM.
e000-ffff work RAM

read:
c000      DSW0
c001      DSW1
c004      COIN (Gun Dealer only)
c005      IN1 (Gun Dealer only)
c006      IN0 (Gun Dealer only)

write:
c010-c011 foreground scroll x lo-hi (Yam Yam)
c012-c013 foreground scroll y lo-hi (Yam Yam)
c014      flip screen
c015      Yam Yam only, maybe reset protection device
c016      ROM bank selector
c020-c021 foreground scroll x hi-lo (Gun Dealer)
c022-c023 foreground scroll y hi-lo (Gun Dealer)

I/O:
read:
01        YM2203 read

write:
00        YM2203 control
01        YM2203 write

Interrupts:
Runs in interrupt mode 0, the interrupt vectors are 0xcf (RST 08h) and
0xd7 (RST 10h)

PCB:  DY-90010001
  CPU: Z80B
Sound: YM2203C + Y3014B DAC
  MCU: Unknown 64 pin DIL
  OSC: 12MHz, 5MHz

Clock measurements:
Z80 CPU - 12MHz/2
 YM2203 - 12MHz/8

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/timer.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_MCUSIM     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_MCUSIM)

#include "logmacro.h"

#define LOGMCUSIM(...)     LOGMASKED(LOG_MCUSIM,     __VA_ARGS__)


namespace {

class gundealr_state : public driver_device
{
public:
	gundealr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_paletteram(*this, "paletteram")
		, m_bg_videoram(*this, "bg_videoram")
		, m_fg_videoram(*this, "fg_videoram")
		, m_mainbank(*this, "mainbank")
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
	{ }

	void gundealr(machine_config &config);
	void gundealrbl(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_paletteram;
	required_shared_ptr<uint8_t> m_bg_videoram;
	required_shared_ptr<uint8_t> m_fg_videoram;
	required_memory_bank m_mainbank;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	uint8_t m_scroll[4]{};

	// misc
	void bankswitch_w(uint8_t data);
	void bg_videoram_w(offs_t offset, uint8_t data);
	void fg_videoram_w(offs_t offset, uint8_t data);
	void paletteram_w(offs_t offset, uint8_t data);
	template<int Xor> void fg_scroll_w(offs_t offset, uint8_t data);
	template<int Bit> void flipscreen_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILEMAP_MAPPER_MEMBER(pagescan);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	void base_map(address_map &map) ATTR_COLD;
	void gundealr_main_map(address_map &map) ATTR_COLD;
	void main_portmap(address_map &map) ATTR_COLD;
	void yamyam_main_map(address_map &map) ATTR_COLD;
};

class yamyam_mcu_state : public gundealr_state
{
public:
	yamyam_mcu_state(const machine_config &mconfig, device_type type, const char *tag)
		: gundealr_state(mconfig, type, tag)
		, m_rambase(*this, "rambase")
		, m_port_in(*this, "IN%u", 0)
	{ }

	void yamyam(machine_config &config);

private:
	required_shared_ptr<uint8_t> m_rambase;

	required_ioport_array<3> m_port_in;

	TIMER_DEVICE_CALLBACK_MEMBER(mcu_sim);
};


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(gundealr_state::get_bg_tile_info)
{
	uint8_t attr = m_bg_videoram[2 * tile_index + 1];
	tileinfo.set(0,
			m_bg_videoram[2 * tile_index] + ((attr & 0x07) << 8),
			(attr & 0xf0) >> 4,
			0);
}

TILEMAP_MAPPER_MEMBER(gundealr_state::pagescan)
{
	// logical (col,row) -> memory offset
	return (row & 0x0f) + ((col & 0x3f) << 4) + ((row & 0x10) << 6);
}

TILE_GET_INFO_MEMBER(gundealr_state::get_fg_tile_info)
{
	uint8_t attr = m_fg_videoram[2 * tile_index + 1];
	tileinfo.set(1,
			m_fg_videoram[2 * tile_index] + ((attr & 0x03) << 8),
			(attr & 0xf0) >> 4,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void gundealr_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gundealr_state::get_bg_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gundealr_state::get_fg_tile_info)), tilemap_mapper_delegate(*this, FUNC(gundealr_state::pagescan)), 16, 16, 64, 32);

	m_fg_tilemap->set_transparent_pen(15);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void gundealr_state::bg_videoram_w(offs_t offset, uint8_t data)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

void gundealr_state::fg_videoram_w(offs_t offset, uint8_t data)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

void gundealr_state::paletteram_w(offs_t offset, uint8_t data)
{
	int val;

	m_paletteram[offset] = data;

	val = m_paletteram[offset & ~1];
	const int r = (val >> 4) & 0x0f;
	const int g = (val >> 0) & 0x0f;

	val = m_paletteram[offset | 1];
	const int b = (val >> 4) & 0x0f;
	// TODO: the bottom 4 bits are used as well, but I'm not sure about the meaning

	m_palette->set_pen_color(offset / 2, pal4bit(r), pal4bit(g), pal4bit(b));
}


/***************************************************************************

  Display refresh

***************************************************************************/

uint32_t gundealr_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


void gundealr_state::bankswitch_w(uint8_t data)
{
	m_mainbank->set_entry(data & 0x07);
}


template<int Xor>
void gundealr_state::fg_scroll_w(offs_t offset, uint8_t data)
{
	m_scroll[offset] = data;
	m_fg_tilemap->set_scrollx(0, m_scroll[0 ^ Xor] | ((m_scroll[1 ^ Xor] & 0x03) << 8));
	m_fg_tilemap->set_scrolly(0, m_scroll[2 ^ Xor] | ((m_scroll[3 ^ Xor] & 0x03) << 8));
}

template<int Bit>
void gundealr_state::flipscreen_w(uint8_t data)
{
	machine().tilemap().set_flip_all(BIT(data, Bit) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
}


void gundealr_state::base_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_mainbank);
	map(0xc000, 0xc000).portr("DSW0");
	map(0xc001, 0xc001).portr("DSW1");
	map(0xc004, 0xc004).portr("IN0");
	map(0xc005, 0xc005).portr("IN1");
	map(0xc006, 0xc006).portr("IN2");
	map(0xc016, 0xc016).w(FUNC(gundealr_state::bankswitch_w));
	map(0xc400, 0xc7ff).ram().w(FUNC(gundealr_state::paletteram_w)).share(m_paletteram);
	map(0xc800, 0xcfff).ram().w(FUNC(gundealr_state::bg_videoram_w)).share(m_bg_videoram);
	map(0xd000, 0xdfff).ram().w(FUNC(gundealr_state::fg_videoram_w)).share(m_fg_videoram);
	map(0xe000, 0xffff).ram().share("rambase");
}

void gundealr_state::gundealr_main_map(address_map &map)
{
	base_map(map);
	map(0xc014, 0xc014).w(FUNC(gundealr_state::flipscreen_w<0>));
	map(0xc020, 0xc023).w(FUNC(gundealr_state::fg_scroll_w<1>));
}

void gundealr_state::yamyam_main_map(address_map &map)
{
	base_map(map);
	map(0xc010, 0xc013).w(FUNC(gundealr_state::fg_scroll_w<0>));
	map(0xc014, 0xc014).w(FUNC(gundealr_state::flipscreen_w<7>));
	map(0xc015, 0xc015).nopw(); // Bit 7 = MCU reset?
}

void gundealr_state::main_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
}



static INPUT_PORTS_START( gundealr )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:8") // Listed in the manual as always OFF
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:7") // Listed in the manual as always OFF
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:4") // Listed in the manual as always OFF
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:3") // Listed in the manual as always OFF
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:8,7,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:5,4,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:2,1") // Both switch 1 & 2 are listed in the manual as always OFF
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0xc0, "3" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused
INPUT_PORTS_END

static INPUT_PORTS_START( gundealt )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0xc0, "3" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( yamyam )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, "Difficulty?" )       PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, "Easy?" )
	PORT_DIPSETTING(    0x04, "Medium?" )
	PORT_DIPSETTING(    0x08, "Hard?" )
	PORT_DIPSETTING(    0x0c, "Hardest?" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x07, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(    0x38, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(   0x80, IP_ACTIVE_LOW, "SW2:8" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )   // "TEST"
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    // probably unused
INPUT_PORTS_END



static GFXDECODE_START( gfx_gundealr )
	GFXDECODE_ENTRY( "bgtiles", 0, gfx_8x8x4_packed_msb,                 0, 16 ) // colors 0-255
	GFXDECODE_ENTRY( "fgtiles", 0, gfx_8x8x4_col_2x2_group_packed_msb, 256, 16 ) // colors 256-511
GFXDECODE_END




void gundealr_state::machine_start()
{
	m_mainbank->configure_entries(0, 8, memregion("maincpu")->base(), 0x4000);

	save_item(NAME(m_scroll));
}

void gundealr_state::machine_reset()
{
	m_scroll[0] = 0;
	m_scroll[1] = 0;
	m_scroll[2] = 0;
	m_scroll[3] = 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(gundealr_state::scanline)
{
	int scanline = param;

	if (scanline == 240) // vblank-out irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xd7); // Z80 - RST 10h
	else if ((scanline == 0) || (scanline == 120) ) //timer irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xcf); // Z80 - RST 10h
}

void gundealr_state::gundealr(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(12'000'000) / 2);   // 6 MHz verified for Yam! Yam!?
	m_maincpu->set_addrmap(AS_PROGRAM, &gundealr_state::gundealr_main_map);
	m_maincpu->set_addrmap(AS_IO, &gundealr_state::main_portmap);
	TIMER(config, "scantimer").configure_scanline(FUNC(gundealr_state::scanline), "screen", 0, 1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(gundealr_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gundealr);
	PALETTE(config, m_palette).set_entries(512);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YM2203(config, "ymsnd", XTAL(12'000'000) / 8).add_route(ALL_OUTPUTS, "mono", 0.25); // 1.5Mhz verified for Yam! Yam!?
}

TIMER_DEVICE_CALLBACK_MEMBER(yamyam_mcu_state::mcu_sim)
{
	static const uint8_t snipped_cmd03[8] = { 0x3a, 0x00, 0xc0, 0x47, 0x3a, 0x01, 0xc0, 0xc9 };
	static const uint8_t snipped_cmd05_1[5] = { 0xcd, 0x20, 0xe0, 0x7e, 0xc9 };
	static const uint8_t snipped_cmd05_2[8] = { 0xc5, 0x01, 0x00, 0x00, 0x4f, 0x09, 0xc1, 0xc9 };

	int i;

	LOGMCUSIM("e000 = %02x\n", m_rambase[0x000]);
	switch(m_rambase[0x000])
	{
		case 0x03:
			m_rambase[0x001] = 0x03;
			/*
			    read dip switches
			    3a 00 c0  ld   a,($c000)
			    47        ld   b,a
			    3a 01 c0  ld   a,($c001)
			    c9        ret
			*/
			for(i = 0; i < 8; i++)
				m_rambase[0x010 + i] = snipped_cmd03[i];

			break;
		case 0x04:
			m_rambase[0x001] = 0x04;
			break;
		case 0x05:
			m_rambase[0x001] = 0x05;
			/*
			    add a to hl
			    c5          push    bc
			    01 00 00    ld      bc,#0000
			    4f          ld      c,a
			    09          add     hl,bc
			    c1          pop     bc
			    c9          ret
			*/
			for(i = 0; i < 8; i++)
				m_rambase[0x020 + i] = snipped_cmd05_2[i];

			/*
			    lookup data in table
			    cd 20 e0    call    #e020
			    7e          ld      a,(hl)
			    c9          ret
			*/
			for(i = 0; i < 5; i++)
				m_rambase[0x010 + i] = snipped_cmd05_1[i];

			break;
		case 0x0a:
			m_rambase[0x001] = 0x08;
			break;
		case 0x0d:
			m_rambase[0x001] = 0x07;
			break;
	}

	m_rambase[0x004] = m_port_in[2]->read();
	m_rambase[0x005] = m_port_in[1]->read();
	m_rambase[0x006] = m_port_in[0]->read();
}

void gundealr_state::gundealrbl(machine_config &config)
{
	gundealr(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &gundealr_state::yamyam_main_map);
}

void yamyam_mcu_state::yamyam(machine_config &config)
{
	gundealrbl(config);

	TIMER(config, "mcusim").configure_periodic(FUNC(yamyam_mcu_state::mcu_sim), attotime::from_hz(6000000 / 60)); // 6mhz confirmed
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( gundealr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.3j",   0x00000, 0x10000, CRC(5797e830) SHA1(54bd9fbcafdf3fff55d73ecfe26d8e8df0dd55d9) ) // 27c512; NOTE: the socket is labeled 1, but the ROM has a '2' sticker on it!
	// banked at 0x8000-0xbfff

	ROM_REGION( 0x10000, "bgtiles", 0 )
	ROM_LOAD( "3.6p",         0x00000, 0x10000, CRC(01f99de2) SHA1(2d9e9c50b0669811beb6fa53c0ff1b240fa939c7) )

	ROM_REGION( 0x20000, "fgtiles", 0 )
	ROM_LOAD( "2.6b",   0x00000, 0x20000, CRC(7874ec41) SHA1(2d2ff013cc37ce5966aa4b6c6724234655196102) ) // NOTE: the socket is labeled 2, but the ROM has a '1' sticker on it!

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s135.7l", 0x0000, 0x0100, NO_DUMP)
	ROM_LOAD( "82s129.7i", 0x0100, 0x0100, NO_DUMP)
ROM_END

ROM_START( gundealra )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gundeala.1.3j",   0x00000, 0x10000, CRC(d87e24f1) SHA1(5ac3e20e5848b9cab2a23e083d2566bfd54502d4) )
	// banked at 0x8000-0xbfff

	ROM_REGION( 0x10000, "bgtiles", 0 )
	ROM_LOAD( "gundeala.3.6p",   0x00000, 0x10000, CRC(836cf1a3) SHA1(ca57e7fc3e4497d249af963d1c8610e80ca65aa7) )

	ROM_REGION( 0x20000, "fgtiles", 0 )
	ROM_LOAD( "gundeala.2.6b",   0x00000, 0x20000, CRC(4b5fb53c) SHA1(3b73d9aeed334aece75f551f5b7f3cec0aedbfaa) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s135.7l", 0x0000, 0x0100, NO_DUMP)
	ROM_LOAD( "82s129.7i", 0x0100, 0x0100, NO_DUMP)
ROM_END

ROM_START( gundealrt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.3j",         0x00000, 0x10000, CRC(1d951292) SHA1(a8bd34dfaf31c7dc4f9e0ec1fd7d4e10c5b29a85) )
	// banked at 0x8000-0xbfff

	ROM_REGION( 0x10000, "bgtiles", 0 )
	ROM_LOAD( "3.6p",         0x00000, 0x10000, CRC(01f99de2) SHA1(2d9e9c50b0669811beb6fa53c0ff1b240fa939c7) )

	ROM_REGION( 0x20000, "fgtiles", 0 )
	ROM_LOAD( "2.6b",         0x00000, 0x20000, CRC(508ed0d0) SHA1(ea6b2d07e2e3d4f6c2a622a73b150ee7709b28de) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s135.7l", 0x0000, 0x0100, NO_DUMP)
	ROM_LOAD( "82s129.7i", 0x0100, 0x0100, NO_DUMP)
ROM_END

ROM_START( gundealrbl ) // gfx customs done out in TTL logic, different proms, patched code rom
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "29.2.am27c512.f10",   0x00000, 0x10000, CRC(7981751e) SHA1(3138581bcff84a11670ba54cbca608d590055b4e) ) // almost == gundealr "1.3j", 5 bytes different: (what does this change?)
	// banked at 0x8000-0xbfff
	// address gundealr gundealrbl
	// 009a    07       00
	// 6d4a    21       10
	// 6d52    20       11
	// 6d58    23       12
	// 6d60    22       13

	ROM_REGION( 0x10000, "bgtiles", 0 )
	ROM_LOAD( "30.3.am27c512.d16",  0x00000, 0x10000, CRC(01f99de2) SHA1(2d9e9c50b0669811beb6fa53c0ff1b240fa939c7) ) // == gundealr "3.16d"

	ROM_REGION( 0x20000, "fgtiles", 0 )
	ROM_LOAD( "22.1.d27c010.a16",   0x00000, 0x20000, CRC(7874ec41) SHA1(2d2ff013cc37ce5966aa4b6c6724234655196102) ) // == gundealr "2.6b"

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "4.82s129.7e", 0x0000, 0x0100, CRC(5c78339e) SHA1(4567c20122ae9694358b462e899f8bd2b453499f) ) // this might match the undumped 82s129.7i on the original dooyong board

	ROM_REGION( 0x0400, "pals", 0 )
	ROM_LOAD( "ep320pc.jed", 0x0000, 0x0400, NO_DUMP) // altera ep320pc on a daughterboard, undumped
ROM_END

ROM_START( gundealrbl2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gd2.512", 0x00000, 0x10000, CRC(91be1d98) SHA1(bb4814f9598c7429058b440f29b60415ee426088) ) // identical to gundealrbl but for a small patch at 0x607c

	ROM_REGION( 0x10000, "bgtiles", 0 )
	ROM_LOAD( "gd3.512", 0x00000, 0x10000, CRC(01f99de2) SHA1(2d9e9c50b0669811beb6fa53c0ff1b240fa939c7) )

	ROM_REGION( 0x20000, "fgtiles", 0 )
	ROM_LOAD( "gd1.010", 0x00000, 0x20000, CRC(7cbb2b1e) SHA1(81b5cb64abcc820c40c42950dc8956894465452b) ) // almost identical to the original, just with the Dooyong copyright blanked

	// there may be PROMs and / or PLDs but the pic is too low res to verify
ROM_END

ROM_START( yamyam ) // DY-90010001 PCB
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "3.10f",       0x00000, 0x20000, CRC(96ae9088) SHA1(a605882dcdcf1e8cf8b0112f614e696d59acfd97) )
	// banked at 0x8000-0xbfff

	ROM_REGION( 0x10000, "mcu", 0 ) // unknown 64 pin MCU at J9 with internal ROM code
	ROM_LOAD( "mcu", 0x0000, 0x10000, NO_DUMP)

	ROM_REGION( 0x10000, "bgtiles", 0 ) // only gfx are different, code is the same
	ROM_LOAD( "b2.16d",       0x00000, 0x10000, CRC(cb4f84ee) SHA1(54319ecbd74b763757eb6d17c8f7be0705ab0714) )

	ROM_REGION( 0x20000, "fgtiles", 0 )
	ROM_LOAD( "1.16a",       0x00000, 0x20000, CRC(b122828d) SHA1(90994ba548893a2eacdd58351cfa3952f4af926a) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "4.7e", 0x0000, 0x0100, NO_DUMP)
ROM_END

ROM_START( yamyamk ) // DY-90010001 PCB
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "3.10f",       0x00000, 0x20000, CRC(96ae9088) SHA1(a605882dcdcf1e8cf8b0112f614e696d59acfd97) )
	// banked at 0x8000-0xbfff

	ROM_REGION( 0x10000, "mcu", 0 ) // unknown 64 pin MCU at J9 with internal ROM code
	ROM_LOAD( "mcu", 0x0000, 0x10000, NO_DUMP)

	ROM_REGION( 0x10000, "bgtiles", 0 ) // only gfx are different, code is the same
	ROM_LOAD( "2.16d",       0x00000, 0x10000, CRC(dc9691d8) SHA1(118a05a1c94020d6739ed8c805c61b8ab003b6af) )

	ROM_REGION( 0x20000, "fgtiles", 0 )
	ROM_LOAD( "1.16a",       0x00000, 0x20000, CRC(b122828d) SHA1(90994ba548893a2eacdd58351cfa3952f4af926a) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "4.7e", 0x0000, 0x0100, NO_DUMP)
ROM_END

ROM_START( wiseguy ) // DY-90010001 PCB
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b3.f10",       0x00000, 0x20000, CRC(96ae9088) SHA1(a605882dcdcf1e8cf8b0112f614e696d59acfd97) )
	// banked at 0x8000-0xbfff

	ROM_REGION( 0x10000, "mcu", 0 ) // unknown 64 pin MCU at J9 with internal ROM code
	ROM_LOAD( "mcu", 0x0000, 0x10000, NO_DUMP)

	ROM_REGION( 0x10000, "bgtiles", 0 ) // only gfx are different, code is the same
	ROM_LOAD( "wguyb2.16d",   0x00000, 0x10000, CRC(1c684c46) SHA1(041bc500e31b02a8bf3ce4683a67de998f938ccc) )

	ROM_REGION( 0x20000, "fgtiles", 0 )
	ROM_LOAD( "1.16a",       0x00000, 0x20000, CRC(b122828d) SHA1(90994ba548893a2eacdd58351cfa3952f4af926a) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "4.7e", 0x0000, 0x0100, NO_DUMP)
ROM_END

} // anonymous namespace


GAME( 1990, gundealr,    0,        gundealr,   gundealr, gundealr_state,   empty_init, ROT270, "Dooyong",                 "Gun Dealer",                              MACHINE_SUPPORTS_SAVE )
GAME( 1990, gundealra,   gundealr, gundealr,   gundealr, gundealr_state,   empty_init, ROT270, "Dooyong",                 "Gun Dealer (alt card set)",               MACHINE_SUPPORTS_SAVE )
GAME( 1990, gundealrt,   gundealr, gundealr,   gundealt, gundealr_state,   empty_init, ROT270, "Dooyong (Tecmo license)", "Gun Dealer (Japan)",                      MACHINE_SUPPORTS_SAVE )
GAME( 1990, gundealrbl,  gundealr, gundealrbl, gundealr, gundealr_state,   empty_init, ROT270, "Dooyong",                 "Gun Dealer (Yam! Yam!? hardware, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, gundealrbl2, gundealr, gundealrbl, gundealr, gundealr_state,   empty_init, ROT270, "Dooyong",                 "Gun Dealer (Yam! Yam!? hardware, set 2)", MACHINE_SUPPORTS_SAVE )

GAME( 1990, yamyam,      0,        yamyam,     yamyam,   yamyam_mcu_state, empty_init, ROT0,   "Dooyong",                 "Yam! Yam!?",                              MACHINE_SUPPORTS_SAVE )
GAME( 1990, yamyamk,     yamyam,   yamyam,     yamyam,   yamyam_mcu_state, empty_init, ROT0,   "Dooyong",                 "Yam! Yam! (Korea)",                       MACHINE_SUPPORTS_SAVE )
GAME( 1990, wiseguy,     yamyam,   yamyam,     yamyam,   yamyam_mcu_state, empty_init, ROT0,   "Dooyong",                 "Wise Guy",                                MACHINE_SUPPORTS_SAVE )

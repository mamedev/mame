// license:BSD-3-Clause
// copyright-holders: David Graves, R. Belmont, David Haywood

/***************************************************************************

Excellent System's ES-9209B Hardware

Games supported:

   Grand Cross Pinball
   Power Flipper Pinball Shooting


Made from Raine source


Code
----

Inputs get tested at $4aca2 on


TODO
----

 - Screen flipping support
 - Figure out which customs use D80010-D80077 and merge implementation with Aquarium
 - Is SW3 actually used?
 - Power Flipper reference video: https://www.youtube.com/watch?v=zBGjncVsSf4 (seems to have been recorded with 'flipscreen' dipswitch on, because it causes a jumping glitch before the raster effect, same in MAME)

BGMs (controlled by OKI MSM6585 sound chip)
  MSM6585: is an upgraded MSM5205 voice synth IC.
   Improvements:
    More precise internal DA converter
    Built in low-pass filter
    Expanded sampling frequency

Stephh's notes (based on the game M68000 code and some tests) :

  - Reset the game while pressing START1 to enter the "test mode"


ES-9209B
+-----------------------------------------------+
|      M6585  U56  ES-8712                      |
| VR1 640kHz  U55           +-------+           |
|1056khz M6295              |ES 9207|           |
|                 6116      |       |  AS7C256  |
|                 6116      +-------+  AS7C256  |
|J                                     AS7C256  |
|A  MB3773                    AS7C256  AS7C256  |
|M  TSW1*               +-------+          U13* |
|M   PAL          32MHz |ES-9303|          U11  |
|A   PAL     68000P-16  +-------+               |
|       62256  4.U46      +-------+      1.U10  |
|       62256  3.U45      |ES-9208|             |
|  93C46         U44*     +-------+        U6   |
|              2.U43                            |
|SW4* SW3 SW2 SW1  14.31818MHz   5864 5864 U1   |
+-----------------------------------------------+

   CPU: TMP68HC000P-16
 Sound: OKI M6295
        OKI M6585
        Excellent ES-8712
   OSC: 32MHz, 14.31818MHz & 1056kHz, 640kHz resonators
   RAM: Sony CXK5864BSP-10L 8K x 8bit high speed CMOS SRAM
        Alliance AS7C256-20PC 32K x 8bit CMOS SRAM
        Hitachi HM6116LK-70 2K x  8bit SRAM
        Hitachi HM62256ALP-8 32K x 8bit CMOS SRAM
EEPROM: 93C46 1K Serial EEPROM
Custom: EXCELLENT SYSTEM ES-9208 347102 (QFP160)
        EXCELLENT SYSTEM LTD. ES 9207 9343 T (QFP208)
        ES-9303 EXCELLENT 9338 C001 (QFP120)

* Denotes unpopulated components

NOTE: Mask ROMs from Power Flipper Pinball Shooting have not been dumped, but assumed to
      be the same data.

***************************************************************************/

#include "emu.h"

#include "excellent_spr.h"

#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "machine/mb3773.h"
#include "machine/timer.h"
#include "sound/es8712.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_IOC      (1U << 1)
#define LOG_ESBANKSW (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_IOC | LOG_ESBANKSW)

#include "logmacro.h"

#define LOGIOC(...)      LOGMASKED(LOG_IOC,      __VA_ARGS__)
#define LOGESBANKSW(...) LOGMASKED(LOG_ESBANKSW, __VA_ARGS__)


namespace {

class gcpinbal_state : public driver_device
{
public:
	gcpinbal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_tilemapram(*this, "tilemapram")
		, m_d80010_ram(*this, "d80010")
		, m_d80060_ram(*this, "d80060")
		, m_maincpu(*this, "maincpu")
		, m_eeprom(*this, "eeprom")
		, m_watchdog(*this, "watchdog")
		, m_oki(*this, "oki")
		, m_essnd(*this, "essnd")
		, m_sprgen(*this, "spritegen")
		, m_screen(*this, "screen")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
	{ }

	void gcpinbal(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<u16> m_tilemapram;
	required_shared_ptr<u16> m_d80010_ram;
	required_shared_ptr<u16> m_d80060_ram;

	// video-related
	tilemap_t *m_tilemap[3]{};
	u16 m_scrollx[3]{};
	u16 m_scrolly[3]{};
	u16 m_bg_gfxset[2]{};
#ifdef MAME_DEBUG
	u8 m_dislayer[4]{};
#endif

	// sound-related
	u8 m_msm_bank = 0U;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<mb3773_device> m_watchdog;
	required_device<okim6295_device> m_oki;
	required_device<es8712_device> m_essnd;
	required_device<excellent_spr_device> m_sprgen;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	void d80010_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void d80040_w(offs_t offset, u8 data);
	void d80060_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void bank_w(u8 data);
	void eeprom_w(u8 data);
	void es8712_reset_w(u8 data);
	void tilemaps_word_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	TILE_GET_INFO_MEMBER(get_bg0_tile_info);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	void colpri_cb(u32 &colour, u32 &pri_mask);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_cb);

	void program_map(address_map &map) ATTR_COLD;
};


/*******************************************************************/


TILE_GET_INFO_MEMBER(gcpinbal_state::get_bg0_tile_info)
{
	const u16 tile = m_tilemapram[0 + tile_index * 2];
	const u16 attr = m_tilemapram[1 + tile_index * 2];

	tileinfo.set(0,
			(tile & 0xfff) + m_bg_gfxset[0],
			(attr & 0x1f),
			TILE_FLIPYX((attr & 0x300) >> 8));
}

TILE_GET_INFO_MEMBER(gcpinbal_state::get_bg1_tile_info)
{
	const u16 tile = m_tilemapram[0x800 + tile_index * 2];
	const u16 attr = m_tilemapram[0x801 + tile_index * 2];

	tileinfo.set(0,
			(tile & 0xfff) + 0x2000 + m_bg_gfxset[1],
			(attr & 0x1f) + 0x30,
			TILE_FLIPYX((attr & 0x300) >> 8));
}

TILE_GET_INFO_MEMBER(gcpinbal_state::get_fg_tile_info)
{
	const u16 tile = m_tilemapram[0x1000 + tile_index];
	tileinfo.set(1, (tile & 0xfff), (tile >> 12), 0);
}

void gcpinbal_state::video_start()
{
	int xoffs = 0;
	int yoffs = 0;

	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gcpinbal_state::get_bg0_tile_info)),TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gcpinbal_state::get_bg1_tile_info)),TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gcpinbal_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS,  8,  8, 64, 64);

	m_tilemap[0]->set_transparent_pen(0);
	m_tilemap[1]->set_transparent_pen(0);
	m_tilemap[2]->set_transparent_pen(0);

	// flipscreen n/a
	m_tilemap[0]->set_scrolldx(-xoffs, 0);
	m_tilemap[1]->set_scrolldx(-xoffs, 0);
	m_tilemap[2]->set_scrolldx(-xoffs, 0);
	m_tilemap[0]->set_scrolldy(-yoffs, 0);
	m_tilemap[1]->set_scrolldy(-yoffs, 0);
	m_tilemap[2]->set_scrolldy(-yoffs, 0);
}

void gcpinbal_state::colpri_cb(u32 &colour, u32 &pri_mask)
{
	pri_mask = (m_d80060_ram[0x8 / 2] & 0x8800) ? 0xf0 : 0xfc;
}


/******************************************************************
                   TILEMAP READ AND WRITE HANDLERS
*******************************************************************/

void gcpinbal_state::tilemaps_word_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_tilemapram[offset]);

	if (offset < 0x800) // BG0
		m_tilemap[0]->mark_tile_dirty(offset / 2);
	else if ((offset < 0x1000)) // BG1
		m_tilemap[1]->mark_tile_dirty((offset % 0x800) / 2);
	else if ((offset < 0x1800)) // FG
		m_tilemap[2]->mark_tile_dirty((offset % 0x800));
}


/**************************************************************
                        SCREEN REFRESH
**************************************************************/

uint32_t gcpinbal_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t layer[3];

#ifdef MAME_DEBUG
	if (machine().input().code_pressed_once(KEYCODE_V))
	{
		m_dislayer[0] ^= 1;
		popmessage("bg0: %01x", m_dislayer[0]);
	}

	if (machine().input().code_pressed_once(KEYCODE_B))
	{
		m_dislayer[1] ^= 1;
		popmessage("bg1: %01x", m_dislayer[1]);
	}

	if (machine().input().code_pressed_once(KEYCODE_N))
	{
		m_dislayer[2] ^= 1;
		popmessage("fg: %01x", m_dislayer[2]);
	}
#endif

	m_scrollx[0] = m_d80010_ram[0x4 / 2];
	m_scrolly[0] = m_d80010_ram[0x6 / 2];
	m_scrollx[1] = m_d80010_ram[0x8 / 2];
	m_scrolly[1] = m_d80010_ram[0xa / 2];
	m_scrollx[2] = m_d80010_ram[0xc / 2];
	m_scrolly[2] = m_d80010_ram[0xe / 2];

	for (int i = 0; i < 3; i++)
	{
		m_tilemap[i]->set_scrollx(0, m_scrollx[i]);
		m_tilemap[i]->set_scrolly(0, m_scrolly[i]);
	}

	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect);

	layer[0] = 0;
	layer[1] = 1;
	layer[2] = 2;


#ifdef MAME_DEBUG
	if (m_dislayer[layer[0]] == 0)
#endif
	m_tilemap[layer[0]]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);

#ifdef MAME_DEBUG
	if (m_dislayer[layer[1]] == 0)
#endif
	m_tilemap[layer[1]]->draw(screen, bitmap, cliprect, 0, 2);

#ifdef MAME_DEBUG
	if (m_dislayer[layer[2]] == 0)
#endif
	m_tilemap[layer[2]]->draw(screen, bitmap, cliprect, 0, 4);

	m_sprgen->gcpinbal_draw_sprites(screen, bitmap, cliprect, 16);

	return 0;
}


/***********************************************************
                      INTERRUPTS
***********************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(gcpinbal_state::scanline_cb)
{
	if (param >= 16)
		m_screen->update_partial(m_screen->vpos() - 1);

	if (param == 240)
		m_maincpu->set_input_line(1, HOLD_LINE); // V-blank
	else if ((param >= 16) && (param < 240))
		m_maincpu->set_input_line(4, HOLD_LINE); // H-blank? (or programmable, used for raster effects)

	// IRQ level 3 is sound related, hooked up to MSM6585
}

/***********************************************************
                          IOC
***********************************************************/

void gcpinbal_state::d80010_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOGIOC("CPU #0 PC %06x: warning - write ioc offset %06x with %04x\n", m_maincpu->pc(), offset, data);
	COMBINE_DATA(&m_d80010_ram[offset]);
}

void gcpinbal_state::d80040_w(offs_t offset, u8 data)
{
	LOGIOC("Writing byte value %02X to offset %X\n", data, offset);
}

void gcpinbal_state::d80060_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOGIOC("CPU #0 PC %06x: warning - write ioc offset %06x with %04x\n", m_maincpu->pc(), offset, data);
	COMBINE_DATA(&m_d80060_ram[offset]);
}

void gcpinbal_state::bank_w(u8 data)
{
	// MSM6585 bank, coin LEDs, maybe others?
	if (m_msm_bank != ((data & 0x10) >> 4))
	{
		m_msm_bank = ((data & 0x10) >> 4);
		m_essnd->set_rom_bank(m_msm_bank);
		LOGESBANKSW("Bankswitching ES8712 ROM %02x\n", m_msm_bank);
	}
	m_oki->set_rom_bank((data & 0x20) >> 5);

	u32 old = m_bg_gfxset[0];
	u32 newbank = (data & 0x04) ? 0x1000 : 0;
	if (old != newbank)
	{
		m_bg_gfxset[0] = (data & 0x04) ? 0x1000 : 0;
		m_tilemap[0]->mark_all_dirty();
	}

	old = m_bg_gfxset[1];
	newbank = (data & 0x08) ? 0x1000 : 0;
	if (old != newbank)
	{
		m_bg_gfxset[1] = (data & 0x04) ? 0x1000 : 0;
		m_tilemap[1]->mark_all_dirty();
	}

	m_watchdog->write_line_ck(BIT(data, 7));

//          machine().bookkeeping().coin_lockout_w(0, ~data & 0x01);
//          machine().bookkeeping().coin_lockout_w(1, ~data & 0x02);
}

void gcpinbal_state::eeprom_w(u8 data)
{
	// 93C46 serial EEPROM (status read at D80087)
	m_eeprom->di_write(BIT(data, 2));
	m_eeprom->clk_write(BIT(data, 1));
	m_eeprom->cs_write(BIT(data, 0));
}

void gcpinbal_state::es8712_reset_w(u8 data)
{
	// This probably works by resetting the ES-8712
	m_essnd->reset();
}


/***********************************************************
                     MEMORY STRUCTURES
***********************************************************/

void gcpinbal_state::program_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
	map(0xc00000, 0xc03fff).ram().w(FUNC(gcpinbal_state::tilemaps_word_w)).share(m_tilemapram);
	map(0xc80000, 0xc81fff).rw(m_sprgen, FUNC(excellent_spr_device::read), FUNC(excellent_spr_device::write)).umask16(0x00ff);
	map(0xd00000, 0xd00fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xd80010, 0xd8002f).ram().w(FUNC(gcpinbal_state::d80010_w)).share(m_d80010_ram);
	map(0xd80040, 0xd8005b).w(FUNC(gcpinbal_state::d80040_w)).umask16(0x00ff);
	map(0xd80060, 0xd80077).ram().w(FUNC(gcpinbal_state::d80060_w)).share(m_d80060_ram);
	map(0xd80080, 0xd80081).portr("DSW");
	map(0xd80084, 0xd80085).portr("IN0");
	map(0xd80086, 0xd80087).portr("IN1");
	map(0xd80088, 0xd80088).w(FUNC(gcpinbal_state::bank_w));
	map(0xd8008a, 0xd8008a).w(FUNC(gcpinbal_state::eeprom_w));
	map(0xd8008e, 0xd8008e).w(FUNC(gcpinbal_state::es8712_reset_w));
	map(0xd800a0, 0xd800a0).mirror(0x2).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xd800c0, 0xd800cd).w(m_essnd, FUNC(es8712_device::write)).umask16(0xff00);
	map(0xff0000, 0xffffff).ram();
}



/***********************************************************
                   INPUT PORTS, DIPs
***********************************************************/

static INPUT_PORTS_START( gcpinbal )
	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0004, "300k" )
	PORT_DIPSETTING(      0x0008, "500k" )
	PORT_DIPSETTING(      0x000c, "1000k" )
	PORT_DIPSETTING(      0x0000, DEF_STR( None ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:5")   // Confirmed via manual - code at 0x000508
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")   // Confirmed via manual - code at 0x00b6d0, 0x00b7e4, 0x00bae4
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0200, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0100, "2 Coins/1 Credit 3/2 4/3 6/5" )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0500, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(      0x0000, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x1000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0800, "2 Coins/1 Credit 3/2 4/3 6/5" )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x2800, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(      0x0000, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0xc000, "3" )
	PORT_DIPSETTING(      0x8000, "4" )
	PORT_DIPSETTING(      0x4000, "5" )

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Item Right") PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Flipper 1 Right") PORT_PLAYER(1)   // Inner flipper right
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Flipper 2 Right") PORT_PLAYER(1)   // Outer flipper right
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Tilt Right") PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Item Left") PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Flipper 1 Left") PORT_PLAYER(1)   // Inner flipper left
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Flipper 2 Left") PORT_PLAYER(1)   // Outer flipper left
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Tilt Left") PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



/**************************************************************
                       GFX DECODING
**************************************************************/

static GFXDECODE_START( gfx_gcpinbal )
	GFXDECODE_ENTRY( "bg0", 0, gfx_16x16x4_packed_msb,     0, 0x60 )  // playfield
	GFXDECODE_ENTRY( "fg0", 0, gfx_8x8x4_packed_msb,   0x700, 0x10 )  // playfield
GFXDECODE_END


/***********************************************************
                        MACHINE DRIVERS
***********************************************************/

void gcpinbal_state::machine_start()
{
	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
	save_item(NAME(m_bg_gfxset));
	save_item(NAME(m_msm_bank));
}

void gcpinbal_state::machine_reset()
{
	for (int i = 0; i < 3; i++)
	{
		m_scrollx[i] = 0;
		m_scrolly[i] = 0;
	}

	m_bg_gfxset[0] = 0;
	m_bg_gfxset[1] = 0;
	m_msm_bank = 0;
}

void gcpinbal_state::gcpinbal(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 32_MHz_XTAL / 2); // 16 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &gcpinbal_state::program_map);

	TIMER(config, "scantimer").configure_scanline(FUNC(gcpinbal_state::scanline_cb), "screen", 0, 1);

	EEPROM_93C46_16BIT(config, "eeprom");

	MB3773(config, m_watchdog, 0);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0)); // frames per second, vblank duration
	m_screen->set_size(40*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(gcpinbal_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gcpinbal);
	PALETTE(config, m_palette).set_format(palette_device::RRRRGGGGBBBBRGBx, 0x1000 / 2);

	EXCELLENT_SPRITE(config, m_sprgen, 0);
	m_sprgen->set_palette(m_palette);
	m_sprgen->set_color_base(0x600);
	m_sprgen->set_colpri_callback(FUNC(gcpinbal_state::colpri_cb));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 1.056_MHz_XTAL, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.30);

	ES8712(config, m_essnd, 0);
	m_essnd->reset_handler().set_inputline("maincpu", 3);
	m_essnd->msm_write_handler().set("msm", FUNC(msm6585_device::data_w));
	m_essnd->set_msm_tag("msm");

	msm6585_device &msm(MSM6585(config, "msm", 640_kHz_XTAL));
	msm.vck_legacy_callback().set("essnd", FUNC(es8712_device::msm_int));
	msm.set_prescaler_selector(msm6585_device::S40); // 16 kHz
	msm.add_route(ALL_OUTPUTS, "mono", 1.0);
}



/***************************************************************************
                                  DRIVERS
***************************************************************************/

ROM_START( pwrflip ) // Updated version of Grand Cross Pinball or semi-sequel?
	ROM_REGION( 0x200000, "maincpu", 0 )  // 68000
	ROM_LOAD16_WORD_SWAP( "p.f_1.33.u43",  0x000000, 0x80000, CRC(d760c987) SHA1(9200604377542193afc866c84733f2d3b5aa1c80) ) // hand written labels on genuine EXCELLENT labels
	ROM_FILL            ( 0x80000,  0x080000, 0x00 ) // unpopulated 27C4096 socket at U44
	ROM_LOAD16_WORD_SWAP( "p.f.u45",       0x100000, 0x80000, CRC(6ad1a457) SHA1(8746c38efa05e3318e9b1a371470d149803fb6bb) )
	ROM_LOAD16_WORD_SWAP( "p.f.u46",       0x180000, 0x80000, CRC(e0f3a1b4) SHA1(761dddf374a92c1a1e4a211ead215d5be461a082) )

	ROM_REGION( 0x200000, "bg0", 0 )  // 16 x 16
	ROM_LOAD16_WORD_SWAP( "u1",      0x000000, 0x100000, CRC(afa459bb) SHA1(7a7c64bcb80d71b8cf3fdd3209ef109997b6417c) ) // 23C8000 mask ROMs
	ROM_LOAD16_WORD_SWAP( "u6",      0x100000, 0x100000, CRC(c3f024e5) SHA1(d197e2b715b154fc64ff9a61f8c6df111d6fd446) )

	ROM_REGION( 0x020000, "fg0", 0 )  // 8 x 8
	ROM_LOAD16_WORD_SWAP( "p.f.u10",   0x000000, 0x020000, CRC(50e34549) SHA1(ca1808513ff3feb8bcd34d9aafd7b374e4244732) )

	ROM_REGION( 0x200000, "spritegen", 0 )  // 16 x 16
	ROM_LOAD16_WORD_SWAP( "u13",     0x000000, 0x200000, CRC(62f3952f) SHA1(7dc9ccb753d46b6aaa791bcbf6e18e6d872f6b79) ) // 23C16000 mask ROM

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "u55",   0x000000, 0x080000, CRC(b3063351) SHA1(825e63e8a824d67d235178897528e5b0b41e4485) ) // OKI M534001B mask ROM

	ROM_REGION( 0x200000, "essnd", 0 )
	ROM_LOAD( "u56",   0x000000, 0x200000, CRC(092b2c0f) SHA1(2ec1904e473ddddb50dbeaa0b561642064d45336) ) // 23C16000 mask ROM

	ROM_REGION( 0x000400, "plds", 0 ) // 2x TIBPAL16L8-15CN
	ROM_LOAD( "a.u72", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "b.u73", 0x200, 0x104, NO_DUMP )
ROM_END

ROM_START( gcpinbal )
	ROM_REGION( 0x200000, "maincpu", 0 )  // 68000
	ROM_LOAD16_WORD_SWAP( "2_excellent.u43",  0x000000, 0x80000, CRC(d174bd7f) SHA1(0e6c17265e1400de941e3e2ca3be835aaaff6695) ) // Red line across label
	ROM_FILL            ( 0x80000,  0x080000, 0x00 ) // unpopulated 27C4096 socket at U44
	ROM_LOAD16_WORD_SWAP( "3_excellent.u45",  0x100000, 0x80000, CRC(0511ad56) SHA1(e0602ece514126ce719ebc9de6649ebe907be904) )
	ROM_LOAD16_WORD_SWAP( "4_excellent.u46",  0x180000, 0x80000, CRC(e0f3a1b4) SHA1(761dddf374a92c1a1e4a211ead215d5be461a082) )

	ROM_REGION( 0x200000, "bg0", 0 )  // 16 x 16
	ROM_LOAD16_WORD_SWAP( "u1",      0x000000, 0x100000, CRC(afa459bb) SHA1(7a7c64bcb80d71b8cf3fdd3209ef109997b6417c) ) // 23C8000 mask ROMs
	ROM_LOAD16_WORD_SWAP( "u6",      0x100000, 0x100000, CRC(c3f024e5) SHA1(d197e2b715b154fc64ff9a61f8c6df111d6fd446) )

	ROM_REGION( 0x020000, "fg0", 0 )  // 8 x 8
	ROM_LOAD16_WORD_SWAP( "1_excellent.u10",   0x000000, 0x020000, CRC(79321550) SHA1(61f1b772ed8cf95bfee9df8394b0c3ff727e8702) )

	ROM_REGION( 0x200000, "spritegen", 0 )  // 16 x 16
	ROM_LOAD16_WORD_SWAP( "u13",     0x000000, 0x200000, CRC(62f3952f) SHA1(7dc9ccb753d46b6aaa791bcbf6e18e6d872f6b79) ) // 23C16000 mask ROM

	ROM_REGION( 0x080000, "oki", 0 )
	ROM_LOAD( "u55",   0x000000, 0x080000, CRC(b3063351) SHA1(825e63e8a824d67d235178897528e5b0b41e4485) ) // OKI M534001B mask ROM

	ROM_REGION( 0x200000, "essnd", 0 )
	ROM_LOAD( "u56",   0x000000, 0x200000, CRC(092b2c0f) SHA1(2ec1904e473ddddb50dbeaa0b561642064d45336) ) // 23C16000 mask ROM

	ROM_REGION( 0x000400, "plds", 0 ) // 2x TIBPAL16L8-15CN
	ROM_LOAD( "a.u72", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "b.u73", 0x200, 0x104, NO_DUMP )
ROM_END

} // anonymous namespace


GAME( 1994, pwrflip,  0, gcpinbal, gcpinbal, gcpinbal_state, empty_init, ROT270, "Excellent System", "Power Flipper Pinball Shooting (v1.33)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1994, gcpinbal, 0, gcpinbal, gcpinbal, gcpinbal_state, empty_init, ROT270, "Excellent System", "Grand Cross (v1.02F)",                   MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

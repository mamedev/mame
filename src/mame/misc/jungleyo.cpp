// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/******************************************************************************

    Jungle (c) 2001 Yonshi
    Fruit Genie (c) 2003 Global

    TODO:
    Jungle:
    - with a clean NVRAM MAME needs to be soft reset after init or the game
        will trip a '1111 exception' (caused by invalid opcode executed at
        0x102, incomplete decryption most likely);
    - Likewise anything in the 0x100-0x1f7 range doesn't seem valid at all;
    - game sometimes expects 1+ coins even if player has available points
      (and freezing with "COIN" text blinking), very unlikely to be intended
      behaviour?
    - system setting screen shows the following settings that don't seem to be
        affected by dips:
      * Min. Bet (always 1),
      * Credit X Ticket Mode (always Cencel (sic)),
      * Max. 10 Mode (always Max. 10);
    - sound doesn't seem to work 100% correctly (i.e. coin sound only seems
        to work from 3rd coin on, lots of invalid sample msgs in error.log).
        fwiw there's no extra OKI bank in the ROM, must be either invalid
        decryption or fancy OKI status readback instead.
        Latter may be confirmed by video display stalling at the end of a
        normal round, OSD coin counter gets updated after the BGM completes
        playback.
    - outputs (lamps & ticket dispenser at very least);

    Fruit Genie:
    - decryption seems good but gets stuck with 'data error' and 'illegal inst'
        errors. Second one probably due to same problem as jungleyo. First one
        possibly checksum failure due to patch in init_frtgenie()?
        To bypass do:
        frtgenie, frtgeniea: bp 5732,1,{curpc=0x5810;g}
        frtgenieb: bp 581e,1,{curpc=0x58fe;g}
        frtgeniec: bp 5812,1,{curpc=0x58f0;g}
        frtgenied: bp 80de,1,{curpc=0x81bc;g}
    - second half of frtgenie's main CPU ROM seems to contain an earlier version
      of the data 'GENIE FRUITS DATA: 2001/08/15 VERSION: VA1.00'. Can it be
      reached or just a leftover?
    - interestingly not only the program ROMs, but the GFX ROMs differ for the
      various sets. Only the Oki ROM is always identical
    - it hits the layer_enable_w popmessage
    - title screen uses 4th 'reel'. Not implemented yet.

   Magical Jack (Plus)
    - with a clean NVRAM MAME needs to be soft reset after init or the game
        will trip a '1111 exception';

===============================================================================

CPUs
QTY  Type           clock      position  function
1x   MC68HC000FN10             u3        16/32-bit Microprocessor - main
1x   u6295                     u98       4-Channel Mixing ADCPM Voice Synthesis LSI - sound
1x   HA17358                   u101      Dual Operational Amplifier - sound
1x   TDA2003                   u104      Audio Amplifier - sound
1x   oscillator     24.000MHz  osc1

ROMs
QTY  Type      position  status
2x   M27C1001  2,3       dumped
1x   M27C2001  1         dumped
3x   M27C4001  4,5,6     dumped

RAMs
QTY  Type            position
11x  LH52B256-10PLL  u16a,u17a,u27,u28,u29,u30,u39,u40,u74,u75,u76

PLDs
QTY  Type            position  status
1x   ATF20V8B-15PC   u37       read protected
2x   A40MX04-F-PL84  u83,u86   read protected

Others
1x 28x2 JAMMA edge connector
1x pushbutton (SW1)
1x trimmer (volume) (VR1)
4x 8x2 switches DIP (SW1,SW2,SW3,SW4)
1x battery 3.6V (BT1)

Notes
PCB silkscreened: "MADE IN TAIWAN YONSHI PCB NO-006F"

*******************************************************************************/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/nvram.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

// configurable logging
#define LOG_OUTPUTS (1U << 1)

// #define VERBOSE (2)

#include "logmacro.h"

#define LOGOUTPUTS(...) LOGMASKED(LOG_OUTPUTS, __VA_ARGS__)

namespace {

class jungleyo_state : public driver_device
{
public:
	jungleyo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_bg_videoram(*this, "bg_videoram")
		, m_fg_videoram(*this, "fg_videoram")
		, m_reel_vram(*this, "reel_vram%u", 1U)
		, m_palette(*this, "palette")
		, m_paletteram(*this, "paletteram", 0x18000, ENDIANNESS_BIG)
	{ }

	void jungleyo(machine_config &config);

	void init_frtgenie();
	void init_jungleyo();
	template <uint16_t Reset_addr> void init_magjack();

protected:
	virtual void video_start() override ATTR_COLD;

private:
	void output_w(u16 data);

	// video-related
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	template <int Layer> TILE_GET_INFO_MEMBER(get_reel_tile_info);
	void bg_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void fg_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	template <int Layer> void reel_vram_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void main_map(address_map &map) ATTR_COLD;

	u8 palette_ram_r(offs_t offset);
	void palette_ram_w(offs_t offset, u8 data);
	void layer_enable_w(u8 data);
	void video_priority_w(u8 data);

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<u16> m_bg_videoram;
	required_shared_ptr<u16> m_fg_videoram;
	required_shared_ptr_array<u16, 3> m_reel_vram;
	required_device<palette_device> m_palette;
	memory_share_creator<u8> m_paletteram;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_reel_tilemap[3];
	u8 m_layer_enable;
	u8 m_video_priority;
};


TILE_GET_INFO_MEMBER(jungleyo_state::get_bg_tile_info)
{
	u16 code = m_bg_videoram[tile_index*2+1];
	u16 color = m_bg_videoram[tile_index*2];
	tileinfo.set(1, code, color & 0x1f, 0);
}

TILE_GET_INFO_MEMBER(jungleyo_state::get_fg_tile_info)
{
	u16 code = m_fg_videoram[tile_index*2+1];
	u16 color = m_fg_videoram[tile_index*2];
	tileinfo.set(2, code, color & 0x1f, 0);
}

template <int Layer> TILE_GET_INFO_MEMBER(jungleyo_state::get_reel_tile_info)
{
	u16 code = m_reel_vram[Layer][tile_index*2+1];
	// colscroll is on upper 8 bits of this (handled in update)
	u16 color = m_reel_vram[Layer][tile_index*2];
	// TODO: confirm if bit 6 is really connected here
	// upper palette bank is initialized with black at POST and never ever touched again,
	// not enough to pinpoint one way or another ...
	tileinfo.set(0, code, color & 0x3f, 0);
}

void jungleyo_state::bg_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

void jungleyo_state::fg_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

template <int Layer> void jungleyo_state::reel_vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_reel_vram[Layer][offset]);
	m_reel_tilemap[Layer]->mark_tile_dirty(offset / 2);
}

void jungleyo_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(jungleyo_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(jungleyo_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_reel_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(jungleyo_state::get_reel_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(jungleyo_state::get_reel_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(jungleyo_state::get_reel_tile_info<2>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);

	m_fg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_transparent_pen(0);
	for (auto &reel : m_reel_tilemap)
	{
		reel->set_scroll_cols(64);
		reel->set_transparent_pen(0xff);
	}

	m_layer_enable = 0;
	m_video_priority = 0;

	save_item(NAME(m_layer_enable));
	save_item(NAME(m_video_priority));
}

u32 jungleyo_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	if (m_layer_enable == 0)
		return 0;

	for (int Layer = 0; Layer < 3; Layer++)
	{
		for (int i = 0; i < 64; i++)
		{
			u16 scroll = m_reel_vram[Layer][i*2] >> 8;
			m_reel_tilemap[Layer]->set_scrolly(i, scroll);
		}

		m_reel_tilemap[Layer]->draw(screen, bitmap, cliprect, 0, 0);
	}

	if ((m_video_priority & 1) == 0)
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	if ((m_video_priority & 1) == 1)
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

u8 jungleyo_state::palette_ram_r(offs_t offset)
{
	return m_paletteram[offset];
}

void jungleyo_state::palette_ram_w(offs_t offset, u8 data)
{
	// RGB888 in three separate banks
	m_paletteram[offset] = data;
	const u32 pal_offs = offset & 0x7fff;
	const int b = (m_paletteram[pal_offs | 0x00000] & 0xff);
	const int g = (m_paletteram[pal_offs | 0x08000] & 0xff);
	const int r = (m_paletteram[pal_offs | 0x10000] & 0xff);

	m_palette->set_pen_color(pal_offs, rgb_t(r, g, b));
}

void jungleyo_state::output_w(u16 data)
{
	// bit 15  ?
	// bit 14  coin counter?
	// bit 13  ?
	// bit 12  ?
	// bit 11  ?
	// bit 10  ?
	// bit 09  ?
	// bit 08  ?
	// bit 07  ?
	// bit 06  ?
	// bit 05  ?
	// bit 04  ?
	// bit 03  ?
	// bit 02  ?
	// bit 01  start lamp?
	// bit 00  bet lamp?

	machine().bookkeeping().coin_counter_w(0, BIT(data, 14));

	if (data & 0xbfff)
		LOGOUTPUTS("%s output_w: %04x\n", machine().describe_context(), data);
}

void jungleyo_state::layer_enable_w(u8 data)
{
	m_layer_enable = data & 7;
	// TODO: we just know that 7 enables and 0 disables all 3 layers, how the composition works out internally is MIA
	if (((m_layer_enable != 7) && (m_layer_enable != 0)) || data & 0xf8)
		popmessage("layer enable %02x contact MAMEdev", data);
}

void jungleyo_state::video_priority_w(u8 data)
{
	m_video_priority = data & 1;
	if (data & 0xfe)
		popmessage("video priority %02x contact MAMEdev", data & 0xfe);
}

void jungleyo_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("maincpu", 0);
	map(0xa00310, 0xa00311).portr("IN0");
	map(0xa0032a, 0xa0032b).portr("DSW12");
	map(0xa0032c, 0xa0032d).portr("DSW34");
	map(0xa00336, 0xa00337).w(FUNC(jungleyo_state::output_w)); // outputs? observed values: lots
	map(0xa00689, 0xa00689).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	//map(0xa0068a, 0xa0068b).nopw(); // observed values: 0x0101
	//map(0xa0082e, 0xa0082f).nopw(); // observed values: 0x0000, 0x4000, 0x5900 (double up),
	map(0xa00831, 0xa00831).w(FUNC(jungleyo_state::layer_enable_w));
	map(0xa00833, 0xa00833).w(FUNC(jungleyo_state::video_priority_w));
	map(0xa00aaa, 0xa00aab).nopw(); // irq ack or watchdog (written at start of vblank irq)
	//map(0xa00d58, 0xa00d59).nopw(); // observed values: 0x0004
	//map(0xa00e9a, 0xa00e9b).nopw(); // observed values: 0x0005
	//map(0xa00fc6, 0xa00fc7).nopw(); // observed values: 0x0006
	map(0xb00000, 0xb2ffff).rw(FUNC(jungleyo_state::palette_ram_r), FUNC(jungleyo_state::palette_ram_w)).umask16(0x00ff);
	map(0xb80000, 0xb81fff).ram().share(m_fg_videoram).w(FUNC(jungleyo_state::fg_videoram_w));
	map(0xb90000, 0xb91fff).ram().share(m_bg_videoram).w(FUNC(jungleyo_state::bg_videoram_w));
	map(0xba0000, 0xba07ff).ram().share(m_reel_vram[0]).w(FUNC(jungleyo_state::reel_vram_w<0>));
	map(0xba0800, 0xba0fff).ram().share(m_reel_vram[1]).w(FUNC(jungleyo_state::reel_vram_w<1>));
	map(0xba1000, 0xba17ff).ram().share(m_reel_vram[2]).w(FUNC(jungleyo_state::reel_vram_w<2>));
	map(0xba1800, 0xba1fff).ram(); // supposedly the 4th reel VRAM bank, inited at POST only and probably just tied to NOP instead

	map(0xff0000, 0xff7fff).ram();
	map(0xff8000, 0xffffff).ram().share("nvram");
}


static INPUT_PORTS_START( jungleyo )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Bet / Stop All")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start / 2 Double Up")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SLOT_STOP4 ) PORT_NAME("Stop4 / Take")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop2 / Double Up")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop1 / Big")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop3 / Small")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) // bookkeeping
	PORT_SERVICE( 0x100, IP_ACTIVE_LOW ) // if active high at boot the game shows the input test, if switched to input high after boot it shows system settings
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in input test
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) // 'key down' in input test. Are they the same thing?
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in input test
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in input test
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("Ticket Sw.")

	PORT_START("DSW12")
	PORT_DIPNAME( 0x0007, 0x0007, "Main Game Rate" ) PORT_DIPLOCATION("DSW1:1,2,3")
	PORT_DIPSETTING(      0x0007, "98%" )
	PORT_DIPSETTING(      0x0003, "96%" )
	PORT_DIPSETTING(      0x0005, "94%" )
	PORT_DIPSETTING(      0x0001, "92%" )
	PORT_DIPSETTING(      0x0006, "90%" )
	PORT_DIPSETTING(      0x0002, "88%" )
	PORT_DIPSETTING(      0x0004, "86%" )
	PORT_DIPSETTING(      0x0000, "84%" )
	PORT_DIPNAME( 0x0018, 0x0018, "Max. Credits Bet" ) PORT_DIPLOCATION("DSW1:4,5")
	PORT_DIPSETTING(      0x0018, "10" )
	PORT_DIPSETTING(      0x0008, "15" )
	PORT_DIPSETTING(      0x0010, "18" )
	PORT_DIPSETTING(      0x0000, "20" )
	PORT_DIPNAME( 0x0060, 0x0060, "Max. Points Bet" ) PORT_DIPLOCATION("DSW1:6,7")
	PORT_DIPSETTING(      0x0060, "10" )
	PORT_DIPSETTING(      0x0020, "15" )
	PORT_DIPSETTING(      0x0040, "18" )
	PORT_DIPSETTING(      0x0000, "20" )
	PORT_DIPNAME( 0x0080, 0x0080, "Bill Time" ) PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(      0x0080, "Anytime" )
	PORT_DIPSETTING(      0x0000, "Zero" )
	PORT_DIPNAME( 0x0700, 0x0700, "Coin In" ) PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(      0x0700, "1" )
	PORT_DIPSETTING(      0x0300, "2" )
	PORT_DIPSETTING(      0x0500, "5" )
	PORT_DIPSETTING(      0x0100, "10" )
	PORT_DIPSETTING(      0x0600, "20" )
	PORT_DIPSETTING(      0x0200, "25" )
	PORT_DIPSETTING(      0x0400, "50" )
	PORT_DIPSETTING(      0x0000, "100" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0800, 0x0800, "DSW2:4" ) // no effect in system settings
	PORT_DIPUNKNOWN_DIPLOC( 0x1000, 0x1000, "DSW2:5" ) // no effect in system settings
	PORT_DIPUNKNOWN_DIPLOC( 0x2000, 0x2000, "DSW2:6" ) // no effect in system settings
	PORT_DIPUNKNOWN_DIPLOC( 0x4000, 0x4000, "DSW2:7" ) // no effect in system settings
	PORT_DIPUNKNOWN_DIPLOC( 0x8000, 0x8000, "DSW2:8" ) // no effect in system settings

	PORT_START("DSW34")
	PORT_DIPUNKNOWN_DIPLOC( 0x0001, 0x0001, "DSW3:1" ) // no effect in system settings
	PORT_DIPUNKNOWN_DIPLOC( 0x0002, 0x0002, "DSW3:2" ) // no effect in system settings
	PORT_DIPUNKNOWN_DIPLOC( 0x0004, 0x0004, "DSW3:3" ) // no effect in system settings
	PORT_DIPUNKNOWN_DIPLOC( 0x0008, 0x0008, "DSW3:4" ) // no effect in system settings
	PORT_DIPNAME( 0x0030, 0x0030, "Game Limit" ) PORT_DIPLOCATION("DSW3:5,6")
	PORT_DIPSETTING(      0x0030, "10000" )
	PORT_DIPSETTING(      0x0010, "30000" )
	PORT_DIPSETTING(      0x0020, "60000" )
	PORT_DIPSETTING(      0x0000, "100000" )
	PORT_DIPNAME( 0x0040, 0x0040, "Credit Limit" ) PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(      0x0040, "5000" )
	PORT_DIPSETTING(      0x0000, "10000" )
	PORT_DIPNAME( 0x0080, 0x0080, "Fever Min. Bet" ) PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(      0x0080, "10" )
	PORT_DIPSETTING(      0x0000, "20" )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DSW4:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Yes ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPNAME( 0x0200, 0x0200, "Skill Mode" ) PORT_DIPLOCATION("DSW4:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Yes ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Double Up Rate" ) PORT_DIPLOCATION("DSW4:3")
	PORT_DIPSETTING(      0x0400, "92%" )
	PORT_DIPSETTING(      0x0000, "96%" )
	PORT_DIPNAME( 0x0800, 0x0800, "Double Up Game" ) PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Yes ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPNAME( 0x1000, 0x1000, "Reel Speed" ) PORT_DIPLOCATION("DSW4:5") // misspellt 'Rell'
	PORT_DIPSETTING(      0x1000, "Slow" )
	PORT_DIPSETTING(      0x0000, "Fast" )
	PORT_DIPNAME( 0x2000, 0x2000, "Take Score Speed" ) PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(      0x2000, "Slow" )
	PORT_DIPSETTING(      0x0000, "Fast" )
	PORT_DIPUNKNOWN_DIPLOC( 0x4000, 0x4000, "DSW4:7" ) // no effect in system settings
	PORT_DIPUNKNOWN_DIPLOC( 0x8000, 0x8000, "DSW4:8" ) // no effect in system settings
INPUT_PORTS_END

static INPUT_PORTS_START( frtgenie )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Play")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("Ticket Sw.")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in input test
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Take / Hold / Stop3")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Double Up / Stop1")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) // 'account' in input test
	PORT_SERVICE( 0x100, IP_ACTIVE_LOW ) // if active high at boot the game shows the input test, if switched to input high after boot it shows system settings
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SLOT_STOP_ALL ) PORT_NAME("Big / All Stop") // no effect in input test
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Small / Stop2")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in input test
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in input test
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )

	PORT_START("DSW12")
	PORT_DIPNAME( 0x0007, 0x0007, "Main Game Rate" ) PORT_DIPLOCATION("DSW1:1,2,3")
	PORT_DIPSETTING(      0x0007, "90%" )
	PORT_DIPSETTING(      0x0003, "85%" )
	PORT_DIPSETTING(      0x0005, "80%" )
	PORT_DIPSETTING(      0x0001, "75%" )
	PORT_DIPSETTING(      0x0006, "70%" )
	PORT_DIPSETTING(      0x0002, "65%" )
	PORT_DIPSETTING(      0x0004, "60%" )
	PORT_DIPSETTING(      0x0000, "55%" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0008, 0x0008, "DSW1:4" ) // no effect in system settings
	PORT_DIPNAME( 0x0010, 0x0010, "Max. Play" ) PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(      0x0010, "40" )
	PORT_DIPSETTING(      0x0000, "80" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0020, 0x0020, "DSW1:6" ) // no effect in system settings
	PORT_DIPNAME( 0x00c0, 0x00c0, "10 Times Feature" ) PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(      0x0000, "No (Win Points to Points)" )
	PORT_DIPSETTING(      0x0080, "No (Win Points to Score)" )
	PORT_DIPSETTING(      0x0040, "Yes (Free Spin Deducts Credits)" )
	PORT_DIPSETTING(      0x00c0, "Yes (Normal)" )
	PORT_DIPNAME( 0x0700, 0x0700, "Coin In" ) PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(      0x0700, "1" )
	PORT_DIPSETTING(      0x0300, "4" )
	PORT_DIPSETTING(      0x0500, "5" )
	PORT_DIPSETTING(      0x0100, "10" )
	PORT_DIPSETTING(      0x0600, "20" )
	PORT_DIPSETTING(      0x0200, "25" )
	PORT_DIPSETTING(      0x0400, "50" )
	PORT_DIPSETTING(      0x0000, "100" )
	PORT_DIPNAME( 0x3800, 0x3800, "Key In" ) PORT_DIPLOCATION("DSW2:4,5,6")
	PORT_DIPSETTING(      0x3800, "1" )
	PORT_DIPSETTING(      0x1800, "4" )
	PORT_DIPSETTING(      0x2800, "5" )
	PORT_DIPSETTING(      0x0800, "15" )
	PORT_DIPSETTING(      0x3000, "25" )
	PORT_DIPSETTING(      0x1000, "75" )
	PORT_DIPSETTING(      0x2000, "100" )
	PORT_DIPSETTING(      0x0000, "500" )
	PORT_DIPNAME( 0xc000, 0xc000, "Pay Out" ) PORT_DIPLOCATION("DSW2:7,8")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0xc000, "10" )
	PORT_DIPSETTING(      0x4000, "20" )
	PORT_DIPSETTING(      0x8000, "50" )

	PORT_START("DSW34")
	PORT_DIPNAME( 0x0003, 0x0003, "Game Limit" ) PORT_DIPLOCATION("DSW3:1,2")
	PORT_DIPSETTING(      0x0003, "10000" )
	PORT_DIPSETTING(      0x0001, "50000" )
	PORT_DIPSETTING(      0x0002, "100000" )
	PORT_DIPSETTING(      0x0000, "200000" )
	PORT_DIPNAME( 0x0004, 0x0004, "Credit Limit" ) PORT_DIPLOCATION("DSW3:3")
	PORT_DIPSETTING(      0x0004, "5000" )
	PORT_DIPSETTING(      0x0000, "10000" )
	PORT_DIPNAME( 0x0008, 0x0008, "Display Odds Table" ) PORT_DIPLOCATION("DSW3:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Reel Auto Stop" ) PORT_DIPLOCATION("DSW3:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Game Count" ) PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "Count" )
	PORT_DIPNAME( 0x0040, 0x0040, "Reel Speed" ) PORT_DIPLOCATION("DSW3:7") // actually spelt 'Rell Speed'
	PORT_DIPSETTING(      0x0040, "Slow" )
	PORT_DIPSETTING(      0x0000, "Fast" )
	PORT_DIPNAME( 0x0080, 0x0080, "Min. Play For Bonus" ) PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(      0x0080, "8" )
	PORT_DIPSETTING(      0x0000, "16" )
	PORT_DIPNAME( 0x0100, 0x0100, "Double Up Rate" ) PORT_DIPLOCATION("DSW4:1")
	PORT_DIPSETTING(      0x0000, "75%" )
	PORT_DIPSETTING(      0x0100, "85%" )
	PORT_DIPNAME( 0x0200, 0x0200, "Play Score" ) PORT_DIPLOCATION("DSW4:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Yes ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Pay Out Mode" ) PORT_DIPLOCATION("DSW4:3")
	PORT_DIPSETTING(      0x0400, "Manual" )
	PORT_DIPSETTING(      0x0000, "Auto" )
	PORT_DIPNAME( 0x0800, 0x0800, "Double Up Game" ) PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Yes ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPNAME( 0x1000, 0x1000, "Lose & Win Mode" ) PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xe000, 0xe000, "Key Out" ) PORT_DIPLOCATION("DSW4:6,7,8")
	PORT_DIPSETTING(      0xe000, "1" )
	PORT_DIPSETTING(      0x6000, "5" )
	PORT_DIPSETTING(      0xa000, "15" )
	PORT_DIPSETTING(      0x2000, "20" )
	PORT_DIPSETTING(      0xc000, "50" )
	PORT_DIPSETTING(      0x4000, "75" )
	PORT_DIPSETTING(      0x8000, "100" )
	PORT_DIPSETTING(      0x0000, "500" )
INPUT_PORTS_END

static INPUT_PORTS_START( frtgeniea )
	PORT_INCLUDE( frtgenie )

	PORT_MODIFY("DSW12")
	PORT_DIPNAME( 0x0010, 0x0010, "Max. Play" ) PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(      0x0010, "120" )
	PORT_DIPSETTING(      0x0000, "240" )

	PORT_MODIFY("DSW34")
	PORT_DIPNAME( 0x0003, 0x0003, "Game Limit" ) PORT_DIPLOCATION("DSW3:1,2")
	PORT_DIPSETTING(      0x0003, "50000" )
	PORT_DIPSETTING(      0x0001, "100000" )
	PORT_DIPSETTING(      0x0002, "200000" )
	PORT_DIPSETTING(      0x0000, "500000" )
INPUT_PORTS_END

static INPUT_PORTS_START( frtgenieb )
	PORT_INCLUDE( frtgenie )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_MODIFY("DSW34")
	PORT_DIPNAME( 0x0100, 0x0100, "Must Off" ) PORT_DIPLOCATION("DSW4:1") // sic
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
INPUT_PORTS_END

static INPUT_PORTS_START( frtgeniec )
	PORT_INCLUDE( frtgenieb )

	PORT_MODIFY("DSW12")
	PORT_DIPNAME( 0x0007, 0x0007, "Main Game Rate" ) PORT_DIPLOCATION("DSW1:1,2,3")
	PORT_DIPSETTING(      0x0007, "90%" )
	PORT_DIPSETTING(      0x0003, "83%" )
	PORT_DIPSETTING(      0x0005, "76%" )
	PORT_DIPSETTING(      0x0001, "69%" )
	PORT_DIPSETTING(      0x0006, "62%" )
	PORT_DIPSETTING(      0x0002, "55%" )
	PORT_DIPSETTING(      0x0004, "48%" )
	PORT_DIPSETTING(      0x0000, "41%" )

	PORT_MODIFY("DSW34")
	PORT_DIPNAME( 0x0003, 0x0003, "Game Limit" ) PORT_DIPLOCATION("DSW3:1,2")
	PORT_DIPSETTING(      0x0003, "10000" )
	PORT_DIPSETTING(      0x0001, "50000" )
	PORT_DIPSETTING(      0x0002, "100000" )
	PORT_DIPSETTING(      0x0000, "200000" )
	PORT_DIPNAME( 0x0008, 0x0008, "One Start One Ticket" ) PORT_DIPLOCATION("DSW3:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Take Score Speed" ) PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(      0x0040, "Normal" )
	PORT_DIPSETTING(      0x0000, "Fast" )
INPUT_PORTS_END

static INPUT_PORTS_START( frtgenied )
	PORT_INCLUDE( frtgenie )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Bet / All Stop")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in input test
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in input test
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Take / Hold / Stop1")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Big / Stop 2")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Small / Stop3")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("Hopper Sw")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )

	PORT_MODIFY("DSW12")
	PORT_DIPNAME( 0x0038, 0x0038, "Min. Bet" ) PORT_DIPLOCATION("DSW1:4,5,6")
	PORT_DIPSETTING(      0x0038, "1" )
	PORT_DIPSETTING(      0x0018, "8" )
	PORT_DIPSETTING(      0x0028, "16" )
	PORT_DIPSETTING(      0x0008, "24" )
	PORT_DIPSETTING(      0x0030, "32" )
	PORT_DIPSETTING(      0x0010, "48" )
	PORT_DIPSETTING(      0x0020, "64" )
	PORT_DIPSETTING(      0x0000, "80" )
	PORT_DIPNAME( 0x00c0, 0x00c0, "Max. Bet" ) PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(      0x00c0, "32" )
	PORT_DIPSETTING(      0x0040, "64" )
	PORT_DIPSETTING(      0x0080, "80" )
	PORT_DIPSETTING(      0x0000, "120" )
	PORT_DIPNAME( 0x0700, 0x0700, "Coin In" ) PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(      0x0700, "1" )
	PORT_DIPSETTING(      0x0300, "2" )
	PORT_DIPSETTING(      0x0500, "5" )
	PORT_DIPSETTING(      0x0100, "10" )
	PORT_DIPSETTING(      0x0600, "20" )
	PORT_DIPSETTING(      0x0200, "25" )
	PORT_DIPSETTING(      0x0400, "50" )
	PORT_DIPSETTING(      0x0000, "100" )
	PORT_DIPNAME( 0x3800, 0x3800, "Key In" ) PORT_DIPLOCATION("DSW2:4,5,6")
	PORT_DIPSETTING(      0x3800, "1" )
	PORT_DIPSETTING(      0x1800, "5" )
	PORT_DIPSETTING(      0x2800, "10" )
	PORT_DIPSETTING(      0x0800, "30" )
	PORT_DIPSETTING(      0x3000, "50" )
	PORT_DIPSETTING(      0x1000, "100" )
	PORT_DIPSETTING(      0x2000, "200" )
	PORT_DIPSETTING(      0x0000, "500" )
	PORT_DIPNAME( 0xc000, 0xc000, "Pay Out" ) PORT_DIPLOCATION("DSW2:7,8")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0xc000, "10" )
	PORT_DIPSETTING(      0x4000, "50" )
	PORT_DIPSETTING(      0x8000, "100" )

	PORT_MODIFY("DSW34")
	PORT_DIPNAME( 0x0007, 0x0007, "Game Limit" ) PORT_DIPLOCATION("DSW3:1,2,3")
	PORT_DIPSETTING(      0x0007, "5000" )
	PORT_DIPSETTING(      0x0003, "10000" )
	PORT_DIPSETTING(      0x0005, "20000" )
	PORT_DIPSETTING(      0x0001, "30000" )
	PORT_DIPSETTING(      0x0006, "50000" )
	PORT_DIPSETTING(      0x0002, "80000" )
	PORT_DIPSETTING(      0x0004, "100000" )
	PORT_DIPSETTING(      0x0000, "200000" )
	PORT_DIPNAME( 0x0018, 0x0018, "Credit Limit" ) PORT_DIPLOCATION("DSW3:4,5")
	PORT_DIPSETTING(      0x0018, "5000" )
	PORT_DIPSETTING(      0x0008, "10000" )
	PORT_DIPSETTING(      0x0010, "50000" )
	PORT_DIPSETTING(      0x0000, "100000" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Strip Girl Available" ) PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Min. Bet For Bonus" ) PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(      0x0080, "8" )
	PORT_DIPSETTING(      0x0000, "16" )
	PORT_DIPNAME( 0x0300, 0x0300, "Double Up Rate" ) PORT_DIPLOCATION("DSW4:1,2")
	PORT_DIPSETTING(      0x0000, "70%" )
	PORT_DIPSETTING(      0x0200, "75%" )
	PORT_DIPSETTING(      0x0100, "80%" )
	PORT_DIPSETTING(      0x0300, "85%" )
	PORT_DIPNAME( 0x3000, 0x3000, "Pay Out Limit" ) PORT_DIPLOCATION("DSW4:5,6")
	PORT_DIPSETTING(      0x1000, "300" )
	PORT_DIPSETTING(      0x2000, "400" )
	PORT_DIPSETTING(      0x0000, "500" )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPNAME( 0xc000, 0xc000, "Key Out" ) PORT_DIPLOCATION("DSW4:7,8")
	PORT_DIPSETTING(      0xc000, "10" )
	PORT_DIPSETTING(      0x4000, "50" )
	PORT_DIPSETTING(      0x8000, "100" )
	PORT_DIPSETTING(      0x0000, "500" )
INPUT_PORTS_END

static INPUT_PORTS_START( magjack )
	PORT_INCLUDE( frtgenie )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Take / Stop3")

	PORT_MODIFY("DSW12")
	PORT_DIPNAME( 0x0018, 0x0018, "Min. Bet" ) PORT_DIPLOCATION("DSW1:4,5")
	PORT_DIPSETTING(      0x0018, "1" )
	PORT_DIPSETTING(      0x0008, "8" )
	PORT_DIPSETTING(      0x0010, "16" )
	PORT_DIPSETTING(      0x0000, "32" )
	PORT_DIPNAME( 0x0060, 0x0060, "Max. Bet" ) PORT_DIPLOCATION("DSW1:6,7")
	PORT_DIPSETTING(      0x0060, "16" )
	PORT_DIPSETTING(      0x0020, "32" )
	PORT_DIPSETTING(      0x0040, "64" )
	PORT_DIPSETTING(      0x0000, "96" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:8") // no effect in test mode
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0700, 0x0700, "Coin In" ) PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(      0x0700, "1" )
	PORT_DIPSETTING(      0x0300, "2" )
	PORT_DIPSETTING(      0x0500, "5" )
	PORT_DIPSETTING(      0x0100, "10" )
	PORT_DIPSETTING(      0x0600, "20" )
	PORT_DIPSETTING(      0x0200, "25" )
	PORT_DIPSETTING(      0x0400, "50" )
	PORT_DIPSETTING(      0x0000, "100" )
	PORT_DIPNAME( 0x3800, 0x3800, "Key In" ) PORT_DIPLOCATION("DSW2:4,5,6")
	PORT_DIPSETTING(      0x3800, "1" )
	PORT_DIPSETTING(      0x1800, "5" )
	PORT_DIPSETTING(      0x2800, "10" )
	PORT_DIPSETTING(      0x0800, "30" )
	PORT_DIPSETTING(      0x3000, "50" )
	PORT_DIPSETTING(      0x1000, "100" )
	PORT_DIPSETTING(      0x2000, "200" )
	PORT_DIPSETTING(      0x0000, "500" )

	PORT_MODIFY("DSW34")
	PORT_DIPNAME( 0x0008, 0x0008, "Display Rate Table" ) PORT_DIPLOCATION("DSW3:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0300, 0x0300, "Double Up Rate" ) PORT_DIPLOCATION("DSW4:1,2")
	PORT_DIPSETTING(      0x0300, "98%" )
	PORT_DIPSETTING(      0x0100, "96%" )
	PORT_DIPSETTING(      0x0200, "94%" )
	PORT_DIPSETTING(      0x0000, "92%" )
	PORT_DIPNAME( 0x1000, 0x1000, "Strip Girl Available" ) PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xe000, 0xe000, "Key Out" ) PORT_DIPLOCATION("DSW4:6,7,8")
	PORT_DIPSETTING(      0xe000, "1" )
	PORT_DIPSETTING(      0x6000, "2" )
	PORT_DIPSETTING(      0xa000, "5" )
	PORT_DIPSETTING(      0x2000, "10" )
	PORT_DIPSETTING(      0xc000, "20" )
	PORT_DIPSETTING(      0x4000, "40" )
	PORT_DIPSETTING(      0x8000, "100" )
	PORT_DIPSETTING(      0x0000, "500" )
INPUT_PORTS_END

static INPUT_PORTS_START( magjacka )
	PORT_INCLUDE( magjack )

	PORT_MODIFY("DSW34")
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( magjackb )
	PORT_INCLUDE( magjacka )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in input test
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect in input test
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("Hopper Sw")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
INPUT_PORTS_END

static INPUT_PORTS_START( magjackc )
	PORT_INCLUDE( magjackb )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Bet / All Stop")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SLOT_STOP3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SLOT_STOP2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SLOT_STOP1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Small")

	PORT_MODIFY("DSW12")
	PORT_DIPNAME( 0x0080, 0x0080, "Hold Function" ) PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )

	PORT_MODIFY("DSW34")
	PORT_DIPNAME( 0x0080, 0x0080, "Fever Min. Bet" ) PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(      0x0080, "8" )
	PORT_DIPSETTING(      0x0000, "16" )
	PORT_DIPNAME( 0x1000, 0x1000, "Take Score Speed" ) PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(      0x1000, "Slow" )
	PORT_DIPSETTING(      0x0000, "Fast" )
	PORT_DIPNAME( 0x2000, 0x2000, "Strip Girl Available" ) PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xc000, 0xc000, "Key Out" ) PORT_DIPLOCATION("DSW4:7,8")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0xc000, "10" )
	PORT_DIPSETTING(      0x4000, "20" )
	PORT_DIPSETTING(      0x8000, "50" )
INPUT_PORTS_END

static INPUT_PORTS_START( magjackp )
	PORT_INCLUDE( magjack )

	PORT_MODIFY("DSW12")
	PORT_DIPNAME( 0x0080, 0x0080, "Win Points to Score" ) PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_DIPNAME( 0x3800, 0x3800, "Key In" ) PORT_DIPLOCATION("DSW2:4,5,6")
	PORT_DIPSETTING(      0x3800, "1" )
	PORT_DIPSETTING(      0x1800, "4" )
	PORT_DIPSETTING(      0x2800, "5" )
	PORT_DIPSETTING(      0x0800, "15" )
	PORT_DIPSETTING(      0x3000, "25" )
	PORT_DIPSETTING(      0x1000, "75" )
	PORT_DIPSETTING(      0x2000, "100" )
	PORT_DIPSETTING(      0x0000, "500" )

	PORT_MODIFY("DSW34")
	PORT_DIPNAME( 0x0080, 0x0080, "Fever Min. Bet" ) PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(      0x0080, "8" )
	PORT_DIPSETTING(      0x0000, "16" )
INPUT_PORTS_END


static const gfx_layout jungleyo16_layout =
{
	8,32,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	{ STEP32(0,8*8) },
	8*64*4
};

static GFXDECODE_START( gfx_jungleyo )
	GFXDECODE_ENTRY( "reelgfx", 0, jungleyo16_layout,   0x4000, 0x40  )
	GFXDECODE_ENTRY( "gfx2",    0, gfx_8x8x8_raw,       0x2000, 0x20  )
	GFXDECODE_ENTRY( "gfx3",    0, gfx_8x8x8_raw,       0x0000, 0x20  )
GFXDECODE_END


void jungleyo_state::jungleyo(machine_config &config)
{
	M68000(config, m_maincpu, 24_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &jungleyo_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(jungleyo_state::irq2_line_hold));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_jungleyo);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 2*8, 32*8-1);
	screen.set_screen_update(FUNC(jungleyo_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(0x8000);

	SPEAKER(config, "speaker", 2).front();

	okim6295_device &oki(OKIM6295(config, "oki", 24_MHz_XTAL / 20, okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 not verified
	oki.add_route(ALL_OUTPUTS, "speaker", 0.47, 0);
	oki.add_route(ALL_OUTPUTS, "speaker", 0.47, 1);
}


// version 3.02 built on 2001/02/09, there's copyright both for Yonshi and Global in strings
ROM_START( jungleyo ) // MADE IN TAIWAN YONSHI PCB NO-006F
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 ) // 68000 code, encrypted
	ROM_LOAD16_BYTE( "jungle_=record=_rom3_vi3.02.u15", 0x00000, 0x20000, CRC(7c9f431e) SHA1(fb3f90c4fe59c938f36b30c5fa3af227031e7d7a) )
	ROM_LOAD16_BYTE( "jungle_=record=_rom2_vi3.02.u14", 0x00001, 0x20000, CRC(f6a71260) SHA1(8e48cbb9d701ad968540244396820359afe97c28) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "jungle_rom1.u99", 0x00000, 0x40000, CRC(05ef5b85) SHA1(ca7584646271c6adc7880eca5cf43a412340c522) )

	ROM_REGION( 0x80000, "reelgfx", 0 )
	ROM_LOAD( "jungle_rom4.u58", 0x000000, 0x80000, CRC(2f37da94) SHA1(6479e3bcff665316903964286d72df9822c05485) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "jungle_rom5.u59", 0x000000, 0x80000, CRC(0ccd9b94) SHA1(f209c7e15967be2e43be018aca89edd0c311503e) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "jungle_rom6.u60", 0x000000, 0x80000, CRC(caab8eb2) SHA1(472ca9f396d7c01a1bd03485581cfae677a3b365) )

	ROM_REGION( 0x157, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "atf20v8b.u37", 0x000, 0x157, NO_DUMP )
ROM_END

// first half is version 1-1-03, with copyright both for Yonshi and Global in strings, second half is version VA1.0 2001/08/15 with copyright both for Yonshi and Global in strings
ROM_START( frtgenie ) // MADE IN TAIWAN YONSHI PCB NO-006E
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code, encrypted
	ROM_LOAD16_BYTE( "fruit_genie_rom3_va1_1.03.u15", 0x00000, 0x40000, CRC(747099c3) SHA1(99f4aa6814ed2868d9758ad94b4497fd4c3142dc) )
	ROM_LOAD16_BYTE( "fruit_genie_rom2_va1_1.03.u14", 0x00001, 0x40000, CRC(627c9dfd) SHA1(c4e393c61911a3d646b53fc2742e8b7495509567) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "fruit_genie_rom_1.u99", 0x00000, 0x40000, CRC(28b0c8fb) SHA1(5cdf59dcbed7da9b882c7dcf27020c1c37dd22cc) )

	ROM_REGION( 0x80000, "reelgfx", 0 )
	ROM_LOAD( "fruit_genie_rom_4.u58", 0x000000, 0x80000, CRC(b3b467b6) SHA1(f1a64af7a8fe22c7ef76617aba359df11e4af737) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "fruit_genie_rom_5.u59", 0x000000, 0x80000, CRC(5c4497ed) SHA1(ed326867edb3a0e841a6d1ab9d6f238e9f7281b7) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "fruit_genie_rom_6.u60", 0x000000, 0x80000, CRC(2f66583e) SHA1(08a8a4266c7e118183784bfdca796da803d3a2dd) )

	ROM_REGION( 0x157, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "atf20v8b.u37", 0x000, 0x157, NO_DUMP )
ROM_END

ROM_START( frtgeniea ) // MADE IN TAIWAN YONSHI PCB NO-006E
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code, encrypted
	ROM_LOAD16_BYTE( "fruit_genie_rom3_va1_1.03.u15", 0x00000, 0x40000, CRC(c9141977) SHA1(6b57631802eab7f4a0d99074844407d009eff07b) ) // SLDH, 27C020
	ROM_LOAD16_BYTE( "fruit_genie_rom2_va1_1.03.u14", 0x00001, 0x40000, CRC(cacd2806) SHA1(af572697b434630740b0edbe901c7b704f2be908) ) // SLDH, 27C020

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "fruit_genie_rom_1.u99", 0x00000, 0x40000, CRC(28b0c8fb) SHA1(5cdf59dcbed7da9b882c7dcf27020c1c37dd22cc) ) // 27C020

	ROM_REGION( 0x80000, "reelgfx", 0 )
	ROM_LOAD( "fruit_genie_rom_4.u58", 0x00000, 0x80000, CRC(74e8235a) SHA1(f16391f824f7ae7ce89e917d94cb784b5ca4a9e1) ) // SLDH, 27C040

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "fruit_genie_rom_5.u59", 0x00000, 0x80000, CRC(00d73415) SHA1(26e65e0e2c91bc71f39cefd065e234e49f2a1d81) ) // SLDH, 27C040

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "fruit_genie_rom_6.u60", 0x00000, 0x80000, CRC(a57f5f4e) SHA1(b7ee54d250a127c211cd5ad11ddb38ae1e0119d5) ) // SLDH, 27C040

	ROM_REGION( 0x157, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "atf20v8b.u37", 0x000, 0x157, NO_DUMP )
ROM_END

ROM_START( frtgenieb ) // MADE IN TAIWAN YONSHI PCB NO-006G
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code, encrypted
	ROM_LOAD16_BYTE( "fruit_genie_rom3_va1_1.03.u15", 0x00000, 0x40000, CRC(fae885b8) SHA1(ac923469cc0c7866be490a43344fe1677d64f617) ) // SLDH, 27C020
	ROM_LOAD16_BYTE( "fruit_genie_rom2_va1_1.03.u14", 0x00001, 0x40000, CRC(4b5e1cbc) SHA1(4e2af889c669d80c6cdba01948858b9e8d8fab95) ) // SLDH, 27C020

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "fruit_genie_rom_1.u99", 0x00000, 0x40000, CRC(28b0c8fb) SHA1(5cdf59dcbed7da9b882c7dcf27020c1c37dd22cc) ) // 27C020

	ROM_REGION( 0x80000, "reelgfx", 0 )
	ROM_LOAD( "fruit_genie_rom_4.u58", 0x00000, 0x80000, CRC(16aaba0a) SHA1(0bad575724d44f5ad62412ec8391fcd42bd0628e) ) // SLDH, 27C040

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "fruit_genie_rom_5.u59", 0x00000, 0x80000, CRC(fe3eece5) SHA1(0fc785a04ee42c5bce4f0e72e608e7ba5aa28412) ) // SLDH, 27C040

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "fruit_genie_rom_6.u60", 0x00000, 0x80000, CRC(8d364563) SHA1(fd1257680eb610885b65f41a8f86e32b5635acfc) ) // SLDH, 27C040

	ROM_REGION( 0x157, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "atf20v8b.u37", 0x000, 0x157, NO_DUMP )
ROM_END

ROM_START( frtgeniec ) // MADE IN TAIWAN YONSHI PCB NO-006G
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code, encrypted
	ROM_LOAD16_BYTE( "fruit_genie_rom3_va1_1.03.u15", 0x00000, 0x40000, CRC(ec6721b5) SHA1(f9621a66964ef5a312c185540f3c4fc52f76c7ef) ) // SLDH, 27C020
	ROM_LOAD16_BYTE( "fruit_genie_rom2_va1_1.03.u14", 0x00001, 0x40000, CRC(3112f27d) SHA1(7d7cc15552e9c453a2f7f7403163123cd392c18d) ) // SLDH, 27C020

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "fruit_genie_rom_1.u99", 0x00000, 0x40000, CRC(28b0c8fb) SHA1(5cdf59dcbed7da9b882c7dcf27020c1c37dd22cc) ) // 27C020

	ROM_REGION( 0x80000, "reelgfx", 0 )
	ROM_LOAD( "fruit_genie_rom_4.u58", 0x00000, 0x80000, CRC(16aaba0a) SHA1(0bad575724d44f5ad62412ec8391fcd42bd0628e) ) // SLDH, 27C040

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "fruit_genie_rom_5.u59", 0x00000, 0x80000, CRC(fe3eece5) SHA1(0fc785a04ee42c5bce4f0e72e608e7ba5aa28412) ) // SLDH, 27C040

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "fruit_genie_rom_6.u60", 0x00000, 0x80000, CRC(b7056d04) SHA1(87b1ffa3c722a0f0eb7e249ba07f540c314d2d79) ) // SLDH, 27C040

	ROM_REGION( 0x157, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "atf20v8b.u37", 0x000, 0x157, NO_DUMP )
ROM_END

ROM_START( frtgenied ) // MADE IN TAIWAN YONSHI PCB NO-006E
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code, encrypted
	ROM_LOAD16_BYTE( "fruit_genie_rom3_vt_2.11.u15", 0x00000, 0x20000, CRC(37bac681) SHA1(39b0b8596e8acc988e4d8c7b0249e9d666b3fc1e) ) // M27C1001
	ROM_LOAD16_BYTE( "fruit_genie_rom2_vt_2.11.u14", 0x00001, 0x20000, CRC(0918cefc) SHA1(4a1bc853f7deb71f504780b84fa7f8c5c1d3330e) ) // M27C1001

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "fruit_genie_rom_1.u99", 0x00000, 0x40000, CRC(28b0c8fb) SHA1(5cdf59dcbed7da9b882c7dcf27020c1c37dd22cc) ) // M27C2001

	ROM_REGION( 0x80000, "reelgfx", 0 )
	ROM_LOAD( "fruit_genie_rom_4.u58", 0x00000, 0x80000, CRC(b3b467b6) SHA1(f1a64af7a8fe22c7ef76617aba359df11e4af737) ) // M27C4001

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "fruit_genie_rom_5.u59", 0x00000, 0x80000, CRC(a7926b81) SHA1(650d85a2dd6850234e0fb68c19470f34aed76577) ) // SLDH, 27C4001

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "fruit_genie_rom_6.u60", 0x00000, 0x80000, CRC(91faa324) SHA1(cda033e948d6f42abb36497619164b26b2201cad) ) // SLDH, 27C4001

	ROM_REGION( 0x157, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "atf20v8b.u37", 0x000, 0x157, NO_DUMP )
ROM_END

ROM_START( magjack ) // MADE IN TAIWAN PCB NO-006A
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code, encrypted
	ROM_LOAD16_BYTE( "3.u15", 0x00000, 0x20000, CRC(7d0855d0) SHA1(489e54f529c648da2333a3a811ced20f0d578029) ) // 27C010
	ROM_LOAD16_BYTE( "2.u14", 0x00001, 0x20000, CRC(5564101a) SHA1(30fc23acf4387221cab705b166d551883c7aaa29) ) // 27C010

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "1.u99", 0x00000, 0x40000, CRC(0bce2433) SHA1(2c9e9cb3ab9076e1a2eb0a58ad0079b86bf4e922) ) // 27C020

	ROM_REGION( 0x80000, "reelgfx", 0 )
	ROM_LOAD( "4.u58", 0x00000, 0x80000, CRC(9b630db6) SHA1(00e04e4b4207ba44617851017c1505eacfcc5375) ) // 27C040

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "5.u59", 0x00000, 0x80000, CRC(5991a0c5) SHA1(29848eae911c47ff911a49f0b3552b0dc958a6c5) ) // 27C040

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "6.u60", 0x00000, 0x80000, CRC(94d25396) SHA1(5bfe7ddf4a5b6a541dac10c50529e025140fd8f2) ) // 27C040

	ROM_REGION( 0x157, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "atf20v8b.u37", 0x000, 0x157, NO_DUMP )
ROM_END

ROM_START( magjacka ) // MADE IN TAIWAN PCB NO-006E
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code, encrypted
	ROM_LOAD16_BYTE( "3.u15", 0x00000, 0x20000, CRC(7ef54fad) SHA1(d4839912a149f42b410a961f356c648521c3f42a) ) // SLDH, 27C010
	ROM_LOAD16_BYTE( "2.u14", 0x00001, 0x20000, CRC(8ad958f3) SHA1(92ee2995189fc423d5f63c26db5e04ac4f09fcb1) ) // SLDH, 27C010

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "1.u99", 0x00000, 0x40000, CRC(0bce2433) SHA1(2c9e9cb3ab9076e1a2eb0a58ad0079b86bf4e922) ) // 27C020

	ROM_REGION( 0x80000, "reelgfx", 0 )
	ROM_LOAD( "4.u58", 0x00000, 0x80000, CRC(9b630db6) SHA1(00e04e4b4207ba44617851017c1505eacfcc5375) ) // 27C040

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "5.u59", 0x00000, 0x80000, CRC(5991a0c5) SHA1(29848eae911c47ff911a49f0b3552b0dc958a6c5) ) // 27C040

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "6.u60", 0x00000, 0x80000, CRC(94d25396) SHA1(5bfe7ddf4a5b6a541dac10c50529e025140fd8f2) ) // 27C040

	ROM_REGION( 0x157, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "atf20v8b.u37", 0x000, 0x157, NO_DUMP )
ROM_END

ROM_START( magjackb ) // MADE IN TAIWAN PCB NO-006A
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code, encrypted
	ROM_LOAD16_BYTE( "3.u15", 0x00000, 0x20000, CRC(7033037a) SHA1(78f3e699094b81630afee18c897dc68ee163634f) ) // SLDH, 27C010
	ROM_LOAD16_BYTE( "2.u14", 0x00001, 0x20000, CRC(926e3a96) SHA1(92bf53bf48307f5656662f2242a99a0af0eeb62d) ) // SLDH, 27C010

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "1.u99", 0x00000, 0x40000, CRC(0bce2433) SHA1(2c9e9cb3ab9076e1a2eb0a58ad0079b86bf4e922) ) // 27C020

	ROM_REGION( 0x80000, "reelgfx", 0 )
	ROM_LOAD( "4.u58", 0x00000, 0x80000, CRC(9b630db6) SHA1(00e04e4b4207ba44617851017c1505eacfcc5375) ) // 27C040

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "5.u59", 0x00000, 0x80000, CRC(5991a0c5) SHA1(29848eae911c47ff911a49f0b3552b0dc958a6c5) ) // 27C040

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "6.u60", 0x00000, 0x80000, CRC(94d25396) SHA1(5bfe7ddf4a5b6a541dac10c50529e025140fd8f2) ) // 27C040

	ROM_REGION( 0x157, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "atf20v8b.u37", 0x000, 0x157, NO_DUMP )
ROM_END

ROM_START( magjackc ) // MADE IN TAIWAN PCB NO-006A
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code, encrypted
	ROM_LOAD16_BYTE( "3.u15", 0x00000, 0x20000, CRC(86c3adc8) SHA1(7e3ee5847f2c170c168b1d9341093e97be855526) ) // SLDH, 27C010
	ROM_LOAD16_BYTE( "2.u14", 0x00001, 0x20000, CRC(e2fa5dbf) SHA1(3d68f57342fa1f70b7f0d956e8d06cb39e84d9f4) ) // SLDH, 27C010

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "1.u99", 0x00000, 0x40000, CRC(bee977a7) SHA1(f924c53781e6a9b2796c23b5d9e63a62e0b75b9a) ) // SLDH, 27C020

	ROM_REGION( 0x80000, "reelgfx", 0 )
	ROM_LOAD( "4.u58", 0x00000, 0x80000, CRC(9b630db6) SHA1(00e04e4b4207ba44617851017c1505eacfcc5375) ) // 27C040

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "5.u59", 0x00000, 0x80000, CRC(6a2c11cc) SHA1(0d3e32f2279a60779b228e5ee485dcdb9c27f30a) ) // SLDH, 27C040

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "6.u60", 0x00000, 0x80000, CRC(e16a09be) SHA1(cc4124841b0fa12776bd20eb9c86e809667f3f49) ) // SLDH, 27C040

	ROM_REGION( 0x157, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "atf20v8b.u37", 0x000, 0x157, NO_DUMP )
ROM_END

ROM_START( magjackp )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code, encrypted
	ROM_LOAD16_BYTE( "3.u15", 0x00000, 0x20000, CRC(c59d43c2) SHA1(a58f1a7b618956d93c27e321e64b86b12f15e3d8) ) // 27C010
	ROM_LOAD16_BYTE( "2.u14", 0x00001, 0x20000, CRC(35321d52) SHA1(31049d154a68c4f65d748cc24c8b680fefe36b89) ) // 27C010

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "1.u99", 0x00000, 0x40000, CRC(0bce2433) SHA1(2c9e9cb3ab9076e1a2eb0a58ad0079b86bf4e922) ) // 27C020, same as the magjack sets

	ROM_REGION( 0x80000, "reelgfx", 0 )
	ROM_LOAD( "4.u58", 0x00000, 0x80000, CRC(59aa4f5f) SHA1(a7d713009efe2077ff26ba3c9bbf960386a43d40) ) // 27C040

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "5.u59", 0x00000, 0x80000, CRC(70784939) SHA1(189c7328ad76496acbbdc02edc2e949be8cff077) ) // 27C040

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "6.u60", 0x00000, 0x80000, CRC(cc397dab) SHA1(564e442364059411038819582b5215ec83752f9a) ) // 27C040

	ROM_REGION( 0x157, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "atf20v8b.u37", 0x000, 0x157, NO_DUMP )
ROM_END

ROM_START( magjackpa )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code, encrypted
	ROM_LOAD16_BYTE( "3.u15", 0x00000, 0x20000, CRC(f7c965e6) SHA1(07139219df87f174a87cd3e524c575fd98265aa8) ) // SLDH, 27C010
	ROM_LOAD16_BYTE( "2.u14", 0x00001, 0x20000, CRC(7bc8c1fa) SHA1(aed37b1585ac05777495fe84ac0d76c6d1e7e377) ) // SLDH, 27C010

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "1.u99", 0x00000, 0x40000, CRC(0bce2433) SHA1(2c9e9cb3ab9076e1a2eb0a58ad0079b86bf4e922) ) // 27C020, same as the magjack sets

	ROM_REGION( 0x80000, "reelgfx", 0 )
	ROM_LOAD( "4.u58", 0x00000, 0x80000, CRC(59aa4f5f) SHA1(a7d713009efe2077ff26ba3c9bbf960386a43d40) ) // 27C040

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "5.u59", 0x00000, 0x80000, CRC(70784939) SHA1(189c7328ad76496acbbdc02edc2e949be8cff077) ) // 27C040

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "6.u60", 0x00000, 0x80000, CRC(cc397dab) SHA1(564e442364059411038819582b5215ec83752f9a) ) // 27C040

	ROM_REGION( 0x157, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "atf20v8b.u37", 0x000, 0x157, NO_DUMP )
ROM_END


void jungleyo_state::init_jungleyo()
{
	u16 *src = (u16 *)memregion("maincpu")->base();

	for (int i = 0x00000; i < 0x10000 / 2; i++)
		src[i] = bitswap<16>(src[i] ^ 0x00ff, 8, 10, 15, 11, 9, 14, 12, 13, 6, 4, 2, 7, 3, 0, 1, 5);

	for (int i = 0x10000 / 2; i < 0x20000 / 2; i++)
		src[i] = bitswap<16>(src[i] ^ 0xff00, 11, 13, 10, 8, 15, 9, 14, 12, 0, 7, 4, 1, 5, 3, 6, 2);

	for (int i = 0x20000 / 2; i < 0x30000 / 2; i++)
		src[i] = bitswap<16>(src[i] ^ 0x00ff, 14, 8, 12, 15, 10, 13, 11, 9, 3, 5, 2, 6, 4, 0, 1, 7);

	for (int i = 0x30000 / 2; i < 0x40000 / 2; i++)
		src[i] = bitswap<16>(src[i] ^ 0xffff, 13, 15, 8, 9, 12, 11, 10, 14, 1, 3, 7, 5, 2, 6, 0, 4);

	// TODO: Stack Pointer/Initial PC settings don't seem to decrypt correctly
	// hack these until better understood (still wrong values)
	src[0x000 / 2] = 0x0000;
	src[0x002 / 2] = 0x0000;
	src[0x004 / 2] = 0x0000;
	src[0x006 / 2] = 0x01f8; // reset opcode
}

void jungleyo_state::init_frtgenie()
{
	u16 *src = (u16 *)memregion("maincpu")->base();

	for (int i = 0x00000; i < 0x10000 / 2; i++)
		src[i] = bitswap<16>(src[i] ^ 0x00ff, 11, 12, 14, 9, 10, 13, 8, 15, 5, 0, 2, 3, 6, 1, 4, 7);

	for (int i = 0x10000 / 2; i < 0x20000 / 2; i++)
		src[i] = bitswap<16>(src[i] ^ 0xff00, 14, 11, 8, 13, 15, 9, 12, 10, 1, 5, 3, 0, 7, 2, 6, 4);

	for (int i = 0x20000 / 2; i < 0x30000 / 2; i++)
		src[i] = bitswap<16>(src[i] ^ 0x00ff, 8, 14, 12, 11, 9, 15, 10, 13, 7, 4, 1, 5, 3, 6, 0, 2);

	for (int i = 0x30000 / 2; i < 0x40000 / 2; i++)
		src[i] = bitswap<16>(src[i] ^ 0xffff, 15, 9, 10, 12, 8, 11, 13, 14, 2, 6, 4, 5, 0, 7, 3, 1);

	for (int i = 0x40000 / 2; i < 0x80000 / 2; i++) // the second half of the ROM seems to have the same bitswap
		src[i] = bitswap<16>(src[i] ^ 0x0000, 10, 13, 15, 8, 12, 14, 11, 9, 0, 7, 6, 4, 1, 5, 2, 3);

	// TODO: Stack Pointer/Initial PC settings don't seem to decrypt correctly
	// hack these until better understood (still wrong values)
	src[0x000 / 2] = 0x0000;
	src[0x002 / 2] = 0x0000;
	src[0x004 / 2] = 0x0000;
	src[0x006 / 2] = 0x01f8; // reset opcode
}

template <uint16_t Reset_addr>
void jungleyo_state::init_magjack()
{
	u16 *src = (u16 *)memregion("maincpu")->base();

	for (int i = 0x00000; i < 0x10000 / 2; i++)
		src[i] = bitswap<16>(src[i] ^ 0xffff, 15, 12, 9, 13, 14, 11, 8, 10, 0, 4, 7, 1, 6, 5, 2, 3);

	for (int i = 0x10000 / 2; i < 0x20000 / 2; i++)
		src[i] = bitswap<16>(src[i] ^ 0x00ff, 9, 11, 14, 15, 8, 13, 10, 12, 7, 6, 1, 3, 2, 4, 0, 5);

	for (int i = 0x20000 / 2; i < 0x30000 / 2; i++)
		src[i] = bitswap<16>(src[i] ^ 0xff00, 12, 15, 8, 10, 13, 9, 14, 11, 2, 7, 4, 0, 1, 6, 5, 3);

	for (int i = 0x30000 / 2; i < 0x40000 / 2; i++)
		src[i] = bitswap<16>(src[i] ^ 0x00ff, 14, 8, 13, 11, 9, 15, 12, 10, 5, 0, 2, 6, 4, 1, 3, 7);

	// TODO: Stack Pointer/Initial PC settings don't seem to decrypt correctly
	// hack these until better understood (still wrong values)
	src[0x000 / 2] = 0x0000;
	src[0x002 / 2] = 0x0000;
	src[0x004 / 2] = 0x0000;
	src[0x006 / 2] = Reset_addr; // reset opcode
}

} // anonymous namespace


GAME( 2001, jungleyo,  0,        jungleyo, jungleyo,  jungleyo_state, init_jungleyo,       ROT0, "Yonshi",       "Jungle (Italy VI3.02)",               MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 2003, frtgenie,  0,        jungleyo, frtgenie,  jungleyo_state, init_frtgenie,       ROT0, "Global",       "Fruit Genie (Version 1-1-03, set 1)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2003, frtgeniea, frtgenie, jungleyo, frtgeniea, jungleyo_state, init_frtgenie,       ROT0, "Global",       "Fruit Genie (Version 1-1-03, set 2)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2003, frtgenieb, frtgenie, jungleyo, frtgenieb, jungleyo_state, init_frtgenie,       ROT0, "Global",       "Fruit Genie (Version 1-1-03, set 3)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2003, frtgeniec, frtgenie, jungleyo, frtgeniec, jungleyo_state, init_frtgenie,       ROT0, "Global",       "Fruit Genie (Version 1-1-03, set 4)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2002, frtgenied, frtgenie, jungleyo, frtgenied, jungleyo_state, init_jungleyo,       ROT0, "Winnin World", "Fruit Genie (VT 2.11)",               MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 2000, magjack,   0,        jungleyo, magjack,   jungleyo_state, init_magjack<0x260>, ROT0, "Global",       "Magical Jack (VA 4.00)",              MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 2000/09/28
GAME( 2000, magjacka,  magjack,  jungleyo, magjacka,  jungleyo_state, init_magjack<0x268>, ROT0, "Global",       "Magical Jack (VA 3.30)",              MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 2000/08/04
GAME( 2000, magjackb,  magjack,  jungleyo, magjackb,  jungleyo_state, init_magjack<0x268>, ROT0, "Global",       "Magical Jack (VA 3.11)",              MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 2000/06/29
GAME( 1999, magjackc,  magjack,  jungleyo, magjackc,  jungleyo_state, init_magjack<0x268>, ROT0, "Global",       "Magical Jack (VA 2.0)",               MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 1999/10/28

GAME( 2001, magjackp,  0,        jungleyo, magjackp,  jungleyo_state, init_magjack<0x1fe>, ROT0, "Global",       "Magical Jack Plus (VA 6.03)",         MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 2001/07/23
GAME( 2000, magjackpa, magjackp, jungleyo, magjackp,  jungleyo_state, init_magjack<0x1fe>, ROT0, "Global",       "Magical Jack Plus (VA 6.01)",         MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 2001/02/14

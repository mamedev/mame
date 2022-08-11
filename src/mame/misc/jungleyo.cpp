// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/******************************************************************************

    Jungle (c) 2001 Yonshi

    TODO:
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

	void init_jungleyo();

protected:
	virtual void video_start() override;

private:
	void output_w(uint16_t data);

	// video-related
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	template <int Layer> TILE_GET_INFO_MEMBER(get_reel_tile_info);
	void bg_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void fg_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	template <int Layer> void reel_vram_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void main_map(address_map &map);

	u8 palette_ram_r(offs_t offset);
	void palette_ram_w(offs_t offset, u8 data);
	void layer_enable_w(u8 data);
	void video_priority_w(u8 data);

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint16_t> m_bg_videoram;
	required_shared_ptr<uint16_t> m_fg_videoram;
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
}

uint32_t jungleyo_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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

void jungleyo_state::output_w(uint16_t data)
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
	map(0x000000, 0x03ffff).rom().region("maincpu", 0);
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

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	okim6295_device &oki(OKIM6295(config, "oki", 24_MHz_XTAL / 20, okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 not verified
	oki.add_route(ALL_OUTPUTS, "lspeaker", 0.47);
	oki.add_route(ALL_OUTPUTS, "rspeaker", 0.47);
}


ROM_START( jungleyo )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code, encrypted
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
ROM_END

void jungleyo_state::init_jungleyo()
{
	uint16_t *src = (uint16_t *)memregion("maincpu")->base();

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

} // Anonymous namespace


// version 3.02 built on 2001/02/09, there's copyright both for Yonshi and Global in strings
GAME( 2001, jungleyo, 0, jungleyo, jungleyo, jungleyo_state, init_jungleyo, ROT0, "Yonshi", "Jungle (Italy VI3.02)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND )

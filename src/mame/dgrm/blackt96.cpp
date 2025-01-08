// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

    Black Touch '96


Black Touch 96
D.G.R.M. of Korea, 1996

This game is a beat'em-up like Double Dragon

PCB Layout
----------

D.G.R.M. NO 1947
|---------------------------------------------|
| M6295    1     8MHz                         |
| M6295    2              2018  2018          |
|         16C57           2018  2018          |
|HA13001                  2018  2018          |
|                         2018  2018          |
|                       PAL     PAL           |
|    6116                 5      6            |
|J   6116                 7      8            |
|A                                            |
|M                                            |
|M                                            |
|A                                            |
|                               9  10         |
|    DSW1           24MHz               PAL   |
|    DSW2                                     |
|   PAL PAL           ACTEL     6116    11    |
|   62256    62256    A1020B            12    |
|   3        4        PL84C     6264    13    |
|                               6264    14    |
|18MHz 68000                    6264          |
|---------------------------------------------|
Notes:
      68000 clock 9.000MHz [18/2]
      M6295 clocks 1.000MHz [8/8] pin 7 high
      PIC16C57 clock 4.000MHz [8/2]


2008-07
Added Dip Locations based on Service Mode

The hardware is cloned from 'snk68' with some extra capabilities
the drivers can probably be merged.


Bugs (all of these looks BTANBs):

- Sometimes if you attack an enemy when you're at the top of the screen they'll
  end up landing in an even higher position, and appear over the backgrounds!

- The timer doesn't work
  Each frame calls:
 00E8CC: 0C39 0000 00C0 16AF        cmpi.b  #$0, $c016af.l
 00E8D4: 6600 0026                  bne     $e8fc
 00E8D8: 0C39 000F 00C0 002E        cmpi.b  #$f, $c0002e.l
 00E8E0: 6700 001A                  beq     $e8fc
 00E8E4: 0C39 000F 00C0 008E        cmpi.b  #$f, $c0008e.l
 00E8EC: 6700 000E                  beq     $e8fc
 00E8F0: 33FC 2000 00C0 1982        move.w  #$2000, $c01982.l   // timer inited again???
 00E8F8: 6100 0118                  bsr     $ea12
 00E8FC: 4E75                       rts
---
 00EA12: 48A7 FCF0                  movem.w D0-D5/A0-A3, -(A7)
 00EA16: 3039 00C0 1982             move.w  $c01982.l, D0
// then calls setup data and drawing for the timer which is always 20 for whatever reason.

- There are some unmapped writes scattered across different areas (text ram, spriteram, 0xe0000 area etc.)

- flip screen doesn't work properly,
  game code explicitly sets flip screen off & the correlated work RAM buffer at 0xee2 no matter the dip setting

- some service mode items are buggy or not functioning properly (font, color, inputs, sound, 2nd item);

*/

#include "emu.h"
#include "snk68_spr.h"

#include "cpu/pic16c5x/pic16c5x.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class blackt96_state : public driver_device
{
public:
	blackt96_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_tilemapram(*this, "tilemapram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sprites(*this, "sprites"),
		m_oki(*this, "oki%u", 1U),
		m_oki1bank(*this, "oki1bank")
	{ }

	void blackt96(machine_config &config);

protected:
	// overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// driver variables
	uint8_t m_soundcmd;
	uint8_t m_soundcmd_ready;
	uint8_t m_port_c_data;
	uint8_t m_port_b_latch;
	uint8_t m_oki_selected;
	uint8_t m_txt_bank;

	// video
	tilemap_t  *m_tx_tilemap;

	// devices
	required_shared_ptr<uint16_t> m_tilemapram;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<snk68_spr_device> m_sprites;
	required_device_array<okim6295_device, 2> m_oki;
	required_memory_bank m_oki1bank;

	// read/write handlers
	void output_w(uint8_t data);
	void sound_cmd_w(uint8_t data);
	void tx_vram_w(offs_t offset, uint16_t data);

	void soundio_port_a_w(uint8_t data);
	uint8_t soundio_port_b_r();
	void soundio_port_b_w(uint8_t data);
	uint8_t soundio_port_c_r();
	void soundio_port_c_w(uint8_t data);

	uint16_t random_r() // todo, get rid of this once we work out where reads are from
	{
		return machine().rand();
	}

	// video
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void tile_callback(int &tile, int& fx, int& fy, int& region);

	void blackt96_map(address_map &map) ATTR_COLD;
	void oki1_map(address_map &map) ATTR_COLD;
};

TILE_GET_INFO_MEMBER(blackt96_state::get_tx_tile_info)
{
	uint16_t tile = m_tilemapram[tile_index*2] & 0xff;
	// following is guessed, game just uses either color 0 or 1 anyway (which is identical palette wise)
	uint8_t color = m_tilemapram[tile_index*2+1] & 0x0f;
	tile += m_txt_bank * 0x100;

	tileinfo.set(2,
			tile,
			color,
			0);
}


void blackt96_state::video_start()
{
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(blackt96_state::get_tx_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 32, 32);

	m_tx_tilemap->set_transparent_pen(0);
}


uint32_t blackt96_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	m_sprites->draw_sprites_all(bitmap, cliprect);
	m_tx_tilemap->draw(screen,bitmap, cliprect, 0, 0);

	return 0;
}


void blackt96_state::sound_cmd_w(uint8_t data)
{
	//logerror("sound_cmd_w %02x\n", data);
	m_soundcmd = data;
	m_soundcmd_ready = 1;
}

void blackt96_state::machine_start()
{
	m_oki1bank->configure_entries(0, 8, memregion("oki1")->base(), 0x10000);
	m_oki1bank->set_entry(0);

	save_item(NAME(m_soundcmd));
	save_item(NAME(m_soundcmd_ready));
	save_item(NAME(m_port_c_data));
	save_item(NAME(m_port_b_latch));
	save_item(NAME(m_oki_selected));
	save_item(NAME(m_txt_bank));
}



void blackt96_state::machine_reset()
{
	m_soundcmd = 0;
	m_soundcmd_ready = 0;
	m_port_c_data = 0;
	m_port_b_latch = 0;
	m_oki_selected = 0;
	m_txt_bank = 0;
}

void blackt96_state::output_w(uint8_t data)
{
	// -bbb 8-21
	// 1 - coin counter 1
	// 2 - coin counter 2
	// 8 - flip screen
	// b = text tile bank

	m_txt_bank = (data & 0x70)>>4;
	flip_screen_set(data & 0x08);
	m_sprites->set_flip(data & 0x08);
	machine().bookkeeping().coin_counter_w(0, data & 1);
	machine().bookkeeping().coin_counter_w(1, data & 2);

//  printf("blackt96_c0000_w %04x %04x\n",data & 0xfc,mem_mask);
}

void blackt96_state::tx_vram_w(offs_t offset, uint16_t data)
{
	m_tilemapram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset/2);
}

void blackt96_state::blackt96_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x080000, 0x080001).portr("P1_P2");
	map(0x080000, 0x080000).w(FUNC(blackt96_state::sound_cmd_w)); // soundlatch
	map(0x0c0000, 0x0c0001).portr("IN1");  // COIN INPUT
	map(0x0c0001, 0x0c0001).w(FUNC(blackt96_state::output_w));
	map(0x0e0000, 0x0e0001).r(FUNC(blackt96_state::random_r)); // unk, from sound? - called in tandem with result discarded, watchdog?
	map(0x0e8000, 0x0e8001).r(FUNC(blackt96_state::random_r)); // unk, from sound? /
	map(0x0f0000, 0x0f0001).portr("DSW1");
	map(0x0f0008, 0x0f0009).portr("DSW2").nopw(); // service mode, left-over?

	map(0x100000, 0x100fff).ram().w(FUNC(blackt96_state::tx_vram_w)).share("tilemapram"); // text tilemap
	map(0x200000, 0x207fff).rw(m_sprites, FUNC(snk68_spr_device::spriteram_r), FUNC(snk68_spr_device::spriteram_w)).share("spriteram");   // only partially populated
	map(0x400000, 0x400fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");

	map(0xc00000, 0xc03fff).ram(); // main ram
}

void blackt96_state::oki1_map(address_map &map)
{
	map(0x00000, 0x2ffff).rom();
	map(0x30000, 0x3ffff).bankr("oki1bank");
}




static INPUT_PORTS_START( blackt96 )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) // kick
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) // jump
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) // punch / pick up
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) // kick
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) // jump
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) // punch / pick up
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) // Test mode lists this as Service 1, but it appears to be Coin 1 (uses Coin 1 coinage etc.)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE1 ) // acts as a service mode mirror
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Test mode lists this as Coin 1, but it doesn't work
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	// Dipswitch Port A
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0300, 0x0000, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:!7,!8")
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0c00, 0x0000, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(      0x0c00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x1000, "3" )
	PORT_DIPNAME( 0x2000, 0x2000, "Bonus Life Type" ) PORT_DIPLOCATION("SW1:!3")
	PORT_DIPSETTING(      0x2000, "Every" )
	PORT_DIPSETTING(      0x0000, "Second Only" )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW1:!2")    // ?
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Yes ) ) // buggy, applies to attract mode only.

	// Dipswitch Port B
	PORT_START("DSW2")
	PORT_SERVICE( 0x0100, IP_ACTIVE_HIGH ) PORT_DIPLOCATION("SW2:!8")
	PORT_DIPNAME( 0x0200, 0x0000, "Continue" ) PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c00, 0x0400, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(      0x0000, "20000 / 50000" )
	PORT_DIPSETTING(      0x0400, "60000 / 150000" )
	PORT_DIPSETTING(      0x0800, "40000 / 100000" )
	PORT_DIPSETTING(      0x0c00, "No Bonus" )
	PORT_DIPNAME( 0x3000, 0x0000, "Demo Sound / Video Freeze" ) PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(      0x0000, "Demo Sound On" )
	PORT_DIPSETTING(      0x1000, "Never Finish" )
	PORT_DIPSETTING(      0x2000, "Demo Sound Off" )
	PORT_DIPSETTING(      0x3000, "Stop Video" )
	PORT_DIPNAME( 0xc000, 0x0000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:!1,!2") // 'Level'
	PORT_DIPSETTING(      0x8000, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x4000, "3" )
	PORT_DIPSETTING(      0xc000, "4" )
INPUT_PORTS_END



static const gfx_layout blackt96_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{  1024+32, 1024+40, 1024+48, 1024+56, 1024+0, 1024+8, 1024+16, 1024+24,
		32,40,48,56,0,8,16,24 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64, 8*64,9*64,10*64,11*64,12*64,13*64,14*64,15*64 },
	32*64
};

static const gfx_layout blackt962_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,24,8, 16 },
	{ 519, 515, 518, 514,  517,513,  516,512, 7,3,6,2,5,1,4,0 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,8*32,9*32,10*32,11*32,12*32,13*32,14*32,15*32 },
	32*32
};


static const gfx_layout blackt96_text_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,4,8, 12 },
	{ 131,130,129,128,3,2,1,0 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*16
};

static GFXDECODE_START( gfx_blackt96 )
	GFXDECODE_ENTRY( "bgtiles", 0, blackt96_layout,      0, 8  )
	GFXDECODE_ENTRY( "sprtiles", 0, blackt962_layout,     0, 128 )
	GFXDECODE_ENTRY( "txttiles", 0, blackt96_text_layout, 0, 16 )
GFXDECODE_END


void blackt96_state::soundio_port_a_w(uint8_t data)
{
	// soundbank
	logerror("%s: soundio_port_a_w (set soundbank %02x)\n", machine().describe_context().c_str(), data);
	m_oki1bank->set_entry(data & 0x07);
}

uint8_t blackt96_state::soundio_port_b_r()
{
	//logerror("%s: soundio_port_b_r (data read is %02x)\n", machine().describe_context().c_str(), m_port_b_latch);
	return m_port_b_latch;
}

void blackt96_state::soundio_port_b_w(uint8_t data)
{
	m_port_b_latch = data;
	//logerror("%s: soundio_port_b_w (set latch to %02x)\n", machine().describe_context().c_str(), m_port_b_latch);
}

uint8_t blackt96_state::soundio_port_c_r()
{
	// bit 0x40 = sound command ready?
	if (m_soundcmd_ready) return 0x40;
	return 0x00;
}

void blackt96_state::soundio_port_c_w(uint8_t data)
{
//  logerror("%s: soundio_port_c_w (PREV DATA %02x CURR DATA %02x)\n", machine().describe_context().c_str(), m_port_c_data, data);
	// data & 0x80 unused?
	// data & 0x40 is read - see above

	if (((data & 0x20) == 0x00) && ((m_port_c_data & 0x20) == 0x20)) // high -> low on bit 0x20 after processing command
	{
		m_soundcmd_ready = 0;
	}

	if (((data & 0x10) == 0x00) && ((m_port_c_data & 0x10) == 0x10)) // high -> low on bit 0x10 latches sound command
	{
		m_port_b_latch = m_soundcmd;
		//logerror("%s: soundio_port_c_w (latch sound command %02x)\n", machine().describe_context().c_str(), m_port_b_latch);
	}

	if (((data & 0x08) == 0x00) && ((m_port_c_data & 0x08) == 0x08)) // high -> low on bit 0x08 selects second oki?
	{
		m_oki_selected = 1;
	}

	if (((data & 0x04) == 0x00) && ((m_port_c_data & 0x04) == 0x04)) // high -> low on bit 0x04 selects first oki?
	{
		m_oki_selected = 0;
	}

	if (((data & 0x02) == 0x00) && ((m_port_c_data & 0x02) == 0x02)) // high -> low on bit 0x02 writes to selected OKI
	{
		//logerror("%s: soundio_port_c_w (write to OKI %02x) (oki selected is %02x)\n", machine().describe_context().c_str(), m_port_b_latch, m_oki_selected);
		if (m_oki_selected == 0) m_oki[0]->write(m_port_b_latch);
		else if (m_oki_selected == 1) m_oki[1]->write(m_port_b_latch);
	}

	if (((data & 0x01) == 0x00) && ((m_port_c_data & 0x01) == 0x01)) // high -> low on bit 0x01 reads to selected OKI
	{
		if (m_oki_selected == 0) m_port_b_latch = m_oki[0]->read();
		else if (m_oki_selected == 1) m_port_b_latch = m_oki[1]->read();
	}

	m_port_c_data = data;
}

void blackt96_state::tile_callback(int &tile, int& fx, int& fy, int& region)
{
	fx = tile & 0x4000;
	fy = tile & 0x8000;
	tile &= 0x3fff;

	if (tile & 0x2000)
	{
		region = 0;
	}
	else
	{
		region = 1;
	}
}


void blackt96_state::blackt96(machine_config &config)
{
	M68000(config, m_maincpu, 18_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &blackt96_state::blackt96_map);
	m_maincpu->set_vblank_int("screen", FUNC(blackt96_state::irq1_line_hold));

	pic16c57_device &audiocpu(PIC16C57(config, "audiocpu", 8_MHz_XTAL / 2));
	audiocpu.write_a().set(FUNC(blackt96_state::soundio_port_a_w));
	audiocpu.read_b().set(FUNC(blackt96_state::soundio_port_b_r));
	audiocpu.write_b().set(FUNC(blackt96_state::soundio_port_b_w));
	audiocpu.read_c().set(FUNC(blackt96_state::soundio_port_c_r));
	audiocpu.write_c().set(FUNC(blackt96_state::soundio_port_c_w));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_blackt96);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
//  screen.set_visarea(0*8, 16*32-1, 0*8, 16*32-1);
	screen.set_visarea(0*8, 256-1, 2*8, 240-1);
	screen.set_screen_update(FUNC(blackt96_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_444, 0x800);

	SNK68_SPR(config, m_sprites, 0);
	m_sprites->set_gfxdecode_tag(m_gfxdecode);
	m_sprites->set_tile_indirect_cb(FUNC(blackt96_state::tile_callback));
	m_sprites->set_no_partial();
	m_sprites->set_xpos_shift(12);
	m_sprites->set_color_entry_mask(0x7f);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	OKIM6295(config, m_oki[0], 8_MHz_XTAL / 8, okim6295_device::PIN7_HIGH); // music
	m_oki[0]->add_route(ALL_OUTPUTS, "lspeaker", 0.47);
	m_oki[0]->add_route(ALL_OUTPUTS, "rspeaker", 0.47);
	m_oki[0]->set_addrmap(0, &blackt96_state::oki1_map);

	OKIM6295(config, m_oki[1], 8_MHz_XTAL / 8, okim6295_device::PIN7_HIGH); // sfx
	m_oki[1]->add_route(ALL_OUTPUTS, "lspeaker", 0.47);
	m_oki[1]->add_route(ALL_OUTPUTS, "rspeaker", 0.47);
}


ROM_START( blackt96 )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "3", 0x00001, 0x40000, CRC(fc2c1d79) SHA1(742478237819af16d3fd66039283202b3c07eedd) )
	ROM_LOAD16_BYTE( "4", 0x00000, 0x40000, CRC(caff5b4a) SHA1(9a388cbb07211fa66f27082a8a5b847168c86a4f) )

	ROM_REGION( 0x2000, "audiocpu", 0 ) // PIC16c57 Code
	ROM_LOAD( "pic16c57.bin", 0x00000, 0x2000, CRC(6053ba2f) SHA1(5dd28ddff17555de0e8574b78ff9e71204c503d3) )

	ROM_REGION( 0x080000, "oki1", 0 ) // Samples
	ROM_LOAD( "1", 0x00000, 0x80000, CRC(6a934174) SHA1(087f5fa226dc68ee217f99c64d16cdf14372d44c) )

	ROM_REGION( 0x040000, "oki2", 0 ) // Samples
	ROM_LOAD( "2", 0x00000, 0x40000, CRC(94009cd4) SHA1(aa36298e280c20bf86d70f3eb3fb33aca4df07e3) )

	ROM_REGION( 0x200000, "bgtiles", 0 )
	ROM_LOAD16_BYTE( "5", 0x100000, 0x40000, CRC(6e52c331) SHA1(31ef1d352d4ee5f7b3ef336b1f052c3a1468f22e) )
	ROM_LOAD16_BYTE( "6", 0x100001, 0x40000, CRC(69637a5a) SHA1(a5731478856d8bb91d34b747838b2b47772864ef) )
	ROM_LOAD16_BYTE( "7", 0x000000, 0x80000, CRC(6b04e8a8) SHA1(309ba1efd60600a30e1ae8f6e8b92939c23cd9c6) )
	ROM_LOAD16_BYTE( "8", 0x000001, 0x80000, CRC(16c656be) SHA1(06c40c16080a97b01a638776d28f36594ce4fb3b) )

	ROM_REGION( 0x100000, "sprtiles", 0 )
	ROM_LOAD32_BYTE( "11", 0x00000, 0x40000, CRC(9eb773a3) SHA1(9c91ee938438a61f5fa650ced6249e34aa5321bd) )
	ROM_LOAD32_BYTE( "12", 0x00001, 0x40000, CRC(8894e608) SHA1(389974a0b208b7cbf7d5f83641ddc058ad5ebe87) )
	ROM_LOAD32_BYTE( "13", 0x00002, 0x40000, CRC(0acceb9d) SHA1(e8a85c7eab45d84613ac37a9b7ffbc45b44eb2e5) )
	ROM_LOAD32_BYTE( "14", 0x00003, 0x40000, CRC(b5e3de25) SHA1(33ac5602ab6bcadc8b0d1aa805a3bdce0b67c215) )

	ROM_REGION( 0x10000, "txttiles", 0 )
	ROM_LOAD16_BYTE( "9",  0x00000, 0x08000, CRC(81a4cf4c) SHA1(94b2bbcbc8327d9babbc3b222bd88954c7e7b80e) )
	ROM_CONTINUE(          0x00000, 0x08000 ) // first half is empty
	ROM_LOAD16_BYTE( "10", 0x00001, 0x08000, CRC(b78232a2) SHA1(36a4f01011faf64e46b73f0082ab04843ac8b0e2) )
	ROM_CONTINUE(          0x00001, 0x08000 ) // first half is empty
ROM_END

} // anonymous namespace


// I'm not really sure this needs MACHINE_IS_INCOMPLETE just because there are some original game bugs, it's quite typical of this type of Korean release
GAME( 1996, blackt96, 0, blackt96, blackt96, blackt96_state, empty_init, ROT0, "D.G.R.M.", "Black Touch '96", MACHINE_IS_INCOMPLETE | MACHINE_SUPPORTS_SAVE )

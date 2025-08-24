// license:BSD-3-Clause
// copyright-holders: Paul Priest, David Haywood, Luca Elia

/***************************************************************************

                          -= Fuuki 32 Bit Games (FG-3) =-

                driver by Paul Priest and David Haywood
                based on fuukifg2 by Luca Elia

Hardware is similar to FG-2 used for :
"Go Go! Mile Smile", "Susume! Mile Smile (Japan)" & "Gyakuten!! Puzzle Bancho (Japan, set 1)"
See fuukifg2.cpp

Main  CPU   :   M68020

---------------------------------------------------------------------------
Year + Game
---------------------------------------------------------------------------
98  Asura Blade - Sword of Dynasty (Japan)
00  Asura Buster - Eternal Warriors (Japan)
01  Asura Buster - Eternal Warriors (USA)

English version of Asura Blade also exist, but it isn't dumped

---------------------------------------------------------------------------

--
Notes so far:

- Dips are correct for game play. Asura Buster's service mode does show the effects of
  dipswitches and dipswitch state. For the "Round" listing, the 2 and 3 are shown in
  reverse of actual game play. Any reference below to "Service Mode" means Asura
  Buster's service mode.

- Raster Effects are imperfect, bad frames when lots of new sprites.

- The scroll values are generally wrong when flip screen is on and rasters are often incorrect

- PCM channels of music in asurabus is sometimes off-tune, check Chen-Mao's stage for example
  note: srom.u7 (z80 prg) is a good dump

Asura Blade
Fuuki Co. Ltd., 1998

PCB Layout
----------

Top Board

FG-3J MAIN-J Revision:1.1
|-----------------------------------------------------|
|  YAC516  YMF278 N341256(x4) N341028 (x4)    FI-002K |
|  33.8688MHz                 N341512(x4)             |
|   PAL N341256                                       |
|   Z80                     N341256                   |
|                           N341256                   |
|J DSW1                                               |
|A                                                    |
|M       12MHz                   FI-003K  N341256(x2) |
|M DSW2                N341256(x3)                    |
|A               PAL                                  |
|         40MHz  PAL                                  |
|  DSW3          PAL  N341256                         |
|         68020  PAL  N341256                         |
|        N341256      28.432MHz    M60067-0901FP      |
|  DSW4  N341256 PAL                                  |
|-----------------------------------------------------|

Notes:
      68020 clock: 20.000MHz
        Z80 clock: 6.000MHz
      YM278 clock: 33.8688MHz
            VSync: 60Hz
            Hsync: 15.81kHz


Bottom Board

FG-3J ROM-J 507KA0301P04       Rev:1.3
|--------------------------------|
|                          SROM  |
|                                |
|  SP01*      SP89         PCM   |
|                                |
|  SP23       SPAB               |
|                                |
|  SP45       SPCD         MAP   |
|                                |
|  SP67       SPEF*        PGM3  |
|                                |
|                          PGM2  |
|                                |
|  BG2123     BG1113       PGM1  |
|                                |
|  BG2022     BG1012       PGM0  |
|--------------------------------|

* = Not populated

****************************************************************************

Asura Buster
Fuuki Co. Ltd.

PCB Layout
----------

Top Board

FG-3J MAIN-J Revision:1.1
|-----------------------------------------------------|
|  YAC516  YMF278 N341256(x4) N341028 (x4)    FI-002K |
|  33.8688MHz                 N341512(x4)             |
|   PAL N341256                                       |
|   Z80                     N341256                   |
|                           N341256                   |
|J DSW1                                               |
|A                                                    |
|M       12MHz                   FI-003K  N341256(x2) |
|M DSW2                N341256(x3)                    |
|A               PAL                                  |
|         40MHz  PAL                                  |
|  DSW3          PAL  N341256                         |
|         68020  PAL  N341256                         |
|        N341256      28.432MHz    M60067-0901FP      |
|  DSW4  N341256 PAL                                  |
|-----------------------------------------------------|

Notes:
      68020 clock: 20.000MHz [40/2]
        Z80 clock: 6.000MHz [12/2]
      YM278 clock: 33.8688MHz
            VSync: 60Hz
            Hsync: 15.81kHz


Bottom Board

FG-3J ROM-J 507KA0301P04       Rev:1.3
|--------------------------------|
|                          SROM  |
|                                |
|  SP01       SP89         OPM   |
|                                |
|  SP23       SPAB               |
|                                |
|  SP45       SPCD         MAP   |
|                                |
|  SP67       SPEF         PGM3  |
|                                |
|                          PGM2  |
|                                |
|  BG2123     BG1113       PGM1  |
|                                |
|  BG2022     BG1012       PGM0  |
|--------------------------------|


There is an Asura Buster known to exist on a FG3-SUB-EP containing all EPROMs
 with the following checksum values for the program ROMS:

  PGM0 - BB1D, PGM1 - 6D84, PGM2 - EE6B & PGM3 - 7977  (which match the current Japan, set 1)
  other game EPROMs dated, 10/30, 11/1 or 11/2

***************************************************************************/

#include "emu.h"

#include "fuukispr.h"
#include "fuukitmap.h"

#include "cpu/m68000/m68020.h"
#include "cpu/z80/z80.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class fuuki32_state : public driver_device
{
public:
	fuuki32_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_fuukispr(*this, "fuukispr")
		, m_fuukitmap(*this, "fuukitmap")
		, m_spriteram(*this, "spriteram", 0x2000, ENDIANNESS_BIG)
		, m_tilebank(*this, "tilebank")
		, m_shared_ram(*this, "shared_ram")
		, m_soundbank(*this, "soundbank")
		, m_system(*this, "SYSTEM")
		, m_inputs(*this, "INPUTS")
		, m_dsw(*this, "DSW%u", 1U)
	{ }

	void fuuki32(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<fuukispr_device> m_fuukispr;
	required_device<fuukitmap_device> m_fuukitmap;

	// memory pointers
	memory_share_creator<u16> m_spriteram;
	required_shared_ptr<u32> m_tilebank;
	required_shared_ptr<u8> m_shared_ram;
	std::unique_ptr<u16[]> m_buf_spriteram[2];

	required_memory_bank m_soundbank;

	required_ioport m_system;
	required_ioport m_inputs;
	required_ioport_array<2> m_dsw;

	// video-related
	u32 m_spr_buffered_tilebank[2]{};

	void main_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	u8 snd_020_r(offs_t offset);
	void snd_020_w(offs_t offset, u8 data, u8 mem_mask = ~0);
	void sprram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 sprram_r(offs_t offset);
	void sound_bw_w(u8 data);

	void spr_tile_cb(u32 &code);
	void spr_colpri_cb(u32 &colour, u32 &pri_mask);
	void tmap_colour_cb(u8 layer, u32 &colour);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
};


/***************************************************************************

    [ 4 Scrolling Layers ]

                            [ Layer 0 ]     [ Layer 1 ]     [ Layer 2 (double-buffered) ]

    Tile Size:              16 x 16 x 8     16 x 16 x 8     8 x 8 x 4
    Layer Size (tiles):     64 x 32         64 x 32         64 x 32

    [ 1024? Zooming Sprites ]

    Sprites are made of 16 x 16 x 4 tiles. Size can vary from 1 to 16
    tiles both horizontally and vertically.
    There is zooming (from full size to half size) and 4 levels of
    priority (wrt layers)

    Per-line raster effects used on many stages
    Sprites buffered by two frames
    Tilebank buffered by 3 frames? Only 2 in attract
    Sprite pens needs to be buffered by 3 frames? Or lazy programming? Probably 2

***************************************************************************/

/***************************************************************************


                            Video Hardware Init


***************************************************************************/

void fuuki32_state::video_start()
{
	const u32 spriteram_size = m_spriteram.bytes();
	m_buf_spriteram[0] = make_unique_clear<u16[]>(spriteram_size / 2);
	m_buf_spriteram[1] = make_unique_clear<u16[]>(spriteram_size / 2);

	m_fuukitmap->set_transparent_pen(0, 0xff);    // 8 bits
	m_fuukitmap->set_transparent_pen(1, 0xff);    // 8 bits
	m_fuukitmap->set_transparent_pen(2, 0x0f);    // 4 bits

	//m_fuukitmap->gfx(0)->set_granularity(16); // 256 colour tiles with palette selectable on 16 colour boundaries
	//m_fuukitmap->gfx(1)->set_granularity(16);

	save_pointer(NAME(m_buf_spriteram[0]), spriteram_size / 2);
	save_pointer(NAME(m_buf_spriteram[1]), spriteram_size / 2);
}


void fuuki32_state::sprram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_spriteram[offset]);
};

u16 fuuki32_state::sprram_r(offs_t offset)
{
	return m_spriteram[offset];
}

void fuuki32_state::spr_tile_cb(u32 &code)
{
	const u32 bank = (code & 0xc000) >> 14;

	const u32 bank_lookedup = ((m_spr_buffered_tilebank[1] & 0xffff0000) >> (16 + bank * 4)) & 0xf;
	code &= 0x3fff;
	code += bank_lookedup * 0x4000;
}

void fuuki32_state::spr_colpri_cb(u32 &colour, u32 &pri_mask)
{
	const u8 priority = (colour >> 6) & 3;
	switch (priority)
	{
		case 3:  pri_mask = 0xf0 | 0xcc | 0xaa;  break;  // behind all layers
		case 2:  pri_mask = 0xf0 | 0xcc;         break;  // behind fg + middle layer
		case 1:  pri_mask = 0xf0;                break;  // behind fg layer
		case 0:
		default: pri_mask = 0;                       // above all
	}
	colour &= 0x3f;
}

void fuuki32_state::tmap_colour_cb(u8 layer, u32 &colour)
{
	if (layer < 2)
		colour >>= 4;
}

u32 fuuki32_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_fuukitmap->prepare();
	flip_screen_set(m_fuukitmap->flip_screen());

	// The bg colour is the last pen i.e. 0x1fff
	bitmap.fill((0x800 * 4) - 1, cliprect);
	screen.priority().fill(0, cliprect);

	m_fuukitmap->draw_layer(screen, bitmap, cliprect, m_fuukitmap->tmap_back(),   0, 1);
	m_fuukitmap->draw_layer(screen, bitmap, cliprect, m_fuukitmap->tmap_middle(), 0, 2);
	m_fuukitmap->draw_layer(screen, bitmap, cliprect, m_fuukitmap->tmap_front(),  0, 4);

	m_fuukispr->draw_sprites(screen, bitmap, cliprect, flip_screen(), m_buf_spriteram[1].get(), m_spriteram.bytes() / 2);
	return 0;
}

void fuuki32_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		// Buffer sprites and tilebank by 2 frames
		m_spr_buffered_tilebank[1] = m_spr_buffered_tilebank[0];
		m_spr_buffered_tilebank[0] = m_tilebank[0];
		memcpy(m_buf_spriteram[1].get(), m_buf_spriteram[0].get(), m_spriteram.bytes());
		memcpy(m_buf_spriteram[0].get(), m_spriteram, m_spriteram.bytes());
	}
}


//-------------------------------------------------
//  memory - main CPU
//-------------------------------------------------

// Sound comms
u8 fuuki32_state::snd_020_r(offs_t offset)
{
	machine().scheduler().synchronize();
	return m_shared_ram[offset];
}

void fuuki32_state::snd_020_w(offs_t offset, u8 data, u8 mem_mask)
{
	machine().scheduler().synchronize();
	COMBINE_DATA(&m_shared_ram[offset]);
}

void fuuki32_state::main_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
	map(0x400000, 0x40ffff).ram();                                                                     // Work RAM
	map(0x410000, 0x41ffff).ram();                                                                     // Work RAM (used by asurabus)

	map(0x500000, 0x507fff).m(m_fuukitmap, FUNC(fuukitmap_device::vram_map));
	map(0x508000, 0x517fff).ram();                                                                     // More tilemap, or linescroll? Seems to be empty all of the time
	map(0x600000, 0x601fff).rw(FUNC(fuuki32_state::sprram_r), FUNC(fuuki32_state::sprram_w));
	map(0x700000, 0x703fff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");

	map(0x800000, 0x800003).lr16(NAME([this] () { return u16(m_system->read()); })).nopw();  // Coin
	map(0x810000, 0x810003).lr16(NAME([this] () { return u16(m_inputs->read()); })).nopw();  // Player inputs
	map(0x880000, 0x880003).lr16(NAME([this] () { return u16(m_dsw[0]->read()); }));           // Service + DIPs
	map(0x890000, 0x890003).lr16(NAME([this] () { return u16(m_dsw[1]->read()); }));           // More DIPs

	map(0x8c0000, 0x8effff).m(m_fuukitmap, FUNC(fuukitmap_device::vregs_map));
	map(0x903fe0, 0x903fff).rw(FUNC(fuuki32_state::snd_020_r), FUNC(fuuki32_state::snd_020_w)).umask32(0x00ff00ff);                                         // Shared with Z80
	map(0xa00000, 0xa00003).writeonly().share(m_tilebank);
}


//-------------------------------------------------
//  memory - sound CPU
//-------------------------------------------------

void fuuki32_state::sound_bw_w(u8 data)
{
	m_soundbank->set_entry(data);
}

void fuuki32_state::sound_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x6fff).ram();
	map(0x7ff0, 0x7fff).ram().share(m_shared_ram);
	map(0x8000, 0xffff).bankr(m_soundbank);
}

void fuuki32_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(fuuki32_state::sound_bw_w));
	map(0x30, 0x30).nopw(); // leftover/unused nmi handler related
	map(0x40, 0x45).rw("ymf", FUNC(ymf278b_device::read), FUNC(ymf278b_device::write));
}


//-------------------------------------------------
//  input ports
//-------------------------------------------------

static INPUT_PORTS_START( asurabld )
	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xfe00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_SERVICE_DIPLOC(  0x0001, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x0002, 0x0002, "Blood Color" )           PORT_DIPLOCATION("SW1:2") // Any other censorship? (Tested in 3 locations)
	PORT_DIPSETTING(      0x0002, "Red" )
	PORT_DIPSETTING(      0x0000, "Green" )
	PORT_DIPNAME( 0x000c, 0x000c, "Demo Sounds & Music" )   PORT_DIPLOCATION("SW1:3,4") // Tested @ 0917AC
	PORT_DIPSETTING(      0x000c, "Both On" )
	PORT_DIPSETTING(      0x0008, "Music Off" )
	PORT_DIPSETTING(      0x0004, "Both Off" )
	PORT_DIPSETTING(      0x0000, "Both Off" )              // Duplicate setting
	PORT_DIPNAME( 0x0030, 0x0030, "Timer" )                 PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0000, "Slow" )
	PORT_DIPSETTING(      0x0030, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0010, "Fast" )
	PORT_DIPSETTING(      0x0020, "Very Fast" )
	PORT_DIPNAME( 0x00c0, 0x0000, "Coinage Mode" )          PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x00c0, "Split" )
	PORT_DIPSETTING(      0x0000, "Joint" )
	PORT_DIPUNUSED_DIPLOC( 0x0100, 0x0100, "SW2:1" )        // DSW2 bank, not used for either game
	PORT_DIPUNUSED_DIPLOC( 0x0200, 0x0200, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x0400, 0x0400, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0800, 0x0800, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x1000, 0x1000, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x2000, 0x2000, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000e, 0x000e, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW3:2,3,4") // AKA Computer Level, see @ 0917CC
	PORT_DIPSETTING(      0x0000, DEF_STR( Easiest ) )      // Level 1
	PORT_DIPSETTING(      0x0008, DEF_STR( Very_Easy ) )    // Level 2
	PORT_DIPSETTING(      0x0004, DEF_STR( Easier ) )       // Level 3
	PORT_DIPSETTING(      0x000c, DEF_STR( Easy ) )         // Level 4
	PORT_DIPSETTING(      0x000e, DEF_STR( Normal ) )       // Level 5
	PORT_DIPSETTING(      0x0002, DEF_STR( Hard ) )         // Level 6
	PORT_DIPSETTING(      0x000a, DEF_STR( Very_Hard ) )    // Level 7
	PORT_DIPSETTING(      0x0006, DEF_STR( Hardest ) )      // Level 8
	PORT_DIPNAME( 0x0030, 0x0030, "Damage" )                PORT_DIPLOCATION("SW3:5,6")
	PORT_DIPSETTING(      0x0020, "75%" )
	PORT_DIPSETTING(      0x0030, "100%" )
	PORT_DIPSETTING(      0x0010, "125%" )
	PORT_DIPSETTING(      0x0000, "150%" )
	PORT_DIPNAME( 0x00c0, 0x00c0, "Max Rounds" )            PORT_DIPLOCATION("SW3:7,8") // Service Mode shows rounds needed to win the match
	PORT_DIPSETTING(      0x0000, "1" )                     // Service Mode Shows 1
	PORT_DIPSETTING(      0x00c0, "3" )                     // Service Mode Shows 3, Service Mode has 2 & 3 reversed compared to game play
	PORT_DIPSETTING(      0x0080, "5" )                     // Service Mode Shows 2, Service Mode has 2 & 3 reversed compared to game play
	PORT_DIPSETTING(      0x0040, "Error!!" )               // Service Mode Shows "Error"
	PORT_DIPNAME( 0xf000, 0xf000, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW4:1,2,3,4") // Service Mode Shows Player 2
	PORT_DIPSETTING(      0x8000, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(      0x9000, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0xb000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0xd000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 2C_1C ) )        // Duplicate 2C_1C
	PORT_DIPSETTING(      0xf000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x2000, "2C Start / 1C Continue" )
	PORT_DIPSETTING(      0x7000, "Error!!" )               // Causes graphics issues - Service Mode shows "Error"
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW2", 0x0f00, NOTEQUALS, 0x0000)
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )    PORT_CONDITION("DSW2", 0x0f00, EQUALS, 0x0000) // Set both for Free Play
	PORT_DIPNAME( 0x0f00, 0x0f00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW4:5,6,7,8") // Service Mode Shows Player 1
	PORT_DIPSETTING(      0x0800, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(      0x0900, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x0b00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0d00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 2C_1C ) )        // Duplicate 2C_1C
	PORT_DIPSETTING(      0x0f00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0200, "2C Start / 1C Continue" )
	PORT_DIPSETTING(      0x0700, "Error!!" )               // Causes graphics issues - Service Mode shows "Error"
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW2", 0xf000, NOTEQUALS, 0x0000)
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )    PORT_CONDITION("DSW2", 0xf000, EQUALS, 0x0000) // Set both for Free Play
INPUT_PORTS_END

static INPUT_PORTS_START( asurabus )
	PORT_INCLUDE(asurabld)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x000c, 0x000c, "Demo Sounds & Music" )   PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x000c, "Both On" )
	PORT_DIPSETTING(      0x0008, "Sounds Off" )
	PORT_DIPSETTING(      0x0004, "Music Off" )
	PORT_DIPSETTING(      0x0000, "Both Off" )
INPUT_PORTS_END

static INPUT_PORTS_START( asurabusa )
	PORT_INCLUDE(asurabld)

	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
INPUT_PORTS_END

//-------------------------------------------------
//  graphics layouts
//-------------------------------------------------

// 16x16x8
static const gfx_layout layout_16x16x8 =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ STEP4(0,1), STEP4(16,1) },
	{ STEP4(0,4), STEP4(16*2,4), STEP4(16*4,4), STEP4(16*6,4) },
	{ STEP16(0,16*8) },
	16*16*8
};

static GFXDECODE_START( gfx_fuuki32 )
	GFXDECODE_ENTRY( "tiles_l0", 0, layout_16x16x8,         0x400*0, 0x40 ) // [0] Layer 1
	GFXDECODE_ENTRY( "tiles_l1", 0, layout_16x16x8,         0x400*1, 0x40 ) // [1] Layer 2
	GFXDECODE_ENTRY( "tiles_bg", 0, gfx_8x8x4_packed_msb,   0x400*3, 0x40 ) // [2] BG Layer
GFXDECODE_END


//-------------------------------------------------
//  driver functions
//-------------------------------------------------

void fuuki32_state::machine_start()
{
	u8 *rom = memregion("soundcpu")->base();

	m_soundbank->configure_entries(0, 0x10, &rom[0], 0x8000);

	save_item(NAME(m_spr_buffered_tilebank));
}


void fuuki32_state::fuuki32(machine_config &config)
{
	// basic machine hardware
	M68EC020(config, m_maincpu, 40_MHz_XTAL / 2); // 20MHz verified
	m_maincpu->set_addrmap(AS_PROGRAM, &fuuki32_state::main_map);

	z80_device &soundcpu(Z80(config, "soundcpu", 12_MHz_XTAL / 2)); // 6MHz verified
	soundcpu.set_addrmap(AS_PROGRAM, &fuuki32_state::sound_map);
	soundcpu.set_addrmap(AS_IO, &fuuki32_state::sound_io_map);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(64 * 8, 32 * 8);
	m_screen->set_visarea(0, 40 * 8 - 1, 0, 30 * 8 - 1);
	m_screen->set_screen_update(FUNC(fuuki32_state::screen_update));
	m_screen->screen_vblank().set(FUNC(fuuki32_state::screen_vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_fuuki32);
	PALETTE(config, m_palette, palette_device::BLACK).set_format(palette_device::xRGB_555, 0x4000 / 2);

	FUUKI_SPRITE(config, m_fuukispr, 0);
	m_fuukispr->set_palette(m_palette);
	m_fuukispr->set_color_base(0x400*2);
	m_fuukispr->set_color_num(0x40);
	m_fuukispr->set_tile_callback(FUNC(fuuki32_state::spr_tile_cb));
	m_fuukispr->set_colpri_callback(FUNC(fuuki32_state::spr_colpri_cb));

	FUUKI_TILEMAP(config, m_fuukitmap, 0, m_palette, gfx_fuuki32);
	m_fuukitmap->set_screen(m_screen);
	m_fuukitmap->set_colour_callback(FUNC(fuuki32_state::tmap_colour_cb));
	m_fuukitmap->level_1_irq_callback().set_inputline(m_maincpu, 1, HOLD_LINE);
	m_fuukitmap->vblank_irq_callback().set_inputline(m_maincpu, 3, HOLD_LINE);
	m_fuukitmap->raster_irq_callback().set_inputline(m_maincpu, 5, HOLD_LINE);
	m_fuukitmap->set_xoffs(0x1f3, 0x103);
	m_fuukitmap->set_yoffs(0x3f6, 0x2c7);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	ymf278b_device &ymf(YMF278B(config, "ymf", 33.8688_MHz_XTAL));
	ymf.irq_handler().set_inputline("soundcpu", 0);
	ymf.add_route(0, "speaker", 0.50, 0);
	ymf.add_route(1, "speaker", 0.50, 1);
	ymf.add_route(2, "speaker", 0.40, 0);
	ymf.add_route(3, "speaker", 0.40, 1);
	ymf.add_route(4, "speaker", 0.50, 0);
	ymf.add_route(5, "speaker", 0.50, 1);
}

//-------------------------------------------------
//  ROM loading
//-------------------------------------------------

/***************************************************************************

                 Asura Blade - Sword of Dynasty (Japan)

Fuuki, 1999   Consists of a FG-3J MAIN-J mainboard &  FG-3J ROM-J combo

***************************************************************************/

ROM_START( asurabld )
	ROM_REGION( 0x200000, "maincpu", 0 ) // M68020
	ROM_LOAD32_BYTE( "pgm3.u1", 0x000000, 0x80000, CRC(053e9758) SHA1(c2754d3f0c607c81c8fa33b667b576eb0474fd0b) )
	ROM_LOAD32_BYTE( "pgm2.u2", 0x000001, 0x80000, CRC(16b656ca) SHA1(5ffb551ce7dec462d3896f0fed693454496894bc) )
	ROM_LOAD32_BYTE( "pgm1.u3", 0x000002, 0x80000, CRC(35104452) SHA1(03cfd81429f8a945d5419c9750925bfa997d0607) )
	ROM_LOAD32_BYTE( "pgm0.u4", 0x000003, 0x80000, CRC(68615497) SHA1(de93751f151f195a863dc6fe83b6e7ed8f99430a) )

	ROM_REGION( 0x80000, "soundcpu", 0 ) // Z80
	ROM_LOAD( "srom.u7", 0x00000, 0x80000, CRC(bb1deb89) SHA1(b1c70abddc0b9a88beb69a592376ff69a7e091eb) )

	ROM_REGION( 0x2000000, "fuukispr", 0 )
	// 0x0000000 - 0x03fffff empty */ /* spXX.uYY - XX is the bank number!
	ROM_LOAD16_WORD_SWAP( "sp23.u14", 0x0400000, 0x400000, CRC(7df492eb) SHA1(30b88a3cd025ffc8c28fef06e0784755be37ef8e) )
	ROM_LOAD16_WORD_SWAP( "sp45.u15", 0x0800000, 0x400000, CRC(1890f42a) SHA1(22254fe38fd83f4602a25e1ccba32df16edaf3f9) )
	ROM_LOAD16_WORD_SWAP( "sp67.u16", 0x0c00000, 0x400000, CRC(a48f1ef0) SHA1(bf8787f293793291a503af662d3738c007654726) )
	ROM_LOAD16_WORD_SWAP( "sp89.u17", 0x1000000, 0x400000, CRC(6b024362) SHA1(8be5cc3c7306d28b75acd970bb3be6d3c9825367) )
	ROM_LOAD16_WORD_SWAP( "spab.u18", 0x1400000, 0x400000, CRC(803d2d8c) SHA1(25df30689e576a0620656c721d92bcc3fbd84844) )
	ROM_LOAD16_WORD_SWAP( "spcd.u19", 0x1800000, 0x400000, CRC(42e5c26e) SHA1(b68875d353bdc5d49113bbac02fd83508bce66a5) )

	ROM_REGION( 0x0800000, "tiles_l0", 0 )
	ROM_LOAD32_WORD_SWAP( "bg1012.u22", 0x0000002, 0x400000, CRC(d717a0a1) SHA1(007df309dc0650ca07e077b983a2b05730349d0b) )
	ROM_LOAD32_WORD_SWAP( "bg1113.u23", 0x0000000, 0x400000, CRC(94338267) SHA1(7848bc57cb0eac216100a508763451eb57a0a082) )

	ROM_REGION( 0x0800000, "tiles_l1", 0 )
	ROM_LOAD32_WORD_SWAP( "bg2022.u25", 0x0000002, 0x400000, CRC(ee312cd3) SHA1(2ef9d51928d80375daf8e6b204bb66a8b9cbaee7) )
	ROM_LOAD32_WORD_SWAP( "bg2123.u24", 0x0000000, 0x400000, CRC(4acfc469) SHA1(a98d06b967ebb3fa3b4c8aa3d7a05063ec981fb2) )

	ROM_REGION( 0x200000, "tiles_bg", 0 )
	ROM_LOAD16_WORD_SWAP( "map.u5", 0x00000, 0x200000, CRC(e681155e) SHA1(458845b9c86df72685d92d0d4052aacc2fa7d1bd) )

	ROM_REGION( 0x400000, "ymf", 0 ) // OPL4 samples
	ROM_LOAD( "pcm.u6", 0x00000, 0x400000, CRC(ac72225a) SHA1(8d16399ed34ac5bd69dbf43b2de2b0db9ac1c610) )
ROM_END

/***************************************************************************

                 Asura Buster - Eternal Warriors (Japan)

Fuuki, 2000   Consists of a FG-3J MAIN-J mainboard &  FG-3J ROM-J combo

***************************************************************************/

ROM_START( asurabus )
	ROM_REGION( 0x200000, "maincpu", 0 ) // M68020
	ROM_LOAD32_BYTE( "uspgm3.u1", 0x000000, 0x80000, CRC(e152cec9) SHA1(af1d93bdabc6732c0ff53972826d67a3753ef785) ) // hand written labels
	ROM_LOAD32_BYTE( "uspgm2.u2", 0x000001, 0x80000, CRC(b19787db) SHA1(3d6757f38f297c1ee89003173567319b5dae8000) )
	ROM_LOAD32_BYTE( "uspgm1.u3", 0x000002, 0x80000, CRC(6588e51a) SHA1(9ab978c80d8ece447697557c8000be95760306f3) )
	ROM_LOAD32_BYTE( "uspgm0.u4", 0x000003, 0x80000, CRC(981e6ff1) SHA1(088d26a3cbd2361ffc756c3da8a67b94ae7bbd65) )

	ROM_REGION( 0x80000, "soundcpu", 0 ) // Z80
	ROM_LOAD( "srom.u7", 0x00000, 0x80000, CRC(368da389) SHA1(1423b709da40bf3033c9032c4bd07658f1a969de) )

	ROM_REGION( 0x2000000, "fuukispr", 0 )
	ROM_LOAD16_WORD_SWAP( "sp01.u13", 0x0000000, 0x400000, CRC(5edea463) SHA1(22a780912f060bae0c9a403a7bfd4d27f25b76e3) )
	ROM_LOAD16_WORD_SWAP( "sp23.u14", 0x0400000, 0x400000, CRC(91b1b0de) SHA1(341367966559ef2027415b673eb0db704680c81f) )
	ROM_LOAD16_WORD_SWAP( "sp45.u15", 0x0800000, 0x400000, CRC(96c69aac) SHA1(cf053523026651427f884b9dd7c095af362dd24e) )
	ROM_LOAD16_WORD_SWAP( "sp67.u16", 0x0c00000, 0x400000, CRC(7c3d83bf) SHA1(7188dd923c6c7eb6aee3323e7ab54aa240c35ea3) )
	ROM_LOAD16_WORD_SWAP( "sp89.u17", 0x1000000, 0x400000, CRC(cb1e14f8) SHA1(941cea1887d7ceb52222adcf1d6913969e6163aa) )
	ROM_LOAD16_WORD_SWAP( "spab.u18", 0x1400000, 0x400000, CRC(e5a4608d) SHA1(b8e39f53e0b7ad1e16ae9c3726597776b404be1c) )
	ROM_LOAD16_WORD_SWAP( "spcd.u19", 0x1800000, 0x400000, CRC(99bfbe32) SHA1(926a8afc4a175874f22f53300e76f59331d3b9ba) )
	ROM_LOAD16_WORD_SWAP( "spef.u20", 0x1c00000, 0x400000, CRC(c9c799cc) SHA1(01373316700d8688deeea2e9e8f831d5f86c7f17) )

	ROM_REGION( 0x0800000, "tiles_l0", 0 )
	ROM_LOAD32_WORD_SWAP( "bg1012.u22", 0x0000002, 0x400000, CRC(e3fb9af0) SHA1(11900cc2873337692f66fb4f1eb9c574e5a967de) )
	ROM_LOAD32_WORD_SWAP( "bg1113.u23", 0x0000000, 0x400000, CRC(5f8657e6) SHA1(7c2854dc5d2d4efe55bda01e329da051350e0031) )

	ROM_REGION( 0x0800000, "tiles_l1", 0 )
	ROM_LOAD32_WORD_SWAP( "bg2022.u25", 0x0000002, 0x400000, CRC(f46eda52) SHA1(46530016b32a164bd76c4f53e7b53b2beb28db06) )
	ROM_LOAD32_WORD_SWAP( "bg2123.u24", 0x0000000, 0x400000, CRC(c4ebb86b) SHA1(a7093e6e02b64566d277cbbd5fa90cd430e7c8a0) )

	ROM_REGION( 0x200000, "tiles_bg", 0 )
	ROM_LOAD16_WORD_SWAP( "map.u5", 0x00000, 0x200000, CRC(bd179dc5) SHA1(ce3fcac573b14fd5365eb5dcec3257e439d2c129) )

	ROM_REGION( 0x400000, "ymf", 0 ) // OPL4 samples
	ROM_LOAD( "opm.u6", 0x00000, 0x400000, CRC(31b05be4) SHA1(d0f4f387f84a74591224b0f42b7f5c538a3dc498) )
ROM_END

ROM_START( asurabusj )
	ROM_REGION( 0x200000, "maincpu", 0 ) // M68020
	ROM_LOAD32_BYTE( "pgm3.u1", 0x000000, 0x80000, CRC(2c6b5271) SHA1(188371f1f003823ac719e962e048719d76696b2f) )
	ROM_LOAD32_BYTE( "pgm2.u2", 0x000001, 0x80000, CRC(8f8694ec) SHA1(3334df4aecc5ab2f8914ef6748c027a99b39ce26) )
	ROM_LOAD32_BYTE( "pgm1.u3", 0x000002, 0x80000, CRC(0a040f0f) SHA1(d5e86d33efcbbde7ee62cfc8dfe867f250a33415) )
	ROM_LOAD32_BYTE( "pgm0.u4", 0x000003, 0x80000, CRC(9b71e9d8) SHA1(9b705b5b6fff549f5679890422b481b5cf1d7bd7) )

	ROM_REGION( 0x80000, "soundcpu", 0 ) // Z80
	ROM_LOAD( "srom.u7", 0x00000, 0x80000, CRC(368da389) SHA1(1423b709da40bf3033c9032c4bd07658f1a969de) )

	ROM_REGION( 0x2000000, "fuukispr", 0 )
	ROM_LOAD16_WORD_SWAP( "sp01.u13", 0x0000000, 0x400000, CRC(5edea463) SHA1(22a780912f060bae0c9a403a7bfd4d27f25b76e3) )
	ROM_LOAD16_WORD_SWAP( "sp23.u14", 0x0400000, 0x400000, CRC(91b1b0de) SHA1(341367966559ef2027415b673eb0db704680c81f) )
	ROM_LOAD16_WORD_SWAP( "sp45.u15", 0x0800000, 0x400000, CRC(96c69aac) SHA1(cf053523026651427f884b9dd7c095af362dd24e) )
	ROM_LOAD16_WORD_SWAP( "sp67.u16", 0x0c00000, 0x400000, CRC(7c3d83bf) SHA1(7188dd923c6c7eb6aee3323e7ab54aa240c35ea3) )
	ROM_LOAD16_WORD_SWAP( "sp89.u17", 0x1000000, 0x400000, CRC(cb1e14f8) SHA1(941cea1887d7ceb52222adcf1d6913969e6163aa) )
	ROM_LOAD16_WORD_SWAP( "spab.u18", 0x1400000, 0x400000, CRC(e5a4608d) SHA1(b8e39f53e0b7ad1e16ae9c3726597776b404be1c) )
	ROM_LOAD16_WORD_SWAP( "spcd.u19", 0x1800000, 0x400000, CRC(99bfbe32) SHA1(926a8afc4a175874f22f53300e76f59331d3b9ba) )
	ROM_LOAD16_WORD_SWAP( "spef.u20", 0x1c00000, 0x400000, CRC(c9c799cc) SHA1(01373316700d8688deeea2e9e8f831d5f86c7f17) )

	ROM_REGION( 0x0800000, "tiles_l0", 0 )
	ROM_LOAD32_WORD_SWAP( "bg1012.u22", 0x0000002, 0x400000, CRC(e3fb9af0) SHA1(11900cc2873337692f66fb4f1eb9c574e5a967de) )
	ROM_LOAD32_WORD_SWAP( "bg1113.u23", 0x0000000, 0x400000, CRC(5f8657e6) SHA1(7c2854dc5d2d4efe55bda01e329da051350e0031) )

	ROM_REGION( 0x0800000, "tiles_l1", 0 )
	ROM_LOAD32_WORD_SWAP( "bg2022.u25", 0x0000002, 0x400000, CRC(f46eda52) SHA1(46530016b32a164bd76c4f53e7b53b2beb28db06) )
	ROM_LOAD32_WORD_SWAP( "bg2123.u24", 0x0000000, 0x400000, CRC(c4ebb86b) SHA1(a7093e6e02b64566d277cbbd5fa90cd430e7c8a0) )

	ROM_REGION( 0x200000, "tiles_bg", 0 )
	ROM_LOAD16_WORD_SWAP( "map.u5", 0x00000, 0x200000, CRC(bd179dc5) SHA1(ce3fcac573b14fd5365eb5dcec3257e439d2c129) )

	ROM_REGION( 0x400000, "ymf", 0 ) // OPL4 samples
	ROM_LOAD( "opm.u6", 0x00000, 0x400000, CRC(31b05be4) SHA1(d0f4f387f84a74591224b0f42b7f5c538a3dc498) )
ROM_END

ROM_START( asurabusja )
	ROM_REGION( 0x200000, "maincpu", 0 ) // M68020
	ROM_LOAD32_BYTE( "pgm3_583a.u1", 0x000000, 0x80000, CRC(46ab3b0e) SHA1(2d6a57352891a484fe11cda9addbff5b3940c17c) ) // hand written labels with checksums
	ROM_LOAD32_BYTE( "pgm2_0ff4.u2", 0x000001, 0x80000, CRC(fa7aa289) SHA1(6f82371274c45f889a19a4fdd859015fb6ea249a) )
	ROM_LOAD32_BYTE( "pgm1_bac7.u3", 0x000002, 0x80000, CRC(67364e19) SHA1(959b896b201f103ef9189b537139c89bfc7144ea) )
	ROM_LOAD32_BYTE( "pgm0_193a.u4", 0x000003, 0x80000, CRC(94d39c64) SHA1(95ca2aa3e19e64bed7add3170653fa3364530fde) )

	ROM_REGION( 0x80000, "soundcpu", 0 ) // Z80
	ROM_LOAD( "srom.u7", 0x00000, 0x80000, CRC(368da389) SHA1(1423b709da40bf3033c9032c4bd07658f1a969de) )

	ROM_REGION( 0x2000000, "fuukispr", 0 )
	ROM_LOAD16_WORD_SWAP( "sp01.u13", 0x0000000, 0x400000, CRC(5edea463) SHA1(22a780912f060bae0c9a403a7bfd4d27f25b76e3) )
	ROM_LOAD16_WORD_SWAP( "sp23.u14", 0x0400000, 0x400000, CRC(91b1b0de) SHA1(341367966559ef2027415b673eb0db704680c81f) )
	ROM_LOAD16_WORD_SWAP( "sp45.u15", 0x0800000, 0x400000, CRC(96c69aac) SHA1(cf053523026651427f884b9dd7c095af362dd24e) )
	ROM_LOAD16_WORD_SWAP( "sp67.u16", 0x0c00000, 0x400000, CRC(7c3d83bf) SHA1(7188dd923c6c7eb6aee3323e7ab54aa240c35ea3) )
	ROM_LOAD16_WORD_SWAP( "sp89.u17", 0x1000000, 0x400000, CRC(cb1e14f8) SHA1(941cea1887d7ceb52222adcf1d6913969e6163aa) )
	ROM_LOAD16_WORD_SWAP( "spab.u18", 0x1400000, 0x400000, CRC(e5a4608d) SHA1(b8e39f53e0b7ad1e16ae9c3726597776b404be1c) )
	ROM_LOAD16_WORD_SWAP( "spcd.u19", 0x1800000, 0x400000, CRC(99bfbe32) SHA1(926a8afc4a175874f22f53300e76f59331d3b9ba) )
	ROM_LOAD16_WORD_SWAP( "spef.u20", 0x1c00000, 0x400000, CRC(c9c799cc) SHA1(01373316700d8688deeea2e9e8f831d5f86c7f17) )

	ROM_REGION( 0x0800000, "tiles_l0", 0 )
	ROM_LOAD32_WORD_SWAP( "bg1012.u22", 0x0000002, 0x400000, CRC(e3fb9af0) SHA1(11900cc2873337692f66fb4f1eb9c574e5a967de) )
	ROM_LOAD32_WORD_SWAP( "bg1113.u23", 0x0000000, 0x400000, CRC(5f8657e6) SHA1(7c2854dc5d2d4efe55bda01e329da051350e0031) )

	ROM_REGION( 0x0800000, "tiles_l1", 0 )
	ROM_LOAD32_WORD_SWAP( "bg2022.u25", 0x0000002, 0x400000, CRC(f46eda52) SHA1(46530016b32a164bd76c4f53e7b53b2beb28db06) )
	ROM_LOAD32_WORD_SWAP( "bg2123.u24", 0x0000000, 0x400000, CRC(c4ebb86b) SHA1(a7093e6e02b64566d277cbbd5fa90cd430e7c8a0) )

	ROM_REGION( 0x200000, "tiles_bg", 0 )
	ROM_LOAD16_WORD_SWAP( "map.u5", 0x00000, 0x200000, CRC(bd179dc5) SHA1(ce3fcac573b14fd5365eb5dcec3257e439d2c129) )

	ROM_REGION( 0x400000, "ymf", 0 ) // OPL4 samples
	ROM_LOAD( "opm.u6", 0x00000, 0x400000, CRC(31b05be4) SHA1(d0f4f387f84a74591224b0f42b7f5c538a3dc498) )
ROM_END

ROM_START( asurabusjr ) // ARCADIA review build
	ROM_REGION( 0x200000, "maincpu", 0 ) // M68020
	ROM_LOAD32_BYTE( "24-31.pgm3", 0x000000, 0x80000, CRC(cfcb9c75) SHA1(51e325d5e60d5bb058429f04a5170dcc17986b7d) )
	ROM_LOAD32_BYTE( "16-23.pgm2", 0x000001, 0x80000, CRC(e4d07738) SHA1(c6c949c5b0cbc129917bb8c93707539adabbd336) )
	ROM_LOAD32_BYTE( "8-15.pgm1",  0x000002, 0x80000, CRC(1dd67fe7) SHA1(3fd340ccd4a306783ba0ccd3343ae505c9de3a73) )
	ROM_LOAD32_BYTE( "0-7.pgm0",   0x000003, 0x80000, CRC(3af08de3) SHA1(1ecc69804693cab6c2c36120acfc6ced094a16e4) )

	ROM_REGION( 0x80000, "soundcpu", 0 ) // Z80
	ROM_LOAD( "srom.u7", 0x00000, 0x80000, CRC(368da389) SHA1(1423b709da40bf3033c9032c4bd07658f1a969de) )

	ROM_REGION( 0x2000000, "fuukispr", 0 )
	ROM_LOAD16_WORD_SWAP( "sp01.u13", 0x0000000, 0x400000, CRC(5edea463) SHA1(22a780912f060bae0c9a403a7bfd4d27f25b76e3) )
	ROM_LOAD16_WORD_SWAP( "sp23.u14", 0x0400000, 0x400000, CRC(91b1b0de) SHA1(341367966559ef2027415b673eb0db704680c81f) )
	ROM_LOAD16_WORD_SWAP( "sp45.u15", 0x0800000, 0x400000, CRC(96c69aac) SHA1(cf053523026651427f884b9dd7c095af362dd24e) )
	ROM_LOAD16_WORD_SWAP( "sp67.u16", 0x0c00000, 0x400000, CRC(7c3d83bf) SHA1(7188dd923c6c7eb6aee3323e7ab54aa240c35ea3) )
	ROM_LOAD16_WORD_SWAP( "sp89.u17", 0x1000000, 0x400000, CRC(cb1e14f8) SHA1(941cea1887d7ceb52222adcf1d6913969e6163aa) )
	ROM_LOAD16_WORD_SWAP( "spab.u18", 0x1400000, 0x400000, CRC(e5a4608d) SHA1(b8e39f53e0b7ad1e16ae9c3726597776b404be1c) )
	ROM_LOAD16_WORD_SWAP( "spcd.u19", 0x1800000, 0x400000, CRC(99bfbe32) SHA1(926a8afc4a175874f22f53300e76f59331d3b9ba) )
	ROM_LOAD16_WORD_SWAP( "spef.u20", 0x1c00000, 0x400000, CRC(c9c799cc) SHA1(01373316700d8688deeea2e9e8f831d5f86c7f17) )

	ROM_REGION( 0x0800000, "tiles_l0", 0 )
	ROM_LOAD32_WORD_SWAP( "bg1012.u22", 0x0000002, 0x400000, CRC(e3fb9af0) SHA1(11900cc2873337692f66fb4f1eb9c574e5a967de) )
	ROM_LOAD32_WORD_SWAP( "bg1113.u23", 0x0000000, 0x400000, CRC(5f8657e6) SHA1(7c2854dc5d2d4efe55bda01e329da051350e0031) )

	ROM_REGION( 0x0800000, "tiles_l1", 0 )
	ROM_LOAD32_WORD_SWAP( "bg2022.u25", 0x0000002, 0x400000, CRC(f46eda52) SHA1(46530016b32a164bd76c4f53e7b53b2beb28db06) )
	ROM_LOAD32_WORD_SWAP( "bg2123.u24", 0x0000000, 0x400000, CRC(c4ebb86b) SHA1(a7093e6e02b64566d277cbbd5fa90cd430e7c8a0) )

	ROM_REGION( 0x200000, "tiles_bg", 0 )
	ROM_LOAD16_WORD_SWAP( "map.u5", 0x00000, 0x200000, CRC(bd179dc5) SHA1(ce3fcac573b14fd5365eb5dcec3257e439d2c129) )

	ROM_REGION( 0x400000, "ymf", 0 ) // OPL4 samples
	ROM_LOAD( "opm.u6", 0x00000, 0x400000, CRC(31b05be4) SHA1(d0f4f387f84a74591224b0f42b7f5c538a3dc498) )
ROM_END

} // anonymous namespace


//-------------------------------------------------
//  game drivers
//-------------------------------------------------

GAME( 1998, asurabld,   0,        fuuki32, asurabld, fuuki32_state, empty_init, ROT0, "Fuuki", "Asura Blade - Sword of Dynasty (Japan)",                         MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 2001, asurabus,   0,        fuuki32, asurabus, fuuki32_state, empty_init, ROT0, "Fuuki", "Asura Buster - Eternal Warriors (USA)",                          MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2000, asurabusj,  asurabus, fuuki32, asurabus, fuuki32_state, empty_init, ROT0, "Fuuki", "Asura Buster - Eternal Warriors (Japan, set 1)",                 MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2000, asurabusja, asurabus, fuuki32, asurabus, fuuki32_state, empty_init, ROT0, "Fuuki", "Asura Buster - Eternal Warriors (Japan, set 2)",                 MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2000, asurabusjr, asurabus, fuuki32, asurabusa,fuuki32_state, empty_init, ROT0, "Fuuki", "Asura Buster - Eternal Warriors (Japan) (ARCADIA review build)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // has pause function on P1 button 4

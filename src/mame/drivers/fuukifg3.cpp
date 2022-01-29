// license:BSD-3-Clause
// copyright-holders:Paul Priest, David Haywood, Luca Elia
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

English versions exist, but are not dumped

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
#include "includes/fuukifg3.h"

#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/ymopl.h"
#include "speaker.h"


/***************************************************************************


                            Memory Maps - Main CPU


***************************************************************************/

/* Sound comms */
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

u16 fuuki32_state::vregs_r(offs_t offset)
{
	return m_vregs[offset];
}

void fuuki32_state::vregs_w(offs_t offset, u16 data, u16 mem_mask)
{
	const u16 old = m_vregs[offset];
	data = COMBINE_DATA(&m_vregs[offset]);
	if (old != data)
	{
		if (offset == 0x1c / 2)
		{
			const rectangle &visarea = m_screen->visible_area();
			attotime period = m_screen->frame_period();
			m_raster_interrupt_timer->adjust(m_screen->time_until_pos(data, visarea.max_x + 1), 0, period);
		}
		if (offset == 0x1e / 2)
		{
			if ((old ^ data) & 0x40)
				m_tilemap[2]->mark_all_dirty();
		}
	}
}

template<int Layer>
void fuuki32_state::vram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_vram[Layer][offset]);
	m_tilemap[Layer]->mark_tile_dirty(offset);
}

template<int Layer>
void fuuki32_state::vram_buffered_w(offs_t offset, u32 data, u32 mem_mask)
{
	const int buffer = (m_vregs[0x1e / 2] & 0x40) >> 6;
	COMBINE_DATA(&m_vram[Layer][offset]);
	if ((Layer & 1) == buffer)
		m_tilemap[2]->mark_tile_dirty(offset);
}

void fuuki32_state::fuuki32_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();                                                                     // ROM
	map(0x400000, 0x40ffff).ram();                                                                     // Work RAM
	map(0x410000, 0x41ffff).ram();                                                                     // Work RAM (used by asurabus)

	map(0x500000, 0x501fff).ram().w(FUNC(fuuki32_state::vram_w<0>)).share("vram.0");  // Tilemap 1
	map(0x502000, 0x503fff).ram().w(FUNC(fuuki32_state::vram_w<1>)).share("vram.1");  // Tilemap 2
	map(0x504000, 0x505fff).ram().w(FUNC(fuuki32_state::vram_buffered_w<2>)).share("vram.2");  // Tilemap bg
	map(0x506000, 0x507fff).ram().w(FUNC(fuuki32_state::vram_buffered_w<3>)).share("vram.3");  // Tilemap bg2
	map(0x508000, 0x517fff).ram();                                                                     // More tilemap, or linescroll? Seems to be empty all of the time
	map(0x600000, 0x601fff).rw(FUNC(fuuki32_state::sprram_r), FUNC(fuuki32_state::sprram_w)); // Sprites
	map(0x700000, 0x703fff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette"); // Palette

	map(0x800000, 0x800003).lr16(NAME([this] () { return u16(m_system->read()); })).nopw();  // Coin
	map(0x810000, 0x810003).lr16(NAME([this] () { return u16(m_inputs->read()); })).nopw();  // Player Inputs
	map(0x880000, 0x880003).lr16(NAME([this] () { return u16(m_dsw1->read()); }));           // Service + DIPS
	map(0x890000, 0x890003).lr16(NAME([this] () { return u16(m_dsw2->read()); }));           // More DIPS

	map(0x8c0000, 0x8c001f).rw(FUNC(fuuki32_state::vregs_r), FUNC(fuuki32_state::vregs_w));        // Video Registers
	map(0x8d0000, 0x8d0003).ram();                                                                     // Flipscreen Related
	map(0x8e0000, 0x8e0003).ram().share("priority");                            // Controls layer order
	map(0x903fe0, 0x903fff).rw(FUNC(fuuki32_state::snd_020_r), FUNC(fuuki32_state::snd_020_w)).umask32(0x00ff00ff);                                         // Shared with Z80
	map(0xa00000, 0xa00003).writeonly().share("tilebank");                      // Tilebank
}


/***************************************************************************

                            Memory Maps - Sound CPU

***************************************************************************/

void fuuki32_state::sound_bw_w(u8 data)
{
	m_soundbank->set_entry(data);
}

void fuuki32_state::fuuki32_sound_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();                             // ROM
	map(0x6000, 0x6fff).ram();                             // RAM
	map(0x7ff0, 0x7fff).ram().share("shared_ram");
	map(0x8000, 0xffff).bankr("soundbank");                // ROM
}

void fuuki32_state::fuuki32_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(fuuki32_state::sound_bw_w));
	map(0x30, 0x30).nopw(); // leftover/unused nmi handler related
	map(0x40, 0x45).rw("ymf", FUNC(ymf278b_device::read), FUNC(ymf278b_device::write));
}

/***************************************************************************


                                Input Ports


***************************************************************************/

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
	PORT_DIPSETTING(      0x0000, "Both Off" )              /* Duplicate setting */
	PORT_DIPNAME( 0x0030, 0x0030, "Timer" )                 PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0000, "Slow" )
	PORT_DIPSETTING(      0x0030, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0010, "Fast" )
	PORT_DIPSETTING(      0x0020, "Very Fast" )
	PORT_DIPNAME( 0x00c0, 0x0000, "Coinage Mode" )          PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x00c0, "Split" )
	PORT_DIPSETTING(      0x0000, "Joint" )
	PORT_DIPUNUSED_DIPLOC( 0x0100, 0x0100, "SW2:1" )        /* DSW2 bank, not used for either game */
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
	PORT_DIPNAME( 0x00c0, 0x00c0, "Max Rounds" )            PORT_DIPLOCATION("SW3:7,8") /* Service Mode shows rounds needed to win the match */
	PORT_DIPSETTING(      0x0000, "1" )                     /* Service Mode Shows 1 */
	PORT_DIPSETTING(      0x00c0, "3" )                     /* Service Mode Shows 3, Service Mode has 2 & 3 reversed compared to game play */
	PORT_DIPSETTING(      0x0080, "5" )                     /* Service Mode Shows 2, Service Mode has 2 & 3 reversed compared to game play */
//  PORT_DIPSETTING(      0x0040, "Error!!" )               /* Service Mode Shows "Error" */
	PORT_DIPNAME( 0xf000, 0xf000, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW4:1,2,3,4") /* Service Mode Shows Player 2 */
	PORT_DIPSETTING(      0x8000, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(      0x9000, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0xb000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0xd000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0xf000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x2000, "2C Start / 1C Continue" )
//  PORT_DIPSETTING(      0x7000, "Error!!" )               // Causes graphics issues - Service Mode shows "Error"
//  PORT_DIPSETTING(      0x1000, DEF_STR( 2C_1C ) )        // Duplicate 2C_1C
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW2",0x0f00,NOTEQUALS,0x0000)
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )        PORT_CONDITION("DSW2",0x0f00,EQUALS,0x0000) // Set both for Free Play
	PORT_DIPNAME( 0x0f00, 0x0f00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW4:5,6,7,8") /* Service Mode Shows Player 1 */
	PORT_DIPSETTING(      0x0800, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(      0x0900, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x0b00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0d00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0f00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0200, "2C Start / 1C Continue" )
//  PORT_DIPSETTING(      0x0700, "Error!!" )               // Causes graphics issues - Service Mode shows "Error"
//  PORT_DIPSETTING(      0x0100, DEF_STR( 2C_1C ) )        // Duplicate 2C_1C
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW2",0xf000,NOTEQUALS,0x0000)
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )        PORT_CONDITION("DSW2",0xf000,EQUALS,0x0000) // Set both for Free Play
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

/***************************************************************************


                            Graphics Layouts


***************************************************************************/

/* 8x8x4 */
static const gfx_layout layout_8x8x4 =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ STEP8(0,4) },
	{ STEP8(0,8*4) },
	8*8*4
};

/* 16x16x8 */
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
	GFXDECODE_ENTRY( "gfx2", 0, layout_16x16x8, 0x400*0, 0x40 ) // [0] Layer 1
	GFXDECODE_ENTRY( "gfx3", 0, layout_16x16x8, 0x400*1, 0x40 ) // [1] Layer 2
	GFXDECODE_ENTRY( "gfx4", 0, layout_8x8x4,   0x400*3, 0x40 ) // [2] BG Layer
GFXDECODE_END


/***************************************************************************


                                Machine Drivers


***************************************************************************/

void fuuki32_state::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
	case TIMER_LEVEL_1_INTERRUPT:
		m_maincpu->set_input_line(1, HOLD_LINE);
		m_level_1_interrupt_timer->adjust(m_screen->time_until_pos(248));
		break;
	case TIMER_VBLANK_INTERRUPT:
		m_maincpu->set_input_line(3, HOLD_LINE);    // VBlank IRQ
		m_vblank_interrupt_timer->adjust(m_screen->time_until_vblank_start());
		break;
	case TIMER_RASTER_INTERRUPT:
		m_maincpu->set_input_line(5, HOLD_LINE);    // Raster Line IRQ
		m_screen->update_partial(m_screen->vpos());
		m_raster_interrupt_timer->adjust(m_screen->frame_period());
		break;
	default:
		throw emu_fatalerror("Unknown id in fuuki32_state::device_timer");
	}
}


void fuuki32_state::machine_start()
{
	u8 *ROM = memregion("soundcpu")->base();

	m_soundbank->configure_entries(0, 0x10, &ROM[0], 0x8000);

	m_level_1_interrupt_timer = timer_alloc(TIMER_LEVEL_1_INTERRUPT);
	m_vblank_interrupt_timer = timer_alloc(TIMER_VBLANK_INTERRUPT);
	m_raster_interrupt_timer = timer_alloc(TIMER_RASTER_INTERRUPT);

	save_item(NAME(m_spr_buffered_tilebank));
}


void fuuki32_state::machine_reset()
{
	const rectangle &visarea = m_screen->visible_area();

	m_level_1_interrupt_timer->adjust(m_screen->time_until_pos(248));
	m_vblank_interrupt_timer->adjust(m_screen->time_until_vblank_start());
	m_raster_interrupt_timer->adjust(m_screen->time_until_pos(0, visarea.max_x + 1));
}

void fuuki32_state::fuuki32(machine_config &config)
{
	/* basic machine hardware */
	M68EC020(config, m_maincpu, CPU_CLOCK); /* 20MHz verified */
	m_maincpu->set_addrmap(AS_PROGRAM, &fuuki32_state::fuuki32_map);

	z80_device &soundcpu(Z80(config, "soundcpu", SOUND_CPU_CLOCK)); /* 6MHz verified */
	soundcpu.set_addrmap(AS_PROGRAM, &fuuki32_state::fuuki32_sound_map);
	soundcpu.set_addrmap(AS_IO, &fuuki32_state::fuuki32_sound_io_map);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(64 * 8, 32 * 8);
	m_screen->set_visarea(0, 40 * 8 - 1, 0, 30 * 8 - 1);
	m_screen->set_screen_update(FUNC(fuuki32_state::screen_update));
	m_screen->screen_vblank().set(FUNC(fuuki32_state::screen_vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_fuuki32);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x4000 / 2);

	FUUKI_VIDEO(config, m_fuukivid, 0);
	m_fuukivid->set_palette(m_palette);
	m_fuukivid->set_color_base(0x400*2);
	m_fuukivid->set_color_num(0x40);
	m_fuukivid->set_tile_callback(FUNC(fuuki32_state::fuuki32_tile_cb));
	m_fuukivid->set_colpri_callback(FUNC(fuuki32_state::fuuki32_colpri_cb));

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ymf278b_device &ymf(YMF278B(config, "ymf", 33.8688_MHz_XTAL));
	ymf.irq_handler().set_inputline("soundcpu", 0);
	ymf.add_route(0, "lspeaker", 0.50);
	ymf.add_route(1, "rspeaker", 0.50);
	ymf.add_route(2, "lspeaker", 0.40);
	ymf.add_route(3, "rspeaker", 0.40);
	ymf.add_route(4, "lspeaker", 0.50);
	ymf.add_route(5, "rspeaker", 0.50);
}

/***************************************************************************


                                ROMs Loading


***************************************************************************/

/***************************************************************************

                 Asura Blade - Sword of Dynasty (Japan)

Fuuki, 1999   Consists of a FG-3J MAIN-J mainboard &  FG-3J ROM-J combo

***************************************************************************/

ROM_START( asurabld )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* M68020 */
	ROM_LOAD32_BYTE( "pgm3.u1", 0x000000, 0x80000, CRC(053e9758) SHA1(c2754d3f0c607c81c8fa33b667b576eb0474fd0b) )
	ROM_LOAD32_BYTE( "pgm2.u2", 0x000001, 0x80000, CRC(16b656ca) SHA1(5ffb551ce7dec462d3896f0fed693454496894bc) )
	ROM_LOAD32_BYTE( "pgm1.u3", 0x000002, 0x80000, CRC(35104452) SHA1(03cfd81429f8a945d5419c9750925bfa997d0607) )
	ROM_LOAD32_BYTE( "pgm0.u4", 0x000003, 0x80000, CRC(68615497) SHA1(de93751f151f195a863dc6fe83b6e7ed8f99430a) )

	ROM_REGION( 0x80000, "soundcpu", 0 ) /* Z80 */
	ROM_LOAD( "srom.u7", 0x00000, 0x80000, CRC(bb1deb89) SHA1(b1c70abddc0b9a88beb69a592376ff69a7e091eb) )

	ROM_REGION( 0x2000000, "fuukivid", 0 )
	/* 0x0000000 - 0x03fffff empty */ /* spXX.uYY - XX is the bank number! */
	ROM_LOAD16_WORD_SWAP( "sp23.u14", 0x0400000, 0x400000, CRC(7df492eb) SHA1(30b88a3cd025ffc8c28fef06e0784755be37ef8e) )
	ROM_LOAD16_WORD_SWAP( "sp45.u15", 0x0800000, 0x400000, CRC(1890f42a) SHA1(22254fe38fd83f4602a25e1ccba32df16edaf3f9) )
	ROM_LOAD16_WORD_SWAP( "sp67.u16", 0x0c00000, 0x400000, CRC(a48f1ef0) SHA1(bf8787f293793291a503af662d3738c007654726) )
	ROM_LOAD16_WORD_SWAP( "sp89.u17", 0x1000000, 0x400000, CRC(6b024362) SHA1(8be5cc3c7306d28b75acd970bb3be6d3c9825367) )
	ROM_LOAD16_WORD_SWAP( "spab.u18", 0x1400000, 0x400000, CRC(803d2d8c) SHA1(25df30689e576a0620656c721d92bcc3fbd84844) )
	ROM_LOAD16_WORD_SWAP( "spcd.u19", 0x1800000, 0x400000, CRC(42e5c26e) SHA1(b68875d353bdc5d49113bbac02fd83508bce66a5) )

	ROM_REGION( 0x0800000, "gfx2", 0 )
	ROM_LOAD32_WORD_SWAP( "bg1012.u22", 0x0000002, 0x400000, CRC(d717a0a1) SHA1(007df309dc0650ca07e077b983a2b05730349d0b) )
	ROM_LOAD32_WORD_SWAP( "bg1113.u23", 0x0000000, 0x400000, CRC(94338267) SHA1(7848bc57cb0eac216100a508763451eb57a0a082) )

	ROM_REGION( 0x0800000, "gfx3", 0 )
	ROM_LOAD32_WORD_SWAP( "bg2022.u25", 0x0000002, 0x400000, CRC(ee312cd3) SHA1(2ef9d51928d80375daf8e6b204bb66a8b9cbaee7) )
	ROM_LOAD32_WORD_SWAP( "bg2123.u24", 0x0000000, 0x400000, CRC(4acfc469) SHA1(a98d06b967ebb3fa3b4c8aa3d7a05063ec981fb2) )

	ROM_REGION( 0x200000, "gfx4", 0 ) // background tiles
	ROM_LOAD16_WORD_SWAP( "map.u5", 0x00000, 0x200000, CRC(e681155e) SHA1(458845b9c86df72685d92d0d4052aacc2fa7d1bd) )

	ROM_REGION( 0x400000, "ymf", 0 ) // OPL4 samples
	ROM_LOAD( "pcm.u6", 0x00000, 0x400000, CRC(ac72225a) SHA1(8d16399ed34ac5bd69dbf43b2de2b0db9ac1c610) )
ROM_END

/***************************************************************************

                 Asura Buster - Eternal Warriors (Japan)

Fuuki, 2000   Consists of a FG-3J MAIN-J mainboard &  FG-3J ROM-J combo

***************************************************************************/

ROM_START( asurabus )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* M68020 */
	ROM_LOAD32_BYTE( "uspgm3.u1", 0x000000, 0x80000, CRC(e152cec9) SHA1(af1d93bdabc6732c0ff53972826d67a3753ef785) ) // hand written labels
	ROM_LOAD32_BYTE( "uspgm2.u2", 0x000001, 0x80000, CRC(b19787db) SHA1(3d6757f38f297c1ee89003173567319b5dae8000) )
	ROM_LOAD32_BYTE( "uspgm1.u3", 0x000002, 0x80000, CRC(6588e51a) SHA1(9ab978c80d8ece447697557c8000be95760306f3) )
	ROM_LOAD32_BYTE( "uspgm0.u4", 0x000003, 0x80000, CRC(981e6ff1) SHA1(088d26a3cbd2361ffc756c3da8a67b94ae7bbd65) )

	ROM_REGION( 0x80000, "soundcpu", 0 ) /* Z80 */
	ROM_LOAD( "srom.u7", 0x00000, 0x80000, CRC(368da389) SHA1(1423b709da40bf3033c9032c4bd07658f1a969de) )

	ROM_REGION( 0x2000000, "fuukivid", 0 )
	ROM_LOAD16_WORD_SWAP( "sp01.u13", 0x0000000, 0x400000, CRC(5edea463) SHA1(22a780912f060bae0c9a403a7bfd4d27f25b76e3) )
	ROM_LOAD16_WORD_SWAP( "sp23.u14", 0x0400000, 0x400000, CRC(91b1b0de) SHA1(341367966559ef2027415b673eb0db704680c81f) )
	ROM_LOAD16_WORD_SWAP( "sp45.u15", 0x0800000, 0x400000, CRC(96c69aac) SHA1(cf053523026651427f884b9dd7c095af362dd24e) )
	ROM_LOAD16_WORD_SWAP( "sp67.u16", 0x0c00000, 0x400000, CRC(7c3d83bf) SHA1(7188dd923c6c7eb6aee3323e7ab54aa240c35ea3) )
	ROM_LOAD16_WORD_SWAP( "sp89.u17", 0x1000000, 0x400000, CRC(cb1e14f8) SHA1(941cea1887d7ceb52222adcf1d6913969e6163aa) )
	ROM_LOAD16_WORD_SWAP( "spab.u18", 0x1400000, 0x400000, CRC(e5a4608d) SHA1(b8e39f53e0b7ad1e16ae9c3726597776b404be1c) )
	ROM_LOAD16_WORD_SWAP( "spcd.u19", 0x1800000, 0x400000, CRC(99bfbe32) SHA1(926a8afc4a175874f22f53300e76f59331d3b9ba) )
	ROM_LOAD16_WORD_SWAP( "spef.u20", 0x1c00000, 0x400000, CRC(c9c799cc) SHA1(01373316700d8688deeea2e9e8f831d5f86c7f17) )

	ROM_REGION( 0x0800000, "gfx2", 0 )
	ROM_LOAD32_WORD_SWAP( "bg1012.u22", 0x0000002, 0x400000, CRC(e3fb9af0) SHA1(11900cc2873337692f66fb4f1eb9c574e5a967de) )
	ROM_LOAD32_WORD_SWAP( "bg1113.u23", 0x0000000, 0x400000, CRC(5f8657e6) SHA1(7c2854dc5d2d4efe55bda01e329da051350e0031) )

	ROM_REGION( 0x0800000, "gfx3", 0 )
	ROM_LOAD32_WORD_SWAP( "bg2022.u25", 0x0000002, 0x400000, CRC(f46eda52) SHA1(46530016b32a164bd76c4f53e7b53b2beb28db06) )
	ROM_LOAD32_WORD_SWAP( "bg2123.u24", 0x0000000, 0x400000, CRC(c4ebb86b) SHA1(a7093e6e02b64566d277cbbd5fa90cd430e7c8a0) )

	ROM_REGION( 0x200000, "gfx4", 0 ) // background tiles
	ROM_LOAD16_WORD_SWAP( "map.u5", 0x00000, 0x200000, CRC(bd179dc5) SHA1(ce3fcac573b14fd5365eb5dcec3257e439d2c129) )

	ROM_REGION( 0x400000, "ymf", 0 ) // OPL4 samples
	ROM_LOAD( "opm.u6", 0x00000, 0x400000, CRC(31b05be4) SHA1(d0f4f387f84a74591224b0f42b7f5c538a3dc498) )
ROM_END

ROM_START( asurabusj )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* M68020 */
	ROM_LOAD32_BYTE( "pgm3.u1", 0x000000, 0x80000, CRC(2c6b5271) SHA1(188371f1f003823ac719e962e048719d76696b2f) )
	ROM_LOAD32_BYTE( "pgm2.u2", 0x000001, 0x80000, CRC(8f8694ec) SHA1(3334df4aecc5ab2f8914ef6748c027a99b39ce26) )
	ROM_LOAD32_BYTE( "pgm1.u3", 0x000002, 0x80000, CRC(0a040f0f) SHA1(d5e86d33efcbbde7ee62cfc8dfe867f250a33415) )
	ROM_LOAD32_BYTE( "pgm0.u4", 0x000003, 0x80000, CRC(9b71e9d8) SHA1(9b705b5b6fff549f5679890422b481b5cf1d7bd7) )

	ROM_REGION( 0x80000, "soundcpu", 0 ) /* Z80 */
	ROM_LOAD( "srom.u7", 0x00000, 0x80000, CRC(368da389) SHA1(1423b709da40bf3033c9032c4bd07658f1a969de) )

	ROM_REGION( 0x2000000, "fuukivid", 0 )
	ROM_LOAD16_WORD_SWAP( "sp01.u13", 0x0000000, 0x400000, CRC(5edea463) SHA1(22a780912f060bae0c9a403a7bfd4d27f25b76e3) )
	ROM_LOAD16_WORD_SWAP( "sp23.u14", 0x0400000, 0x400000, CRC(91b1b0de) SHA1(341367966559ef2027415b673eb0db704680c81f) )
	ROM_LOAD16_WORD_SWAP( "sp45.u15", 0x0800000, 0x400000, CRC(96c69aac) SHA1(cf053523026651427f884b9dd7c095af362dd24e) )
	ROM_LOAD16_WORD_SWAP( "sp67.u16", 0x0c00000, 0x400000, CRC(7c3d83bf) SHA1(7188dd923c6c7eb6aee3323e7ab54aa240c35ea3) )
	ROM_LOAD16_WORD_SWAP( "sp89.u17", 0x1000000, 0x400000, CRC(cb1e14f8) SHA1(941cea1887d7ceb52222adcf1d6913969e6163aa) )
	ROM_LOAD16_WORD_SWAP( "spab.u18", 0x1400000, 0x400000, CRC(e5a4608d) SHA1(b8e39f53e0b7ad1e16ae9c3726597776b404be1c) )
	ROM_LOAD16_WORD_SWAP( "spcd.u19", 0x1800000, 0x400000, CRC(99bfbe32) SHA1(926a8afc4a175874f22f53300e76f59331d3b9ba) )
	ROM_LOAD16_WORD_SWAP( "spef.u20", 0x1c00000, 0x400000, CRC(c9c799cc) SHA1(01373316700d8688deeea2e9e8f831d5f86c7f17) )

	ROM_REGION( 0x0800000, "gfx2", 0 )
	ROM_LOAD32_WORD_SWAP( "bg1012.u22", 0x0000002, 0x400000, CRC(e3fb9af0) SHA1(11900cc2873337692f66fb4f1eb9c574e5a967de) )
	ROM_LOAD32_WORD_SWAP( "bg1113.u23", 0x0000000, 0x400000, CRC(5f8657e6) SHA1(7c2854dc5d2d4efe55bda01e329da051350e0031) )

	ROM_REGION( 0x0800000, "gfx3", 0 )
	ROM_LOAD32_WORD_SWAP( "bg2022.u25", 0x0000002, 0x400000, CRC(f46eda52) SHA1(46530016b32a164bd76c4f53e7b53b2beb28db06) )
	ROM_LOAD32_WORD_SWAP( "bg2123.u24", 0x0000000, 0x400000, CRC(c4ebb86b) SHA1(a7093e6e02b64566d277cbbd5fa90cd430e7c8a0) )

	ROM_REGION( 0x200000, "gfx4", 0 ) // background tiles
	ROM_LOAD16_WORD_SWAP( "map.u5", 0x00000, 0x200000, CRC(bd179dc5) SHA1(ce3fcac573b14fd5365eb5dcec3257e439d2c129) )

	ROM_REGION( 0x400000, "ymf", 0 ) // OPL4 samples
	ROM_LOAD( "opm.u6", 0x00000, 0x400000, CRC(31b05be4) SHA1(d0f4f387f84a74591224b0f42b7f5c538a3dc498) )
ROM_END

ROM_START( asurabusja )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* M68020 */
	ROM_LOAD32_BYTE( "pgm3_583a.u1", 0x000000, 0x80000, CRC(46ab3b0e) SHA1(2d6a57352891a484fe11cda9addbff5b3940c17c) ) // hand written labels with checksums
	ROM_LOAD32_BYTE( "pgm2_0ff4.u2", 0x000001, 0x80000, CRC(fa7aa289) SHA1(6f82371274c45f889a19a4fdd859015fb6ea249a) )
	ROM_LOAD32_BYTE( "pgm1_bac7.u3", 0x000002, 0x80000, CRC(67364e19) SHA1(959b896b201f103ef9189b537139c89bfc7144ea) )
	ROM_LOAD32_BYTE( "pgm0_193a.u4", 0x000003, 0x80000, CRC(94d39c64) SHA1(95ca2aa3e19e64bed7add3170653fa3364530fde) )

	ROM_REGION( 0x80000, "soundcpu", 0 ) /* Z80 */
	ROM_LOAD( "srom.u7", 0x00000, 0x80000, CRC(368da389) SHA1(1423b709da40bf3033c9032c4bd07658f1a969de) )

	ROM_REGION( 0x2000000, "fuukivid", 0 )
	ROM_LOAD16_WORD_SWAP( "sp01.u13", 0x0000000, 0x400000, CRC(5edea463) SHA1(22a780912f060bae0c9a403a7bfd4d27f25b76e3) )
	ROM_LOAD16_WORD_SWAP( "sp23.u14", 0x0400000, 0x400000, CRC(91b1b0de) SHA1(341367966559ef2027415b673eb0db704680c81f) )
	ROM_LOAD16_WORD_SWAP( "sp45.u15", 0x0800000, 0x400000, CRC(96c69aac) SHA1(cf053523026651427f884b9dd7c095af362dd24e) )
	ROM_LOAD16_WORD_SWAP( "sp67.u16", 0x0c00000, 0x400000, CRC(7c3d83bf) SHA1(7188dd923c6c7eb6aee3323e7ab54aa240c35ea3) )
	ROM_LOAD16_WORD_SWAP( "sp89.u17", 0x1000000, 0x400000, CRC(cb1e14f8) SHA1(941cea1887d7ceb52222adcf1d6913969e6163aa) )
	ROM_LOAD16_WORD_SWAP( "spab.u18", 0x1400000, 0x400000, CRC(e5a4608d) SHA1(b8e39f53e0b7ad1e16ae9c3726597776b404be1c) )
	ROM_LOAD16_WORD_SWAP( "spcd.u19", 0x1800000, 0x400000, CRC(99bfbe32) SHA1(926a8afc4a175874f22f53300e76f59331d3b9ba) )
	ROM_LOAD16_WORD_SWAP( "spef.u20", 0x1c00000, 0x400000, CRC(c9c799cc) SHA1(01373316700d8688deeea2e9e8f831d5f86c7f17) )

	ROM_REGION( 0x0800000, "gfx2", 0 )
	ROM_LOAD32_WORD_SWAP( "bg1012.u22", 0x0000002, 0x400000, CRC(e3fb9af0) SHA1(11900cc2873337692f66fb4f1eb9c574e5a967de) )
	ROM_LOAD32_WORD_SWAP( "bg1113.u23", 0x0000000, 0x400000, CRC(5f8657e6) SHA1(7c2854dc5d2d4efe55bda01e329da051350e0031) )

	ROM_REGION( 0x0800000, "gfx3", 0 )
	ROM_LOAD32_WORD_SWAP( "bg2022.u25", 0x0000002, 0x400000, CRC(f46eda52) SHA1(46530016b32a164bd76c4f53e7b53b2beb28db06) )
	ROM_LOAD32_WORD_SWAP( "bg2123.u24", 0x0000000, 0x400000, CRC(c4ebb86b) SHA1(a7093e6e02b64566d277cbbd5fa90cd430e7c8a0) )

	ROM_REGION( 0x200000, "gfx4", 0 ) // background tiles
	ROM_LOAD16_WORD_SWAP( "map.u5", 0x00000, 0x200000, CRC(bd179dc5) SHA1(ce3fcac573b14fd5365eb5dcec3257e439d2c129) )

	ROM_REGION( 0x400000, "ymf", 0 ) // OPL4 samples
	ROM_LOAD( "opm.u6", 0x00000, 0x400000, CRC(31b05be4) SHA1(d0f4f387f84a74591224b0f42b7f5c538a3dc498) )
ROM_END

ROM_START( asurabusjr ) // ARCADIA review build
	ROM_REGION( 0x200000, "maincpu", 0 ) /* M68020 */
	ROM_LOAD32_BYTE( "24-31.pgm3", 0x000000, 0x80000, CRC(cfcb9c75) SHA1(51e325d5e60d5bb058429f04a5170dcc17986b7d) )
	ROM_LOAD32_BYTE( "16-23.pgm2", 0x000001, 0x80000, CRC(e4d07738) SHA1(c6c949c5b0cbc129917bb8c93707539adabbd336) )
	ROM_LOAD32_BYTE( "8-15.pgm1",  0x000002, 0x80000, CRC(1dd67fe7) SHA1(3fd340ccd4a306783ba0ccd3343ae505c9de3a73) )
	ROM_LOAD32_BYTE( "0-7.pgm0",   0x000003, 0x80000, CRC(3af08de3) SHA1(1ecc69804693cab6c2c36120acfc6ced094a16e4) )

	ROM_REGION( 0x80000, "soundcpu", 0 ) /* Z80 */
	ROM_LOAD( "srom.u7", 0x00000, 0x80000, CRC(368da389) SHA1(1423b709da40bf3033c9032c4bd07658f1a969de) )

	ROM_REGION( 0x2000000, "fuukivid", 0 )
	ROM_LOAD16_WORD_SWAP( "sp01.u13", 0x0000000, 0x400000, CRC(5edea463) SHA1(22a780912f060bae0c9a403a7bfd4d27f25b76e3) )
	ROM_LOAD16_WORD_SWAP( "sp23.u14", 0x0400000, 0x400000, CRC(91b1b0de) SHA1(341367966559ef2027415b673eb0db704680c81f) )
	ROM_LOAD16_WORD_SWAP( "sp45.u15", 0x0800000, 0x400000, CRC(96c69aac) SHA1(cf053523026651427f884b9dd7c095af362dd24e) )
	ROM_LOAD16_WORD_SWAP( "sp67.u16", 0x0c00000, 0x400000, CRC(7c3d83bf) SHA1(7188dd923c6c7eb6aee3323e7ab54aa240c35ea3) )
	ROM_LOAD16_WORD_SWAP( "sp89.u17", 0x1000000, 0x400000, CRC(cb1e14f8) SHA1(941cea1887d7ceb52222adcf1d6913969e6163aa) )
	ROM_LOAD16_WORD_SWAP( "spab.u18", 0x1400000, 0x400000, CRC(e5a4608d) SHA1(b8e39f53e0b7ad1e16ae9c3726597776b404be1c) )
	ROM_LOAD16_WORD_SWAP( "spcd.u19", 0x1800000, 0x400000, CRC(99bfbe32) SHA1(926a8afc4a175874f22f53300e76f59331d3b9ba) )
	ROM_LOAD16_WORD_SWAP( "spef.u20", 0x1c00000, 0x400000, CRC(c9c799cc) SHA1(01373316700d8688deeea2e9e8f831d5f86c7f17) )

	ROM_REGION( 0x0800000, "gfx2", 0 )
	ROM_LOAD32_WORD_SWAP( "bg1012.u22", 0x0000002, 0x400000, CRC(e3fb9af0) SHA1(11900cc2873337692f66fb4f1eb9c574e5a967de) )
	ROM_LOAD32_WORD_SWAP( "bg1113.u23", 0x0000000, 0x400000, CRC(5f8657e6) SHA1(7c2854dc5d2d4efe55bda01e329da051350e0031) )

	ROM_REGION( 0x0800000, "gfx3", 0 )
	ROM_LOAD32_WORD_SWAP( "bg2022.u25", 0x0000002, 0x400000, CRC(f46eda52) SHA1(46530016b32a164bd76c4f53e7b53b2beb28db06) )
	ROM_LOAD32_WORD_SWAP( "bg2123.u24", 0x0000000, 0x400000, CRC(c4ebb86b) SHA1(a7093e6e02b64566d277cbbd5fa90cd430e7c8a0) )

	ROM_REGION( 0x200000, "gfx4", 0 ) // background tiles
	ROM_LOAD16_WORD_SWAP( "map.u5", 0x00000, 0x200000, CRC(bd179dc5) SHA1(ce3fcac573b14fd5365eb5dcec3257e439d2c129) )

	ROM_REGION( 0x400000, "ymf", 0 ) // OPL4 samples
	ROM_LOAD( "opm.u6", 0x00000, 0x400000, CRC(31b05be4) SHA1(d0f4f387f84a74591224b0f42b7f5c538a3dc498) )
ROM_END

/***************************************************************************


                                Game Drivers


***************************************************************************/

GAME( 1998, asurabld,   0,        fuuki32, asurabld, fuuki32_state, empty_init, ROT0, "Fuuki", "Asura Blade - Sword of Dynasty (Japan)",                         MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 2001, asurabus,   0,        fuuki32, asurabus, fuuki32_state, empty_init, ROT0, "Fuuki", "Asura Buster - Eternal Warriors (USA)",                          MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2000, asurabusj,  asurabus, fuuki32, asurabus, fuuki32_state, empty_init, ROT0, "Fuuki", "Asura Buster - Eternal Warriors (Japan, set 1)",                 MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2000, asurabusja, asurabus, fuuki32, asurabus, fuuki32_state, empty_init, ROT0, "Fuuki", "Asura Buster - Eternal Warriors (Japan, set 2)",                 MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2000, asurabusjr, asurabus, fuuki32, asurabusa,fuuki32_state, empty_init, ROT0, "Fuuki", "Asura Buster - Eternal Warriors (Japan) (ARCADIA review build)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // has pause function on P1 button 4

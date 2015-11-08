// license:BSD-3-Clause
// copyright-holders:Paul Priest, David Haywood, Luca Elia
/***************************************************************************

                          -= Fuuki 32 Bit Games (FG-3) =-

                driver by Paul Priest and David Haywood
                based on fuukifg2 by Luca Elia

Hardware is similar to FG-2 used for :
"Go Go! Mile Smile", "Susume! Mile Smile (Japan)" & "Gyakuten!! Puzzle Bancho (Japan)"
See fuukifg2.c

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

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/ymf278b.h"
#include "includes/fuukifg3.h"


/***************************************************************************


                            Memory Maps - Main CPU


***************************************************************************/

/* Sound comms */
READ32_MEMBER(fuuki32_state::snd_020_r)
{
	machine().scheduler().synchronize();
	UINT32 retdata = m_shared_ram[offset * 2] << 16 | m_shared_ram[(offset * 2) + 1];
	return retdata;
}

WRITE32_MEMBER(fuuki32_state::snd_020_w)
{
	machine().scheduler().synchronize();

	if (ACCESSING_BITS_16_23)
		m_shared_ram[offset * 2] = data >> 16;

	if (ACCESSING_BITS_0_7)
		m_shared_ram[(offset * 2) + 1] = data & 0xff;
}

WRITE32_MEMBER(fuuki32_state::vregs_w)
{
	if (m_vregs[offset] != data)
	{
		COMBINE_DATA(&m_vregs[offset]);
		if (offset == 0x1c / 4)
		{
			const rectangle &visarea = m_screen->visible_area();
			attotime period = m_screen->frame_period();
			m_raster_interrupt_timer->adjust(m_screen->time_until_pos(m_vregs[0x1c / 4] >> 16, visarea.max_x + 1), 0, period);
		}
	}
}

static ADDRESS_MAP_START( fuuki32_map, AS_PROGRAM, 32, fuuki32_state )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM                                                                     // ROM
	AM_RANGE(0x400000, 0x40ffff) AM_RAM                                                                     // Work RAM
	AM_RANGE(0x410000, 0x41ffff) AM_RAM                                                                     // Work RAM (used by asurabus)

	AM_RANGE(0x500000, 0x501fff) AM_RAM_WRITE(vram_0_w) AM_SHARE("vram.0")  // Tilemap 1
	AM_RANGE(0x502000, 0x503fff) AM_RAM_WRITE(vram_1_w) AM_SHARE("vram.1")  // Tilemap 2
	AM_RANGE(0x504000, 0x505fff) AM_RAM_WRITE(vram_2_w) AM_SHARE("vram.2")  // Tilemap bg
	AM_RANGE(0x506000, 0x507fff) AM_RAM_WRITE(vram_3_w) AM_SHARE("vram.3")  // Tilemap bg2
	AM_RANGE(0x508000, 0x517fff) AM_RAM                                                                     // More tilemap, or linescroll? Seems to be empty all of the time
	AM_RANGE(0x600000, 0x601fff) AM_RAM AM_DEVREADWRITE16("fuukivid", fuukivid_device, fuuki_sprram_r, fuuki_sprram_w, 0xffffffff) // Sprites
	AM_RANGE(0x700000, 0x703fff) AM_RAM_DEVWRITE("palette",  palette_device, write) AM_SHARE("palette") // Palette

	AM_RANGE(0x800000, 0x800003) AM_READ_PORT("800000") AM_WRITENOP                                         // Coin
	AM_RANGE(0x810000, 0x810003) AM_READ_PORT("810000") AM_WRITENOP                                         // Player Inputs
	AM_RANGE(0x880000, 0x880003) AM_READ_PORT("880000")                                                     // Service + DIPS
	AM_RANGE(0x890000, 0x890003) AM_READ_PORT("890000")                                                     // More DIPS

	AM_RANGE(0x8c0000, 0x8c001f) AM_RAM_WRITE(vregs_w) AM_SHARE("vregs")        // Video Registers
	AM_RANGE(0x8d0000, 0x8d0003) AM_RAM                                                                     // Flipscreen Related
	AM_RANGE(0x8e0000, 0x8e0003) AM_RAM AM_SHARE("priority")                            // Controls layer order
	AM_RANGE(0x903fe0, 0x903fff) AM_READWRITE(snd_020_r, snd_020_w)                                         // Shared with Z80
	AM_RANGE(0xa00000, 0xa00003) AM_WRITEONLY AM_SHARE("tilebank")                      // Tilebank
ADDRESS_MAP_END


/***************************************************************************

                            Memory Maps - Sound CPU

***************************************************************************/

WRITE8_MEMBER(fuuki32_state::sound_bw_w)
{
	membank("bank1")->set_entry(data);
}

READ8_MEMBER(fuuki32_state::snd_z80_r)
{
	UINT8 retdata = m_shared_ram[offset];
	return retdata;
}

WRITE8_MEMBER(fuuki32_state::snd_z80_w)
{
	m_shared_ram[offset] = data;
}

WRITE8_MEMBER(fuuki32_state::snd_ymf278b_w)
{
	machine().device<ymf278b_device>("ymf1")->write(space, offset, data);
}

static ADDRESS_MAP_START( fuuki32_sound_map, AS_PROGRAM, 8, fuuki32_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM                             // ROM
	AM_RANGE(0x6000, 0x6fff) AM_RAM                             // RAM
	AM_RANGE(0x7ff0, 0x7fff) AM_READWRITE(snd_z80_r, snd_z80_w)
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank1")                // ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( fuuki32_sound_io_map, AS_IO, 8, fuuki32_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(sound_bw_w)
	AM_RANGE(0x30, 0x30) AM_WRITENOP // leftover/unused nmi handler related
	AM_RANGE(0x40, 0x45) AM_DEVREAD("ymf1", ymf278b_device, read) AM_WRITE(snd_ymf278b_w)
ADDRESS_MAP_END

/***************************************************************************


                                Input Ports


***************************************************************************/

static INPUT_PORTS_START( asurabld )
	PORT_START("800000")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "SYSTEM")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "SYSTEM")

	PORT_START("810000")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "INPUTS")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "INPUTS")

	PORT_START("880000")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "DSW1")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "DSW1")

	PORT_START("890000")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "DSW2")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "DSW2")

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
	{ 2*4,3*4,   0*4,1*4,   6*4,7*4, 4*4,5*4 },
	{ STEP8(0,8*4) },
	8*8*4
};

/* 16x16x4 */
static const gfx_layout layout_16x16x4 =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{   2*4,3*4,   0*4,1*4,   6*4,7*4, 4*4,5*4,
		10*4,11*4, 8*4,9*4, 14*4,15*4, 12*4,13*4    },
	{ STEP16(0,16*4) },
	16*16*4
};

/* 16x16x8 */
static const gfx_layout layout_16x16x8 =
{
	16,16,
	RGN_FRAC(1,2),
	8,
	{ STEP4(RGN_FRAC(1,2),1), STEP4(0,1) },
	{   2*4,3*4,   0*4,1*4,   6*4,7*4, 4*4,5*4,
		10*4,11*4, 8*4,9*4, 14*4,15*4, 12*4,13*4    },
	{ STEP16(0,16*4) },
	16*16*4
};

static GFXDECODE_START( fuuki32 )
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x4, 0x400*2, 0x40 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, layout_16x16x8, 0x400*0, 0x40 ) // [1] Layer 1
	GFXDECODE_ENTRY( "gfx3", 0, layout_16x16x8, 0x400*1, 0x40 ) // [2] Layer 2
	GFXDECODE_ENTRY( "gfx4", 0, layout_8x8x4,   0x400*3, 0x40 ) // [3] BG Layer
	GFXDECODE_ENTRY( "gfx4", 0, layout_8x8x4,   0x400*3, 0x40 ) // [4] BG Layer 2 (GFX4!)
GFXDECODE_END


/***************************************************************************


                                Machine Drivers


***************************************************************************/

void fuuki32_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
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
		assert_always(FALSE, "Unknown id in fuuki32_state::device_timer");
	}
}


void fuuki32_state::machine_start()
{
	UINT8 *ROM = memregion("soundcpu")->base();

	membank("bank1")->configure_entries(0, 0x10, &ROM[0x10000], 0x8000);

	m_level_1_interrupt_timer = timer_alloc(TIMER_LEVEL_1_INTERRUPT);
	m_vblank_interrupt_timer = timer_alloc(TIMER_VBLANK_INTERRUPT);
	m_raster_interrupt_timer = timer_alloc(TIMER_RASTER_INTERRUPT);

	save_item(NAME(m_spr_buffered_tilebank));
	save_item(NAME(m_shared_ram));
}


void fuuki32_state::machine_reset()
{
	const rectangle &visarea = m_screen->visible_area();

	m_level_1_interrupt_timer->adjust(m_screen->time_until_pos(248));
	m_vblank_interrupt_timer->adjust(m_screen->time_until_vblank_start());
	m_raster_interrupt_timer->adjust(m_screen->time_until_pos(0, visarea.max_x + 1));
}


static MACHINE_CONFIG_START(fuuki32, fuuki32_state)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68EC020, CPU_CLOCK) /* 20MHz verified */
	MCFG_CPU_PROGRAM_MAP(fuuki32_map)

	MCFG_CPU_ADD("soundcpu", Z80, SOUND_CPU_CLOCK) /* 6MHz verified */
	MCFG_CPU_PROGRAM_MAP(fuuki32_sound_map)
	MCFG_CPU_IO_MAP(fuuki32_sound_io_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(64 * 8, 32 * 8)
	MCFG_SCREEN_VISIBLE_AREA(0, 40 * 8 - 1, 0, 30 * 8 - 1)
	MCFG_SCREEN_UPDATE_DRIVER(fuuki32_state, screen_update)
	MCFG_SCREEN_VBLANK_DRIVER(fuuki32_state, screen_eof)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", fuuki32)
	MCFG_PALETTE_ADD("palette", 0x4000 / 2)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)

	MCFG_DEVICE_ADD("fuukivid", FUUKI_VIDEO, 0)
	MCFG_FUUKI_VIDEO_GFXDECODE("gfxdecode")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymf1", YMF278B, YMF278B_STD_CLOCK) // 33.8688MHz
	MCFG_YMF278B_IRQ_HANDLER(INPUTLINE("soundcpu", 0))
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.50)
	MCFG_SOUND_ROUTE(2, "lspeaker", 0.40)
	MCFG_SOUND_ROUTE(3, "rspeaker", 0.40)
	MCFG_SOUND_ROUTE(4, "lspeaker", 0.40)
	MCFG_SOUND_ROUTE(5, "rspeaker", 0.40)

MACHINE_CONFIG_END

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

	ROM_REGION( 0x090000, "soundcpu", 0 ) /* Z80 */
	ROM_LOAD( "srom.u7", 0x00000, 0x80000, CRC(bb1deb89) SHA1(b1c70abddc0b9a88beb69a592376ff69a7e091eb) )
	ROM_RELOAD(          0x10000, 0x80000) /* for banks */

	ROM_REGION( 0x2000000, "gfx1", 0 )
	/* 0x0000000 - 0x03fffff empty */ /* spXX.uYY - XX is the bank number! */
	ROM_LOAD( "sp23.u14", 0x0400000, 0x400000, CRC(7df492eb) SHA1(30b88a3cd025ffc8c28fef06e0784755be37ef8e) )
	ROM_LOAD( "sp45.u15", 0x0800000, 0x400000, CRC(1890f42a) SHA1(22254fe38fd83f4602a25e1ccba32df16edaf3f9) )
	ROM_LOAD( "sp67.u16", 0x0c00000, 0x400000, CRC(a48f1ef0) SHA1(bf8787f293793291a503af662d3738c007654726) )
	ROM_LOAD( "sp89.u17", 0x1000000, 0x400000, CRC(6b024362) SHA1(8be5cc3c7306d28b75acd970bb3be6d3c9825367) )
	ROM_LOAD( "spab.u18", 0x1400000, 0x400000, CRC(803d2d8c) SHA1(25df30689e576a0620656c721d92bcc3fbd84844) )
	ROM_LOAD( "spcd.u19", 0x1800000, 0x400000, CRC(42e5c26e) SHA1(b68875d353bdc5d49113bbac02fd83508bce66a5) )

	ROM_REGION( 0x0800000, "gfx2", 0 )
	ROM_LOAD( "bg1012.u22", 0x0000000, 0x400000, CRC(d717a0a1) SHA1(007df309dc0650ca07e077b983a2b05730349d0b) )
	ROM_LOAD( "bg1113.u23", 0x0400000, 0x400000, CRC(94338267) SHA1(7848bc57cb0eac216100a508763451eb57a0a082) )

	ROM_REGION( 0x0800000, "gfx3", 0 )
	ROM_LOAD( "bg2022.u25", 0x0000000, 0x400000, CRC(ee312cd3) SHA1(2ef9d51928d80375daf8e6b204bb66a8b9cbaee7) )
	ROM_LOAD( "bg2123.u24", 0x0400000, 0x400000, CRC(4acfc469) SHA1(a98d06b967ebb3fa3b4c8aa3d7a05063ec981fb2) )

	ROM_REGION( 0x200000, "gfx4", 0 ) // background tiles
	ROM_LOAD( "map.u5", 0x00000, 0x200000, CRC(e681155e) SHA1(458845b9c86df72685d92d0d4052aacc2fa7d1bd) )

	ROM_REGION( 0x400000, "ymf1", 0 ) // OPL4 samples
	ROM_LOAD( "pcm.u6", 0x00000, 0x400000, CRC(ac72225a) SHA1(8d16399ed34ac5bd69dbf43b2de2b0db9ac1c610) )
ROM_END

/***************************************************************************

                 Asura Buster - Eternal Warriors (Japan)

Fuuki, 2000   Consists of a FG-3J MAIN-J mainboard &  FG-3J ROM-J combo

***************************************************************************/

ROM_START( asurabus )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* M68020 */
	ROM_LOAD32_BYTE( "pgm3.u1", 0x000000, 0x80000, CRC(2c6b5271) SHA1(188371f1f003823ac719e962e048719d76696b2f) )
	ROM_LOAD32_BYTE( "pgm2.u2", 0x000001, 0x80000, CRC(8f8694ec) SHA1(3334df4aecc5ab2f8914ef6748c027a99b39ce26) )
	ROM_LOAD32_BYTE( "pgm1.u3", 0x000002, 0x80000, CRC(0a040f0f) SHA1(d5e86d33efcbbde7ee62cfc8dfe867f250a33415) )
	ROM_LOAD32_BYTE( "pgm0.u4", 0x000003, 0x80000, CRC(9b71e9d8) SHA1(9b705b5b6fff549f5679890422b481b5cf1d7bd7) )

	ROM_REGION( 0x090000, "soundcpu", 0 ) /* Z80 */
	ROM_LOAD( "srom.u7", 0x00000, 0x80000, CRC(368da389) SHA1(1423b709da40bf3033c9032c4bd07658f1a969de) )
	ROM_RELOAD(          0x10000, 0x80000) /* for banks */

	ROM_REGION( 0x2000000, "gfx1", 0 )
	ROM_LOAD( "sp01.u13", 0x0000000, 0x400000, CRC(5edea463) SHA1(22a780912f060bae0c9a403a7bfd4d27f25b76e3) )
	ROM_LOAD( "sp23.u14", 0x0400000, 0x400000, CRC(91b1b0de) SHA1(341367966559ef2027415b673eb0db704680c81f) )
	ROM_LOAD( "sp45.u15", 0x0800000, 0x400000, CRC(96c69aac) SHA1(cf053523026651427f884b9dd7c095af362dd24e) )
	ROM_LOAD( "sp67.u16", 0x0c00000, 0x400000, CRC(7c3d83bf) SHA1(7188dd923c6c7eb6aee3323e7ab54aa240c35ea3) )
	ROM_LOAD( "sp89.u17", 0x1000000, 0x400000, CRC(cb1e14f8) SHA1(941cea1887d7ceb52222adcf1d6913969e6163aa) )
	ROM_LOAD( "spab.u18", 0x1400000, 0x400000, CRC(e5a4608d) SHA1(b8e39f53e0b7ad1e16ae9c3726597776b404be1c) )
	ROM_LOAD( "spcd.u19", 0x1800000, 0x400000, CRC(99bfbe32) SHA1(926a8afc4a175874f22f53300e76f59331d3b9ba) )
	ROM_LOAD( "spef.u20", 0x1c00000, 0x400000, CRC(c9c799cc) SHA1(01373316700d8688deeea2e9e8f831d5f86c7f17) )

	ROM_REGION( 0x0800000, "gfx2", 0 )
	ROM_LOAD( "bg1012.u22", 0x0000000, 0x400000, CRC(e3fb9af0) SHA1(11900cc2873337692f66fb4f1eb9c574e5a967de) )
	ROM_LOAD( "bg1113.u23", 0x0400000, 0x400000, CRC(5f8657e6) SHA1(7c2854dc5d2d4efe55bda01e329da051350e0031) )

	ROM_REGION( 0x0800000, "gfx3", 0 )
	ROM_LOAD( "bg2022.u25", 0x0000000, 0x400000, CRC(f46eda52) SHA1(46530016b32a164bd76c4f53e7b53b2beb28db06) )
	ROM_LOAD( "bg2123.u24", 0x0400000, 0x400000, CRC(c4ebb86b) SHA1(a7093e6e02b64566d277cbbd5fa90cd430e7c8a0) )

	ROM_REGION( 0x200000, "gfx4", 0 ) // background tiles
	ROM_LOAD( "map.u5", 0x00000, 0x200000, CRC(bd179dc5) SHA1(ce3fcac573b14fd5365eb5dcec3257e439d2c129) )

	ROM_REGION( 0x400000, "ymf1", 0 ) // OPL4 samples
	ROM_LOAD( "opm.u6", 0x00000, 0x400000, CRC(31b05be4) SHA1(d0f4f387f84a74591224b0f42b7f5c538a3dc498) )
ROM_END

ROM_START( asurabusa )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* M68020 */
	ROM_LOAD32_BYTE( "24-31.pgm3", 0x000000, 0x80000, CRC(cfcb9c75) SHA1(51e325d5e60d5bb058429f04a5170dcc17986b7d) )
	ROM_LOAD32_BYTE( "16-23.pgm2", 0x000001, 0x80000, CRC(e4d07738) SHA1(c6c949c5b0cbc129917bb8c93707539adabbd336) )
	ROM_LOAD32_BYTE( "8-15.pgm1",  0x000002, 0x80000, CRC(1dd67fe7) SHA1(3fd340ccd4a306783ba0ccd3343ae505c9de3a73) )
	ROM_LOAD32_BYTE( "0-7.pgm0",   0x000003, 0x80000, CRC(3af08de3) SHA1(1ecc69804693cab6c2c36120acfc6ced094a16e4) )

	ROM_REGION( 0x090000, "soundcpu", 0 ) /* Z80 */
	ROM_LOAD( "srom.u7", 0x00000, 0x80000, CRC(368da389) SHA1(1423b709da40bf3033c9032c4bd07658f1a969de) )
	ROM_RELOAD(          0x10000, 0x80000) /* for banks */

	ROM_REGION( 0x2000000, "gfx1", 0 )
	ROM_LOAD( "sp01.u13", 0x0000000, 0x400000, CRC(5edea463) SHA1(22a780912f060bae0c9a403a7bfd4d27f25b76e3) )
	ROM_LOAD( "sp23.u14", 0x0400000, 0x400000, CRC(91b1b0de) SHA1(341367966559ef2027415b673eb0db704680c81f) )
	ROM_LOAD( "sp45.u15", 0x0800000, 0x400000, CRC(96c69aac) SHA1(cf053523026651427f884b9dd7c095af362dd24e) )
	ROM_LOAD( "sp67.u16", 0x0c00000, 0x400000, CRC(7c3d83bf) SHA1(7188dd923c6c7eb6aee3323e7ab54aa240c35ea3) )
	ROM_LOAD( "sp89.u17", 0x1000000, 0x400000, CRC(cb1e14f8) SHA1(941cea1887d7ceb52222adcf1d6913969e6163aa) )
	ROM_LOAD( "spab.u18", 0x1400000, 0x400000, CRC(e5a4608d) SHA1(b8e39f53e0b7ad1e16ae9c3726597776b404be1c) )
	ROM_LOAD( "spcd.u19", 0x1800000, 0x400000, CRC(99bfbe32) SHA1(926a8afc4a175874f22f53300e76f59331d3b9ba) )
	ROM_LOAD( "spef.u20", 0x1c00000, 0x400000, CRC(c9c799cc) SHA1(01373316700d8688deeea2e9e8f831d5f86c7f17) )

	ROM_REGION( 0x0800000, "gfx2", 0 )
	ROM_LOAD( "bg1012.u22", 0x0000000, 0x400000, CRC(e3fb9af0) SHA1(11900cc2873337692f66fb4f1eb9c574e5a967de) )
	ROM_LOAD( "bg1113.u23", 0x0400000, 0x400000, CRC(5f8657e6) SHA1(7c2854dc5d2d4efe55bda01e329da051350e0031) )

	ROM_REGION( 0x0800000, "gfx3", 0 )
	ROM_LOAD( "bg2022.u25", 0x0000000, 0x400000, CRC(f46eda52) SHA1(46530016b32a164bd76c4f53e7b53b2beb28db06) )
	ROM_LOAD( "bg2123.u24", 0x0400000, 0x400000, CRC(c4ebb86b) SHA1(a7093e6e02b64566d277cbbd5fa90cd430e7c8a0) )

	ROM_REGION( 0x200000, "gfx4", 0 ) // background tiles
	ROM_LOAD( "map.u5", 0x00000, 0x200000, CRC(bd179dc5) SHA1(ce3fcac573b14fd5365eb5dcec3257e439d2c129) )

	ROM_REGION( 0x400000, "ymf1", 0 ) // OPL4 samples
	ROM_LOAD( "opm.u6", 0x00000, 0x400000, CRC(31b05be4) SHA1(d0f4f387f84a74591224b0f42b7f5c538a3dc498) )
ROM_END

/***************************************************************************


                                Game Drivers


***************************************************************************/

GAME( 1998, asurabld,   0, fuuki32, asurabld, driver_device, 0, ROT0, "Fuuki", "Asura Blade - Sword of Dynasty (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 2000, asurabus,   0,        fuuki32, asurabus, driver_device, 0, ROT0, "Fuuki", "Asura Buster - Eternal Warriors (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2000, asurabusa,  asurabus, fuuki32, asurabusa,driver_device, 0, ROT0, "Fuuki", "Asura Buster - Eternal Warriors (Japan) (ARCADIA review build)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // has pause function on P1 button 4

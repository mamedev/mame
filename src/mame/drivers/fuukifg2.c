/***************************************************************************

                          -= Fuuki 16 Bit Games =-

                    driver by   Luca Elia (l.elia@tin.it)


Main  CPU   :   M68000
Sound Chips :   YM2203  +  YM3812  +  M6295
Video Chips :   FI-002K (208pin PQFP, GA2)
                FI-003K (208pin PQFP, GA3)
Other       :   Mitsubishi M60067-0901FP 452100 (208pin PQFP, GA1)


---------------------------------------------------------------------------
Year + Game
---------------------------------------------------------------------------
95  Go Go! Mile Smile
96  Gyakuten!! Puzzle Bancho
---------------------------------------------------------------------------

Do NOT trust the Service Mode for dipswitch settings for Go Go! Mile Smile:
  Service Mode shows Coin A as SW2:3-5 & Coin B as SW2:6-8, but the game ignores the
  setting of Coin B and only uses the settings for Coin A, except for Coin B "Free Play"
  The game says Press 1 or 2, and will start the game, but jumps right to the Game Over
  and "Continue" countdown.
The Service Mode is WAY off on effects of dipswitches for the Japanese set!!! It reports
  the effects of MAME's SW1:3-8 have been moved to SW1:2-7 and Demo Sound has moved to
  SW2:7. What MAME shows as settings are according to actual game effect and reflect what
  the manual states.


To Do:

- Raster effects (level 5 interrupt is used for that). In pbancho
  they involve changing the *vertical* scroll value of the layers
  each scanline (when you are about to die, in the solo game).
  In gogomile they weave the water backgrounds and do some
  parallactic scrolling on later levels. *partly done, could do with
  some tweaking

- The scroll values are generally wrong when flip screen is on and rasters are often incorrect

- gogomile M6295 banking is incorrect (the story sequence should have speech)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/2203intf.h"
#include "sound/3812intf.h"
#include "sound/okim6295.h"
#include "includes/fuukifg2.h"


/***************************************************************************


                            Memory Maps - Main CPU


***************************************************************************/

WRITE16_MEMBER(fuuki16_state::fuuki16_vregs_w)
{
	UINT16 old_data = m_vregs[offset];
	UINT16 new_data = COMBINE_DATA(&m_vregs[offset]);
	if ((offset == 0x1c/2) && old_data != new_data)
	{
		const rectangle &visarea = machine().primary_screen->visible_area();
		attotime period = machine().primary_screen->frame_period();
		m_raster_interrupt_timer->adjust(machine().primary_screen->time_until_pos(new_data, visarea.max_x + 1), 0, period);
	}
}

WRITE16_MEMBER(fuuki16_state::fuuki16_sound_command_w)
{
	if (ACCESSING_BITS_0_7)
	{
		soundlatch_w(space,0,data & 0xff);
		device_set_input_line(m_audiocpu, INPUT_LINE_NMI, PULSE_LINE);
//      device_spin_until_time(&space.device(), attotime::from_usec(50));   // Allow the other CPU to reply
		machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(50)); // Fixes glitching in rasters
	}
}

static ADDRESS_MAP_START( fuuki16_map, AS_PROGRAM, 16, fuuki16_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM																		// ROM
	AM_RANGE(0x400000, 0x40ffff) AM_RAM																		// RAM
	AM_RANGE(0x500000, 0x501fff) AM_RAM_WRITE(fuuki16_vram_0_w) AM_BASE(m_vram[0])					// Layers
	AM_RANGE(0x502000, 0x503fff) AM_RAM_WRITE(fuuki16_vram_1_w) AM_BASE(m_vram[1])					//
	AM_RANGE(0x504000, 0x505fff) AM_RAM_WRITE(fuuki16_vram_2_w) AM_BASE(m_vram[2])					//
	AM_RANGE(0x506000, 0x507fff) AM_RAM_WRITE(fuuki16_vram_3_w) AM_BASE(m_vram[3])					//
	AM_RANGE(0x600000, 0x601fff) AM_MIRROR(0x008000) AM_RAM AM_BASE_SIZE(m_spriteram, m_spriteram_size)	// Sprites, mirrored?
	AM_RANGE(0x700000, 0x703fff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_SHARE("paletteram")	// Palette
	AM_RANGE(0x800000, 0x800001) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x810000, 0x810001) AM_READ_PORT("P1_P2")
	AM_RANGE(0x880000, 0x880001) AM_READ_PORT("DSW")
	AM_RANGE(0x8a0000, 0x8a0001) AM_WRITE(fuuki16_sound_command_w)											// To Sound CPU
	AM_RANGE(0x8c0000, 0x8c001f) AM_RAM_WRITE(fuuki16_vregs_w) AM_BASE(m_vregs )						// Video Registers
	AM_RANGE(0x8d0000, 0x8d0003) AM_RAM AM_BASE(m_unknown)											//
	AM_RANGE(0x8e0000, 0x8e0001) AM_RAM AM_BASE(m_priority)											//
ADDRESS_MAP_END


/***************************************************************************


                            Memory Maps - Sound CPU


***************************************************************************/

WRITE8_MEMBER(fuuki16_state::fuuki16_sound_rombank_w)
{
	if (data <= 2)
		memory_set_bank(machine(), "bank1", data);
	else
		logerror("CPU #1 - PC %04X: unknown bank bits: %02X\n", cpu_get_pc(&space.device()), data);
}

static WRITE8_DEVICE_HANDLER( fuuki16_oki_banking_w )
{
	/*
        data & 0x06 is always equals to data & 0x60
        data & 0x10 is always set
    */

	okim6295_device *oki = downcast<okim6295_device *>(device);
	oki->set_bank_base(((data & 6) >> 1) * 0x40000);
}

static ADDRESS_MAP_START( fuuki16_sound_map, AS_PROGRAM, 8, fuuki16_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM			// ROM
	AM_RANGE(0x6000, 0x7fff) AM_RAM			// RAM
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank1")	// Banked ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( fuuki16_sound_io_map, AS_IO, 8, fuuki16_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(fuuki16_sound_rombank_w)	// ROM Bank
	AM_RANGE(0x11, 0x11) AM_READ(soundlatch_r) AM_WRITENOP	// From Main CPU / ? To Main CPU ?
	AM_RANGE(0x20, 0x20) AM_DEVWRITE_LEGACY("oki", fuuki16_oki_banking_w)	// Oki Banking
	AM_RANGE(0x30, 0x30) AM_WRITENOP	// ? In the NMI routine
	AM_RANGE(0x40, 0x41) AM_DEVWRITE_LEGACY("ym1", ym2203_w)
	AM_RANGE(0x50, 0x51) AM_DEVREADWRITE_LEGACY("ym2", ym3812_r, ym3812_w)
	AM_RANGE(0x60, 0x60) AM_DEVREAD("oki", okim6295_device, read)	// M6295
	AM_RANGE(0x61, 0x61) AM_DEVWRITE("oki", okim6295_device, write)	// M6295
ADDRESS_MAP_END


/***************************************************************************


                                Input Ports


***************************************************************************/

static INPUT_PORTS_START( gogomile )
	PORT_START("SYSTEM")	// $800000.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xfe00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1_P2")		// $810000.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )				// There's code that uses
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )				// these unknown bits
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")		// $880000.w
	PORT_SERVICE_DIPLOC(  0x0001, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x0002, 0x0002, "Demo Music" )		PORT_DIPLOCATION("SW1:2") /* Game play sounds still play, only effects Music */
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0030, 0x0020, DEF_STR( Language ) )	PORT_DIPLOCATION("SW1:5,6") /* Default Language: English */
	PORT_DIPSETTING(      0x0010, DEF_STR( Chinese ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( Japanese ) )	/* Only setting to give a "For use only in...." Copyright Notice */
	PORT_DIPSETTING(      0x0000, DEF_STR( Korean ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( English ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Lives ) )	PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x00c0, "3" )
	PORT_DIPSETTING(      0x0080, "4" )
	PORT_DIPSETTING(      0x0040, "5" )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) )	PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0200, 0x0200, "SW2:2" )	/* Manual states this dip is "Unused" */
	PORT_DIPNAME( 0x1c00, 0x1c00, DEF_STR( Coinage ) )	PORT_DIPLOCATION("SW2:3,4,5")
	PORT_DIPSETTING(      0x0400, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1400, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x1c00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPUNUSED_DIPLOC( 0x2000, 0x2000, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" )
/*
    PORT_DIPNAME( 0xe000, 0xe000, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW2:6,7,8")
    PORT_DIPSETTING(      0x2000, DEF_STR( 4C_1C ) )
    PORT_DIPSETTING(      0xa000, DEF_STR( 3C_1C ) )
    PORT_DIPSETTING(      0x6000, DEF_STR( 2C_1C ) )
    PORT_DIPSETTING(      0xe000, DEF_STR( 1C_1C ) )
    PORT_DIPSETTING(      0xc000, DEF_STR( 1C_2C ) )
    PORT_DIPSETTING(      0x4000, DEF_STR( 1C_3C ) )
    PORT_DIPSETTING(      0x8000, DEF_STR( 1C_4C ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
*/
INPUT_PORTS_END

/* Same as gogomile, but the default country is different and the coinage settings too. */
static INPUT_PORTS_START( gogomilej )
	PORT_INCLUDE( gogomile )

	PORT_MODIFY("DSW")		// $880000.w
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Language ) ) 	PORT_DIPLOCATION("SW1:5,6") /* Default Language: Japanese */
	PORT_DIPSETTING(      0x0010, DEF_STR( Chinese ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( Japanese ) )	/* Only setting to give a "For use only in...." Copyright Notice */
	PORT_DIPSETTING(      0x0000, DEF_STR( Korean ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( English ) )

	PORT_DIPNAME( 0x1c00, 0x1c00, DEF_STR( Coin_A ) )	PORT_DIPLOCATION("SW2:3,4,5")
	PORT_DIPSETTING(      0x1800, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1400, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x1c00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
/*
    PORT_DIPNAME( 0xe000, 0xe000, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW2:6,7,8")
    PORT_DIPSETTING(      0xc000, DEF_STR( 4C_1C ) )
    PORT_DIPSETTING(      0xa000, DEF_STR( 3C_1C ) )
    PORT_DIPSETTING(      0x8000, DEF_STR( 2C_1C ) )
    PORT_DIPSETTING(      0xe000, DEF_STR( 1C_1C ) )
    PORT_DIPSETTING(      0x2000, DEF_STR( 1C_2C ) )
    PORT_DIPSETTING(      0x4000, DEF_STR( 1C_3C ) )
    PORT_DIPSETTING(      0x6000, DEF_STR( 1C_4C ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
*/
INPUT_PORTS_END

static INPUT_PORTS_START( pbancho )
	PORT_INCLUDE( gogomile )

	PORT_MODIFY("SYSTEM")	// $800000.w
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2    )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_MODIFY("DSW")		// $880000.w
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(      0x0008, DEF_STR( Easiest ) )	// 1
	PORT_DIPSETTING(      0x0010, DEF_STR( Easy )    )	// 2
	PORT_DIPSETTING(      0x001c, DEF_STR( Normal )  )	// 3
	PORT_DIPSETTING(      0x0018, DEF_STR( Hard )    )	// 4
	PORT_DIPSETTING(      0x0004, DEF_STR( Hardest ) )	// 5
//  PORT_DIPSETTING(      0x0000, DEF_STR( Normal )  )  // 3
//  PORT_DIPSETTING(      0x000c, DEF_STR( Normal )  )  // 3
//  PORT_DIPSETTING(      0x0014, DEF_STR( Normal )  )  // 3
	PORT_DIPNAME( 0x0060, 0x0060, "Lives (Vs Mode)" )	PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(      0x0000, "1" )	// 1 1
	PORT_DIPSETTING(      0x0060, "2" )	// 2 3
//  PORT_DIPSETTING(      0x0020, "2" ) // 2 3
	PORT_DIPSETTING(      0x0040, "3" )	// 3 5
	PORT_DIPNAME( 0x0080, 0x0080, "? Senin Mode ?" )	PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) )	PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "Allow Versus Mode" )	PORT_DIPLOCATION("SW2:2") // "unused" in the manual?
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x1c00, 0x1c00, DEF_STR( Coin_A ) )	PORT_DIPLOCATION("SW2:3,4,5")
	PORT_DIPSETTING(      0x0c00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1400, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x1c00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xe000, 0xe000, DEF_STR( Coin_B ) )	PORT_DIPLOCATION("SW2:6,7,8")
	PORT_DIPSETTING(      0x6000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
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
	{	2*4,3*4,   0*4,1*4,   6*4,7*4, 4*4,5*4,
		10*4,11*4, 8*4,9*4, 14*4,15*4, 12*4,13*4	},
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
	{	2*4,3*4,   0*4,1*4,   6*4,7*4, 4*4,5*4,
		10*4,11*4, 8*4,9*4, 14*4,15*4, 12*4,13*4	},
	{ STEP16(0,16*4) },
	16*16*4
};

static GFXDECODE_START( fuuki16 )
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x4, 0x400*2, 0x40 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, layout_16x16x4, 0x400*0, 0x40 ) // [1] Layer 0
	GFXDECODE_ENTRY( "gfx3", 0, layout_16x16x8, 0x400*1, 0x40 ) // [2] Layer 1
	GFXDECODE_ENTRY( "gfx4", 0, layout_8x8x4,   0x400*3, 0x40 ) // [3] Layer 2
	GFXDECODE_ENTRY( "gfx4", 0, layout_8x8x4,   0x400*3, 0x40 ) // [4] Layer 3 (GFX4!)
GFXDECODE_END


/***************************************************************************


                                Machine Drivers


***************************************************************************/

static void soundirq( device_t *device, int state )
{
	fuuki16_state *fuuki16 = device->machine().driver_data<fuuki16_state>();
	device_set_input_line(fuuki16->m_audiocpu, 0, state);
}

static const ym3812_interface fuuki16_ym3812_intf =
{
	soundirq	/* IRQ Line */
};

/*
    - Interrupts (pbancho) -

    Lev 1:  Sets bit 5 of $400010. Prints "credit .." with sprites.
    Lev 2:  Sets bit 7 of $400010. Clears $8c0012.
            It seems unused by the game.
    Lev 3:  VBlank.
    Lev 5:  Programmable to happen on a raster line. Used to do raster
            effects when you die and its clearing the blocks
            also used for water effects and titlescreen linescroll on gogomile
*/

static TIMER_CALLBACK( level_1_interrupt_callback )
{
	fuuki16_state *state = machine.driver_data<fuuki16_state>();
	device_set_input_line(state->m_maincpu, 1, HOLD_LINE);
	machine.scheduler().timer_set(machine.primary_screen->time_until_pos(248), FUNC(level_1_interrupt_callback));
}


static TIMER_CALLBACK( vblank_interrupt_callback )
{
	fuuki16_state *state = machine.driver_data<fuuki16_state>();
	device_set_input_line(state->m_maincpu, 3, HOLD_LINE);	// VBlank IRQ
	machine.scheduler().timer_set(machine.primary_screen->time_until_vblank_start(), FUNC(vblank_interrupt_callback));
}


static TIMER_CALLBACK( raster_interrupt_callback )
{
	fuuki16_state *state = machine.driver_data<fuuki16_state>();
	device_set_input_line(state->m_maincpu, 5, HOLD_LINE);	// Raster Line IRQ
	machine.primary_screen->update_partial(machine.primary_screen->vpos());
	state->m_raster_interrupt_timer->adjust(machine.primary_screen->frame_period());
}


static MACHINE_START( fuuki16 )
{
	fuuki16_state *state = machine.driver_data<fuuki16_state>();
	UINT8 *ROM = machine.region("audiocpu")->base();

	memory_configure_bank(machine, "bank1", 0, 3, &ROM[0x10000], 0x8000);

	state->m_maincpu = machine.device("maincpu");
	state->m_audiocpu = machine.device("audiocpu");

	state->m_raster_interrupt_timer = machine.scheduler().timer_alloc(FUNC(raster_interrupt_callback));
}


static MACHINE_RESET( fuuki16 )
{
	fuuki16_state *state = machine.driver_data<fuuki16_state>();
	const rectangle &visarea = machine.primary_screen->visible_area();

	machine.scheduler().timer_set(machine.primary_screen->time_until_pos(248), FUNC(level_1_interrupt_callback));
	machine.scheduler().timer_set(machine.primary_screen->time_until_vblank_start(), FUNC(vblank_interrupt_callback));
	state->m_raster_interrupt_timer->adjust(machine.primary_screen->time_until_pos(0, visarea.max_x + 1));
}


static MACHINE_CONFIG_START( fuuki16, fuuki16_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 16000000)
	MCFG_CPU_PROGRAM_MAP(fuuki16_map)

	MCFG_CPU_ADD("audiocpu", Z80, 3000000)	/* ? */
	MCFG_CPU_PROGRAM_MAP(fuuki16_sound_map)
	MCFG_CPU_IO_MAP(fuuki16_sound_io_map)

	MCFG_MACHINE_START(fuuki16)
	MCFG_MACHINE_RESET(fuuki16)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(320, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 256-16-1)
	MCFG_SCREEN_UPDATE_STATIC(fuuki16)

	MCFG_GFXDECODE(fuuki16)
	MCFG_PALETTE_LENGTH(0x800*4)

	MCFG_VIDEO_START(fuuki16)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ym1", YM2203, 4000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.15)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.15)

	MCFG_SOUND_ADD("ym2", YM3812, 4000000)
	MCFG_SOUND_CONFIG(fuuki16_ym3812_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.30)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.30)

	MCFG_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.85)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.85)
MACHINE_CONFIG_END


/***************************************************************************


                                ROMs Loading


***************************************************************************/

/***************************************************************************

                    Go! Go! Mile Smile / Susume! Mile Smile

(c)1995 Fuuki
FG-1C AI AM-2 (same board as Gyakuten Puzzle Banchou)

CPU  : TMP68HC000P-16
Sound: Z80 YM2203C YM3812 M6295 Y3014Bx2
OSC  : 32.00000MHz(OSC1) 28.64000MHz(OSC2) 12.000MHz(Xtal1)

ROMs:
fp2.2 - Main programs (27c4000)
fp1.1 /

lh538n1d.25 - Samples (Sharp mask, read as 27c8001)
fs1.24 - Sound program (27c010)

lh5370h8.11 - Sprites? (Sharp Mask, read as 27c160)
lh5370ha.12 |
lh5370h7.15 |
lh5370h9.16 /

lh537k2r.20 - Tiles? (Sharp Mask, read as 27c160)
lh5370hb.19 |
lh5370h6.3  /

Custom chips:
FI-002K (208pin PQFP, GA2)
FI-003K (208pin PQFP, GA3)


Others:
Mitsubishi M60067-0901FP 452100 (208pin PQFP, GA1)
4 GALs (16V8B, not dumped)

***************************************************************************/

ROM_START( gogomile )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "fp2n", 0x000000, 0x080000, CRC(e73583a0) SHA1(05c6ee5cb2c151b32c462e8b920f9a57fb6cce5b) )
	ROM_LOAD16_BYTE( "fp1n", 0x000001, 0x080000, CRC(7b110824) SHA1(980e326d3b9e113ed522be3076663a249da4e739) )

	ROM_REGION( 0x28000, "audiocpu", 0 )		/* Z80 Code */
	ROM_LOAD( "fs1.24", 0x00000, 0x08000, CRC(4e4bd371) SHA1(429e776135ce8960e147762763d952d16ed3f9d4) )	// same as japanese version
	ROM_CONTINUE(       0x10000, 0x18000             )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* 16x16x4 Sprites */
	ROM_LOAD( "lh537k2r.20", 0x000000, 0x200000, CRC(525dbf51) SHA1(f21876676cc60ed65bc86884da894b24830826bb) )

	ROM_REGION( 0x200000, "gfx2", 0 )	/* 16x16x4 Tiles */
	ROM_LOAD( "lh5370h6.3", 0x000000, 0x200000, CRC(e2ca7107) SHA1(7174c2e1e2106275ad41b53af22651dca492367a) )	// x11xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x800000, "gfx3", 0 )	/* 16x16x8 Tiles */
	ROM_LOAD( "lh5370h8.11", 0x000000, 0x200000, CRC(9961c925) SHA1(c47b4f19f090527b3e0c04dd046aa9cd51ca0e16) )
	ROM_LOAD( "lh5370ha.12", 0x200000, 0x200000, CRC(5f2a87de) SHA1(d7ed8f01b40aaf58126aaeee10ec7d948a144080) )
	ROM_LOAD( "lh5370h7.15", 0x400000, 0x200000, CRC(34921680) SHA1(d9862f106caa14ea6ad925174e6bf2d542511593) )
	ROM_LOAD( "lh5370h9.16", 0x600000, 0x200000, CRC(e0118483) SHA1(36f9068e6c81c171b4426c3794277742bbc926f5) )

	ROM_REGION( 0x200000, "gfx4", 0 )	/* 16x16x4 Tiles */
	ROM_LOAD( "lh5370hb.19", 0x000000, 0x200000, CRC(bd1e896f) SHA1(075f7600cbced1d285cf32fc196844720eb12671) )	// FIRST AND SECOND HALF IDENTICAL

	/* 0x40000 * 4: sounds+speech (japanese),sounds+speech (english) */
	ROM_REGION( 0x100000, "oki", 0 )	/* Samples */
	ROM_LOAD( "lh538n1d.25", 0x000000, 0x100000, CRC(01622a95) SHA1(8d414bfc6dcfab1cf9cfe5738eb5c2ff31b77df6) )	// 0x40000 * 4
ROM_END

ROM_START( gogomilej )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "fp2.2", 0x000000, 0x080000, CRC(28fd3e4e) SHA1(3303e5759c0781035c74354587e1916719695754) )	// 1xxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "fp1.1", 0x000001, 0x080000, CRC(35a5fc45) SHA1(307207791cee7f40e88feffc5805ac25008a8566) )	// 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x28000, "audiocpu", 0 )		/* Z80 Code */
	ROM_LOAD( "fs1.24", 0x00000, 0x08000, CRC(4e4bd371) SHA1(429e776135ce8960e147762763d952d16ed3f9d4) )
	ROM_CONTINUE(       0x10000, 0x18000             )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* 16x16x4 Sprites */
	ROM_LOAD( "lh537k2r.20", 0x000000, 0x200000, CRC(525dbf51) SHA1(f21876676cc60ed65bc86884da894b24830826bb) )

	ROM_REGION( 0x200000, "gfx2", 0 )	/* 16x16x4 Tiles */
	ROM_LOAD( "lh5370h6.3", 0x000000, 0x200000, CRC(e2ca7107) SHA1(7174c2e1e2106275ad41b53af22651dca492367a) )	// x11xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x800000, "gfx3", 0 )	/* 16x16x8 Tiles */
	ROM_LOAD( "lh5370h8.11", 0x000000, 0x200000, CRC(9961c925) SHA1(c47b4f19f090527b3e0c04dd046aa9cd51ca0e16) )
	ROM_LOAD( "lh5370ha.12", 0x200000, 0x200000, CRC(5f2a87de) SHA1(d7ed8f01b40aaf58126aaeee10ec7d948a144080) )
	ROM_LOAD( "lh5370h7.15", 0x400000, 0x200000, CRC(34921680) SHA1(d9862f106caa14ea6ad925174e6bf2d542511593) )
	ROM_LOAD( "lh5370h9.16", 0x600000, 0x200000, CRC(e0118483) SHA1(36f9068e6c81c171b4426c3794277742bbc926f5) )

	ROM_REGION( 0x200000, "gfx4", 0 )	/* 16x16x4 Tiles */
	ROM_LOAD( "lh5370hb.19", 0x000000, 0x200000, CRC(bd1e896f) SHA1(075f7600cbced1d285cf32fc196844720eb12671) )	// FIRST AND SECOND HALF IDENTICAL

	/* 0x40000 * 4: sounds+speech (japanese),sounds+speech (english) */
	ROM_REGION( 0x100000, "oki", 0 )	/* Samples */
	ROM_LOAD( "lh538n1d.25", 0x000000, 0x100000, CRC(01622a95) SHA1(8d414bfc6dcfab1cf9cfe5738eb5c2ff31b77df6) )	// 0x40000 * 4
ROM_END



/***************************************************************************

                            Gyakuten!! Puzzle Bancho

(c)1996 Fuuki
FG-1C AI AM-2

CPU  : TMP68HC000P-16
Sound: Z80 YM2203 YM3812 M6295
OSC  : 32.00000MHz(OSC1) 28.64000MHz(OSC2) 12.000MHz(Xtal1)

ROMs:
rom2.no1 - Main program (even)(27c4000)
rom1.no2 - Main program (odd) (27c4000)

rom25.no3 - Samples (27c2001)
rom24.no4 - Sound program (27c010)

rom11.61 - Graphics (Mask, read as 27c160)
rom15.59 |
rom20.58 |
rom3.60  /

Custom chips:
FI-002K (208pin PQFP, GA2)
FI-003K (208pin PQFP, GA3)

Others:
Mitsubishi M60067-0901FP 452100 (208pin PQFP, GA1)

***************************************************************************/

ROM_START( pbancho )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "rom2.no1", 0x000000, 0x080000, CRC(1b4fd178) SHA1(02cf3d2554b29cd253470d68ea959738f3b98dbe) )	// 1xxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "rom1.no2", 0x000001, 0x080000, CRC(9cf510a5) SHA1(08e79b5bbd1c011c32f82dd15fba42d7898861be) )	// 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x28000, "audiocpu", 0 )		/* Z80 Code */
	ROM_LOAD( "rom24.no4", 0x00000, 0x08000, CRC(dfbfdb81) SHA1(84b0cbe843a9bbae43975afdbd029a9b76fd488b) )
	ROM_CONTINUE(          0x10000, 0x18000             )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* 16x16x4 Sprites */
	ROM_LOAD( "rom20.58", 0x000000, 0x200000, CRC(4dad0a2e) SHA1(a4f70557503110a5457b9096a79a5f249095fa55) )

	ROM_REGION( 0x200000, "gfx2", 0 )	/* 16x16x4 Tiles */
	ROM_LOAD( "rom3.60",  0x000000, 0x200000, CRC(a50a3c1b) SHA1(a2b30f9f83f5dc2e069d7559aefbda9929fc640c) )

	ROM_REGION( 0x400000, "gfx3", 0 )	/* 16x16x8 Tiles */
	ROM_LOAD( "rom11.61", 0x000000, 0x200000, CRC(7f1213b9) SHA1(f8d6432b270c4d0954602e430ddd26841eb05656) )
	ROM_LOAD( "rom15.59", 0x200000, 0x200000, CRC(b83dcb70) SHA1(b0b9df451535d85612fa095b4f694cf2e7930bca) )

	ROM_REGION( 0x200000, "gfx4", 0 )	/* 16x16x4 Tiles */
	ROM_LOAD( "rom3.60",  0x000000, 0x200000, CRC(a50a3c1b) SHA1(a2b30f9f83f5dc2e069d7559aefbda9929fc640c) )	// ?maybe?

	ROM_REGION( 0x040000, "oki", 0 )	/* Samples */
	ROM_LOAD( "rom25.no3", 0x000000, 0x040000, CRC(a7bfb5ea) SHA1(61937eae4f8855bc09c494aff52d76d41dc3b76a) )
ROM_END


/***************************************************************************


                                Game Drivers


***************************************************************************/

GAME( 1995, gogomile, 0,        fuuki16, gogomile, 0, ROT0, "Fuuki", "Go Go! Mile Smile", GAME_IMPERFECT_SOUND )
GAME( 1995, gogomilej,gogomile, fuuki16, gogomilej,0, ROT0, "Fuuki", "Susume! Mile Smile (Japan)", GAME_IMPERFECT_SOUND )
GAME( 1996, pbancho,  0,        fuuki16, pbancho,  0, ROT0, "Fuuki", "Gyakuten!! Puzzle Bancho (Japan)", 0)

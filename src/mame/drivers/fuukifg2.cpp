// license:BSD-3-Clause
// copyright-holders:Luca Elia,Paul Priest
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
95  Susume! Mile Smile / Go Go! Mile Smile
96  Gyakuten!! Puzzle Bancho
---------------------------------------------------------------------------

Do NOT trust the Service Mode for dipswitch settings for Go Go! Mile Smile:
  Service Mode shows Coin A as SW2:3-5 & Coin B as SW2:6-8, but the game ignores the
  setting of Coin B and only uses the settings for Coin A, except for Coin B "Free Play"
  The game says Press 1 or 2, and will start the game, but jumps right to the Game Over
  and "Continue" countdown.
The Service Mode is WAY off on effects of dipswitches for the ealier set!!! It reports
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

***************************************************************************/

#include "emu.h"
#include "includes/fuukifg2.h"

#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/ymopn.h"
#include "sound/ymopl.h"
#include "speaker.h"


/***************************************************************************


                            Memory Maps - Main CPU


***************************************************************************/

void fuuki16_state::vregs_w(offs_t offset, u16 data, u16 mem_mask)
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

void fuuki16_state::sound_command_w(u8 data)
{
	m_soundlatch->write(data & 0xff);
	m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
//      m_maincpu->spin_until_time(attotime::from_usec(50));   // Allow the other CPU to reply
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(50)); // Fixes glitching in rasters
}

template<int Layer>
void fuuki16_state::vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_vram[Layer][offset]);
	m_tilemap[Layer]->mark_tile_dirty(offset / 2);
}

template<int Layer>
void fuuki16_state::vram_buffered_w(offs_t offset, u16 data, u16 mem_mask)
{
	const int buffer = (m_vregs[0x1e / 2] & 0x40) >> 6;
	COMBINE_DATA(&m_vram[Layer][offset]);
	if ((Layer & 1) == buffer)
		m_tilemap[2]->mark_tile_dirty(offset / 2);
}

void fuuki16_state::fuuki16_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                                                     // ROM
	map(0x400000, 0x40ffff).ram();                                                                     // RAM
	map(0x500000, 0x501fff).ram().w(FUNC(fuuki16_state::vram_w<0>)).share("vram.0");                  // Layers
	map(0x502000, 0x503fff).ram().w(FUNC(fuuki16_state::vram_w<1>)).share("vram.1");                  //
	map(0x504000, 0x505fff).ram().w(FUNC(fuuki16_state::vram_buffered_w<2>)).share("vram.2");                  //
	map(0x506000, 0x507fff).ram().w(FUNC(fuuki16_state::vram_buffered_w<3>)).share("vram.3");                  //
	map(0x600000, 0x601fff).mirror(0x008000).ram().share("spriteram");   // Sprites, mirrored?
	map(0x700000, 0x703fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");    // Palette
	map(0x800000, 0x800001).portr("SYSTEM");
	map(0x810000, 0x810001).portr("P1_P2");
	map(0x880000, 0x880001).portr("DSW");
	map(0x8a0001, 0x8a0001).w(FUNC(fuuki16_state::sound_command_w));                                          // To Sound CPU
	map(0x8c0000, 0x8c001f).ram().w(FUNC(fuuki16_state::vregs_w)).share("vregs");                        // Video Registers
	map(0x8d0000, 0x8d0003).ram().share("unknown");                                         //
	map(0x8e0000, 0x8e0001).ram().share("priority");                                            //
}


/***************************************************************************


                            Memory Maps - Sound CPU


***************************************************************************/

void fuuki16_state::sound_rombank_w(u8 data)
{
	if (data <= 2)
		m_soundbank->set_entry(data);
	else
		logerror("CPU #1 - PC %04X: unknown bank bits: %02X\n", m_audiocpu->pc(), data);
}

void fuuki16_state::oki_banking_w(u8 data)
{
	/*
	    data & 0x06 is always equals to data & 0x60
	    data & 0x10 is always set
	*/

	m_oki->set_rom_bank((data & 6) >> 1);
}

void fuuki16_state::fuuki16_sound_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();         // ROM
	map(0x6000, 0x7fff).ram();         // RAM
	map(0x8000, 0xffff).bankr("soundbank");    // Banked ROM
}

void fuuki16_state::fuuki16_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(fuuki16_state::sound_rombank_w));  // ROM Bank
	map(0x11, 0x11).r(m_soundlatch, FUNC(generic_latch_8_device::read)).nopw(); // From Main CPU / ? To Main CPU ?
	map(0x20, 0x20).w(FUNC(fuuki16_state::oki_banking_w));    // Oki Banking
	map(0x30, 0x30).nopw();    // ? In the NMI routine
	map(0x40, 0x41).w("ym1", FUNC(ym2203_device::write));
	map(0x50, 0x51).rw("ym2", FUNC(ym3812_device::read), FUNC(ym3812_device::write));
	map(0x60, 0x60).r(m_oki, FUNC(okim6295_device::read));   // M6295
	map(0x61, 0x61).w(m_oki, FUNC(okim6295_device::write)); // M6295
}


/***************************************************************************


                                Input Ports


***************************************************************************/

static INPUT_PORTS_START( gogomile )
	PORT_START("SYSTEM")    // $800000.w
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

	PORT_START("P1_P2")     // $810000.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )              // There's code that uses
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )              // these unknown bits
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")       // $880000.w
	PORT_SERVICE_DIPLOC(  0x0001, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x0002, 0x0002, "Demo Music" )        PORT_DIPLOCATION("SW1:2") /* Game play sounds still play, only effects Music */
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Language ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0010, DEF_STR( Chinese ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( Japanese ) ) /* Only setting to give a "For use only in...." Copyright Notice */
	PORT_DIPSETTING(      0x0000, DEF_STR( Korean ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( English ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x00c0, "3" )
	PORT_DIPSETTING(      0x0080, "4" )
	PORT_DIPSETTING(      0x0040, "5" )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0200, 0x0200, "SW2:2" )    /* Manual states this dip is "Unused" */
	PORT_DIPNAME( 0x1c00, 0x1c00, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW2:3,4,5")
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


static INPUT_PORTS_START( gogomileo )   /* The ealier version has different coinage settings. */
	PORT_INCLUDE( gogomile )

	PORT_MODIFY("DSW")      // $880000.w
	PORT_DIPNAME( 0x1c00, 0x1c00, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW2:3,4,5")
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

	PORT_MODIFY("SYSTEM")   // $800000.w
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2    )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_MODIFY("DSW")      // $880000.w
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(      0x0008, DEF_STR( Easiest ) )  // 1
	PORT_DIPSETTING(      0x0010, DEF_STR( Easy )    )  // 2
	PORT_DIPSETTING(      0x001c, DEF_STR( Normal )  )  // 3
	PORT_DIPSETTING(      0x0018, DEF_STR( Hard )    )  // 4
	PORT_DIPSETTING(      0x0004, DEF_STR( Hardest ) )  // 5
//  PORT_DIPSETTING(      0x0000, DEF_STR( Normal )  )  // 3
//  PORT_DIPSETTING(      0x000c, DEF_STR( Normal )  )  // 3
//  PORT_DIPSETTING(      0x0014, DEF_STR( Normal )  )  // 3
	PORT_DIPNAME( 0x0060, 0x0060, "Lives (Vs Mode)" )   PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(      0x0000, "1" ) // 1 1
	PORT_DIPSETTING(      0x0060, "2" ) // 2 3
//  PORT_DIPSETTING(      0x0020, "2" ) // 2 3
	PORT_DIPSETTING(      0x0040, "3" ) // 3 5
	PORT_DIPNAME( 0x0080, 0x0080, "? Senin Mode ?" )    PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "Allow Versus Mode" ) PORT_DIPLOCATION("SW2:2") // "unused" in the manual?
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x1c00, 0x1c00, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW2:3,4,5")
	PORT_DIPSETTING(      0x0c00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1400, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x1c00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xe000, 0xe000, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW2:6,7,8")
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

/* 16x16x4 */
static const gfx_layout layout_16x16x4 =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ STEP16(0,4) },
	{ STEP16(0,16*4) },
	16*16*4
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

static GFXDECODE_START( gfx_fuuki16 )
	GFXDECODE_ENTRY( "gfx2", 0, layout_16x16x4,         0x400*0, 0x40 ) // [0] Layer 0
	GFXDECODE_ENTRY( "gfx3", 0, layout_16x16x8,         0x400*1, 0x40 ) // [1] Layer 1
	GFXDECODE_ENTRY( "gfx4", 0, gfx_8x8x4_packed_msb,   0x400*3, 0x40 ) // [2] Layer 2
GFXDECODE_END


/***************************************************************************


                                Machine Drivers


***************************************************************************/

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

void fuuki16_state::device_timer(emu_timer &timer, device_timer_id id, int param)
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
		throw emu_fatalerror("Unknown id in fuuki16_state::device_timer");
	}
}


void fuuki16_state::machine_start()
{
	u8 *ROM = memregion("audiocpu")->base();

	m_soundbank->configure_entries(0, 3, &ROM[0x8000], 0x8000);

	m_level_1_interrupt_timer = timer_alloc(TIMER_LEVEL_1_INTERRUPT);
	m_vblank_interrupt_timer = timer_alloc(TIMER_VBLANK_INTERRUPT);
	m_raster_interrupt_timer = timer_alloc(TIMER_RASTER_INTERRUPT);
}


void fuuki16_state::machine_reset()
{
	const rectangle &visarea = m_screen->visible_area();

	m_level_1_interrupt_timer->adjust(m_screen->time_until_pos(248));
	m_vblank_interrupt_timer->adjust(m_screen->time_until_vblank_start());
	m_raster_interrupt_timer->adjust(m_screen->time_until_pos(0, visarea.max_x + 1));
}


void fuuki16_state::fuuki16(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(32'000'000) / 2);    /* 16 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &fuuki16_state::fuuki16_map);

	Z80(config, m_audiocpu, XTAL(12'000'000) / 2);      /* 6 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &fuuki16_state::fuuki16_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &fuuki16_state::fuuki16_sound_io_map);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 256);
	m_screen->set_visarea(0, 320-1, 0, 256-16-1);
	m_screen->set_screen_update(FUNC(fuuki16_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_fuuki16);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x4000 / 2);

	FUUKI_VIDEO(config, m_fuukivid, 0);
	m_fuukivid->set_palette(m_palette);
	m_fuukivid->set_color_base(0x400*2);
	m_fuukivid->set_color_num(0x40);
	m_fuukivid->set_colpri_callback(FUNC(fuuki16_state::fuuki16_colpri_cb));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	ym2203_device &ym1(YM2203(config, "ym1", XTAL(28'640'000) / 8)); /* 3.58 MHz */
	ym1.add_route(ALL_OUTPUTS, "mono", 0.15);

	ym3812_device &ym2(YM3812(config, "ym2", XTAL(28'640'000) / 8)); /* 3.58 MHz */
	ym2.irq_handler().set_inputline(m_audiocpu, 0);
	ym2.add_route(ALL_OUTPUTS, "mono", 0.30);

	OKIM6295(config, m_oki, XTAL(32'000'000) / 32, okim6295_device::PIN7_HIGH); /* 1 Mhz */
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.85);
}


/***************************************************************************

                                ROM Loading

***************************************************************************/

/***************************************************************************

                    Go! Go! Mile Smile / Susume! Mile Smile

(c)1995 Fuuki
FG-1C AI AM-2 (same board as Gyakuten Puzzle Banchou)

CPU  : TMP68HC000P-16
Sound: Z80 YM2203C YM3812 M6295 Y3014Bx2
OSC  : 32.00000MHz(OSC1) 28.64000MHz(OSC2) 12.000MHz(Xtal1)

ROMs:
fp2.rom2 - Main programs (27c4000)
fp1.rom1 /

lh538n1d.rom25 - Samples (Sharp mask, read as 27c8001)
fs1.rom24 - Sound program (27c010)

lh5370h8.rom11 - Sprites? (Sharp Mask, read as 27c160)
lh5370ha.rom12 |
lh5370h7.rom15 |
lh5370h9.rom16 /

lh537k2r.rom20 - Tiles? (Sharp Mask, read as 27c160)
lh5370hb.rom19 |
lh5370h6.rom3  /

Custom chips:
FI-002K (208pin PQFP, GA2)
FI-003K (208pin PQFP, GA3)


Others:
Mitsubishi M60067-0901FP 452100 (208pin PQFP, GA1)
5 GALs (GAL16V8B, not dumped)

Measured clocks:

    68k - 16mhz, OSC1 / 2
    Z80 - 6mhz, Xtal1 / 2
  M6295 - 1mhz,  OSC1 / 32
YM2203C - 3.58mhz, OSC2 / 8
 YM3812 - 3.58mhz, OSC2 / 8

***************************************************************************/

ROM_START( gogomile )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "fp2n.rom2", 0x000000, 0x080000, CRC(e73583a0) SHA1(05c6ee5cb2c151b32c462e8b920f9a57fb6cce5b) )
	ROM_LOAD16_BYTE( "fp1n.rom1", 0x000001, 0x080000, CRC(7b110824) SHA1(980e326d3b9e113ed522be3076663a249da4e739) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Z80 Code */
	ROM_LOAD( "fs1.rom24", 0x00000, 0x20000, CRC(4e4bd371) SHA1(429e776135ce8960e147762763d952d16ed3f9d4) )

	ROM_REGION( 0x200000, "fuukivid", 0 )   /* 16x16x4 Sprites */
	ROM_LOAD16_WORD_SWAP( "lh537k2r.rom20", 0x000000, 0x200000, CRC(525dbf51) SHA1(f21876676cc60ed65bc86884da894b24830826bb) )

	ROM_REGION( 0x200000, "gfx2", 0 )   /* 16x16x4 Tiles */
	ROM_LOAD16_WORD_SWAP( "lh5370h6.rom3", 0x000000, 0x200000, CRC(e2ca7107) SHA1(7174c2e1e2106275ad41b53af22651dca492367a) )  // x11xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x800000, "gfx3", 0 )   /* 16x16x8 Tiles */
	ROM_LOAD32_WORD_SWAP( "lh5370h7.rom15", 0x000000, 0x200000, CRC(34921680) SHA1(d9862f106caa14ea6ad925174e6bf2d542511593) )
	ROM_LOAD32_WORD_SWAP( "lh5370h8.rom11", 0x000002, 0x200000, CRC(9961c925) SHA1(c47b4f19f090527b3e0c04dd046aa9cd51ca0e16) )
	ROM_LOAD32_WORD_SWAP( "lh5370h9.rom16", 0x400000, 0x200000, CRC(e0118483) SHA1(36f9068e6c81c171b4426c3794277742bbc926f5) )
	ROM_LOAD32_WORD_SWAP( "lh5370ha.rom12", 0x400002, 0x200000, CRC(5f2a87de) SHA1(d7ed8f01b40aaf58126aaeee10ec7d948a144080) )

	ROM_REGION( 0x200000, "gfx4", 0 )   /* 16x16x4 Tiles */
	ROM_LOAD16_WORD_SWAP( "lh5370hb.rom19", 0x000000, 0x200000, CRC(bd1e896f) SHA1(075f7600cbced1d285cf32fc196844720eb12671) ) // FIRST AND SECOND HALF IDENTICAL

	/* 0x40000 * 4: sounds+speech (japanese), sounds+speech (english) */
	ROM_REGION( 0x100000, "oki", 0 )    /* Samples */
	ROM_LOAD( "lh538n1d.rom25", 0x000000, 0x100000, CRC(01622a95) SHA1(8d414bfc6dcfab1cf9cfe5738eb5c2ff31b77df6) ) // 0x40000 * 4
ROM_END

ROM_START( gogomileo )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "fp2.rom2", 0x000000, 0x080000, CRC(28fd3e4e) SHA1(3303e5759c0781035c74354587e1916719695754) )    // 1xxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "fp1.rom1", 0x000001, 0x080000, CRC(35a5fc45) SHA1(307207791cee7f40e88feffc5805ac25008a8566) )    // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Z80 Code */
	ROM_LOAD( "fs1.rom24", 0x00000, 0x20000, CRC(4e4bd371) SHA1(429e776135ce8960e147762763d952d16ed3f9d4) )

	ROM_REGION( 0x200000, "fuukivid", 0 )   /* 16x16x4 Sprites */
	ROM_LOAD16_WORD_SWAP( "lh537k2r.rom20", 0x000000, 0x200000, CRC(525dbf51) SHA1(f21876676cc60ed65bc86884da894b24830826bb) )

	ROM_REGION( 0x200000, "gfx2", 0 )   /* 16x16x4 Tiles */
	ROM_LOAD16_WORD_SWAP( "lh5370h6.rom3", 0x000000, 0x200000, CRC(e2ca7107) SHA1(7174c2e1e2106275ad41b53af22651dca492367a) )  // x11xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x800000, "gfx3", 0 )   /* 16x16x8 Tiles */
	ROM_LOAD32_WORD_SWAP( "lh5370h7.rom15", 0x000000, 0x200000, CRC(34921680) SHA1(d9862f106caa14ea6ad925174e6bf2d542511593) )
	ROM_LOAD32_WORD_SWAP( "lh5370h8.rom11", 0x000002, 0x200000, CRC(9961c925) SHA1(c47b4f19f090527b3e0c04dd046aa9cd51ca0e16) )
	ROM_LOAD32_WORD_SWAP( "lh5370h9.rom16", 0x400000, 0x200000, CRC(e0118483) SHA1(36f9068e6c81c171b4426c3794277742bbc926f5) )
	ROM_LOAD32_WORD_SWAP( "lh5370ha.rom12", 0x400002, 0x200000, CRC(5f2a87de) SHA1(d7ed8f01b40aaf58126aaeee10ec7d948a144080) )

	ROM_REGION( 0x200000, "gfx4", 0 )   /* 16x16x4 Tiles */
	ROM_LOAD16_WORD_SWAP( "lh5370hb.rom19", 0x000000, 0x200000, CRC(bd1e896f) SHA1(075f7600cbced1d285cf32fc196844720eb12671) ) // FIRST AND SECOND HALF IDENTICAL

	/* 0x40000 * 4: sounds+speech (japanese), sounds+speech (english) */
	ROM_REGION( 0x100000, "oki", 0 )    /* Samples */
	ROM_LOAD( "lh538n1d.rom25", 0x000000, 0x100000, CRC(01622a95) SHA1(8d414bfc6dcfab1cf9cfe5738eb5c2ff31b77df6) ) // 0x40000 * 4
ROM_END



/***************************************************************************

                            Gyakuten!! Puzzle Bancho

(c)1996 Fuuki
FG-1C AI AM-2

CPU  : TMP68HC000P-16
Sound: Z80 YM2203 YM3812 M6295
OSC  : 32.00000MHz(OSC1) 28.64000MHz(OSC2) 12.000MHz(Xtal1)

ROMs:
no1.rom2 - Main program (even)(27c4000)
no2.rom1 - Main program (odd) (27c4000)

no3.rom25 - Samples (27c2001)
no4.rom24 - Sound program (27c010)

61.rom11 - Graphics (Mask, read as 27c160)
59.rom15 |
58.rom20 |
60.rom3  /

Custom chips:
FI-002K (208pin PQFP, GA2)
FI-003K (208pin PQFP, GA3)

Others:
Mitsubishi M60067-0901FP 452100 (208pin PQFP, GA1)

***************************************************************************/

ROM_START( pbancho )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "no1.rom2", 0x000000, 0x080000, CRC(1b4fd178) SHA1(02cf3d2554b29cd253470d68ea959738f3b98dbe) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "no2,rom1", 0x000001, 0x080000, CRC(9cf510a5) SHA1(08e79b5bbd1c011c32f82dd15fba42d7898861be) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Z80 Code */
	ROM_LOAD( "no4.rom23", 0x00000, 0x20000, CRC(dfbfdb81) SHA1(84b0cbe843a9bbae43975afdbd029a9b76fd488b) )

	ROM_REGION( 0x200000, "fuukivid", 0 )   /* 16x16x4 Sprites */
	ROM_LOAD16_WORD_SWAP( "58.rom20", 0x000000, 0x200000, CRC(4dad0a2e) SHA1(a4f70557503110a5457b9096a79a5f249095fa55) )

	ROM_REGION( 0x200000, "gfx2", 0 )   /* 16x16x4 Tiles */
	ROM_LOAD16_WORD_SWAP( "60.rom3",  0x000000, 0x200000, CRC(a50a3c1b) SHA1(a2b30f9f83f5dc2e069d7559aefbda9929fc640c) )

	ROM_REGION( 0x400000, "gfx3", 0 )   /* 16x16x8 Tiles */
	ROM_LOAD32_WORD_SWAP( "59.rom15", 0x000000, 0x200000, CRC(b83dcb70) SHA1(b0b9df451535d85612fa095b4f694cf2e7930bca) )
	ROM_LOAD32_WORD_SWAP( "61.rom11", 0x000002, 0x200000, CRC(7f1213b9) SHA1(f8d6432b270c4d0954602e430ddd26841eb05656) )

	ROM_REGION( 0x200000, "gfx4", 0 )   /* 16x16x4 Tiles */
	ROM_LOAD16_WORD_SWAP( "60.rom3",  0x000000, 0x200000, CRC(a50a3c1b) SHA1(a2b30f9f83f5dc2e069d7559aefbda9929fc640c) )    // ?maybe?

	ROM_REGION( 0x040000, "oki", 0 )    /* Samples */
	ROM_LOAD( "n03.rom25", 0x000000, 0x040000, CRC(a7bfb5ea) SHA1(61937eae4f8855bc09c494aff52d76d41dc3b76a) )
ROM_END


/***************************************************************************


                                Game Drivers


***************************************************************************/

GAME( 1995, gogomile,  0,        fuuki16, gogomile,  fuuki16_state, empty_init, ROT0, "Fuuki", "Susume! Mile Smile / Go Go! Mile Smile (newer)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, gogomileo, gogomile, fuuki16, gogomileo, fuuki16_state, empty_init, ROT0, "Fuuki", "Susume! Mile Smile / Go Go! Mile Smile (older)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, pbancho,   0,        fuuki16, pbancho,   fuuki16_state, empty_init, ROT0, "Fuuki", "Gyakuten!! Puzzle Bancho (Japan)",               MACHINE_SUPPORTS_SAVE )

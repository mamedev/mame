// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Acho A. Tang, Nicola Salmoria
/***************************************************************************

    POW - Prisoners Of War (US version 1)    A7008   SNK 1988
    POW - Prisoners Of War (Japan)           A7008   SNK 1988
    SAR - Search And Rescue (World)          A8007   SNK 1989
    SAR - Search And Rescue (US)             A8007   SNK 1989
    SAR - Search And Rescue (Japan)          A8007   SNK 1989
    Street Smart (US version 1)              A8007   SNK 1989
    Street Smart (US version 2)              A7008   SNK 1989
    Street Smart (World version 1)           A8007   SNK 1989
    Street Smart (Japan version 1)           A8007   SNK 1989
    Ikari III - The Rescue (US)              A7007   SNK 1989

    For some strange reason version 2 of Street Smart runs on Pow hardware!

    Driver by Bryan McPhail, Acho A. Tang, Nicola Salmoria


Notes:
------
- All evidence suggests that the sprite hardware doesn't have a frame buffer
  but just a raster line buffer, like NeoGeo (unsurprisingly). The maths
  confirm this:
  384 pixels per raster line at 4 clocks per pixel = 1536 clocks per line
  96 sprites * 16 pixels per sprite  = 1536 clocks to draw the sprites

  While this board doesn't have a raster interrupt capability, the way how
  sprites are drawn needs to be kept in consideration because at least in one
  case the program modifies the sprite list in the middle of the frame:
  bug 00871: pow: At 3/4 of the 1st level, there is a large pillar, which pops up too late.
  The problem in this case is that the sprite list is built by the IRQ handler,
  however there is code in the main loop that clears some portions of sprite
  RAM under certain conditions. Usually, this isn't a problem, but in that
  specific point the sprites added by the IRQ handler are erased during the
  frame.
  To avoid glitches in that case, we force a partial screen update every time
  sprite RAM changes. It's possible that there are other unknown small glitches
  fixed by this (earlier notes in this driver talked about "sprite flickerings
  and pop-ups" but I don't know where they happened).

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/3812intf.h"
#include "includes/snk68.h"

/******************************************************************************/

void snk68_state::machine_start()
{
	save_item(NAME(m_invert_controls));
}

READ16_MEMBER(snk68_state::control_1_r)
{
	return (ioport("P1")->read() + (ioport("P2")->read() << 8));
}

READ16_MEMBER(snk68_state::rotary_1_r)
{
	return (( ~(1 << ioport("ROT1")->read()) )<<8)&0xff00;
}

READ16_MEMBER(snk68_state::rotary_2_r)
{
	return (( ~(1 << ioport("ROT2")->read()) )<<8)&0xff00;
}

READ16_MEMBER(snk68_state::rotary_lsb_r)
{
	return ((( ~(1 << ioport("ROT2")->read())  ) <<4)&0xf000)
			+ ((( ~(1 << ioport("ROT1")->read())  )    )&0x0f00);
}

READ16_MEMBER(snk68_state::protcontrols_r)
{
	static const char *const portnames[] = { "P1", "P2", "SYSTEM" };

	return ioport(portnames[offset])->read() ^ m_invert_controls;
}

WRITE16_MEMBER(snk68_state::protection_w)
{
	/* top byte is used, meaning unknown */
	/* bottom byte is protection in ikari 3 and streetsm */
	if (ACCESSING_BITS_0_7)
	{
		m_invert_controls = ((data & 0xff) == 0x07) ? 0xff : 0x00;
	}
}

WRITE16_MEMBER(snk68_state::sound_w)
{
	if (ACCESSING_BITS_8_15)
	{
		soundlatch_byte_w(space, 0, data >> 8);
		m_soundcpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}
}

/*******************************************************************************/

static ADDRESS_MAP_START( pow_map, AS_PROGRAM, 16, snk68_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x040000, 0x043fff) AM_RAM
	AM_RANGE(0x080000, 0x080001) AM_READ(control_1_r)
	AM_RANGE(0x080000, 0x080001) AM_WRITE(sound_w)
	AM_RANGE(0x0c0000, 0x0c0001) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x0c0000, 0x0c0001) AM_WRITE(pow_flipscreen_w)   // + char bank
	AM_RANGE(0x0e0000, 0x0e0001) AM_READNOP /* Watchdog or IRQ ack */
	AM_RANGE(0x0e8000, 0x0e8001) AM_READNOP /* Watchdog or IRQ ack */
	AM_RANGE(0x0f0000, 0x0f0001) AM_READ_PORT("DSW1")
	AM_RANGE(0x0f0008, 0x0f0009) AM_READ_PORT("DSW2")
//  AM_RANGE(0x0f0008, 0x0f0009) AM_WRITENOP    /* ?? */
	AM_RANGE(0x100000, 0x100fff) AM_READWRITE(pow_fg_videoram_r, pow_fg_videoram_w) AM_MIRROR(0x1000) AM_SHARE("pow_fg_videoram")   // 8-bit
	AM_RANGE(0x200000, 0x207fff) AM_DEVREADWRITE("sprites", snk68_spr_device, spriteram_r, spriteram_w) AM_SHARE("spriteram")   // only partially populated
	AM_RANGE(0x400000, 0x400fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
ADDRESS_MAP_END

static ADDRESS_MAP_START( searchar_map, AS_PROGRAM, 16, snk68_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x040000, 0x043fff) AM_RAM
	AM_RANGE(0x080000, 0x080005) AM_READ(protcontrols_r) /* Player 1 & 2 */
	AM_RANGE(0x080000, 0x080001) AM_WRITE(sound_w)
	AM_RANGE(0x080006, 0x080007) AM_WRITE(protection_w) /* top byte unknown, bottom is protection in ikari3 and streetsm */
	AM_RANGE(0x0c0000, 0x0c0001) AM_WRITE(searchar_flipscreen_w)
	AM_RANGE(0x0c0000, 0x0c0001) AM_READ(rotary_1_r) /* Player 1 rotary */
	AM_RANGE(0x0c8000, 0x0c8001) AM_READ(rotary_2_r) /* Player 2 rotary */
	AM_RANGE(0x0d0000, 0x0d0001) AM_READ(rotary_lsb_r) /* Extra rotary bits */
	AM_RANGE(0x0e0000, 0x0e0001) AM_READNOP /* Watchdog or IRQ ack */
	AM_RANGE(0x0e8000, 0x0e8001) AM_READNOP /* Watchdog or IRQ ack */
//  AM_RANGE(0x0f0000, 0x0f0001) AM_WRITENOP    /* ?? */
	AM_RANGE(0x0f0000, 0x0f0001) AM_READ_PORT("DSW1")
	AM_RANGE(0x0f0008, 0x0f0009) AM_READ_PORT("DSW2")
	AM_RANGE(0x0f8000, 0x0f8001) AM_READ8(soundlatch2_byte_r, 0xff00)
	AM_RANGE(0x100000, 0x107fff) AM_DEVREADWRITE("sprites", snk68_spr_device, spriteram_r, spriteram_w) AM_SHARE("spriteram")   // only partially populated
	AM_RANGE(0x200000, 0x200fff) AM_RAM_WRITE(searchar_fg_videoram_w) AM_MIRROR(0x1000) AM_SHARE("pow_fg_videoram") /* Mirror is used by Ikari 3 */
	AM_RANGE(0x300000, 0x33ffff) AM_ROM AM_REGION("user1", 0) /* Extra code bank */
	AM_RANGE(0x400000, 0x400fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
ADDRESS_MAP_END

/******************************************************************************/

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, snk68_state )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xf800) AM_READ(soundlatch_byte_r) AM_WRITE(soundlatch2_byte_w)
ADDRESS_MAP_END

WRITE8_MEMBER(snk68_state::D7759_write_port_0_w)
{
	m_upd7759->port_w(space, 0, data);
	m_upd7759->start_w(0);
	m_upd7759->start_w(1);
}

WRITE8_MEMBER(snk68_state::D7759_upd_reset_w)
{
	m_upd7759->reset_w(data & 0x80);
}

static ADDRESS_MAP_START( sound_io_map, AS_IO, 8, snk68_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVREADWRITE("ymsnd", ym3812_device, status_port_r, control_port_w)
	AM_RANGE(0x20, 0x20) AM_DEVWRITE("ymsnd", ym3812_device, write_port_w)
	AM_RANGE(0x40, 0x40) AM_WRITE(D7759_write_port_0_w)
	AM_RANGE(0x80, 0x80) AM_WRITE(D7759_upd_reset_w)
ADDRESS_MAP_END

/******************************************************************************/

static INPUT_PORTS_START( pow )
	PORT_START("P1")    /* Player 1 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")    /* Player 2 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE )    /* same as the service mode dsw */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0300, 0x0000, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:!8,!7")
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0c00, 0x0000, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:!6,!5")
	PORT_DIPSETTING(      0x0c00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x1000, "3" )
	PORT_DIPNAME( 0x2000, 0x0000, "Bonus Occurrence" )      PORT_DIPLOCATION("SW1:!3")
	PORT_DIPSETTING(      0x0000, "1st & 2nd only" )
	PORT_DIPSETTING(      0x2000, "1st & every 2nd" )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Language ) )     PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_DIPLOC(  0x0100, IP_ACTIVE_HIGH, "SW2:!8" )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(      0x0200, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0c00, 0x0000, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:!6,!5")
	PORT_DIPSETTING(      0x0000, "20k 50k" )
	PORT_DIPSETTING(      0x0800, "40k 100k" )
	PORT_DIPSETTING(      0x0400, "60k 150k" )
	PORT_DIPSETTING(      0x0c00, DEF_STR( None ) )
	PORT_DIPNAME( 0x3000, 0x0000, "Game Mode" )         PORT_DIPLOCATION("SW2:!4,!3")
	PORT_DIPSETTING(      0x2000, "Demo Sounds Off" )
	PORT_DIPSETTING(      0x0000, "Demo Sounds On" )
	PORT_DIPSETTING(      0x3000, "Freeze" )
	PORT_DIPSETTING(      0x1000, "Infinite Lives (Cheat)")
	PORT_DIPNAME( 0xc000, 0x0000, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:!2,!1")
	PORT_DIPSETTING(      0x8000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( Hardest ) )
INPUT_PORTS_END

/* Identical to pow, but the Language dip switch has no effect */
static INPUT_PORTS_START( powj )
	PORT_INCLUDE( pow )

	PORT_MODIFY("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x4000, IP_ACTIVE_HIGH, "SW1:!2" )
INPUT_PORTS_END


static INPUT_PORTS_START( searchar )
	PORT_START("P1")    /* Player 1 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")    /* Player 2 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE )    /* same as the service mode dsw */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Joystick ) )     PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(      0x0000, "Rotary Joystick" )
	PORT_DIPSETTING(      0x0100, "Standard Joystick" )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c00, 0x0000, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!6,!5")
	PORT_DIPSETTING(      0x0800, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPSETTING(      0x0400, "4" )
	PORT_DIPSETTING(      0x0c00, "5" )
	PORT_DIPNAME( 0x3000, 0x0000, "Coin A & B" )            PORT_DIPLOCATION("SW1:!4,!3")
	PORT_DIPSETTING(      0x2000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x4000, 0x0000, "Bonus Occurrence" )      PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(      0x0000, "1st & 2nd only" )
	PORT_DIPSETTING(      0x4000, "1st & every 2nd" )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_DIPLOC(  0x0100, IP_ACTIVE_HIGH, "SW2:!8" )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(      0x0200, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0c00, 0x0000, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:!6,!5")
	PORT_DIPSETTING(      0x0000, "50k 200k" )
	PORT_DIPSETTING(      0x0800, "70k 270k" )
	PORT_DIPSETTING(      0x0400, "90k 350k" )
	PORT_DIPSETTING(      0x0c00, DEF_STR( None ) )
	PORT_DIPNAME( 0x3000, 0x0000, "Game Mode" )         PORT_DIPLOCATION("SW2:!4,!3")
	PORT_DIPSETTING(      0x2000, "Demo Sounds Off" )
	PORT_DIPSETTING(      0x0000, "Demo Sounds On" )
	PORT_DIPSETTING(      0x3000, "Freeze" )
	PORT_DIPSETTING(      0x1000, "Infinite Lives (Cheat)")
	PORT_DIPNAME( 0xc000, 0x0000, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:!2,!1")
	PORT_DIPSETTING(      0x8000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( Hardest ) )

	PORT_START("ROT1")  /* player 1 12-way rotary control  */
	PORT_BIT( 0x0f, 0x00, IPT_POSITIONAL )  PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(5) PORT_KEYDELTA(5) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X) PORT_REVERSE PORT_FULL_TURN_COUNT(12)

	PORT_START("ROT2")  /* player 2 12-way rotary control */
	PORT_BIT( 0x0f, 0x00, IPT_POSITIONAL )  PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(5) PORT_KEYDELTA(5) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_M) PORT_PLAYER(2) PORT_REVERSE PORT_FULL_TURN_COUNT(12)
INPUT_PORTS_END


static INPUT_PORTS_START( streetsm )
	PORT_START("P1")    /* Player 1 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")    /* Player 2 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE )    /* same as the service mode dsw */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")  /* Dip switches (Active high) */
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0300, 0x0000, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!8,7")
	PORT_DIPSETTING(      0x0200, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0100, "3" )
	PORT_DIPSETTING(      0x0300, "4" )
	PORT_DIPNAME( 0x0c00, 0x0000, "Coin A & B" )            PORT_DIPLOCATION("SW1:!6,5")
	PORT_DIPSETTING(      0x0800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, "Bonus Occurrence" )      PORT_DIPLOCATION("SW1:!3")
	PORT_DIPSETTING(      0x0000, "1st & 2nd only" )
	PORT_DIPSETTING(      0x2000, "1st & every 2nd" )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("DSW2") /* Dip switches (Active high) */
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_DIPLOC(  0x0100, IP_ACTIVE_HIGH, "SW2:!8" )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(      0x0200, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0c00, 0x0000, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:!6,!5")
	PORT_DIPSETTING(      0x0000, "200k 400k" )
	PORT_DIPSETTING(      0x0800, "400k 600k" )
	PORT_DIPSETTING(      0x0400, "600k 800k" )
	PORT_DIPSETTING(      0x0c00, DEF_STR( None ) )
	PORT_DIPNAME( 0x3000, 0x0000, "Game Mode" )         PORT_DIPLOCATION("SW2:!4,!3")
	PORT_DIPSETTING(      0x2000, "Demo Sounds Off" )
	PORT_DIPSETTING(      0x0000, "Demo Sounds On" )
	PORT_DIPSETTING(      0x3000, "Freeze" )
	PORT_DIPSETTING(      0x1000, "Infinite Lives (Cheat)")
	PORT_DIPNAME( 0xc000, 0x0000, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:!2,!1")
	PORT_DIPSETTING(      0x8000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( Hardest ) )

	PORT_START("ROT1")  /* player 1 12-way rotary control - not used in this game */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROT2")  /* player 2 12-way rotary control - not used in this game */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/* Same as streetsm, but Coinage is different */
static INPUT_PORTS_START( streetsj )
	PORT_INCLUDE( streetsm )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x0c00, 0x0000, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!6,5")
	PORT_DIPSETTING(      0x0c00, "A 4/1 B 1/4" )
	PORT_DIPSETTING(      0x0400, "A 3/1 B 1/3" )
	PORT_DIPSETTING(      0x0800, "A 2/1 B 1/2" )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
INPUT_PORTS_END


static INPUT_PORTS_START( ikari3 )
	PORT_START("P1")    /* Player 1 controls, maybe all are active_high? */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")    /* Player 2 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE )    /* same as the service mode dsw */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")  /* Dip switches (Active high) */
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0300, 0x0000, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!8,!7")
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPSETTING(      0x0100, "4" )
	PORT_DIPSETTING(      0x0300, "5" )
	PORT_DIPNAME( 0x0c00, 0x0000, "Coin A & B" )            PORT_DIPLOCATION("SW1:!6,!5")
	PORT_DIPSETTING(      0x0800, "First 2 Coins/1 Credit then 1/1" )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0400, "First 1 Coin/2 Credits then 1/1" )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, "Bonus Occurrence" )      PORT_DIPLOCATION("SW1:!3")
	PORT_DIPSETTING(      0x0000, "1st & 2nd only" )
	PORT_DIPSETTING(      0x2000, "1st & every 2nd" )
	PORT_DIPNAME( 0x4000, 0x0000, "Blood" )             PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("DSW2") /* Dip switches (Active high) */
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_DIPLOC(  0x0100, IP_ACTIVE_HIGH, "SW2:!8" )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(      0x0200, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0c00, 0x0000, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:!6,!5")
	PORT_DIPSETTING(      0x0000, "20k 50k" )
	PORT_DIPSETTING(      0x0800, "40k 100k" )
	PORT_DIPSETTING(      0x0400, "60k 150k" )
	PORT_DIPSETTING(      0x0c00, DEF_STR( None ) )
	PORT_DIPNAME( 0x3000, 0x0000, "Game Mode" )         PORT_DIPLOCATION("SW2:!4,!3")
	PORT_DIPSETTING(      0x2000, "Demo Sounds Off" )
	PORT_DIPSETTING(      0x0000, "Demo Sounds On" )
	PORT_DIPSETTING(      0x3000, "Freeze" )
	PORT_DIPSETTING(      0x1000, "Infinite Lives (Cheat)")
	PORT_DIPNAME( 0xc000, 0x8000, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:!2,!1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( Hardest ) )

	PORT_START("ROT1")  /* player 1 12-way rotary control */
	PORT_BIT( 0x0f, 0x00, IPT_POSITIONAL )  PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(5) PORT_KEYDELTA(5) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X) PORT_REVERSE PORT_FULL_TURN_COUNT(12)

	PORT_START("ROT2")  /* player 2 12-way rotary control */
	PORT_BIT( 0x0f, 0x00, IPT_POSITIONAL )  PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(5) PORT_KEYDELTA(5) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_M) PORT_PLAYER(2) PORT_REVERSE PORT_FULL_TURN_COUNT(12)
INPUT_PORTS_END

/******************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ 0, 4, RGN_FRAC(1,2), RGN_FRAC(1,2)+4 },
	{ STEP4(8*8+3,-1), STEP4(3,-1) },
	{ STEP8(0,8) },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 0, 8, RGN_FRAC(1,2), RGN_FRAC(1,2)+8 },
	{ STEP8(32*8+7,-1), STEP8(7,-1) },
	{ STEP16(0,16) },
	64*8
};

static GFXDECODE_START( pow )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0,  0x80>>4 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0, 0x800>>4 )
GFXDECODE_END

/******************************************************************************/

// pow has 0x4000 tiles and independent x/y flipping
// the other games have > 0x4000 tiles and flipping in only one direction
// (globally selected)
void snk68_state::tile_callback_pow(int &tile, int& fx, int& fy, int& region)
{
	fx = tile & 0x4000;
	fy = tile & 0x8000;
	tile &= 0x3fff;
	region = 1;
}

void snk68_state::tile_callback_notpow(int &tile, int& fx, int& fy, int& region)
{
	if (m_sprite_flip_axis)
	{
		fx = 0;
		fy = tile & 0x8000;
	}
	else
	{
		fx = tile & 0x8000;
		fy = 0;
	}
	tile &= 0x7fff;
	region = 1;
}

static MACHINE_CONFIG_START( pow, snk68_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_18MHz/2) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(pow_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", snk68_state,  irq1_line_hold)

	MCFG_CPU_ADD("soundcpu", Z80, XTAL_8MHz/2) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_io_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	// the screen parameters are guessed but should be accurate. They
	// give a theoretical refresh rate of 59.1856Hz while the measured
	// rate on a SAR board is 59.16Hz.
	MCFG_SCREEN_RAW_PARAMS(XTAL_24MHz/4, 384, 0, 256, 264, 16, 240)
	MCFG_SCREEN_UPDATE_DRIVER(snk68_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pow)
	MCFG_PALETTE_ADD("palette", 0x800)
	MCFG_PALETTE_FORMAT(xRGBRRRRGGGGBBBB_bit0)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_DEVICE_ADD("sprites", SNK68_SPR, 0)
	MCFG_SNK68_SPR_GFXDECODE("gfxdecode")
	MCFG_SNK68_SPR_SET_TILE_INDIRECT( snk68_state, tile_callback_pow )

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_8MHz/2) /* verified on pcb  */
	MCFG_YM3812_IRQ_HANDLER(INPUTLINE("soundcpu", 0))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("upd", UPD7759, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( searchar, pow )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(searchar_map)

	MCFG_VIDEO_START_OVERRIDE(snk68_state,searchar)

	MCFG_DEVICE_MODIFY("sprites")
	MCFG_SNK68_SPR_SET_TILE_INDIRECT( snk68_state, tile_callback_notpow )
MACHINE_CONFIG_END


/******************************************************************************/

ROM_START( pow )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "dg1ver1.j14",   0x000000, 0x20000, CRC(8e71a8af) SHA1(72c2eb2316c2684491331e8adabcb2be084aa6a2) )
	ROM_LOAD16_BYTE( "dg2ver1.l14",   0x000001, 0x20000, CRC(4287affc) SHA1(59dfb37296edd3b42231319a9f4df819d384db38) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Sound CPU */
	ROM_LOAD( "dg8.e25",   0x000000, 0x10000, CRC(d1d61da3) SHA1(4e78643f8a7d44db3ff091acb0a5da1cc836e3cb) )

	ROM_REGION( 0x010000, "gfx1", 0 )   /* characters */
	ROM_LOAD( "dg9.l25",   0x000000, 0x08000, CRC(df864a08) SHA1(dd996070077efbbf9d784299b6563cab258e4a8e) )
	ROM_LOAD( "dg10.m25",  0x008000, 0x08000, CRC(9e470d53) SHA1(f7dc6ac39ade573480e87170a2781f0f72930580) )

	ROM_REGION( 0x200000, "gfx2", 0 )   /* sprites */
	ROM_LOAD16_BYTE( "snk880.11a", 0x000000, 0x20000, CRC(e70fd906) SHA1(b9e734c074ee1c8ae73e6041d739ab30d2df7d62) )
	ROM_LOAD16_BYTE( "snk880.15a", 0x000001, 0x20000, CRC(7a90e957) SHA1(9650d7cdbcbbbcdd7f75a1c3c08a195aa456e169) )
	ROM_LOAD16_BYTE( "snk880.12a", 0x040000, 0x20000, CRC(628b1aed) SHA1(1065ef835da03f7d9776e81c225c3ecdd2affae2) )
	ROM_LOAD16_BYTE( "snk880.16a", 0x040001, 0x20000, CRC(e40a6c13) SHA1(7ad9dfc65f8c8b316933f0fdd3bc7a243d6eff65) )
	ROM_LOAD16_BYTE( "snk880.13a", 0x080000, 0x20000, CRC(19dc8868) SHA1(82aaf93fc8f4b3bf947d373d0f41cc0044207c34) ) /* Also can be labeled as DG13 */
	ROM_LOAD16_BYTE( "snk880.17a", 0x080001, 0x20000, CRC(c7931cc2) SHA1(908313d9b7fa4395b5fb79925d068e4f5d354b21) )
	ROM_LOAD16_BYTE( "snk880.14a", 0x0c0000, 0x20000, CRC(47cd498b) SHA1(7fbc007f2d817c26af02fef233f5e0681a17052a) )
	ROM_LOAD16_BYTE( "snk880.18a", 0x0c0001, 0x20000, CRC(eed72232) SHA1(ad614e752cf1d3eac9a04cbc90435f988e90ace7) )
	ROM_LOAD16_BYTE( "snk880.19a", 0x100000, 0x20000, CRC(1775b8dd) SHA1(c01154749379be6e18baa99f4d94d97942f3dd85) )
	ROM_LOAD16_BYTE( "snk880.23a", 0x100001, 0x20000, CRC(adb6ad68) SHA1(ed4323d2dfa3efaa496b17f4719f9566d56725e5) )
	ROM_LOAD16_BYTE( "snk880.20a", 0x140000, 0x20000, CRC(f8e752ec) SHA1(1e1e178303f9af84cbb15249c49a870193ef805f) )
	ROM_LOAD16_BYTE( "snk880.24a", 0x140001, 0x20000, CRC(dd41865a) SHA1(c86f14342beca896784b88920d9e0879af4179ab) )
	ROM_LOAD16_BYTE( "snk880.21a", 0x180000, 0x20000, CRC(27e9fffe) SHA1(e8058db40832b986c5addd22dd69b0308d10ec71) )
	ROM_LOAD16_BYTE( "snk880.25a", 0x180001, 0x20000, CRC(055759ad) SHA1(f9b12320f142075d49d447fb107af99272567d58) )
	ROM_LOAD16_BYTE( "snk880.22a", 0x1c0000, 0x20000, CRC(aa9c00d8) SHA1(1017ed1cc036c6084b71204a998fd05557a6e59f) )
	ROM_LOAD16_BYTE( "snk880.26a", 0x1c0001, 0x20000, CRC(9bc261c5) SHA1(f07fef465191d48ccc149d1a62e6382d3fc0ef9f) )

	ROM_REGION( 0x10000, "upd", 0 ) /* UPD7759 samples */
	ROM_LOAD( "dg7.d20",  0x000000, 0x10000, CRC(aba9a9d3) SHA1(5098cd3a064b8ede24797de8879a277d79e79d75) )

	ROM_REGION( 0x0100, "plds", 0 )
	ROM_LOAD( "pal20l10.a6", 0x0000, 0x00cc, CRC(c3d9e729) SHA1(f05f03eecf12b4d0793124ecd3195307be04046b) )
ROM_END

ROM_START( powj )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1-2",   0x000000, 0x20000, CRC(2f17bfb0) SHA1(8be18990829eb2586c00b9e8b35e8779bc48296a) )
	ROM_LOAD16_BYTE( "2-2",   0x000001, 0x20000, CRC(baa32354) SHA1(a235b82527dc025e699ba2e8e9797dac15ea9440) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Sound CPU */
	ROM_LOAD( "dg8.e25",   0x000000, 0x10000, CRC(d1d61da3) SHA1(4e78643f8a7d44db3ff091acb0a5da1cc836e3cb) )

	ROM_REGION( 0x010000, "gfx1", 0 )   /* characters */
	ROM_LOAD( "dg9.l25",   0x000000, 0x08000, CRC(df864a08) SHA1(dd996070077efbbf9d784299b6563cab258e4a8e) )
	ROM_LOAD( "dg10.m25",  0x008000, 0x08000, CRC(9e470d53) SHA1(f7dc6ac39ade573480e87170a2781f0f72930580) )

	ROM_REGION( 0x200000, "gfx2", 0 )   /* sprites */
	ROM_LOAD16_BYTE( "snk880.11a", 0x000000, 0x20000, CRC(e70fd906) SHA1(b9e734c074ee1c8ae73e6041d739ab30d2df7d62) )
	ROM_LOAD16_BYTE( "snk880.15a", 0x000001, 0x20000, CRC(7a90e957) SHA1(9650d7cdbcbbbcdd7f75a1c3c08a195aa456e169) )
	ROM_LOAD16_BYTE( "snk880.12a", 0x040000, 0x20000, CRC(628b1aed) SHA1(1065ef835da03f7d9776e81c225c3ecdd2affae2) )
	ROM_LOAD16_BYTE( "snk880.16a", 0x040001, 0x20000, CRC(e40a6c13) SHA1(7ad9dfc65f8c8b316933f0fdd3bc7a243d6eff65) )
	ROM_LOAD16_BYTE( "snk880.13a", 0x080000, 0x20000, CRC(19dc8868) SHA1(82aaf93fc8f4b3bf947d373d0f41cc0044207c34) ) /* Also can be labeled as DG13 */
	ROM_LOAD16_BYTE( "snk880.17a", 0x080001, 0x20000, CRC(c7931cc2) SHA1(908313d9b7fa4395b5fb79925d068e4f5d354b21) )
	ROM_LOAD16_BYTE( "snk880.14a", 0x0c0000, 0x20000, CRC(47cd498b) SHA1(7fbc007f2d817c26af02fef233f5e0681a17052a) )
	ROM_LOAD16_BYTE( "snk880.18a", 0x0c0001, 0x20000, CRC(eed72232) SHA1(ad614e752cf1d3eac9a04cbc90435f988e90ace7) )
	ROM_LOAD16_BYTE( "snk880.19a", 0x100000, 0x20000, CRC(1775b8dd) SHA1(c01154749379be6e18baa99f4d94d97942f3dd85) )
	ROM_LOAD16_BYTE( "snk880.23a", 0x100001, 0x20000, CRC(adb6ad68) SHA1(ed4323d2dfa3efaa496b17f4719f9566d56725e5) )
	ROM_LOAD16_BYTE( "snk880.20a", 0x140000, 0x20000, CRC(f8e752ec) SHA1(1e1e178303f9af84cbb15249c49a870193ef805f) )
	ROM_LOAD16_BYTE( "snk880.24a", 0x140001, 0x20000, CRC(dd41865a) SHA1(c86f14342beca896784b88920d9e0879af4179ab) )
	ROM_LOAD16_BYTE( "snk880.21a", 0x180000, 0x20000, CRC(27e9fffe) SHA1(e8058db40832b986c5addd22dd69b0308d10ec71) )
	ROM_LOAD16_BYTE( "snk880.25a", 0x180001, 0x20000, CRC(055759ad) SHA1(f9b12320f142075d49d447fb107af99272567d58) )
	ROM_LOAD16_BYTE( "snk880.22a", 0x1c0000, 0x20000, CRC(aa9c00d8) SHA1(1017ed1cc036c6084b71204a998fd05557a6e59f) )
	ROM_LOAD16_BYTE( "snk880.26a", 0x1c0001, 0x20000, CRC(9bc261c5) SHA1(f07fef465191d48ccc149d1a62e6382d3fc0ef9f) )

	ROM_REGION( 0x10000, "upd", 0 ) /* UPD7759 samples */
	ROM_LOAD( "dg7.d20",  0x000000, 0x10000, CRC(aba9a9d3) SHA1(5098cd3a064b8ede24797de8879a277d79e79d75) )

	ROM_REGION( 0x0100, "plds", 0 )
	ROM_LOAD( "pal20l10.a6", 0x0000, 0x00cc, CRC(c3d9e729) SHA1(f05f03eecf12b4d0793124ecd3195307be04046b) )
ROM_END

ROM_START( streetsm )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "s2-1ver2.14h", 0x00000, 0x20000, CRC(655f4773) SHA1(5374a6cf0b895c5ff839b0f52402df4cc53241cf) )
	ROM_LOAD16_BYTE( "s2-2ver2.14k", 0x00001, 0x20000, CRC(efae4823) SHA1(f3be25b76cf13feeaaaf0e9640c30a6a7371f108) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Sound CPU */
	ROM_LOAD( "s2-5.16c",    0x000000, 0x10000, CRC(ca4b171e) SHA1(a05fd81f68759a09be3ec09f38d7c9364dfb6c14) )

	ROM_REGION( 0x010000, "gfx1", 0 )   /* characters */
	ROM_LOAD( "s2-9.25l",    0x000000, 0x08000, CRC(09b6ac67) SHA1(0b1ef51d9cd755eacc25b33360811cc86c32c0b7) )
	ROM_LOAD( "s2-10.25m",   0x008000, 0x08000, CRC(89e4ee6f) SHA1(21797286836ad71d2497e3e6d4df1fbe545562ab) )

	ROM_REGION( 0x400000, "gfx2", 0 )   /* sprites */
	ROM_LOAD( "stsmart.900", 0x000000, 0x80000, CRC(a8279a7e) SHA1(244bdacb29b00f71da93ed8ddddbcffcce110be8) )
	ROM_LOAD( "stsmart.902", 0x080000, 0x80000, CRC(2f021aa1) SHA1(699d0b5ac79e34e4fc4cef70eb448f21f1c3e9e2) )
	ROM_LOAD( "stsmart.904", 0x100000, 0x80000, CRC(167346f7) SHA1(fb4ea412622245db49ec15449ee4fa0d90922f06) )
	// 180000-1fffff empty
	ROM_LOAD( "stsmart.901", 0x200000, 0x80000, CRC(c305af12) SHA1(18b5d448fe9608efcd2e5bb8faa24808d1489ec8) )
	ROM_LOAD( "stsmart.903", 0x280000, 0x80000, CRC(73c16d35) SHA1(40cf7a58926c649f89b08917afb35b08918d1a0f) )
	ROM_LOAD( "stsmart.905", 0x300000, 0x80000, CRC(a5beb4e2) SHA1(c26b7eee2ca32bd73fb7a09c6ef52c2ae1c7fc1c) )
	// 380000-3fffff empty

	ROM_REGION( 0x20000, "upd", 0 ) /* UPD7759 samples */
	ROM_LOAD( "s2-6.18d",    0x000000, 0x20000, CRC(47db1605) SHA1(ae00e633eb98567f04ff97e3d63e04e049d955ec) )

	ROM_REGION( 0x0100, "plds", 0 )
	ROM_LOAD( "pl20l10a.1n", 0x0000, 0x00cc, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( streetsm1 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "s2-1ver1.9c",  0x00000, 0x20000, CRC(b59354c5) SHA1(086c87541d422f90bdaad8d63b14d0d520c12564) )
	ROM_LOAD16_BYTE( "s2-2ver1.10c", 0x00001, 0x20000, CRC(e448b68b) SHA1(08d674ab3d9bd3d3b1d50967a56fa6a002ce0b8d) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Sound CPU */
	ROM_LOAD( "s2-5.16c",    0x000000, 0x10000, CRC(ca4b171e) SHA1(a05fd81f68759a09be3ec09f38d7c9364dfb6c14) )

	ROM_REGION( 0x010000, "gfx1", 0 )   /* characters */
	ROM_LOAD( "s2-7.15l",    0x000000, 0x08000, CRC(22bedfe5) SHA1(64efb2281c32afe5a06f35cce939e6a53226c6ed) )
	ROM_LOAD( "s2-8.15m",    0x008000, 0x08000, CRC(6a1c70ab) SHA1(019538ddcb713d0810b26b6aa65f6e4596931621) )

	ROM_REGION( 0x400000, "gfx2", 0 )   /* sprites */
	ROM_LOAD( "stsmart.900", 0x000000, 0x80000, CRC(a8279a7e) SHA1(244bdacb29b00f71da93ed8ddddbcffcce110be8) )
	ROM_LOAD( "stsmart.902", 0x080000, 0x80000, CRC(2f021aa1) SHA1(699d0b5ac79e34e4fc4cef70eb448f21f1c3e9e2) )
	ROM_LOAD( "stsmart.904", 0x100000, 0x80000, CRC(167346f7) SHA1(fb4ea412622245db49ec15449ee4fa0d90922f06) )
	// 180000-1fffff empty
	ROM_LOAD( "stsmart.901", 0x200000, 0x80000, CRC(c305af12) SHA1(18b5d448fe9608efcd2e5bb8faa24808d1489ec8) )
	ROM_LOAD( "stsmart.903", 0x280000, 0x80000, CRC(73c16d35) SHA1(40cf7a58926c649f89b08917afb35b08918d1a0f) )
	ROM_LOAD( "stsmart.905", 0x300000, 0x80000, CRC(a5beb4e2) SHA1(c26b7eee2ca32bd73fb7a09c6ef52c2ae1c7fc1c) )
	// 380000-3fffff empty

	ROM_REGION( 0x20000, "upd", 0 ) /* UPD7759 samples */
	ROM_LOAD( "s2-6.18d",    0x000000, 0x20000, CRC(47db1605) SHA1(ae00e633eb98567f04ff97e3d63e04e049d955ec) )

	ROM_REGION16_BE( 0x40000, "user1", ROMREGION_ERASE00 ) /* Extra code bank */
ROM_END

ROM_START( streetsmw )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "s-smart1.bin", 0x00000, 0x20000, CRC(a1f5ceab) SHA1(74f5a4288618fbce6ed3dc75b6ccfa695396193c) )
	ROM_LOAD16_BYTE( "s-smart2.bin", 0x00001, 0x20000, CRC(263f615d) SHA1(4576f9d2abb31ecf747a5075716579e75613d57c) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Sound CPU */
	ROM_LOAD( "s2-5.16c",    0x000000, 0x10000, CRC(ca4b171e) SHA1(a05fd81f68759a09be3ec09f38d7c9364dfb6c14) )

	ROM_REGION( 0x010000, "gfx1", 0 )   /* characters */
	ROM_LOAD( "s2-7.15l",    0x000000, 0x08000, CRC(22bedfe5) SHA1(64efb2281c32afe5a06f35cce939e6a53226c6ed) )
	ROM_LOAD( "s2-8.15m",    0x008000, 0x08000, CRC(6a1c70ab) SHA1(019538ddcb713d0810b26b6aa65f6e4596931621) )

	ROM_REGION( 0x400000, "gfx2", 0 )   /* sprites */
	ROM_LOAD( "stsmart.900", 0x000000, 0x80000, CRC(a8279a7e) SHA1(244bdacb29b00f71da93ed8ddddbcffcce110be8) )
	ROM_LOAD( "stsmart.902", 0x080000, 0x80000, CRC(2f021aa1) SHA1(699d0b5ac79e34e4fc4cef70eb448f21f1c3e9e2) )
	ROM_LOAD( "stsmart.904", 0x100000, 0x80000, CRC(167346f7) SHA1(fb4ea412622245db49ec15449ee4fa0d90922f06) )
	// 180000-1fffff empty
	ROM_LOAD( "stsmart.901", 0x200000, 0x80000, CRC(c305af12) SHA1(18b5d448fe9608efcd2e5bb8faa24808d1489ec8) )
	ROM_LOAD( "stsmart.903", 0x280000, 0x80000, CRC(73c16d35) SHA1(40cf7a58926c649f89b08917afb35b08918d1a0f) )
	ROM_LOAD( "stsmart.905", 0x300000, 0x80000, CRC(a5beb4e2) SHA1(c26b7eee2ca32bd73fb7a09c6ef52c2ae1c7fc1c) )
	// 380000-3fffff empty

	ROM_REGION( 0x20000, "upd", 0 ) /* UPD7759 samples */
	ROM_LOAD( "s2-6.18d",    0x000000, 0x20000, CRC(47db1605) SHA1(ae00e633eb98567f04ff97e3d63e04e049d955ec) )

	ROM_REGION16_BE( 0x40000, "user1", ROMREGION_ERASE00 ) /* Extra code bank */
ROM_END

ROM_START( streetsmj )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "s2v1j_01.bin", 0x00000, 0x20000, CRC(f031413c) SHA1(5d7dfeac03f786736914f047c28a7a0488175176) )
	ROM_LOAD16_BYTE( "s2v1j_02.bin", 0x00001, 0x20000, CRC(e403a40b) SHA1(e740848d716586737eff6e3c201fb3e3da048a09) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Sound CPU */
	ROM_LOAD( "s2-5.16c",    0x000000, 0x10000, CRC(ca4b171e) SHA1(a05fd81f68759a09be3ec09f38d7c9364dfb6c14) )

	ROM_REGION( 0x010000, "gfx1", 0 )   /* characters */
	ROM_LOAD( "s2-7.15l",    0x000000, 0x08000, CRC(22bedfe5) SHA1(64efb2281c32afe5a06f35cce939e6a53226c6ed) )
	ROM_LOAD( "s2-8.15m",    0x008000, 0x08000, CRC(6a1c70ab) SHA1(019538ddcb713d0810b26b6aa65f6e4596931621) )

	ROM_REGION( 0x400000, "gfx2", 0 )   /* sprites */
	ROM_LOAD( "stsmart.900", 0x000000, 0x80000, CRC(a8279a7e) SHA1(244bdacb29b00f71da93ed8ddddbcffcce110be8) )
	ROM_LOAD( "stsmart.902", 0x080000, 0x80000, CRC(2f021aa1) SHA1(699d0b5ac79e34e4fc4cef70eb448f21f1c3e9e2) )
	ROM_LOAD( "stsmart.904", 0x100000, 0x80000, CRC(167346f7) SHA1(fb4ea412622245db49ec15449ee4fa0d90922f06) )
	// 180000-1fffff empty
	ROM_LOAD( "stsmart.901", 0x200000, 0x80000, CRC(c305af12) SHA1(18b5d448fe9608efcd2e5bb8faa24808d1489ec8) )
	ROM_LOAD( "stsmart.903", 0x280000, 0x80000, CRC(73c16d35) SHA1(40cf7a58926c649f89b08917afb35b08918d1a0f) )
	ROM_LOAD( "stsmart.905", 0x300000, 0x80000, CRC(a5beb4e2) SHA1(c26b7eee2ca32bd73fb7a09c6ef52c2ae1c7fc1c) )
	// 380000-3fffff empty

	ROM_REGION( 0x20000, "upd", 0 ) /* UPD7759 samples */
	ROM_LOAD( "s2-6.18d",    0x000000, 0x20000, CRC(47db1605) SHA1(ae00e633eb98567f04ff97e3d63e04e049d955ec) )

	ROM_REGION16_BE( 0x40000, "user1", ROMREGION_ERASE00 ) /* Extra code bank */
ROM_END

ROM_START( ikari3 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ik3-2-ver1.c10", 0x000000, 0x20000, CRC(1bae8023) SHA1(42d590a545cbabc596f2e0d9a3d56b1bc270ec9a) ) /* 8-Way Joystick */
	ROM_LOAD16_BYTE( "ik3-3-ver1.c9",  0x000001, 0x20000, CRC(10e38b66) SHA1(28cc82d868f59cd6dde1c4e4c890627012e5e978) ) /* 8-Way Joystick */

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Sound CPU */
	ROM_LOAD( "ik3-5.16d",  0x000000, 0x10000, CRC(ce6706fc) SHA1(95505b90a9524abf0c8c1ec6b2c40d8f25cb1d92) )

	ROM_REGION( 0x010000, "gfx1", 0 )   /* characters */
	ROM_LOAD( "ik3-7.16l",  0x000000, 0x08000, CRC(0b4804df) SHA1(66d16d245bfc404366164823faaea0bfec83e487) )
	ROM_LOAD( "ik3-8.16m",  0x008000, 0x08000, CRC(10ab4e50) SHA1(dee8416eb720848cf6471e568dadc1cfc6c2e67f) )

	ROM_REGION( 0x400000, "gfx2", 0 )   /* sprites */
	ROM_LOAD16_BYTE( "ik3-23.bin", 0x000000, 0x20000, CRC(d0fd5c77) SHA1(c171c64ad252f0ba5b0bbdf37808102fca37b488) )
	ROM_LOAD16_BYTE( "ik3-13.bin", 0x000001, 0x20000, CRC(9a56bd32) SHA1(9301b48f970b71a909fb44514b2e93c3f1516b38) )
	ROM_LOAD16_BYTE( "ik3-22.bin", 0x040000, 0x20000, CRC(4878d883) SHA1(8cdb541bad00e707fb65399d637b7cc9288ada77) )
	ROM_LOAD16_BYTE( "ik3-12.bin", 0x040001, 0x20000, CRC(0ce6a10a) SHA1(13a231aa0002b2c5a0d9404ba05a879e212d638e) )
	ROM_LOAD16_BYTE( "ik3-21.bin", 0x080000, 0x20000, CRC(50d0fbf0) SHA1(9ff5fbea8d35d0f9a38ddd7eb093edcd91d9f874) )
	ROM_LOAD16_BYTE( "ik3-11.bin", 0x080001, 0x20000, CRC(e4e2be43) SHA1(959d2799708ddae909b017c0696694c46a52697e) )
	ROM_LOAD16_BYTE( "ik3-20.bin", 0x0c0000, 0x20000, CRC(9a851efc) SHA1(bc7be338ee4da7fbfe6fe44a9c7889817416bc44) )
	ROM_LOAD16_BYTE( "ik3-10.bin", 0x0c0001, 0x20000, CRC(ac222372) SHA1(8a17e37699d691b962a6d0256a18550cc73ddfef) )
	ROM_LOAD16_BYTE( "ik3-19.bin", 0x100000, 0x20000, CRC(4ebdba89) SHA1(f3ecfef4c9d2aba58dc3e6aa3cf5813d68686909) )
	ROM_LOAD16_BYTE( "ik3-9.bin",  0x100001, 0x20000, CRC(c33971c2) SHA1(91f3eb301803f5a7027da1ff7dd2a28bc97e5125) )
	// 140000-1fffff empty
	ROM_LOAD16_BYTE( "ik3-14.bin", 0x200000, 0x20000, CRC(453bea77) SHA1(f8f8d0c048fcf32ad99e1de622d9ab635bb86eae) )
	ROM_LOAD16_BYTE( "ik3-24.bin", 0x200001, 0x20000, CRC(e9b26d68) SHA1(067d582d33157ed4b7980bd87f2f260ab74c347b) )
	ROM_LOAD16_BYTE( "ik3-15.bin", 0x240000, 0x20000, CRC(781a81fc) SHA1(e08a6cf9c632d1002176afe618605bc06168e8aa) )
	ROM_LOAD16_BYTE( "ik3-25.bin", 0x240001, 0x20000, CRC(073b03f1) SHA1(b8053139799fa06c7324cee928154c89d4425ab1) )
	ROM_LOAD16_BYTE( "ik3-16.bin", 0x280000, 0x20000, CRC(80ba400b) SHA1(2cc3e53c45f239516a60c461ad9cfa5955164262) )
	ROM_LOAD16_BYTE( "ik3-26.bin", 0x280001, 0x20000, CRC(9c613561) SHA1(fc7c9a642b18faa94e6a2ba53f35a4d756a25da3) )
	ROM_LOAD16_BYTE( "ik3-17.bin", 0x2c0000, 0x20000, CRC(0cc3ce4a) SHA1(7b34435d0bbb089055a183b821ab255170db6bec) )
	ROM_LOAD16_BYTE( "ik3-27.bin", 0x2c0001, 0x20000, CRC(16dd227e) SHA1(db3b1718dea65bc9a1a736aa62aa2be389313baf) )
	ROM_LOAD16_BYTE( "ik3-18.bin", 0x300000, 0x20000, CRC(ba106245) SHA1(ac609ec3046c21fe6058f91dd4528c5c6448dc15) )
	ROM_LOAD16_BYTE( "ik3-28.bin", 0x300001, 0x20000, CRC(711715ae) SHA1(90978c86884ca3d23c138d95b654e2fb3afc6f9a) )
	// 340000-3fffff empty

	ROM_REGION( 0x20000, "upd", 0 ) /* UPD7759 samples */
	ROM_LOAD( "ik3-6.18e",  0x000000, 0x20000, CRC(59d256a4) SHA1(1e7b33329f761c695bc9a817bbc0c5e13386d073) )

	ROM_REGION16_BE( 0x40000, "user1", 0 ) /* Extra code bank */
	ROM_LOAD16_BYTE( "ik3-1.c8",   0x000000, 0x10000, CRC(47e4d256) SHA1(7c6921cf2f1b8c3dae867eb1fc14e3da218cc1e0) )
	ROM_LOAD16_BYTE( "ik3-4.c12",  0x000001, 0x10000, CRC(a43af6b5) SHA1(1ad3acadbadd21642932028ecd7c282f7fd02856) )
ROM_END

ROM_START( ikari3u )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ik3-2.c10", 0x000000, 0x20000, CRC(a7b34dcd) SHA1(7c2f20ae4f7dbebd3dfa3ec5408ed714e6535b6a) ) /* Rotary Joystick */
	ROM_LOAD16_BYTE( "ik3-3.c9",  0x000001, 0x20000, CRC(50f2b83d) SHA1(b1f0c554b262614dd2cff7a3857cb974d361937f) ) /* Rotary Joystick */

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Sound CPU */
	ROM_LOAD( "ik3-5.15d",  0x000000, 0x10000, CRC(ce6706fc) SHA1(95505b90a9524abf0c8c1ec6b2c40d8f25cb1d92) )

	ROM_REGION( 0x010000, "gfx1", 0 )   /* characters */
	ROM_LOAD( "ik3-7.16l",  0x000000, 0x08000, CRC(0b4804df) SHA1(66d16d245bfc404366164823faaea0bfec83e487) )
	ROM_LOAD( "ik3-8.16m",  0x008000, 0x08000, CRC(10ab4e50) SHA1(dee8416eb720848cf6471e568dadc1cfc6c2e67f) )

	ROM_REGION( 0x400000, "gfx2", 0 )   /* sprites */
	ROM_LOAD16_BYTE( "ik3-23.bin", 0x000000, 0x20000, CRC(d0fd5c77) SHA1(c171c64ad252f0ba5b0bbdf37808102fca37b488) )
	ROM_LOAD16_BYTE( "ik3-13.bin", 0x000001, 0x20000, CRC(9a56bd32) SHA1(9301b48f970b71a909fb44514b2e93c3f1516b38) )
	ROM_LOAD16_BYTE( "ik3-22.bin", 0x040000, 0x20000, CRC(4878d883) SHA1(8cdb541bad00e707fb65399d637b7cc9288ada77) )
	ROM_LOAD16_BYTE( "ik3-12.bin", 0x040001, 0x20000, CRC(0ce6a10a) SHA1(13a231aa0002b2c5a0d9404ba05a879e212d638e) )
	ROM_LOAD16_BYTE( "ik3-21.bin", 0x080000, 0x20000, CRC(50d0fbf0) SHA1(9ff5fbea8d35d0f9a38ddd7eb093edcd91d9f874) )
	ROM_LOAD16_BYTE( "ik3-11.bin", 0x080001, 0x20000, CRC(e4e2be43) SHA1(959d2799708ddae909b017c0696694c46a52697e) )
	ROM_LOAD16_BYTE( "ik3-20.bin", 0x0c0000, 0x20000, CRC(9a851efc) SHA1(bc7be338ee4da7fbfe6fe44a9c7889817416bc44) )
	ROM_LOAD16_BYTE( "ik3-10.bin", 0x0c0001, 0x20000, CRC(ac222372) SHA1(8a17e37699d691b962a6d0256a18550cc73ddfef) )
	ROM_LOAD16_BYTE( "ik3-19.bin", 0x100000, 0x20000, CRC(4ebdba89) SHA1(f3ecfef4c9d2aba58dc3e6aa3cf5813d68686909) )
	ROM_LOAD16_BYTE( "ik3-9.bin",  0x100001, 0x20000, CRC(c33971c2) SHA1(91f3eb301803f5a7027da1ff7dd2a28bc97e5125) )
	// 140000-1fffff empty
	ROM_LOAD16_BYTE( "ik3-14.bin", 0x200000, 0x20000, CRC(453bea77) SHA1(f8f8d0c048fcf32ad99e1de622d9ab635bb86eae) )
	ROM_LOAD16_BYTE( "ik3-24.bin", 0x200001, 0x20000, CRC(e9b26d68) SHA1(067d582d33157ed4b7980bd87f2f260ab74c347b) )
	ROM_LOAD16_BYTE( "ik3-15.bin", 0x240000, 0x20000, CRC(781a81fc) SHA1(e08a6cf9c632d1002176afe618605bc06168e8aa) )
	ROM_LOAD16_BYTE( "ik3-25.bin", 0x240001, 0x20000, CRC(073b03f1) SHA1(b8053139799fa06c7324cee928154c89d4425ab1) )
	ROM_LOAD16_BYTE( "ik3-16.bin", 0x280000, 0x20000, CRC(80ba400b) SHA1(2cc3e53c45f239516a60c461ad9cfa5955164262) )
	ROM_LOAD16_BYTE( "ik3-26.bin", 0x280001, 0x20000, CRC(9c613561) SHA1(fc7c9a642b18faa94e6a2ba53f35a4d756a25da3) )
	ROM_LOAD16_BYTE( "ik3-17.bin", 0x2c0000, 0x20000, CRC(0cc3ce4a) SHA1(7b34435d0bbb089055a183b821ab255170db6bec) )
	ROM_LOAD16_BYTE( "ik3-27.bin", 0x2c0001, 0x20000, CRC(16dd227e) SHA1(db3b1718dea65bc9a1a736aa62aa2be389313baf) )
	ROM_LOAD16_BYTE( "ik3-18.bin", 0x300000, 0x20000, CRC(ba106245) SHA1(ac609ec3046c21fe6058f91dd4528c5c6448dc15) )
	ROM_LOAD16_BYTE( "ik3-28.bin", 0x300001, 0x20000, CRC(711715ae) SHA1(90978c86884ca3d23c138d95b654e2fb3afc6f9a) )
	// 340000-3fffff empty

	ROM_REGION( 0x20000, "upd", 0 ) /* UPD7759 samples */
	ROM_LOAD( "ik3-6.18e",  0x000000, 0x20000, CRC(59d256a4) SHA1(1e7b33329f761c695bc9a817bbc0c5e13386d073) )

	ROM_REGION16_BE( 0x40000, "user1", 0 ) /* Extra code bank */
	ROM_LOAD16_BYTE( "ik3-1.c8",   0x000000, 0x10000, CRC(47e4d256) SHA1(7c6921cf2f1b8c3dae867eb1fc14e3da218cc1e0) )
	ROM_LOAD16_BYTE( "ik3-4.c12",  0x000001, 0x10000, CRC(a43af6b5) SHA1(1ad3acadbadd21642932028ecd7c282f7fd02856) )
ROM_END

ROM_START( ikari3j )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ik3-2-j.c10", 0x000000, 0x20000, CRC(7b1b4be4) SHA1(22b7504040da7364b811c07014a776fa5d1d5d9f) ) /* Rotary Joystick */
	ROM_LOAD16_BYTE( "ik3-3-j.c9",  0x000001, 0x20000, CRC(8e6e2aa9) SHA1(e624809c42a79510b34d99d9ca152a38c7051e87) ) /* Rotary Joystick */

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Sound CPU */
	ROM_LOAD( "ik3-5.16d",  0x000000, 0x10000, CRC(ce6706fc) SHA1(95505b90a9524abf0c8c1ec6b2c40d8f25cb1d92) )

	ROM_REGION( 0x010000, "gfx1", 0 )   /* characters */
	ROM_LOAD( "ik3-7.16l",  0x000000, 0x08000, CRC(0b4804df) SHA1(66d16d245bfc404366164823faaea0bfec83e487) )
	ROM_LOAD( "ik3-8.16m",  0x008000, 0x08000, CRC(10ab4e50) SHA1(dee8416eb720848cf6471e568dadc1cfc6c2e67f) )

	ROM_REGION( 0x400000, "gfx2", 0 )   /* sprites */
	ROM_LOAD16_BYTE( "ik3-23.bin", 0x000000, 0x20000, CRC(d0fd5c77) SHA1(c171c64ad252f0ba5b0bbdf37808102fca37b488) )
	ROM_LOAD16_BYTE( "ik3-13.bin", 0x000001, 0x20000, CRC(9a56bd32) SHA1(9301b48f970b71a909fb44514b2e93c3f1516b38) )
	ROM_LOAD16_BYTE( "ik3-22.bin", 0x040000, 0x20000, CRC(4878d883) SHA1(8cdb541bad00e707fb65399d637b7cc9288ada77) )
	ROM_LOAD16_BYTE( "ik3-12.bin", 0x040001, 0x20000, CRC(0ce6a10a) SHA1(13a231aa0002b2c5a0d9404ba05a879e212d638e) )
	ROM_LOAD16_BYTE( "ik3-21.bin", 0x080000, 0x20000, CRC(50d0fbf0) SHA1(9ff5fbea8d35d0f9a38ddd7eb093edcd91d9f874) )
	ROM_LOAD16_BYTE( "ik3-11.bin", 0x080001, 0x20000, CRC(e4e2be43) SHA1(959d2799708ddae909b017c0696694c46a52697e) )
	ROM_LOAD16_BYTE( "ik3-20.bin", 0x0c0000, 0x20000, CRC(9a851efc) SHA1(bc7be338ee4da7fbfe6fe44a9c7889817416bc44) )
	ROM_LOAD16_BYTE( "ik3-10.bin", 0x0c0001, 0x20000, CRC(ac222372) SHA1(8a17e37699d691b962a6d0256a18550cc73ddfef) )
	ROM_LOAD16_BYTE( "ik3-19.bin", 0x100000, 0x20000, CRC(4ebdba89) SHA1(f3ecfef4c9d2aba58dc3e6aa3cf5813d68686909) )
	ROM_LOAD16_BYTE( "ik3-9.bin",  0x100001, 0x20000, CRC(c33971c2) SHA1(91f3eb301803f5a7027da1ff7dd2a28bc97e5125) )
	// 140000-1fffff empty
	ROM_LOAD16_BYTE( "ik3-14.bin", 0x200000, 0x20000, CRC(453bea77) SHA1(f8f8d0c048fcf32ad99e1de622d9ab635bb86eae) )
	ROM_LOAD16_BYTE( "ik3-24.bin", 0x200001, 0x20000, CRC(e9b26d68) SHA1(067d582d33157ed4b7980bd87f2f260ab74c347b) )
	ROM_LOAD16_BYTE( "ik3-15.bin", 0x240000, 0x20000, CRC(781a81fc) SHA1(e08a6cf9c632d1002176afe618605bc06168e8aa) )
	ROM_LOAD16_BYTE( "ik3-25.bin", 0x240001, 0x20000, CRC(073b03f1) SHA1(b8053139799fa06c7324cee928154c89d4425ab1) )
	ROM_LOAD16_BYTE( "ik3-16.bin", 0x280000, 0x20000, CRC(80ba400b) SHA1(2cc3e53c45f239516a60c461ad9cfa5955164262) )
	ROM_LOAD16_BYTE( "ik3-26.bin", 0x280001, 0x20000, CRC(9c613561) SHA1(fc7c9a642b18faa94e6a2ba53f35a4d756a25da3) )
	ROM_LOAD16_BYTE( "ik3-17.bin", 0x2c0000, 0x20000, CRC(0cc3ce4a) SHA1(7b34435d0bbb089055a183b821ab255170db6bec) )
	ROM_LOAD16_BYTE( "ik3-27.bin", 0x2c0001, 0x20000, CRC(16dd227e) SHA1(db3b1718dea65bc9a1a736aa62aa2be389313baf) )
	ROM_LOAD16_BYTE( "ik3-18.bin", 0x300000, 0x20000, CRC(ba106245) SHA1(ac609ec3046c21fe6058f91dd4528c5c6448dc15) )
	ROM_LOAD16_BYTE( "ik3-28.bin", 0x300001, 0x20000, CRC(711715ae) SHA1(90978c86884ca3d23c138d95b654e2fb3afc6f9a) )
	// 340000-3fffff empty

	ROM_REGION( 0x20000, "upd", 0 ) /* UPD7759 samples */
	ROM_LOAD( "ik3-6.18e",  0x000000, 0x20000, CRC(59d256a4) SHA1(1e7b33329f761c695bc9a817bbc0c5e13386d073) )

	ROM_REGION16_BE( 0x40000, "user1", 0 ) /* Extra code bank */
	ROM_LOAD16_BYTE( "ik3-1.c8",   0x000000, 0x10000, CRC(47e4d256) SHA1(7c6921cf2f1b8c3dae867eb1fc14e3da218cc1e0) )
	ROM_LOAD16_BYTE( "ik3-4.c12",  0x000001, 0x10000, CRC(a43af6b5) SHA1(1ad3acadbadd21642932028ecd7c282f7fd02856) )
ROM_END

ROM_START( ikari3k )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ik3-2k.c10", 0x000000, 0x20000, CRC(a15d2222) SHA1(7f9702516f9c74314b093435937dfecb69495b6c) ) /* 8-Way Joystick */
	ROM_LOAD16_BYTE( "ik3-3k.c9",  0x000001, 0x20000, CRC(e3fc006e) SHA1(14e8ba1064e9bd168a4f7e5b5cc2a4b1ddbc7e32) ) /* 8-Way Joystick */

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Sound CPU */
	ROM_LOAD( "ik3-5.16d",  0x000000, 0x10000, CRC(ce6706fc) SHA1(95505b90a9524abf0c8c1ec6b2c40d8f25cb1d92) )

	ROM_REGION( 0x010000, "gfx1", 0 )   /* characters */
	ROM_LOAD( "ik3-7k.16l",  0x000000, 0x08000, CRC(8bfb399b) SHA1(f9f9d947a7739d13269e2ab84ab25a7e403aed34) )
	ROM_LOAD( "ik3-8k.16m",  0x008000, 0x08000, CRC(3f0fe576) SHA1(70f4438e31b06ee0dc4660c04f512ccf3b7fa55f) )

	ROM_REGION( 0x400000, "gfx2", 0 )   /* sprites */
	ROM_LOAD16_BYTE( "ik3-23.bin", 0x000000, 0x20000, CRC(d0fd5c77) SHA1(c171c64ad252f0ba5b0bbdf37808102fca37b488) ) /* Missing 4M mask ROM dumps from the A7007-SUB4M */
	ROM_LOAD16_BYTE( "ik3-13.bin", 0x000001, 0x20000, CRC(9a56bd32) SHA1(9301b48f970b71a909fb44514b2e93c3f1516b38) ) /* ROM board. Assumed to be the same data */
	ROM_LOAD16_BYTE( "ik3-22.bin", 0x040000, 0x20000, CRC(4878d883) SHA1(8cdb541bad00e707fb65399d637b7cc9288ada77) )
	ROM_LOAD16_BYTE( "ik3-12.bin", 0x040001, 0x20000, CRC(0ce6a10a) SHA1(13a231aa0002b2c5a0d9404ba05a879e212d638e) )
	ROM_LOAD16_BYTE( "ik3-21.bin", 0x080000, 0x20000, CRC(50d0fbf0) SHA1(9ff5fbea8d35d0f9a38ddd7eb093edcd91d9f874) )
	ROM_LOAD16_BYTE( "ik3-11.bin", 0x080001, 0x20000, CRC(e4e2be43) SHA1(959d2799708ddae909b017c0696694c46a52697e) )
	ROM_LOAD16_BYTE( "ik3-20.bin", 0x0c0000, 0x20000, CRC(9a851efc) SHA1(bc7be338ee4da7fbfe6fe44a9c7889817416bc44) )
	ROM_LOAD16_BYTE( "ik3-10.bin", 0x0c0001, 0x20000, CRC(ac222372) SHA1(8a17e37699d691b962a6d0256a18550cc73ddfef) )
	ROM_LOAD16_BYTE( "ik3-19.bin", 0x100000, 0x20000, CRC(4ebdba89) SHA1(f3ecfef4c9d2aba58dc3e6aa3cf5813d68686909) )
	ROM_LOAD16_BYTE( "ik3-9.bin",  0x100001, 0x20000, CRC(c33971c2) SHA1(91f3eb301803f5a7027da1ff7dd2a28bc97e5125) )
	// 140000-1fffff empty
	ROM_LOAD16_BYTE( "ik3-14.bin", 0x200000, 0x20000, CRC(453bea77) SHA1(f8f8d0c048fcf32ad99e1de622d9ab635bb86eae) )
	ROM_LOAD16_BYTE( "ik3-24.bin", 0x200001, 0x20000, CRC(e9b26d68) SHA1(067d582d33157ed4b7980bd87f2f260ab74c347b) )
	ROM_LOAD16_BYTE( "ik3-15.bin", 0x240000, 0x20000, CRC(781a81fc) SHA1(e08a6cf9c632d1002176afe618605bc06168e8aa) )
	ROM_LOAD16_BYTE( "ik3-25.bin", 0x240001, 0x20000, CRC(073b03f1) SHA1(b8053139799fa06c7324cee928154c89d4425ab1) )
	ROM_LOAD16_BYTE( "ik3-16.bin", 0x280000, 0x20000, CRC(80ba400b) SHA1(2cc3e53c45f239516a60c461ad9cfa5955164262) )
	ROM_LOAD16_BYTE( "ik3-26.bin", 0x280001, 0x20000, CRC(9c613561) SHA1(fc7c9a642b18faa94e6a2ba53f35a4d756a25da3) )
	ROM_LOAD16_BYTE( "ik3-17.bin", 0x2c0000, 0x20000, CRC(0cc3ce4a) SHA1(7b34435d0bbb089055a183b821ab255170db6bec) )
	ROM_LOAD16_BYTE( "ik3-27.bin", 0x2c0001, 0x20000, CRC(16dd227e) SHA1(db3b1718dea65bc9a1a736aa62aa2be389313baf) )
	ROM_LOAD16_BYTE( "ik3-18.bin", 0x300000, 0x20000, CRC(ba106245) SHA1(ac609ec3046c21fe6058f91dd4528c5c6448dc15) )
	ROM_LOAD16_BYTE( "ik3-28.bin", 0x300001, 0x20000, CRC(711715ae) SHA1(90978c86884ca3d23c138d95b654e2fb3afc6f9a) )
	// 340000-3fffff empty

	ROM_REGION( 0x20000, "upd", 0 ) /* UPD7759 samples */
	ROM_LOAD( "ik3-6.18e",  0x000000, 0x20000, CRC(59d256a4) SHA1(1e7b33329f761c695bc9a817bbc0c5e13386d073) )

	ROM_REGION16_BE( 0x40000, "user1", 0 ) /* Extra code bank */
	ROM_LOAD16_BYTE( "ik3-1.c8",   0x000000, 0x10000, CRC(47e4d256) SHA1(7c6921cf2f1b8c3dae867eb1fc14e3da218cc1e0) )
	ROM_LOAD16_BYTE( "ik3-4.c12",  0x000001, 0x10000, CRC(a43af6b5) SHA1(1ad3acadbadd21642932028ecd7c282f7fd02856) )
ROM_END

ROM_START( searchar )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bhw.2", 0x000000, 0x20000, CRC(e1430138) SHA1(eddc192524a13b2c09bd2bddcd5f8e8b771ceb21) )
	ROM_LOAD16_BYTE( "bhw.3", 0x000001, 0x20000, CRC(ee1f9374) SHA1(fd41c74fd69d65713d8e1a9b8078328381119379) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Sound CPU */
	ROM_LOAD( "bh.5",       0x000000, 0x10000, CRC(53e2fa76) SHA1(cf25b1def82545a1fd013822ab3cf02483074623) )

	ROM_REGION( 0x010000, "gfx1", 0 )   /* characters */
	ROM_LOAD( "bh.7",       0x000000, 0x08000, CRC(b0f1b049) SHA1(ec276984d91b5759a5e2b6815d1db2abc37b99f8) )
	ROM_LOAD( "bh.8",       0x008000, 0x08000, CRC(174ddba7) SHA1(7b19087cd2ccc409878aefe7fa08bb2e9953d352) )

	ROM_REGION( 0x400000, "gfx2", 0 )   /* sprites */
	ROM_LOAD( "bh.c1",      0x000000, 0x80000, CRC(1fb8f0ae) SHA1(d63c7376aa5f01bc009176b23324e720bada4286) )
	ROM_LOAD( "bh.c3",      0x080000, 0x80000, CRC(fd8bc407) SHA1(88d750293808bf6ea23864b22070314b14fbee3c) )
	ROM_LOAD( "bh.c5",      0x100000, 0x80000, CRC(1d30acc3) SHA1(e5ca39853779475b83fe37304e7bed2c293bd587) )
	// 180000-1fffff empty
	ROM_LOAD( "bh.c2",      0x200000, 0x80000, CRC(7c803767) SHA1(992516fbb28d00feabbed5769fa3a5748199a7d8) )
	ROM_LOAD( "bh.c4",      0x280000, 0x80000, CRC(eede7c43) SHA1(7645acf0beb4fff9ec92205dcf34124360cd52f6) )
	ROM_LOAD( "bh.c6",      0x300000, 0x80000, CRC(9f785cd9) SHA1(e5c7797ae7a3139e1814b068c5ecfe5c6bf30d0f) )
	// 380000-3fffff empty

	ROM_REGION( 0x20000, "upd", 0 ) /* UPD7759 samples */
	ROM_LOAD( "bh.v1",      0x000000, 0x20000, CRC(07a6114b) SHA1(224df4616b77a56f33974d3b1793473d48ad52ca) )

	ROM_REGION16_BE( 0x40000, "user1", 0 ) /* Extra code bank */
	ROM_LOAD16_BYTE( "bhw.1", 0x000000, 0x20000, CRC(62b60066) SHA1(f7e7985c8f5f8191c580e777e1b7ed29d944d23f) )
	ROM_LOAD16_BYTE( "bhw.4", 0x000001, 0x20000, CRC(16d8525c) SHA1(0098b0a7fcb23de2661bbec9a05254aa46579bb2) )
ROM_END

ROM_START( searcharu )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bh.2",  0x000000, 0x20000, CRC(c852e2e2) SHA1(c4b1b366f452122549046a3dec9b6b375bc273af) )
	ROM_LOAD16_BYTE( "bh.3",  0x000001, 0x20000, CRC(bc04a4a1) SHA1(aa91583b987248a3e99813ab5e8ee03c02dac9b9) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Sound CPU */
	ROM_LOAD( "bh.5",       0x000000, 0x10000, CRC(53e2fa76) SHA1(cf25b1def82545a1fd013822ab3cf02483074623) )

	ROM_REGION( 0x010000, "gfx1", 0 )   /* characters */
	ROM_LOAD( "bh.7",       0x000000, 0x08000, CRC(b0f1b049) SHA1(ec276984d91b5759a5e2b6815d1db2abc37b99f8) )
	ROM_LOAD( "bh.8",       0x008000, 0x08000, CRC(174ddba7) SHA1(7b19087cd2ccc409878aefe7fa08bb2e9953d352) )

	ROM_REGION( 0x400000, "gfx2", 0 )   /* sprites */
	ROM_LOAD( "bh.c1",      0x000000, 0x80000, CRC(1fb8f0ae) SHA1(d63c7376aa5f01bc009176b23324e720bada4286) )
	ROM_LOAD( "bh.c3",      0x080000, 0x80000, CRC(fd8bc407) SHA1(88d750293808bf6ea23864b22070314b14fbee3c) )
	ROM_LOAD( "bh.c5",      0x100000, 0x80000, CRC(1d30acc3) SHA1(e5ca39853779475b83fe37304e7bed2c293bd587) )
	// 180000-1fffff empty
	ROM_LOAD( "bh.c2",      0x200000, 0x80000, CRC(7c803767) SHA1(992516fbb28d00feabbed5769fa3a5748199a7d8) )
	ROM_LOAD( "bh.c4",      0x280000, 0x80000, CRC(eede7c43) SHA1(7645acf0beb4fff9ec92205dcf34124360cd52f6) )
	ROM_LOAD( "bh.c6",      0x300000, 0x80000, CRC(9f785cd9) SHA1(e5c7797ae7a3139e1814b068c5ecfe5c6bf30d0f) )
	// 380000-3fffff empty

	ROM_REGION( 0x20000, "upd", 0 ) /* UPD7759 samples */
	ROM_LOAD( "bh.v1",      0x000000, 0x20000, CRC(07a6114b) SHA1(224df4616b77a56f33974d3b1793473d48ad52ca) )

	ROM_REGION16_BE( 0x40000, "user1", 0 ) /* Extra code bank */
	ROM_LOAD16_BYTE( "bh.1",  0x000000, 0x20000, CRC(ba9ca70b) SHA1(c46727473673554cbe4bbbc0288d66357f99a80e) )
	ROM_LOAD16_BYTE( "bh.4",  0x000001, 0x20000, CRC(eabc5ddf) SHA1(08a2a8fcdf6a08a2694e00f4232a5bfbec98fd27) )
ROM_END

ROM_START( searcharj )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bh2ver3j.9c", 0x000000, 0x20000, CRC(7ef7b172) SHA1(85669ba72f59e4ff3a483bf611bf41c73f4e1930) )
	ROM_LOAD16_BYTE( "bh3ver3j.10c", 0x000001, 0x20000, CRC(3fdea793) SHA1(49bafb53466afb7e4486a4894e4fd6fa08ea2eb2) )

	ROM_REGION( 0x10000, "soundcpu", 0 )    /* Sound CPU */
	ROM_LOAD( "bh.5",       0x000000, 0x10000, CRC(53e2fa76) SHA1(cf25b1def82545a1fd013822ab3cf02483074623) )

	ROM_REGION( 0x010000, "gfx1", 0 )   /* characters */
	ROM_LOAD( "bh.7",       0x000000, 0x08000, CRC(b0f1b049) SHA1(ec276984d91b5759a5e2b6815d1db2abc37b99f8) )
	ROM_LOAD( "bh.8",       0x008000, 0x08000, CRC(174ddba7) SHA1(7b19087cd2ccc409878aefe7fa08bb2e9953d352) )

	ROM_REGION( 0x400000, "gfx2", 0 )   /* sprites */
	ROM_LOAD( "bh.c1",      0x000000, 0x80000, CRC(1fb8f0ae) SHA1(d63c7376aa5f01bc009176b23324e720bada4286) )
	ROM_LOAD( "bh.c3",      0x080000, 0x80000, CRC(fd8bc407) SHA1(88d750293808bf6ea23864b22070314b14fbee3c) )
	ROM_LOAD( "bh.c5",      0x100000, 0x80000, CRC(1d30acc3) SHA1(e5ca39853779475b83fe37304e7bed2c293bd587) )
	// 180000-1fffff empty
	ROM_LOAD( "bh.c2",      0x200000, 0x80000, CRC(7c803767) SHA1(992516fbb28d00feabbed5769fa3a5748199a7d8) )
	ROM_LOAD( "bh.c4",      0x280000, 0x80000, CRC(eede7c43) SHA1(7645acf0beb4fff9ec92205dcf34124360cd52f6) )
	ROM_LOAD( "bh.c6",      0x300000, 0x80000, CRC(9f785cd9) SHA1(e5c7797ae7a3139e1814b068c5ecfe5c6bf30d0f) )
	// 380000-3fffff empty

	ROM_REGION( 0x20000, "upd", 0 ) /* UPD7759 samples */
	ROM_LOAD( "bh.v1",      0x000000, 0x20000, CRC(07a6114b) SHA1(224df4616b77a56f33974d3b1793473d48ad52ca) )

	ROM_REGION16_BE( 0x40000, "user1", 0 ) /* Extra code bank */
	ROM_LOAD16_BYTE( "bhw.1", 0x000000, 0x20000, CRC(62b60066) SHA1(f7e7985c8f5f8191c580e777e1b7ed29d944d23f) )
	ROM_LOAD16_BYTE( "bhw.4", 0x000001, 0x20000, CRC(16d8525c) SHA1(0098b0a7fcb23de2661bbec9a05254aa46579bb2) )
ROM_END

/******************************************************************************/

GAME( 1988, pow,       0,        pow,      pow,      driver_device, 0, ROT0,  "SNK", "P.O.W. - Prisoners of War (US version 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, powj,      pow,      pow,      powj,     driver_device, 0, ROT0,  "SNK", "Datsugoku - Prisoners of War (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, streetsm,  0,        pow,      streetsm, driver_device, 0, ROT0,  "SNK", "Street Smart (US version 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, streetsm1, streetsm, searchar, streetsm, driver_device, 0, ROT0,  "SNK", "Street Smart (US version 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, streetsmw, streetsm, searchar, streetsj, driver_device, 0, ROT0,  "SNK", "Street Smart (World version 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, streetsmj, streetsm, searchar, streetsj, driver_device, 0, ROT0,  "SNK", "Street Smart (Japan version 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, ikari3,    0,        searchar, ikari3,   driver_device, 0, ROT0,  "SNK", "Ikari III - The Rescue (World, 8-Way Joystick)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, ikari3u,   ikari3,   searchar, ikari3,   driver_device, 0, ROT0,  "SNK", "Ikari III - The Rescue (US, Rotary Joystick)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, ikari3j,   ikari3,   searchar, ikari3,   driver_device, 0, ROT0,  "SNK", "Ikari Three (Japan, Rotary Joystick)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, ikari3k,   ikari3,   searchar, ikari3,   driver_device, 0, ROT0,  "SNK", "Ikari Three (Korea, 8-Way Joystick)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, searchar,  0,        searchar, searchar, driver_device, 0, ROT90, "SNK", "SAR - Search And Rescue (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, searcharu, searchar, searchar, searchar, driver_device, 0, ROT90, "SNK", "SAR - Search And Rescue (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, searcharj, searchar, searchar, searchar, driver_device, 0, ROT90, "SNK", "SAR - Search And Rescue (Japan)", MACHINE_SUPPORTS_SAVE )

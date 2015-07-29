// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo
/****************************************************************************

    Blockade/Comotion/Blasto/Hustle Memory MAP
    Frank Palazzolo (palazzol@comcast.net)

    CPU - Intel 8080A

    Memory Address              (Upper/Lower)

    0xxx 00aa aaaa aaaa     ROM     U2/U3    R       1K for Blockade/Comotion/Blasto
    0xxx 01aa aaaa aaaa     ROM     U4/U5    R       1K for Comotion/Blasto/Hustle Only
    1xx0 xxaa aaaa aaaa    VRAM              W       1K playfield
    xxx1 xxxx aaaa aaaa     RAM              R/W     256 bytes

                        CHAR ROM  U29/U43            256 bytes for Blockade/Comotion
                                                     512 for Blasto/Hustle

    Ports    In            Out
    1        Controls      bit 7 = Coin Latch Reset
                           bit 5 = Pin 19?
    2        Controls      Square Wave Pitch Register
    4        Controls      Noise On
    8        N/A           Noise Off


    Notes:  Support is complete with the exception of the noise generator.
            Hopefully I can add this based on some measurements from a real board

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "includes/blockade.h"
#include "sound/samples.h"
#include "sound/discrete.h"

#include "blockade.lh"

#define BLOCKADE_LOG 0
#define MASTER_CLOCK XTAL_20_079MHz

/* These are used to simulate coin latch circuitry */

/*************************************************************/
/*                                                           */
/* Inserting a coin should work like this:                   */
/*  1) Reset the CPU                                         */
/*  2) CPU Sets the coin latch                               */
/*  3) Finally the CPU coin latch is Cleared by the hardware */
/*     (by the falling coin..?)                              */
/*                                                           */
/*  I am faking this by keeping the CPU from Setting         */
/*  the coin latch if we have just been reset.               */
/*                                                           */
/*************************************************************/


/* Need to check for a coin on the interrupt, */
/* This will reset the cpu                    */

INTERRUPT_GEN_MEMBER(blockade_state::blockade_interrupt)
{
	device.execute().resume(SUSPEND_ANY_REASON);

	if ((ioport("IN0")->read() & 0x80) == 0)
	{
		m_just_been_reset = 1;
		device.execute().set_input_line(INPUT_LINE_RESET, PULSE_LINE);
	}
}

/*************************************
 *
 *  Memory handlers
 *
 *************************************/

READ8_MEMBER(blockade_state::blockade_input_port_0_r)
{
	/* coin latch is bit 7 */
	UINT8 temp = (ioport("IN0")->read() & 0x7f);

	return (m_coin_latch << 7) | temp;
}

WRITE8_MEMBER(blockade_state::blockade_coin_latch_w)
{
	if (data & 0x80)
	{
		if (BLOCKADE_LOG) osd_printf_debug("Reset Coin Latch\n");
		if (m_just_been_reset)
		{
			m_just_been_reset = 0;
			m_coin_latch = 0;
		}
		else
			m_coin_latch = 1;
	}

	if (data & 0x20)
	{
		if (BLOCKADE_LOG) osd_printf_debug("Pin 19 High\n");
	}
	else
	{
		if (BLOCKADE_LOG) osd_printf_debug("Pin 19 Low\n");
	}

	return;
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, blockade_state )
	AM_RANGE(0x0000, 0x07ff) AM_ROM AM_MIRROR(0x6000)
	AM_RANGE(0x8000, 0x83ff) AM_RAM_WRITE(blockade_videoram_w) AM_SHARE("videoram") AM_MIRROR(0x6c00)
	AM_RANGE(0x9000, 0x90ff) AM_RAM AM_MIRROR(0x6f00)
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_io_map, AS_IO, 8, blockade_state )
	AM_RANGE(0x01, 0x01) AM_READWRITE(blockade_input_port_0_r, blockade_coin_latch_w)
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_WRITE(blockade_sound_freq_w)
	AM_RANGE(0x04, 0x04) AM_READ_PORT("IN2")
	AM_RANGE(0x04, 0x04) AM_WRITE(blockade_env_on_w)
	AM_RANGE(0x08, 0x08) AM_WRITE(blockade_env_off_w)
ADDRESS_MAP_END


/*************************************
 *
 *  Input ports
 *
 *************************************/

/* These are not dip switches, they are mapped to */
/* connectors on the board.  Different games had  */
/* different harnesses which plugged in here, and */
/* some pins were unused.                         */

static INPUT_PORTS_START( blockade )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_CONFNAME( 0x04, 0x04, "Boom Switch" )
	PORT_CONFSETTING(      0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(      0x04, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_CONFNAME( 0x70, 0x70, DEF_STR( Lives ) )
	PORT_CONFSETTING(      0x60, "3" )
	PORT_CONFSETTING(      0x50, "4" )
	PORT_CONFSETTING(      0x30, "5" )
	PORT_CONFSETTING(      0x70, "6" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
								/* this is really used for the coin latch,  */
								/* see blockade_interrupt()                 */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( comotion )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_CONFNAME( 0x04, 0x04, "Boom Switch" )
	PORT_CONFSETTING(      0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(      0x04, DEF_STR( On ) )
	PORT_CONFNAME( 0x08, 0x00, DEF_STR( Lives ) )
	PORT_CONFSETTING(      0x00, "3" )
	PORT_CONFSETTING(      0x08, "4" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
								/* this is really used for the coin latch,  */
								/* see blockade_interrupt()                 */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(3)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(4)

	PORT_START("IN3")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END

static INPUT_PORTS_START( blasto )
	PORT_START("IN0")
	PORT_CONFNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_CONFSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_CONFSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_CONFSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_CONFSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_CONFNAME( 0x04, 0x04, "Attract Sound" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x04, DEF_STR( On ) )
	PORT_CONFNAME( 0x08, 0x08, DEF_STR( Game_Time ) )
	PORT_CONFSETTING(    0x00, "70 Secs" ) // though service manual says 60
	PORT_CONFSETTING(    0x08, "90 Secs" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
								/* this is really used for the coin latch,  */
								/* see blockade_interrupt()                 */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)

	PORT_START("IN3")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END

static INPUT_PORTS_START( hustle )
	PORT_START("IN0")
	PORT_CONFNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_CONFSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_CONFSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_CONFSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_CONFSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_CONFNAME( 0x04, 0x04, DEF_STR( Game_Time ) )
	PORT_CONFSETTING(    0x00, "1.5 mins" )
	PORT_CONFSETTING(    0x04, "2 mins" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
								/* this is really used for the coin latch,  */
								/* see blockade_interrupt()                 */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)

	PORT_START("IN2")
	PORT_CONFNAME( 0xf1, 0xf0, "Free Game" )
	PORT_CONFSETTING(    0x71, "11000" )
	PORT_CONFSETTING(    0xb1, "13000" )
	PORT_CONFSETTING(    0xd1, "15000" )
	PORT_CONFSETTING(    0xe1, "17000" )
	PORT_CONFSETTING(    0xf0, "Disabled" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END

static INPUT_PORTS_START( mineswpr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* This wiring selects upright mode */
	PORT_CONFNAME( 0x04, 0x04, "Boom Switch" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x04, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_CONFNAME( 0x70, 0x70, DEF_STR( Lives ) )
	PORT_CONFSETTING(    0x60, "3" )
	PORT_CONFSETTING(    0x50, "4" )
	PORT_CONFSETTING(    0x30, "5" )
	PORT_CONFSETTING(    0x70, "6" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
								/* this is really used for the coin latch,  */
								/* see blockade_interrupt()                 */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END

static INPUT_PORTS_START( mineswpr4 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* This wiring selects cocktail mode */
	PORT_CONFNAME( 0x04, 0x04, "Boom Switch" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x04, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_CONFNAME( 0x70, 0x70, DEF_STR( Lives ) )
	PORT_CONFSETTING(    0x60, "3" )
	PORT_CONFSETTING(    0x50, "4" )
	PORT_CONFSETTING(    0x30, "5" )
	PORT_CONFSETTING(    0x70, "6" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
								/* this is really used for the coin latch,  */
								/* see blockade_interrupt()                 */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(4)

	PORT_START("IN3")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout blockade_layout =
{
	8,8,    /* 8*8 characters */
	32, /* 32 characters */
	1,  /* 1 bit per pixel */
	{ 0 },  /* no separation in 1 bpp */
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static const gfx_layout blasto_layout =
{
	8,8,    /* 8*8 characters */
	64, /* 64 characters */
	1,  /* 1 bit per pixel */
	{ 0 },  /* no separation in 1 bpp */
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static GFXDECODE_START( blockade )
	GFXDECODE_ENTRY( "gfx1", 0x0000, blockade_layout, 0, 1 )
GFXDECODE_END

static GFXDECODE_START( blasto )
	GFXDECODE_ENTRY( "gfx1", 0x0000, blasto_layout,   0, 1 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void blockade_state::machine_start()
{
	save_item(NAME(m_coin_latch));
	save_item(NAME(m_just_been_reset));
}

void blockade_state::machine_reset()
{
	m_coin_latch = 1;
	m_just_been_reset = 0;
}

static MACHINE_CONFIG_START( blockade, blockade_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8080, MASTER_CLOCK/10)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(main_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", blockade_state,  blockade_interrupt)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 28*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(blockade_state, screen_update_blockade)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", blockade)
	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(1)
	MCFG_SAMPLES_NAMES(blockade_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(blockade)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( blasto, blockade )
	MCFG_GFXDECODE_MODIFY("gfxdecode", blasto)
MACHINE_CONFIG_END

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( blockade )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD_NIB_HIGH( "316-04.u2", 0x0000, 0x0400, CRC(a93833e9) SHA1(e29e7b29900f8305effa700a53806a12bf9d37bd) )
	ROM_LOAD_NIB_LOW(  "316-03.u3", 0x0000, 0x0400, CRC(85960d3b) SHA1(aabfe8f9c26126299d6c07a31ef1aac5300deff5) )

	ROM_REGION( 0x100, "gfx1", 0 )
	ROM_LOAD_NIB_HIGH( "316-02.u29", 0x0000, 0x0100, CRC(409f610f) SHA1(0c2253f4b72d8aa395f87cc0abe07f0b46fa538b) )
	ROM_LOAD_NIB_LOW(  "316-01.u43", 0x0000, 0x0100, CRC(41a00b28) SHA1(2d0a90aac9d10a1ded240e5202fdf9cd7f70c4a7) )
ROM_END

ROM_START( comotion )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD_NIB_HIGH( "316-07.u2", 0x0000, 0x0400, CRC(5b9bd054) SHA1(324b844788945e7bc82d096d6d375e79e3e1a634) )
	ROM_LOAD_NIB_LOW(  "316-08.u3", 0x0000, 0x0400, CRC(1a856042) SHA1(91bdc260e8c88ce2b6ac05bfba043ed611bc30de) )
	ROM_LOAD_NIB_HIGH( "316-09.u4", 0x0400, 0x0400, CRC(2590f87c) SHA1(95a7af04b610d79fb3f6d74dda322e66164b9484) )
	ROM_LOAD_NIB_LOW(  "316-10.u5", 0x0400, 0x0400, CRC(fb49a69b) SHA1(4009c3256a86508d981c1f77b65e6bff1face1e7) )

	ROM_REGION( 0x100, "gfx1", 0 )
	ROM_LOAD_NIB_HIGH( "316-06.u43", 0x0000, 0x0100, CRC(8f071297) SHA1(811471c87b77b4b9ab056cf0c0743fc2616b754c) )  /* Note: these are reversed */
	ROM_LOAD_NIB_LOW(  "316-05.u29", 0x0000, 0x0100, CRC(53fb8821) SHA1(0a499aa4cf15f7ebea155aacd914de8851544215) )
ROM_END

ROM_START( blasto )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD_NIB_HIGH( "blasto.u2", 0x0000, 0x0400, CRC(ec99d043) SHA1(10650e54bf55f3ace5c199215c2fce211916d3b7) )
	ROM_LOAD_NIB_LOW(  "blasto.u3", 0x0000, 0x0400, CRC(be333415) SHA1(386cab720f0c2da16b9ec84f67ccebf23406c58d) )
	ROM_LOAD_NIB_HIGH( "blasto.u4", 0x0400, 0x0400, CRC(1c889993) SHA1(e23c72d075cf3d209081bca5a953c33c8ae042ea) )
	ROM_LOAD_NIB_LOW(  "blasto.u5", 0x0400, 0x0400, CRC(efb640cb) SHA1(2dff5b249f876d7d13cc6dfad652ce7e5af10370) )

	ROM_REGION( 0x200, "gfx1", 0 )
	ROM_LOAD_NIB_HIGH( "blasto.u29", 0x0000, 0x0200, CRC(4dd69499) SHA1(34f097477a297bf5f986804e5967c92f9292be29) )
	ROM_LOAD_NIB_LOW(  "blasto.u43", 0x0000, 0x0200, CRC(104051a4) SHA1(cae6b9d48e3eda5ba12ff5d9835ce2733e90f774) )
ROM_END

ROM_START( hustle )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD_NIB_HIGH( "3160016.u2", 0x0000, 0x0400, CRC(d983de7c) SHA1(af6e0ea78449bfba4fe8affd724d7b0eb3d38706) )
	ROM_LOAD_NIB_LOW(  "3160017.u3", 0x0000, 0x0400, CRC(edec9cb9) SHA1(548cc7b0a15a1c977b7ef4a99ff88101893f661a) )
	ROM_LOAD_NIB_HIGH( "3160018.u4", 0x0400, 0x0400, CRC(f599b9c0) SHA1(c55ed33ac51b9cfbb2fe4321bbb1e0a16694f065) )
	ROM_LOAD_NIB_LOW(  "3160019.u5", 0x0400, 0x0400, CRC(7794bc7e) SHA1(b3d577291dea0e096b2ee56b0ef612f41b2e859c) )

	ROM_REGION( 0x200, "gfx1", 0 )
	ROM_LOAD_NIB_HIGH( "3160020.u29", 0x0000, 0x0200, CRC(541d2c67) SHA1(abdb918f302352693870b0a50eabaf95acf1cf63) )
	ROM_LOAD_NIB_LOW(  "3160021.u43", 0x0000, 0x0200, CRC(b5083128) SHA1(d7e8242e9d12d09f3d69c08e373ede2bdd4deba9) )
ROM_END

ROM_START( mineswpr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD_NIB_HIGH( "mineswee.h0p", 0x0000, 0x0400, CRC(5850a4ba) SHA1(9f097d31428f4494573187049c53fbed2075ff32) )
	ROM_LOAD_NIB_LOW(  "mineswee.l0p", 0x0000, 0x0400, CRC(05961379) SHA1(3d59341be8a663e8c54c1556442c992a6eb886ab) )

	ROM_REGION( 0x200, "gfx1", 0 )
	ROM_LOAD_NIB_HIGH( "mineswee.ums", 0x0000, 0x0200, CRC(0e1c5c37) SHA1(d4d56bd63307e387771c48304724dfc1ea1306d9) )
	ROM_LOAD_NIB_LOW(  "mineswee.uls", 0x0000, 0x0200, CRC(3a4f66e1) SHA1(bd7f6c51d568a79fb06414b2a6ef245d0d983c3e) )
ROM_END

ROM_START( mineswpr4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD_NIB_HIGH( "mineswee.h0p", 0x0000, 0x0400, CRC(5850a4ba) SHA1(9f097d31428f4494573187049c53fbed2075ff32) )
	ROM_LOAD_NIB_LOW(  "mineswee.l0p", 0x0000, 0x0400, CRC(05961379) SHA1(3d59341be8a663e8c54c1556442c992a6eb886ab) )

	ROM_REGION( 0x200, "gfx1", 0 )
	ROM_LOAD_NIB_HIGH( "mineswee.cms", 0x0000, 0x0200, CRC(aad3ce0c) SHA1(92257706ae0c9c1a258eed3311116063e647e1ae) )
	ROM_LOAD_NIB_LOW(  "mineswee.cls", 0x0000, 0x0200, CRC(70959755) SHA1(f62d448742da3fae8bbd96eb3a2714db500cecce) )
ROM_END

/*************************************
 *
 *  Game driver
 *
 *************************************/

GAMEL(1976, blockade,  0,        blockade, blockade, driver_device, 0, ROT0, "Gremlin", "Blockade", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_blockade )
GAMEL(1976, comotion,  0,        blockade, comotion, driver_device, 0, ROT0, "Gremlin", "Comotion", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_blockade )
GAME( 1978, blasto,    0,        blasto,   blasto, driver_device,   0, ROT0, "Gremlin", "Blasto", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // b/w, no overlay
GAMEL(1977, hustle,    0,        blasto,   hustle, driver_device,   0, ROT0, "Gremlin", "Hustle", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_blockade )
GAME( 1977, mineswpr,  0,        blasto,   mineswpr, driver_device, 0, ROT0, "Amutech", "Minesweeper", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1977, mineswpr4, mineswpr, blasto,   mineswpr4, driver_device,0, ROT0, "Amutech", "Minesweeper (4-Player)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

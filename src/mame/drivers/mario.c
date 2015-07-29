// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/***************************************************************************

TODO:
    - start up sound (Sound #2 should play but does not

Done:
    - discrete sound
    - hooked up z80dma
    - combined memory maps
    - statics in mario_state struct
    - fixed save state issues
    - combine sh_* writes into one routine
    - Hooked up flipscreen
    - Changed monitor orientation to ROT0
    - fixed mario0110u1gre
    - rewrote driver, separate MACHINE_DRIVER(mario_audio)
    - palette from schematics
    - video timing from schematics
    - driver configuration switch Nintendo/Std Monitor
    - got rid of COLORTABLE
    - clocks as defines in .h
    - use XTAL_*

Mario Bros memory map (preliminary):

driver by Mirko Buffoni


0000-5fff ROM
6000-6fff RAM
7000-73ff ?
7400-77ff Video RAM
f000-ffff ROM

read:
7c00      IN0
7c80      IN1
7f80      DSW

*
 * IN0 (bits NOT inverted)
 * bit 7 : TEST
 * bit 6 : START 2
 * bit 5 : START 1
 * bit 4 : JUMP player 1
 * bit 3 : ? DOWN player 1 ?
 * bit 2 : ? UP player 1 ?
 * bit 1 : LEFT player 1
 * bit 0 : RIGHT player 1
 *
*
 * IN1 (bits NOT inverted)
 * bit 7 : ?
 * bit 6 : COIN 2
 * bit 5 : COIN 1
 * bit 4 : JUMP player 2
 * bit 3 : ? DOWN player 2 ?
 * bit 2 : ? UP player 2 ?
 * bit 1 : LEFT player 2
 * bit 0 : RIGHT player 2
 *
*
 * DSW (bits NOT inverted)
 * bit 7 : \ difficulty
 * bit 6 : / 00 = easy  01 = medium  10 = hard  11 = hardest
 * bit 5 : \ bonus
 * bit 4 : / 00 = 20000  01 = 30000  10 = 40000  11 = none
 * bit 3 : \ coins per play
 * bit 2 : /
 * bit 1 : \ 00 = 3 lives  01 = 4 lives
 * bit 0 : / 10 = 5 lives  11 = 6 lives
 *

write:
7d00      vertical scroll (pow)
7d80      ?
7e00      sound
7e80-7e82 ?
7e83      sprite palette bank select
7e84      interrupt enable
7e85      ?
7f00-7f07 sound triggers


I/O ports

write:
00        ?

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80dma.h"
#include "sound/ay8910.h"

#include "includes/mario.h"

/*************************************
 *
 *  statics
 *
 *************************************/

READ8_MEMBER(mario_state::memory_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

WRITE8_MEMBER(mario_state::memory_write_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.write_byte(offset, data);
}

WRITE8_MEMBER(mario_state::mario_z80dma_rdy_w)
{
	m_z80dma->rdy_w(data & 0x01);
}

WRITE8_MEMBER(mario_state::nmi_mask_w)
{
	m_nmi_mask = data & 1;
}

/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( mario_map, AS_PROGRAM, 8, mario_state)
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x6fff) AM_RAM
	AM_RANGE(0x7000, 0x73ff) AM_RAM AM_SHARE("spriteram") /* physical sprite ram */
	AM_RANGE(0x7400, 0x77ff) AM_RAM_WRITE(mario_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x7c00, 0x7c00) AM_READ_PORT("IN0") AM_WRITE(mario_sh1_w) /* Mario run sample */
	AM_RANGE(0x7c80, 0x7c80) AM_READ_PORT("IN1") AM_WRITE(mario_sh2_w) /* Luigi run sample */
	AM_RANGE(0x7d00, 0x7d00) AM_WRITE(mario_scroll_w)
	AM_RANGE(0x7e00, 0x7e00) AM_WRITE(mario_sh_tuneselect_w)
	AM_RANGE(0x7e80, 0x7e80) AM_WRITE(mario_gfxbank_w)
	AM_RANGE(0x7e82, 0x7e82) AM_WRITE(mario_flip_w)
	AM_RANGE(0x7e83, 0x7e83) AM_WRITE(mario_palettebank_w)
	AM_RANGE(0x7e84, 0x7e84) AM_WRITE(nmi_mask_w)
	AM_RANGE(0x7e85, 0x7e85) AM_WRITE(mario_z80dma_rdy_w)   /* ==> DMA Chip */
	AM_RANGE(0x7f00, 0x7f07) AM_WRITE(mario_sh3_w) /* Sound port */
	AM_RANGE(0x7f80, 0x7f80) AM_READ_PORT("DSW")    /* DSW */
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( masao_map, AS_PROGRAM, 8, mario_state)
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x6fff) AM_RAM
	AM_RANGE(0x7000, 0x73ff) AM_RAM AM_SHARE("spriteram") /* physical sprite ram */
	AM_RANGE(0x7400, 0x77ff) AM_RAM_WRITE(mario_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x7c00, 0x7c00) AM_READ_PORT("IN0")
	AM_RANGE(0x7c80, 0x7c80) AM_READ_PORT("IN1")
	AM_RANGE(0x7d00, 0x7d00) AM_WRITE(mario_scroll_w)
	AM_RANGE(0x7e00, 0x7e00) AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0x7e80, 0x7e80) AM_WRITE(mario_gfxbank_w)
	AM_RANGE(0x7e82, 0x7e82) AM_WRITE(mario_flip_w)
	AM_RANGE(0x7e83, 0x7e83) AM_WRITE(mario_palettebank_w)
	AM_RANGE(0x7e84, 0x7e84) AM_WRITE(nmi_mask_w)
	AM_RANGE(0x7e85, 0x7e85) AM_WRITE(mario_z80dma_rdy_w)   /* ==> DMA Chip */
	AM_RANGE(0x7f00, 0x7f00) AM_WRITE(masao_sh_irqtrigger_w)
	AM_RANGE(0x7f80, 0x7f80) AM_READ_PORT("DSW")    /* DSW */
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mario_io_map, AS_IO, 8, mario_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVREADWRITE("z80dma", z80dma_device, read, write)  /* dma controller */
ADDRESS_MAP_END


static ADDRESS_MAP_START( mariobl_map, AS_PROGRAM, 8, mario_state)
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x6fff) AM_RAM
	AM_RANGE(0x7000, 0x71ff) AM_RAM AM_SHARE("spriteram") /* physical sprite ram */
	AM_RANGE(0x7200, 0x72ff) AM_RAM // attrram? (only enough for sprites?)
	AM_RANGE(0x7300, 0x737f) AM_RAM // probably x-scroll?
	AM_RANGE(0x7380, 0x7380) AM_WRITE(mariobl_scroll_w)
	AM_RANGE(0x7281, 0x73ff) AM_RAM // seems to have scroll vals for every column on this bl
	AM_RANGE(0x7400, 0x77ff) AM_RAM_WRITE(mario_videoram_w) AM_SHARE("videoram")
	//AM_RANGE(0xa000, 0xa000) AM_READ_PORT("IN1")
	AM_RANGE(0xa000, 0xa000) AM_READNOP   /* watchdog? */
	AM_RANGE(0xa100, 0xa100) AM_READ_PORT("DSW")    /* DSW */
	AM_RANGE(0xa206, 0xa206) AM_WRITE(mario_gfxbank_w)

	AM_RANGE(0x8000, 0x9fff) AM_ROM
	AM_RANGE(0xb000, 0xbfff) AM_ROM
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mariobl_io_map, AS_IO, 8, mario_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVREADWRITE("ay1", ay8910_device, data_r, address_w)
	AM_RANGE(0x01, 0x01) AM_DEVWRITE("ay1", ay8910_device, data_w)
	AM_RANGE(0x80, 0x80) AM_DEVREADWRITE("ay2", ay8910_device, data_r, address_w)
	AM_RANGE(0x81, 0x81) AM_DEVWRITE("ay2", ay8910_device, data_w)
ADDRESS_MAP_END


/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( mario )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_HIGH )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 ) /* doesn't work in game, but does in service mode */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x00, "20k only" )
	PORT_DIPSETTING(    0x10, "30k only" )
	PORT_DIPSETTING(    0x20, "40k only" )
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:!7,!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Hardest ) )

	PORT_START("MONITOR")
	PORT_CONFNAME( 0x01, 0x00, "Monitor" )
	PORT_CONFSETTING(    0x00, "Nintendo" )
	PORT_CONFSETTING(    0x01, "Std 15.72Khz" )

INPUT_PORTS_END

static INPUT_PORTS_START( marioo )
	PORT_INCLUDE( mario )

	PORT_MODIFY( "IN1" )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 )
INPUT_PORTS_END

static INPUT_PORTS_START( marioj )
	PORT_INCLUDE( mario )

	PORT_MODIFY( "DSW" )
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x1c, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!3,!4,!5")
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x20, 0x20, "2 Players Game" )        PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x00, "1 Credit" )
	PORT_DIPSETTING(    0x20, "2 Credits" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:!7,!8")
	PORT_DIPSETTING(    0x00, "20k 50k 30k+" )
	PORT_DIPSETTING(    0x40, "30k 60k 30k+" )
	PORT_DIPSETTING(    0x80, "40k 70k 30k+" )
	PORT_DIPSETTING(    0xc0, DEF_STR( None ) )
INPUT_PORTS_END

static INPUT_PORTS_START( masao )
	PORT_INCLUDE( marioo )

	PORT_MODIFY( "DSW" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x00, "20k 40k 20k+" )
	PORT_DIPSETTING(    0x10, "30k 50k 20k+" )
	PORT_DIPSETTING(    0x20, "40k 60k 20k+" )
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
INPUT_PORTS_END


static INPUT_PORTS_START( mariobl )

	PORT_START("SYSTEM")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x1c, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!3,!4,!5")
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x20, 0x20, "2 Players Game" )        PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x00, "1 Credit" )
	PORT_DIPSETTING(    0x20, "2 Credits" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:!7,!8")
	PORT_DIPSETTING(    0x00, "20k 50k 30k+" )
	PORT_DIPSETTING(    0x40, "30k 60k 30k+" )
	PORT_DIPSETTING(    0x80, "40k 70k 30k+" )
	PORT_DIPSETTING(    0xc0, DEF_STR( None ) )

	PORT_START("MONITOR")
	PORT_CONFNAME( 0x01, 0x00, "Monitor" )
	PORT_CONFSETTING(    0x00, "Nintendo" )
	PORT_CONFSETTING(    0x01, "Std 15.72Khz" )

INPUT_PORTS_END



static INPUT_PORTS_START( dkong3abl )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("MONITOR")
	PORT_CONFNAME( 0x01, 0x00, "Monitor" )
	PORT_CONFSETTING(    0x00, "Nintendo" )
	PORT_CONFSETTING(    0x01, "Std 15.72Khz" )

INPUT_PORTS_END

/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	512,    /* 512 characters */
	2,  /* 2 bits per pixel */
	{ 512*8*8, 0 }, /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 }, /* pretty straightforward layout */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};


static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	256,    /* 256 sprites */
	3,  /* 3 bits per pixel */
	{ 2*256*16*16, 256*16*16, 0 },  /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7,       /* the two halves of the sprite are separated */
			256*16*8+0, 256*16*8+1, 256*16*8+2, 256*16*8+3, 256*16*8+4, 256*16*8+5, 256*16*8+6, 256*16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*8    /* every sprite takes 16 consecutive bytes */
};

static GFXDECODE_START( mario )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0, 32 )
GFXDECODE_END

static const gfx_layout spritelayout_bl =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,3),    /* 256 sprites */
	3,  /* 3 bits per pixel */
	{ RGN_FRAC(2,3),RGN_FRAC(1,3),RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			64,65,66,67,68,69,70,71 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	16*16
};

static GFXDECODE_START( mariobl )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,      0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout_bl, 0, 32 )
GFXDECODE_END

static const gfx_layout spritelayout_bl2 =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,2),    /* 256 sprites */
	2,  /* 3 bits per pixel */
	{ RGN_FRAC(1,2),RGN_FRAC(0,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			64,65,66,67,68,69,70,71 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	16*16
};

static GFXDECODE_START( dkong3abl )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,      0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout_bl2,     0, 32 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

INTERRUPT_GEN_MEMBER(mario_state::vblank_irq)
{
	if(m_nmi_mask)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_CONFIG_START( mario_base, mario_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, Z80_CLOCK) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(mario_map)
	MCFG_CPU_IO_MAP(mario_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", mario_state,  vblank_irq)

	/* devices */
	MCFG_DEVICE_ADD("z80dma", Z80DMA, Z80_CLOCK)
	MCFG_Z80DMA_OUT_BUSREQ_CB(INPUTLINE("maincpu", INPUT_LINE_HALT))
	MCFG_Z80DMA_IN_MREQ_CB(READ8(mario_state, memory_read_byte))
	MCFG_Z80DMA_OUT_MREQ_CB(WRITE8(mario_state, memory_write_byte))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(mario_state, screen_update_mario)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", mario)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(mario_state, mario)

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mario, mario_base )

	/* basic machine hardware */

	/* sound hardware */
	MCFG_FRAGMENT_ADD(mario_audio)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( masao, mario_base )

	/* basic machine hardware */

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(4000000)        /* 4.000 MHz (?) */
	MCFG_CPU_PROGRAM_MAP(masao_map)

	/* sound hardware */
	MCFG_FRAGMENT_ADD(masao_audio)
MACHINE_CONFIG_END

/*
Mario Bros japan bootleg on Ambush hardware

This romset (japanese version) comes from a faulty bootleg pcb.Game differences are none.
Note:it runs on a modified (extended) Tecfri's Ambush hardware.
Main cpu Z80
Sound ic AY-3-8910 x2 -instead of AY-3-8912 x2 of Ambush
Work ram 4Kb (6116 x2) -double of Ambush
OSC: 18,432 Mhz
Rom definition:
mbjba-6, mbjba-7, mbjba-8 main program
mbjba-1 to mbjba-5 gfx (chars,sprites)
Eproms are 2732,2764,27128

Dumped by tirino73
*/

static MACHINE_CONFIG_START( mariobl, mario_state )

	MCFG_CPU_ADD("maincpu", Z80, XTAL_18_432MHz/6)      /* XTAL confirmed, divisor guessed */
	MCFG_CPU_PROGRAM_MAP(mariobl_map)
	MCFG_CPU_IO_MAP(mariobl_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", mario_state,  irq0_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(mario_state, screen_update_mariobl)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", mariobl)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(mario_state, mario)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, XTAL_18_432MHz/6/2)   /* XTAL confirmed, divisor guessed */
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("SYSTEM"))
//  MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(mario_state, ay1_outputb_w))

	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.33)

	MCFG_SOUND_ADD("ay2", AY8910, XTAL_18_432MHz/6/2)   /* XTAL confirmed, divisor guessed */
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("INPUTS"))
//  MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(mario_state, ay2_outputb_w))

	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.33)

MACHINE_CONFIG_END

/*
Donkey Kong 3 bootleg on Ambush hardware

This romset comes from a faulty bootleg pcb.Game differences are none.
Note:it runs on a modified (extended) Tecfri's Ambush hardware.
Main cpu Z80
Sound ic AY-3-8910 x2 -instead of AY-3-8912 x2 of Ambush
Work ram 4Kb (6116 x2) -double of Ambush
OSC: 18,432 Mhz
Rom definition:
dk3ba-5,dk3ba-6,dk3ba-7 main program
dk3ba-1 to dk3ba-4 gfx (chars,sprites)
Eproms are 2732,2764,27128

Dumped by tirino73 >isolani1973@libero.it<
*/
static MACHINE_CONFIG_DERIVED( dkong3abl, mariobl )
	MCFG_GFXDECODE_MODIFY("gfxdecode", dkong3abl)
MACHINE_CONFIG_END

/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( mario )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tma1-c-7f_f.7f",  0x0000, 0x2000, CRC(c0c6e014) SHA1(36a04f9ca1c2a583477cb8a6f2ef94e044e08296) )
	ROM_LOAD( "tma1-c-7e_f.7e",  0x2000, 0x2000, CRC(94fb60d6) SHA1(e74d74aa27f87a164bdd453ab0076efeeb7d4ea3) )
	ROM_LOAD( "tma1-c-7d_f.7d",  0x4000, 0x2000, CRC(dcceb6c1) SHA1(b19804e69ce2c98cf276c6055c3a250316b96b45) )
	ROM_LOAD( "tma1-c-7c_f.7c",  0xf000, 0x1000, CRC(4a63d96b) SHA1(b09060b2c84ab77cc540a27b8f932cb60ec8d442) )

	ROM_REGION( 0x1800, "audiocpu", 0 ) /* sound */
	/* internal rom */
	ROM_FILL(                 0x0000, 0x0800, 0x00)
	/* first half banked */
	ROM_LOAD( "tma1-c-6k_e.6k",   0x1000, 0x0800, CRC(06b9ff85) SHA1(111a29bcb9cda0d935675fa26eca6b099a88427f) )
	/* second half always read */
	ROM_CONTINUE(             0x0800, 0x0800)

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "tma1-v-3f.3f",     0x0000, 0x1000, CRC(28b0c42c) SHA1(46749568aff88a28c3b6a1ac423abd1b90742a4d) )
	ROM_LOAD( "tma1-v-3j.3j",     0x1000, 0x1000, CRC(0c8cc04d) SHA1(15fae47d701dc1ef15c943cee6aa991776ecffdf) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "tma1-v-7m.7m",     0x0000, 0x1000, CRC(22b7372e) SHA1(4a1c1e239cb6d483e76f50d7a3b941025963c6a3) )
	ROM_LOAD( "tma1-v-7n.7n",     0x1000, 0x1000, CRC(4f3a1f47) SHA1(0747d693b9482f6dd28b0bc484fd1d3e29d35654) )
	ROM_LOAD( "tma1-v-7p.7p",     0x2000, 0x1000, CRC(56be6ccd) SHA1(15a6e16c189d45f72761ebcbe9db5001bdecd659) )
	ROM_LOAD( "tma1-v-7s.7s",     0x3000, 0x1000, CRC(56f1d613) SHA1(9af6844dbaa3615433d0595e9e85e72493e31a54) )
	ROM_LOAD( "tma1-v-7t.7t",     0x4000, 0x1000, CRC(641f0008) SHA1(589fe108c7c11278fd897f2ded8f0498bc149cfd) )
	ROM_LOAD( "tma1-v-7u.7u",     0x5000, 0x1000, CRC(7baf5309) SHA1(d9194ff7b89a18273d37b47228fc7fb7e2a0ed1f) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tma1-c-4p_1.4p", 0x0000, 0x0200, CRC(8187d286) SHA1(8a6d8e622599f1aacaeb10f7b1a39a23c8a840a0) ) /* BPROM was a MB7124E read as 82S147 */

	ROM_REGION( 0x0020, "unk_proms", 0 ) /* is this the color prom? */
	ROM_LOAD( "tma1-c-5p.5p", 0x0000, 0x0020, CRC(58d86098) SHA1(d654995004b9052b12d3b682a2b39530e70030fc) ) /* BPROM was a TBP18S030N read as 82S123, unknown use */
ROM_END

ROM_START( marioe )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tma1-c-7f_e-1.7f",     0x0000, 0x2000, CRC(c0c6e014) SHA1(36a04f9ca1c2a583477cb8a6f2ef94e044e08296) )
	ROM_LOAD( "tma1-c-7e_e-3.7e",     0x2000, 0x2000, CRC(b09ab857) SHA1(35b91cd1c4c3dd2d543a1ea8ff7b951715727792) )
	ROM_LOAD( "tma1-c-7d_e-1.7d",     0x4000, 0x2000, CRC(dcceb6c1) SHA1(b19804e69ce2c98cf276c6055c3a250316b96b45) )
	ROM_LOAD( "tma1-c-7c_e-3.7c",     0xf000, 0x1000, CRC(0d31bd1c) SHA1(a2e238470ba2ea3c81225fec687f61f047c68c59) )

	ROM_REGION( 0x1800, "audiocpu", 0 ) /* sound */
	/* internal rom */
	ROM_FILL(                 0x0000, 0x0800, 0x00)
	/* first half banked */
	ROM_LOAD( "tma1-c-6k_e.6k",   0x1000, 0x0800, CRC(06b9ff85) SHA1(111a29bcb9cda0d935675fa26eca6b099a88427f) )
	/* second half always read */
	ROM_CONTINUE(             0x0800, 0x0800)

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "tma1-v-3f.3f",     0x0000, 0x1000, CRC(28b0c42c) SHA1(46749568aff88a28c3b6a1ac423abd1b90742a4d) )
	ROM_LOAD( "tma1-v-3j.3j",     0x1000, 0x1000, CRC(0c8cc04d) SHA1(15fae47d701dc1ef15c943cee6aa991776ecffdf) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "tma1-v.7m",     0x0000, 0x1000, CRC(d01c0e2c) SHA1(cd6cc9d69c36db15543601f5da2abf109cde43c9) )
	ROM_LOAD( "tma1-v-7n.7n",  0x1000, 0x1000, CRC(4f3a1f47) SHA1(0747d693b9482f6dd28b0bc484fd1d3e29d35654) )
	ROM_LOAD( "tma1-v-7p.7p",  0x2000, 0x1000, CRC(56be6ccd) SHA1(15a6e16c189d45f72761ebcbe9db5001bdecd659) )
	ROM_LOAD( "tma1-v.7s",     0x3000, 0x1000, CRC(ff856e6f) SHA1(2bc9ff18bb1842e8de2bc61ac828f1b175290bed) )
	ROM_LOAD( "tma1-v-7t.7t",  0x4000, 0x1000, CRC(641f0008) SHA1(589fe108c7c11278fd897f2ded8f0498bc149cfd) )
	ROM_LOAD( "tma1-v.7u",     0x5000, 0x1000, CRC(d2dbeb75) SHA1(676cf3e15252cd0d9e926ca15c3aa0caa39be269) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tma1-c-4p_1.4p",     0x0000, 0x0200, CRC(8187d286) SHA1(8a6d8e622599f1aacaeb10f7b1a39a23c8a840a0) ) /* BPROM was a MB7124E read as 82S147 */

	ROM_REGION( 0x0020, "unk_proms", 0 ) /* is this the color prom? */
	ROM_LOAD( "tma1-c-5p.5p", 0x0000, 0x0020, CRC(58d86098) SHA1(d654995004b9052b12d3b682a2b39530e70030fc) ) /* BPROM was a TBP18S030N read as 82S123, unknown use */
ROM_END

ROM_START( marioo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tma1-c-7f_.7f",     0x0000, 0x2000, CRC(c0c6e014) SHA1(36a04f9ca1c2a583477cb8a6f2ef94e044e08296) ) /* Uknown revision */
	ROM_LOAD( "tma1-c-7f_.7e",     0x2000, 0x2000, CRC(116b3856) SHA1(e372f846d0e5a2b9b47ebd0330293fcc8a12363f) )
	ROM_LOAD( "tma1-c-7f_.7d",     0x4000, 0x2000, CRC(dcceb6c1) SHA1(b19804e69ce2c98cf276c6055c3a250316b96b45) )
	ROM_LOAD( "tma1-c-7f_.7c",     0xf000, 0x1000, CRC(4a63d96b) SHA1(b09060b2c84ab77cc540a27b8f932cb60ec8d442) )

	ROM_REGION( 0x1800, "audiocpu", 0 ) /* sound */
	/* internal rom */
	ROM_FILL(                 0x0000, 0x0800, 0x00)
	/* first half banked */
	ROM_LOAD( "tma1-c-6k_e.6k",   0x1000, 0x0800, CRC(06b9ff85) SHA1(111a29bcb9cda0d935675fa26eca6b099a88427f) )
	/* second half always read */
	ROM_CONTINUE(             0x0800, 0x0800)

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "tma1-v-3f.3f",     0x0000, 0x1000, CRC(28b0c42c) SHA1(46749568aff88a28c3b6a1ac423abd1b90742a4d) )
	ROM_LOAD( "tma1-v-3j.3j",     0x1000, 0x1000, CRC(0c8cc04d) SHA1(15fae47d701dc1ef15c943cee6aa991776ecffdf) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "tma1-v-7m.7m",     0x0000, 0x1000, CRC(22b7372e) SHA1(4a1c1e239cb6d483e76f50d7a3b941025963c6a3) )
	ROM_LOAD( "tma1-v-7n.7n",     0x1000, 0x1000, CRC(4f3a1f47) SHA1(0747d693b9482f6dd28b0bc484fd1d3e29d35654) )
	ROM_LOAD( "tma1-v-7p.7p",     0x2000, 0x1000, CRC(56be6ccd) SHA1(15a6e16c189d45f72761ebcbe9db5001bdecd659) )
	ROM_LOAD( "tma1-v-7s.7s",     0x3000, 0x1000, CRC(56f1d613) SHA1(9af6844dbaa3615433d0595e9e85e72493e31a54) )
	ROM_LOAD( "tma1-v-7t.7t",     0x4000, 0x1000, CRC(641f0008) SHA1(589fe108c7c11278fd897f2ded8f0498bc149cfd) )
	ROM_LOAD( "tma1-v-7u.7u",     0x5000, 0x1000, CRC(7baf5309) SHA1(d9194ff7b89a18273d37b47228fc7fb7e2a0ed1f) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tma1-c-4p.4p",     0x0000, 0x0200, CRC(afc9bd41) SHA1(90b739c4c7f24a88b6ac5ca29b06c032906a2801) )

	ROM_REGION( 0x0020, "unk_proms", 0 ) /* is this the color prom? */
	ROM_LOAD( "tma1-c-5p.5p", 0x0000, 0x0020, CRC(58d86098) SHA1(d654995004b9052b12d3b682a2b39530e70030fc) ) /* BPROM was a TBP18S030N read as 82S123, unknown use */
ROM_END

ROM_START( marioj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tma1c-a1.7f",  0x0000, 0x2000, CRC(b64b6330) SHA1(f7084251ac325bbfa3fb804da16a50622e1fd213) )
	ROM_LOAD( "tma1c-a2.7e",  0x2000, 0x2000, CRC(290c4977) SHA1(5af266be0ddc883c6548c90e4a9084024a1e91a0) )
	ROM_LOAD( "tma1c-a1.7d",  0x4000, 0x2000, CRC(f8575f31) SHA1(710d0e72fcfce700ed2a22fb9c7c392cc76b250b) )
	ROM_LOAD( "tma1c-a2.7c",  0xf000, 0x1000, CRC(a3c11e9e) SHA1(d0612b0f8c2ea4e798f551922a04a324f4ed5f3d) )

	ROM_REGION( 0x1800, "audiocpu", 0 ) /* sound */
	/* internal rom */
	ROM_FILL(                 0x0000, 0x0800, 0x00)
	/* first half banked */
	ROM_LOAD( "tma1c-a.6k",   0x1000, 0x0800, CRC(06b9ff85) SHA1(111a29bcb9cda0d935675fa26eca6b099a88427f) )
	/* second half always read */
	ROM_CONTINUE(             0x0800, 0x0800)

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "tma1v-a.3f",   0x0000, 0x1000, CRC(adf49ee0) SHA1(11fc2cd197bfe3ecb6af55c3c7a326c94988d2bd) )
	ROM_LOAD( "tma1v-a.3j",   0x1000, 0x1000, CRC(a5318f2d) SHA1(e42f5e51804195c64a56addb18b7ad12c57bb09a) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "tma1v-a.7m",   0x0000, 0x1000, CRC(186762f8) SHA1(711fdd37392656bdd5027e020d51d083ccd7c407) )
	ROM_LOAD( "tma1v-a.7n",   0x1000, 0x1000, CRC(e0e08bba) SHA1(315eba2c10d426c9c0bb4e36987bf8ebed7df9a0) )
	ROM_LOAD( "tma1v-a.7p",   0x2000, 0x1000, CRC(7b27c8c1) SHA1(3fb2613ce19e353fbcc77b6817927794fb35810f) )
	ROM_LOAD( "tma1v-a.7s",   0x3000, 0x1000, CRC(912ba80a) SHA1(351fb5b160216eb10e281815d05a7165ca0e5909) )
	ROM_LOAD( "tma1v-a.7t",   0x4000, 0x1000, CRC(5cbb92a5) SHA1(a78a378e6d3060143dc456e9c33a5068da648331) )
	ROM_LOAD( "tma1v-a.7u",   0x5000, 0x1000, CRC(13afb9ed) SHA1(b29dcd91cf5e639ee50b734afc7a3afce79634df) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tma1-c-4p.4p",     0x0000, 0x0200, CRC(afc9bd41) SHA1(90b739c4c7f24a88b6ac5ca29b06c032906a2801) )

	ROM_REGION( 0x0020, "unk_proms", 0 ) /* is this the color prom? */
	ROM_LOAD( "tma1-c-5p.5p", 0x0000, 0x0020, CRC(58d86098) SHA1(d654995004b9052b12d3b682a2b39530e70030fc) ) /* BPROM was a TBP18S030N read as 82S123, unknown use */
ROM_END

ROM_START( masao )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "masao-4.rom",  0x0000, 0x2000, CRC(07a75745) SHA1(acc760242a8862d177e3cff90aa32c4f3dac4e65) )
	ROM_LOAD( "masao-3.rom",  0x2000, 0x2000, CRC(55c629b6) SHA1(1f5b5699821871aadacc511663cb4bd4e357e215) )
	ROM_LOAD( "masao-2.rom",  0x4000, 0x2000, CRC(42e85240) SHA1(bc8cdf867b743c5ee58fcacb63a44f826c8f8c1a) )
	ROM_LOAD( "masao-1.rom",  0xf000, 0x1000, CRC(b2817af9) SHA1(95e83752e544671a68df2107fae1010b187f04a6) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for sound */
	ROM_LOAD( "masao-5.rom",  0x0000, 0x1000, CRC(bd437198) SHA1(ebae88461984afc97bbc103fc6d95bc3c1865eec) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "masao-6.rom",  0x0000, 0x1000, CRC(1c9e0be2) SHA1(b4a650412dad90c6f6d79e93cde49055703b7f3e) )
	ROM_LOAD( "masao-7.rom",  0x1000, 0x1000, CRC(747c1349) SHA1(54674f78edf86953b7d500b66393483d1a5ce8ab) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "tma1v-a.7m",   0x0000, 0x1000, CRC(186762f8) SHA1(711fdd37392656bdd5027e020d51d083ccd7c407) )
	ROM_LOAD( "masao-9.rom",  0x1000, 0x1000, CRC(50be3918) SHA1(73e22eee67a03732ff57e523f900f20c6aee0491) )
	ROM_LOAD( "mario.7p",     0x2000, 0x1000, CRC(56be6ccd) SHA1(15a6e16c189d45f72761ebcbe9db5001bdecd659) )
	ROM_LOAD( "tma1v-a.7s",   0x3000, 0x1000, CRC(912ba80a) SHA1(351fb5b160216eb10e281815d05a7165ca0e5909) )
	ROM_LOAD( "tma1v-a.7t",   0x4000, 0x1000, CRC(5cbb92a5) SHA1(a78a378e6d3060143dc456e9c33a5068da648331) )
	ROM_LOAD( "tma1v-a.7u",   0x5000, 0x1000, CRC(13afb9ed) SHA1(b29dcd91cf5e639ee50b734afc7a3afce79634df) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tma1-c-4p.4p",     0x0000, 0x0200, CRC(afc9bd41) SHA1(90b739c4c7f24a88b6ac5ca29b06c032906a2801) )
ROM_END

ROM_START( mariobl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mbjba-8.7i",    0x0000, 0x4000, CRC(344c959d) SHA1(162e39c3a17e0dcde3b7eefebe224318c8884de2) )
	ROM_LOAD( "mbjba-7.7g",    0x4000, 0x2000, CRC(06faf308) SHA1(8c213d9390c168034c1673f3dd97b99322b3485a) )
	ROM_LOAD( "mbjba-6.7f",    0xe000, 0x2000, CRC(761dd670) SHA1(6d6e45ced8c535cdf56e0ed1bcedb342e9e10a55) )

	ROM_REGION( 0x2000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "mbjba-4.4l",   0x1000, 0x1000, CRC(9379e836) SHA1(fcce66c1b2c5120441840b80723c7d209d42bc45) )
	ROM_LOAD( "mbjba-5.4n",   0x0000, 0x1000, CRC(9bbcf9fb) SHA1(a917241a3bd94bff72f509d6b3ab8358b9c03eac) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "mbjba-1.3l",   0x4000, 0x2000, CRC(c772cb8f) SHA1(7fd6dd9888928fad5c50d96b4ecff954ea8975ce) )
	ROM_LOAD( "mbjba-2.3ls",  0x2000, 0x2000, CRC(7b58c92e) SHA1(25dfce7a4a93f661f495cc80378d445a2b064ba7) )
	ROM_LOAD( "mbjba-3.3ns",  0x0000, 0x2000, CRC(3981adb2) SHA1(c12a0c2ae04de6969f4b2dae3bdefc4515d87c55) )

	ROM_REGION( 0x0200, "proms", 0 ) // no prom was present in the dump, assuming to be the same
	ROM_LOAD( "tma1-c-4p.4p",     0x0000, 0x0200, CRC(afc9bd41) SHA1(90b739c4c7f24a88b6ac5ca29b06c032906a2801) )
ROM_END

ROM_START( dkong3abl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dk3ba-7.7i",    0x0000, 0x4000, CRC(a9263275) SHA1(c3867f6b0d379b70669b3b954e582533406db203) )
	ROM_LOAD( "dk3ba-6.7g",    0x4000, 0x2000, CRC(31b8401d) SHA1(0e3dfea0c7fe99d48c5d984c47fa746caf0879f3) )
	ROM_CONTINUE(0x8000,0x2000)
	ROM_LOAD( "dk3ba-5.7f",    0xb000, 0x1000, CRC(07d3fd88) SHA1(721f401d077e3e051672513f9df5614eeb0f6466) )

	ROM_REGION( 0x2000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "dk3ba-3.4l",   0x1000, 0x1000, CRC(67ac65d4) SHA1(d28bdb99310370513597ca80185ac6c56a11f63c) )
	ROM_LOAD( "dk3ba-4.4n",   0x0000, 0x1000, CRC(84b319d6) SHA1(eaf160948f8cd4fecfdd909876de7cd16340885c) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "dk3ba-1.3l",   0x0000, 0x2000, CRC(d4a88e04) SHA1(4f797c25d26c1022dcf026021979ef0fbab48baf) )
	ROM_LOAD( "dk3ba-2.3m",   0x2000, 0x2000, CRC(f71185ee) SHA1(6652cf958d7afa8bb8dcfded997bb418a75223d8) )

	ROM_REGION( 0x0200, "proms", 0 ) // no prom was present in the dump, probably need to use the original ones again
	ROM_LOAD( "tma1-c-4p.4p",     0x0000, 0x0200, CRC(afc9bd41) SHA1(90b739c4c7f24a88b6ac5ca29b06c032906a2801) ) // this is from mario.. remove
ROM_END

/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1983, mario,    0,       mario,   marioo, driver_device,  0, ROT0, "Nintendo of America", "Mario Bros. (US, Revision F)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, marioe,   mario,   mario,   mario, driver_device,   0, ROT0, "Nintendo of America", "Mario Bros. (US, Revision E)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, marioo,   mario,   mario,   marioo, driver_device,  0, ROT0, "Nintendo of America", "Mario Bros. (US, Unknown Rev)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, marioj,   mario,   mario,   marioj, driver_device,  0, ROT0, "Nintendo", "Mario Bros. (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, masao,    mario,   masao,   masao, driver_device,   0, ROT0, "bootleg", "Masao", MACHINE_SUPPORTS_SAVE )

// todo, these might have a better home than in here
GAME( 1983, mariobl,  mario,   mariobl, mariobl,driver_device,  0, ROT180, "bootleg", "Mario Bros. (Japan, bootleg)", MACHINE_SUPPORTS_SAVE ) // was listed as 'on extended Ambush hardware' but doesn't seem similar apart from the sound system?
GAME( 1983, dkong3abl,dkong3,  dkong3abl,dkong3abl,driver_device,0, ROT90,  "bootleg", "Donkey Kong 3 (bootleg with 2xAY8910)", MACHINE_NOT_WORKING ) //  likewise, put here because it's similar to mariobl

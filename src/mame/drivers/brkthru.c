// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/***************************************************************************

    Break Thru Doc. Data East (1986)

    driver by Phil Stroffolino

    UNK-1.1    (16Kb)  Code (4000-7FFF)
    UNK-1.2    (32Kb)  Main 6809 (8000-FFFF)
    UNK-1.3    (32Kb)  Mapped (2000-3FFF)
    UNK-1.4    (32Kb)  Mapped (2000-3FFF)

    UNK-1.5    (32Kb)  Sound 6809 (8000-FFFF)

    Background has 4 banks, with 256 16x16x8 tiles in each bank.
    UNK-1.6    (32Kb)  GFX Background
    UNK-1.7    (32Kb)  GFX Background
    UNK-1.8    (32Kb)  GFX Background

    UNK-1.9    (32Kb)  GFX Sprites
    UNK-1.10   (32Kb)  GFX Sprites
    UNK-1.11   (32Kb)  GFX Sprites

    Text has 256 8x8x4 characters.
    UNK-1.12   (8Kb)   GFX Text

    **************************************************************************
    Memory Map for Main CPU by Carlos A. Lozano
    **************************************************************************

    MAIN CPU
    0000-03FF                                   W                   Plane0
    0400-0BFF                                  R/W                  RAM
    0C00-0FFF                                   W                   Plane2(Background)
    1000-10FF                                   W                   Plane1(sprites)
    1100-17FF                                  R/W                  RAM
    1800-180F                                  R/W                  In/Out
    1810-1FFF                                  R/W                  RAM (unmapped?)
    2000-3FFF                                   R                   ROM Mapped(*)
    4000-7FFF                                   R                   ROM(UNK-1.1)
    8000-FFFF                                   R                   ROM(UNK-1.2)

    Interrupts: Reset, NMI, IRQ
    The test routine is at F000

    Sound: YM2203 and YM3526 driven by 6809.  Sound added by Bryan McPhail, 1/4/98.

    2008-07
    Dip locations verified with manual for brkthru (US conv. kit) and darwin

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "sound/2203intf.h"
#include "sound/3526intf.h"
#include "includes/brkthru.h"


#define MASTER_CLOCK        XTAL_12MHz


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_MEMBER(brkthru_state::brkthru_1803_w)
{
	/* bit 0 = NMI enable */
	m_nmi_mask = ~data & 1;

	if(data & 2)
		m_maincpu->set_input_line(0, CLEAR_LINE);

	/* bit 1 = ? maybe IRQ acknowledge */
}

WRITE8_MEMBER(brkthru_state::darwin_0803_w)
{
	/* bit 0 = NMI enable */
	m_nmi_mask = data & 1;
	logerror("0803 %02X\n",data);

	if(data & 2)
		m_maincpu->set_input_line(0, CLEAR_LINE);


	/* bit 1 = ? maybe IRQ acknowledge */
}

WRITE8_MEMBER(brkthru_state::brkthru_soundlatch_w)
{
	soundlatch_byte_w(space, offset, data);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INPUT_CHANGED_MEMBER(brkthru_state::coin_inserted)
{
	/* coin insertion causes an IRQ */
	if (oldval)
		m_maincpu->set_input_line(0, ASSERT_LINE);
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( brkthru_map, AS_PROGRAM, 8, brkthru_state )
	AM_RANGE(0x0000, 0x03ff) AM_RAM_WRITE(brkthru_fgram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0x0400, 0x0bff) AM_RAM
	AM_RANGE(0x0c00, 0x0fff) AM_RAM_WRITE(brkthru_bgram_w) AM_SHARE("videoram")
	AM_RANGE(0x1000, 0x10ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x1100, 0x17ff) AM_RAM
	AM_RANGE(0x1800, 0x1800) AM_READ_PORT("P1")
	AM_RANGE(0x1801, 0x1801) AM_READ_PORT("P2")
	AM_RANGE(0x1802, 0x1802) AM_READ_PORT("DSW1")
	AM_RANGE(0x1803, 0x1803) AM_READ_PORT("DSW2/COIN")
	AM_RANGE(0x1800, 0x1801) AM_WRITE(brkthru_1800_w)   /* bg scroll and color, ROM bank selection, flip screen */
	AM_RANGE(0x1802, 0x1802) AM_WRITE(brkthru_soundlatch_w)
	AM_RANGE(0x1803, 0x1803) AM_WRITE(brkthru_1803_w)   /* NMI enable, + ? */
	AM_RANGE(0x2000, 0x3fff) AM_ROMBANK("bank1")
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END

/* same as brktrhu, but xor 0x1000 below 8k */
static ADDRESS_MAP_START( darwin_map, AS_PROGRAM, 8, brkthru_state )
	AM_RANGE(0x1000, 0x13ff) AM_RAM_WRITE(brkthru_fgram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0x1400, 0x1bff) AM_RAM
	AM_RANGE(0x1c00, 0x1fff) AM_RAM_WRITE(brkthru_bgram_w) AM_SHARE("videoram")
	AM_RANGE(0x0000, 0x00ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x0100, 0x01ff) AM_WRITENOP /*tidyup, nothing really here?*/
	AM_RANGE(0x0800, 0x0800) AM_READ_PORT("P1")
	AM_RANGE(0x0801, 0x0801) AM_READ_PORT("P2")
	AM_RANGE(0x0802, 0x0802) AM_READ_PORT("DSW1")
	AM_RANGE(0x0803, 0x0803) AM_READ_PORT("DSW2/COIN")
	AM_RANGE(0x0800, 0x0801) AM_WRITE(brkthru_1800_w)     /* bg scroll and color, ROM bank selection, flip screen */
	AM_RANGE(0x0802, 0x0802) AM_WRITE(brkthru_soundlatch_w)
	AM_RANGE(0x0803, 0x0803) AM_WRITE(darwin_0803_w)     /* NMI enable, + ? */
	AM_RANGE(0x2000, 0x3fff) AM_ROMBANK("bank1")
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, brkthru_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2001) AM_DEVWRITE("ym2", ym3526_device, write)
	AM_RANGE(0x4000, 0x4000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x6000, 0x6001) AM_DEVREADWRITE("ym1", ym2203_device, read, write)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( brkthru )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")   /* used only by the self test */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x10, "Enemy Vehicles" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x20, 0x20, "Enemy Bullets" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x40, 0x00, "Control Panel" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, "1 Player" )
	PORT_DIPSETTING(    0x00, "2 Players" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW2/COIN")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "99 (Cheat)")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "20000 Points Only" )
	PORT_DIPSETTING(    0x04, "10000/20000 Points" )
	PORT_DIPSETTING(    0x0c, "20000/30000 Points" )
	PORT_DIPSETTING(    0x08, "20000/40000 Points" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:5") /* Manual says ALWAYS OFF */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	/* According to the manual, bit 5 should control Flip Screen */
//  PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:6")
//  PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* SW2:7,8 ALWAYS OFF according to the manual  */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, brkthru_state,coin_inserted, 0)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, brkthru_state,coin_inserted, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, brkthru_state,coin_inserted, 0)
INPUT_PORTS_END

static INPUT_PORTS_START( brkthruj )
	PORT_INCLUDE( brkthru )

	PORT_MODIFY("DSW2/COIN")
	PORT_SERVICE_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" )
INPUT_PORTS_END

static INPUT_PORTS_START( darwin )
	PORT_INCLUDE( brkthru )

	PORT_MODIFY("DSW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW1:5" )   /* Manual says must be OFF */
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW1:8" )   /* Manual says must be OFF */

	PORT_MODIFY("DSW2/COIN")    /* modified by Shingo Suzuki 1999/11/02 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, "20k, 50k and every 50k" )
	PORT_DIPSETTING(    0x00, "30k, 80k and every 80k" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	/* According to the manual, bit 5 should control Flip Screen */
//  PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:6")
//  PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* SW2:5,7,8 ALWAYS OFF according to the manual  */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, brkthru_state,coin_inserted, 0)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, brkthru_state,coin_inserted, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, brkthru_state,coin_inserted, 0)
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 chars */
	256,    /* 256 characters */
	3,  /* 3 bits per pixel */
	{ 512*8*8+4, 0, 4 },    /* plane offset */
	{ 256*8*8+0, 256*8*8+1, 256*8*8+2, 256*8*8+3, 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static const gfx_layout tilelayout1 =
{
	16,16,  /* 16*16 tiles */
	128,    /* 128 tiles */
	3,  /* 3 bits per pixel */
	{ 0x4000*8+4, 0, 4 },   /* plane offset */
	{ 0, 1, 2, 3, 1024*8*8+0, 1024*8*8+1, 1024*8*8+2, 1024*8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+1024*8*8+0, 16*8+1024*8*8+1, 16*8+1024*8*8+2, 16*8+1024*8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    /* every tile takes 32 consecutive bytes */
};

static const gfx_layout tilelayout2 =
{
	16,16,  /* 16*16 tiles */
	128,    /* 128 tiles */
	3,  /* 3 bits per pixel */
	{ 0x3000*8+0, 0, 4 },   /* plane offset */
	{ 0, 1, 2, 3, 1024*8*8+0, 1024*8*8+1, 1024*8*8+2, 1024*8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+1024*8*8+0, 16*8+1024*8*8+1, 16*8+1024*8*8+2, 16*8+1024*8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    /* every tile takes 32 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	1024,   /* 1024 sprites */
	3,  /* 3 bits per pixel */
	{ 2*1024*32*8, 1024*32*8, 0 },  /* plane offset */
	{ 16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7,
			0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    /* every sprite takes 32 consecutive bytes */
};

static GFXDECODE_START( brkthru )
	GFXDECODE_ENTRY( "gfx1", 0x00000, charlayout,   0x00,  1 )  /* use colors 0x00-0x07 */
	GFXDECODE_ENTRY( "gfx2", 0x00000, tilelayout1,  0x80, 16 )  /* use colors 0x80-0xff */
	GFXDECODE_ENTRY( "gfx2", 0x01000, tilelayout2,  0x80, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x08000, tilelayout1,  0x80, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x09000, tilelayout2,  0x80, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x10000, tilelayout1,  0x80, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x11000, tilelayout2,  0x80, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x18000, tilelayout1,  0x80, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x19000, tilelayout2,  0x80, 16 )
	GFXDECODE_ENTRY( "gfx3", 0x00000, spritelayout, 0x40,  8 )  /* use colors 0x40-0x7f */
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void brkthru_state::machine_start()
{
	save_item(NAME(m_bgscroll));
	save_item(NAME(m_bgbasecolor));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_nmi_mask));
}

void brkthru_state::machine_reset()
{
	m_bgscroll = 0;
	m_bgbasecolor = 0;
	m_flipscreen = 0;
	m_nmi_mask = 0;
}

INTERRUPT_GEN_MEMBER(brkthru_state::vblank_irq)
{
	if(m_nmi_mask)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_CONFIG_START( brkthru, brkthru_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, MASTER_CLOCK/8)        /* 1.5 MHz ? */
	MCFG_CPU_PROGRAM_MAP(brkthru_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", brkthru_state,  vblank_irq)

	MCFG_CPU_ADD("audiocpu", M6809, MASTER_CLOCK/8)     /* 1.5 MHz ? */
	MCFG_CPU_PROGRAM_MAP(sound_map)


	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", brkthru)

	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(brkthru_state, brkthru)

	/* not sure; assuming to be the same as darwin */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK/2, 384, 8, 248, 272, 8, 248)
	MCFG_SCREEN_UPDATE_DRIVER(brkthru_state, screen_update_brkthru)
	MCFG_SCREEN_PALETTE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, MASTER_CLOCK/8)
	MCFG_SOUND_ROUTE(0, "mono", 0.10)
	MCFG_SOUND_ROUTE(1, "mono", 0.10)
	MCFG_SOUND_ROUTE(2, "mono", 0.10)
	MCFG_SOUND_ROUTE(3, "mono", 0.50)

	MCFG_SOUND_ADD("ym2", YM3526, MASTER_CLOCK/4)
	MCFG_YM3526_IRQ_HANDLER(DEVWRITELINE("audiocpu", m6809_device, irq_line))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( darwin, brkthru_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, MASTER_CLOCK/8)        /* 1.5 MHz ? */
	MCFG_CPU_PROGRAM_MAP(darwin_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", brkthru_state,  vblank_irq)

	MCFG_CPU_ADD("audiocpu", M6809, MASTER_CLOCK/8)     /* 1.5 MHz ? */
	MCFG_CPU_PROGRAM_MAP(sound_map)


	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", brkthru)

	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(brkthru_state, brkthru)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK/2, 384, 8, 248, 272, 8, 248)
	/* frames per second, vblank duration
	    Horizontal video frequency:
	        HSync = Dot Clock / Horizontal Frame Length
	              = Xtal /2   / (HDisplay + HBlank)
	              = 12MHz/2   / (240 + 144)
	              = 15.625kHz
	    Vertical Video frequency:
	        VSync = HSync / Vertical Frame Length
	              = HSync / (VDisplay + VBlank)
	              = 15.625kHz / (240 + 32)
	              = 57.444855Hz
	    tuned by Shingo SUZUKI(VSyncMAME Project) 2000/10/19 */
	MCFG_SCREEN_UPDATE_DRIVER(brkthru_state, screen_update_brkthru)
	MCFG_SCREEN_PALETTE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, MASTER_CLOCK/8)
	MCFG_SOUND_ROUTE(0, "mono", 0.10)
	MCFG_SOUND_ROUTE(1, "mono", 0.10)
	MCFG_SOUND_ROUTE(2, "mono", 0.10)
	MCFG_SOUND_ROUTE(3, "mono", 0.50)

	MCFG_SOUND_ADD("ym2", YM3526, MASTER_CLOCK/4)
	MCFG_YM3526_IRQ_HANDLER(DEVWRITELINE("audiocpu", m6809_device, irq_line))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( brkthru )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for main CPU + 64k for banked ROMs */
	ROM_LOAD( "brkthru.1",    0x04000, 0x4000, CRC(cfb4265f) SHA1(4cd748fa06fd2727de1694196912d605672d4883) )
	ROM_LOAD( "brkthru.2",    0x08000, 0x8000, CRC(fa8246d9) SHA1(d6da03b2a3d8a83411191351ee110b89352a3ead) )
	ROM_LOAD( "brkthru.4",    0x10000, 0x8000, CRC(8cabf252) SHA1(45e8847b2e6b278989f67e0b27b827a9b3b92581) )
	ROM_LOAD( "brkthru.3",    0x18000, 0x8000, CRC(2f2c40c2) SHA1(fcb78941453520a3a07f272127dae7c2cc1999ea) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "brkthru.12",   0x00000, 0x2000, CRC(58c0b29b) SHA1(9dc075f8afae7e8fe164a9fe325e9948cdc7e4bb) )   /* characters */

	ROM_REGION( 0x20000, "gfx2", 0 )
	/* background */
	/* we do a lot of scatter loading here, to place the data in a format */
	/* which can be decoded by MAME's standard functions */
	ROM_LOAD( "brkthru.7",    0x00000, 0x4000, CRC(920cc56a) SHA1(c75806691073f1f3bd54dcaca4c14155ecf4471d) )   /* bitplanes 1,2 for bank 1,2 */
	ROM_CONTINUE(             0x08000, 0x4000 )             /* bitplanes 1,2 for bank 3,4 */
	ROM_LOAD( "brkthru.6",    0x10000, 0x4000, CRC(fd3cee40) SHA1(3308b96bb69e0fa6dffbdff296273fafa16d5e70) )   /* bitplanes 1,2 for bank 5,6 */
	ROM_CONTINUE(             0x18000, 0x4000 )             /* bitplanes 1,2 for bank 7,8 */
	ROM_LOAD( "brkthru.8",    0x04000, 0x1000, CRC(f67ee64e) SHA1(75634bd481ae44b8aa02acb4f9b4d7ff973a4c71) )   /* bitplane 3 for bank 1,2 */
	ROM_CONTINUE(             0x06000, 0x1000 )
	ROM_CONTINUE(             0x0c000, 0x1000 )             /* bitplane 3 for bank 3,4 */
	ROM_CONTINUE(             0x0e000, 0x1000 )
	ROM_CONTINUE(             0x14000, 0x1000 )             /* bitplane 3 for bank 5,6 */
	ROM_CONTINUE(             0x16000, 0x1000 )
	ROM_CONTINUE(             0x1c000, 0x1000 )             /* bitplane 3 for bank 7,8 */
	ROM_CONTINUE(             0x1e000, 0x1000 )

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "brkthru.9",    0x00000, 0x8000, CRC(f54e50a7) SHA1(eccf4d859c26944271ec6586644b4730a72851fd) )   /* sprites */
	ROM_LOAD( "brkthru.10",   0x08000, 0x8000, CRC(fd156945) SHA1(a0575a4164217e63317886176ab7e59d255fc771) )
	ROM_LOAD( "brkthru.11",   0x10000, 0x8000, CRC(c152a99b) SHA1(f96133aa01219eda357b9e906bd9577dbfe359c0) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "brkthru.13",   0x0000, 0x0100, CRC(aae44269) SHA1(7c66aeb93577104109d264ee8b848254256c81eb) ) /* red and green component */
	ROM_LOAD( "brkthru.14",   0x0100, 0x0100, CRC(f2d4822a) SHA1(f535e91b87ff01f2a73662856fd3f72907ca62e9) ) /* blue component */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "brkthru.5",    0x8000, 0x8000, CRC(c309435f) SHA1(82914004c2b169a7c31aa49af83a699ebbc7b33f) )
ROM_END

ROM_START( brkthruj )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for main CPU + 64k for banked ROMs */
	ROM_LOAD( "1",            0x04000, 0x4000, CRC(09bd60ee) SHA1(9591a4c89bb69d5615a5d6b29c47e6b17350c007) )
	ROM_LOAD( "2",            0x08000, 0x8000, CRC(f2b2cd1c) SHA1(dafccc74310876bc1c88de7f3c86f93ed8a0eb62) )
	ROM_LOAD( "4",            0x10000, 0x8000, CRC(b42b3359) SHA1(c1da550e0f7cc52721802c7c0f2770ef0087e28b) )
	ROM_LOAD( "brkthru.3",    0x18000, 0x8000, CRC(2f2c40c2) SHA1(fcb78941453520a3a07f272127dae7c2cc1999ea) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "12",   0x00000, 0x2000, CRC(3d9a7003) SHA1(2e5de982eb75ac75312fb29bb4cb2ed12ec0fd56) )   /* characters */

	ROM_REGION( 0x20000, "gfx2", 0 )
	/* background */
	/* we do a lot of scatter loading here, to place the data in a format */
	/* which can be decoded by MAME's standard functions */
	ROM_LOAD( "brkthru.7",    0x00000, 0x4000, CRC(920cc56a) SHA1(c75806691073f1f3bd54dcaca4c14155ecf4471d) )   /* bitplanes 1,2 for bank 1,2 */
	ROM_CONTINUE(             0x08000, 0x4000 )             /* bitplanes 1,2 for bank 3,4 */
	ROM_LOAD( "6",            0x10000, 0x4000, CRC(cb47b395) SHA1(bf5459d696e863644f13c8b0786b8f45caf6ceb6) )   /* bitplanes 1,2 for bank 5,6 */
	ROM_CONTINUE(             0x18000, 0x4000 )             /* bitplanes 1,2 for bank 7,8 */
	ROM_LOAD( "8",            0x04000, 0x1000, CRC(5e5a2cd7) SHA1(f1782d67b924b4b89bcb6602e970c28fbeaab522) )   /* bitplane 3 for bank 1,2 */
	ROM_CONTINUE(             0x06000, 0x1000 )
	ROM_CONTINUE(             0x0c000, 0x1000 )             /* bitplane 3 for bank 3,4 */
	ROM_CONTINUE(             0x0e000, 0x1000 )
	ROM_CONTINUE(             0x14000, 0x1000 )             /* bitplane 3 for bank 5,6 */
	ROM_CONTINUE(             0x16000, 0x1000 )
	ROM_CONTINUE(             0x1c000, 0x1000 )             /* bitplane 3 for bank 7,8 */
	ROM_CONTINUE(             0x1e000, 0x1000 )

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "brkthru.9",    0x00000, 0x8000, CRC(f54e50a7) SHA1(eccf4d859c26944271ec6586644b4730a72851fd) )   /* sprites */
	ROM_LOAD( "brkthru.10",   0x08000, 0x8000, CRC(fd156945) SHA1(a0575a4164217e63317886176ab7e59d255fc771) )
	ROM_LOAD( "brkthru.11",   0x10000, 0x8000, CRC(c152a99b) SHA1(f96133aa01219eda357b9e906bd9577dbfe359c0) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "brkthru.13",   0x0000, 0x0100, CRC(aae44269) SHA1(7c66aeb93577104109d264ee8b848254256c81eb) ) /* red and green component */
	ROM_LOAD( "brkthru.14",   0x0100, 0x0100, CRC(f2d4822a) SHA1(f535e91b87ff01f2a73662856fd3f72907ca62e9) ) /* blue component */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "brkthru.5",    0x8000, 0x8000, CRC(c309435f) SHA1(82914004c2b169a7c31aa49af83a699ebbc7b33f) )
ROM_END

/* bootleg, changed prg rom fails test, probably just the japanese version modified to have english title */
ROM_START( forcebrk )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for main CPU + 64k for banked ROMs */
	ROM_LOAD( "1",            0x04000, 0x4000, CRC(09bd60ee) SHA1(9591a4c89bb69d5615a5d6b29c47e6b17350c007) )
	ROM_LOAD( "2",            0x08000, 0x8000, CRC(f2b2cd1c) SHA1(dafccc74310876bc1c88de7f3c86f93ed8a0eb62) )
	ROM_LOAD( "forcebrk4",    0x10000, 0x8000, CRC(b4838c19) SHA1(b32f183ee042872a6eb6689aab219108d37829e4) )
	ROM_LOAD( "brkthru.3",    0x18000, 0x8000, CRC(2f2c40c2) SHA1(fcb78941453520a3a07f272127dae7c2cc1999ea) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "12",           0x00000, 0x2000, CRC(3d9a7003) SHA1(2e5de982eb75ac75312fb29bb4cb2ed12ec0fd56) )   /* characters */

	ROM_REGION( 0x20000, "gfx2", 0 )
	/* background */
	/* we do a lot of scatter loading here, to place the data in a format */
	/* which can be decoded by MAME's standard functions */
	ROM_LOAD( "brkthru.7",    0x00000, 0x4000, CRC(920cc56a) SHA1(c75806691073f1f3bd54dcaca4c14155ecf4471d) )   /* bitplanes 1,2 for bank 1,2 */
	ROM_CONTINUE(             0x08000, 0x4000 )             /* bitplanes 1,2 for bank 3,4 */
	ROM_LOAD( "forcebrk6",    0x10000, 0x4000, CRC(08bca16a) SHA1(d5dcf5cf68a5090f467c076abb1b9cf0baffe272) )   /* bitplanes 1,2 for bank 5,6 */
	ROM_CONTINUE(             0x18000, 0x4000 )             /* bitplanes 1,2 for bank 7,8 */
	ROM_LOAD( "forcebrk8",    0x04000, 0x1000, CRC(a3a1131e) SHA1(e0b73c8b2c8ea6b31418bc642830875c5985f800) )   /* bitplane 3 for bank 1,2 */
	ROM_CONTINUE(             0x06000, 0x1000 )
	ROM_CONTINUE(             0x0c000, 0x1000 )             /* bitplane 3 for bank 3,4 */
	ROM_CONTINUE(             0x0e000, 0x1000 )
	ROM_CONTINUE(             0x14000, 0x1000 )             /* bitplane 3 for bank 5,6 */
	ROM_CONTINUE(             0x16000, 0x1000 )
	ROM_CONTINUE(             0x1c000, 0x1000 )             /* bitplane 3 for bank 7,8 */
	ROM_CONTINUE(             0x1e000, 0x1000 )

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "brkthru.9",    0x00000, 0x8000, CRC(f54e50a7) SHA1(eccf4d859c26944271ec6586644b4730a72851fd) )   /* sprites */
	ROM_LOAD( "brkthru.10",   0x08000, 0x8000, CRC(fd156945) SHA1(a0575a4164217e63317886176ab7e59d255fc771) )
	ROM_LOAD( "brkthru.11",   0x10000, 0x8000, CRC(c152a99b) SHA1(f96133aa01219eda357b9e906bd9577dbfe359c0) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "brkthru.13",   0x0000, 0x0100, CRC(aae44269) SHA1(7c66aeb93577104109d264ee8b848254256c81eb) ) /* red and green component */
	ROM_LOAD( "brkthru.14",   0x0100, 0x0100, CRC(f2d4822a) SHA1(f535e91b87ff01f2a73662856fd3f72907ca62e9) ) /* blue component */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "brkthru.5",    0x8000, 0x8000, CRC(c309435f) SHA1(82914004c2b169a7c31aa49af83a699ebbc7b33f) )
ROM_END

ROM_START( darwin )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for main CPU + 64k for banked ROMs */
	ROM_LOAD( "darw_04.rom",  0x04000, 0x4000, CRC(0eabf21c) SHA1(ccad6b30fe9361e8a21b8aaf8116aa85f9e6bb19) )
	ROM_LOAD( "darw_05.rom",  0x08000, 0x8000, CRC(e771f864) SHA1(8ba9f97c6abf035ceaf9f5505495708506f1b0c5) )
	ROM_LOAD( "darw_07.rom",  0x10000, 0x8000, CRC(97ac052c) SHA1(8baa117472d46b99e5946f095b869de9b5c48f9a) )
	ROM_LOAD( "darw_06.rom",  0x18000, 0x8000, CRC(2a9fb208) SHA1(f04a5502600e49e2494a87ec65a44a2843441d37) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "darw_09.rom",  0x00000, 0x2000, CRC(067b4cf5) SHA1(fc752bb72e4850b71565afd1df0cbb4f732f131c) )   /* characters */

	ROM_REGION( 0x20000, "gfx2", 0 )
	/* background */
	/* we do a lot of scatter loading here, to place the data in a format */
	/* which can be decoded by MAME's standard functions */
	ROM_LOAD( "darw_03.rom",  0x00000, 0x4000, CRC(57d0350d) SHA1(6f904047485e669afb5f4b590818743111f010c6) )   /* bitplanes 1,2 for bank 1,2 */
	ROM_CONTINUE(             0x08000, 0x4000 )             /* bitplanes 1,2 for bank 3,4 */
	ROM_LOAD( "darw_02.rom",  0x10000, 0x4000, CRC(559a71ab) SHA1(a28de25e89e0d68332f4095b988827a9cb72c675) )   /* bitplanes 1,2 for bank 5,6 */
	ROM_CONTINUE(             0x18000, 0x4000 )             /* bitplanes 1,2 for bank 7,8 */
	ROM_LOAD( "darw_01.rom",  0x04000, 0x1000, CRC(15a16973) SHA1(5eb978a32be88176936e5d37b6ec18820d9720d8) )   /* bitplane 3 for bank 1,2 */
	ROM_CONTINUE(             0x06000, 0x1000 )
	ROM_CONTINUE(             0x0c000, 0x1000 )             /* bitplane 3 for bank 3,4 */
	ROM_CONTINUE(             0x0e000, 0x1000 )
	ROM_CONTINUE(             0x14000, 0x1000 )             /* bitplane 3 for bank 5,6 */
	ROM_CONTINUE(             0x16000, 0x1000 )
	ROM_CONTINUE(             0x1c000, 0x1000 )             /* bitplane 3 for bank 7,8 */
	ROM_CONTINUE(             0x1e000, 0x1000 )

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "darw_10.rom",  0x00000, 0x8000, CRC(487a014c) SHA1(c9543df8115088b02019e76a6473ecc5f645a836) )   /* sprites */
	ROM_LOAD( "darw_11.rom",  0x08000, 0x8000, CRC(548ce2d1) SHA1(3b1757c70346ab4ee19ec85e7ae5137f8ccf446f) )
	ROM_LOAD( "darw_12.rom",  0x10000, 0x8000, CRC(faba5fef) SHA1(848da4d4888f0218b737f1dc9b62944f68349a43) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "df.12",   0x0000, 0x0100, CRC(89b952ef) SHA1(77dc4020a2e25f81fae1182d58993cf09d13af00) ) /* red and green component */
	ROM_LOAD( "df.13",   0x0100, 0x0100, CRC(d595e91d) SHA1(5e9793f6602455c79afdc855cd13183a7f48ab1e) ) /* blue component */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "darw_08.rom",  0x8000, 0x8000, CRC(6b580d58) SHA1(a70aebc6b4a291b4adddbb41d092b2682fc2d421) )
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(brkthru_state,brkthru)
{
	UINT8 *ROM = memregion("maincpu")->base();
	membank("bank1")->configure_entries(0, 8, &ROM[0x10000], 0x2000);
}

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1986, brkthru,  0,       brkthru, brkthru, brkthru_state,  brkthru, ROT0,   "Data East USA",         "Break Thru (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, brkthruj, brkthru, brkthru, brkthruj, brkthru_state, brkthru, ROT0,   "Data East Corporation", "Kyohkoh-Toppa (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, forcebrk, brkthru, brkthru, brkthruj, brkthru_state, brkthru, ROT0,   "bootleg",               "Force Break (bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, darwin,   0,       darwin,  darwin, brkthru_state,   brkthru, ROT270, "Data East Corporation", "Darwin 4078 (Japan)", MACHINE_SUPPORTS_SAVE )

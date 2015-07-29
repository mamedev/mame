// license:BSD-3-Clause
// copyright-holders:Carlos A. Lozano
/***************************************************************************

Cops 01      (c) 1985 Nichibutsu
Mighty Guy   (c) 1986 Nichibutsu

driver by Carlos A. Lozano <calb@gsyc.inf.uc3m.es>

TODO:
----
- Fix priority kludge (see video/cop01.c)
mightguy:
- missing emulation of the 1412M2 protection chip, used by the sound CPU.
  This is probably an extra CPU (program rom is the ic2 one), presumably
  with data / address line scrambling


Mighty Guy board layout:
-----------------------

  cpu

12MHz     SW1
          SW2                 clr.13D clr.14D clr.15D      clr.19D

      Nichibutsu
      NBB 60-06                        4     5

  1 2 3  6116 6116                    6116   6116

-------

 video

 6116   11  Nichibutsu
            NBA 60-07    13B                               20MHz
                                2148                2148
                                2148                2148

              6116             9                        8  2E
 20G          10               7                        6
 -------

 audio sub-board MT-S3
 plugs into 40 pin socket at 20G

                     10.IC2

                     Nichibutsu
                     1412M2 (Yamaha 3810?)
                               8MHz
                     YM3526

***************************************************************************/
#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/3526intf.h"
#include "includes/cop01.h"


#define MIGHTGUY_HACK    0
#define TIMER_RATE       11475  /* unknown, hand-tuned to match audio reference */

#define MAINCPU_CLOCK    XTAL_12MHz
#define AUDIOCPU_CLOCK   XTAL_8MHz

/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_MEMBER(cop01_state::cop01_sound_command_w)
{
	soundlatch_byte_w(space, offset, data);
	m_audiocpu->set_input_line(0, ASSERT_LINE );
}

READ8_MEMBER(cop01_state::cop01_sound_command_r)
{
	int res = (soundlatch_byte_r(space, offset) & 0x7f) << 1;

	/* bit 0 seems to be a timer */
	if ((m_audiocpu->total_cycles() / TIMER_RATE) & 1)
	{
		if (m_pulse == 0)
			res |= 1;

		m_pulse = 1;
	}
	else
		m_pulse = 0;

	return res;
}


CUSTOM_INPUT_MEMBER(cop01_state::mightguy_area_r)
{
	int bit_mask = (FPTR)param;
	return (ioport("FAKE")->read() & bit_mask) ? 0x01 : 0x00;
}

WRITE8_MEMBER(cop01_state::cop01_irq_ack_w)
{
	m_maincpu->set_input_line(0, CLEAR_LINE );
}

READ8_MEMBER(cop01_state::cop01_sound_irq_ack_w)
{
	m_audiocpu->set_input_line(0, CLEAR_LINE );
	return 0;
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( cop01_map, AS_PROGRAM, 8, cop01_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xcfff) AM_RAM /* c000-c7ff in cop01 */
	AM_RANGE(0xd000, 0xdfff) AM_RAM_WRITE(cop01_background_w) AM_SHARE("bgvideoram")
	AM_RANGE(0xe000, 0xe0ff) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0xf000, 0xf3ff) AM_WRITE(cop01_foreground_w) AM_SHARE("fgvideoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, cop01_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("P1")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("P2")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("DSW1")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("DSW2")
	AM_RANGE(0x40, 0x43) AM_WRITE(cop01_vreg_w)
	AM_RANGE(0x44, 0x44) AM_WRITE(cop01_sound_command_w)
	AM_RANGE(0x45, 0x45) AM_WRITE(cop01_irq_ack_w) /* ? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( mightguy_io_map, AS_IO, 8, cop01_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("P1")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("P2")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("DSW1")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("DSW2")
	AM_RANGE(0x40, 0x43) AM_WRITE(cop01_vreg_w)
	AM_RANGE(0x44, 0x44) AM_WRITE(cop01_sound_command_w)
	AM_RANGE(0x45, 0x45) AM_WRITE(cop01_irq_ack_w) /* ? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, cop01_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8000) AM_READ(cop01_sound_irq_ack_w)
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( audio_io_map, AS_IO, 8, cop01_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0x02, 0x03) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
	AM_RANGE(0x04, 0x05) AM_DEVWRITE("ay3", ay8910_device, address_data_w)
	AM_RANGE(0x06, 0x06) AM_READ(cop01_sound_command_r)
ADDRESS_MAP_END


/* this just gets some garbage out of the YM3526 */
READ8_MEMBER(cop01_state::kludge)
{
	return m_timer++;
}

static ADDRESS_MAP_START( mightguy_audio_io_map, AS_IO, 8, cop01_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("ymsnd", ym3526_device, write)
	AM_RANGE(0x02, 0x02) AM_WRITENOP    /* 1412M2? */
	AM_RANGE(0x03, 0x03) AM_WRITENOP    /* 1412M2? */
	AM_RANGE(0x03, 0x03) AM_READ(kludge)    /* 1412M2? */
	AM_RANGE(0x06, 0x06) AM_READ(cop01_sound_command_r)
ADDRESS_MAP_END


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( cop01 )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")    /* TEST, COIN, START */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_6C ) )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	/* DP2:3,4,5 defined in manual/test-mode as:
	PORT_DIPNAME( 0x10, 0x10, "1st Bonus Life" )
	PORT_DIPSETTING(    0x10, "20000" )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPNAME( 0x60, 0x60, "2nd Bonus Life" )
	PORT_DIPSETTING(    0x60, "30000" )
	PORT_DIPSETTING(    0x20, "50000" )
	PORT_DIPSETTING(    0x40, "100000" )
	PORT_DIPSETTING(    0x00, "150000" ) */
	PORT_DIPNAME( 0x70, 0x70, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x70, "20k 50k 30k+" )
	PORT_DIPSETTING(    0x30, "20k 70k 50k+" )
	PORT_DIPSETTING(    0x50, "20k 120k 100k+" )
	PORT_DIPSETTING(    0x10, "20k 170k 150k+" )
	PORT_DIPSETTING(    0x60, "30k 60k 30k+" )
	PORT_DIPSETTING(    0x20, "30k 80k 50k+" )
	PORT_DIPSETTING(    0x40, "30k 130k 100k+" )
	PORT_DIPSETTING(    0x00, "30k 180k 150k+" )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END

/* There is an ingame bug at 0x00e4 to 0x00e6 that performs 3 times 'rrca' instead of 'rlca'
   so DSW1-8 has no effect and you can NOT start a game at areas 5 to 8. */
static INPUT_PORTS_START( mightguy )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )            /* same as the service dip switch */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x04, "Every 200k" )
	PORT_DIPSETTING(    0x00, "500k only" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, cop01_state,mightguy_area_r, (void *)0x04)    // "Start Area" - see fake Dip Switch

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, "Invincibility")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, cop01_state,mightguy_area_r, (void *)0x01)    // "Start Area" - see fake Dip Switch
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, cop01_state,mightguy_area_r, (void *)0x02)    // "Start Area" - see fake Dip Switch

	PORT_START("FAKE")  /* FAKE Dip Switch */
	PORT_DIPNAME( 0x07, 0x07, "Starting Area" )
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	/* Not working due to ingame bug (see above) */
#if MIGHTGUY_HACK
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8" )
#endif
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout tilelayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 4+8*0, 0+8*0, 4+8*1, 0+8*1, 4+8*2, 0+8*2, 4+8*3, 0+8*3 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 0, 1, 2, 3 },
	{
		RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0,   4, 0,
		RGN_FRAC(1,2)+12, RGN_FRAC(1,2)+8,  12, 8,
		RGN_FRAC(1,2)+20, RGN_FRAC(1,2)+16, 20, 16,
		RGN_FRAC(1,2)+28, RGN_FRAC(1,2)+24, 28, 24,
	},
	{
		0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32
	},
	64*8
};

static GFXDECODE_START( cop01 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,         0,  1 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,        16,  8 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout, 16+8*16, 16 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void cop01_state::machine_start()
{
	save_item(NAME(m_pulse));
	save_item(NAME(m_timer));
	save_item(NAME(m_vreg));
}

void cop01_state::machine_reset()
{
	m_pulse = 0;
	m_timer = 0;
	m_vreg[0] = 0;
	m_vreg[1] = 0;
	m_vreg[2] = 0;
	m_vreg[3] = 0;
}


static MACHINE_CONFIG_START( cop01, cop01_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MAINCPU_CLOCK/2)   /* unknown clock / divider */
	MCFG_CPU_PROGRAM_MAP(cop01_map)
	MCFG_CPU_IO_MAP(io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", cop01_state,  irq0_line_assert)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_3MHz)    /* unknown clock / divider, hand-tuned to match audio reference */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(audio_io_map)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(cop01_state, screen_update_cop01)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", cop01)
	MCFG_PALETTE_ADD("palette", 16+8*16+16*16)
	MCFG_PALETTE_INDIRECT_ENTRIES(256)
	MCFG_PALETTE_INIT_OWNER(cop01_state, cop01)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 1250000) /* unknown clock / divider, hand-tuned to match audio reference */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("ay2", AY8910, 1250000) /* unknown clock / divider, hand-tuned to match audio reference */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay3", AY8910, 1250000) /* unknown clock / divider, hand-tuned to match audio reference */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( mightguy, cop01_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MAINCPU_CLOCK/2)   /* unknown divider */
	MCFG_CPU_PROGRAM_MAP(cop01_map)
	MCFG_CPU_IO_MAP(mightguy_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", cop01_state,  irq0_line_assert)

	MCFG_CPU_ADD("audiocpu", Z80, AUDIOCPU_CLOCK/2) /* unknown divider */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(mightguy_audio_io_map)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(cop01_state, screen_update_cop01)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", cop01)
	MCFG_PALETTE_ADD("palette", 16+8*16+16*16)
	MCFG_PALETTE_INDIRECT_ENTRIES(256)
	MCFG_PALETTE_INIT_OWNER(cop01_state, cop01)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3526, AUDIOCPU_CLOCK/2) /* unknown divider */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( cop01 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cop01.2b",     0x0000, 0x4000, CRC(5c2734ab) SHA1(dd6724dfb1c58e6ce3c1c99cad8732a0f5c9b773) )
	ROM_LOAD( "cop02.4b",     0x4000, 0x4000, CRC(9c7336ef) SHA1(2aa58aea19dafb53190d9bef7b3aa9c3004522f0) )
	ROM_LOAD( "cop03.5b",     0x8000, 0x4000, CRC(2566c8bf) SHA1(c9d98afd1f02a208b1af1d418e69e88f8703afa5) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "cop15.17b",    0x0000, 0x4000, CRC(6a5f08fa) SHA1(8838549502b1a6ac72dd5efd58e0968f8abe338a) )
	ROM_LOAD( "cop16.18b",    0x4000, 0x4000, CRC(56bf6946) SHA1(5414d00c6de96cfb5a3e68c35376333df6c525ee) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "cop14.15g",    0x00000, 0x2000, CRC(066d1c55) SHA1(017a0576799d39b919e6ca9b4a7f106ed04c0f94) )   /* chars */

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "cop04.15c",    0x00000, 0x4000, CRC(622d32e6) SHA1(982b585e9a1115bce25c1788999d34423ccb83ab) )   /* tiles */
	ROM_LOAD( "cop05.16c",    0x04000, 0x4000, CRC(c6ac5a35) SHA1(dab3500981663ee19abac5bfeaaf6a07a3953d75) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "cop06.3g",     0x00000, 0x2000, CRC(f1c1f4a5) SHA1(139aa23416e71361fe62ce336e3f0529a21acb81) )   /* sprites */
	ROM_LOAD( "cop07.5g",     0x02000, 0x2000, CRC(11db7b72) SHA1(47a1223ed3e7b294d7e59c05d119488ef6b3dc7a) )
	ROM_LOAD( "cop08.6g",     0x04000, 0x2000, CRC(a63ddda6) SHA1(59aaa1fe0c023c4f0d4cfbdb9ca922182201c145) )
	ROM_LOAD( "cop09.8g",     0x06000, 0x2000, CRC(855a2ec3) SHA1(8a54c0ceedeeafd7c1a6a35b4efab28046967951) )
	ROM_LOAD( "cop10.3e",     0x08000, 0x2000, CRC(444cb19d) SHA1(e74118b027db65ba06291bc0fe0ff50bcacc32c2) )
	ROM_LOAD( "cop11.5e",     0x0a000, 0x2000, CRC(9078bc04) SHA1(3d8614415027f5db9ddb77b89656e4c7fc9d28de) )
	ROM_LOAD( "cop12.6e",     0x0c000, 0x2000, CRC(257a6706) SHA1(9e7ef1f40630b94849bdc3fd13ee6e7311fffd45) )
	ROM_LOAD( "cop13.8e",     0x0e000, 0x2000, CRC(07c4ea66) SHA1(12665c0fb648fd208805e81d056ab377d65b267a) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "copproma.13d", 0x0000, 0x0100, CRC(97f68a7a) SHA1(010eaca95eeb5caec083bd184ec31e0f433fff8c) )    /* red */
	ROM_LOAD( "coppromb.14d", 0x0100, 0x0100, CRC(39a40b4c) SHA1(456b7f97fbd1cb4beb756033ec76a89ffe8de168) )    /* green */
	ROM_LOAD( "coppromc.15d", 0x0200, 0x0100, CRC(8181748b) SHA1(0098ae250095b4ac8af1811b4e41d86e3f587c7b) )    /* blue */
	ROM_LOAD( "coppromd.19d", 0x0300, 0x0100, CRC(6a63dbb8) SHA1(50f971f173147203cd24dc4fa7f0a27d2179f1cc) )    /* tile lookup table */
	ROM_LOAD( "copprome.2e",  0x0400, 0x0100, CRC(214392fa) SHA1(59d235c3e584e7fd484edf5c78c43d2597c1c3a8) )    /* sprite lookup table */
	ROM_LOAD( "13b",          0x0500, 0x0100, NO_DUMP ) /* state machine data used for video signals generation (not used in emulation)*/
ROM_END

ROM_START( cop01a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cop01alt.001", 0x0000, 0x4000, CRC(a13ee0d3) SHA1(2f28f901bdc041c79f785821d0052823654983a2) )
	ROM_LOAD( "cop01alt.002", 0x4000, 0x4000, CRC(20bad28e) SHA1(79155880ae1c9e8d19390c163cac31093ee11604) )
	ROM_LOAD( "cop01alt.003", 0x8000, 0x4000, CRC(a7e10b79) SHA1(ec7e4153a211d070c2dc09ab98a59f61ab10ea78) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "cop01alt.015", 0x0000, 0x4000, CRC(95be9270) SHA1(ffb4786e354c4c6ddce134ae3362da660199fd44) )
	ROM_LOAD( "cop01alt.016", 0x4000, 0x4000, CRC(c20bf649) SHA1(a719ad6bf35811957ad32e6f07494bb00f256965) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "cop01alt.014", 0x00000, 0x2000, CRC(edd8a474) SHA1(42f0655535f1e10840da383129da69627d67ff8a) )   /* chars */

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "cop04.15c",    0x00000, 0x4000, CRC(622d32e6) SHA1(982b585e9a1115bce25c1788999d34423ccb83ab) )   /* tiles */
	ROM_LOAD( "cop05.16c",    0x04000, 0x4000, CRC(c6ac5a35) SHA1(dab3500981663ee19abac5bfeaaf6a07a3953d75) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "cop01alt.006", 0x00000, 0x2000, CRC(cac7dac8) SHA1(25990ac4614de2ae61d663323bd67acc137bbc4a) )   /* sprites */
	ROM_LOAD( "cop07.5g",     0x02000, 0x2000, CRC(11db7b72) SHA1(47a1223ed3e7b294d7e59c05d119488ef6b3dc7a) )
	ROM_LOAD( "cop08.6g",     0x04000, 0x2000, CRC(a63ddda6) SHA1(59aaa1fe0c023c4f0d4cfbdb9ca922182201c145) )
	ROM_LOAD( "cop09.8g",     0x06000, 0x2000, CRC(855a2ec3) SHA1(8a54c0ceedeeafd7c1a6a35b4efab28046967951) )
	ROM_LOAD( "cop01alt.010", 0x08000, 0x2000, CRC(94aee9d6) SHA1(dd6f27dcee761c84447b8326bfa0532b7d708721) )
	ROM_LOAD( "cop11.5e",     0x0a000, 0x2000, CRC(9078bc04) SHA1(3d8614415027f5db9ddb77b89656e4c7fc9d28de) )
	ROM_LOAD( "cop12.6e",     0x0c000, 0x2000, CRC(257a6706) SHA1(9e7ef1f40630b94849bdc3fd13ee6e7311fffd45) )
	ROM_LOAD( "cop13.8e",     0x0e000, 0x2000, CRC(07c4ea66) SHA1(12665c0fb648fd208805e81d056ab377d65b267a) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "copproma.13d", 0x0000, 0x0100, CRC(97f68a7a) SHA1(010eaca95eeb5caec083bd184ec31e0f433fff8c) )    /* red */
	ROM_LOAD( "coppromb.14d", 0x0100, 0x0100, CRC(39a40b4c) SHA1(456b7f97fbd1cb4beb756033ec76a89ffe8de168) )    /* green */
	ROM_LOAD( "coppromc.15d", 0x0200, 0x0100, CRC(8181748b) SHA1(0098ae250095b4ac8af1811b4e41d86e3f587c7b) )    /* blue */
	ROM_LOAD( "coppromd.19d", 0x0300, 0x0100, CRC(6a63dbb8) SHA1(50f971f173147203cd24dc4fa7f0a27d2179f1cc) )    /* tile lookup table */
	ROM_LOAD( "copprome.2e",  0x0400, 0x0100, CRC(214392fa) SHA1(59d235c3e584e7fd484edf5c78c43d2597c1c3a8) )    /* sprite lookup table */
	ROM_LOAD( "13b",          0x0500, 0x0100, NO_DUMP ) /* state machine data used for video signals generation (not used in emulation)*/
ROM_END

ROM_START( mightguy )
	ROM_REGION( 0x60000, "maincpu", 0 ) /* Z80 code (main cpu) */
	ROM_LOAD( "1.2b",       0x0000, 0x4000,CRC(bc8e4557) SHA1(4304ac1a0e11bad254ad937195f0be6e7186577d) )
	ROM_LOAD( "2.4b",       0x4000, 0x4000,CRC(fb73d684) SHA1(d8a4b6fb93b2c3710fc66f92df05c1459e4171c3) )
	ROM_LOAD( "3.5b",       0x8000, 0x4000,CRC(b14b6ab8) SHA1(a60059dd54c8cc974334fd879ff0cfd436a7a981) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code (sound cpu) */
	ROM_LOAD( "11.15b",     0x0000, 0x4000, CRC(576183ea) SHA1(e3f28e8e8c34ab396d158da122584ed226729c99) )

	ROM_REGION( 0x8000, "user1", 0 ) /* 1412M2 protection data, z80 encrypted code presumably */
	ROM_LOAD( "10.ic2",     0x0000, 0x8000, CRC(1a5d2bb1) SHA1(0fd4636133a980ba9ffa076f9010474586d37635) )

	ROM_REGION( 0x02000, "gfx1", 0 ) /* alpha */
	ROM_LOAD( "10.15g",     0x0000, 0x2000, CRC(17874300) SHA1(f97bee0ab491b04fe4950ebe9587031db6c815a3) )

	ROM_REGION( 0x08000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "4.15c",      0x0000, 0x4000,CRC(84d29e76) SHA1(98e6c6e4a95471c5bef9fcb85a663b86eeda6b6d) )
	ROM_LOAD( "5.16c",      0x4000, 0x4000,CRC(f7bb8d82) SHA1(6ab6585827482fd23c3be129977a4443623d831c) )

	ROM_REGION( 0x14000, "gfx3", 0 ) /* sprites */
	ROM_LOAD( "6.3g",       0x00000, 0x2000, CRC(6ff88615) SHA1(8bfeab97bd1a14861e3a7538c0dd3c073adf29aa) )
	ROM_LOAD( "7.8g",       0x02000, 0x8000, CRC(e79ea66f) SHA1(2db80eef5375294bf9c7819f4090ec71f7f2be25) )
	ROM_LOAD( "8.3e",       0x0a000, 0x2000, CRC(29f6eb44) SHA1(d7957c8579d7d32c52c19d2fe7b90d1c890f29ea) )
	ROM_LOAD( "9.8e",       0x0c000, 0x8000, CRC(b9f64c6f) SHA1(82ec6ba689f16fed1141cd32640a8b1f1ab14bdd) )

	ROM_REGION( 0x600, "proms", 0 )
	ROM_LOAD( "clr.13d",    0x000, 0x100, CRC(c4cf0bdd) SHA1(350842c46a71fb5db43c8823662378f178bbda4f) ) /* red */
	ROM_LOAD( "clr.14d",    0x100, 0x100, CRC(5b3b8a9b) SHA1(6b660f5f7b0efdc20a79a9fd5a1eb30c85b27324) ) /* green */
	ROM_LOAD( "clr.15d",    0x200, 0x100, CRC(6c072a64) SHA1(5ce5306af478330eb3e94aa7c8bff08f34ba6ea5) ) /* blue */
	ROM_LOAD( "clr.19d",    0x300, 0x100, CRC(19b66ac6) SHA1(5e7de11f40685effa077377e7a55d7fecf752508) ) /* tile lookup table */
	ROM_LOAD( "2e",         0x400, 0x100, CRC(d9c45126) SHA1(aafebe424afa400ed320f17afc2b910eaada29f5) ) /* sprite lookup table */
	ROM_LOAD( "13b",        0x500, 0x100, CRC(4a6f9a6d) SHA1(65f1e0bfacd1f354ece1b18598a551044c27c4d1) ) /* state machine data used for video signals generation (not used in emulation)*/
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(cop01_state,mightguy)
{
#if MIGHTGUY_HACK
	/* This is a hack to fix the game code to get a fully working
	   "Starting Area" fake Dip Switch */
	UINT8 *RAM = (UINT8 *)memregion("maincpu")->base();
	RAM[0x00e4] = 0x07; // rlca
	RAM[0x00e5] = 0x07; // rlca
	RAM[0x00e6] = 0x07; // rlca
	/* To avoid checksum error */
	RAM[0x027f] = 0x00;
	RAM[0x0280] = 0x00;
#endif
}


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1985, cop01,    0,     cop01,    cop01, driver_device,    0,        ROT0,   "Nichibutsu", "Cop 01 (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, cop01a,   cop01, cop01,    cop01, driver_device,    0,        ROT0,   "Nichibutsu", "Cop 01 (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, mightguy, 0,     mightguy, mightguy, cop01_state, mightguy, ROT270, "Nichibutsu", "Mighty Guy", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )

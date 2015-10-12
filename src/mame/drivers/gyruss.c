// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Couriersud
/***************************************************************************

Gyruss memory map (preliminary)

Main processor memory map.
0000-5fff ROM (6000-7fff diagnostics)
8000-83ff Color RAM
8400-87ff Video RAM
9000-a7ff RAM
a000-a17f \ sprites
a200-a27f /

memory mapped ports:

read:
c080      IN0  (system inputs)
c0a0      IN1
c0c0      IN2
c0e0      DSW1
c000      DSW2
c100      DSW3

write:
a000-a1ff  Odd frame spriteram
a200-a3ff  Even frame spriteram
a700       Frame odd or even?
a701       Semaphore system:  tells 6809 to draw queued sprites
a702       Semaphore system:  tells 6809 to queue sprites
c000       watchdog reset
c080       trigger interrupt on audio CPU
c100       command for the audio CPU
c180       interrupt enable
c185       flip screen

interrupts:
standard NMI at 0x66


SOUND BOARD:
0000-3fff  Audio ROM (4000-5fff diagnostics)
6000-63ff  Audio RAM
8000       Read Sound Command

I/O:

Gyruss has 5 PSGs:
1)  Control: 0x00    Read: 0x01    Write: 0x02
2)  Control: 0x04    Read: 0x05    Write: 0x06
3)  Control: 0x08    Read: 0x09    Write: 0x0a
4)  Control: 0x0c    Read: 0x0d    Write: 0x0e
5)  Control: 0x10    Read: 0x11    Write: 0x12

and 1 SFX channel controlled by an 8039:
1)  SoundOn: 0x14    SoundData: 0x18

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6809/m6809.h"
#include "machine/konami1.h"
#include "cpu/mcs48/mcs48.h"
#include "sound/ay8910.h"
#include "sound/discrete.h"
#include "includes/konamipt.h"
#include "includes/gyruss.h"


#define MASTER_CLOCK    XTAL_18_432MHz
#define SOUND_CLOCK     XTAL_14_31818MHz

// Video timing
// PCB measured: H = 15.50khz V = 60.56hz, +/- 0.01hz
// --> VTOTAL should be OK, HTOTAL not 100% certain
#define PIXEL_CLOCK     MASTER_CLOCK/3

#define HTOTAL          396
#define HBEND           0
#define HBSTART         256

#define VTOTAL          256
#define VBEND           0+2*8
#define VBSTART         224+2*8


/* The timer clock which feeds the upper 4 bits of                      */
/* AY-3-8910 port A is based on the same clock                          */
/* feeding the sound CPU Z80.  It is a divide by                        */
/* 10240, formed by a standard divide by 1024,                          */
/* followed by a divide by 10 using a 4 bit                             */
/* bi-quinary count sequence. (See LS90 data sheet                      */
/* for an example).                                                     */
/*                                                                      */
/* Bit 0 comes from the output of the divide by 1024                    */
/*       0, 1, 0, 1, 0, 1, 0, 1, 0, 1                                   */
/* Bit 1 comes from the QC output of the LS90 producing a sequence of   */
/*       0, 0, 1, 1, 0, 0, 1, 1, 1, 0                                   */
/* Bit 2 comes from the QD output of the LS90 producing a sequence of   */
/*       0, 0, 0, 0, 1, 0, 0, 0, 0, 1                                   */
/* Bit 3 comes from the QA output of the LS90 producing a sequence of   */
/*       0, 0, 0, 0, 0, 1, 1, 1, 1, 1                                   */

static const int gyruss_timer[10] =
{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x09, 0x0a, 0x0b, 0x0a, 0x0d
};

READ8_MEMBER(gyruss_state::gyruss_portA_r)
{
	return gyruss_timer[(m_audiocpu->total_cycles() / 1024) % 10];
}


WRITE8_MEMBER(gyruss_state::gyruss_dac_w)
{
	m_discrete->write(space, NODE(16), data);
}

WRITE8_MEMBER(gyruss_state::gyruss_irq_clear_w)
{
	m_audiocpu_2->set_input_line(0, CLEAR_LINE);
}

void gyruss_state::filter_w(address_space &space, int chip, int data )
{
	//printf("chip %d - %02x\n", chip, data);
	for (int i = 0; i < 3; i++)
	{
		/* low bit: 47000pF = 0.047uF */
		/* high bit: 220000pF = 0.22uF */
		m_discrete->write(space, NODE(3 * chip + i + 21), data & 3);
		data >>= 2;
	}
}

WRITE8_MEMBER(gyruss_state::gyruss_filter0_w)
{
	filter_w(space, 0, data);
}

WRITE8_MEMBER(gyruss_state::gyruss_filter1_w)
{
	filter_w(space, 1, data);
}


WRITE8_MEMBER(gyruss_state::gyruss_sh_irqtrigger_w)
{
	/* writing to this register triggers IRQ on the sound CPU */
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff);
}

WRITE8_MEMBER(gyruss_state::gyruss_i8039_irq_w)
{
	m_audiocpu_2->set_input_line(0, ASSERT_LINE);
}

WRITE8_MEMBER(gyruss_state::master_nmi_mask_w)
{
	m_master_nmi_mask = data & 1;
	if (!m_master_nmi_mask)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

WRITE8_MEMBER(gyruss_state::slave_irq_mask_w)
{
	m_slave_irq_mask = data & 1;
	if (!m_slave_irq_mask)
		m_subcpu->set_input_line(0, CLEAR_LINE);
}

static ADDRESS_MAP_START( main_cpu1_map, AS_PROGRAM, 8, gyruss_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM AM_SHARE("colorram")
	AM_RANGE(0x8400, 0x87ff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x9000, 0x9fff) AM_RAM
	AM_RANGE(0xa000, 0xa7ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xc000, 0xc000) AM_READ_PORT("DSW2") AM_WRITENOP   /* watchdog reset */
	AM_RANGE(0xc080, 0xc080) AM_READ_PORT("SYSTEM") AM_WRITE(gyruss_sh_irqtrigger_w)
	AM_RANGE(0xc0a0, 0xc0a0) AM_READ_PORT("P1")
	AM_RANGE(0xc0c0, 0xc0c0) AM_READ_PORT("P2")
	AM_RANGE(0xc0e0, 0xc0e0) AM_READ_PORT("DSW1")
	AM_RANGE(0xc100, 0xc100) AM_READ_PORT("DSW3") AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0xc180, 0xc180) AM_WRITE(master_nmi_mask_w)
	AM_RANGE(0xc185, 0xc185) AM_WRITEONLY AM_SHARE("flipscreen")
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_cpu2_map, AS_PROGRAM, 8, gyruss_state )
	AM_RANGE(0x0000, 0x0000) AM_READ(gyruss_scanline_r)
	AM_RANGE(0x2000, 0x2000) AM_WRITE(slave_irq_mask_w) AM_READNOP
	AM_RANGE(0x4000, 0x403f) AM_RAM
	AM_RANGE(0x4040, 0x40ff) AM_RAM_WRITE(gyruss_spriteram_w) AM_SHARE("spriteram")
	AM_RANGE(0x4100, 0x47ff) AM_RAM
	AM_RANGE(0x6000, 0x67ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( audio_cpu1_map, AS_PROGRAM, 8, gyruss_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x63ff) AM_RAM
	AM_RANGE(0x8000, 0x8000) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( audio_cpu1_io_map, AS_IO, 8, gyruss_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVWRITE("ay1", ay8910_device, address_w)
	AM_RANGE(0x01, 0x01) AM_DEVREAD("ay1", ay8910_device, data_r)
	AM_RANGE(0x02, 0x02) AM_DEVWRITE("ay1", ay8910_device, data_w)
	AM_RANGE(0x04, 0x04) AM_DEVWRITE("ay2", ay8910_device, address_w)
	AM_RANGE(0x05, 0x05) AM_DEVREAD("ay2", ay8910_device, data_r)
	AM_RANGE(0x06, 0x06) AM_DEVWRITE("ay2", ay8910_device, data_w)
	AM_RANGE(0x08, 0x08) AM_DEVWRITE("ay3", ay8910_device, address_w)
	AM_RANGE(0x09, 0x09) AM_DEVREAD("ay3", ay8910_device, data_r)
	AM_RANGE(0x0a, 0x0a) AM_DEVWRITE("ay3", ay8910_device, data_w)
	AM_RANGE(0x0c, 0x0c) AM_DEVWRITE("ay4", ay8910_device, address_w)
	AM_RANGE(0x0d, 0x0d) AM_DEVREAD("ay4", ay8910_device, data_r)
	AM_RANGE(0x0e, 0x0e) AM_DEVWRITE("ay4", ay8910_device, data_w)
	AM_RANGE(0x10, 0x10) AM_DEVWRITE("ay5", ay8910_device, address_w)
	AM_RANGE(0x11, 0x11) AM_DEVREAD("ay5", ay8910_device, data_r)
	AM_RANGE(0x12, 0x12) AM_DEVWRITE("ay5", ay8910_device, data_w)
	AM_RANGE(0x14, 0x14) AM_WRITE(gyruss_i8039_irq_w)
	AM_RANGE(0x18, 0x18) AM_WRITE(soundlatch2_byte_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( audio_cpu2_map, AS_PROGRAM, 8, gyruss_state )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( audio_cpu2_io_map, AS_IO, 8, gyruss_state )
	AM_RANGE(0x00, 0xff) AM_READ(soundlatch2_byte_r)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_WRITE(gyruss_dac_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_WRITE(gyruss_irq_clear_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( gyruss )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* 1p shoot 2 - unused */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* 2p shoot 2 - unused */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), DEF_STR( Free_Play ), SW1)

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "255 (Cheat)")
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:4")     /* tables at 0x1653 (15 bytes) or 0x4bf3 (13 bytes) */
	PORT_DIPSETTING(    0x08, "30k 90k 60k+" )              /* last bonus life at 810k : max. 14 bonus lives */
	PORT_DIPSETTING(    0x00, "40k 110k 70k+" )             /* last bonus life at 810k : max. 12 bonus lives */
	PORT_DIPNAME( 0x70, 0x30, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:5,6,7")
	PORT_DIPSETTING(    0x70, "1 (Easiest)" )
	PORT_DIPSETTING(    0x60, "2" )
	PORT_DIPSETTING(    0x50, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x30, "5 (Average)" )
	PORT_DIPSETTING(    0x20, "6" )
	PORT_DIPSETTING(    0x10, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hardest)" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, "Demo Music" )                PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( gyrussce )
	PORT_INCLUDE( gyruss )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW2:3")     /* tables at 0x1653 (15 bytes) or 0x4bf3 (13 bytes) */
	PORT_DIPSETTING(    0x08, "50k 120k 70k+" )             /* last bonus life at 960k : max. 14 bonus lives */
	PORT_DIPSETTING(    0x00, "60k 140k 80k+" )             /* last bonus life at 940k : max. 12 bonus lives */
	PORT_DIPNAME( 0x70, 0x20, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:5,6,7") /* "Difficult" default setting according to Centuri manual */
	PORT_DIPSETTING(    0x70, "1 (Easiest)" )
	PORT_DIPSETTING(    0x60, "2" )
	PORT_DIPSETTING(    0x50, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x30, "5 (Average)" )
	PORT_DIPSETTING(    0x20, "6" )
	PORT_DIPSETTING(    0x10, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hardest)" )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	512,    /* 512 characters */
	2,  /* 2 bits per pixel */
	{ 4, 0 },
	{ 0, 1, 2, 3, 8*8+0,8*8+1,8*8+2,8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8    /* every char takes 16 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	8,16,   /* 8*16 sprites */
	256,    /* 256 sprites */
	4,  /* 4 bits per pixel */
	{ 0x4000*8+4, 0x4000*8+0, 4, 0  },
	{ 0, 1, 2, 3,  8*8, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8    /* every sprite takes 64 consecutive bytes */
};


static GFXDECODE_START( gyruss )
	GFXDECODE_ENTRY( "gfx1", 0x0000, spritelayout, 0, 16 )  /* upper half */
	GFXDECODE_ENTRY( "gfx1", 0x0010, spritelayout, 0, 16 )  /* lower half */
	GFXDECODE_ENTRY( "gfx2", 0x0000, charlayout,   16*16, 16 )
GFXDECODE_END

static const discrete_mixer_desc konami_right_mixer_desc =
	{DISC_MIXER_IS_RESISTOR,
	{RES_K(2.2), RES_K(2.2), RES_K(2.2), RES_K(3.3)/3, RES_K(3.3)/3 },
	{0,0,0,0,0,0},  /* no variable resistors   */
	{0,0,0,0,0,0},  /* no node capacitors      */
	0, 200,
	CAP_U(0.1),
	CAP_U(1),       /* DC - Removal, not in schematics */
	0, 1};

static const discrete_mixer_desc konami_left_mixer_desc =
	{DISC_MIXER_IS_RESISTOR,
	{RES_K(2.2), RES_K(2.2), RES_K(2.2), RES_K(3.3)/3, RES_K(4.7) },
	{0,0,0,0,0,0},  /* no variable resistors   */
	{0,0,0,0,0,0},  /* no node capacitors      */
	0, 200,
	CAP_U(0.1),
	CAP_U(1),       /* DC - Removal, not in schematics */
	0, 1};

static DISCRETE_SOUND_START( gyruss_sound )

	/* Chip 1 right */
	DISCRETE_INPUTX_STREAM(NODE_01, 0, 1.0, 0)
	DISCRETE_INPUTX_STREAM(NODE_02, 1, 1.0, 0)
	DISCRETE_INPUTX_STREAM(NODE_03, 2, 1.0, 0)

	/* Chip 2 left */
	DISCRETE_INPUTX_STREAM(NODE_04, 3, 1.0, 0)
	DISCRETE_INPUTX_STREAM(NODE_05, 4, 1.0, 0)
	DISCRETE_INPUTX_STREAM(NODE_06, 5, 1.0, 0)

	/* Chip 3 right */
	/* Outputs are tied together after 3.3k resistor on each channel.
	 * A/R + B/R + C/R = (A + B + C) / 3 * (1/(R/3))
	 */
	DISCRETE_INPUTX_STREAM(NODE_07, 6, 0.33, 0)
	DISCRETE_INPUTX_STREAM(NODE_08, 7, 0.33, 0)
	DISCRETE_INPUTX_STREAM(NODE_09, 8, 0.33, 0)

	/* Chip 4 right */
	DISCRETE_INPUTX_STREAM(NODE_10, 9, 0.33, 0)
	DISCRETE_INPUTX_STREAM(NODE_11,10, 0.33, 0)
	DISCRETE_INPUTX_STREAM(NODE_12,11, 0.33, 0)

	/* Chip 5 left */
	DISCRETE_INPUTX_STREAM(NODE_13,12, 0.33, 0)
	DISCRETE_INPUTX_STREAM(NODE_14,13, 0.33, 0)
	DISCRETE_INPUTX_STREAM(NODE_15,14, 0.33, 0)

	/* DAC left */
	/* Output voltage depends on load. Datasheet gives 2.4 as minimum.
	 * This is in line with TTL, so 4V with no load seems adequate */
	DISCRETE_INPUTX_DATA(NODE_16, 256.0 * 4.0 / 5.0, 0.0, 0.0)

	/* Chip 1 Filter enable */
	DISCRETE_INPUT_DATA(NODE_21)
	DISCRETE_INPUT_DATA(NODE_22)
	DISCRETE_INPUT_DATA(NODE_23)

	/* Chip 2 Filter enable */
	DISCRETE_INPUT_DATA(NODE_24)
	DISCRETE_INPUT_DATA(NODE_25)
	DISCRETE_INPUT_DATA(NODE_26)

	/* Chip 1 Filter */
	DISCRETE_RCFILTER_SW(NODE_31, 1, NODE_01, NODE_21, AY8910_INTERNAL_RESISTANCE+1000, CAP_U(0.047), CAP_U(0.22), 0, 0)
	DISCRETE_RCFILTER_SW(NODE_32, 1, NODE_02, NODE_22, AY8910_INTERNAL_RESISTANCE+1000, CAP_U(0.047), CAP_U(0.22), 0, 0)
	DISCRETE_RCFILTER_SW(NODE_33, 1, NODE_03, NODE_23, AY8910_INTERNAL_RESISTANCE+1000, CAP_U(0.047), CAP_U(0.22), 0, 0)

	/* Chip 2 Filter */
	DISCRETE_RCFILTER_SW(NODE_34, 1, NODE_04, NODE_24, AY8910_INTERNAL_RESISTANCE+1000, CAP_U(0.047), CAP_U(0.22), 0, 0)
	DISCRETE_RCFILTER_SW(NODE_35, 1, NODE_05, NODE_25, AY8910_INTERNAL_RESISTANCE+1000, CAP_U(0.047), CAP_U(0.22), 0, 0)
	DISCRETE_RCFILTER_SW(NODE_36, 1, NODE_06, NODE_26, AY8910_INTERNAL_RESISTANCE+1000, CAP_U(0.047), CAP_U(0.22), 0, 0)

	/* Chip 3 */
	DISCRETE_ADDER3(NODE_40, 1, NODE_07, NODE_08, NODE_09)
	/* Chip 4 */
	DISCRETE_ADDER3(NODE_41, 1, NODE_10, NODE_11, NODE_12)
	/* Chip 5 */
	DISCRETE_ADDER3(NODE_42, 1, NODE_13, NODE_14, NODE_15)

	/* right channel */
	DISCRETE_MIXER5(NODE_50, 1, NODE_31, NODE_32, NODE_33, NODE_40, NODE_41, &konami_right_mixer_desc)
	/* left channel */
	DISCRETE_MIXER5(NODE_51, 1, NODE_34, NODE_35, NODE_36, NODE_42, NODE_16, &konami_left_mixer_desc)

	DISCRETE_OUTPUT(NODE_50, 11.0)
	DISCRETE_OUTPUT(NODE_51, 11.0)

DISCRETE_SOUND_END


void gyruss_state::machine_start()
{
	save_item(NAME(m_master_nmi_mask));
	save_item(NAME(m_slave_irq_mask));
}

INTERRUPT_GEN_MEMBER(gyruss_state::master_vblank_irq)
{
	if (m_master_nmi_mask)
		device.execute().set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

INTERRUPT_GEN_MEMBER(gyruss_state::slave_vblank_irq)
{
	if (m_slave_irq_mask)
		device.execute().set_input_line(0, ASSERT_LINE);
}

static MACHINE_CONFIG_START( gyruss, gyruss_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK/6)    /* 3.072 MHz */
	MCFG_CPU_PROGRAM_MAP(main_cpu1_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gyruss_state,  master_vblank_irq)

	MCFG_CPU_ADD("sub", KONAMI1, MASTER_CLOCK/12)     /* 1.536 MHz */
	MCFG_CPU_PROGRAM_MAP(main_cpu2_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gyruss_state,  slave_vblank_irq)

	MCFG_CPU_ADD("audiocpu", Z80, SOUND_CLOCK/4)    /* 3.579545 MHz */
	MCFG_CPU_PROGRAM_MAP(audio_cpu1_map)
	MCFG_CPU_IO_MAP(audio_cpu1_io_map)

	MCFG_CPU_ADD("audio2", I8039, XTAL_8MHz)
	MCFG_CPU_PROGRAM_MAP(audio_cpu2_map)
	MCFG_CPU_IO_MAP(audio_cpu2_io_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(gyruss_state, screen_update_gyruss)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", gyruss)
	MCFG_PALETTE_ADD("palette", 16*4+16*16)
	MCFG_PALETTE_INDIRECT_ENTRIES(32)
	MCFG_PALETTE_INIT_OWNER(gyruss_state, gyruss)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ay1", AY8910, SOUND_CLOCK/8)
	MCFG_AY8910_OUTPUT_TYPE(AY8910_DISCRETE_OUTPUT)
	MCFG_AY8910_RES_LOADS(RES_K(3.3), RES_K(3.3), RES_K(3.3))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(gyruss_state, gyruss_filter0_w))
	MCFG_SOUND_ROUTE_EX(0, "discrete", 1.0, 0)
	MCFG_SOUND_ROUTE_EX(1, "discrete", 1.0, 1)
	MCFG_SOUND_ROUTE_EX(2, "discrete", 1.0, 2)

	MCFG_SOUND_ADD("ay2", AY8910, SOUND_CLOCK/8)
	MCFG_AY8910_OUTPUT_TYPE(AY8910_DISCRETE_OUTPUT)
	MCFG_AY8910_RES_LOADS(RES_K(3.3), RES_K(3.3), RES_K(3.3))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(gyruss_state, gyruss_filter1_w))
	MCFG_SOUND_ROUTE_EX(0, "discrete", 1.0, 3)
	MCFG_SOUND_ROUTE_EX(1, "discrete", 1.0, 4)
	MCFG_SOUND_ROUTE_EX(2, "discrete", 1.0, 5)

	MCFG_SOUND_ADD("ay3", AY8910, SOUND_CLOCK/8)
	MCFG_AY8910_OUTPUT_TYPE(AY8910_DISCRETE_OUTPUT)
	MCFG_AY8910_RES_LOADS(RES_K(3.3), RES_K(3.3), RES_K(3.3))
	MCFG_AY8910_PORT_A_READ_CB(READ8(gyruss_state, gyruss_portA_r))
	MCFG_SOUND_ROUTE_EX(0, "discrete", 1.0, 6)
	MCFG_SOUND_ROUTE_EX(1, "discrete", 1.0, 7)
	MCFG_SOUND_ROUTE_EX(2, "discrete", 1.0, 8)

	MCFG_SOUND_ADD("ay4", AY8910, SOUND_CLOCK/8)
	MCFG_AY8910_OUTPUT_TYPE(AY8910_DISCRETE_OUTPUT)
	MCFG_AY8910_RES_LOADS(RES_K(3.3), RES_K(3.3), RES_K(3.3))
	MCFG_SOUND_ROUTE_EX(0, "discrete", 1.0, 9)
	MCFG_SOUND_ROUTE_EX(1, "discrete", 1.0, 10)
	MCFG_SOUND_ROUTE_EX(2, "discrete", 1.0, 11)

	MCFG_SOUND_ADD("ay5", AY8910, SOUND_CLOCK/8)
	MCFG_AY8910_OUTPUT_TYPE(AY8910_DISCRETE_OUTPUT)
	MCFG_AY8910_RES_LOADS(RES_K(3.3), RES_K(3.3), RES_K(3.3))
	MCFG_SOUND_ROUTE_EX(0, "discrete", 1.0, 12)
	MCFG_SOUND_ROUTE_EX(1, "discrete", 1.0, 13)
	MCFG_SOUND_ROUTE_EX(2, "discrete", 1.0, 14)

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(gyruss_sound)
	MCFG_SOUND_ROUTE(0, "rspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "lspeaker",  1.0)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( gyruss )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gyrussk.1",    0x0000, 0x2000, CRC(c673b43d) SHA1(7c464fb154bac35dd6e2f547e157addeb8798194) )
	ROM_LOAD( "gyrussk.2",    0x2000, 0x2000, CRC(a4ec03e4) SHA1(08c33ad7fcc2ad5e5787a1050284e3f8164f4618) )
	ROM_LOAD( "gyrussk.3",    0x4000, 0x2000, CRC(27454a98) SHA1(030c7df225652ee20d5ef64d005eb011dc89a27d) )
	/* the diagnostics ROM would go here */

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "gyrussk.9",    0xe000, 0x2000, CRC(822bf27e) SHA1(36d5bea2392a7d3476dd797dc05602705cfa23ef) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gyrussk.1a",   0x0000, 0x2000, CRC(f4ae1c17) SHA1(ae568c96a31d910afe30d2b7eeb9ed1ed07290e3) )
	ROM_LOAD( "gyrussk.2a",   0x2000, 0x2000, CRC(ba498115) SHA1(9cd1f42898cc590f39ba7cb3c975b0b3d3062eba) )
	/* the diagnostics ROM would go here */

	ROM_REGION( 0x1000, "audio2", 0 )   /* 8039 */
	ROM_LOAD( "gyrussk.3a",   0x0000, 0x1000, CRC(3f9b5dea) SHA1(6e807da02c2885b18e8cc2199f12f6be9040bf75) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "gyrussk.6",    0x0000, 0x2000, CRC(c949db10) SHA1(fcb8bcbd2bdd751fecb322a33c8a92fb6f07a7ab) )
	ROM_LOAD( "gyrussk.5",    0x2000, 0x2000, CRC(4f22411a) SHA1(763bcd039f8c1838a0d7da7d4dadc14a26e25596) )
	ROM_LOAD( "gyrussk.8",    0x4000, 0x2000, CRC(47cd1fbc) SHA1(8203c4ff0b1cd7b4dbc708e300bfeac1e7366e09) )
	ROM_LOAD( "gyrussk.7",    0x6000, 0x2000, CRC(8e8d388c) SHA1(8f2928d71c02aba977d67575d6e34d69bda2b9d4) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "gyrussk.4",    0x0000, 0x2000, CRC(27d8329b) SHA1(564ff945465a23d93a93137ad277298770dfa06a) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "gyrussk.pr3",  0x0000, 0x0020, CRC(98782db3) SHA1(b891e43b25187faca8002919ccb44d744daa3594) )    /* palette */
	ROM_LOAD( "gyrussk.pr1",  0x0020, 0x0100, CRC(7ed057de) SHA1(c04069ae1e2c62f9b3048844cd8cf5e1b03b7d3c) )    /* sprite lookup table */
	ROM_LOAD( "gyrussk.pr2",  0x0120, 0x0100, CRC(de823a81) SHA1(1af94b2a6a319a89b238a5076a2867f1cfd279b0) )    /* character lookup table */
ROM_END

ROM_START( gyrussce )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gya-1.bin",    0x0000, 0x2000, CRC(85f8b7c2) SHA1(5dde696b53efedee671d500feae1d314e95b1c96) )
	ROM_LOAD( "gya-2.bin",    0x2000, 0x2000, CRC(1e1a970f) SHA1(5a2e391489608f7571bbb4f85549a79795e2177e) )
	ROM_LOAD( "gya-3.bin",    0x4000, 0x2000, CRC(f6dbb33b) SHA1(19cab8e7f2f2358b6271ab402f132654e8be95d4) )
	/* the diagnostics ROM would go here */

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "gyrussk.9",    0xe000, 0x2000, CRC(822bf27e) SHA1(36d5bea2392a7d3476dd797dc05602705cfa23ef) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gyrussk.1a",   0x0000, 0x2000, CRC(f4ae1c17) SHA1(ae568c96a31d910afe30d2b7eeb9ed1ed07290e3) )
	ROM_LOAD( "gyrussk.2a",   0x2000, 0x2000, CRC(ba498115) SHA1(9cd1f42898cc590f39ba7cb3c975b0b3d3062eba) )
	/* the diagnostics ROM would go here */

	ROM_REGION( 0x1000, "audio2", 0 )   /* 8039 */
	ROM_LOAD( "gyrussk.3a",   0x0000, 0x1000, CRC(3f9b5dea) SHA1(6e807da02c2885b18e8cc2199f12f6be9040bf75) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "gyrussk.6",    0x0000, 0x2000, CRC(c949db10) SHA1(fcb8bcbd2bdd751fecb322a33c8a92fb6f07a7ab) )
	ROM_LOAD( "gyrussk.5",    0x2000, 0x2000, CRC(4f22411a) SHA1(763bcd039f8c1838a0d7da7d4dadc14a26e25596) )
	ROM_LOAD( "gyrussk.8",    0x4000, 0x2000, CRC(47cd1fbc) SHA1(8203c4ff0b1cd7b4dbc708e300bfeac1e7366e09) )
	ROM_LOAD( "gyrussk.7",    0x6000, 0x2000, CRC(8e8d388c) SHA1(8f2928d71c02aba977d67575d6e34d69bda2b9d4) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "gyrussk.4",    0x0000, 0x2000, CRC(27d8329b) SHA1(564ff945465a23d93a93137ad277298770dfa06a) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "gyrussk.pr3",  0x0000, 0x0020, CRC(98782db3) SHA1(b891e43b25187faca8002919ccb44d744daa3594) )    /* palette */
	ROM_LOAD( "gyrussk.pr1",  0x0020, 0x0100, CRC(7ed057de) SHA1(c04069ae1e2c62f9b3048844cd8cf5e1b03b7d3c) )    /* sprite lookup table */
	ROM_LOAD( "gyrussk.pr2",  0x0120, 0x0100, CRC(de823a81) SHA1(1af94b2a6a319a89b238a5076a2867f1cfd279b0) )    /* character lookup table */
ROM_END

ROM_START( gyrussb ) /* PCB has stickers stating "TAITO (NEW ZEALAND) LTD" */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin",        0x0000, 0x2000, CRC(6bc21c10) SHA1(9d44f766398b9994f90edb2ffb272b4f22564854) ) /* Labeled as "1", minor code patch / redirection */
	ROM_LOAD( "gyrussk.2",    0x2000, 0x2000, CRC(a4ec03e4) SHA1(08c33ad7fcc2ad5e5787a1050284e3f8164f4618) ) /* Labeled as "2" */
	ROM_LOAD( "gyrussk.3",    0x4000, 0x2000, CRC(27454a98) SHA1(030c7df225652ee20d5ef64d005eb011dc89a27d) ) /* Labeled as "3" */
	/* the diagnostics ROM would go here */

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "gyrussk.9",    0xe000, 0x2000, CRC(822bf27e) SHA1(36d5bea2392a7d3476dd797dc05602705cfa23ef) ) /* Labeled as "9" */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gyrussk.1a",   0x0000, 0x2000, CRC(f4ae1c17) SHA1(ae568c96a31d910afe30d2b7eeb9ed1ed07290e3) ) /* Labeled as "11" */
	ROM_LOAD( "gyrussk.2a",   0x2000, 0x2000, CRC(ba498115) SHA1(9cd1f42898cc590f39ba7cb3c975b0b3d3062eba) ) /* Labeled as "12" */
	/* the diagnostics ROM would go here */

	ROM_REGION( 0x1000, "audio2", 0 )   /* 8039 */
	ROM_LOAD( "gyrussk.3a",   0x0000, 0x1000, CRC(3f9b5dea) SHA1(6e807da02c2885b18e8cc2199f12f6be9040bf75) ) /* Labeled as "13" */

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "gyrussk.6",    0x0000, 0x2000, CRC(c949db10) SHA1(fcb8bcbd2bdd751fecb322a33c8a92fb6f07a7ab) ) /* Labeled as "6" */
	ROM_LOAD( "gyrussk.5",    0x2000, 0x2000, CRC(4f22411a) SHA1(763bcd039f8c1838a0d7da7d4dadc14a26e25596) ) /* Labeled as "5" */
	ROM_LOAD( "gyrussk.8",    0x4000, 0x2000, CRC(47cd1fbc) SHA1(8203c4ff0b1cd7b4dbc708e300bfeac1e7366e09) ) /* Labeled as "8" */
	ROM_LOAD( "gyrussk.7",    0x6000, 0x2000, CRC(8e8d388c) SHA1(8f2928d71c02aba977d67575d6e34d69bda2b9d4) ) /* Labeled as "7" */

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "gyrussk.4",    0x0000, 0x2000, CRC(27d8329b) SHA1(564ff945465a23d93a93137ad277298770dfa06a) ) /* Labeled as "4" */

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "gyrussk.pr3",  0x0000, 0x0020, CRC(98782db3) SHA1(b891e43b25187faca8002919ccb44d744daa3594) )    /* palette */
	ROM_LOAD( "gyrussk.pr1",  0x0020, 0x0100, CRC(7ed057de) SHA1(c04069ae1e2c62f9b3048844cd8cf5e1b03b7d3c) )    /* sprite lookup table */
	ROM_LOAD( "gyrussk.pr2",  0x0120, 0x0100, CRC(de823a81) SHA1(1af94b2a6a319a89b238a5076a2867f1cfd279b0) )    /* character lookup table */
ROM_END

ROM_START( venus )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "r1",           0x0000, 0x2000, CRC(d030abb1) SHA1(14a70e15f5df9ef957779771d8915203d3828532) )
	ROM_LOAD( "r2",           0x2000, 0x2000, CRC(dbf65d4d) SHA1(a0ad0dc3420442f06691bda2115fadd961ce86a7) )
	ROM_LOAD( "r3",           0x4000, 0x2000, CRC(db246fcd) SHA1(c0228b35591c9e1c778370a2abd3739c441f14aa) )
	/* the diagnostics ROM would go here */

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "gyrussk.9",    0xe000, 0x2000, CRC(822bf27e) SHA1(36d5bea2392a7d3476dd797dc05602705cfa23ef) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gyrussk.1a",   0x0000, 0x2000, CRC(f4ae1c17) SHA1(ae568c96a31d910afe30d2b7eeb9ed1ed07290e3) )
	ROM_LOAD( "gyrussk.2a",   0x2000, 0x2000, CRC(ba498115) SHA1(9cd1f42898cc590f39ba7cb3c975b0b3d3062eba) )
	/* the diagnostics ROM would go here */

	ROM_REGION( 0x1000, "audio2", 0 )   /* 8039 */
	ROM_LOAD( "gyrussk.3a",   0x0000, 0x1000, CRC(3f9b5dea) SHA1(6e807da02c2885b18e8cc2199f12f6be9040bf75) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "gyrussk.6",    0x0000, 0x2000, CRC(c949db10) SHA1(fcb8bcbd2bdd751fecb322a33c8a92fb6f07a7ab) )
	ROM_LOAD( "gyrussk.5",    0x2000, 0x2000, CRC(4f22411a) SHA1(763bcd039f8c1838a0d7da7d4dadc14a26e25596) )
	ROM_LOAD( "gyrussk.8",    0x4000, 0x2000, CRC(47cd1fbc) SHA1(8203c4ff0b1cd7b4dbc708e300bfeac1e7366e09) )
	ROM_LOAD( "gyrussk.7",    0x6000, 0x2000, CRC(8e8d388c) SHA1(8f2928d71c02aba977d67575d6e34d69bda2b9d4) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "gyrussk.4",    0x0000, 0x2000, CRC(27d8329b) SHA1(564ff945465a23d93a93137ad277298770dfa06a) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "gyrussk.pr3",  0x0000, 0x0020, CRC(98782db3) SHA1(b891e43b25187faca8002919ccb44d744daa3594) )    /* palette */
	ROM_LOAD( "gyrussk.pr1",  0x0020, 0x0100, CRC(7ed057de) SHA1(c04069ae1e2c62f9b3048844cd8cf5e1b03b7d3c) )    /* sprite lookup table */
	ROM_LOAD( "gyrussk.pr2",  0x0120, 0x0100, CRC(de823a81) SHA1(1af94b2a6a319a89b238a5076a2867f1cfd279b0) )    /* character lookup table */
ROM_END


DRIVER_INIT_MEMBER(gyruss_state,gyruss)
{
}


GAME( 1983, gyruss,   0,        gyruss,   gyruss, gyruss_state,   gyruss, ROT90, "Konami", "Gyruss", MACHINE_SUPPORTS_SAVE )
GAME( 1983, gyrussce, gyruss,   gyruss,   gyrussce, gyruss_state, gyruss, ROT90, "Konami (Centuri license)", "Gyruss (Centuri)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, gyrussb,  gyruss,   gyruss,   gyruss, gyruss_state,   gyruss, ROT90, "bootleg?", "Gyruss (bootleg?)", MACHINE_SUPPORTS_SAVE ) /* Supposed Taito NZ license, but (c) Konami */
GAME( 1983, venus,    gyruss,   gyruss,   gyruss, gyruss_state,   gyruss, ROT90, "bootleg", "Venus (bootleg of Gyruss)", MACHINE_SUPPORTS_SAVE )

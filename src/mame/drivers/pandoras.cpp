// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

Pandora's Palace(GX328) (c) 1984 Konami/Interlogic

Driver by Manuel Abadia <emumanu+mame@gmail.com>

Notes:
- Press 1P and 2P together to enter test mode.

TODO:
- CPU B continuously reads from 1e00. It seems to be important, could be a
  scanline counter or something like that.

2009-03:
Added dsw locations and verified factory setting based on Guru's notes
(DSW3 not mentioned)

Boards:
- CPU/Video board labeled PWB(A)2000109B
- Sound board labeled PWB(B)3000154A

***************************************************************************/

#include "emu.h"
#include "includes/pandoras.h"
#include "includes/konamipt.h"

#include "cpu/m6809/m6809.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "screen.h"
#include "speaker.h"


#define MASTER_CLOCK        XTAL(18'432'000)
#define SOUND_CLOCK         XTAL(14'318'181)


INTERRUPT_GEN_MEMBER(pandoras_state::pandoras_master_interrupt)
{
	if (m_irq_enable_a)
		device.execute().set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
}

INTERRUPT_GEN_MEMBER(pandoras_state::pandoras_slave_interrupt)
{
	if (m_irq_enable_b)
		device.execute().set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
}

WRITE_LINE_MEMBER(pandoras_state::cpua_irq_enable_w)
{
	if (!state)
		m_maincpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
	m_irq_enable_a = state;
}

WRITE_LINE_MEMBER(pandoras_state::cpub_irq_enable_w)
{
	if (!state)
		m_subcpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
	m_irq_enable_b = state;
}

WRITE8_MEMBER(pandoras_state::pandoras_cpua_irqtrigger_w)
{
	if (!m_firq_old_data_a && data)
		m_maincpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);

	m_firq_old_data_a = data;
}

WRITE8_MEMBER(pandoras_state::pandoras_cpub_irqtrigger_w)
{
	if (!m_firq_old_data_b && data)
		m_subcpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);

	m_firq_old_data_b = data;
}

WRITE8_MEMBER(pandoras_state::pandoras_i8039_irqtrigger_w)
{
	m_mcu->set_input_line(0, ASSERT_LINE);
}

WRITE8_MEMBER(pandoras_state::i8039_irqen_and_status_w)
{
	/* bit 7 enables IRQ */
	if ((data & 0x80) == 0)
		m_mcu->set_input_line(0, CLEAR_LINE);

	/* bit 5 goes to 8910 port A */
	m_i8039_status = (data & 0x20) >> 5;
}

WRITE8_MEMBER(pandoras_state::pandoras_z80_irqtrigger_w)
{
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff);
}

WRITE_LINE_MEMBER(pandoras_state::coin_counter_1_w)
{
	machine().bookkeeping().coin_counter_w(0, state);
}

WRITE_LINE_MEMBER(pandoras_state::coin_counter_2_w)
{
	machine().bookkeeping().coin_counter_w(1, state);
}


ADDRESS_MAP_START(pandoras_state::pandoras_master_map)
	AM_RANGE(0x0000, 0x0fff) AM_RAM AM_SHARE("spriteram")               /* Work RAM (Shared with CPU B) */
	AM_RANGE(0x1000, 0x13ff) AM_RAM_WRITE(pandoras_cram_w) AM_SHARE("colorram") /* Color RAM (shared with CPU B) */
	AM_RANGE(0x1400, 0x17ff) AM_RAM_WRITE(pandoras_vram_w) AM_SHARE("videoram") /* Video RAM (shared with CPU B) */
	AM_RANGE(0x1800, 0x1807) AM_DEVWRITE("mainlatch", ls259_device, write_d0)               /* INT control */
	AM_RANGE(0x1a00, 0x1a00) AM_WRITE(pandoras_scrolly_w)                                   /* bg scroll */
	AM_RANGE(0x1c00, 0x1c00) AM_WRITE(pandoras_z80_irqtrigger_w)                            /* cause INT on the Z80 */
	AM_RANGE(0x1e00, 0x1e00) AM_DEVWRITE("soundlatch", generic_latch_8_device, write)                                            /* sound command to the Z80 */
	AM_RANGE(0x2000, 0x2000) AM_WRITE(pandoras_cpub_irqtrigger_w)                           /* cause FIRQ on CPU B */
	AM_RANGE(0x2001, 0x2001) AM_DEVWRITE("watchdog", watchdog_timer_device, reset_w)        /* watchdog reset */
	AM_RANGE(0x4000, 0x5fff) AM_ROM                                                         /* space for diagnostic ROM */
	AM_RANGE(0x6000, 0x67ff) AM_RAM AM_SHARE("share4")                                      /* Shared RAM with CPU B */
	AM_RANGE(0x8000, 0xffff) AM_ROM                                                         /* ROM */
ADDRESS_MAP_END

ADDRESS_MAP_START(pandoras_state::pandoras_slave_map)
	AM_RANGE(0x0000, 0x0fff) AM_RAM AM_SHARE("spriteram")                                       /* Work RAM (Shared with CPU A) */
	AM_RANGE(0x1000, 0x13ff) AM_RAM_WRITE(pandoras_cram_w) AM_SHARE("colorram")             /* Color RAM (shared with CPU A) */
	AM_RANGE(0x1400, 0x17ff) AM_RAM_WRITE(pandoras_vram_w) AM_SHARE("videoram")             /* Video RAM (shared with CPU A) */
	AM_RANGE(0x1800, 0x1800) AM_READ_PORT("DSW1")
	AM_RANGE(0x1800, 0x1807) AM_DEVWRITE("mainlatch", ls259_device, write_d0)               /* INT control */
	AM_RANGE(0x1a00, 0x1a00) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x1a01, 0x1a01) AM_READ_PORT("P1")
	AM_RANGE(0x1a02, 0x1a02) AM_READ_PORT("P2")
	AM_RANGE(0x1a03, 0x1a03) AM_READ_PORT("DSW3")
	AM_RANGE(0x1c00, 0x1c00) AM_READ_PORT("DSW2")
//  AM_RANGE(0x1e00, 0x1e00) AM_READNOP                                                     /* ??? seems to be important */
	AM_RANGE(0x8000, 0x8000) AM_DEVWRITE("watchdog", watchdog_timer_device, reset_w)        /* watchdog reset */
	AM_RANGE(0xa000, 0xa000) AM_WRITE(pandoras_cpua_irqtrigger_w)                           /* cause FIRQ on CPU A */
	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_SHARE("share4")                                      /* Shared RAM with the CPU A */
	AM_RANGE(0xe000, 0xffff) AM_ROM                                                         /* ROM */
ADDRESS_MAP_END

ADDRESS_MAP_START(pandoras_state::pandoras_sound_map)
	AM_RANGE(0x0000, 0x1fff) AM_ROM                                                         /* ROM */
	AM_RANGE(0x2000, 0x23ff) AM_RAM                                                         /* RAM */
	AM_RANGE(0x4000, 0x4000) AM_DEVREAD("soundlatch", generic_latch_8_device, read)
	AM_RANGE(0x6000, 0x6000) AM_DEVWRITE("aysnd", ay8910_device, address_w)                          /* AY-8910 */
	AM_RANGE(0x6001, 0x6001) AM_DEVREAD("aysnd", ay8910_device, data_r)                                   /* AY-8910 */
	AM_RANGE(0x6002, 0x6002) AM_DEVWRITE("aysnd", ay8910_device, data_w)                         /* AY-8910 */
	AM_RANGE(0x8000, 0x8000) AM_WRITE(pandoras_i8039_irqtrigger_w)                          /* cause INT on the 8039 */
	AM_RANGE(0xa000, 0xa000) AM_DEVWRITE("soundlatch2", generic_latch_8_device, write)      /* sound command to the 8039 */
ADDRESS_MAP_END

ADDRESS_MAP_START(pandoras_state::pandoras_i8039_map)
	AM_RANGE(0x0000, 0x0fff) AM_ROM
ADDRESS_MAP_END

ADDRESS_MAP_START(pandoras_state::pandoras_i8039_io_map)
	AM_RANGE(0x00, 0xff) AM_DEVREAD("soundlatch2", generic_latch_8_device, read)
ADDRESS_MAP_END


/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( pandoras )
	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "20k and every 60k" )
	PORT_DIPSETTING(    0x10, "30k and every 70k" )
	PORT_DIPSETTING(    0x08, "20k" )
	PORT_DIPSETTING(    0x00, "30k" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Freeze" )
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

	PORT_START("SYSTEM")
	KONAMI8_SYSTEM_10
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	KONAMI8_MONO_B1_UNK

	PORT_START("P2")
	KONAMI8_COCKTAIL_B1_UNK
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 15*4, 14*4, 13*4, 12*4, 11*4, 10*4, 9*4, 8*4,
			7*4, 6*4, 5*4, 4*4, 3*4, 2*4, 1*4, 0*4 },
	{ 15*4*16, 14*4*16, 13*4*16, 12*4*16, 11*4*16, 10*4*16, 9*4*16, 8*4*16,
			7*4*16, 6*4*16, 5*4*16, 4*4*16, 3*4*16, 2*4*16, 1*4*16, 0*4*16 },
	32*4*8
};

static GFXDECODE_START( pandoras )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,     0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout,   16*16, 16 )
GFXDECODE_END

/***************************************************************************

    Machine Driver

***************************************************************************/

void pandoras_state::machine_start()
{
	save_item(NAME(m_firq_old_data_a));
	save_item(NAME(m_firq_old_data_b));
	save_item(NAME(m_irq_enable_a));
	save_item(NAME(m_irq_enable_b));
	save_item(NAME(m_i8039_status));
}

void pandoras_state::machine_reset()
{
	m_firq_old_data_a = 0;
	m_firq_old_data_b = 0;
	m_i8039_status = 0;
}

READ8_MEMBER(pandoras_state::pandoras_portA_r)
{
	return m_i8039_status;
}

READ8_MEMBER(pandoras_state::pandoras_portB_r)
{
	return (m_audiocpu->total_cycles() / 512) & 0x0f;
}

MACHINE_CONFIG_START(pandoras_state::pandoras)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", MC6809E, MASTER_CLOCK/6)  /* CPU A */
	MCFG_CPU_PROGRAM_MAP(pandoras_master_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", pandoras_state,  pandoras_master_interrupt)

	MCFG_CPU_ADD("sub", MC6809E, MASTER_CLOCK/6)      /* CPU B */
	MCFG_CPU_PROGRAM_MAP(pandoras_slave_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", pandoras_state,  pandoras_slave_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80, SOUND_CLOCK/8)
	MCFG_CPU_PROGRAM_MAP(pandoras_sound_map)

	MCFG_CPU_ADD("mcu", I8039, SOUND_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(pandoras_i8039_map)
	MCFG_CPU_IO_MAP(pandoras_i8039_io_map)
	MCFG_MCS48_PORT_P1_OUT_CB(DEVWRITE8("dac", dac_byte_interface, write))
	MCFG_MCS48_PORT_P2_OUT_CB(WRITE8(pandoras_state, i8039_irqen_and_status_w))

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))  /* 100 CPU slices per frame - needed for correct synchronization of the sound CPUs */

	MCFG_DEVICE_ADD("mainlatch", LS259, 0) // C3
	MCFG_ADDRESSABLE_LATCH_Q0_OUT_CB(WRITELINE(pandoras_state, cpua_irq_enable_w)) // ENA
	MCFG_ADDRESSABLE_LATCH_Q1_OUT_CB(NOOP) // OFSET - unknown
	MCFG_ADDRESSABLE_LATCH_Q2_OUT_CB(WRITELINE(pandoras_state, coin_counter_1_w))
	MCFG_ADDRESSABLE_LATCH_Q3_OUT_CB(WRITELINE(pandoras_state, coin_counter_2_w))
	MCFG_ADDRESSABLE_LATCH_Q5_OUT_CB(WRITELINE(pandoras_state, flipscreen_w)) // FLIP
	MCFG_ADDRESSABLE_LATCH_Q6_OUT_CB(WRITELINE(pandoras_state, cpub_irq_enable_w)) // ENB
	MCFG_ADDRESSABLE_LATCH_Q7_OUT_CB(INPUTLINE("sub", INPUT_LINE_RESET)) MCFG_DEVCB_INVERT // RESETB

	MCFG_WATCHDOG_ADD("watchdog")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(pandoras_state, screen_update_pandoras)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pandoras)
	MCFG_PALETTE_ADD("palette", 16*16+16*16)
	MCFG_PALETTE_INDIRECT_ENTRIES(32)
	MCFG_PALETTE_INIT_OWNER(pandoras_state, pandoras)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("speaker")

	MCFG_GENERIC_LATCH_8_ADD("soundlatch")
	MCFG_GENERIC_LATCH_8_ADD("soundlatch2")

	MCFG_SOUND_ADD("aysnd", AY8910, SOUND_CLOCK/8)
	MCFG_AY8910_PORT_A_READ_CB(READ8(pandoras_state, pandoras_portA_r))   // not used
	MCFG_AY8910_PORT_B_READ_CB(READ8(pandoras_state, pandoras_portB_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.4)

	MCFG_SOUND_ADD("dac", DAC_8BIT_R2R, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.12) // unknown DAC
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE_EX(0, "dac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "dac", -1.0, DAC_VREF_NEG_INPUT)
MACHINE_CONFIG_END


/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( pandoras )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64K for the CPU A */
	ROM_LOAD( "pand_j13.cpu",   0x08000, 0x02000, CRC(7a0fe9c5) SHA1(e68c8d76d1abb69ac72b0e2cd8c1dfc540064ee3) )
	ROM_LOAD( "pand_j12.cpu",   0x0a000, 0x02000, CRC(7dc4bfe1) SHA1(359c3051e5d7a34d0e49578e4c168fd19c73e202) )
	ROM_LOAD( "pand_j10.cpu",   0x0c000, 0x02000, CRC(be3af3b7) SHA1(91321b53e17e58b674104cb95b1c35ee8fecae22) )
	ROM_LOAD( "pand_j9.cpu",    0x0e000, 0x02000, CRC(e674a17a) SHA1(a4b096dc455425dd60298acf2203659ef6f8d857) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64K for the CPU B */
	ROM_LOAD( "pand_j5.cpu",    0x0e000, 0x02000, CRC(4aab190b) SHA1(d2204953d6b6b34cea851bfc9c2b31426e75f90b) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64K for the Sound CPU */
	ROM_LOAD( "pand_6c.snd",    0x00000, 0x02000, CRC(0c1f109d) SHA1(4e6cdee99261764bd2fea5abbd49d800baba0dc5) )

	ROM_REGION( 0x2000, "mcu", 0 ) /* 4K for the Sound CPU 2 (Data is mirrored to fit into an 8K rom) */
	ROM_LOAD( "pand_7e.snd",    0x00000, 0x02000, CRC(1071c1ba) SHA1(3693be69f4b32fb3031bcdee8cac0d46ec8c2804) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "pand_j18.cpu",   0x00000, 0x02000, CRC(99a696c5) SHA1(35a27cd5ecc51a9a1acf01eb8078a1028f03be32) )    /* sprites */
	ROM_LOAD( "pand_j17.cpu",   0x02000, 0x02000, CRC(38a03c21) SHA1(b0c8f642787bab3cd1d76657e56f07f4f6f9073c) )
	ROM_LOAD( "pand_j16.cpu",   0x04000, 0x02000, CRC(e0708a78) SHA1(9dbd08b6ca8a66a61e128d1806888696273de848) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "pand_a18.cpu",   0x00000, 0x02000, CRC(23706d4a) SHA1(cca92e6ff90e3006a79a214f1211fd659771de53) )    /* tiles */
	ROM_LOAD( "pand_a19.cpu",   0x02000, 0x02000, CRC(a463b3f9) SHA1(549b7ee6e47325b80186441da11879fb8b1b47be) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "pandora.2a",     0x0000, 0x020, CRC(4d56f939) SHA1(a8dac604bfdaf4b153b75dbf165de113152b6daa) ) /* palette */
	ROM_LOAD( "pandora.17g",    0x0020, 0x100, CRC(c1a90cfc) SHA1(c6581f2d543e38f1de399774183cf0698e61dab5) ) /* sprite lookup table */
	ROM_LOAD( "pandora.16b",    0x0120, 0x100, CRC(c89af0c3) SHA1(4072c8d61521b34ce4dbce1d48f546402e9539cd) ) /* character lookup table */
ROM_END


GAME( 1984, pandoras, 0, pandoras, pandoras, pandoras_state, 0, ROT90, "Konami / Interlogic", "Pandora's Palace", MACHINE_SUPPORTS_SAVE )

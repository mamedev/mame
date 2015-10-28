// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi

/* PGM System (c)1997 IGS

Based on Information from ElSemi

A flexible cartridge based platform some would say was designed to compete with
SNK's NeoGeo and Capcom's CPS Hardware systems, despite its age it only uses a
68000 for the main processor and a Z80 to drive the sound, just like the two
previously mentioned systems in that respect..

Resolution is 448x224, 15 bit colour

Sound system is ICS WaveFront 2115 Wavetable midi synthesizer, used in some
actual sound cards (Turtle Beach)

Later games are encrypted.  Latest games (kov2, ddp2) include an arm7
coprocessor with an internal rom and an encrypted external rom.

Roms Contain the Following Data

Pxxxx - 68k Program
Txxxx - TX & BG Graphics (2 formats within the same rom)
Mxxxx - Music samples (8 bit mono 11025Hz)
Axxxx - Colour Data (for sprites)
Bxxxx - Masks & A Rom Colour Indexes (for sprites)

There is no rom for the Z80, the program is uploaded by the 68k

Known Games on this Platform
----------------------------


010x  - 1997  - Oriental Legend
020x  - 1997  - Dragon World 2
030x  - 1998  - The Killing Blade
040x  - 1998  - Dragon World 3
050x  - 1999? - Oriental Legend Super
060x  - 1999  - Knights of Valor, Knights of Valor Plus, Knights of Valor Super Heroes
070x  - 1999  - Photo Y2k
080x  - 1999  - Puzzle Star
090x  - 2001  - Puzzli II
100x  - 2001  - Martial Masters

120x  - 2001  - Knights of Valor 2 Plus (9 Dragons?)
130x  - 2001  - DoDonpachi II

0450x - 2002  - Demon Front (also known to be produced / sold in a single PCB version)

(add more)

ToDo:

 IRQ4 generation - Puzzli 2 doens't like this, many other games require it for coins / inputs to work.


Protection Devices / Co-processors
----------------------------------

IGS used a variety of additional ASIC chips on the game boards, these act as protection and
also give additional power to the board to make up for the limited power of the 68000
processor.  Some protection devices use external data roms, others have internal code only.
Most of these are not emulated correctly. In most cases the protection device supplies the
game region code..

ASIC 3:
    state based device?
    see
    machine/pgmprot.c

ASIC 25 + ASIC 12
    state based device + rom overlays
    see
    machine/pgmprot5.c

ASIC 25 + ASIC 22
    state based device + encrypted DMA device
    see
    machine/pgmprot4.c

ASIC 25 + ASIC 28
    state based device + encrypted DMA device?
    see
    machine/pgmprot6.c

ASIC 027A(55857F/55857G):
    ARM based CPUs with internal ROM
    see
    machine/pgmprot1.c
    machine/pgmprot2.c
    machine/pgmprot3.c


there are probably more...

PCB Layout
----------

IGS PCB NO-0133-2 (Main Board)
|-------------------------------------------------------------------------------------|
|   |----------------------------|   |----------|   |----------------------------|    |
|   |----------------------------|   |----------|   |----------------------------|    |
|                                      PGM_T01S.U29  UM61256    SRM2B61256  SRM2B61256|
| |---------|  33.8688MHz   |----------|                        SRM2B61256  SRM2B61256|
| |WAVEFRONT|               |L8A0290   |   UM6164  UM6164                             |
| |ICS2115V |               |IGS023    |                 PGM_P01S.U20              SW2|
| |(PLCC84) |               |(QFP256)  |                                              |
| |         |               |          |                                              |
| |---------|        50MHz  |----------|                                              |
|    UPD6379  PGM_M01S.U18                             |----------|                   |
|VOL                                                   |MC68HC000 |          74HC132  |
|                                                      |FN20      |   20MHz  74HC132  |
|  UPC844C    |------|                                 |(PLCC68)  |                   |
|             |Z80   |                                 |          |          V3021    |
|             |PLCC44|                  PAL            |----------|                   |
|             |------|    |--------|                                      32.768kHz   |-|
|                         |IGS026  |                                                    |
|                         |(QFP144)|           |--------|                              I|
|                         |        |           |IGS026  |                              D|
|                         |--------|           |(QFP144)|                              C|
|TDA1519A    UM61256 UM61256                   |        |                              3|
|                              TD62064         |--------|                              4|
|                                                                          3.6V_BATT    |
|                                                                                     |-|
|              |----|                                           |-----|     SW3       |
|              |    |               J  A  M  M  A               |     | SW1           |
|--------------|    |-------------------------------------------|     |---------------|


IGS PCB NO-0136 (Riser)
|-------------------------------------------------------------------------------------|
|      |---------------------------------|  |---------------------------------|       |
|      |---------------------------------|  |---------------------------------|       |
|                                                                                     |
|      |---------------------------------|  |---------------------------------|       |
|      |---------------------------------|  |---------------------------------|       |
|                                                                                     |
|   |----------------------------|   |----------|   |----------------------------|    |
|---|                            |---|          |---|                            |----|
    |----------------------------|   |----------|   |----------------------------|

Notes:
      All IC's are shown.

      CPU's
      -----
         68HC000FN20 - Motorola 68000 processor, clocked at 20.000MHz (PLCC68)
         Z80         - Zilog Z0840008VSC Z80 processor, clocked at 8.468MHz (PLCC44)

      SOUND
      -----
         ICS2115     - ICS WaveFront ICS2115V Wavetable Midi Synthesizer, clocked at 33.8688MHz (PLCC84)

      RAM
      ---
         SRM2B256 - Epson SRM2B256SLMX55 8K x8 SRAM (x4, SOP28)
         UM6164   - Unicorn Microelectronics UM6164DS-12 8K x8 SRAM (x2, SOJ28)
         UM61256  - Unicorn Microelectronics UM61256FS-15 32K x8 SRAM (x3, SOJ28)

      ROMs
      ----
         PGM_M01S.U18 - 16MBit MASKROM (TSOP48)
         PGM_P01S.U20 - 1MBit  MASKROM (DIP40, socketed, equivalent to 27C1024 EPROM)
         PGM_T01S.U29 - 16MBit MASKROM (SOP44)

      CUSTOM IC's
      -----------
         IGS023 (QFP256)
         IGS026 (x2, QFP144)

      OTHER
      -----
         3.6V_BATT - 3.6V NICad battery, connected to the V3021 RTC
         IDC34     - IDC34 way flat cable plug, PGM can support 4 players max in two cabs,
                     this is jamma connecter for another cab and the P3&P4
         PAL       - Atmel ATF16V8B PAL (DIP20)
         SW1       - Push button switch to enter Test Mode
         SW2       - 8 position DIP Switch (for configuration of PCB/game options)
         SW3       - SPDT switch , to clear the NVRAM and reset the whole system
         TD62064   - Toshiba NPN 50V 1.5A Quad Darlinton Switch; for driving coin meters (DIP16)
         TDA1519A  - Philips 2x 6W Stereo Power AMP (SIL9)
         uPD6379   - NEC 2-channel 16-bit D/A converter 10mW typ. (SOIC8)
         uPC844C   - NEC Quad High Speed Wide Band Operational Amplifier (DIP14)
         V3021     - EM Microelectronic-Marin SA Ultra Low Power 32kHz CMOS Real Time Clock (DIP8)
         VOL       - Volume potentiometer

*/

#define PGMLOGERROR 0

#include "emu.h"
#include "includes/pgm.h"


READ16_MEMBER(pgm_state::pgm_videoram_r)
{
	if (offset < 0x4000 / 2)
		return m_bg_videoram[offset&0x7ff];
	else if (offset < 0x7000 / 2)
		return m_tx_videoram[offset&0xfff];
	else
		return m_videoram[offset];
}

WRITE16_MEMBER(pgm_state::pgm_videoram_w)
{
	if (offset < 0x4000 / 2)
		pgm_bg_videoram_w(space, offset&0x7ff, data, mem_mask);
	else if (offset < 0x7000 / 2)
		pgm_tx_videoram_w(space, offset&0xfff, data, mem_mask);
	else
		COMBINE_DATA(&m_videoram[offset]);
}

WRITE16_MEMBER(pgm_state::pgm_coin_counter_w)
{
	coin_counter_w(machine(), 0, data & 0x0001);
	coin_counter_w(machine(), 1, data & 0x0002);
	coin_counter_w(machine(), 2, data & 0x0004);
	coin_counter_w(machine(), 3, data & 0x0008);
}

READ16_MEMBER(pgm_state::z80_ram_r)
{
	return (m_z80_mainram[offset * 2] << 8) | m_z80_mainram[offset * 2 + 1];
}

WRITE16_MEMBER(pgm_state::z80_ram_w)
{
	int pc = space.device().safe_pc();

	if (ACCESSING_BITS_8_15)
		m_z80_mainram[offset * 2] = data >> 8;
	if (ACCESSING_BITS_0_7)
		m_z80_mainram[offset * 2 + 1] = data;

	if (pc != 0xf12 && pc != 0xde2 && pc != 0x100c50 && pc != 0x100b20)
		if (PGMLOGERROR)
			logerror("Z80: write %04x, %04x @ %04x (%06x)\n", offset * 2, data, mem_mask, space.device().safe_pc());
}

WRITE16_MEMBER(pgm_state::z80_reset_w)
{
	if (PGMLOGERROR)
		logerror("Z80: reset %04x @ %04x (%06x)\n", data, mem_mask, space.device().safe_pc());

	if (data == 0x5050)
	{
		m_ics->reset();
		m_soundcpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		m_soundcpu->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
	}
	else
	{
		/* this might not be 100% correct, but several of the games (ddp2, puzzli2 etc. expect the z80 to be turned
		off during data uploads, they write here before the upload */
		m_soundcpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	}
}

WRITE16_MEMBER(pgm_state::z80_ctrl_w)
{
	if (PGMLOGERROR)
		logerror("Z80: ctrl %04x @ %04x (%06x)\n", data, mem_mask, space.device().safe_pc());
}

WRITE16_MEMBER(pgm_state::m68k_l1_w)
{
	if(ACCESSING_BITS_0_7)
	{
		if (PGMLOGERROR)
			logerror("SL 1 m68.w %02x (%06x) IRQ\n", data & 0xff, space.device().safe_pc());
		soundlatch_byte_w(space, 0, data);
		m_soundcpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE );
	}
}

WRITE8_MEMBER(pgm_state::z80_l3_w)
{
	if (PGMLOGERROR)
		logerror("SL 3 z80.w %02x (%04x)\n", data, space.device().safe_pc());
	soundlatch3_byte_w(space, 0, data);
}

WRITE_LINE_MEMBER(pgm_state::pgm_sound_irq)
{
	m_soundcpu->set_input_line(0, state);
}

/*** Memory Maps *************************************************************/

/*** Z80 (sound CPU)**********************************************************/

ADDRESS_MAP_START( pgm_z80_mem, AS_PROGRAM, 8, pgm_state )
	AM_RANGE(0x0000, 0xffff) AM_RAM AM_SHARE("z80_mainram")
ADDRESS_MAP_END

ADDRESS_MAP_START( pgm_z80_io, AS_IO, 8, pgm_state )
	AM_RANGE(0x8000, 0x8003) AM_DEVREADWRITE("ics", ics2115_device, read, write)
	AM_RANGE(0x8100, 0x81ff) AM_READ(soundlatch3_byte_r) AM_WRITE(z80_l3_w)
	AM_RANGE(0x8200, 0x82ff) AM_READWRITE(soundlatch_byte_r, soundlatch_byte_w)
	AM_RANGE(0x8400, 0x84ff) AM_READWRITE(soundlatch2_byte_r, soundlatch2_byte_w)
ADDRESS_MAP_END

/*** 68000 (main CPU) + variants for protection devices **********************/

ADDRESS_MAP_START( pgm_base_mem, AS_PROGRAM, 16, pgm_state )
	AM_RANGE(0x700006, 0x700007) AM_WRITENOP // Watchdog?

	AM_RANGE(0x800000, 0x81ffff) AM_RAM AM_MIRROR(0x0e0000) AM_SHARE("sram") /* Main Ram */

	AM_RANGE(0x900000, 0x907fff) AM_MIRROR(0x0f8000) AM_READWRITE(pgm_videoram_r, pgm_videoram_w) AM_SHARE("videoram") /* IGS023 VIDEO CHIP */
	AM_RANGE(0xa00000, 0xa011ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xb00000, 0xb0ffff) AM_RAM AM_SHARE("videoregs") /* Video Regs inc. Zoom Table */

	AM_RANGE(0xc00002, 0xc00003) AM_READ(soundlatch_word_r) AM_WRITE(m68k_l1_w)
	AM_RANGE(0xc00004, 0xc00005) AM_READWRITE(soundlatch2_word_r, soundlatch2_word_w)
	AM_RANGE(0xc00006, 0xc00007) AM_DEVREADWRITE8("rtc", v3021_device, read, write, 0x00ff)
	AM_RANGE(0xc00008, 0xc00009) AM_WRITE(z80_reset_w)
	AM_RANGE(0xc0000a, 0xc0000b) AM_WRITE(z80_ctrl_w)
	AM_RANGE(0xc0000c, 0xc0000d) AM_READWRITE(soundlatch3_word_r, soundlatch3_word_w)

	AM_RANGE(0xc08000, 0xc08001) AM_READ_PORT("P1P2")
	AM_RANGE(0xc08002, 0xc08003) AM_READ_PORT("P3P4")
	AM_RANGE(0xc08004, 0xc08005) AM_READ_PORT("Service")
	AM_RANGE(0xc08006, 0xc08007) AM_READ_PORT("DSW") AM_WRITE(pgm_coin_counter_w)

	AM_RANGE(0xc10000, 0xc1ffff) AM_READWRITE(z80_ram_r, z80_ram_w) /* Z80 Program */
ADDRESS_MAP_END

ADDRESS_MAP_START( pgm_mem, AS_PROGRAM, 16, pgm_state )
	AM_IMPORT_FROM(pgm_base_mem)
	AM_RANGE(0x000000, 0x01ffff) AM_ROM   /* BIOS ROM */
ADDRESS_MAP_END

ADDRESS_MAP_START( pgm_basic_mem, AS_PROGRAM, 16, pgm_state )
	AM_IMPORT_FROM(pgm_mem)
	AM_RANGE(0x100000, 0x3fffff) AM_ROMBANK("bank1") /* Game ROM */
ADDRESS_MAP_END


/*** Input Ports *************************************************************/

/* enough for 4 players, the basic dips mapped are listed in the test mode */

INPUT_PORTS_START( pgm )
	PORT_START("P1P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("P3P4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(4)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)

	PORT_START("Service")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 )
//  PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 )  // test 1p+2p
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )  // what should i use?
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 ) // service 1p+2p
//  PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON6 )  // test 3p+4p
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )  // what should i use?
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SERVICE2 ) // service 3p+4p
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )  // unused?
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )  // unused?
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )  // unused?
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )  // unused?

	PORT_START("DSW")
	PORT_SERVICE( 0x0001, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0002, 0x0002, "Music" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Voice" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Free" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Stop" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("Region")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/*** GFX Decodes *************************************************************/

/* We can't decode the sprite data like this because it isn't tile based.
   Note that the bit indexes are reversed compared to usual gfx layouts
   (0-7 = LSB to MSB) */

static const gfx_layout pgm8_charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 3, 2, 1, 0 },
	{ 0, 4, 8, 12, 16, 20, 24, 28 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};

static const gfx_layout pgm32_charlayout =
{
	32,32,
	RGN_FRAC(1,1),
	5,
	{ 4, 3, 2, 1, 0 },
	{ 0, 5, 10, 15, 20, 25, 30, 35,
		40, 45, 50, 55, 60, 65, 70, 75,
		80, 85, 90, 95, 100, 105, 110, 115,
		120, 125, 130, 135, 140, 145, 150, 155 },
	{ 0*160, 1*160, 2*160, 3*160, 4*160, 5*160, 6*160, 7*160,
		8*160, 9*160,10*160,11*160,12*160,13*160,14*160,15*160,
		16*160,17*160,18*160,19*160,20*160,21*160,22*160,23*160,
		24*160,25*160,26*160,27*160,28*160,29*160,30*160,31*160 },
		32*160
};

GFXDECODE_START( pgm )
	GFXDECODE_REVERSEBITS( "tiles", 0, pgm8_charlayout,    0x800, 32  ) /* 8x8x4 Tiles */
	GFXDECODE_REVERSEBITS( "tiles", 0, pgm32_charlayout,   0x400, 32  ) /* 32x32x5 Tiles */
GFXDECODE_END

/*** Machine Driver **********************************************************/

/* most games require IRQ4 for inputs to work, Puzzli 2 is explicit about not wanting it tho
   what is the source? */
TIMER_DEVICE_CALLBACK_MEMBER(pgm_state::pgm_interrupt)
{
	int scanline = param;

// already being generated  by MCFG_CPU_VBLANK_INT_DRIVER("screen", pgm_state,  irq6_line_hold)
//  if(scanline == 224)
//      m_maincpu->set_input_line(6, HOLD_LINE);

	if(scanline == 0)
		if (!m_irq4_disabled) m_maincpu->set_input_line(4, HOLD_LINE);
}

MACHINE_START_MEMBER(pgm_state,pgm)
{
//  machine().base_datetime(m_systime);

	m_ics = machine().device("ics");
}

MACHINE_RESET_MEMBER(pgm_state,pgm)
{
	m_soundcpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

MACHINE_CONFIG_FRAGMENT( pgmbase )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 20000000) /* 20 mhz! verified on real board */
	MCFG_CPU_PROGRAM_MAP(pgm_basic_mem)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", pgm_state,  irq6_line_hold)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", pgm_state, pgm_interrupt, "screen", 0, 1)

	MCFG_CPU_ADD("soundcpu", Z80, 33868800/4)
	MCFG_CPU_PROGRAM_MAP(pgm_z80_mem)
	MCFG_CPU_IO_MAP(pgm_z80_io)

	MCFG_MACHINE_START_OVERRIDE(pgm_state, pgm )
	MCFG_MACHINE_RESET_OVERRIDE(pgm_state, pgm )
	MCFG_NVRAM_ADD_0FILL("sram")

	MCFG_V3021_ADD("rtc")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60) // killing blade won't boot (just displays 'error') if this is lower than 59.9 or higher than 60.1 .. are actual PGM boards different to the Cave one?
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 56*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(pgm_state, screen_update_pgm)
	MCFG_SCREEN_VBLANK_DRIVER(pgm_state, screen_eof_pgm)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pgm)
	MCFG_PALETTE_ADD("palette", 0x1200/2)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)

	MCFG_VIDEO_START_OVERRIDE(pgm_state,pgm)

	/*sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_ICS2115_ADD("ics", 0)
	MCFG_ICS2115_IRQ_CB(WRITELINE(pgm_state, pgm_sound_irq))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 5.0)
MACHINE_CONFIG_END

MACHINE_CONFIG_START( pgm, pgm_state )
	MCFG_FRAGMENT_ADD(pgmbase)
MACHINE_CONFIG_END


/*** Rom Loading *************************************************************/

/* take note of "sprmask" needed for expanding the Sprite Colour Data */

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(bios+1)) /* Note '+1' */

#define PGM_68K_BIOS \
	ROM_SYSTEM_BIOS( 0, "v2",     "PGM Bios V2" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "pgm_p02s.u20",    0x00000, 0x020000, CRC(78c15fa2) SHA1(885a6558e022602cc6f482ac9667ba9f61e75092) ) /* Version 2 (Label: IGS | PGM P02S | 1P0792D1 | J992438 )*/ \
	ROM_SYSTEM_BIOS( 1, "v1",     "PGM Bios V1" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 1, "pgm_p01s.u20",    0x00000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) ) /* Version 1 */
#define PGM_AUDIO_BIOS \
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) )
#define PGM_VIDEO_BIOS \
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) )
/* The Bios - NOT A GAME */
ROM_START( pgm )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS

	ROM_REGION( 0x200000, "tiles", 0 ) /* 8x8 Text Layer Tiles */
	PGM_VIDEO_BIOS

	ROM_REGION( 0x200000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS

	ROM_REGION( 0x1000000, "sprcol", ROMREGION_ERASEFF ) /* Sprite Colour Data */
	ROM_REGION( 0x1000000, "sprmask", ROMREGION_ERASEFF ) /* Sprite Masks + Colour Indexes */
ROM_END

ROM_START( orlegend )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0103.rom",    0x100000, 0x200000, CRC(d5e93543) SHA1(f081edc26514ca8354c13c7f6f89aba8e4d3e7d2) )

	ROM_REGION( 0x800000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0100.u8",     0x180000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0100.u5",     0x0000000, 0x400000, CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "a0101.u6",     0x0400000, 0x400000, CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "a0102.u7",     0x0800000, 0x400000, CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "a0103.u8",     0x0c00000, 0x400000, CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "a0104.u11",    0x1000000, 0x400000, CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "a0105.u12",    0x1400000, 0x400000, CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0100.u9",     0x0000000, 0x400000, CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "b0101.u10",    0x0400000, 0x400000, CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "b0102.u15",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0100.u1",     0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END

ROM_START( orlegende )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0102.rom",    0x100000, 0x200000, CRC(4d0f6cc5) SHA1(8d41f0a712fb11a1da865f5159e5e27447b4388a) )

	ROM_REGION( 0x800000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0100.u8",     0x180000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0100.u5",     0x0000000, 0x400000, CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "a0101.u6",     0x0400000, 0x400000, CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "a0102.u7",     0x0800000, 0x400000, CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "a0103.u8",     0x0c00000, 0x400000, CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "a0104.u11",    0x1000000, 0x400000, CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "a0105.u12",    0x1400000, 0x400000, CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0100.u9",     0x0000000, 0x400000, CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "b0101.u10",    0x0400000, 0x400000, CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "b0102.u15",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0100.u1",     0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END

ROM_START( orlegendc )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0101.160",    0x100000, 0x200000, CRC(b24f0c1e) SHA1(a2cf75d739681f091c24ef78ed6fc13aa8cfe0c6) )

	ROM_REGION( 0x800000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0100.u8",     0x180000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0100.u5",     0x0000000, 0x400000, CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "a0101.u6",     0x0400000, 0x400000, CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "a0102.u7",     0x0800000, 0x400000, CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "a0103.u8",     0x0c00000, 0x400000, CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "a0104.u11",    0x1000000, 0x400000, CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "a0105.u12",    0x1400000, 0x400000, CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0100.u9",     0x0000000, 0x400000, CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "b0101.u10",    0x0400000, 0x400000, CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "b0102.u15",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0100.u1",     0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END

ROM_START( orlegendca )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0101.102",    0x100000, 0x200000, CRC(7a22e1cb) SHA1(4fe0fde00521b0915146334ea7213f3eb7e2affc) )

	ROM_REGION( 0x800000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0100.u8",     0x180000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0100.u5",     0x0000000, 0x400000, CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "a0101.u6",     0x0400000, 0x400000, CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "a0102.u7",     0x0800000, 0x400000, CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "a0103.u8",     0x0c00000, 0x400000, CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "a0104.u11",    0x1000000, 0x400000, CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "a0105.u12",    0x1400000, 0x400000, CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0100.u9",     0x0000000, 0x400000, CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "b0101.u10",    0x0400000, 0x400000, CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "b0102.u15",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0100.u1",     0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END


/*

Oriental Legend / Xi You Shi E Zhuan (CHINA 111 Ver.)
(c)1997 IGS

PGM system
IGS PCB NO-0134-1
IGS PCB NO-0135

OLV111CH.U11 [b80ddd3c]
OLV111CH.U6  [5fb86373]
OLV111CH.U7  [6ee79faf]
OLV111CH.U9  [83cf09c8]

T0100.U8

A0100.U5
A0101.U6
A0102.U7
A0103.U8
A0104.U11
A0105.U12

B0100.U9
B0101.U10
B0102.U15

M0100.U1

*/

ROM_START( orlegend111c )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "olv111ch.u6",     0x100001, 0x080000, CRC(5fb86373) SHA1(2fc58eff1f38754c75819fde666244b867ca4f05) )
	ROM_LOAD16_BYTE( "olv111ch.u9",     0x100000, 0x080000, CRC(83cf09c8) SHA1(959780b45326059517f3008a356657f4f3d2908f) )
	ROM_LOAD16_BYTE( "olv111ch.u7",     0x200001, 0x080000, CRC(6ee79faf) SHA1(039b4b07b8577f0d3022ae01210c00375624cb3c) )
	ROM_LOAD16_BYTE( "olv111ch.u11",    0x200000, 0x080000, CRC(b80ddd3c) SHA1(55c700ce71ffdee392e03fd9d4719542c3527132) )

	ROM_REGION( 0x800000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0100.u8",     0x180000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0100.u5",     0x0000000, 0x400000, CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "a0101.u6",     0x0400000, 0x400000, CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "a0102.u7",     0x0800000, 0x400000, CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "a0103.u8",     0x0c00000, 0x400000, CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "a0104.u11",    0x1000000, 0x400000, CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "a0105.u12",    0x1400000, 0x400000, CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0100.u9",     0x0000000, 0x400000, CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "b0101.u10",    0x0400000, 0x400000, CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "b0102.u15",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0100.u1",     0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END


ROM_START( orlegend111t )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "olv111tw.u6",     0x100001, 0x080000, CRC(b205a733) SHA1(33f4c9162e36be4957004f80593f94fc33b163f8) )
	ROM_LOAD16_BYTE( "olv111tw.u9",     0x100000, 0x080000, CRC(6d9d29b4) SHA1(29a18de7e5b58c2f3125d6dc9cc8a8186180e956) )
	ROM_LOAD16_BYTE( "olv111tw.u7",     0x200001, 0x080000, CRC(27628e87) SHA1(a0effd83dc57ac72ba4f110737a075705d78e798) )
	ROM_LOAD16_BYTE( "olv111tw.u11",    0x200000, 0x080000, CRC(23f33bc9) SHA1(f24490370d40d905afe8b716a3953b4e9f0aada4) )

	ROM_REGION( 0x800000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0100.u8",     0x180000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0100.u5",     0x0000000, 0x400000, CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "a0101.u6",     0x0400000, 0x400000, CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "a0102.u7",     0x0800000, 0x400000, CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "a0103.u8",     0x0c00000, 0x400000, CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "a0104.u11",    0x1000000, 0x400000, CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "a0105.u12",    0x1400000, 0x400000, CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0100.u9",     0x0000000, 0x400000, CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "b0101.u10",    0x0400000, 0x400000, CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "b0102.u15",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0100.u1",     0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END

ROM_START( orlegend111k )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "olv111ko.u6",     0x100001, 0x080000, CRC(1ff35baa) SHA1(f6791cc37ea468d0154b0e31a99f6f33a74bca81) )
	ROM_LOAD16_BYTE( "olv111ko.u9",     0x100000, 0x080000, CRC(87b6d202) SHA1(32828166a645158630f79e7d493a2774e69bc265) )
	ROM_LOAD16_BYTE( "olv111ko.u7",     0x200001, 0x080000, CRC(27628e87) SHA1(a0effd83dc57ac72ba4f110737a075705d78e798) )
	ROM_LOAD16_BYTE( "olv111ko.u11",    0x200000, 0x080000, CRC(23f33bc9) SHA1(f24490370d40d905afe8b716a3953b4e9f0aada4) )

	ROM_REGION( 0x800000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0100.u8",     0x180000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0100.u5",     0x0000000, 0x400000, CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "a0101.u6",     0x0400000, 0x400000, CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "a0102.u7",     0x0800000, 0x400000, CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "a0103.u8",     0x0c00000, 0x400000, CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "a0104.u11",    0x1000000, 0x400000, CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "a0105.u12",    0x1400000, 0x400000, CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0100.u9",     0x0000000, 0x400000, CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "b0101.u10",    0x0400000, 0x400000, CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "b0102.u15",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0100.u1",     0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END

/*

Oriental Legend / Xi You Shi E Zhuan (KOREA 105 Ver.)
(c)1997 IGS

PGM system
IGS PCB NO-0134-2
IGS PCB NO-0135

OLV105KO.U11 [40ae4d9e]
OLV105KO.U6  [b86703fe]
OLV105KO.U7  [5712facc]
OLV105KO.U9  [5a108e39]

T0100.U8

A0100.U5
A0101.U6
A0102.U7
A0103.U8
A0104.U11
A0105.U12

B0100.U9
B0101.U10
B0102.U15

M0100.U1

*/

ROM_START( orlegend105k )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "olv105ko.u6",     0x100001, 0x080000, CRC(b86703fe) SHA1(a3529b45efd400ecd5e76f764b528ebce46e24ab) )
	ROM_LOAD16_BYTE( "olv105ko.u9",     0x100000, 0x080000, CRC(5a108e39) SHA1(2033f4fe3f2dfd725dac535324f58348b9ac3914) )
	ROM_LOAD16_BYTE( "olv105ko.u7",     0x200001, 0x080000, CRC(5712facc) SHA1(2d95ebd1703874e89ac3a206f8c1f0ece6e833e0) )
	ROM_LOAD16_BYTE( "olv105ko.u11",    0x200000, 0x080000, CRC(40ae4d9e) SHA1(62d7a96438b7fe93f74753333f50e077d417971e) )

	ROM_REGION( 0x800000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0100.u8",     0x180000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0100.u5",     0x0000000, 0x400000, CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "a0101.u6",     0x0400000, 0x400000, CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "a0102.u7",     0x0800000, 0x400000, CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "a0103.u8",     0x0c00000, 0x400000, CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "a0104.u11",    0x1000000, 0x400000, CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "a0105.u12",    0x1400000, 0x400000, CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0100.u9",     0x0000000, 0x400000, CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "b0101.u10",    0x0400000, 0x400000, CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "b0102.u15",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0100.u1",     0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END

/*

Dragon World 2 (English / World Version)
IGS, 1997

This is a cart for the IGS PGM system.


PCB Layout
----------

IGS PCB NO-0162
|-----------------------------------------------|
| |------|                                      |
| |IGS012|       *1                    T0200.U7 |
| |      |                                      |
| |------|                                      |
|              |--------|                       |
|              |        |                       |
|              | IGS025 |  *2   V-110X.U2       |
| PAL    PAL   |        |                  PAL  |
|              |--------|                       |
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      IGS012       - Custom IGS IC (QFP80)

      -- on english version
      IGS025       - Custom IGS IC (PLCC68, labelled "DRAGON II 0006")
      -- on china version
      IGS025       - Custom IGS IC (PLCC68, labelled "DRAGON II 0005")


      T0200.U7     - 32MBit MaskROM (SOP44)

      -- on english version
      V-110X.U2    - AM27C4096 4MBit EPROM (DIP42, labelled "DRAGON II V-110X")
      -- on china version
      V-110X.U2    - AM27C4096 4MBit EPROM (DIP42, labelled "DRAGON II V-100C")

      PALs         - x3, labelled "CZ U3", "CZ U4", "CZ U6"
      *1           - Unpopulated position for MX23C4100 SOP40 MASKROM
      *2           - Unpopulated position for MX23C4100 DIP40 EPROM/MASKROM


IGS PCB NO-0135
|-----------------------------------------------|
| U11    U12     U13      U14       U15      U16|
|                                               |
|                                               |
|A0200.U5                       B0200.U9        |
|        U6      U7       U8                 U10|
|                                               |
|                                               |
|74LS138         U1       U2             74LS139|
|                                               |
|-|                                           |-|
  |--------------------||---------------------|

Notes:
      This PCB contains only SOP44 MASKROMS and 2 logic IC's
      Only U5 and U9 are populated

      glitch on select screen exists on real board.

*/

ROM_START( drgw2 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v-110x.u2",    0x100000, 0x080000, CRC(1978106b) SHA1(af8a13d7783b755a58762c98bdc32cab845b2251) )

	ROM_REGION( 0x800000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgmt0200.u7",    0x180000, 0x400000, CRC(b0f6534d) SHA1(174cacd81169a0e0d14790ac06d03caed737e05d) )

	ROM_REGION( 0x400000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgma0200.u5",    0x0000000, 0x400000, CRC(13b95069) SHA1(4888b06002afb18eab81c010e9362629045767af) )

	ROM_REGION( 0x400000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgmb0200.u9",    0x0000000, 0x400000, CRC(932d0f13) SHA1(4b8e008f9c617cb2b95effeb81abc065b30e5c86) )

	ROM_REGION( 0x400000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
ROM_END

ROM_START( dw2v100x )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "dragonv100x.bin",    0x100000, 0x080000,  CRC(5e71851d) SHA1(62052469f69daec88efd26652c1b893d6f981912) )

	ROM_REGION( 0x800000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgmt0200.u7",    0x180000, 0x400000, CRC(b0f6534d) SHA1(174cacd81169a0e0d14790ac06d03caed737e05d) )

	ROM_REGION( 0x400000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgma0200.u5",    0x0000000, 0x400000, CRC(13b95069) SHA1(4888b06002afb18eab81c010e9362629045767af) )

	ROM_REGION( 0x400000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgmb0200.u9",    0x0000000, 0x400000, CRC(932d0f13) SHA1(4b8e008f9c617cb2b95effeb81abc065b30e5c86) )

	ROM_REGION( 0x400000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
ROM_END

ROM_START( drgw2c )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v-100c.u2",    0x100000, 0x080000, CRC(67467981) SHA1(58af01a3871b6179fe42ff471cc39a2161940043) )

	ROM_REGION( 0x800000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgmt0200.u7",    0x180000, 0x400000, CRC(b0f6534d) SHA1(174cacd81169a0e0d14790ac06d03caed737e05d) )

	ROM_REGION( 0x400000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgma0200.u5",    0x0000000, 0x400000, CRC(13b95069) SHA1(4888b06002afb18eab81c010e9362629045767af) )

	ROM_REGION( 0x400000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgmb0200.u9",    0x0000000, 0x400000, CRC(932d0f13) SHA1(4b8e008f9c617cb2b95effeb81abc065b30e5c86) )

	ROM_REGION( 0x400000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
ROM_END

ROM_START( drgw2j )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v-100j.u2",    0x100000, 0x080000, CRC(f8f8393e) SHA1(ef0db668b4e4f661d4c1e95d57afe881bcdf13cc) )

	ROM_REGION( 0x800000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgmt0200.u7",    0x180000, 0x400000, CRC(b0f6534d) SHA1(174cacd81169a0e0d14790ac06d03caed737e05d) )

	ROM_REGION( 0x400000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgma0200.u5",    0x0000000, 0x400000, CRC(13b95069) SHA1(4888b06002afb18eab81c010e9362629045767af) )

	ROM_REGION( 0x400000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgmb0200.u9",    0x0000000, 0x400000, CRC(932d0f13) SHA1(4b8e008f9c617cb2b95effeb81abc065b30e5c86) )

	ROM_REGION( 0x400000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
ROM_END

ROM_START( drgw2hk ) // the IGS025 has a "DRAGON-II 0004-1" sticker, the IGS012 has no per-game marking
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "dragon_ii_v-100-h.u2",    0x100000, 0x080000, CRC(c6e2e6ec) SHA1(84145dfb26857ea20efb233363f175bc9bb25b0c) )

	ROM_REGION( 0x800000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgmt0200.u7",    0x180000, 0x400000, CRC(b0f6534d) SHA1(174cacd81169a0e0d14790ac06d03caed737e05d) )

	ROM_REGION( 0x400000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgma0200.u5",    0x0000000, 0x400000, CRC(13b95069) SHA1(4888b06002afb18eab81c010e9362629045767af) )

	ROM_REGION( 0x400000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgmb0200.u9",    0x0000000, 0x400000, CRC(932d0f13) SHA1(4b8e008f9c617cb2b95effeb81abc065b30e5c86) )

	ROM_REGION( 0x400000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
ROM_END

/*

Dragon World 3 (KOREA 106 Ver.)
(c)1998 IGS

PGM system
IGS PCB NO-0189
IGS PCB NO-0178


DW3_V106.U12 [c3f6838b]
DW3_V106.U13 [28284e22]


*/

ROM_START( drgw3 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "dw3_v106.u12",     0x100001, 0x080000,  CRC(c3f6838b) SHA1(c135b1d4dd62af308139d40d03c29be7508fb1e7) )
	ROM_LOAD16_BYTE( "dw3_v106.u13",     0x100000, 0x080000,  CRC(28284e22) SHA1(4643a69881ddb7383ca10f3eb2aa2cf41be39e9f) )

	ROM_REGION( 0x010000, "igs022data", 0 ) /* Protection Data - is it correct for this set? */
	ROM_LOAD( "dw3_v100.u15", 0x000000, 0x010000, CRC(03dc4fdf) SHA1(b329b04325d4f725231b1bb7862eedef2319b652) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "dw3t0400.u18",   0x180000, 0x400000, CRC(b70f3357) SHA1(8733969d7d21f540f295a9f747a4bb8f0d325cf0) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "dw3a0400.u9",     0x0000000, 0x400000, CRC(dd7bfd40) SHA1(fb7ec5bf89a413c5208716083762a725ff63f5db) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "dw3a0401.u10",    0x0400000, 0x400000, CRC(cab6557f) SHA1(1904dd86645eea27ac1ab8a2462b20f6531356f8) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "dw3b0400.u13",    0x0000000, 0x400000,  CRC(4bb87cc0) SHA1(71b2dc43fd11f7a6dffaba501e4e344b843583d8) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "dw3m0400.u1",  0x400000, 0x400000, CRC(031eb9ce) SHA1(0673ec194732becc6648c2ae1396e894aa269f9a) )
ROM_END


ROM_START( drgw3105 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "dw3_v105.u12",     0x100001, 0x080000,  CRC(c5e24318) SHA1(c6954495bbc72c3985df75aecf6afd6826c8e30e) )
	ROM_LOAD16_BYTE( "dw3_v105.u13",     0x100000, 0x080000,  CRC(8d6c9d39) SHA1(cb79303ab551e91f07e11414db4254d5b161d415) )

	ROM_REGION( 0x010000, "igs022data", 0 ) /* Protection Data - is it correct for this set? */
	ROM_LOAD( "dw3_v100.u15", 0x000000, 0x010000, CRC(03dc4fdf) SHA1(b329b04325d4f725231b1bb7862eedef2319b652) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "dw3t0400.u18",   0x180000, 0x400000, CRC(b70f3357) SHA1(8733969d7d21f540f295a9f747a4bb8f0d325cf0) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "dw3a0400.u9",     0x0000000, 0x400000, CRC(dd7bfd40) SHA1(fb7ec5bf89a413c5208716083762a725ff63f5db) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "dw3a0401.u10",    0x0400000, 0x400000, CRC(cab6557f) SHA1(1904dd86645eea27ac1ab8a2462b20f6531356f8) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "dw3b0400.u13",    0x0000000, 0x400000,  CRC(4bb87cc0) SHA1(71b2dc43fd11f7a6dffaba501e4e344b843583d8) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "dw3m0400.u1",  0x400000, 0x400000, CRC(031eb9ce) SHA1(0673ec194732becc6648c2ae1396e894aa269f9a) )
ROM_END

ROM_START( drgw3103 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "dw3_v103j.u12",     0x100001, 0x080000,  CRC(275b39a2) SHA1(8ba4d2601734c2dda3d4269fbe8f543dc3f0b212) )
	ROM_LOAD16_BYTE( "dw3_v103j.u13",     0x100000, 0x080000,  CRC(9aa56e8f) SHA1(c3f27d8b59adf72040a2e2c11e34f9b07efd7e9e) )

	ROM_REGION( 0x010000, "igs022data", 0 ) /* Protection Data */
	ROM_LOAD( "dw3_v100.u15", 0x000000, 0x010000, CRC(03dc4fdf) SHA1(b329b04325d4f725231b1bb7862eedef2319b652) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "dw3t0400.u18",   0x180000, 0x400000, CRC(b70f3357) SHA1(8733969d7d21f540f295a9f747a4bb8f0d325cf0) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "dw3a0400.u9",     0x0000000, 0x400000, CRC(dd7bfd40) SHA1(fb7ec5bf89a413c5208716083762a725ff63f5db) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "dw3a0401.u10",    0x0400000, 0x400000, CRC(cab6557f) SHA1(1904dd86645eea27ac1ab8a2462b20f6531356f8) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "dw3b0400.u13",    0x0000000, 0x400000,  CRC(4bb87cc0) SHA1(71b2dc43fd11f7a6dffaba501e4e344b843583d8) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "dw3m0400.u1",  0x400000, 0x400000, CRC(031eb9ce) SHA1(0673ec194732becc6648c2ae1396e894aa269f9a) )
ROM_END
/*

Dragon World 3
Alta Co./IGS, 1998

Cart for IGS PGM system

Top board of cart contains.....
8MHz Xtal
32.768kHz Xtal
UM6164 (RAM x 2)
MACH211 CPLD
IGS022 ASIC
IGS025 ASIC
1x PAL
2x 27C040 EPROMs (main 68k program)
1x 27C512 EPROM (protection code?)
1x 32MBit smt MASKROM (T0400)

Bottom board contains.....
4x 32MBit smt MASKROMs (A0400, A0401, B0400, M0400)

*/

ROM_START( drgw3100 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "dw3_v100.u12",     0x100001, 0x080000,  CRC(47243906) SHA1(9cd46e3cba97f049bcb238ceb6edf27a760ef831) )
	ROM_LOAD16_BYTE( "dw3_v100.u13",     0x100000, 0x080000,  CRC(b7cded21) SHA1(c1ae2af2e42227503c81bbcd2bd6862aa416bd78) )

	ROM_REGION( 0x010000, "igs022data", 0 ) /* Protection Data */
	ROM_LOAD( "dw3_v100.u15", 0x000000, 0x010000, CRC(03dc4fdf) SHA1(b329b04325d4f725231b1bb7862eedef2319b652) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "dw3t0400.u18",   0x180000, 0x400000, CRC(b70f3357) SHA1(8733969d7d21f540f295a9f747a4bb8f0d325cf0) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "dw3a0400.u9",     0x0000000, 0x400000, CRC(dd7bfd40) SHA1(fb7ec5bf89a413c5208716083762a725ff63f5db) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "dw3a0401.u10",    0x0400000, 0x400000, CRC(cab6557f) SHA1(1904dd86645eea27ac1ab8a2462b20f6531356f8) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "dw3b0400.u13",    0x0000000, 0x400000,  CRC(4bb87cc0) SHA1(71b2dc43fd11f7a6dffaba501e4e344b843583d8) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "dw3m0400.u1",  0x400000, 0x400000, CRC(031eb9ce) SHA1(0673ec194732becc6648c2ae1396e894aa269f9a) )
ROM_END

/*

Dragon World EX IGS PGM cart

Top board
---------
PCB Number: IGS PCB-0189-1
8MHz XTAL
2x 27C040 EPROMs at U12, U13
27C512 EPROM at U15
PAL at U17
2x 6164 SRAM at U1, U4
MACH211 at U11
IGS022 at U14
IGS025 at U16
16M SOP44 mask ROM at U18

Bottom Board
------------
PCB Number: IGS PCB-0178
2x 16M mask ROMs at U1, U10
2x 32M mask ROMs at U9, U13

*/

// seems to be an updated version of dw3, most roms are the same, but it's a sequel, not a clone.
// the non-program roms that differ are actually the same, but in the dw3 sets they're double sized with duplicate data (overdumped, or just different roms used on pcb?)
ROM_START( dwex )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "ex_v100.u12",     0x100001, 0x080000, CRC(bc171799) SHA1(142329dffbca199f3e748a52146a03e27b36db6a) )
	ROM_LOAD16_BYTE( "ex_v100.u13",     0x100000, 0x080000, CRC(7afe6322) SHA1(a52d71af1d6de16c5a3df23eacdab3466693ba8d) )

	ROM_REGION( 0x010000, "igs022data", 0 ) /* Protection Data */
	ROM_LOAD( "ex_data.u15", 0x000000, 0x010000, CRC(03dc4fdf) SHA1(b329b04325d4f725231b1bb7862eedef2319b652) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "ex_t0400.u18", 0x180000, 0x200000, CRC(9ecc950d) SHA1(fd97f43818a3eb18254636166871fa09bd0d6c07) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "ex_a0400.u9",     0x0000000, 0x400000, CRC(dd7bfd40) SHA1(fb7ec5bf89a413c5208716083762a725ff63f5db) )
	ROM_LOAD( "ex_a0401.u10",    0x0400000, 0x200000, CRC(d36c06a4) SHA1(f192e8bfdfbe3d82a49d8f0d3cb0603e39719773) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "ex_b0400.u13",    0x0000000, 0x400000,  CRC(4bb87cc0) SHA1(71b2dc43fd11f7a6dffaba501e4e344b843583d8) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "ex_m0400.u1",  0x400000, 0x200000, CRC(42d54fd5) SHA1(ad915b514aa6cae6f72dea78e6208f40b08ceac0) )
ROM_END


ROM_START( kov )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0600.117",    0x100000, 0x400000, CRC(c4d19fe6) SHA1(14ef31539bfbc665e76c9703ee01b12228344052) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0600.rom",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0603.rom",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0601.rom",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kov115 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0600.115",    0x100000, 0x400000, CRC(527a2924) SHA1(7e3b166dddc5245d7b408e78437c16fd2986d1d9) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0600.rom",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0603.rom",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0601.rom",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

/*

Sangoku Senki / Knights of Valour (JPN 100 Ver.)
(c)1999 ALTA / IGS

PGM system
IGS PCB NO-0212-1
IGS PCB NO-0213T


SAV111.U10   [d5536107]
SAV111.U4    [ae2f1b4e]
SAV111.U5    [5fdd4aa8]
SAV111.U7    [95eedf0e]
SAV111.U8    [003cbf49]

T0600.U11


A0600.U2
A0601.U4
A0602.U6
A0603.U9

M0600.U3

B0600.U5
B0601.U7

*/

ROM_START( kov100 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "sav111.u4",      0x100001, 0x080000, CRC(ae2f1b4e) SHA1(2ac9d84f5dee52f374941cfd68e2b98ecad436a8) )
	ROM_LOAD16_BYTE( "sav111.u7",      0x100000, 0x080000, CRC(95eedf0e) SHA1(582a54e9a1eda7ff73e20f0e69d2d50141772378) )
	ROM_LOAD16_BYTE( "sav111.u5",      0x200001, 0x080000, CRC(5fdd4aa8) SHA1(43c96e21ad4f11148e1e94a59c53780b2edd43ba) )
	ROM_LOAD16_BYTE( "sav111.u8",      0x200000, 0x080000, CRC(003cbf49) SHA1(fb5bea47ecae025b1b425af52cd05e061f45e377) )
	ROM_LOAD16_WORD_SWAP( "sav111.u10",0x300000, 0x080000, CRC(d5536107) SHA1(f963e015d99c1621323eecf63e773c0b9f4b6a43) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0600.rom",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0603.rom",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0601.rom",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kovplus )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0600.119",    0x100000, 0x400000, CRC(e4b0875d) SHA1(e8382e131b0e431406dc2a05cc1ef128302d987c) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0600.rom",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0603.rom",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0601.rom",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END


/*

Sangoku Senki Plus / Knights of Valour Plus (Alt 119 Ver.)
(c)1999 IGS

PGM system
IGS PCB NO-0222
IGS PCB NO-0213


V119.U2      [29588ef2]
V119.U3      [6750388f]
V119.U4      [8200ece6]
V119.U5      [d4101ffd]
V119.U6      [71e28f27]

T0600.U11


A0600.U2
A0601.U4
A0602.U6
A0603.U9

M0600.U3

B0600.U5
B0601.U7

*/

ROM_START( kovplusa )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "v119.u3",     0x100001, 0x080000, CRC(6750388f) SHA1(869f4ad27f2992cc62baa9a78bf7984a43ec4cc5) )
	ROM_LOAD16_BYTE( "v119.u5",     0x100000, 0x080000, CRC(d4101ffd) SHA1(a327fd56eec65b07df9305cd93ef2c46bf8e40f3) )
	ROM_LOAD16_BYTE( "v119.u4",     0x200001, 0x080000, CRC(8200ece6) SHA1(97081d2e8aed2ac6fbe5951890aecea18af5ce2e) )
	ROM_LOAD16_BYTE( "v119.u6",     0x200000, 0x080000, CRC(71e28f27) SHA1(db382807e9185f0dc17124f210165fa1b36ca6ac) )
	ROM_LOAD16_WORD_SWAP( "v119.u2",0x300000, 0x080000, CRC(29588ef2) SHA1(17d1a308d44434cf65224a24360cf4b6e32d28f3) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0600.rom",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0603.rom",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0601.rom",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kovsgqyz )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "pgm_sgqyz.rom",    0x100000, 0x400000, CRC(18e1eed9) SHA1(db18d9121bb533140957e9c58dbc38211d164b01) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsgqyz_igs027a.bin", 0x000000, 0x04000, NO_DUMP ) // bootleg is probably a different device

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0600.rom",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION( 0x2000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "qyza0603.rom", 0x1800000, 0x0800000, CRC(c8b92220) SHA1(4f9c43970d92ac8a8f1563021022797ae8e32012) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "qyzb0601.rom", 0x0800000, 0x0800000, CRC(64f55882) SHA1(ab9ac1396587c3d78d06f6ec83cab61d6a9faacd) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kovsgqyza )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "pgm_sgqyza.rom",    0x100000, 0x400000, CRC(5a30dcb7) SHA1(64f34faf99a19c0a54899990695129c512d5a3c8) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsgqyza_igs027a.bin", 0x000000, 0x04000, NO_DUMP ) // bootleg is probably a different device

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0600.rom",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION( 0x2000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "qyza0603.rom", 0x1800000, 0x0800000, CRC(c8b92220) SHA1(4f9c43970d92ac8a8f1563021022797ae8e32012) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "qyzb0601.rom", 0x0800000, 0x0800000, CRC(64f55882) SHA1(ab9ac1396587c3d78d06f6ec83cab61d6a9faacd) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kovsgqyzb )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "pgm_sgqyzb.rom",    0x100000, 0x400000, CRC(18b8b9c0) SHA1(f4937aa21cd11af16fb50e7a75c8d4c4ed27c5cf) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsgqyzb_igs027a.bin", 0x000000, 0x04000, NO_DUMP ) // bootleg is probably a different device

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0600.rom",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION( 0x2000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "qyza0603.rom", 0x1800000, 0x0800000, CRC(c8b92220) SHA1(4f9c43970d92ac8a8f1563021022797ae8e32012) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "qyzb0601.rom", 0x0800000, 0x0800000, CRC(64f55882) SHA1(ab9ac1396587c3d78d06f6ec83cab61d6a9faacd) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END


ROM_START( kovsh )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0600.322",    0x100000, 0x400000, CRC(7c78e5f3) SHA1(9b1e4bd63fb1294ebeb539966842273c8dc7683b) )
	//ROM_LOAD16_WORD_SWAP( "kovsh-v0104-u1.bin",    0x100000, 0x400000, CRC(4e2ba39b) SHA1(f3b5aa6f45cfd5a7f1e2a2e893d1652a3f23d6b8) ) // identical but the last 1MB is filled with 0xff instead of 0x00

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsh_v100_china.asic", 0x000000, 0x04000,  CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0600.rom",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	/* all roms below need checking to see if they're the same on this board */
	ROM_REGION( 0x1e00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0603.rom",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )
	ROM_LOAD( "a0604.rom",    0x1a00000, 0x0400000, CRC(26b59fd3) SHA1(53219376056f4766dc5236735599d982ceb56b84) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0601.rom",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )
	ROM_LOAD( "b0602.rom",    0x0c00000, 0x0100000, CRC(9df77934) SHA1(99a3fe337c13702c9aa2373bcd1bb1befd0e2a13) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kovsh103 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0600.103",    0x100000, 0x400000, CRC(f0b3da82) SHA1(4067beb69c049b51bce6154f4cf880600ca4de11) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsh_v100_china.asic", 0x000000, 0x04000,  CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0600.rom",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	/* all roms below need checking to see if they're the same on this board */
	ROM_REGION( 0x1e00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0603.rom",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )
	ROM_LOAD( "a0604.rom",    0x1a00000, 0x0400000, CRC(26b59fd3) SHA1(53219376056f4766dc5236735599d982ceb56b84) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0601.rom",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )
	ROM_LOAD( "b0602.rom",    0x0c00000, 0x0100000, CRC(9df77934) SHA1(99a3fe337c13702c9aa2373bcd1bb1befd0e2a13) ) // this dump had the same rom 4x bigger but with the data duplicated 4x, which is correct?

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kovsh102 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0600.102",    0x100000, 0x400000, CRC(fdd4fb0f) SHA1(6906ce68f37b82e52ba30e5cceb1304d9e01430a) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsh_v100_china.asic", 0x000000, 0x04000,  CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0600.rom",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	/* all roms below need checking to see if they're the same on this board */
	ROM_REGION( 0x1e00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0603.rom",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )
	ROM_LOAD( "a0604.rom",    0x1a00000, 0x0400000, CRC(26b59fd3) SHA1(53219376056f4766dc5236735599d982ceb56b84) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0601.rom",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )
	ROM_LOAD( "b0602.rom",    0x0c00000, 0x0100000, CRC(9df77934) SHA1(99a3fe337c13702c9aa2373bcd1bb1befd0e2a13) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kovsh101 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0600.101",    0x100000, 0x400000, CRC(517a9bcf) SHA1(1ee0333aee2a7569e15bb2a1be8dd03f8b08e08c) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsh_v100_china.asic", 0x000000, 0x04000,  CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0600.rom",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	/* all roms below need checking to see if they're the same on this board */
	ROM_REGION( 0x1e00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0603.rom",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )
	ROM_LOAD( "a0604.rom",    0x1a00000, 0x0400000, CRC(26b59fd3) SHA1(53219376056f4766dc5236735599d982ceb56b84) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0601.rom",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )
	ROM_LOAD( "b0602.rom",    0x0c00000, 0x0100000, CRC(9df77934) SHA1(99a3fe337c13702c9aa2373bcd1bb1befd0e2a13) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kovsh100 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "kovsh-v0100-u1.bin",    0x100000, 0x400000, CRC(d145c1ca) SHA1(ce4da36791bc5eea9fe1ef6db180d789bab0bab7) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsh_v100_china.asic", 0x000000, 0x04000,  CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0600.rom",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	/* all roms below need checking to see if they're the same on this board */
	ROM_REGION( 0x1e00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0603.rom",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )
	ROM_LOAD( "a0604.rom",    0x1a00000, 0x0400000, CRC(26b59fd3) SHA1(53219376056f4766dc5236735599d982ceb56b84) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0601.rom",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )
	ROM_LOAD( "b0602.rom",    0x0c00000, 0x0100000, CRC(9df77934) SHA1(99a3fe337c13702c9aa2373bcd1bb1befd0e2a13) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kovqhsgs )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "qhsg_c51.rom", 0x100000, 0x400000, CRC(e5cbac85) SHA1(4b424206387057863990b04f6d5bd0b6f754814f) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "qhsg_prot.c51", 0x000000, 0x04000, BAD_DUMP CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) )

	ROM_REGION( 0xc00000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "lsqh2_t01.rom",0x180000, 0x800000, CRC(d498d97f) SHA1(97a7b6d2ed1170449e7c2899448af7cbbca4c94f) )
	ROM_IGNORE( 0x800000 )  // second half identical

	ROM_REGION( 0x3000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "lsqh2_a01.rom", 0x0000000, 0x1000000, CRC(25ae3efd) SHA1(083d977602ddb5ad54fbdcba000cd4287de8d463) )
	ROM_LOAD( "lsqh2_a23.rom", 0x1000000, 0x1000000, CRC(7a779112) SHA1(0a7d36b3715063d8eac629b95a9bb3ecd8e54fca) )
	ROM_LOAD( "lsqh2_a45.rom", 0x2000000, 0x1000000, CRC(5d7de052) SHA1(7663b6cf09f65c4644661005a38f9aba84a32913) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "lsqh2_b01.rom", 0x0000000, 0x1000000, CRC(df7ca696) SHA1(7af3d27957a39de7e4873867c9972c05af7e7964) )

	ROM_REGION( 0xc00000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "lsqh2_m01.rom",0x400000, 0x400000, CRC(01af1b65) SHA1(6cf523fa8f1e03f974771611bb9a4e08a4d4443f) )
	ROM_IGNORE( 0x400000 )  // 400000-7fffff empty
	ROM_CONTINUE( 0x800000, 0x400000 )
	ROM_IGNORE( 0x400000 )  // c00000-ffffff empty
ROM_END


/*

p0701_v105.u2

IGS
PGM P0701 V105
1B2687LC
C994746

*/

ROM_START( photoy2k )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0701_v105.u2",     0x100000, 0x200000, CRC(fab142e0) SHA1(8dc7e53b740ed68bac98c0ef7ca4943c517e6f5d) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "igs027a_photoy2k_v100_china.asic", 0x000000, 0x04000, CRC(1a0b68f6) SHA1(290441ed652f54b26ace8f59a26220881fb62084) ) // 3 bytes differ from the read in the other sets.  I think this one is GOOD and the other is bad.  This always gives the same read, so unless the actual chips is bad... TBC

	ROM_REGION( 0x480000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0700.rom",    0x180000, 0x080000, CRC(93943b4d) SHA1(3b439903853727d45d62c781af6073024eb3c5a3) )

	ROM_REGION( 0x1080000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0700.l",    0x0000000, 0x0400000, CRC(26a9ae9c) SHA1(c977c89db6fdf47ee260ff687b80375caeab975c) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0700.h",    0x0400000, 0x0400000, CRC(79bc1fc1) SHA1(a09472a9b75704c1d31ab828f92c2a5007b2b4ed) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0701.l",    0x0800000, 0x0400000, CRC(23607f81) SHA1(8b6dbcdce9b131370693847ed9771aa04b62711c) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0701.h",    0x0c00000, 0x0400000, CRC(5f2efd37) SHA1(9a5bd9751691bc085b0751b9fa8ede9eb97b1248) )
	ROM_LOAD( "a0702.rom",  0x1000000, 0x0080000, CRC(42239e1b) SHA1(2b6d20958abf8a67ce525d5c8964b6d225ccaeda) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0700.l",    0x0000000, 0x0400000, CRC(af096904) SHA1(8e86b36cc44720ece68022e409279bf9144341ba) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "b0700.h",    0x0400000, 0x0400000, CRC(6d53de26) SHA1(f3f93fd2f87adb815834ba0242b94073fbb5e333) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "cgv101.rom", 0x0800000, 0x0020000, CRC(da02ec3e) SHA1(7ee21d748c9b932f53e790a9040167f904fecefc) )

	ROM_REGION( 0x480000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0700.rom",    0x400000, 0x080000, CRC(acc7afce) SHA1(ac2d344ebac336f0f363bb045dd8ea4e83d1fb50) )
ROM_END


ROM_START( photoy2k104 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v104.16m",     0x100000, 0x200000, CRC(e051070f) SHA1(a5a1a8dd7542a30632501af8d02fda07475fd9aa) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "igs027a_photoy2k_v100_china_alt.asic", 0x000000, 0x04000,  CRC(6dd7f257) SHA1(1984f98a282d8b3264674f231c3b7def1757cf72) )

	ROM_REGION( 0x480000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0700.rom",    0x180000, 0x080000, CRC(93943b4d) SHA1(3b439903853727d45d62c781af6073024eb3c5a3) )

	ROM_REGION( 0x1080000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0700.l",    0x0000000, 0x0400000, CRC(26a9ae9c) SHA1(c977c89db6fdf47ee260ff687b80375caeab975c) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0700.h",    0x0400000, 0x0400000, CRC(79bc1fc1) SHA1(a09472a9b75704c1d31ab828f92c2a5007b2b4ed) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0701.l",    0x0800000, 0x0400000, CRC(23607f81) SHA1(8b6dbcdce9b131370693847ed9771aa04b62711c) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0701.h",    0x0c00000, 0x0400000, CRC(5f2efd37) SHA1(9a5bd9751691bc085b0751b9fa8ede9eb97b1248) )
	ROM_LOAD( "a0702.rom",  0x1000000, 0x0080000, CRC(42239e1b) SHA1(2b6d20958abf8a67ce525d5c8964b6d225ccaeda) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0700.l",    0x0000000, 0x0400000, CRC(af096904) SHA1(8e86b36cc44720ece68022e409279bf9144341ba) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "b0700.h",    0x0400000, 0x0400000, CRC(6d53de26) SHA1(f3f93fd2f87adb815834ba0242b94073fbb5e333) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "cgv101.rom", 0x0800000, 0x0020000, CRC(da02ec3e) SHA1(7ee21d748c9b932f53e790a9040167f904fecefc) )

	ROM_REGION( 0x480000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0700.rom",    0x400000, 0x080000, CRC(acc7afce) SHA1(ac2d344ebac336f0f363bb045dd8ea4e83d1fb50) )
ROM_END

/*

Real and Fake / Photo Y2K (JPN 102 Ver.)
(c)1999 ALTA / IGS

PGM system
IGS PCB NO-0220
IGS PCB NO-0221


V102.U4      [a65eda9f]
V102.U5      [9201621b]
V102.U6      [b9ca5504]
V102.U8      [3be22b8f]

T0700.U11


A0700.U2
A0701.U4

SP_V102.U5

B0700.U7

CG_V101.U3
CG_V101.U6

*/

ROM_START( photoy2k102 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "v102.u4",     0x100001, 0x080000, CRC(a65eda9f) SHA1(6307cacf4a262e781753eff14700a0455837780c) )
	ROM_LOAD16_BYTE( "v102.u6",     0x100000, 0x080000, CRC(b9ca5504) SHA1(058cf01316f233236ca9861349f515935283b75e) )
	ROM_LOAD16_BYTE( "v102.u5",     0x200001, 0x080000, CRC(9201621b) SHA1(1ca3ebe7eec40614bfa8b911657fa2b51f2c51a4) )
	ROM_LOAD16_BYTE( "v102.u8",     0x200000, 0x080000, CRC(3be22b8f) SHA1(03634fbd6a8a8369c6cb1fd6694a3784dac5bf59) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "igs027a_photoy2k_v100_china_alt.asic", 0x000000, 0x04000,  CRC(6dd7f257) SHA1(1984f98a282d8b3264674f231c3b7def1757cf72) )

	ROM_REGION( 0x480000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0700.rom",    0x180000, 0x080000, CRC(93943b4d) SHA1(3b439903853727d45d62c781af6073024eb3c5a3) )

	ROM_REGION( 0x1080000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0700.l",    0x0000000, 0x0400000, CRC(26a9ae9c) SHA1(c977c89db6fdf47ee260ff687b80375caeab975c) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0700.h",    0x0400000, 0x0400000, CRC(79bc1fc1) SHA1(a09472a9b75704c1d31ab828f92c2a5007b2b4ed) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0701.l",    0x0800000, 0x0400000, CRC(23607f81) SHA1(8b6dbcdce9b131370693847ed9771aa04b62711c) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0701.h",    0x0c00000, 0x0400000, CRC(5f2efd37) SHA1(9a5bd9751691bc085b0751b9fa8ede9eb97b1248) )
	ROM_LOAD( "a0702.rom",  0x1000000, 0x0080000, CRC(42239e1b) SHA1(2b6d20958abf8a67ce525d5c8964b6d225ccaeda) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0700.l",    0x0000000, 0x0400000, CRC(af096904) SHA1(8e86b36cc44720ece68022e409279bf9144341ba) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "b0700.h",    0x0400000, 0x0400000, CRC(6d53de26) SHA1(f3f93fd2f87adb815834ba0242b94073fbb5e333) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "cgv101.rom", 0x0800000, 0x0020000, CRC(da02ec3e) SHA1(7ee21d748c9b932f53e790a9040167f904fecefc) )

	ROM_REGION( 0x480000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0700.rom",    0x400000, 0x080000, CRC(acc7afce) SHA1(ac2d344ebac336f0f363bb045dd8ea4e83d1fb50) )
ROM_END


/*

Photo Y2K2 IGS PGM cart

Top board
---------
PCB Number: IGS PCB-0313-00T
27C160 EPROM at U1
PAL at U3
IGS027A at U4

Bottom Board
------------
PCB Number: IGS PCB-0314-00
1x 16M SOP44 mask ROM at U3
6x 64M SOP44 mask ROMs at U4, U5, U6, U7, U8, U9

*/

ROM_START( py2k2 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "y2k2_m-101xx.u1",     0x100000, 0x200000, CRC(c47795f1) SHA1(5be4af4275571932d7740c3ea0857a1f58a3f6d9) ) // 68k (encrypted) 2nd half empty...

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "igs027a_photoy2k2.asic", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0x480000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	/* no extra tilemap rom */

	ROM_REGION( 0x2000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "y2k2_a1100.u6",    0x0000000, 0x0800000, CRC(e32ce499) SHA1(f84c7daa55c25a05da467b5654ebf432b7ce1754) )
	ROM_LOAD( "y2k2_a1101.u7",    0x0800000, 0x0800000, CRC(4e7568bc) SHA1(bf9cc453191bd5ec9fbcce62891809f253a44267) )
	ROM_LOAD( "y2k2_a1102.u8",    0x1000000, 0x0800000, CRC(6da7c143) SHA1(9408ba7722bfc8013f851aadea5e2819f5263129) )
	ROM_LOAD( "y2k2_a1103.u9",    0x1800000, 0x0800000, CRC(0ebebfdc) SHA1(4faad7f97c7e734f179ec934a37e75d8d6adccf4) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "y2k2_b1100.u4",    0x0000000, 0x0800000,  CRC(fa53d6f6) SHA1(c2da55f4b7e721fa1c63bd7f9528f261643164e8) )
	ROM_LOAD( "y2k2_b1101.u5",    0x0800000, 0x0800000, CRC(001e4c81) SHA1(21119055f8fd7f831529e73ff9c97bca3987a1dc))

	ROM_REGION( 0x880000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "y2k2_m1100.u3",    0x400000, 0x200000, CRC(fb1515f8) SHA1(90e5e5bfdac9a460445bf224952e4a536888dc1b) )
ROM_END


ROM_START( pgm3in1 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v-100cn.u3",     0x100000, 0x200000, CRC(a39f59b4) SHA1(4eb53fb9f173cb470e16dc8f193c8909cf045e3d)) // 68k (encrypted)

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "igs027a_pgm3in1.asic", 0x000000, 0x04000, NO_DUMP )

	/* No external ARM rom */

	ROM_REGION( 0x380000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "u5.u5",    0x180000, 0x200000, CRC(da375a50) SHA1(62cd2fd3dfc1897528eaa38d243d7a9526eac71b) )

	ROM_REGION( 0x2800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgm_a1100.u4",    0x0000000, 0x0800000, CRC(e32ce499) SHA1(f84c7daa55c25a05da467b5654ebf432b7ce1754) ) // == y2k2_a1100.u6
	ROM_LOAD( "pgm_a1101.u5",    0x0800000, 0x0800000, CRC(4e7568bc) SHA1(bf9cc453191bd5ec9fbcce62891809f253a44267) ) // == y2k2_a1101.u7
	ROM_LOAD( "pgm_a1102.u6",    0x1000000, 0x0800000, CRC(6da7c143) SHA1(9408ba7722bfc8013f851aadea5e2819f5263129) ) // == y2k2_a1102.u8
	ROM_LOAD( "pgm_a1103.u7",    0x1800000, 0x0800000, CRC(0ebebfdc) SHA1(4faad7f97c7e734f179ec934a37e75d8d6adccf4) ) // == y2k2_a1103.u9
	ROM_LOAD( "ext_bit_cg.u20",  0x2000000, 0x0400000, CRC(fe314754) SHA1(ae3e8bbdce852a3fa39806a5221c053dee5abfd4) )

	ROM_REGION( 0x1000000, "sprmask", ROMREGION_ERASE00 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgm_b1100.u8",    0x0000000, 0x0800000, CRC(fa53d6f6) SHA1(c2da55f4b7e721fa1c63bd7f9528f261643164e8) ) // == y2k2_b1100.u4
	ROM_LOAD( "pgm_b1101.u9",    0x0800000, 0x0800000, CRC(001e4c81) SHA1(21119055f8fd7f831529e73ff9c97bca3987a1dc) ) // == y2k2_b1101.u5
	ROM_LOAD( "ext_bit_map.u21", 0x0f00000, 0x0100000, CRC(fe31dca6) SHA1(825bab7342c944794514fc7fe3e41779de3b5cd4) ) // yes this loads over the empty part of u9
	ROM_IGNORE(0x0100000) // the 2nd half is empty

	ROM_REGION( 0xe80000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "pgm_m1100.u17",    0x400000, 0x200000, CRC(fb1515f8) SHA1(90e5e5bfdac9a460445bf224952e4a536888dc1b) ) // == y2k2_m1100.u3
	ROM_LOAD( "u16.u16",          0x600000, 0x800000, CRC(714c33e5) SHA1(5478d5247349cdfb5f835171615d6ca2e5689140) ) // check loading
ROM_END


/*

The Killing Blade (English / World Version)
IGS, 1998

This is a cart for the IGS PGM system.


PCB Layout
----------

IGS PCB NO-0179
|-----------------------------------------------|
|                      8MHz  |--------|         |
|            32.768kHz|----| |        |T0300.U14|
|6164  6164           |IGS | | IGS025 |         |
|                     |022 | |        |         |
|*                    |----| |--------|         |
|                                               |
|           U2     U3     U4     U5     U6      |
| PAL   PAL                                PAL  |
|                                               |
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      IGS022       - Custom IGS IC (QFP100)
      IGS025       - Custom IGS IC (PLCC68, labelled "ENGLISH")
      T0300.U14    - 32MBit MaskROM (SOP44, labelled "T0300")
      6164         - x2, 8K x8 SRAM (SOJ28)
      U2           - 27C512 512KBit EPROM (DIP28, labelled "KB U2 V104")
      U3           - 27C4000 4MBit EPROM (DIP32, labelled "KB U3 V104")
      U4           - 27C4000 4MBit EPROM (DIP32, labelled "KB U4 V104")
      U5           - 27C4000 4MBit EPROM (DIP32, labelled "KB U5 V104")
      U6           - 27C4000 4MBit EPROM (DIP32, labelled "KB U6 V104")
      PALs         - x3, labelled "DH U8", "DH U1", "DH U7"
      *            - Unpopulated position for DIP42 EPROM/MASKROM (labelled "P0300")


IGS PCB NO-0178
|-----------------------------------------------|
| U9    U10   U11    U12     U13     U14     U15|
|                                               |
|                                               |
|                                               |
| U1    U2                         74LS138      |
|                                  74LS139      |
|             U3     U4      U5              U8 |
|                                               |
|                                               |
|-|                                           |-|
  |--------------------||---------------------|

Notes:
      U1           - 32MBit MASKROM (SOP44, labelled "M0300")
      U2           - 32MBit MASKROM (SOP44, labelled "A0307")
      U3           - 16MBit MASKROM (DIP42, labelled "A0302")
      U4           - 16MBit MASKROM (DIP42, labelled "A0304")
      U5           - 16MBit MASKROM (DIP42, labelled "A0305")
      U8           - 16MBit MASKROM (DIP42, labelled "B0301")
      U9           - 32MBit MASKROM (SOP44, labelled "A0300")
      U10          - 32MBit MASKROM (SOP44, labelled "A0301")
      U11          - 32MBit MASKROM (SOP44, labelled "A0303")
      U12          - 32MBit MASKROM (SOP44, labelled "A0306")
      U13          - 32MBit MASKROM (SOP44, labelled "B0300")
      U14          - 32MBit MASKROM (SOP44, labelled "B0302")
      U15          - 32MBit MASKROM (SOP44, labelled "B0303")

*/



/*
 the text on the chip are
-----------------
IGS
PGM P0300 V109
1A0577Y3
J982846
-----------------
*/

ROM_START( killbld )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0300_v109.u9", 0x100000, 0x200000, CRC(2fcee215) SHA1(855281a9090bfdf3da9f4d50c121765131a13400) )

	ROM_REGION( 0x010000, "igs022data", 0 ) /* Protection Data */
	ROM_LOAD( "kb_u2.rom", 0x000000, 0x010000,  CRC(de3eae63) SHA1(03af767ef764055bda528b5cc6a24b9e1218cca8) )

	ROM_REGION( 0x800000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0300.u14",    0x180000, 0x400000, CRC(0922f7d9) SHA1(4302b4b7369e13f315fad14f7d6cad1321101d24) )

	ROM_REGION( 0x2000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0300.u9",   0x0000000, 0x0400000,  CRC(3f9455d3) SHA1(3718ce00ad93975383aafc14e5a74dc297b011a1) )
	ROM_LOAD( "a0301.u10",  0x0400000, 0x0400000,  CRC(92776889) SHA1(6d677837fefff47bfd1c6166322f69f89989a5e2) )
	ROM_LOAD( "a0303.u11",  0x0800000, 0x0400000,  CRC(33f5cc69) SHA1(9cacd5058d4bb25b77f71658bbbbd4b38d0a6b6a) )
	ROM_LOAD( "a0306.u12",  0x0c00000, 0x0400000,  CRC(cc018a8e) SHA1(37752d46f238fb57c0ab5a4f96b1e013f2077347) )
	ROM_LOAD( "a0307.u2",   0x1000000, 0x0400000,  CRC(bc772e39) SHA1(079cc42a190cb916f02b59bca8fa90e524acefe9) )
//  ROM_LOAD( "a0302.u3",   0x1400000, 0x0200000,  CRC(a4810e38) SHA1(c31fe641feab2c93795fc35bf71d4f37af1056d4) ) // from lord of gun! unused..
//  ROM_LOAD( "a0304.u4",   0x1600000, 0x0200000,  CRC(3096de1c) SHA1(d010990d21cfda9cb8ab5b4bc0e329c23b7719f5) ) // from lord of gun! unused..
//  ROM_LOAD( "a0305.u5",   0x1800000, 0x0200000,  CRC(2234531e) SHA1(58a82e31a1c0c1a4dd026576319f4e7ecffd140e) ) // from lord of gun! unused..

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0300.u13",    0x0000000, 0x0400000, CRC(7f876981) SHA1(43555a200929ad5ecc42137fc9aeb42dc4f50d20) )
	ROM_LOAD( "b0302.u14",    0x0400000, 0x0400000, CRC(eea9c502) SHA1(04b3972c7111ea59a3cceab6ad124080c4ce3520) )
	ROM_LOAD( "b0303.u15",    0x0800000, 0x0200000, CRC(77a9652e) SHA1(2342f643d37945fbda224a5034c013796e5134ca) )
//  ROM_LOAD( "b0301.u8",     0x0a00000, 0x0200000, CRC(400abe33) SHA1(20de1eb626424ea41bd55eb3cecd6b50be744ee0) ) // from lord of gun! unused..

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0300.u1",     0x400000, 0x400000, CRC(93159695) SHA1(50c5976c9b681bd3d1ebefa3bfa9fe6e72dcb96f) )
ROM_END

ROM_START( killbld104 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "kb_u3_v104.u3",     0x100001, 0x080000, CRC(6db1d719) SHA1(804002f014d275aaf0368fb7f904938fe4ac07ee) )
	ROM_LOAD16_BYTE( "kb_u6_v104.u6",     0x100000, 0x080000, CRC(31ecc978) SHA1(82666d534e4151775063af6d39f575faba0f1047) )
	ROM_LOAD16_BYTE( "kb_u4_v104.u4",     0x200001, 0x080000, CRC(1ed8b2e7) SHA1(331c037640cfc1fe743cd0e65a1156c470b3303e) )
	ROM_LOAD16_BYTE( "kb_u5_v104.u5",     0x200000, 0x080000, CRC(a0bafc29) SHA1(b20db7c16353c6f87ed3c08c9d037b07336711f1) )

	ROM_REGION( 0x010000, "igs022data", 0 ) /* Protection Data */
	ROM_LOAD( "kb_u2_v104.u2", 0x000000, 0x010000,  CRC(c970f6d5) SHA1(399fc6f80262784c566363c847dc3fdc4fb37494) )

	ROM_REGION( 0x800000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0300.u14",    0x180000, 0x400000, CRC(0922f7d9) SHA1(4302b4b7369e13f315fad14f7d6cad1321101d24) )

	ROM_REGION( 0x2000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0300.u9",   0x0000000, 0x0400000,  CRC(3f9455d3) SHA1(3718ce00ad93975383aafc14e5a74dc297b011a1) )
	ROM_LOAD( "a0301.u10",  0x0400000, 0x0400000,  CRC(92776889) SHA1(6d677837fefff47bfd1c6166322f69f89989a5e2) )
	ROM_LOAD( "a0303.u11",  0x0800000, 0x0400000,  CRC(33f5cc69) SHA1(9cacd5058d4bb25b77f71658bbbbd4b38d0a6b6a) )
	ROM_LOAD( "a0306.u12",  0x0c00000, 0x0400000,  CRC(cc018a8e) SHA1(37752d46f238fb57c0ab5a4f96b1e013f2077347) )
	ROM_LOAD( "a0307.u2",   0x1000000, 0x0400000,  CRC(bc772e39) SHA1(079cc42a190cb916f02b59bca8fa90e524acefe9) )
//  ROM_LOAD( "a0302.u3",   0x1400000, 0x0200000,  CRC(a4810e38) SHA1(c31fe641feab2c93795fc35bf71d4f37af1056d4) ) // from lord of gun! unused..
//  ROM_LOAD( "a0304.u4",   0x1600000, 0x0200000,  CRC(3096de1c) SHA1(d010990d21cfda9cb8ab5b4bc0e329c23b7719f5) ) // from lord of gun! unused..
//  ROM_LOAD( "a0305.u5",   0x1800000, 0x0200000,  CRC(2234531e) SHA1(58a82e31a1c0c1a4dd026576319f4e7ecffd140e) ) // from lord of gun! unused..

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0300.u13",    0x0000000, 0x0400000, CRC(7f876981) SHA1(43555a200929ad5ecc42137fc9aeb42dc4f50d20) )
	ROM_LOAD( "b0302.u14",    0x0400000, 0x0400000, CRC(eea9c502) SHA1(04b3972c7111ea59a3cceab6ad124080c4ce3520) )
	ROM_LOAD( "b0303.u15",    0x0800000, 0x0200000, CRC(77a9652e) SHA1(2342f643d37945fbda224a5034c013796e5134ca) )
//  ROM_LOAD( "b0301.u8",     0x0a00000, 0x0200000, CRC(400abe33) SHA1(20de1eb626424ea41bd55eb3cecd6b50be744ee0) ) // from lord of gun! unused..

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0300.u1",     0x400000, 0x400000, CRC(93159695) SHA1(50c5976c9b681bd3d1ebefa3bfa9fe6e72dcb96f) )
ROM_END




/*
Puzzle Star
IGS, 1999

Cart for IGS PGM system. This game is a 'columns' type game.

PCB Layout
----------

IGS PCB NO- T0236
|-----------------------------------------------|
|                        U6 U7                  |
|         |-------|                             |
|         |IGS027A|                             |
|         |       |                     T0800.U5|
|         |       |                             |
|         |-------|                             |
|          U1_V100MG.U1                         |
|          U2_V100MG.U2   U3   PAL              |
|                                               |
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      IGS027A       - Custom IGS IC, ARM7/9? based CPU (QFP120, stamped 'IGS027A' & labelled 'ENGLISH')
      T0800.U5      - 16MBit MaskROM (DIP42)
      U1_V100MG.U1  - MX27C4000 512K x8 EPROM (DIP32, labelled 'PuzzleStar U1 V100MG')
      U2_V100MG.U2  - MX27C4000 512K x8 EPROM (DIP32, labelled 'PuzzleStar U2 V100MG')
      PAL           - Atmel ATF22V10B PAL (DIP24, labelled 'EA U4')
      U3            - Unpopulated position for 32MBit MASKROM (DIP42)
      U6, U7        - Unpopulated position for 74LS245 logic chip (x2)


IGS PCB NO- T0237
|-----------------------------------------------|
|                                               |
|                                               |
|                                               |
|                                               |
|       A0800.U1   M0800.U2   B0800.U3          |
|                                               |
|                                               |
|                                               |
|                                               |
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      U1 - 32MBit MaskROM (DIP42)
      U2 - 32MBit MaskROM (DIP42)
      U3 - 16MBit MaskROM (DIP42)
*/

ROM_START( puzlstar )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "v100mg.u1",     0x100000, 0x080000, CRC(5788b77d) SHA1(7770aae6e686da92b2623c977d1bc8f019f48267) )
	ROM_LOAD16_BYTE( "v100mg.u2",     0x100001, 0x080000, CRC(4c79d979) SHA1(3b92052a35994f2b3dd164930154184c45d5e2d0) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "puzlstar_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0800.u5",    0x180000, 0x200000, CRC(f9d84e59) SHA1(80ec77025ac5bf355b1a60f2a678dd4c56071f6b) )

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0800.u1",    0x0000000, 0x0400000, CRC(e1e6ec40) SHA1(390432431f144ef63424a426582b311765a61771) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0800.u3",    0x0000000, 0x0200000, CRC(52e7bef5) SHA1(a678251b7e46a1016d0afc1d8d5c9928008ad5b1) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0800.u2",    0x400000, 0x400000,  CRC(e1a46541) SHA1(6fe9de5700d8638374734d80551dcedb62975140) )
ROM_END


/*

Oriental Legend Super
IGS, 1998

This is a cart for the IGS PGM system.

PCB Layout
----------
IGS PCB NO-0191-1
|-----------------------------------------------|
|6264                 8MHz|--------|            |
|6264                     |        |   T0500.U18|
|                         | IGS025 |            |
|                 |-----| |        |   T0501.U19|
|                 | IGS | |--------|            |
|                 | 028 |                       |
|        *1       |-----|           V101.U1     |
|              V101.U2   V101.U4  PAL      PAL  |
|  V101.U6          V101.U3   V101.U5           |
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      IGS028       - Custom IGS IC (QFP100)
      IGS025       - Custom IGS IC (PLCC68, labelled "KOREA")
      T0500.U18    - 32MBit MaskROM (SOP44)
      T0501.U19    - 16MBit MaskROM (SOP44)
      V101.U1      - MX27C4096 4MBit EPROM (DIP40)
      V101.U2/3/4/5- MX27C4000 4MBit EPROM (DIP32)
      PALs         - x2, labelled "CW-2 U8", "CW-2 U7"
      6264         - 8K x8 SRAM
      *1           - Unpopulated position for SOP44 MASKROM labelled "P0500"


IGS PCB NO-0135
|-----------------------------------------------|
|A0504.U11        A0506.U13     B0502.U15       |
|         A0505.U12         U14        B0503.U16|
|                                               |
|A0500.U5                       B0500.U9        |
|         A0501.U6       A0503.U8      B0501.U10|
|                 A0502.U7                      |
|                                               |
|74LS138          M0500.U1               74LS139|
|                           U2                  |
|-|                                           |-|
  |--------------------||---------------------|

Notes:
      This PCB contains only SOP44 MaskROMS and 2 logic IC's
      U2 and U14 are not populated.
      All are 32MBit except M0500 which is 16MBit.

*/





ROM_START( olds103t )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0500.v103",0x100000, 0x400000, CRC(17e32e14) SHA1(b8f731087af2c59fe5b1da31f1cb055d35c8b440) )

	ROM_REGION( 0xc00000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0500.rom",    0x180000, 0x400000, CRC(d881726c) SHA1(a82517e665996f7b7017c940f1fcf016fccb65c2) )
	ROM_LOAD( "t0501.rom",    0x580000, 0x200000, CRC(d2106864) SHA1(65d827135b87d82196433aea3279608ee263feca) )

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0500.rom",    0x0000000, 0x400000, CRC(80a59197) SHA1(7d0108e2f0d0347d43ace2d96c0388202c05fdfb) )
	ROM_LOAD( "a0501.rom",    0x0400000, 0x400000, CRC(98c931b1) SHA1(9b70d1a7beb4c3a0c1436c25fd1fb76e47539538) )
	ROM_LOAD( "a0502.rom",    0x0800000, 0x400000, CRC(c3fcdf1d) SHA1(107585fd103fcd0af0fb7db283be4f7c7058aef7) )
	ROM_LOAD( "a0503.rom",    0x0c00000, 0x400000, CRC(066dffec) SHA1(f023032a7202b7090fb609a39e0f19018e664bf3) )
	ROM_LOAD( "a0504.rom",    0x1000000, 0x400000, CRC(45337583) SHA1(c954d0e5bf7fa99c90b0d154e7119d2b0c461f1c) )
	ROM_LOAD( "a0505.rom",    0x1400000, 0x400000, CRC(5b8cf3a5) SHA1(856d1e47b5d9a66dcfbdc74a51ed646fd7d96a35) )
	ROM_LOAD( "a0506.rom",    0x1800000, 0x400000, CRC(087ac60c) SHA1(3d5bf7dd40c8a3c1224cf82e12410ca904c0c5db) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0500.rom",    0x0000000, 0x400000, CRC(cde07f74) SHA1(1fe90db7c57faa28f3a054b2c07909bef89e3efb) )
	ROM_LOAD( "b0501.rom",    0x0400000, 0x400000, CRC(1546c2e9) SHA1(a7b9c8b44203db54a59d49fe469bb52bba807ba2) )
	ROM_LOAD( "b0502.rom",    0x0800000, 0x400000, CRC(e97b31c3) SHA1(1a7ca4f6c8644e84a33ae41cd4637f21046b14c5) )
	ROM_LOAD( "b0503.u16",    0x0c00000, 0x400000, CRC(e41d98e4) SHA1(f80b27fcee81762993e09bf1b3cad6e85274760c) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0500.rom",    0x400000, 0x200000, CRC(37928cdd) SHA1(e80498cabc2a6a54d4f3ebcb097d4b3fad96fe55) )
ROM_END

ROM_START( olds )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "sp_v101.u2",      0x100001, 0x080000,   CRC(08eb9661) SHA1(105946e72e562adb1a9fd794ca0fd2c91967eb56) )
	ROM_LOAD16_BYTE( "sp_v101.u3",      0x100000, 0x080000,   CRC(0a358c1e) SHA1(95c7c3f069c5d05001e22535750f6b3cd7de105f) )
	ROM_LOAD16_BYTE( "sp_v101.u4",      0x200001, 0x080000,   CRC(766570e0) SHA1(e7c3f5664ec69b662b82c2e1375555db7305390c) )
	ROM_LOAD16_BYTE( "sp_v101.u5",      0x200000, 0x080000,   CRC(58662e12) SHA1(2b39bd847e9c4968a8e77a2f3cec77cf323ceee3) )
	ROM_LOAD16_WORD_SWAP( "sp_v101.u1",0x300000, 0x080000,    CRC(2b2f4f1e) SHA1(67b97cf8cc7f517d67cd45588addd2ad8e24612a) )

	ROM_REGION( 0x010000, "user1", 0 ) /* IGS028 Protection Data */
	ROM_LOAD( "sp_v101.u6", 0x000000, 0x010000,  CRC(097046bc) SHA1(6d75db85cf4c79b63e837897785c253014b2126d) )

	ROM_REGION( 0x4000, "user2", ROMREGION_ERASEFF ) /* its a dump of the shared protection rom/ram from pcb. */
	// clearly not for this revision
	ROM_LOAD( "ram_dump", 0x000000, 0x04000, CRC(280cfb4e) SHA1(cd2bdcaa21347952c2bf38b105a204d327fde39e) )

	ROM_REGION( 0xc00000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0500.rom",    0x180000, 0x400000, CRC(d881726c) SHA1(a82517e665996f7b7017c940f1fcf016fccb65c2) )
	ROM_LOAD( "t0501.rom",    0x580000, 0x200000, CRC(d2106864) SHA1(65d827135b87d82196433aea3279608ee263feca) )

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0500.rom",    0x0000000, 0x400000, CRC(80a59197) SHA1(7d0108e2f0d0347d43ace2d96c0388202c05fdfb) )
	ROM_LOAD( "a0501.rom",    0x0400000, 0x400000, CRC(98c931b1) SHA1(9b70d1a7beb4c3a0c1436c25fd1fb76e47539538) )
	ROM_LOAD( "a0502.rom",    0x0800000, 0x400000, CRC(c3fcdf1d) SHA1(107585fd103fcd0af0fb7db283be4f7c7058aef7) )
	ROM_LOAD( "a0503.rom",    0x0c00000, 0x400000, CRC(066dffec) SHA1(f023032a7202b7090fb609a39e0f19018e664bf3) )
	ROM_LOAD( "a0504.rom",    0x1000000, 0x400000, CRC(45337583) SHA1(c954d0e5bf7fa99c90b0d154e7119d2b0c461f1c) )
	ROM_LOAD( "a0505.rom",    0x1400000, 0x400000, CRC(5b8cf3a5) SHA1(856d1e47b5d9a66dcfbdc74a51ed646fd7d96a35) )
	ROM_LOAD( "a0506.rom",    0x1800000, 0x400000, CRC(087ac60c) SHA1(3d5bf7dd40c8a3c1224cf82e12410ca904c0c5db) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0500.rom",    0x0000000, 0x400000, CRC(cde07f74) SHA1(1fe90db7c57faa28f3a054b2c07909bef89e3efb) )
	ROM_LOAD( "b0501.rom",    0x0400000, 0x400000, CRC(1546c2e9) SHA1(a7b9c8b44203db54a59d49fe469bb52bba807ba2) )
	ROM_LOAD( "b0502.rom",    0x0800000, 0x400000, CRC(e97b31c3) SHA1(1a7ca4f6c8644e84a33ae41cd4637f21046b14c5) )
	ROM_LOAD( "b0503.u16",    0x0c00000, 0x400000, CRC(e41d98e4) SHA1(f80b27fcee81762993e09bf1b3cad6e85274760c) )


	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0500.rom",    0x400000, 0x200000, CRC(37928cdd) SHA1(e80498cabc2a6a54d4f3ebcb097d4b3fad96fe55) )
ROM_END

ROM_START( olds100 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "v100-u2.040",      0x100001, 0x080000,  CRC(517c2a06) SHA1(bbf5b311fac9b0bb4d4129c0561e5e24f6963fa2) )
	ROM_LOAD16_BYTE( "v100-u3.040",      0x100000, 0x080000,  CRC(d0e2b741) SHA1(2e671dbb4320d1f0c059b35efd33cdea26f12131) )
	ROM_LOAD16_BYTE( "v100-u4.040",      0x200001, 0x080000,  CRC(32a6bdbd) SHA1(a93d7f4eae722a58eca9ec351ad5890cefda56f0) )
	ROM_LOAD16_BYTE( "v100-u5.040",      0x200000, 0x080000,  CRC(b4a1cafb) SHA1(b2fccd480ede93f58ad043387b18b898152f06ef) )
	ROM_LOAD16_WORD_SWAP( "v100-u1.040", 0x300000, 0x080000,  CRC(37ea4e75) SHA1(a94fcb89da3394a43d360f885419677f511d2580) )

	ROM_REGION( 0x4000, "user2", ROMREGION_ERASEFF ) /* its a dump of the shared protection rom/ram from pcb. */
	// used to simulate encrypted DMA protection device for now ..
	ROM_LOAD( "ram_dump", 0x000000, 0x04000, CRC(280cfb4e) SHA1(cd2bdcaa21347952c2bf38b105a204d327fde39e) )

	ROM_REGION( 0x010000, "user1", 0 ) /* IGS028 Protection Data */
	ROM_LOAD( "kd-u6.512", 0x000000, 0x010000,  CRC(e7613dda) SHA1(0d7c043b90e2f9a36a45066f22e3e305dc716676) )

	ROM_REGION( 0xc00000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0500.rom",    0x180000, 0x400000, CRC(d881726c) SHA1(a82517e665996f7b7017c940f1fcf016fccb65c2) )
	ROM_LOAD( "t0501.rom",    0x580000, 0x200000, CRC(d2106864) SHA1(65d827135b87d82196433aea3279608ee263feca) )

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0500.rom",    0x0000000, 0x400000, CRC(80a59197) SHA1(7d0108e2f0d0347d43ace2d96c0388202c05fdfb) )
	ROM_LOAD( "a0501.rom",    0x0400000, 0x400000, CRC(98c931b1) SHA1(9b70d1a7beb4c3a0c1436c25fd1fb76e47539538) )
	ROM_LOAD( "a0502.rom",    0x0800000, 0x400000, CRC(c3fcdf1d) SHA1(107585fd103fcd0af0fb7db283be4f7c7058aef7) )
	ROM_LOAD( "a0503.rom",    0x0c00000, 0x400000, CRC(066dffec) SHA1(f023032a7202b7090fb609a39e0f19018e664bf3) )
	ROM_LOAD( "a0504.rom",    0x1000000, 0x400000, CRC(45337583) SHA1(c954d0e5bf7fa99c90b0d154e7119d2b0c461f1c) )
	ROM_LOAD( "a0505.rom",    0x1400000, 0x400000, CRC(5b8cf3a5) SHA1(856d1e47b5d9a66dcfbdc74a51ed646fd7d96a35) )
	ROM_LOAD( "a0506.rom",    0x1800000, 0x400000, CRC(087ac60c) SHA1(3d5bf7dd40c8a3c1224cf82e12410ca904c0c5db) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0500.rom",    0x0000000, 0x400000, CRC(cde07f74) SHA1(1fe90db7c57faa28f3a054b2c07909bef89e3efb) )
	ROM_LOAD( "b0501.rom",    0x0400000, 0x400000, CRC(1546c2e9) SHA1(a7b9c8b44203db54a59d49fe469bb52bba807ba2) )
	ROM_LOAD( "b0502.rom",    0x0800000, 0x400000, CRC(e97b31c3) SHA1(1a7ca4f6c8644e84a33ae41cd4637f21046b14c5) )
	ROM_LOAD( "b0503.u16",    0x0c00000, 0x400000, CRC(e41d98e4) SHA1(f80b27fcee81762993e09bf1b3cad6e85274760c) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0500.rom",    0x400000, 0x200000, CRC(37928cdd) SHA1(e80498cabc2a6a54d4f3ebcb097d4b3fad96fe55) )
ROM_END

/* this is the set which the protection ram dump seems to be for.. */
ROM_START( olds100a )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	/* this rom had a lame hack applied to it by the dumper, this was removed, hopefully it is correct now */
	ROM_LOAD16_WORD_SWAP( "p0500.v10",    0x100000, 0x400000, CRC(8981fc87) SHA1(678d6705d06b99bca5951ff77708adadc4c4396b) )

	ROM_REGION( 0x010000, "user1", ROMREGION_ERASEFF ) /* IGS028 Protection Data */
	/* missing from this set .. */

	ROM_REGION( 0x4000, "user2", ROMREGION_ERASEFF ) /* its a dump of the shared protection rom/ram from pcb. */
	// used to simulate encrypted DMA protection device for now ..
	ROM_LOAD( "ram_dump", 0x000000, 0x04000, CRC(280cfb4e) SHA1(cd2bdcaa21347952c2bf38b105a204d327fde39e) )

	ROM_REGION( 0xc00000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0500.rom",    0x180000, 0x400000, CRC(d881726c) SHA1(a82517e665996f7b7017c940f1fcf016fccb65c2) )
	ROM_LOAD( "t0501.rom",    0x580000, 0x200000, CRC(d2106864) SHA1(65d827135b87d82196433aea3279608ee263feca) )

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0500.rom",    0x0000000, 0x400000, CRC(80a59197) SHA1(7d0108e2f0d0347d43ace2d96c0388202c05fdfb) )
	ROM_LOAD( "a0501.rom",    0x0400000, 0x400000, CRC(98c931b1) SHA1(9b70d1a7beb4c3a0c1436c25fd1fb76e47539538) )
	ROM_LOAD( "a0502.rom",    0x0800000, 0x400000, CRC(c3fcdf1d) SHA1(107585fd103fcd0af0fb7db283be4f7c7058aef7) )
	ROM_LOAD( "a0503.rom",    0x0c00000, 0x400000, CRC(066dffec) SHA1(f023032a7202b7090fb609a39e0f19018e664bf3) )
	ROM_LOAD( "a0504.rom",    0x1000000, 0x400000, CRC(45337583) SHA1(c954d0e5bf7fa99c90b0d154e7119d2b0c461f1c) )
	ROM_LOAD( "a0505.rom",    0x1400000, 0x400000, CRC(5b8cf3a5) SHA1(856d1e47b5d9a66dcfbdc74a51ed646fd7d96a35) )
	ROM_LOAD( "a0506.rom",    0x1800000, 0x400000, CRC(087ac60c) SHA1(3d5bf7dd40c8a3c1224cf82e12410ca904c0c5db) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0500.rom",    0x0000000, 0x400000, CRC(cde07f74) SHA1(1fe90db7c57faa28f3a054b2c07909bef89e3efb) )
	ROM_LOAD( "b0501.rom",    0x0400000, 0x400000, CRC(1546c2e9) SHA1(a7b9c8b44203db54a59d49fe469bb52bba807ba2) )
	ROM_LOAD( "b0502.rom",    0x0800000, 0x400000, CRC(e97b31c3) SHA1(1a7ca4f6c8644e84a33ae41cd4637f21046b14c5) )
	ROM_LOAD( "b0503.u16",    0x0c00000, 0x400000, CRC(e41d98e4) SHA1(f80b27fcee81762993e09bf1b3cad6e85274760c) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0500.rom",    0x400000, 0x200000, CRC(37928cdd) SHA1(e80498cabc2a6a54d4f3ebcb097d4b3fad96fe55) )
ROM_END


ROM_START( kov2 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "u18.107",      0x100000, 0x400000, CRC(661a5b2c) SHA1(125054fabc93d4f4cba869c3e6adf863650d30cf) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2_v100_hongkong.asic", 0x000000, 0x04000,  CRC(e0d7679f) SHA1(e1c2d127eba4ddbeb8ad173c55b90ac1467e1ca8) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "u19.102", 0x000000, 0x200000,   CRC(462e2980) SHA1(3da7c3d2c65b59f50c78be1c25922b71d40f6080) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t1200.rom",    0x180000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION( 0x2800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a1200.rom",    0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "a1201.rom",    0x0800000, 0x0800000, CRC(21063ca7) SHA1(cf561b44902425a920d5cbea5bf65dd9530b2289) )
	ROM_LOAD( "a1202.rom",    0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "a1203.rom",    0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "a1204.rom",    0x2000000, 0x0200000, CRC(14b4b5bb) SHA1(d7db5740eec971f2782fb2885ee3af8f2a796550) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b1200.rom",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "b1201.rom",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m1200.rom",    0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END

ROM_START( kov2106 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "u18.106",    0x100000, 0x400000, CRC(40051ad9) SHA1(ba2ddf267fe688d5dfed575aeeccbab10135b37b) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2_v100_hongkong.asic", 0x000000, 0x04000,  CRC(e0d7679f) SHA1(e1c2d127eba4ddbeb8ad173c55b90ac1467e1ca8) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "u19.102", 0x000000, 0x200000,   CRC(462e2980) SHA1(3da7c3d2c65b59f50c78be1c25922b71d40f6080) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t1200.rom",    0x180000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION( 0x2800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a1200.rom",    0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "a1201.rom",    0x0800000, 0x0800000, CRC(21063ca7) SHA1(cf561b44902425a920d5cbea5bf65dd9530b2289) )
	ROM_LOAD( "a1202.rom",    0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "a1203.rom",    0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "a1204.rom",    0x2000000, 0x0200000, CRC(14b4b5bb) SHA1(d7db5740eec971f2782fb2885ee3af8f2a796550) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b1200.rom",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "b1201.rom",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m1200.rom",    0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END


ROM_START( kov2103 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "u18.103",    0x100000, 0x400000, CRC(98c32f76) SHA1(ec7e35e8071bb7097e415493be4e40be0ca19fd6) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2_v100_hongkong.asic", 0x000000, 0x04000,  CRC(e0d7679f) SHA1(e1c2d127eba4ddbeb8ad173c55b90ac1467e1ca8) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "u19.101", 0x000000, 0x200000, CRC(8c35f2fe) SHA1(d4858f97fcfad0f342fccbc9cf1590276cc3c69c) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t1200.rom",    0x180000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION( 0x2800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a1200.rom",    0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "a1201.rom",    0x0800000, 0x0800000, CRC(21063ca7) SHA1(cf561b44902425a920d5cbea5bf65dd9530b2289) )
	ROM_LOAD( "a1202.rom",    0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "a1203.rom",    0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "a1204.rom",    0x2000000, 0x0200000, CRC(14b4b5bb) SHA1(d7db5740eec971f2782fb2885ee3af8f2a796550) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b1200.rom",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "b1201.rom",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m1200.rom",    0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END


ROM_START( kov2102 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "u18.102",    0x100000, 0x400000, CRC(a2489c37) SHA1(77ea7cdec211848296dafd45bee1d042133ea2a6) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2_v100_hongkong.asic", 0x000000, 0x04000,  CRC(e0d7679f) SHA1(e1c2d127eba4ddbeb8ad173c55b90ac1467e1ca8) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "u19.101", 0x000000, 0x200000, CRC(8c35f2fe) SHA1(d4858f97fcfad0f342fccbc9cf1590276cc3c69c) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t1200.rom",    0x180000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION( 0x2800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a1200.rom",    0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "a1201.rom",    0x0800000, 0x0800000, CRC(21063ca7) SHA1(cf561b44902425a920d5cbea5bf65dd9530b2289) )
	ROM_LOAD( "a1202.rom",    0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "a1203.rom",    0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "a1204.rom",    0x2000000, 0x0200000, CRC(14b4b5bb) SHA1(d7db5740eec971f2782fb2885ee3af8f2a796550) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b1200.rom",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "b1201.rom",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m1200.rom",    0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END

ROM_START( kov2101 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "u18.101",    0x100000, 0x400000, CRC(c9926f35) SHA1(a9c72d0c5d239164107894c7d3fffe4af29ed201) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2_v100_hongkong.asic", 0x000000, 0x04000,  CRC(e0d7679f) SHA1(e1c2d127eba4ddbeb8ad173c55b90ac1467e1ca8) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "u19.101", 0x000000, 0x200000, CRC(8c35f2fe) SHA1(d4858f97fcfad0f342fccbc9cf1590276cc3c69c) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t1200.rom",    0x180000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION( 0x2800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a1200.rom",    0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "a1201.rom",    0x0800000, 0x0800000, CRC(21063ca7) SHA1(cf561b44902425a920d5cbea5bf65dd9530b2289) )
	ROM_LOAD( "a1202.rom",    0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "a1203.rom",    0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "a1204.rom",    0x2000000, 0x0200000, CRC(14b4b5bb) SHA1(d7db5740eec971f2782fb2885ee3af8f2a796550) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b1200.rom",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "b1201.rom",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m1200.rom",    0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END

ROM_START( kov2100 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "igs_u18.rom",    0x100000, 0x400000, CRC(86205879) SHA1(f73d5b70b41d39be1cac75e474b025de2cce0b01) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2_v100_hongkong.asic", 0x000000, 0x04000,  CRC(e0d7679f) SHA1(e1c2d127eba4ddbeb8ad173c55b90ac1467e1ca8) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "u19.100", 0x000000, 0x200000,   CRC(edd59922) SHA1(09b14f20f685944a93292c83e5830849aade42c9) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t1200.rom",    0x180000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION( 0x2800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a1200.rom",    0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "a1201.rom",    0x0800000, 0x0800000, CRC(21063ca7) SHA1(cf561b44902425a920d5cbea5bf65dd9530b2289) )
	ROM_LOAD( "a1202.rom",    0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "a1203.rom",    0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "a1204.rom",    0x2000000, 0x0200000, CRC(14b4b5bb) SHA1(d7db5740eec971f2782fb2885ee3af8f2a796550) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b1200.rom",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "b1201.rom",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m1200.rom",    0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END



ROM_START( kov2p )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "u8-27322.rom",    0x100000, 0x400000, CRC(3a2cc0de) SHA1(d7511478b34bfb03b2fb5b8268b60502d05b9414) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2p_igs027a_china.bin", 0x000000, 0x04000, CRC(19a0bd95) SHA1(83e9f22512832a51d41c588debe8be7adb3b1df7) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v200-16.rom", 0x000000, 0x200000,  CRC(16a0c11f) SHA1(ce449cef76ebd5657d49b57951e2eb0f132e203e) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t1200.rom",    0x180000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION( 0x2800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a1200.rom",    0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "a1201.rom",    0x0800000, 0x0800000, CRC(21063ca7) SHA1(cf561b44902425a920d5cbea5bf65dd9530b2289) )
	ROM_LOAD( "a1202.rom",    0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "a1203.rom",    0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "a1204.rom",    0x2000000, 0x0200000, CRC(14b4b5bb) SHA1(d7db5740eec971f2782fb2885ee3af8f2a796550) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b1200.rom",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "b1201.rom",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m1200.rom",    0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END

ROM_START( kov2p204 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v204-32m.rom",    0x100000, 0x400000, CRC(583e0650) SHA1(2e5656dd9c6cba9f84af9baa3f5f70cdccf9db47) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2p_igs027a_china.bin", 0x000000, 0x04000, CRC(19a0bd95) SHA1(83e9f22512832a51d41c588debe8be7adb3b1df7) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v200-16.rom", 0x000000, 0x200000,  CRC(16a0c11f) SHA1(ce449cef76ebd5657d49b57951e2eb0f132e203e) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t1200.rom",    0x180000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION( 0x2800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a1200.rom",    0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "a1201.rom",    0x0800000, 0x0800000, CRC(21063ca7) SHA1(cf561b44902425a920d5cbea5bf65dd9530b2289) )
	ROM_LOAD( "a1202.rom",    0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "a1203.rom",    0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "a1204.rom",    0x2000000, 0x0200000, CRC(14b4b5bb) SHA1(d7db5740eec971f2782fb2885ee3af8f2a796550) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b1200.rom",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "b1201.rom",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m1200.rom",    0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END

ROM_START( kov2p202 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v202.bin",    0x100000, 0x400000, CRC(e9b5aa0c) SHA1(39a776c8501e8557d305cfa56c997f9adeb6bcd2) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2p_igs027a_china.bin", 0x000000, 0x04000, CRC(19a0bd95) SHA1(83e9f22512832a51d41c588debe8be7adb3b1df7) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v200-16.rom", 0x000000, 0x200000,  CRC(16a0c11f) SHA1(ce449cef76ebd5657d49b57951e2eb0f132e203e) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t1200.rom",    0x180000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION( 0x2800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a1200.rom",    0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "a1201.rom",    0x0800000, 0x0800000, CRC(21063ca7) SHA1(cf561b44902425a920d5cbea5bf65dd9530b2289) )
	ROM_LOAD( "a1202.rom",    0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "a1203.rom",    0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "a1204.rom",    0x2000000, 0x0200000, CRC(14b4b5bb) SHA1(d7db5740eec971f2782fb2885ee3af8f2a796550) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b1200.rom",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "b1201.rom",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m1200.rom",    0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END

/*

Do Donpachi II
Cave, 2001

This is a PGM cart containing not a lot....
5x SOP44 mask ROMs (4x 64M, 1x 32M)
2x EPROMs (1x 1M, 1x 16M)
2x PALs (labelled FN U14 and FN U15)
1x custom IGS027A (QFP120)
3x RAMs WINBOND W24257AJ-8N
Some logic IC's, resistors, caps etc.

*/

/*
  For ddp2 the China and Japan internal ASIC27 roms were dumped, and confirmed to differ only by the byte at offset 0x2882
  we're assuming this to be true for other regions, this is a fair assumption based on the evidence we have.
*/

#define DDP2_ASIC027_CHINA \
	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */ \
	ROM_LOAD( "ddp2_igs027a_china.bin", 0x000000, 0x04000, CRC(8c566319) SHA1(bb001d8ada56bf446f9ab88e00936501652daf11) ) /* 00 */

#define DDP2_ASIC027_TAIWAN \
	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */ \
	ROM_LOAD( "ddp2_igs027a_taiwan.bin", 0x000000, 0x04000, CRC(1dd34bdc) SHA1(4f2ee38ee37d5cc63b2a63ef51bd392e4f18d836) ) /* 01 */

#define DDP2_ASIC027_JAPAN \
	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */ \
	ROM_LOAD( "ddp2_igs027a_japan.bin", 0x000000, 0x04000, CRC(742d34d2) SHA1(4491c08f3cefef2933ad5a741f4bb05cc2f3e1a0) ) /* 02 */

#define DDP2_ASIC027_KOREA \
	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */ \
	ROM_LOAD( "ddp2_igs027a_korea.bin", 0x000000, 0x04000, CRC(e5a81c17) SHA1(cfe4e28a44a1b3a5c1c9e303941b335dbde7dd8d) ) /* 03 */

#define DDP2_ASIC027_HONG_KONG \
	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */ \
	ROM_LOAD( "ddp2_igs027a_hong_kong.bin", 0x000000, 0x04000, CRC(a7d1cace) SHA1(b9391da52c8234ca0182484f33b5242827c51c76) ) /* 04 */

#define DDP2_ASIC027_WORLD \
	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */ \
	ROM_LOAD( "ddp2_igs027a_world.bin", 0x000000, 0x04000, CRC(3654e20b) SHA1(e00ef1d51efe66354e3eaf4750d9d819c74ddfbf) ) /* 05 */

#define DDP2_COMMON_ROMS \
	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */ \
	ROM_LOAD( "v100.u23", 0x000000, 0x20000, CRC(06c3dd29) SHA1(20c9479f158467fc2037dcf162b6c6be18c91d46) ) \
	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */ \
	PGM_VIDEO_BIOS \
	ROM_LOAD( "t1300.u21",    0x180000, 0x800000, CRC(e748f0cb) SHA1(5843bee3a17c33648ce904af2b98c6a90aff7393) ) \
	ROM_REGION( 0x1000000, "sprcol", 0 ) /* Sprite Colour Data */ \
	ROM_LOAD( "a1300.u1",    0x0000000, 0x0800000, CRC(fc87a405) SHA1(115c21ecc56997652e527c92654076870bc9fa51) ) /* FIXED BITS (xxxxxxxx1xxxxxxx) */ \
	ROM_LOAD( "a1301.u2",    0x0800000, 0x0800000, CRC(0c8520da) SHA1(390317857ae5baa94a4cc042874b00a811f06a63) ) /* FIXED BITS (xxxxxxxx1xxxxxxx) */ \
	ROM_REGION( 0x0800000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */ \
	ROM_LOAD( "b1300.u7",    0x0000000, 0x0800000,  CRC(ef646604) SHA1(d737ff513792962f18df88c2caa9dd71de449079) ) \
	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */ \
	PGM_AUDIO_BIOS \
	ROM_LOAD( "m1300.u5",    0x400000, 0x400000, CRC(82d4015d) SHA1(d4cdc1aec1c97cf23ff7a20ccaad822962e66ffa) )

#define DDP2_PROGRAM_102 \
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */ \
	PGM_68K_BIOS \
	ROM_LOAD16_WORD_SWAP( "v102.u8", 0x100000, 0x200000, CRC(5a9ea040) SHA1(51eaec46c368f7cfc5245e64896092f52b1193e0) )
#define DDP2_PROGRAM_101 \
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */ \
	PGM_68K_BIOS \
	ROM_LOAD16_WORD_SWAP( "v101_16m.u8", 0x100000, 0x200000, CRC(5e5786fd) SHA1(c6fc2956b5dc6a97c0d7d808a8c58aa21fa023b9) )
#define DDP2_PROGRAM_100 \
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */ \
	PGM_68K_BIOS \
	ROM_LOAD16_WORD_SWAP( "v100.u8", 0x100000, 0x200000, CRC(0c8aa8ea) SHA1(57e33224622607a1df8daabf26ba063cf8a6d3fc) )


ROM_START( ddp2 )
	DDP2_PROGRAM_102
	DDP2_ASIC027_WORLD
	DDP2_COMMON_ROMS
ROM_END

ROM_START( ddp2101 )
	DDP2_PROGRAM_101
	DDP2_ASIC027_WORLD
	DDP2_COMMON_ROMS
ROM_END

ROM_START( ddp2100 )
	DDP2_PROGRAM_100
	DDP2_ASIC027_WORLD
	DDP2_COMMON_ROMS
ROM_END


ROM_START( ddp2hk )
	DDP2_PROGRAM_102
	DDP2_ASIC027_HONG_KONG
	DDP2_COMMON_ROMS
ROM_END

ROM_START( ddp2101hk )
	DDP2_PROGRAM_101
	DDP2_ASIC027_HONG_KONG
	DDP2_COMMON_ROMS
ROM_END

ROM_START( ddp2100hk )
	DDP2_PROGRAM_100
	DDP2_ASIC027_HONG_KONG
	DDP2_COMMON_ROMS
ROM_END


ROM_START( ddp2k )
	DDP2_PROGRAM_102
	DDP2_ASIC027_KOREA
	DDP2_COMMON_ROMS
ROM_END

ROM_START( ddp2101k )
	DDP2_PROGRAM_101
	DDP2_ASIC027_KOREA
	DDP2_COMMON_ROMS
ROM_END

ROM_START( ddp2100k )
	DDP2_PROGRAM_100
	DDP2_ASIC027_KOREA
	DDP2_COMMON_ROMS
ROM_END

ROM_START( ddp2j )
	DDP2_PROGRAM_102
	DDP2_ASIC027_JAPAN
	DDP2_COMMON_ROMS
ROM_END

ROM_START( ddp2101j )
	DDP2_PROGRAM_101
	DDP2_ASIC027_JAPAN
	DDP2_COMMON_ROMS
ROM_END

ROM_START( ddp2100j )
	DDP2_PROGRAM_100
	DDP2_ASIC027_JAPAN
	DDP2_COMMON_ROMS
ROM_END

ROM_START( ddp2t )
	DDP2_PROGRAM_102
	DDP2_ASIC027_TAIWAN
	DDP2_COMMON_ROMS
ROM_END

ROM_START( ddp2101t )
	DDP2_PROGRAM_101
	DDP2_ASIC027_TAIWAN
	DDP2_COMMON_ROMS
ROM_END

ROM_START( ddp2100t )
	DDP2_PROGRAM_100
	DDP2_ASIC027_TAIWAN
	DDP2_COMMON_ROMS
ROM_END

ROM_START( ddp2c )
	DDP2_PROGRAM_102
	DDP2_ASIC027_CHINA
	DDP2_COMMON_ROMS
ROM_END

ROM_START( ddp2101c )
	DDP2_PROGRAM_101
	DDP2_ASIC027_CHINA
	DDP2_COMMON_ROMS
ROM_END

ROM_START( ddp2100c )
	DDP2_PROGRAM_100
	DDP2_ASIC027_CHINA
	DDP2_COMMON_ROMS
ROM_END


/*

Dragon World 2001 IGS PGM cart

Top board
---------
PCB Number: IGS PCB-0349-01-FL
22MHz OSC
2x 27C4096 EPROMs at U12, U22
1x 27C160 EPROM at U11
2x PALs at U4, U6
3x 62256 SRAM at U1, U23, U24
IGS027A at U7

Bottom Board
------------
PCB Number: IGS PCB-0350-00T-FL-A
4x 27C160 EPROMs at U2, U3, U7, U9

*/

ROM_START( dw2001 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "2001.u22", 0x100000, 0x80000, CRC(5cabed92) SHA1(d513e353c5c4695b16228e0bda9388c396aa4a81) )

	ROM_REGION( 0x4000, "prot", ROMREGION_ERASEFF ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "dw2001_igs027a_japan.bin", 0x000000, 0x04000, CRC(3a79159b) SHA1(0d693c798ce24c6a749669be8c7b1e4633409e49) )

	ROM_REGION32_LE( 0x4000000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD( "2001.u12", 0x000000, 0x80000, CRC(973db1ab) SHA1(cc35e1a8534fa5d59d888f530769bae4e08c62ca) ) // external ARM data rom (encrypted)

	ROM_REGION( 0x600000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "2001.u11",    0x180000, 0x200000, CRC(b27cf093) SHA1(7c5736a3d72b89742da1c92b2604d66e48b95e56) )

	ROM_REGION( 0x400000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "2001.u2",    0x000000, 0x200000, CRC(d11c733c) SHA1(8faad32e8e215631a2263bdd51a9ae434540d028) )
	ROM_LOAD( "2001.u3",    0x200000, 0x200000, CRC(1435aef2) SHA1(582eb9f6415c89418401be7ebad041adeb600515) )

	ROM_REGION( 0x0200000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "2001.u9",    0x000000, 0x200000, CRC(ccbca572) SHA1(4d3512e82cb65e5cdfcc6cb18deec9f4a6dd350a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "2001.u7",      0x200000, 0x200000, CRC(4ea62f21) SHA1(318f8a1ff5d4ff029a1c4133fe7acc2fc185d112) )
ROM_END


ROM_START( dwpc )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "dwpc_v101jp.u22", 0x100000, 0x80000, CRC(b93027c0) SHA1(602e5f651ccb63e6465ebd7762d8d2dcf7d54077) )

	ROM_REGION( 0x4000, "prot", ROMREGION_ERASEFF ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "dw2001_igs027a_japan.bin", 0x000000, 0x04000, CRC(3a79159b) SHA1(0d693c798ce24c6a749669be8c7b1e4633409e49) )

	ROM_REGION32_LE( 0x4000000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD( "dwpc_v100jp.u12", 0x000000, 0x80000, CRC(0d112126) SHA1(2b569b8ef974d1d9906cc052eee63b869c8d4fa4) ) // external ARM data rom (encrypted)

	ROM_REGION( 0x600000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "dwpc_v100jp.u11",    0x180000, 0x400000, CRC(3aa5a787) SHA1(ef7bb83f7141b24621c86237244fd9f280923ed1) )

	ROM_REGION( 0x400000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "dwpc_v100jp.u2",    0x000000, 0x200000, CRC(e7115763) SHA1(f1bf06e9434a3b962166849f51b9dc3a74d7f2a4) )
	ROM_LOAD( "dwpc_v100jp.u3",    0x200000, 0x200000, CRC(49c184a4) SHA1(320504adf596c38db56247e9cef02e7c7a363ccb) )

	ROM_REGION( 0x0200000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "dwpc_v100jp.u9",    0x000000, 0x200000, CRC(412b9913) SHA1(52fc42a966575e02991aa92382b855744f44854a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "dwpc_v100jp.u7",      0x200000, 0x200000, CRC(5cf9bada) SHA1(c5868a31e09e6909c724411a402d8964c29584fc) )
ROM_END

/*

Puzzli 2
IGS, 2001

Cart for IGS PGM system. The layout of the PCB is virtually identical to Puzzle Star.

PCB Layout
----------

IGS PCB NO- 0259
|-----------------------------------------------|
|                        U6 U7                  |
|         |-------|                             |
|         |IGS027A|                             |
|         |       |                     T0900.U9|
|         |       |                             |
|         |-------|                             |
|          2SP_V200.U3                          |
|          2SP_V200.U4    U5   PAL              |
|                                               |
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      IGS027A     - Custom IGS IC, ARM7/9? based CPU (QFP120, stamped 'IGS027A')
      T0900.U9    - 16MBit MaskROM (SOP44)
      2SP_V200.U3 - MX27C4000 512K x8 EPROM (DIP32, labelled '2SP V200 U3')
      2SP_V200.U4 - MX27C4000 512K x8 EPROM (DIP32, labelled '2SP V200 U4')
      PAL         - AMD PALCE22V10 PAL (DIP24, labelled 'EL U8')
      U5          - Unpopulated position for 16MBit MaskROM (DIP42)
      U6, U7      - Unpopulated position for 74LS245 logic chip (x2)


IGS PCB NO- 0258
|-----------------------------------------------|
|                                               |
|                                               |
|                                               |
|                                               |
|   *    M0900.U2   A0900.U3   B0900.U4         |
|                                               |
|                                               |
|                                               |
|                                               |
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      *  - Unpopulated position for Oki MSM27C3202CZ 32MBit MaskROM (TSOP48 Type II)
      U2 - 32MBit MaskROM (DIP42, Byte mode)
      U3 - 32MBit MaskROM (SOP44)
      U4 - 16MBit MaskROM (SOP44)

*/

ROM_START( puzzli2 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v100.u5",     0x100000, 0x200000, CRC(1abb4595) SHA1(860bb49efc3cb55b6b9846f5ab787d6fd586432d) )

	ROM_REGION( 0x4000, "prot", ROMREGION_ERASEFF ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "puzzli2_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0x600000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0900.u9",    0x180000, 0x200000, CRC(70615611) SHA1(a46d4aa71396947b427f9ba4ba0e636876c09d6b) )

	ROM_REGION( 0x400000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0900.u3",    0x0000000, 0x0400000, CRC(14911251) SHA1(e0d10ef50c408dbcf0907f81d4f0e49aeb651a6c) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x0200000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0900.u4",    0x0000000, 0x0200000,  CRC(6f0638b6) SHA1(14b315fe9e80b3314bb63487e6ea9ce04c9703bd) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0900.u2",    0x400000, 0x400000, CRC(9ea7af2e) SHA1(d2593d391a93c5cf5a554750c32886dea6599b3d) )
ROM_END


ROM_START( puzzli2s )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "2sp_v200.u3",     0x100000, 0x080000, CRC(2a5ba8a6) SHA1(4c87b849fd6f39152e3e2ef699b78ce24b3fb6d0) )
	ROM_LOAD16_BYTE( "2sp_v200.u4",     0x100001, 0x080000, CRC(fa5c86c1) SHA1(11c219722b891b775c0f7f9bc8276cdd8f74d657) )

	ROM_REGION( 0x4000, "prot", ROMREGION_ERASEFF ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "puzzli2_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0x600000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0900.u9",    0x180000, 0x200000, CRC(70615611) SHA1(a46d4aa71396947b427f9ba4ba0e636876c09d6b) )

	ROM_REGION( 0x400000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0900.u3",    0x0000000, 0x0400000, CRC(14911251) SHA1(e0d10ef50c408dbcf0907f81d4f0e49aeb651a6c) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x0200000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0900.u4",    0x0000000, 0x0200000,  CRC(6f0638b6) SHA1(14b315fe9e80b3314bb63487e6ea9ce04c9703bd) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0900.u2",    0x400000, 0x400000, CRC(9ea7af2e) SHA1(d2593d391a93c5cf5a554750c32886dea6599b3d) )
ROM_END


/*

Martial Masters
IGS, 2001

Cart for IGS PGM system. This game is a straight rip-off of any of the
late side-by-side fighting games made by SNK or Capcom such as King Of Fighters
or Super Street Fighter II etc


PCB Layout
----------

IGS PCB-0293-01
|-----------------------------------------------|
| 62256              62256         IGS027A      |
| 62256                                         |
|                      *                        |
|                                               |
| PAL                                           |
|                                               |
| PAL             V102_16M.U10  T1000.U3        |
|                                               |
|                 V104_32M.U9              22MHz|
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      62256        - 32K x8 SRAM (SOJ28)
      IGS027A      - Custom IGS IC, ARM7 based CPU with internal 64K ROM (QFP120)
      T1000.U3     - 23C6410 64MBit MaskROM (SOP44)
      V102_16M.U10 - MX29F1610MC 16MBit SOP44 FlashROM mounted onto a tiny DIP42 to SOP44 adapter board
                     (manufactured by IGS) which is plugged into a standard DIP42 socket. This chip was
                     read directly on the adapter as a 27C160 EPROM. The socket is wired to accept 32MBit
                     DIP42 EPROMs.
      V104_32M.U9  - M27C3202CZ 32MBit TSOP48 Type II OTP MaskROM mounted onto a tiny DIP42 to TSOP48 Type II
                     adapter board (manufactured by IGS) which is plugged into a standard DIP42 socket. This
                     chip was read directly on the adapter as a 27C322 EPROM. The socket is wired to accept
                     32MBit DIP42 EPROMs.
      *            - Unpopulated position for 62256 SRAM


IGS PCB-0292-00
|-----------------------------------------------|
| A1000.U3         A1002.U6           A1004.U10 |
|          A1001.U4         A1003.U8            |
|                                               |
|                                               |
|                                               |
|                                               |
|                  M1001.U7           B1001.U11 |
|          M1000.U5         B1000.U9            |
|                                               |
|-|                                           |-|
  |--------------------||---------------------|



*/

ROM_START( martmast )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v104_32m.u9",    0x100000, 0x400000, CRC(cfd9dff4) SHA1(328eaf6ac49a73265ee4e0f992b1b1312f49877b) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "martial_masters_v102_usa.asic", 0x000000, 0x04000, CRC(a6c0828c) SHA1(0a5bda56dca264c3c7ff7698b8f699563f203c4d) ) // not verified, could be bad

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v102_16m.u10", 0x000000, 0x200000,  CRC(18b745e6) SHA1(7bcb58dd3a2d6072f492cf0dd7181cb061c1f49d) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t1000.u3",    0x180000, 0x800000, CRC(bbf879b5) SHA1(bd9a6aea34ad4001e89e62ff4b7a2292eb833c00) )

	ROM_REGION( 0x2800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a1000.u3",    0x0000000, 0x0800000, CRC(43577ac8) SHA1(6eea8b455985d5bac74dcc9943cdc3c0902de6cc) )
	ROM_LOAD( "a1001.u4",    0x0800000, 0x0800000, CRC(fe7a476f) SHA1(a8c7f1f0dd3e53141aed6d927eb88a3ceebb81e4) )
	ROM_LOAD( "a1002.u6",    0x1000000, 0x0800000, CRC(62e33d38) SHA1(96163d583e25073594f8413ce263e56b66bd69a1) )
	ROM_LOAD( "a1003.u8",    0x1800000, 0x0800000, CRC(b2c4945a) SHA1(7b18287a2db56db3651cfd4deb607af53522fefd) )
	ROM_LOAD( "a1004.u10",   0x2000000, 0x0400000, CRC(9fd3f5fd) SHA1(057531f91062be51589c6cf8f4170089b9be6380) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b1000.u9",    0x0000000, 0x0800000,  CRC(c5961f6f) SHA1(a68060b10edbd084cbde79d2ed1c9084777beb10) )
	ROM_LOAD( "b1001.u11",   0x0800000, 0x0800000,  CRC(0b7e1c06) SHA1(545e15e0087f8621d593fecd8b4013f7ca311686) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m1000.u5",    0x400000, 0x800000, CRC(ed407ae8) SHA1(a6e9c09b39c13e8fb7fbc89fa9f823cbeb66e901) )
	ROM_LOAD( "m1001.u7",    0xc00000, 0x400000, CRC(662d2d48) SHA1(2fcc3099d9c04456cae3b13035fb28eaf709e7d8) )
ROM_END

ROM_START( martmastc )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v104_32m.u9",    0x100000, 0x400000, CRC(cfd9dff4) SHA1(328eaf6ac49a73265ee4e0f992b1b1312f49877b) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "martial_masters_v101_cn.asic", 0x000000, 0x04000, CRC(b3e25b7d) SHA1(6147d7ee2e11636521df1bb96ed5da8ad21b2a57) ) // not verified, could be bad

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v102_16m.u10", 0x000000, 0x200000,  CRC(18b745e6) SHA1(7bcb58dd3a2d6072f492cf0dd7181cb061c1f49d) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t1000.u3",    0x180000, 0x800000, CRC(bbf879b5) SHA1(bd9a6aea34ad4001e89e62ff4b7a2292eb833c00) )

	ROM_REGION( 0x2800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a1000.u3",    0x0000000, 0x0800000, CRC(43577ac8) SHA1(6eea8b455985d5bac74dcc9943cdc3c0902de6cc) )
	ROM_LOAD( "a1001.u4",    0x0800000, 0x0800000, CRC(fe7a476f) SHA1(a8c7f1f0dd3e53141aed6d927eb88a3ceebb81e4) )
	ROM_LOAD( "a1002.u6",    0x1000000, 0x0800000, CRC(62e33d38) SHA1(96163d583e25073594f8413ce263e56b66bd69a1) )
	ROM_LOAD( "a1003.u8",    0x1800000, 0x0800000, CRC(b2c4945a) SHA1(7b18287a2db56db3651cfd4deb607af53522fefd) )
	ROM_LOAD( "a1004.u10",   0x2000000, 0x0400000, CRC(9fd3f5fd) SHA1(057531f91062be51589c6cf8f4170089b9be6380) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b1000.u9",    0x0000000, 0x0800000,  CRC(c5961f6f) SHA1(a68060b10edbd084cbde79d2ed1c9084777beb10) )
	ROM_LOAD( "b1001.u11",   0x0800000, 0x0800000,  CRC(0b7e1c06) SHA1(545e15e0087f8621d593fecd8b4013f7ca311686) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m1000.u5",    0x400000, 0x800000, CRC(ed407ae8) SHA1(a6e9c09b39c13e8fb7fbc89fa9f823cbeb66e901) )
	ROM_LOAD( "m1001.u7",    0xc00000, 0x400000, CRC(662d2d48) SHA1(2fcc3099d9c04456cae3b13035fb28eaf709e7d8) )
ROM_END

ROM_START( martmastc102 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "martmast_u9-v102.322",    0x100000, 0x400000, CRC(bb24b92a) SHA1(442cb9e3f51727be82f71c078c5c3e49dc1a23f0) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "martial_masters_v101_cn.asic", 0x000000, 0x04000, CRC(b3e25b7d) SHA1(6147d7ee2e11636521df1bb96ed5da8ad21b2a57) ) // not verified, could be bad

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "martmast_u10-v101.160", 0x000000, 0x200000,   CRC(d5d93294) SHA1(58d0a99749f7dc05814892b508ce21b160410947) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t1000.u3",    0x180000, 0x800000, CRC(bbf879b5) SHA1(bd9a6aea34ad4001e89e62ff4b7a2292eb833c00) )

	ROM_REGION( 0x2800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a1000.u3",    0x0000000, 0x0800000, CRC(43577ac8) SHA1(6eea8b455985d5bac74dcc9943cdc3c0902de6cc) )
	ROM_LOAD( "a1001.u4",    0x0800000, 0x0800000, CRC(fe7a476f) SHA1(a8c7f1f0dd3e53141aed6d927eb88a3ceebb81e4) )
	ROM_LOAD( "a1002.u6",    0x1000000, 0x0800000, CRC(62e33d38) SHA1(96163d583e25073594f8413ce263e56b66bd69a1) )
	ROM_LOAD( "a1003.u8",    0x1800000, 0x0800000, CRC(b2c4945a) SHA1(7b18287a2db56db3651cfd4deb607af53522fefd) )
	ROM_LOAD( "a1004.u10",   0x2000000, 0x0400000, CRC(9fd3f5fd) SHA1(057531f91062be51589c6cf8f4170089b9be6380) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b1000.u9",    0x0000000, 0x0800000,  CRC(c5961f6f) SHA1(a68060b10edbd084cbde79d2ed1c9084777beb10) )
	ROM_LOAD( "b1001.u11",   0x0800000, 0x0800000,  CRC(0b7e1c06) SHA1(545e15e0087f8621d593fecd8b4013f7ca311686) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m1000.u5",    0x400000, 0x800000, CRC(ed407ae8) SHA1(a6e9c09b39c13e8fb7fbc89fa9f823cbeb66e901) )
	ROM_LOAD( "m1001.u7",    0xc00000, 0x400000, CRC(662d2d48) SHA1(2fcc3099d9c04456cae3b13035fb28eaf709e7d8) )
ROM_END


/*

Demon Front
IGS, 2002

Cart for IGS PGM system. This game is a straight rip-off of Metal Slug.

PCB Layout
----------

IGS PCB-0387-02-FV
|-----------------------------------------------|
| BS616LV1010                      IGS027A      |
| BS616LV1010                                   |
|                                               |
|                              *     BS616LV1010|
|            PAL  PAL                           |
|                                               |
| V102_16M.U5        V101_32M.U26               |
|                                        PAL    |
|                             T04501.U29   22MHz|
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      BS616LV1010  - 64K x16 SRAM (TSOP44)
      IGS027A      - Custom IGS IC, ARM7 based CPU (QFP120)
      T04501.U29   - 23C6410 64MBit MaskROM (SOP44)
      V102_16M.U5  - 27C160 16MBit EPROM (DIP42)
      V101_32M.U26 - 27C322 32MBit EPROM (DIP42)
      *            - Unpopulated position for 29F1610 16MBit SOP44 FlashROM, linked to IGS027A


IGS PCB-0390-00-FV-A
|-----------------------------------------------|
| A04501.U3  A04502.U4  A04503.U6   U8*     U10*|
|                                               |
|                                               |
|                                               |
|                                               |
|     W04501.U5   U7*    B04501.U9   B04502.U11 |
|                                               |
|                                               |
|                                               |
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      *  - Unpopulated SOP44 ROM position.

*/


ROM_START( dmnfrnt )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v105_16m.u5",    0x100000, 0x200000, CRC(bda083bd) SHA1(58d6438737a2c43aa8bbcb7f34fb51375b781b1c) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "dmnfrnt_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data, internal missing) */
	ROM_LOAD( "v105_32m.u26", 0x000000, 0x400000,  CRC(c798c2ef) SHA1(91e364c33b935293fa765ca521cdb67ac45ec70f) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t04501.u29",    0x180000, 0x800000, CRC(900eaaac) SHA1(4033cb7b28fcadb92d5af3ea7fdd1c22747618fd) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04501.u3",    0x0000000, 0x0800000, CRC(9741bea6) SHA1(e3e904249be228628c8c2bd3495cda23586dc048) )
	ROM_LOAD( "a04502.u4",    0x0800000, 0x0800000, CRC(e104f405) SHA1(124b3deed3e838f8bae6c7d78bdd788859597585) )
	ROM_LOAD( "a04503.u6",    0x1000000, 0x0800000, CRC(bfd5cfe3) SHA1(fbe4c0a2987c2036df707b86597d78124ee2e665) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04501.u9",    0x0000000, 0x0800000,  CRC(29320b7d) SHA1(59c78805e666f912df201c34616744f46057937b) )
	ROM_LOAD( "b04502.u11",   0x0800000, 0x0200000,  CRC(578c00e9) SHA1(14235cc8b0f8c7dd659512f017a2d4aacd91d89d) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "w04501.u5",    0x400000, 0x800000, CRC(3ab58137) SHA1(b221f7e551ff0bfa3fd97b6ebedbac69442a66e9) )
ROM_END

ROM_START( dmnfrnta )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v102_16m.u5",    0x100000, 0x200000, CRC(3d4d481a) SHA1(95953b8f31343389405cc722b4177ff5adf67b62) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "dmnfrnt_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data, internal missing) */
	ROM_LOAD( "v101_32m.u26", 0x000000, 0x400000,  CRC(93965281) SHA1(89da198aaa7ca759cb96b5f18859a477e55fd590) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t04501.u29",    0x180000, 0x800000, CRC(900eaaac) SHA1(4033cb7b28fcadb92d5af3ea7fdd1c22747618fd) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04501.u3",    0x0000000, 0x0800000, CRC(9741bea6) SHA1(e3e904249be228628c8c2bd3495cda23586dc048) )
	ROM_LOAD( "a04502.u4",    0x0800000, 0x0800000, CRC(e104f405) SHA1(124b3deed3e838f8bae6c7d78bdd788859597585) )
	ROM_LOAD( "a04503.u6",    0x1000000, 0x0800000, CRC(bfd5cfe3) SHA1(fbe4c0a2987c2036df707b86597d78124ee2e665) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04501.u9",    0x0000000, 0x0800000,  CRC(29320b7d) SHA1(59c78805e666f912df201c34616744f46057937b) )
	ROM_LOAD( "b04502.u11",   0x0800000, 0x0200000,  CRC(578c00e9) SHA1(14235cc8b0f8c7dd659512f017a2d4aacd91d89d) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "w04501.u5",    0x400000, 0x800000, CRC(3ab58137) SHA1(b221f7e551ff0bfa3fd97b6ebedbac69442a66e9) )
ROM_END


ROM_START( dmnfrntb )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v103_16m.u5",    0x100000, 0x200000, CRC(2ddafb3d) SHA1(c7d22e007952459de6d23a42ce32aab67b493fc3) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "dmnfrnt_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data, internal missing) */
	ROM_LOAD( "v103_32m.u26", 0x000000, 0x400000, CRC(e78383a3) SHA1(7ae99e93489e79fb1e4240124d22b6002fb7fe18) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t04501.u29",    0x180000, 0x800000, CRC(900eaaac) SHA1(4033cb7b28fcadb92d5af3ea7fdd1c22747618fd) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04501.u3",    0x0000000, 0x0800000, CRC(9741bea6) SHA1(e3e904249be228628c8c2bd3495cda23586dc048) )
	ROM_LOAD( "a04502.u4",    0x0800000, 0x0800000, CRC(e104f405) SHA1(124b3deed3e838f8bae6c7d78bdd788859597585) )
	ROM_LOAD( "a04503.u6",    0x1000000, 0x0800000, CRC(bfd5cfe3) SHA1(fbe4c0a2987c2036df707b86597d78124ee2e665) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04501.u9",    0x0000000, 0x0800000,  CRC(29320b7d) SHA1(59c78805e666f912df201c34616744f46057937b) )
	ROM_LOAD( "b04502.u11",   0x0800000, 0x0200000,  CRC(578c00e9) SHA1(14235cc8b0f8c7dd659512f017a2d4aacd91d89d) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "w04501.u5",    0x400000, 0x800000, CRC(3ab58137) SHA1(b221f7e551ff0bfa3fd97b6ebedbac69442a66e9) )
ROM_END

ROM_START( dmnfrntpcb )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p02s.u42",    0x00000, 0x020000, CRC(78c15fa2) SHA1(885a6558e022602cc6f482ac9667ba9f61e75092) ) /* IGS PGM P02S 1A3708A1A0 S002838  (uses standard PGM v2 bios) */
	ROM_LOAD16_WORD_SWAP( "demonfront_v107-u43.bin",    0x100000, 0x200000,  CRC(671d8a31) SHA1(a0c2af67d7c56b4b355883892a47640fc72408a1) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "dmnfrnt_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data, internal missing) */
	ROM_LOAD( "demonfront_v107-u62.bin",     0x000000, 0x400000, CRC(cb94772e) SHA1(4213600be41fd9ea295dd308920b1d89b635724f) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t04501.u29",    0x180000, 0x800000, CRC(900eaaac) SHA1(4033cb7b28fcadb92d5af3ea7fdd1c22747618fd) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04501.u3",    0x0000000, 0x0800000, CRC(9741bea6) SHA1(e3e904249be228628c8c2bd3495cda23586dc048) )
	ROM_LOAD( "a04502.u4",    0x0800000, 0x0800000, CRC(e104f405) SHA1(124b3deed3e838f8bae6c7d78bdd788859597585) )
	ROM_LOAD( "a04503.u6",    0x1000000, 0x0800000, CRC(bfd5cfe3) SHA1(fbe4c0a2987c2036df707b86597d78124ee2e665) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04501.u9",    0x0000000, 0x0800000,  CRC(29320b7d) SHA1(59c78805e666f912df201c34616744f46057937b) )
	ROM_LOAD( "b04502.u11",   0x0800000, 0x0200000,  CRC(578c00e9) SHA1(14235cc8b0f8c7dd659512f017a2d4aacd91d89d) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "w04501.u5",    0x400000, 0x800000, CRC(3ab58137) SHA1(b221f7e551ff0bfa3fd97b6ebedbac69442a66e9) )
ROM_END

// the (readable part of) internal roms have been verified on
// Japan PCB
// Overseas Cart
// the two dumps differed by the region byte and internal checksum byte

ROM_START( theglad )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v101.u6",      0x100000, 0x080000, CRC(f799e866) SHA1(dccc3c903357c40c3cf85ac0ae8fc12fb0f853a6) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "theglad_igs027a_execute_only_area", 0x0000, 0x00188, NO_DUMP )
	ROM_LOAD( "theglad_igs027a_v100_overseas.bin", 0x0188, 0x3e78, CRC(02fe6f52) SHA1(0b0ddf4507856cfc5b7d4ef7e4c5375254c2a024) )

	ROM_REGION( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data, internal missing) */
	ROM_LOAD( "v107.u26", 0x000000, 0x200000,  CRC(f7c61357) SHA1(52d31c464dfc83c5371b078cb6b73c0d0e0d57e3) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t04601.u33",   0x180000, 0x800000, CRC(e5dab371) SHA1(2e3c93958eb0326b6b84b95c2168626f26bbac76) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04601.u2",    0x0000000, 0x0800000,  CRC(d9b2e004) SHA1(8e1882b800fe9f12d7d49303e7417ba5b6f8ef85) )
	ROM_LOAD( "a04602.u4",    0x0800000, 0x0800000,  CRC(14f22308) SHA1(7fad54704e8c97eab723f53dfb50fb3e7bb606d2) )
	ROM_LOAD( "a04603.u6",    0x1000000, 0x0800000,  CRC(8f621e17) SHA1(b0f87f378e0115d0c95017ca0f1b0d508827a7c6) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04601.u11",   0x0000000, 0x0800000, CRC(ee72bccf) SHA1(73c25fe659f6c903447066e4ef83d2f580449d76) )
	ROM_LOAD( "b04602.u12",   0x0800000, 0x0400000, CRC(7dba9c38) SHA1(a03d509274e8f6a500a7ebe2da5aab8bed4e7f2f) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "w04601.u1",    0x400000, 0x800000, CRC(5f15ddb3) SHA1(c38dcef8e06802a84e42a7fc9fa505475fc3ac65) )
ROM_END

ROM_START( theglad100 ) // is this actually a pre-v100 proto?
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "u6.rom",       0x100000, 0x080000, CRC(14c85212) SHA1(8d2489708e176a2c460498a13173be01f645b79e) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "theglad_igs027a_execute_only_area", 0x0000, 0x00188, NO_DUMP )
	ROM_LOAD( "theglad_igs027a_older.bin", 0x0188, 0x3e78, BAD_DUMP CRC(02fe6f52) SHA1(0b0ddf4507856cfc5b7d4ef7e4c5375254c2a024) ) // this is wrong for this set, we patch it to work

	ROM_REGION( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data, internal missing) */
	ROM_LOAD( "u2.rom", 0x000000, 0x200000,  CRC(c7bcf2ae) SHA1(10bc012c83987f594d5375a51bc4be2e17568a81) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t04601.u33",   0x180000, 0x800000, CRC(e5dab371) SHA1(2e3c93958eb0326b6b84b95c2168626f26bbac76) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04601.u2",    0x0000000, 0x0800000,  CRC(d9b2e004) SHA1(8e1882b800fe9f12d7d49303e7417ba5b6f8ef85) )
	ROM_LOAD( "a04602.u4",    0x0800000, 0x0800000,  CRC(14f22308) SHA1(7fad54704e8c97eab723f53dfb50fb3e7bb606d2) )
	ROM_LOAD( "a04603.u6",    0x1000000, 0x0800000,  CRC(8f621e17) SHA1(b0f87f378e0115d0c95017ca0f1b0d508827a7c6) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04601.u11",   0x0000000, 0x0800000, CRC(ee72bccf) SHA1(73c25fe659f6c903447066e4ef83d2f580449d76) )
	ROM_LOAD( "b04602.u12",   0x0800000, 0x0400000, CRC(7dba9c38) SHA1(a03d509274e8f6a500a7ebe2da5aab8bed4e7f2f) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "w04601.u1",    0x400000, 0x800000, CRC(5f15ddb3) SHA1(c38dcef8e06802a84e42a7fc9fa505475fc3ac65) )
ROM_END


ROM_START( theglad101 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v100.u6",       0x100000, 0x080000, CRC(bcf3b172) SHA1(df7e2808c0341be0a59eefa852c857a3a919223e) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "theglad_igs027a_execute_only_area", 0x0000, 0x00188, NO_DUMP )
	ROM_LOAD( "theglad_igs027a_v100_overseas.bin", 0x0188, 0x3e78, CRC(02fe6f52) SHA1(0b0ddf4507856cfc5b7d4ef7e4c5375254c2a024) )

	ROM_REGION( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data, internal missing) */
	ROM_LOAD( "v101.u26", 0x000000, 0x200000, CRC(23faec02) SHA1(9065d55c2a14e6889e735a452bbc32530056645a) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t04601.u33",   0x180000, 0x800000, CRC(e5dab371) SHA1(2e3c93958eb0326b6b84b95c2168626f26bbac76) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04601.u2",    0x0000000, 0x0800000,  CRC(d9b2e004) SHA1(8e1882b800fe9f12d7d49303e7417ba5b6f8ef85) )
	ROM_LOAD( "a04602.u4",    0x0800000, 0x0800000,  CRC(14f22308) SHA1(7fad54704e8c97eab723f53dfb50fb3e7bb606d2) )
	ROM_LOAD( "a04603.u6",    0x1000000, 0x0800000,  CRC(8f621e17) SHA1(b0f87f378e0115d0c95017ca0f1b0d508827a7c6) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04601.u11",   0x0000000, 0x0800000, CRC(ee72bccf) SHA1(73c25fe659f6c903447066e4ef83d2f580449d76) )
	ROM_LOAD( "b04602.u12",   0x0800000, 0x0400000, CRC(7dba9c38) SHA1(a03d509274e8f6a500a7ebe2da5aab8bed4e7f2f) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "w04601.u1",    0x400000, 0x800000, CRC(5f15ddb3) SHA1(c38dcef8e06802a84e42a7fc9fa505475fc3ac65) )
ROM_END


ROM_START( thegladpcb )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "bios.42",    0x000000, 0x020000, CRC(517cf7a2) SHA1(f5720b29e3be6ec22be03a768618cb2a1aa4ade7) )
	ROM_LOAD16_WORD_SWAP( "glad_v100.43",  0x100000, 0x080000, CRC(bcf3b172) SHA1(df7e2808c0341be0a59eefa852c857a3a919223e) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "thegladpcb_igs027a_execute_only_area", 0x0000, 0x00188, NO_DUMP )
	ROM_LOAD( "thegladpcb_igs027a_v100_japan.bin", 0x0188, 0x3e78, CRC(d7f06e2d) SHA1(9c3aca7a487f5329d84731e2c63d5ed591bf9d24) )    // from 'thegladpcb set'

	ROM_REGION( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data, internal missing) */
	ROM_LOAD( "igs_v100.62", 0x000000, 0x200000, CRC(0f3f511e) SHA1(28dd8d27495cec86e968a3ea549c5b30513dbb6e) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.u72", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // standard PGM tx bios
	ROM_LOAD( "t04601.u71",   0x180000, 0x800000, CRC(e5dab371) SHA1(2e3c93958eb0326b6b84b95c2168626f26bbac76) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04601.u30",    0x0000000, 0x0800000,  CRC(d9b2e004) SHA1(8e1882b800fe9f12d7d49303e7417ba5b6f8ef85) )
	ROM_LOAD( "a04602.u31",    0x0800000, 0x0800000,  CRC(14f22308) SHA1(7fad54704e8c97eab723f53dfb50fb3e7bb606d2) )
	ROM_LOAD( "a04603.u32",    0x1000000, 0x0800000,  CRC(8f621e17) SHA1(b0f87f378e0115d0c95017ca0f1b0d508827a7c6) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04601.u40",   0x0000000, 0x0800000, CRC(ee72bccf) SHA1(73c25fe659f6c903447066e4ef83d2f580449d76) )
	ROM_LOAD( "b04602.u41",   0x0800000, 0x0400000, CRC(7dba9c38) SHA1(a03d509274e8f6a500a7ebe2da5aab8bed4e7f2f) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.u4", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // standard PGM sample bios
	ROM_LOAD( "w04601.u8",    0x400000, 0x800000, CRC(5f15ddb3) SHA1(c38dcef8e06802a84e42a7fc9fa505475fc3ac65) )
	ROM_LOAD( "igs29.bin",    0xc00000, 0x200000, CRC(51acb395) SHA1(65a2ecd3de2ff782f2aa0f0f905f9b18323aea64) ) // extra ROM on the PCB version for the Japanese music
ROM_END

ROM_START( oldsplus )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p05301.rom",   0x100000, 0x400000, CRC(923f7246) SHA1(818ade79e9724f5a2b0cc5a647ae5d4ee0374799) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "oldsplus_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0x800000, "user1", ROMREGION_ERASE00 )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t05301.rom",   0x180000, 0x800000, CRC(8257bbb0) SHA1(b48067b7e7081a15fddf21739b641d677c2df3d9) )

	ROM_REGION( 0x2000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a05301.rom",   0x0000000, 0x0800000, CRC(57946fd2) SHA1(5d79bc71a1881f3099821a9b255a5f271e0eeff6) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a05302.rom",   0x0800000, 0x0800000, CRC(3459a0b8) SHA1(94ab6f980b5582f1db9bb12019d03f0b6e0a06df) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a05303.rom",   0x1000000, 0x0800000, CRC(13475d85) SHA1(4683a3bf304fdc15ffb1c61b7957ad68b023fa33) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a05304.rom",   0x1800000, 0x0800000, CRC(f03ef7a6) SHA1(c18b1b622b430d5e031e65daa6819b84c3e12ef5) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b05301.rom",   0x0000000, 0x0800000, CRC(fd98f503) SHA1(02046ab1aa89f63bff149003d9d61776e025a92a) )
	ROM_LOAD( "b05302.rom",   0x0800000, 0x0800000, CRC(9f6094a8) SHA1(69f6f2003ab975eae13ea6b5c2ffa40df6e6bdf6) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m05301.rom",   0x400000, 0x400000, CRC(86ec83bc) SHA1(067cb7ec449eacd1f49298f45a364368934db5dd) )
ROM_END

ROM_START( kovshp )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0600h_101.rom",   0x100000, 0x400000, CRC(e1d89a19) SHA1(30e11c145652d03464b14d3cd09e4f35fff6120e) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsh_v100_china.asic", 0x000000, 0x04000, BAD_DUMP CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) ) // this is the kovsh ARM rom, we intercept and modify protection calls

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0600.rom",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION( 0x2000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0540.rom",    0x1800000, 0x0800000, CRC(4fd3413e) SHA1(5e8f3e421342bf558c77e59635f9b5d713e825c2) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0540.rom",    0x0800000, 0x0800000, CRC(60999757) SHA1(118cf058e67858958bcb931e14f5d160c7de87cc) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kovshpa )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0600h.rom",   0x100000, 0x400000, CRC(e251e8e4) SHA1(af5b7c81632a39e1450d932951bed634c76b84e8) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsh_v100_china.asic", 0x000000, 0x04000, BAD_DUMP CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) ) // this is the kovsh ARM rom, we intercept and modify protection calls

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0600.rom",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION( 0x2000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0540.rom",    0x1800000, 0x0800000, CRC(4fd3413e) SHA1(5e8f3e421342bf558c77e59635f9b5d713e825c2) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0540.rom",    0x0800000, 0x0800000, CRC(60999757) SHA1(118cf058e67858958bcb931e14f5d160c7de87cc) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END



ROM_START( kovytzy )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "ytzy_v201cn.rom",   0x100000, 0x400000, CRC(f3705ea0) SHA1(e31ad474d0c2364311d21a8ce37d49919c7b999c) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsh_v100_china.asic", 0x000000, 0x04000, BAD_DUMP CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) ) // this is the kovsh ARM rom, we intercept and modify protection calls

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0600.rom",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION( 0x2000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0540.rom",    0x1800000, 0x0800000, CRC(4fd3413e) SHA1(5e8f3e421342bf558c77e59635f9b5d713e825c2) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0540.rom",    0x0800000, 0x0800000, CRC(60999757) SHA1(118cf058e67858958bcb931e14f5d160c7de87cc) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kovshxas )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "de_p0609.322",   0x100000, 0x400000, CRC(3b7b627f) SHA1(b331148501f9349fbd5882fb3f184f6304e58646) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsh_v100_china.asic", 0x000000, 0x04000, BAD_DUMP CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) ) // this is the kovsh ARM rom, we intercept and modify protection calls

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0600.rom",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION( 0x2000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0603xas.rom",    0x1800000, 0x0800000, CRC(7057b37e) SHA1(85a19f23303b4d581c4fa315b2c204af92fcb424) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0601xas.rom",    0x0800000, 0x0800000, CRC(3784fb49) SHA1(7e85fe5b5fb8746f1321c03ad2350d2a58969d7a) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END


ROM_START( kovlsqh )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "lsqh_v200cn.rom", 0x100000, 0x400000, CRC(9935a27a) SHA1(3075935293172466c4bd997dcb67f864ae26493e) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsh_v100_china.asic", 0x000000, 0x04000, BAD_DUMP CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) ) // this is the kovsh ARM rom, we intercept and modify protection calls

	ROM_REGION( 0xc00000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "lsqh2_t01.rom",0x180000, 0x800000, CRC(d498d97f) SHA1(97a7b6d2ed1170449e7c2899448af7cbbca4c94f) )
	ROM_IGNORE( 0x800000 )  // second half identical

	ROM_REGION( 0x3000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "lsqh2_a01.rom", 0x0000000, 0x1000000, CRC(25ae3efd) SHA1(083d977602ddb5ad54fbdcba000cd4287de8d463) )
	ROM_LOAD( "lsqh2_a23.rom", 0x1000000, 0x1000000, CRC(7a779112) SHA1(0a7d36b3715063d8eac629b95a9bb3ecd8e54fca) )
	ROM_LOAD( "lsqh2_a45.rom", 0x2000000, 0x1000000, CRC(5d7de052) SHA1(7663b6cf09f65c4644661005a38f9aba84a32913) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "lsqh2_b01.rom", 0x0000000, 0x1000000, CRC(df7ca696) SHA1(7af3d27957a39de7e4873867c9972c05af7e7964) )

	ROM_REGION( 0xc00000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "lsqh2_m01.rom",0x400000, 0x400000, CRC(01af1b65) SHA1(6cf523fa8f1e03f974771611bb9a4e08a4d4443f) )
	ROM_IGNORE( 0x400000 )  // 400000-7fffff empty
	ROM_CONTINUE( 0x800000, 0x400000 )
	ROM_IGNORE( 0x400000 )  // c00000-ffffff empty
ROM_END

ROM_START( kovlsqh2 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "lsqh2_prg.rom",0x100000, 0x400000, CRC(d71e3d50) SHA1(bda78648bc176b0ded74118a8e340ee661bb930d) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsh_v100_china.asic", 0x000000, 0x04000, BAD_DUMP CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) ) // this is the kovsh ARM rom, we intercept and modify protection calls

	ROM_REGION( 0xc00000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "lsqh2_t01.rom",0x180000, 0x800000, CRC(d498d97f) SHA1(97a7b6d2ed1170449e7c2899448af7cbbca4c94f) )
	ROM_IGNORE( 0x800000 )  // second half identical

	ROM_REGION( 0x3000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "lsqh2_a01.rom", 0x0000000, 0x1000000, CRC(25ae3efd) SHA1(083d977602ddb5ad54fbdcba000cd4287de8d463) )
	ROM_LOAD( "lsqh2_a23.rom", 0x1000000, 0x1000000, CRC(7a779112) SHA1(0a7d36b3715063d8eac629b95a9bb3ecd8e54fca) )
	ROM_LOAD( "lsqh2_a45.rom", 0x2000000, 0x1000000, CRC(5d7de052) SHA1(7663b6cf09f65c4644661005a38f9aba84a32913) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "lsqh2_b01.rom", 0x0000000, 0x1000000, CRC(df7ca696) SHA1(7af3d27957a39de7e4873867c9972c05af7e7964) )

	ROM_REGION( 0xc00000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "lsqh2_m01.rom",0x400000, 0x400000, CRC(01af1b65) SHA1(6cf523fa8f1e03f974771611bb9a4e08a4d4443f) )
	ROM_IGNORE( 0x400000 )  // 400000-7fffff empty
	ROM_CONTINUE( 0x800000, 0x400000 )
	ROM_IGNORE( 0x400000 )  // c00000-ffffff empty
ROM_END


ROM_START( kovlsjb )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "lsjb_prg.rom", 0x100000, 0x400000, CRC(adf06b37) SHA1(be3c0af64de374046d28492ac49c01da1ec78e40) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsh_v100_china.asic", 0x000000, 0x04000, BAD_DUMP CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) ) // this is the kovsh ARM rom, we intercept and modify protection calls

	ROM_REGION( 0xc00000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "lsqh2_t01.rom",0x180000, 0x800000, CRC(d498d97f) SHA1(97a7b6d2ed1170449e7c2899448af7cbbca4c94f) )
	ROM_IGNORE( 0x800000 )  // second half identical

	ROM_REGION( 0x3000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "lsqh2_a01.rom", 0x0000000, 0x1000000, CRC(25ae3efd) SHA1(083d977602ddb5ad54fbdcba000cd4287de8d463) )
	ROM_LOAD( "lsqh2_a23.rom", 0x1000000, 0x1000000, CRC(7a779112) SHA1(0a7d36b3715063d8eac629b95a9bb3ecd8e54fca) )
	ROM_LOAD( "lsqh2_a45.rom", 0x2000000, 0x1000000, CRC(5d7de052) SHA1(7663b6cf09f65c4644661005a38f9aba84a32913) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "lsqh2_b01.rom", 0x0000000, 0x1000000, CRC(df7ca696) SHA1(7af3d27957a39de7e4873867c9972c05af7e7964) )

	ROM_REGION( 0xc00000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "lsqh2_m01.rom",0x400000, 0x400000, CRC(01af1b65) SHA1(6cf523fa8f1e03f974771611bb9a4e08a4d4443f) )
	ROM_IGNORE( 0x400000 )  // 400000-7fffff empty
	ROM_CONTINUE( 0x800000, 0x400000 )
	ROM_IGNORE( 0x400000 )  // c00000-ffffff empty
ROM_END

ROM_START( kovlsjba )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "lsjba_prg.rom",0x100000, 0x400000, CRC(8b42f456) SHA1(48796e48f6f1a5f68442cf15a6b195095d443a35) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsh_v100_china.asic", 0x000000, 0x04000, BAD_DUMP CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) ) // this is the kovsh ARM rom, we intercept and modify protection calls

	ROM_REGION( 0xc00000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "lsqh2_t01.rom",0x180000, 0x800000, CRC(d498d97f) SHA1(97a7b6d2ed1170449e7c2899448af7cbbca4c94f) )
	ROM_IGNORE( 0x800000 )  // second half identical

	ROM_REGION( 0x3000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "lsqh2_a01.rom", 0x0000000, 0x1000000, CRC(25ae3efd) SHA1(083d977602ddb5ad54fbdcba000cd4287de8d463) )
	ROM_LOAD( "lsqh2_a23.rom", 0x1000000, 0x1000000, CRC(7a779112) SHA1(0a7d36b3715063d8eac629b95a9bb3ecd8e54fca) )
	ROM_LOAD( "lsqh2_a45.rom", 0x2000000, 0x1000000, CRC(5d7de052) SHA1(7663b6cf09f65c4644661005a38f9aba84a32913) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "lsqh2_b01.rom", 0x0000000, 0x1000000, CRC(df7ca696) SHA1(7af3d27957a39de7e4873867c9972c05af7e7964) )

	ROM_REGION( 0xc00000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "lsqh2_m01.rom",0x400000, 0x400000, CRC(01af1b65) SHA1(6cf523fa8f1e03f974771611bb9a4e08a4d4443f) )
	ROM_IGNORE( 0x400000 )  // 400000-7fffff empty
	ROM_CONTINUE( 0x800000, 0x400000 )
	ROM_IGNORE( 0x400000 )  // c00000-ffffff empty
ROM_END


ROM_START( killbldp )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v300x.u6",     0x100000, 0x080000, CRC(b7fb8ec9) SHA1(e71b2d74269a82c7155b9818821156e128b68b28) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	/* the first 0x268 bytes of this are EXECUTE ONLY in the original chip, attempting to read them even via the original CPU just returns what is on the bus */
//  ROM_LOAD( "killbldp_igs027a.bin", 0x000000, 0x04000, CRC(c7868d90) SHA1(335c99933a38b77fcfc3f8004063f35124364f3e) ) // this is the original rom with the first 0x268 bytes from the bootleg - but it doesn't work?
	/* there are some differences around 0x2e80, investigate - maybe above is badly dumped?, padding at 0x3ac0 is also different */
	ROM_LOAD( "killbldp_igs027a_alt.bin", 0x000000, 0x04000, CRC(98316b06) SHA1(09be9fad24d68980a0a5beae60ced48012286216) ) // from a bootleg



	ROM_REGION( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v300x.u26", 0x000000, 0x200000,  CRC(144388c8) SHA1(d7469df077c1a674129f18210584ba4d05a61888) )

	ROM_REGION( 0x800000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t05701w032.bin",0x180000, 0x400000, CRC(567c714f) SHA1(b25b20e1ec9f077d6f7b9d41723a68d0d461bef2) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a05701w064.bin",   0x0000000, 0x0800000, CRC(8c0c992c) SHA1(28391e50ca4400060676f1524bd49ede373292da) )
	ROM_LOAD( "a05702w064.bin",   0x0800000, 0x0800000, CRC(7e5b0f27) SHA1(9e8d69f34c30216925fcb7af87f8b37f703317e7) )
	ROM_LOAD( "a05703w064.bin",   0x1000000, 0x0800000, CRC(accbdb44) SHA1(d59b2452c7a5b4e666473dc973b73a0f2b4edc13) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b05701w064.bin",   0x0000000, 0x0800000, CRC(a20cdcef) SHA1(029a49971adf1e72ab556a207172bdfbd0b86b03) )
	ROM_LOAD( "b05702w016.bin",   0x0800000, 0x0200000, CRC(fe7457df) SHA1(d66b1b31102b0210f9faf40e1473cd1511ccaf1f) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "w05701b032.bin",   0x400000, 0x400000, CRC(2d3ae593) SHA1(b9c1d2994be95ba974bc134a3bf115bc9c9c9c16) )
ROM_END

ROM_START( svg )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "u30.bin",      0x100000, 0x080000, CRC(34c18f3f) SHA1(42d1edd0dcfaa5e44861c6a1d4cb24f51ba23de8) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "svg_igs027a_execute_only_area", 0x0000, 0x00188, NO_DUMP )
	ROM_LOAD( "svg_igs027a_v200_china.bin", 0x0188, 0x3e78, CRC(72b73169) SHA1(ffc0caea855ab4b01beb3aebd0bf17187c66c22c) )

	ROM_REGION( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "u26.bin", 0x000000, 0x400000, CRC(46826ec8) SHA1(ad1daf6f615fb8d748ce7f98f19dd3bf22f79fba) )
	ROM_LOAD( "u29.bin", 0x400000, 0x400000, CRC(fa5f3901) SHA1(8ab7c6763df4f752b50ed2197063f58046b32ddb) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t05601w016.bin",0x180000, 0x200000, CRC(03e110dc) SHA1(41c8f286e9303b24ba6235b341371c298226fb6a) )

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a05601w064.bin",  0x0000000, 0x0800000, CRC(ea6453e4) SHA1(b5c82edafa8008ad59b5f2219511947d078d446e) )
	ROM_LOAD( "a05602w064.bin",  0x0800000, 0x0800000, CRC(6d00621b) SHA1(55a4bc357e14b975b0234a9cd49e2224f509dad9) )
	ROM_LOAD( "a05603w064.bin",  0x1000000, 0x0800000, CRC(7b71c64f) SHA1(bec7c7edf0634cf8351a54abb867c56af08ad2c3) )
	ROM_LOAD( "a05604w032.bin",  0x1800000, 0x0400000, CRC(9452a567) SHA1(01fdb8e1f131603843ef4c49ab76d7a56b2d6414) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b05601w064.bin",  0x0000000, 0x0800000, CRC(35c0a489) SHA1(a7d5527da01f8eaa7499fb6785b57094521bce97) )
	ROM_LOAD( "b05602w064.bin",  0x0800000, 0x0800000, CRC(8aad3f85) SHA1(da6996d901d42b3a2ba7019ad014bb938a5e328b) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "w05601b064.bin",  0x400000, 0x800000, CRC(bfe61a71) SHA1(e682ca8d57ca51c4d72f64fc091161f3dbdce871) )
	ROM_LOAD( "w05602b032.bin",  0xc00000, 0x400000, CRC(0685166d) SHA1(64dac49abd2a46d5fb58c678027aa5e23d672dc4) )
ROM_END

ROM_START( svgtw )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v101tw.u30",      0x100000, 0x080000, CRC(8d0405e4) SHA1(b6175c9ffeaac531d28e7845cb34c673476e286a) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */ // marked H1
	ROM_LOAD( "svgpcb_igs027a_execute_only_area", 0x0000, 0x00188, NO_DUMP )
	ROM_LOAD( "svgcpb_igs027a_v100_japan.bin", 0x0188, 0x3e78, CRC(7a59da5d) SHA1(d67ba465db40ca716b4b901b1c8e762716fb954e) ) // this is from the Japan set, the cart this came from was Taiwan

	ROM_REGION( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v101tw.u26", 0x000000, 0x400000, CRC(cc24f542) SHA1(623ed398d2eeea229833d92eb4fb6492133202b3) )
	ROM_LOAD( "v101tw.u36", 0x400000, 0x400000, CRC(f18283e2) SHA1(15323c5f816a0bf6f510311eb49d485ccf713cf7) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t05601w016.bin",0x180000, 0x200000, CRC(03e110dc) SHA1(41c8f286e9303b24ba6235b341371c298226fb6a) )

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a05601w064.bin",  0x0000000, 0x0800000, CRC(ea6453e4) SHA1(b5c82edafa8008ad59b5f2219511947d078d446e) )
	ROM_LOAD( "a05602w064.bin",  0x0800000, 0x0800000, CRC(6d00621b) SHA1(55a4bc357e14b975b0234a9cd49e2224f509dad9) )
	ROM_LOAD( "a05603w064.bin",  0x1000000, 0x0800000, CRC(7b71c64f) SHA1(bec7c7edf0634cf8351a54abb867c56af08ad2c3) )
	ROM_LOAD( "a05604w032.bin",  0x1800000, 0x0400000, CRC(9452a567) SHA1(01fdb8e1f131603843ef4c49ab76d7a56b2d6414) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b05601w064.bin",  0x0000000, 0x0800000, CRC(35c0a489) SHA1(a7d5527da01f8eaa7499fb6785b57094521bce97) )
	ROM_LOAD( "b05602w064.bin",  0x0800000, 0x0800000, CRC(8aad3f85) SHA1(da6996d901d42b3a2ba7019ad014bb938a5e328b) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "w05601b064.bin",  0x400000, 0x800000, CRC(bfe61a71) SHA1(e682ca8d57ca51c4d72f64fc091161f3dbdce871) )
	ROM_LOAD( "w05602b032.bin",  0xc00000, 0x400000, CRC(0685166d) SHA1(64dac49abd2a46d5fb58c678027aa5e23d672dc4) )
ROM_END


ROM_START( svgpcb )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "svg_bios.u49",    0x000000, 0x020000, CRC(3346401f) SHA1(28bd730b6026c1e521c95072d33c7bdcd19c1460) )
	ROM_LOAD16_WORD_SWAP( "svg_v100jp.u50",  0x100000, 0x080000, CRC(8d0405e4) SHA1(b6175c9ffeaac531d28e7845cb34c673476e286a) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "svgpcb_igs027a_execute_only_area", 0x0000, 0x00188, NO_DUMP )
	ROM_LOAD( "svgcpb_igs027a_v100_japan.bin", 0x0188, 0x3e78, CRC(7a59da5d) SHA1(d67ba465db40ca716b4b901b1c8e762716fb954e) )


	ROM_REGION( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "svg_v100jp.u64", 0x000000, 0x400000, CRC(399d4a8b) SHA1(b120e8386a259e6fd7941acf3c33cf288eda616c) )
	ROM_LOAD( "svg_v100jp.u65", 0x400000, 0x400000, CRC(6e1c33b1) SHA1(66f26b2f4c0b3dcf6d1bb1df48e2ddbcc9d9432d) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS // IGS PGM T01S 1B8558A1 M002146
	ROM_LOAD( "t05601w016.bin",0x180000, 0x200000, CRC(03e110dc) SHA1(41c8f286e9303b24ba6235b341371c298226fb6a) ) // IGS T05601W016 2C35 B270 2L464103 B050924

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a05601w064.bin",  0x0000000, 0x0800000, CRC(ea6453e4) SHA1(b5c82edafa8008ad59b5f2219511947d078d446e) ) // IGS A05601W064 44BF 2ECD 2E153602A1 S050914
	ROM_LOAD( "a05602w064.bin",  0x0800000, 0x0800000, CRC(6d00621b) SHA1(55a4bc357e14b975b0234a9cd49e2224f509dad9) ) // IGS A05602W064 85C1 8A1F 2E153602A2 S050914
	ROM_LOAD( "a05603w064.bin",  0x1000000, 0x0800000, CRC(7b71c64f) SHA1(bec7c7edf0634cf8351a54abb867c56af08ad2c3) ) // IGS A05603W064 8EC7 329A 2E153602A3 S050914
	ROM_LOAD( "a05604w032.bin",  0x1800000, 0x0400000, CRC(9452a567) SHA1(01fdb8e1f131603843ef4c49ab76d7a56b2d6414) ) // IGS A05604W032 55F5 4CDB SL529808 S050912

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b05601w064.bin",  0x0000000, 0x0800000, CRC(35c0a489) SHA1(a7d5527da01f8eaa7499fb6785b57094521bce97) ) // IGS B05601W064 EED5 E656 2E350003A1 S050914
	ROM_LOAD( "b05602w064.bin",  0x0800000, 0x0800000, CRC(8aad3f85) SHA1(da6996d901d42b3a2ba7019ad014bb938a5e328b) ) // IGS B05602W064 DD65 3D89 2E350004A2 S050914

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS // IGS PGM M01S 1BB278A1 E002146
	ROM_LOAD( "w05601b064.bin",  0x400000, 0x800000, CRC(bfe61a71) SHA1(e682ca8d57ca51c4d72f64fc091161f3dbdce871) ) // IGS W05601B064 2530 FBF6 2E350004A3 S050914
	ROM_LOAD( "w05602b032.bin",  0xc00000, 0x400000, CRC(0685166d) SHA1(64dac49abd2a46d5fb58c678027aa5e23d672dc4) ) // IGS w05602B032 1BC2 90D3 2K453504 S050912
ROM_END


ROM_START( happy6 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v101cn.u5",      0x100000, 0x080000, CRC(aa4646e3) SHA1(e6772cc480ddd3e1d199364b1f2ff93b973e6842) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	// data before 0x188 is read-protected and cannot be read even with a trojan (as with most 2001/2+ IGS titles)
//  ROM_LOAD( "happy6_igs027a.bin", 0x000000, 0x04000, NO_DUMP )
	ROM_LOAD( "happy6_igs027a_execute_only_area", 0x0000, 0x00188, NO_DUMP )
	ROM_LOAD( "happy6_igs027a_v100_china.bin", 0x0188, 0x3e78, CRC(ed530445) SHA1(05c92d649701be2541557b1334dd6c820ca1009e) )


	ROM_REGION( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v102cn.u26", 0x000000, 0x400000, CRC(310510fb) SHA1(e0e80a04e9f7bf27e6581a8935c960bad33bb6de) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t01w64m.u29",0x180000, 0x800000, CRC(2d3feb8b) SHA1(9832b1c46b1ee73febf5c5c8913859f4e0581665) )

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a01w64m.u5",  0x0000000, 0x0800000, CRC(bbaa3df3) SHA1(a72268d3989e96d571242279922291d0dc72db28) )
	ROM_LOAD( "a02w64m.u6",  0x0800000, 0x0800000, CRC(f8c9cd36) SHA1(d9613a83bcc2364492fa922fde1c4f0d07b3009c) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b01w64m.u19",  0x0000000, 0x0800000, CRC(73f5f225) SHA1(507126fa96dcec967bdbc0978d79fbce9d25db37) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "w01w64m.u17",  0x400000, 0x800000, CRC(7e23e2be) SHA1(5e920b8e480f00b6666292d4a56039b40af18141) )
ROM_END

ROM_START( happy6101 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "happy6in1_v100cn.u5",      0x100000, 0x080000, CRC(a25418e8) SHA1(acd7e7b69956cb4ce8e26c6420cb97bb4bf404e7) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	// data before 0x188 is read-protected and cannot be read even with a trojan (as with most 2001/2+ IGS titles)
//  ROM_LOAD( "happy6_igs027a.bin", 0x000000, 0x04000, NO_DUMP )
	ROM_LOAD( "happy6_igs027a_execute_only_area", 0x0000, 0x00188, NO_DUMP )
	ROM_LOAD( "happy6_igs027a_v100_china.bin", 0x0188, 0x3e78, CRC(ed530445) SHA1(05c92d649701be2541557b1334dd6c820ca1009e) )


	ROM_REGION( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "happy6in1_v101cn.u26", 0x000000, 0x400000, CRC(4a48ca1c) SHA1(3bebc091787903d45cb84c7302046602a903f59c) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t01w64m.u29",0x180000, 0x800000, CRC(2d3feb8b) SHA1(9832b1c46b1ee73febf5c5c8913859f4e0581665) )

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a01w64m.u5",  0x0000000, 0x0800000, CRC(bbaa3df3) SHA1(a72268d3989e96d571242279922291d0dc72db28) )
	ROM_LOAD( "a02w64m.u6",  0x0800000, 0x0800000, CRC(f8c9cd36) SHA1(d9613a83bcc2364492fa922fde1c4f0d07b3009c) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b01w64m.u19",  0x0000000, 0x0800000, CRC(73f5f225) SHA1(507126fa96dcec967bdbc0978d79fbce9d25db37) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "w01w64m.u17",  0x400000, 0x800000, CRC(7e23e2be) SHA1(5e920b8e480f00b6666292d4a56039b40af18141) )
ROM_END

/* all known revisions of ketsui have roms marked v100, even when the actual game revision is upgraded */

ROM_START( ket )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	/* doesn't use a separate BIOS rom */
	ROM_LOAD16_WORD_SWAP( "ketsui_v100.u38", 0x000000, 0x200000, CRC(dfe62f3b) SHA1(baa58d1ce47a707f84f65779ac0689894793e9d9) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ket_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM text bios - surface scratched to remove details
	ROM_LOAD( "t04701w064.u19",   0x180000, 0x800000, CRC(2665b041) SHA1(fb1107778b66f2af0de77ac82e1ee2902f53a959) ) //text-1

	ROM_REGION( 0x1000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04701w064.u7",    0x0000000, 0x0800000, CRC(5ef1b94b) SHA1(f10dfa46e0a4d297c3a856aea5b49d648f98935c) ) //image-1
	ROM_LOAD( "a04702w064.u8",    0x0800000, 0x0800000, CRC(26d6da7f) SHA1(f20e07a7994f41b5ed917f8b0119dc5542f3541c) ) //image-2

	ROM_REGION( 0x0800000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04701w064.u1",    0x0000000, 0x0800000, CRC(1bec008d) SHA1(07d117dc2eebb35727fb18a7c563acbaf25a8d36) ) //bitmap-1

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	/* there is a position for the PGM audio bios rom, but it's unpopulated, and the M of PGM has been scratched off the PCB */
	ROM_LOAD( "m04701b032.u17",    0x400000, 0x400000, CRC(b46e22d1) SHA1(670853dc485942fb96380568494bdf3235f446ee) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ket_defaults.nv",  0x0000000, 0x020000, CRC(3ca892d8) SHA1(67430df5217e453ae8140c5653deeadfad8fa684) )
ROM_END

ROM_START( ket1 ) // only difference between this and ket1 is the rom fill on the unused area
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	/* doesn't use a separate BIOS rom */
	ROM_LOAD16_WORD_SWAP( "ketsui_v100_alt_fill.u38", 0x000000, 0x200000, CRC(e140f8a4) SHA1(34fd25f8896935503d7537e89a4cd174e8995070) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ket_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM text bios - surface scratched to remove details
	ROM_LOAD( "t04701w064.u19",   0x180000, 0x800000, CRC(2665b041) SHA1(fb1107778b66f2af0de77ac82e1ee2902f53a959) ) //text-1

	ROM_REGION( 0x1000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04701w064.u7",    0x0000000, 0x0800000, CRC(5ef1b94b) SHA1(f10dfa46e0a4d297c3a856aea5b49d648f98935c) ) //image-1
	ROM_LOAD( "a04702w064.u8",    0x0800000, 0x0800000, CRC(26d6da7f) SHA1(f20e07a7994f41b5ed917f8b0119dc5542f3541c) ) //image-2

	ROM_REGION( 0x0800000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04701w064.u1",    0x0000000, 0x0800000, CRC(1bec008d) SHA1(07d117dc2eebb35727fb18a7c563acbaf25a8d36) ) //bitmap-1

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	/* there is a position for the PGM audio bios rom, but it's unpopulated, and the M of PGM has been scratched off the PCB */
	ROM_LOAD( "m04701b032.u17",    0x400000, 0x400000, CRC(b46e22d1) SHA1(670853dc485942fb96380568494bdf3235f446ee) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ket_defaults.nv",  0x0000000, 0x020000, CRC(3ca892d8) SHA1(67430df5217e453ae8140c5653deeadfad8fa684) )
ROM_END

ROM_START( keta )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	/* doesn't use a separate BIOS rom */
	ROM_LOAD16_WORD_SWAP( "ketsui_prg_revised.bin", 0x000000, 0x200000, CRC(69fcf5eb) SHA1(f726e251b4daa2f8d717e32000d4d7abc71c710d) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ket_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM text bios - surface scratched to remove details
	ROM_LOAD( "t04701w064.u19",   0x180000, 0x800000, CRC(2665b041) SHA1(fb1107778b66f2af0de77ac82e1ee2902f53a959) ) //text-1

	ROM_REGION( 0x1000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04701w064.u7",    0x0000000, 0x0800000, CRC(5ef1b94b) SHA1(f10dfa46e0a4d297c3a856aea5b49d648f98935c) ) //image-1
	ROM_LOAD( "a04702w064.u8",    0x0800000, 0x0800000, CRC(26d6da7f) SHA1(f20e07a7994f41b5ed917f8b0119dc5542f3541c) ) //image-2

	ROM_REGION( 0x0800000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04701w064.u1",    0x0000000, 0x0800000, CRC(1bec008d) SHA1(07d117dc2eebb35727fb18a7c563acbaf25a8d36) ) //bitmap-1

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	/* there is a position for the PGM audio bios rom, but it's unpopulated, and the M of PGM has been scratched off the PCB */
	ROM_LOAD( "m04701b032.u17",    0x400000, 0x400000, CRC(b46e22d1) SHA1(670853dc485942fb96380568494bdf3235f446ee) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ket_defaults.nv",  0x0000000, 0x020000, CRC(3ca892d8) SHA1(67430df5217e453ae8140c5653deeadfad8fa684) )
ROM_END

ROM_START( ketb )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	/* doesn't use a separate BIOS rom */
	ROM_LOAD16_WORD_SWAP( "ketsui_prg_original.bin", 0x000000, 0x200000, CRC(cca5e153) SHA1(b653feaa2004c379312def6b1613c3497f654ddf) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ket_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM text bios - surface scratched to remove details
	ROM_LOAD( "t04701w064.u19",   0x180000, 0x800000, CRC(2665b041) SHA1(fb1107778b66f2af0de77ac82e1ee2902f53a959) ) //text-1

	ROM_REGION( 0x1000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04701w064.u7",    0x0000000, 0x0800000, CRC(5ef1b94b) SHA1(f10dfa46e0a4d297c3a856aea5b49d648f98935c) ) //image-1
	ROM_LOAD( "a04702w064.u8",    0x0800000, 0x0800000, CRC(26d6da7f) SHA1(f20e07a7994f41b5ed917f8b0119dc5542f3541c) ) //image-2

	ROM_REGION( 0x0800000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04701w064.u1",    0x0000000, 0x0800000, CRC(1bec008d) SHA1(07d117dc2eebb35727fb18a7c563acbaf25a8d36) ) //bitmap-1

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	/* there is a position for the PGM audio bios rom, but it's unpopulated, and the M of PGM has been scratched off the PCB */
	ROM_LOAD( "m04701b032.u17",    0x400000, 0x400000, CRC(b46e22d1) SHA1(670853dc485942fb96380568494bdf3235f446ee) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ket_defaults.nv",  0x0000000, 0x020000, CRC(3ca892d8) SHA1(67430df5217e453ae8140c5653deeadfad8fa684) )
ROM_END


ROM_START( ketarr10 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	/* doesn't use a separate BIOS rom */
	ROM_LOAD16_WORD_SWAP( "ketarr_v100.u38", 0x000000, 0x200000, CRC(d4c7a8ab) SHA1(65d104d17bd4fd03a2b44297a003ba03d746c7ee) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ket_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM text bios - surface scratched to remove details
	ROM_LOAD( "t04701w064.u19", 0x180000, 0x800000, CRC(2665b041) SHA1(fb1107778b66f2af0de77ac82e1ee2902f53a959) ) //text-1

	ROM_REGION( 0x1000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04701w064.u7", 0x0000000, 0x0800000, CRC(5ef1b94b) SHA1(f10dfa46e0a4d297c3a856aea5b49d648f98935c) ) //image-1
	ROM_LOAD( "a04702w064.u8", 0x0800000, 0x0800000, CRC(26d6da7f) SHA1(f20e07a7994f41b5ed917f8b0119dc5542f3541c) ) //image-2

	ROM_REGION( 0x0800000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04701w064.u1", 0x0000000, 0x0800000, CRC(1bec008d) SHA1(07d117dc2eebb35727fb18a7c563acbaf25a8d36) ) //bitmap-1

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	/* there is a position for the PGM audio bios rom, but it's unpopulated, and the M of PGM has been scratched off the PCB */
	ROM_LOAD( "m04701b032.u17", 0x400000, 0x400000, CRC(b46e22d1) SHA1(670853dc485942fb96380568494bdf3235f446ee) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ket_defaults.nv", 0x0000000, 0x020000, CRC(3ca892d8) SHA1(67430df5217e453ae8140c5653deeadfad8fa684) )
ROM_END


ROM_START( ketarrf )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	/* doesn't use a separate BIOS rom */
	ROM_LOAD16_WORD_SWAP( "ketarrf_v100.u38", 0x000000, 0x200000, CRC(6ad17aa4) SHA1(791bd1a107433a3811c8a79ea26a73e66ddd296f) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ket_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM text bios - surface scratched to remove details
	ROM_LOAD( "t04701w064.u19", 0x180000, 0x800000, CRC(2665b041) SHA1(fb1107778b66f2af0de77ac82e1ee2902f53a959) ) //text-1

	ROM_REGION( 0x1000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04701w064.u7", 0x0000000, 0x0800000, CRC(5ef1b94b) SHA1(f10dfa46e0a4d297c3a856aea5b49d648f98935c) ) //image-1
	ROM_LOAD( "a04702w064.u8", 0x0800000, 0x0800000, CRC(26d6da7f) SHA1(f20e07a7994f41b5ed917f8b0119dc5542f3541c) ) //image-2

	ROM_REGION( 0x0800000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04701w064.u1", 0x0000000, 0x0800000, CRC(1bec008d) SHA1(07d117dc2eebb35727fb18a7c563acbaf25a8d36) ) //bitmap-1

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	/* there is a position for the PGM audio bios rom, but it's unpopulated, and the M of PGM has been scratched off the PCB */
	ROM_LOAD( "m04701b032.u17", 0x400000, 0x400000, CRC(b46e22d1) SHA1(670853dc485942fb96380568494bdf3235f446ee) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ket_defaults.nv", 0x0000000, 0x020000, CRC(3ca892d8) SHA1(67430df5217e453ae8140c5653deeadfad8fa684) )
ROM_END

ROM_START( ketarr15 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	/* doesn't use a separate BIOS rom */
	ROM_LOAD16_WORD_SWAP( "ketarr15_v100.u38", 0x000000, 0x200000, CRC(552a7d95) SHA1(4f3fb13f34d58a7482e1d26623d38aa0b54ca8dd) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ket_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM text bios - surface scratched to remove details
	ROM_LOAD( "t04701w064.u19", 0x180000, 0x800000, CRC(2665b041) SHA1(fb1107778b66f2af0de77ac82e1ee2902f53a959) ) //text-1

	ROM_REGION( 0x1000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04701w064.u7", 0x0000000, 0x0800000, CRC(5ef1b94b) SHA1(f10dfa46e0a4d297c3a856aea5b49d648f98935c) ) //image-1
	ROM_LOAD( "a04702w064.u8", 0x0800000, 0x0800000, CRC(26d6da7f) SHA1(f20e07a7994f41b5ed917f8b0119dc5542f3541c) ) //image-2

	ROM_REGION( 0x0800000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04701w064.u1", 0x0000000, 0x0800000, CRC(1bec008d) SHA1(07d117dc2eebb35727fb18a7c563acbaf25a8d36) ) //bitmap-1

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	/* there is a position for the PGM audio bios rom, but it's unpopulated, and the M of PGM has been scratched off the PCB */
	ROM_LOAD( "m04701b032.u17", 0x400000, 0x400000, CRC(b46e22d1) SHA1(670853dc485942fb96380568494bdf3235f446ee) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ket_defaults.nv", 0x0000000, 0x020000, CRC(3ca892d8) SHA1(67430df5217e453ae8140c5653deeadfad8fa684) )
ROM_END

ROM_START( ketarrs15 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	/* doesn't use a separate BIOS rom */
	ROM_LOAD16_WORD_SWAP( "ketarrs15_v100.u38", 0x000000, 0x200000, CRC(a95e71e0) SHA1(182c12e3581ebb20176d8abca41ee62aadcd63e0) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ket_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM text bios - surface scratched to remove details
	ROM_LOAD( "t04701w064.u19", 0x180000, 0x800000, CRC(2665b041) SHA1(fb1107778b66f2af0de77ac82e1ee2902f53a959) ) //text-1

	ROM_REGION( 0x1000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04701w064.u7", 0x0000000, 0x0800000, CRC(5ef1b94b) SHA1(f10dfa46e0a4d297c3a856aea5b49d648f98935c) ) //image-1
	ROM_LOAD( "a04702w064.u8", 0x0800000, 0x0800000, CRC(26d6da7f) SHA1(f20e07a7994f41b5ed917f8b0119dc5542f3541c) ) //image-2

	ROM_REGION( 0x0800000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04701w064.u1", 0x0000000, 0x0800000, CRC(1bec008d) SHA1(07d117dc2eebb35727fb18a7c563acbaf25a8d36) ) //bitmap-1

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	/* there is a position for the PGM audio bios rom, but it's unpopulated, and the M of PGM has been scratched off the PCB */
	ROM_LOAD( "m04701b032.u17", 0x400000, 0x400000, CRC(b46e22d1) SHA1(670853dc485942fb96380568494bdf3235f446ee) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ket_defaults.nv", 0x0000000, 0x020000, CRC(3ca892d8) SHA1(67430df5217e453ae8140c5653deeadfad8fa684) )
ROM_END

ROM_START( ketarr151 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	/* doesn't use a separate BIOS rom */
	ROM_LOAD16_WORD_SWAP( "ketarr151_v100.u38", 0x000000, 0x200000, CRC(2b7c030d) SHA1(9aaba1242d7ce29915a31d40341da82985927f9d) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ket_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM text bios - surface scratched to remove details
	ROM_LOAD( "t04701w064.u19", 0x180000, 0x800000, CRC(2665b041) SHA1(fb1107778b66f2af0de77ac82e1ee2902f53a959) ) //text-1

	ROM_REGION( 0x1000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04701w064.u7", 0x0000000, 0x0800000, CRC(5ef1b94b) SHA1(f10dfa46e0a4d297c3a856aea5b49d648f98935c) ) //image-1
	ROM_LOAD( "a04702w064.u8", 0x0800000, 0x0800000, CRC(26d6da7f) SHA1(f20e07a7994f41b5ed917f8b0119dc5542f3541c) ) //image-2

	ROM_REGION( 0x0800000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04701w064.u1", 0x0000000, 0x0800000, CRC(1bec008d) SHA1(07d117dc2eebb35727fb18a7c563acbaf25a8d36) ) //bitmap-1

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	/* there is a position for the PGM audio bios rom, but it's unpopulated, and the M of PGM has been scratched off the PCB */
	ROM_LOAD( "m04701b032.u17", 0x400000, 0x400000, CRC(b46e22d1) SHA1(670853dc485942fb96380568494bdf3235f446ee) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ket_defaults.nv", 0x0000000, 0x020000, CRC(3ca892d8) SHA1(67430df5217e453ae8140c5653deeadfad8fa684) )
ROM_END

ROM_START( ketarrs151 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	/* doesn't use a separate BIOS rom */
	ROM_LOAD16_WORD_SWAP( "ketarrs151_v100.u38", 0x000000, 0x200000, CRC(35c984e4) SHA1(d4517f318de0c40a3b30e41374f33bb355581434) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ket_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM text bios - surface scratched to remove details
	ROM_LOAD( "t04701w064.u19", 0x180000, 0x800000, CRC(2665b041) SHA1(fb1107778b66f2af0de77ac82e1ee2902f53a959) ) //text-1

	ROM_REGION( 0x1000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04701w064.u7", 0x0000000, 0x0800000, CRC(5ef1b94b) SHA1(f10dfa46e0a4d297c3a856aea5b49d648f98935c) ) //image-1
	ROM_LOAD( "a04702w064.u8", 0x0800000, 0x0800000, CRC(26d6da7f) SHA1(f20e07a7994f41b5ed917f8b0119dc5542f3541c) ) //image-2

	ROM_REGION( 0x0800000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04701w064.u1", 0x0000000, 0x0800000, CRC(1bec008d) SHA1(07d117dc2eebb35727fb18a7c563acbaf25a8d36) ) //bitmap-1

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	/* there is a position for the PGM audio bios rom, but it's unpopulated, and the M of PGM has been scratched off the PCB */
	ROM_LOAD( "m04701b032.u17", 0x400000, 0x400000, CRC(b46e22d1) SHA1(670853dc485942fb96380568494bdf3235f446ee) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ket_defaults.nv", 0x0000000, 0x020000, CRC(3ca892d8) SHA1(67430df5217e453ae8140c5653deeadfad8fa684) )
ROM_END

ROM_START( ketarr )
	ROM_REGION( 0x600000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "ketarr17_v100.u38", 0x000000, 0x200000, CRC(2cb80b89) SHA1(e1aa072b8344890486e11795e02703aa2d234bb1) )

	ROM_REGION( 0x4000, "prot", 0 )
	ROM_LOAD( "ket_igs027a.bin", 0x000000, 0x04000, CRC(ab54d286) SHA1(897256b6709e1a4da9daba92b6bde39ccfccd8c1) )

	ROM_REGION( 0xc00000, "tiles", 0 )
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) )
	ROM_LOAD( "t04701w064.u19", 0x180000, 0x800000, CRC(2665b041) SHA1(fb1107778b66f2af0de77ac82e1ee2902f53a959) ) //text-1

	ROM_REGION( 0x1000000, "sprcol", 0 )
	ROM_LOAD( "a04701w064.u7", 0x0000000, 0x0800000, CRC(5ef1b94b) SHA1(f10dfa46e0a4d297c3a856aea5b49d648f98935c) ) //image-1
	ROM_LOAD( "a04702w064.u8", 0x0800000, 0x0800000, CRC(26d6da7f) SHA1(f20e07a7994f41b5ed917f8b0119dc5542f3541c) ) //image-2

	ROM_REGION( 0x0800000, "sprmask", 0 )
	ROM_LOAD( "b04701w064.u1", 0x0000000, 0x0800000, CRC(1bec008d) SHA1(07d117dc2eebb35727fb18a7c563acbaf25a8d36) ) //bitmap-1

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 )
	ROM_LOAD( "m04701b032.u17", 0x400000, 0x400000, CRC(b46e22d1) SHA1(670853dc485942fb96380568494bdf3235f446ee) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ket_defaults.nv", 0x0000000, 0x020000, CRC(3ca892d8) SHA1(67430df5217e453ae8140c5653deeadfad8fa684) )
ROM_END

ROM_START( espgal )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	/* doesn't use a separate BIOS rom */
	ROM_LOAD16_WORD_SWAP( "espgaluda_v100.u38", 0x000000, 0x200000, CRC(08ecec34) SHA1(bce2e7fb9105ed51603d09cbd3a9eeb5b8f47ee2) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "espgal_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "t01s.u18", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM text bios - surface scratched to remove details
	ROM_LOAD( "t04801w064.u19",   0x180000, 0x800000, CRC(6021c79e) SHA1(fbc340dafb18aa3094de29b881318a5a9794e4bc) ) //text-1

	ROM_REGION( 0x1000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04801w064.u7",    0x0000000, 0x0800000, CRC(26dd4932) SHA1(9bbabb5a53cb5ba88397cc2c258980f3b70314ce) ) //image-1
	ROM_LOAD( "a04802w064.u8",    0x0800000, 0x0800000, CRC(0e6bf7a9) SHA1(a7541e2b5a0df2bc62a5b347e54dbc2ed1922db2) ) //image-2

	ROM_REGION( 0x0800000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04801w064.u1",    0x0000000, 0x0800000, CRC(98dce13a) SHA1(61d48b7117459f7babc022b68231f6928177a71d) ) //bitmap-1

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	/* there is a position for the PGM audio bios rom, but it's unpopulated, and the M of PGM has been scratched off the PCB */
	ROM_LOAD( "w04801b032.u17",    0x400000, 0x400000, CRC(60298536) SHA1(6b7333f16cce778c5725dbdf75a5446f0906397a) ) //music-1
ROM_END

ROM_START( ddpdoj )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "ddp3_bios.u37",    0x00000, 0x080000, CRC(b3cc5c8f) SHA1(02d9511cf71e4a0d6ca8fd9a1ef2c79b0d001824) ) // uses a standard PGM bios with the startup logos hacked out
	ROM_LOAD16_WORD_SWAP( "ddp3_v101.u36",  0x100000, 0x200000, CRC(195b5c1e) SHA1(f18d791c034b0a3d85888a92fb5d326ee3deb04f) ) // yes this one was actually marked v101 which goes against the standard Cave marking system

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ddp3_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM bios
	ROM_LOAD( "t04401w064.u19",0x180000, 0x800000, CRC(3a95f19c) SHA1(fd3c47cf0b8b1e20c6bec4be68a089fc8bbf4dbe) ) //text-1

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04401w064.u7",  0x0000000, 0x0800000, CRC(ed229794) SHA1(1cf1863495a18c7c7d277a9be43ec116b00960b0) ) //image-1
	ROM_LOAD( "a04402w064.u8",  0x0800000, 0x0800000, CRC(752167b0) SHA1(c33c3398dd8e479c9d5bd348924958a6aecbf0fc) ) //image-2

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04401w064.u1",  0x0000000, 0x0800000, CRC(8cbff066) SHA1(eef1cd566bc70ebf45f047e56026803d5c1dac43) ) //bitmap-1

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // same as standard PGM bios
	ROM_LOAD( "m04401b032.u17",  0x400000, 0x400000, CRC(5a0dbd76) SHA1(06ab202f6bd5ebfb35b9d8cc7a8fb83ec8840659) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ddp3_defaults.nv",  0x0000000, 0x020000, CRC(571e96c0) SHA1(348940c77ca348213331b85b9b1d3aabb96a528a) )
ROM_END


ROM_START( ddpdoja )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "ddp3_bios.u37",    0x00000, 0x080000, CRC(b3cc5c8f) SHA1(02d9511cf71e4a0d6ca8fd9a1ef2c79b0d001824) ) // uses a standard PGM bios with the startup logos hacked out
	ROM_LOAD16_WORD_SWAP( "ddp3_d_d_1_0.u36",  0x100000, 0x200000, CRC(5d3f85ba) SHA1(4c24ea206140863d456179750366921442e1d2b8) ) // marked v100

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ddp3_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM bios
	ROM_LOAD( "t04401w064.u19",0x180000, 0x800000, CRC(3a95f19c) SHA1(fd3c47cf0b8b1e20c6bec4be68a089fc8bbf4dbe) ) //text-1

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04401w064.u7",  0x0000000, 0x0800000, CRC(ed229794) SHA1(1cf1863495a18c7c7d277a9be43ec116b00960b0) ) //image-1
	ROM_LOAD( "a04402w064.u8",  0x0800000, 0x0800000, CRC(752167b0) SHA1(c33c3398dd8e479c9d5bd348924958a6aecbf0fc) ) //image-2

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04401w064.u1",  0x0000000, 0x0800000, CRC(8cbff066) SHA1(eef1cd566bc70ebf45f047e56026803d5c1dac43) ) //bitmap-1

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // same as standard PGM bios
	ROM_LOAD( "m04401b032.u17",  0x400000, 0x400000, CRC(5a0dbd76) SHA1(06ab202f6bd5ebfb35b9d8cc7a8fb83ec8840659) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ddp3_defaults.nv",  0x0000000, 0x020000, CRC(571e96c0) SHA1(348940c77ca348213331b85b9b1d3aabb96a528a) )
ROM_END

ROM_START( ddpdojb )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "ddp3_bios.u37",    0x00000, 0x080000, CRC(b3cc5c8f) SHA1(02d9511cf71e4a0d6ca8fd9a1ef2c79b0d001824) ) // uses a standard PGM bios with the startup logos hacked out
	ROM_LOAD16_WORD_SWAP( "dd v100.bin",  0x100000, 0x200000, CRC(7da0c1e4) SHA1(aca2fe35ba0ab3628900fa2aba2d22fc4fd7046d) ) // marked v100

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ddp3_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM bios
	ROM_LOAD( "t04401w064.u19",0x180000, 0x800000, CRC(3a95f19c) SHA1(fd3c47cf0b8b1e20c6bec4be68a089fc8bbf4dbe) ) //text-1

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04401w064.u7",  0x0000000, 0x0800000, CRC(ed229794) SHA1(1cf1863495a18c7c7d277a9be43ec116b00960b0) ) //image-1
	ROM_LOAD( "a04402w064.u8",  0x0800000, 0x0800000, CRC(752167b0) SHA1(c33c3398dd8e479c9d5bd348924958a6aecbf0fc) ) //image-2

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04401w064.u1",  0x0000000, 0x0800000, CRC(8cbff066) SHA1(eef1cd566bc70ebf45f047e56026803d5c1dac43) ) //bitmap-1

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // same as standard PGM bios
	ROM_LOAD( "m04401b032.u17",  0x400000, 0x400000, CRC(5a0dbd76) SHA1(06ab202f6bd5ebfb35b9d8cc7a8fb83ec8840659) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ddp3_defaults.nv",  0x0000000, 0x020000, CRC(571e96c0) SHA1(348940c77ca348213331b85b9b1d3aabb96a528a) )
ROM_END

/* this expects Magic values in NVRAM to boot */
ROM_START( ddpdojblk )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "ddp3_bios.u37",    0x00000, 0x080000, CRC(b3cc5c8f) SHA1(02d9511cf71e4a0d6ca8fd9a1ef2c79b0d001824) ) // uses a standard PGM bios with the startup logos hacked out
	ROM_LOAD16_WORD_SWAP( "ddb_1dot.u45",  0x100000, 0x200000, CRC(265f26cd) SHA1(91abc7fc4722f3d01d76a4c1ae14c4132e4e576c) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ddp3_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM bios
	ROM_LOAD( "t04401w064.u19",0x180000, 0x800000, CRC(3a95f19c) SHA1(fd3c47cf0b8b1e20c6bec4be68a089fc8bbf4dbe) ) //text-1

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04401w064.u7",  0x0000000, 0x0800000, CRC(ed229794) SHA1(1cf1863495a18c7c7d277a9be43ec116b00960b0) ) //image-1
	ROM_LOAD( "a04402w064.u8",  0x0800000, 0x0800000, CRC(752167b0) SHA1(c33c3398dd8e479c9d5bd348924958a6aecbf0fc) ) //image-2

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04401w064.u1",  0x0000000, 0x0800000, CRC(8cbff066) SHA1(eef1cd566bc70ebf45f047e56026803d5c1dac43) ) //bitmap-1

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // same as standard PGM bios
	ROM_LOAD( "m04401b032.u17",  0x400000, 0x400000, CRC(5a0dbd76) SHA1(06ab202f6bd5ebfb35b9d8cc7a8fb83ec8840659) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* NVRAM with factory programmed values - needed to boot */
	ROM_LOAD( "ddp3blk_defaults.nv",  0x0000000, 0x020000, CRC(a1651904) SHA1(5b80d3c4c764895c40953a66161d4dd84f742604) )
ROM_END

ROM_START( ddpdojblka )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "ddp3_bios.u37",    0x00000, 0x080000, CRC(b3cc5c8f) SHA1(02d9511cf71e4a0d6ca8fd9a1ef2c79b0d001824) ) // uses a standard PGM bios with the startup logos hacked out
	ROM_LOAD16_WORD_SWAP( "ddb10.u45",  0x100000, 0x200000, CRC(72b35510) SHA1(9a432e5e1ebe61aafd737b6acc905653e5af0d38) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ddp3_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM bios
	ROM_LOAD( "t04401w064.u19",0x180000, 0x800000, CRC(3a95f19c) SHA1(fd3c47cf0b8b1e20c6bec4be68a089fc8bbf4dbe) ) //text-1

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04401w064.u7",  0x0000000, 0x0800000, CRC(ed229794) SHA1(1cf1863495a18c7c7d277a9be43ec116b00960b0) ) //image-1
	ROM_LOAD( "a04402w064.u8",  0x0800000, 0x0800000, CRC(752167b0) SHA1(c33c3398dd8e479c9d5bd348924958a6aecbf0fc) ) //image-2

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04401w064.u1",  0x0000000, 0x0800000, CRC(8cbff066) SHA1(eef1cd566bc70ebf45f047e56026803d5c1dac43) ) //bitmap-1

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // same as standard PGM bios
	ROM_LOAD( "m04401b032.u17",  0x400000, 0x400000, CRC(5a0dbd76) SHA1(06ab202f6bd5ebfb35b9d8cc7a8fb83ec8840659) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* NVRAM with factory programmed values - needed to boot */
	ROM_LOAD( "ddp3blk_defaults.nv",  0x0000000, 0x020000, CRC(a1651904) SHA1(5b80d3c4c764895c40953a66161d4dd84f742604) )
ROM_END

/*** Init Stuff **************************************************************/

/* This function expands the sprite colour data (in the A Roms) from 3 pixels
   in each word to a byte per pixel making it easier to use */

void pgm_state::expand_colourdata()
{
	UINT8 *src = memregion( "sprcol" )->base();
	size_t srcsize = memregion( "sprcol" )->bytes();
	int cnt;
	size_t needed = srcsize / 2 * 3;

	/* work out how much ram we need to allocate to expand the sprites into
	   and be able to mask the offset */
	m_sprite_a_region_size = 1;
	while (m_sprite_a_region_size < needed)
		m_sprite_a_region_size <<= 1;

	m_sprite_a_region = auto_alloc_array(machine(), UINT8, m_sprite_a_region_size);

	for (cnt = 0 ; cnt < srcsize / 2 ; cnt++)
	{
		UINT16 colpack;

		colpack = ((src[cnt * 2]) | (src[cnt * 2 + 1] << 8));
		m_sprite_a_region[cnt * 3 + 0] = (colpack >> 0 ) & 0x1f;
		m_sprite_a_region[cnt * 3 + 1] = (colpack >> 5 ) & 0x1f;
		m_sprite_a_region[cnt * 3 + 2] = (colpack >> 10) & 0x1f;
	}
}

void pgm_state::pgm_basic_init( bool set_bank)
{
	UINT8 *ROM = memregion("maincpu")->base();
	if (set_bank) membank("bank1")->set_base(&ROM[0x100000]);

	expand_colourdata();

	m_bg_videoram = &m_videoram[0];
	m_tx_videoram = &m_videoram[0x4000/2];
	m_rowscrollram = &m_videoram[0x7000/2];
}

DRIVER_INIT_MEMBER(pgm_state,pgm)
{
	pgm_basic_init();
}


/*** GAME ********************************************************************/

GAME( 1997, pgm,          0,         pgm,     pgm, pgm_state,      pgm,        ROT0,   "IGS", "PGM (Polygame Master) System BIOS", MACHINE_IS_BIOS_ROOT )

/* -----------------------------------------------------------------------------------------------------------------------
   Working (at least one set of the game is fully working)
   -----------------------------------------------------------------------------------------------------------------------*/

// the version numbering on these is a mess... date strings from ROM (and in some cases even those are missing..)
GAME( 1997, orlegend,     pgm,       pgm_asic3,     orlegend, pgm_asic3_state, orlegend,   ROT0,   "IGS", "Oriental Legend / Xi You Shi E Zhuan (ver. 126)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )                 // V0001 01/14/98 18:16:38 - runs as World
GAME( 1997, orlegende,    orlegend,  pgm_asic3,     orlegend, pgm_asic3_state, orlegend,   ROT0,   "IGS", "Oriental Legend / Xi You Shi E Zhuan (ver. 112)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )                 // V0001 07/14/97 11:19:45 - runs as World
GAME( 1997, orlegendc,    orlegend,  pgm_asic3,     orlegend, pgm_asic3_state, orlegend,   ROT0,   "IGS", "Oriental Legend / Xi You Shi E Zhuan (ver. 112, Chinese Board)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )  // V0001 05/05/97 10:08:21 - runs as World, Korea, China
GAME( 1997, orlegendca,   orlegend,  pgm_asic3,     orlegend, pgm_asic3_state, orlegend,   ROT0,   "IGS", "Oriental Legend / Xi You Shi E Zhuan (ver. ???, Chinese Board)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )  // V0001 04/02/97 13:35:43 - runs as HongKong, China, China
GAME( 1997, orlegend111c, orlegend,  pgm_asic3,     orlegend, pgm_asic3_state, orlegend,   ROT0,   "IGS", "Oriental Legend / Xi You Shi E Zhuan (ver. 111, Chinese Board)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )  // V0001 no date!          - runs as HongKong, China, China
GAME( 1997, orlegend111t, orlegend,  pgm_asic3,     orlegendt,pgm_asic3_state, orlegend,   ROT0,   "IGS", "Oriental Legend / Xi You Shi E Zhuan (ver. 111, Taiwanese Board)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )// V0001 no date! - needs a different protection sequence
GAME( 1997, orlegend111k, orlegend,  pgm_asic3,     orlegendk,pgm_asic3_state, orlegend,   ROT0,   "IGS", "Oriental Legend / Xi You Shi E Zhuan (ver. 111, Korean Board)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )   // not checked
GAME( 1997, orlegend105k, orlegend,  pgm_asic3,     orlegendk,pgm_asic3_state, orlegend,   ROT0,   "IGS", "Oriental Legend / Xi You Shi E Zhuan (ver. 105, Korean Board)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )  //  V0000 no date!          - runs as Korea

GAME( 1997, drgw2,        pgm,       pgm_012_025_drgw2,     pgm, pgm_012_025_state,      drgw2,      ROT0,   "IGS", "Dragon World II (ver. 110X, Export)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1997, dw2v100x,     drgw2,     pgm_012_025_drgw2,     pgm, pgm_012_025_state,      dw2v100x,   ROT0,   "IGS", "Dragon World II (ver. 100X, Export)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1997, drgw2j,       drgw2,     pgm_012_025_drgw2,     pgm, pgm_012_025_state,      drgw2j,     ROT0,   "IGS", "Chuugokuryuu II (ver. 100J, Japan)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1997, drgw2c,       drgw2,     pgm_012_025_drgw2,     pgm, pgm_012_025_state,      drgw2c,     ROT0,   "IGS", "Zhong Guo Long II (ver. 100C, China)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1997, drgw2hk,      drgw2,     pgm_012_025_drgw2,     pgm, pgm_012_025_state,      drgw2hk,    ROT0,   "IGS", "Dragon World II (ver. 100H, Hong Kong [Hokg Kong])", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING ) // correct title? (region is shown as Hokg Kong, Dragon World 3 is the same)

GAME( 1998, killbld,      pgm,       pgm_022_025_killbld, killbld, pgm_022_025_state,  killbld,    ROT0,   "IGS", "The Killing Blade (ver. 109, Chinese Board)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) /* region provided by protection device */
GAME( 1998, killbld104,   killbld,   pgm_022_025_killbld, killbld, pgm_022_025_state,  killbld,    ROT0,   "IGS", "The Killing Blade (ver. 104)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) /* region provided by protection device */

// these seem playable but the DMA mode transfering 68k code to RAM is not emulated so there could still be problems
// when set to Japan it has the extra subtitle and so gets referred to as Dragon World 3 Special / Chuugokuryuu 3 Special.  The earliest versions seem to only contain the code for the Japanese region, presumably the support for other regions was added later.
GAME( 1998, drgw3,        pgm,       pgm_022_025_dw3,     dw3, pgm_022_025_state,      drgw3,      ROT0,   "IGS", "Dragon World 3 / Chuugokuryuu 3 Special (ver. 106)", MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) /* region provided by protection device */
GAME( 1998, drgw3105,     drgw3,     pgm_022_025_dw3,     dw3, pgm_022_025_state,      drgw3,      ROT0,   "IGS", "Dragon World 3 / Chuugokuryuu 3 Special (ver. 105)", MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) /* region provided by protection device */
GAME( 1998, drgw3103,     drgw3,     pgm_022_025_dw3,     dw3, pgm_022_025_state,      drgw3,      ROT0,   "IGS", "Chuugokuryuu 3 Special (Japan, ver. 103)", MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // Japan only, has an extra game mode option!
GAME( 1998, drgw3100,     drgw3,     pgm_022_025_dw3,     dw3j,pgm_022_025_state,      drgw3,      ROT0,   "IGS", "Chuugokuryuu 3 Special (Japan, ver. 100)", MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // ^

GAME( 1998, dwex,         pgm,       pgm_022_025_dw3,     dw3, pgm_022_025_state,      drgw3,      ROT0,   "IGS", "Dragon World 3 EX (ver. 100)", MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) /* region provided by protection device */

// region provided by internal ARM rom
GAME( 1999, photoy2k,     pgm,       pgm_arm_type1,     photoy2k, pgm_arm_type1_state, photoy2k,   ROT0,   "IGS", "Photo Y2K (ver. 105)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) /* region provided by protection device */
GAME( 1999, photoy2k104,  photoy2k,  pgm_arm_type1,     photoy2k, pgm_arm_type1_state, photoy2k,   ROT0,   "IGS", "Photo Y2K (ver. 104)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) /* region provided by protection device */
GAME( 1999, photoy2k102,  photoy2k,  pgm_arm_type1,     photoy2k, pgm_arm_type1_state, photoy2k,   ROT0,   "IGS", "Photo Y2K (ver. 102, Japanese Board)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) /* region provided by protection device */

// region provided by internal ARM rom
GAME( 1999, kovsh,        pgm,       pgm_arm_type1,     kovsh, pgm_arm_type1_state,    kovsh,      ROT0,   "IGS", "Knights of Valour Super Heroes / Sangoku Senki Super Heroes (ver. 104, CN)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 68k: SANGO EX V104 03/24/00 11:15:25, ARM: China internal ROM
GAME( 1999, kovsh103,     kovsh,     pgm_arm_type1,     kovsh, pgm_arm_type1_state,    kovsh,      ROT0,   "IGS", "Knights of Valour Super Heroes / Sangoku Senki Super Heroes (ver. 103, CN)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 68k: SANGO EX V103 01/18/00 17:37:12, ARM: China internal ROM
GAME( 1999, kovsh102,     kovsh,     pgm_arm_type1,     kovsh, pgm_arm_type1_state,    kovsh,      ROT0,   "IGS", "Knights of Valour Super Heroes / Sangoku Senki Super Heroes (ver. 102, CN)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 68k: SANGO EX V102 12/23/99 15:17:57, ARM: China internal ROM
GAME( 1999, kovsh101,     kovsh,     pgm_arm_type1,     kovsh, pgm_arm_type1_state,    kovsh,      ROT0,   "IGS", "Knights of Valour Super Heroes / Sangoku Senki Super Heroes (ver. 101, CN)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 68k: SANGO EX V101 12/20/99 10:59:05, ARM: China internal ROM
GAME( 1999, kovsh100,     kovsh,     pgm_arm_type1,     kovsh, pgm_arm_type1_state,    kovsh,      ROT0,   "IGS", "Knights of Valour Super Heroes / Sangoku Senki Super Heroes (ver. 100, CN)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 68k: SANGO EX V100 12/06/99 13:36:04, ARM: China internal ROM
// nasty modern asian bootleg of Knights of Valour Super Heroes with characters ripped from SNK's The King of Fighters series!
GAME( 1999, kovqhsgs,     kovsh,     pgm_arm_type1,     kovsh, pgm_arm_type1_state, kovqhsgs,   ROT0,   "bootleg", "Knights of Valour: Quan Huang San Guo Special / Sangoku Senki: Quan Huang San Guo Special (ver. 303CN)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )






// region provided by internal ARM rom
GAME( 2000, kov2,         pgm,       pgm_arm_type2,    kov2, pgm_arm_type2_state,    kov2,       ROT0,   "IGS", "Knights of Valour 2 / Sangoku Senki 2 (ver. 107, 102, 100HK)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 05/10/01 14:24:08 V107 (Ext. Arm V102, Int. Arm V100HK)
GAME( 2000, kov2106,      kov2,      pgm_arm_type2,    kov2, pgm_arm_type2_state,    kov2,       ROT0,   "IGS", "Knights of Valour 2 / Sangoku Senki 2 (ver. 106, 102, 100HK)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 02/27/01 13:26:46 V106 (Ext. Arm V102, Int. Arm V100HK)
GAME( 2000, kov2103,      kov2,      pgm_arm_type2,    kov2, pgm_arm_type2_state,    kov2,       ROT0,   "IGS", "Knights of Valour 2 / Sangoku Senki 2 (ver. 103, 101, 100HK)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 12/28/00 15:09:31 V103 (Ext. Arm V101, Int. Arm V100HK)
GAME( 2000, kov2102,      kov2,      pgm_arm_type2,    kov2, pgm_arm_type2_state,    kov2,       ROT0,   "IGS", "Knights of Valour 2 / Sangoku Senki 2 (ver. 102, 101, 100HK)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 12/14/00 10:33:36 V102 (Ext. Arm V101, Int. Arm V100HK)
GAME( 2000, kov2101,      kov2,      pgm_arm_type2,    kov2, pgm_arm_type2_state,    kov2,       ROT0,   "IGS", "Knights of Valour 2 / Sangoku Senki 2 (ver. 101, 101, 100HK)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 11/29/00 11:03:08 V100 (Ext. Arm V100, Int. Arm V100HK)
GAME( 2000, kov2100,      kov2,      pgm_arm_type2,    kov2, pgm_arm_type2_state,    kov2,       ROT0,   "IGS", "Knights of Valour 2 / Sangoku Senki 2 (ver. 100, 100, 100HK)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 11/29/00 11:03:08 V100 (Ext. Arm V100, Int. Arm V100HK)

// region provided by internal ARM rom (we only have a China internal ROM)
GAME( 2001, kov2p,        pgm,       pgm_arm_type2,    kov2, pgm_arm_type2_state,    kov2p,      ROT0,   "IGS", "Knights of Valour 2 Plus - Nine Dragons / Sangoku Senki 2 Plus - Nine Dragons (ver. M205XX, 200, 100CN)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 04/25/02  17:48:27 M205XX
GAME( 2001, kov2p204,     kov2p,     pgm_arm_type2,    kov2, pgm_arm_type2_state,    kov2p,      ROT0,   "IGS", "Knights of Valour 2 Plus - Nine Dragons / Sangoku Senki 2 Plus - Nine Dragons (ver. M204XX, 200, 100CN)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 08/28/01  09:11:49 M204XX
GAME( 2001, kov2p202,     kov2p,     pgm_arm_type2,    kov2, pgm_arm_type2_state,    kov2p,      ROT0,   "IGS", "Knights of Valour 2 Plus - Nine Dragons / Sangoku Senki 2 Plus - Nine Dragons (ver. M202XX, 200, 100CN)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 07/09/01  11:03:50 M202XX




// region provided by internal ARM rom
GAME( 2001, martmast,     pgm,       pgm_arm_type2,    martmast, pgm_arm_type2_state,  martmast,   ROT0,   "IGS", "Martial Masters (ver. 104, 102, 102US)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 68k V104, Ext Arm 102, Int Arm 102US
GAME( 2001, martmastc,    martmast,  pgm_arm_type2,    martmast, pgm_arm_type2_state,  martmast,   ROT0,   "IGS", "Martial Masters (ver. 104, 102, 101CN)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 68k V104, Ext Arm 102, Int Arm 101CN
GAME( 2001, martmastc102, martmast,  pgm_arm_type2,    martmast, pgm_arm_type2_state,  martmast,   ROT0,   "IGS", "Martial Masters (ver. 102, 101, 101CN)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 68k V102, Ext Arm 101, Int Arm 101CN

// region provided by internal ARM rom
GAME( 2001, ddp2,         pgm,       pgm_arm_type2,    pgm, pgm_arm_type2_state,     ddp2,       ROT270, "IGS", "DoDonPachi II - Bee Storm (World, ver. 102)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2101,      ddp2,      pgm_arm_type2,    pgm, pgm_arm_type2_state,     ddp2,       ROT270, "IGS", "DoDonPachi II - Bee Storm (World, ver. 101)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2100,      ddp2,      pgm_arm_type2,    pgm, pgm_arm_type2_state,     ddp2,       ROT270, "IGS", "DoDonPachi II - Bee Storm (World, ver. 100)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2hk,       ddp2,      pgm_arm_type2,    pgm, pgm_arm_type2_state,     ddp2,       ROT270, "IGS", "DoDonPachi II - Bee Storm (Hong Kong, ver. 102)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2101hk,    ddp2,      pgm_arm_type2,    pgm, pgm_arm_type2_state,     ddp2,       ROT270, "IGS", "DoDonPachi II - Bee Storm (Hong Kong, ver. 101)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2100hk,    ddp2,      pgm_arm_type2,    pgm, pgm_arm_type2_state,     ddp2,       ROT270, "IGS", "DoDonPachi II - Bee Storm (Hong Kong, ver. 100)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2k,        ddp2,      pgm_arm_type2,    pgm, pgm_arm_type2_state,     ddp2,       ROT270, "IGS", "DoDonPachi II - Bee Storm (Korea, ver. 102)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2101k,     ddp2,      pgm_arm_type2,    pgm, pgm_arm_type2_state,     ddp2,       ROT270, "IGS", "DoDonPachi II - Bee Storm (Korea, ver. 101)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2100k,     ddp2,      pgm_arm_type2,    pgm, pgm_arm_type2_state,     ddp2,       ROT270, "IGS", "DoDonPachi II - Bee Storm (Korea, ver. 100)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2j,        ddp2,      pgm_arm_type2,    pgm, pgm_arm_type2_state,     ddp2,       ROT270, "IGS", "DoDonPachi II - Bee Storm (Japan, ver. 102)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2101j,     ddp2,      pgm_arm_type2,    pgm, pgm_arm_type2_state,     ddp2,       ROT270, "IGS", "DoDonPachi II - Bee Storm (Japan, ver. 101)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2100j,     ddp2,      pgm_arm_type2,    pgm, pgm_arm_type2_state,     ddp2,       ROT270, "IGS", "DoDonPachi II - Bee Storm (Japan, ver. 100)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2t,        ddp2,      pgm_arm_type2,    pgm, pgm_arm_type2_state,     ddp2,       ROT270, "IGS", "DoDonPachi II - Bee Storm (Taiwan, ver. 102)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2101t,     ddp2,      pgm_arm_type2,    pgm, pgm_arm_type2_state,     ddp2,       ROT270, "IGS", "DoDonPachi II - Bee Storm (Taiwan, ver. 101)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2100t,     ddp2,      pgm_arm_type2,    pgm, pgm_arm_type2_state,     ddp2,       ROT270, "IGS", "DoDonPachi II - Bee Storm (Taiwan, ver. 100)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2c,        ddp2,      pgm_arm_type2,    pgm, pgm_arm_type2_state,     ddp2,       ROT270, "IGS", "DoDonPachi II - Bee Storm (China, ver. 102)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2101c,     ddp2,      pgm_arm_type2,    pgm, pgm_arm_type2_state,     ddp2,       ROT270, "IGS", "DoDonPachi II - Bee Storm (China, ver. 101)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, ddp2100c,     ddp2,      pgm_arm_type2,    pgm, pgm_arm_type2_state,     ddp2,       ROT270, "IGS", "DoDonPachi II - Bee Storm (China, ver. 100)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )


// japan region only? service mode calls it Dragon World 2001 so I'm leaving that title in the description
GAME( 2001, dw2001,       pgm,       pgm_arm_type2,     dw2001, pgm_arm_type2_state,   dw2001,    ROT0,   "IGS", "Chuugokuryuu 2001 [Dragon World 2001] (V100?, Japan)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 02/21/01 16:05:16

// japan region only? service mode calls it Dragon World Pretty Chance so I'm leaving that title in the description
GAME( 2001, dwpc,         pgm,       pgm_arm_type2,     dw2001, pgm_arm_type2_state,   dwpc,      ROT0,   "IGS", "Chuugokuryuu Pretty Chance [Dragon World Pretty Chance] (V101, Japan)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 09/26/01 10:23:26

// we bypass the internal ARM rom on these, ideally it should still be dumped tho! the region screens show a blank string where the internal ROM revision would otherwise be displayed
// ARM version strings don't match 100% with labels... for 68k ROMs I'm using the build time / date stamp from near the start of the rom, there are some slightly different time stamps later
GAME( 2002, dmnfrnt,      pgm,       pgm_arm_type3,     pgm, pgm_arm_type3_state,    dmnfrnt,    ROT0,   "IGS", "Demon Front (68k label V105, ROM M105XX 08/05/02) (ARM label V105, ROM 08/05/02 S105XX)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 68k time: 10:24:11 ARM time: 10:33:23
GAME( 2002, dmnfrntb,     dmnfrnt,   pgm_arm_type3,     pgm, pgm_arm_type3_state,    dmnfrnt,    ROT0,   "IGS", "Demon Front (68k label V103, ROM M103XX 07/05/02) (ARM label V103, ROM 07/05/02 S103XX)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 68k time: 14:43:13 ARM time: 11:04:24
GAME( 2002, dmnfrnta,     dmnfrnt,   pgm_arm_type3,     pgm, pgm_arm_type3_state,    dmnfrnt,    ROT0,   "IGS", "Demon Front (68k label V102, ROM M102XX 06/19/02) (ARM label V102, ROM 05/24/02 S101XX)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // 68k time: 13:44:08 ARM time: 13:04:31  (from the strings it looks like V102 only upgraded the 68k ROM)
GAME( 2002, dmnfrntpcb,   dmnfrnt,   pgm_arm_type3,     pgm, pgm_arm_type3_state,    dmnfrnt,    ROT0,   "IGS", "Demon Front (68k label V107KR, ROM M107KR 11/03/03) (ARM label V106KR, ROM 10/16/03 S106KR) (JAMMA PCB)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // works but reports version mismatch (wants internal rom version and region to match external?)


/* these don't use an External ARM rom, and don't have any weak internal functions which would allow the internal ROM to be read out */
GAME( 2002, ddpdoj,       0,         pgm_arm_type1_cave,    pgm, pgm_arm_type1_state,     ddp3,      ROT270, "Cave (AMI license)", "DoDonPachi Dai-Ou-Jou V101 (2002.04.05.Master Ver)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // is there a v101 without the . after 05?
GAME( 2002, ddpdoja,    ddpdoj,      pgm_arm_type1_cave,    pgm, pgm_arm_type1_state,     ddp3,      ROT270, "Cave (AMI license)", "DoDonPachi Dai-Ou-Jou V100 (2002.04.05.Master Ver)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2002, ddpdojb,    ddpdoj,      pgm_arm_type1_cave,    pgm, pgm_arm_type1_state,     ddp3,      ROT270, "Cave (AMI license)", "DoDonPachi Dai-Ou-Jou (2002.04.05 Master Ver)",      MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2002, ddpdojblk,  ddpdoj,      pgm_arm_type1_cave,    pgm, pgm_arm_type1_state,     ddp3,      ROT270, "Cave (AMI license)", "DoDonPachi Dai-Ou-Jou (2002.10.07.Black Ver)",                MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // Displays "2002.04.05.Master Ver" (old) or "2002.10.07.Black Ver" (new)
GAME( 2002, ddpdojblka, ddpdoj,      pgm_arm_type1_cave,    pgm, pgm_arm_type1_state,     ddp3,      ROT270, "Cave (AMI license)", "DoDonPachi Dai-Ou-Jou (2002.10.07 Black Ver)",                MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // Displays "2002.04.05.Master Ver" (old) or "2002.10.07 Black Ver" (new)

// the exact text of the 'version' shows which revision of the game it is; the newest has 2 '.' symbols in the string, the oldest, none.
// the only difference between 'ket' and 'ket1' is the ROM fill at 0x1443bc-0x1c88cd, on ket1 it seems to be randomized / garbage data, on ket it's all 0xff, both have been seen on more than one PCB.
GAME( 2002, ket,          0,         pgm_arm_type1_cave,    pgm, pgm_arm_type1_state,     ket,       ROT270, "Cave (AMI license)", "Ketsui: Kizuna Jigoku Tachi (2003/01/01. Master Ver.)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2002, ket1,         ket,       pgm_arm_type1_cave,    pgm, pgm_arm_type1_state,     ket,       ROT270, "Cave (AMI license)", "Ketsui: Kizuna Jigoku Tachi (2003/01/01. Master Ver.) (alt rom fill)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2002, keta,         ket,       pgm_arm_type1_cave,    pgm, pgm_arm_type1_state,     ket,       ROT270, "Cave (AMI license)", "Ketsui: Kizuna Jigoku Tachi (2003/01/01 Master Ver.)",  MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2002, ketb,         ket,       pgm_arm_type1_cave,    pgm, pgm_arm_type1_state,     ket,       ROT270, "Cave (AMI license)", "Ketsui: Kizuna Jigoku Tachi (2003/01/01 Master Ver)",   MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

// these are modern hacks, some of them have been seen on original PCBs, also reportedly on a bootleg PCB with mostly original components but the ARM replaced with a custom chip.
// this is a significantly reworked version of the game
GAME( 2014, ketarr,    ket,       pgm_arm_type1_cave, pgm,      pgm_arm_type1_state, ket,        ROT270, "hack (trap15)", "Ketsui: Kizuna Jigoku Tachi (2014/07/16 ARRANGE 1.7 VER) (hack)", MACHINE_SUPPORTS_SAVE )
GAME( 2012, ketarr151, ket,       pgm_arm_type1_cave, pgm,      pgm_arm_type1_state, ket,        ROT270, "hack (trap15)", "Ketsui: Kizuna Jigoku Tachi (2012/06/26 ARRANGE 1.51 VER) (hack)", MACHINE_SUPPORTS_SAVE ) // this apparently crashes on an original PGM PCB when displaying the text after starting a game, find out why and reproduce the issue in MAME.
GAME( 2012, ketarr15,  ket,       pgm_arm_type1_cave, pgm,      pgm_arm_type1_state, ket,        ROT270, "hack (trap15)", "Ketsui: Kizuna Jigoku Tachi (2012/06/26 ARRANGE 1.5 VER) (hack)", MACHINE_SUPPORTS_SAVE )
GAME( 2012, ketarr10,  ket,       pgm_arm_type1_cave, pgm,      pgm_arm_type1_state, ket,        ROT270, "hack (trap15)", "Ketsui: Kizuna Jigoku Tachi (2012/04/17 ARRANGE VER) (hack)", MACHINE_SUPPORTS_SAVE )

// these simplify the scoring system
GAME( 2012, ketarrs151, ket,       pgm_arm_type1_cave, pgm,      pgm_arm_type1_state, ket,        ROT270, "hack (trap15)", "Ketsui: Kizuna Jigoku Tachi (2012/06/27 MR.STOIC 1.51 VER) (hack)", MACHINE_SUPPORTS_SAVE )
GAME( 2012, ketarrs15,  ket,       pgm_arm_type1_cave, pgm,      pgm_arm_type1_state, ket,        ROT270, "hack (trap15)", "Ketsui: Kizuna Jigoku Tachi (2012/06/27 MR.STOIC 1.5 VER) (hack)", MACHINE_SUPPORTS_SAVE )

// this has the 'programmed slowdown' removed.
GAME( 2012, ketarrf,    ket,       pgm_arm_type1_cave, pgm,      pgm_arm_type1_state, ket,        ROT270, "hack (trap15)", "Ketsui: Kizuna Jigoku Tachi (2012/04/17 FAST. VER) (hack)", MACHINE_SUPPORTS_SAVE )

// this version is stupid, it just simulates what happens if the protection chip isn't returning proper values
// ROM_LOAD16_WORD_SWAP( "ketarrb_v100.u38", 0x000000, 0x200000, CRC(ec7a4f92) SHA1(6351fb386586956fbdb5f0730c481fb539cc267a) )
// GAME( 2002, ketarrb,    ket,       pgm_arm_type1_cave, pgm,      pgm_arm_type1_state, ket,        ROT270, "trap15", "Ketsui: Kizuna Jigoku Tachi (2012/04/17 BACK. VER)", MACHINE_SUPPORTS_SAVE )


GAME( 2003, espgal,       0,         pgm_arm_type1_cave,    pgm, pgm_arm_type1_state,     espgal,    ROT270, "Cave (AMI license)", "Espgaluda (2003/10/15 Master Ver)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

// protection simulated, but should be correct
GAME( 1999, puzzli2,      pgm,       pgm_arm_type1_sim,  puzzli2, pgm_arm_type1_state,    puzzli2,    ROT0,   "IGS", "Puzzli 2 (ver. 100)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // ROM label is V100 ( V0001, 11/22/99 09:27:58 in program ROM )
GAME( 2001, puzzli2s,     puzzli2,   pgm_arm_type1_sim,  puzzli2, pgm_arm_type1_state,    puzzli2,    ROT0,   "IGS", "Puzzli 2 Super (ver. 200)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )  // ( V200, 12/28/01 12:53:34 in program ROM )

GAME( 2005, killbldp,     pgm,       pgm_arm_type3,     pgm, pgm_arm_type3_state,    killbldp,   ROT0,   "IGS", "The Killing Blade Plus (China, ver. 300)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) /* using internal rom from bootleg */

// we're using a partial dump of the internal rom (sans the execute only area) with handcrafted startup code..
// all 3 68k roms still have V100 strings, but are clearly different builds, there don't appear to be build string dates in them.  Two of the external ARM roms are marked V100 but are different builds, the single PCB v100 appears to be a later revision than the cart V100 as it shares the internal ROM with the V107 cart version, the v100 cart has a different internal ROM
GAME( 2003, theglad,      pgm,       pgm_arm_type3,     theglad, pgm_arm_type3_state,    theglad,    ROT0,   "IGS", "The Gladiator / Road of the Sword / Shen Jian (M68k label V101) (ARM label V107, ROM 06/06/03 SHEN JIAN V107)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // ARM time: 16:17:27
GAME( 2003, theglad101,   theglad,   pgm_arm_type3,     theglad, pgm_arm_type3_state,    theglad,    ROT0,   "IGS", "The Gladiator / Road of the Sword / Shen Jian (M68k label V100) (ARM label V101, ROM 03/13/03 SHEN JIAN)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // ARM time: 14:06:44
// the v100 68k ROM on this is older than the v101 set, this set also uses a different internal ROM to everything else, must be a very early release, maybe pre v100 proto with v100 strings?
GAME( 2003, theglad100,   theglad,   pgm_arm_type3,     theglad, pgm_arm_type3_state,    theglada,   ROT0,   "IGS", "The Gladiator / Road of the Sword / Shen Jian (M68k label V100) (ARM label V100, ROM 01/16/03 SHEN JIAN)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) /* need correct internal rom of IGS027A - we currently patch the one we have */ // ARM time: 10:39:25
// newer than ARM V100 Cart, older than ARM V101 Cart, same 68k rom as V101 Cart.
GAME( 2003, thegladpcb,   theglad,   pgm_arm_type3,     pgm,    pgm_arm_type3_state,    theglad,    ROT0,   "IGS", "The Gladiator / Road of the Sword / Shen Jian (M68k label V100) (ARM label V100, ROM 02/25/03 SHEN JIAN) (Japan, JAMMA PCB)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )// ARM time: 16:32:21 // PCB version only released in Japan?

GAME( 2005, svg,          pgm,       pgm_arm_type3,     svg, pgm_arm_type3_state,    svg,        ROT0,   "IGS / Idea Factory", "S.V.G. - Spectral vs Generation (M68k label V200) (ARM label V200, ROM 10/11/05 S.V.G V201)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // ARM label was 200, but it's code rev 201? // ARM time: 10:07:20
GAME( 2005, svgtw,        svg,       pgm_arm_type3,     svgtw,pgm_arm_type3_state,   svgpcb,     ROT0,   "IGS / Idea Factory", "S.V.G. - Spectral vs Generation (M68k label V101TW) (ARM label V101TW, ROM 06/20/05 S.V.G V100)", MACHINE_NOT_WORKING ) // 68k label was 101 but it's same as v100
GAME( 2005, svgpcb,       svg,       pgm_arm_type3,     svg, pgm_arm_type3_state,    svgpcb,     ROT0,   "IGS / Idea Factory", "S.V.G. - Spectral vs Generation (M68k label V100JP) (ARM label V100JP, ROM 05/12/05 S.V.G V100) (Japan, JAMMA PCB)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )// ARM time: 15:31:35 // PCB version only released in Japan?

GAME( 2004, happy6,       pgm,       pgm_arm_type3,     happy6, pgm_arm_type3_state,    happy6,     ROT0,   "IGS", "Happy 6-in-1 (ver. 102CN)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2004, happy6101,    happy6,    pgm_arm_type3,     happy6, pgm_arm_type3_state,    happy6,     ROT0,   "IGS", "Happy 6-in-1 (ver. 101CN)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

/* -----------------------------------------------------------------------------------------------------------------------
   Partially Working, playable, but some imperfections
   -----------------------------------------------------------------------------------------------------------------------*/

GAME( 1998, olds,         pgm,       pgm_028_025_ol,    olds, pgm_028_025_state,     olds,       ROT0,   "IGS", "Oriental Legend Special / Xi You Shi E Zhuan Super (ver. 101, Korean Board)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE ) /* region provided by protection device */
GAME( 1998, olds100,      olds,      pgm_028_025_ol,    olds, pgm_028_025_state,     olds,       ROT0,   "IGS", "Oriental Legend Special / Xi You Shi E Zhuan Super (ver. 100, set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE ) /* region provided by protection device */
GAME( 1998, olds100a,     olds,      pgm_028_025_ol,    olds, pgm_028_025_state,     olds,       ROT0,   "IGS", "Oriental Legend Special / Xi You Shi E Zhuan Super (ver. 100, set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE ) /* region provided by protection device */
// This version was specially made for a Chinese online gaming company. While it may not be entirely suitable for MAME, it can give some insight into how protection should work.
GAME( 1998, olds103t,     olds,      pgm,               pgm,  pgm_state,             pgm,        ROT0,   "bootleg", "Oriental Legend Special / Xi You Shi E Zhuan Super (ver. 103, China, Tencent) (unprotected)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1999, kov,          pgm,       pgm_arm_type1_sim,     sango, pgm_arm_type1_state,    kov,        ROT0,   "IGS", "Knights of Valour / Sangoku Senki (ver. 117)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */                 // V0008 04/27/99 10:33:33
GAME( 1999, kov115,       kov,       pgm_arm_type1_sim,     sango, pgm_arm_type1_state,    kov,        ROT0,   "IGS", "Knights of Valour / Sangoku Senki (ver. 115)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */                 // V0006 02/22/99 11:53:18
GAME( 1999, kov100,       kov,       pgm_arm_type1_sim,     sango, pgm_arm_type1_state,    kov,        ROT0,   "IGS", "Knights of Valour / Sangoku Senki (ver. 100, Japanese Board)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */ // V0002 01/31/99 01:54:16

GAME( 1999, kovplus,      pgm,       pgm_arm_type1_sim,     sango, pgm_arm_type1_state,    kov,        ROT0,   "IGS", "Knights of Valour Plus / Sangoku Senki Plus (ver. 119, set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
GAME( 1999, kovplusa,     kovplus,   pgm_arm_type1_sim,     sango, pgm_arm_type1_state,    kov,        ROT0,   "IGS", "Knights of Valour Plus / Sangoku Senki Plus (ver. 119, set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */

// modified title screen is only visible for china region, so use that by default.  Character select portraits don't seem quite right (different protection?)
GAME( 1999, kovsgqyz,     kovplus,   pgm_arm_type1_sim,     sango_ch, pgm_arm_type1_state,    kovboot,    ROT0,   "bootleg", "Knights of Valour: SanGuo QunYingZhuan / Sangoku Senki: SanGuo QunYingZhuan (bootleg, set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
GAME( 1999, kovsgqyza,    kovplus,   pgm_arm_type1_sim,     sango_ch, pgm_arm_type1_state,    kovboot,    ROT0,   "bootleg", "Knights of Valour: SanGuo QunYingZhuan / Sangoku Senki: SanGuo QunYingZhuan (bootleg, set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
GAME( 1999, kovsgqyzb,    kovplus,   pgm_arm_type1_sim,     sango_ch, pgm_arm_type1_state,    kovboot,    ROT0,   "bootleg", "Knights of Valour: SanGuo QunYingZhuan / Sangoku Senki: SanGuo QunYingZhuan (bootleg, set 3)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */


/* -----------------------------------------------------------------------------------------------------------------------
   NOT Working (mostly due to needing internal protection roms dumped)
   -----------------------------------------------------------------------------------------------------------------------*/

GAME( 1999, puzlstar,     pgm,       pgm_arm_type1_sim,    pstar, pgm_arm_type1_state,    pstar,      ROT0,   "IGS", "Puzzle Star (ver. 100MG)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */

GAME( 2001, py2k2,        pgm,       pgm_arm_type1_sim,    py2k2, pgm_arm_type1_state,    py2k2,      ROT0,   "IGS", "Photo Y2K 2", MACHINE_NOT_WORKING )  /* need internal rom of IGS027A */
GAME( 2004, pgm3in1,      pgm,       pgm_arm_type1_sim,    py2k2, pgm_arm_type1_state,    pgm3in1,      ROT0,   "IGS", "Photo Y2K 2 (Flash 3-in-1)", MACHINE_NOT_WORKING )  /* need internal rom of IGS027A */


/* Games below this point are known to have an 'execute only' internal ROM area covering an area at the start of the internal ROM.  This can't be read when running code from either internal or external ROM space. */


// simulation doesn't seem 100%
GAME( 2004, oldsplus,     pgm,       pgm_arm_type1_sim, oldsplus, pgm_arm_type1_state,     oldsplus,   ROT0,   "IGS", "Oriental Legend Special Plus / Xi You Shi E Zhuan Super Plus", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */

// we use the kovsh ARM rom for this, itercepting commands and changing them to match it, doesn't seem 100% correct tho so I'm leaving it as NOT WORKING; for example the ARM rom supplies addresses of Z80 music data sections, which have moved causing incorrect music, some damage rates could be different too.
// the game logo remains stuck on the screen during gameplay, but videos of the original board suggest this happens on real hardware as well
// if the internal ROM can't be extracted (likely case, execute only area and NO chance of custom code execution at all due to lack of external ROM) then a reference simulator should probably be written based on the actual
// kovsh code, tweaked based on tests done with this specific board to catch any different behaviors.  These all seem to be for China only, they don't work as expected when set to other regions.
GAME( 2004, kovshp,       pgm,       pgm_arm_type1,     kovsh, pgm_arm_type1_state,    kovshp,     ROT0,   "IGS", "Knights of Valour Super Heroes Plus / Sangoku Senki Super Heroes Plus (ver. 101)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
GAME( 2004, kovshpa,      kovshp,    pgm_arm_type1,     kovsh, pgm_arm_type1_state,    kovshp,     ROT0,   "IGS", "Knights of Valour Super Heroes Plus / Sangoku Senki Super Heroes Plus (ver. 100)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
// Seems to be an earlier version, uses same PCB as kovsh.
GAME( 1999, kovytzy,      pgm,       pgm_arm_type1,     kovsh, pgm_arm_type1_state,    kovshp,     ROT0,   "IGS", "Knights of Valour: Yi Tong Zhong Yuan / Sangoku Senki: Yi Tong Zhong Yuan (ver. 201, China)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
// this bootleg is very close to kovshp
GAME( 2004, kovshxas,     kovshp,    pgm_arm_type1,     kovsh, pgm_arm_type1_state, kovshxas,   ROT0,   "bootleg", "Knights of Valour: Aoshi Sanguo / Sangoku Senki: Aoshi Sanguo (ver. 202CN)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
// these should be bootlegs of kovshp, but have further command changes in their ARMs and have a lot of code shuffled around, bootlegs of a different revision?
GAME( 2004, kovlsqh,      kovshp,    pgm_arm_type1,     kovsh, pgm_arm_type1_state, kovlsqh2,   ROT0,   "bootleg", "Knights of Valour: Luan Shi Quan Huang / Sangoku Senki: Luan Shi Quan Huang (ver. 200CN)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
GAME( 2004, kovlsqh2,     kovshp,    pgm_arm_type1,     kovsh, pgm_arm_type1_state, kovlsqh2,   ROT0,   "bootleg", "Knights of Valour: Luan Shi Quan Huang 2 / Sangoku Senki: Luan Shi Quan Huang 2 (ver. 200CN)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
GAME( 2004, kovlsjb,      kovshp,    pgm_arm_type1,     kovsh, pgm_arm_type1_state, kovlsqh2,   ROT0,   "bootleg", "Knights of Valour: Luan Shi Jie Ba / Sangoku Senki: Luan Shi Jie Ba (ver. 200CN, set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
GAME( 2004, kovlsjba,     kovshp,    pgm_arm_type1,     kovsh, pgm_arm_type1_state, kovlsqh2,   ROT0,   "bootleg", "Knights of Valour: Luan Shi Jie Ba / Sangoku Senki: Luan Shi Jie Ba (ver. 200CN, set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) /* need internal rom of IGS027A */

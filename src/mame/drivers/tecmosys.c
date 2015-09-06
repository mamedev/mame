// license:BSD-3-Clause
// copyright-holders:Farfetch'd,David Haywood,Tomasz Slanina
/* Tecmo System
 Driver by Farfetch'd, David Haywood & Tomasz Slanina
 Protection simulation by nuapete

 ToDo:
  - Dump / Decap MCUs to allow for proper protection emulation.
  - tkdenshoa pcb shows some garbage sprites at the top during y-scroll


T.Slanina 20040530 :
 - preliminary gfx decode,
 - Angel Eyes - patched interrupt level1 vector
 - EEPROM r/w
 - txt layer
 - added hacks to see more gfx (press Z or X)
 - palette (press X in angel eyes to see 'color bar chack'(!))
 - watchdog (?) simulation

 20080528
 - Removed ROM patches and debug keypresses
 - Added protection simulation in machine/tecmosys.c
 - Fixed inputs
 - Added watchdog

   To enter test mode, you have to press the test switch before you insert any coins.

*/


/*

Deroon Dero Dero
Tecmo, 1996

This game is a Puyo Puyo rip-off.

PCB Layout
----------

TECMO SYSTEM BOARD A
|-------------------------------------------------------------------------|
|  LM324  UPC452C      16.9MHz          |--------|    |--------|    6264  |
| TA8205 LM324  YAC513 YMF262 YMZ280B   |TECMO   |    |TECMO   |    6264  |
|        LM324  M6295  UPC452C          |AA03-8431    |AA02-1927          |
|                      YAC512           |        |    |        |          |
|                                       |--------|    |--------|          |
|        Z80  6264 28MHz 14.31818MHz                  |--------|          |
|                    16MHz             62256          |TECMO   |          |
|            TA8030                    62256          |AA02-1927    6264  |
|                                                     |        |    6264  |
|J  93C46                               |--------|    |--------|          |
|A                                      |TECMO   |    |--------|          |
|M                                      |AA03-8431    |TECMO   |          |
|M          68000                       |        |    |AA02-1927          |
|A                                      |--------|    |        |    6264  |
|                  PAL              6116              |--------|    6264  |
|                                    6116             |--------|          |
|  |--------|                                         |TECMO   |          |
|  |TECMO   |                     PAL                 |AA02-1927          |
|  |AA03-8431  62256                                  |        |    6264  |
|  |        |  62256                                  |--------|    6264  |
|  |--------|                                  |---------|                |
|                                              |TECMO    |                |
|                                              |AA03-8431|                |
|                                              |         |                |
|                                              |---------|          424260|
|                                              62256 62256          424260|
|-------------------------------------------------------------------------|
Notes:
68000 @ 16MHz
Z80 @ 8MHz [16/2]
YMZ280B @ 16.9MHz
YMF262 @ 14.31818MHz
OKI M6295 @ 2MHz [16/8]. Pin 7 HIGH

Game Board
----------

TECMO SYSTEM BOARD B2
|-------------------------------------------------------------------------|
|    T201_DIP42_MASK.UBB1                                                 |
| |----|                                              T202_DIP42_MASK.UBC1|
| |*   |                                                                  |
| |----|                                                                  |
|                                                                         |
|  T003_2M_EPROM.UZ1                        T101_SOP44.UAH1               |
|                                                                         |
|                                            T301_DIP42_MASK.UBD1         |
|                                                                         |
|                                                                         |
|                                                                         |
|  T401_DIP42_MASK.UYA1      T104_SOP44.UCL1    T001_4M_EPROM.UPAU1       |
|                              T103_SOP44.UBL1                            |
|  T501_DIP32_MASK.UAD1      T102_SOP44.UAL1                              |
|                                                  T002_4M_EPROM.UPAL1    |
|-------------------------------------------------------------------------|
Notes:
      * - QFP64 microcontroller marked 'TECMO SC432146FU E23D 185 SSAB9540B'
          this is a 68HC11A8 with 8k ROM, 512 bytes EEPROM and 256 bytes on-chip RAM.
          Clocks: pin 33 - 8MHz, pin 31: 8MHz, pin 29 - 2MHz
          GND on pins 49, 23, 24, 27
          Power on pins 55, 25
          Note - Pins 25 and 27 are tied to some jumpers, so these
          appear to be some kind of configuration setting.

CPU  : TMP68HC000P-16
Sound: TMPZ84C00AP-8 YMF262 YMZ280B M6295
OSC  : 14.3181MHz (X1) 28.0000MHz (X2) 16.0000MHz (X3) 16.9MHz (X4)

Custom chips:
TECMO AA02-1927 (160pin PQFP) (x4)
TECMO AA03-8431 (208pin PQFP) (x4)

Others:
93C46 EEPROM (settings are stored to this)

ROMs:

name            type
t001.upau1      27c040 dip32 eprom
t002.upal1      27c040 dip32 eprom
t003.uz1        27c2001 dip32 eprom

t101.uah1       23c16000 sop44 maskrom
t102.ual1       23c16000 sop44 maskrom
t103.ubl1       23c32000 sop44 maskrom
t104.ucl1       23c16000 sop44 maskrom
t201.ubb1       23c8000 dip42 maskrom
t202.ubc1       23c8000 dip42 maskrom
t301.ubd1       23c8000 dip42 maskrom
t401.uya1       23c16000 dip42 maskrom
t501.uad1       23c4001 dip32 maskrom

*/

/*

Toukidenshou -Angel Eyes-
(c)1996 Tecmo
Tecmo System Board

CPU  : TMP68HC000P-16
Sound: TMPZ84C00AP-8 YMF262 YMZ280B M6295
OSC  : 14.3181MHz (X1) 28.0000MHz (X2) 16.0000MHz (X3) 16.9MHz (X4)

Custom chips:
TECMO AA02-1927 (160pin PQFP) (x4)
TECMO AA03-8431 (208pin PQFP) (x4)

Others:
93C46 EEPROM (settings are stored to this)

EPROMs:
aeprge-2.pal - Main program (even) (27c4001)
aeprgo-2.pau - Main program (odd)  (27c4001)

aesprg-2.z1 - Sound program (27c1001)

Mask ROMs:
ae100h.ah1 - Graphics (23c32000/16000 SOP)
ae100.al1  |
ae101h.bh1 |
ae101.bl1  |
ae102h.ch1 |
ae102.cl1  |
ae104.el1  |
ae105.fl1  |
ae106.gl1  /

ae200w74.ba1 - Graphics (23c16000)
ae201w75.bb1 |
ae202w76.bc1 /

ae300w36.bd1 - Graphics (23c4000)

ae400t23.ya1 - YMZ280B Samples (23c16000)
ae401t24.yb1 /

ae500w07.ad1 - M6295 Samples (23c4001)

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "includes/tecmosys.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "sound/262intf.h"
#include "sound/ymz280b.h"


// It looks like this needs a synch between z80 and 68k ??? See z80:006A-0091
READ16_MEMBER(tecmosys_state::sound_r)
{
	if (ACCESSING_BITS_0_7)
	{
		machine().scheduler().synchronize();
		return soundlatch2_byte_r(space,  0 );
	}

	return 0;
}

WRITE16_MEMBER(tecmosys_state::sound_w)
{
	if (ACCESSING_BITS_0_7)
	{
		machine().scheduler().synchronize();
		soundlatch_byte_w(space, 0x00, data & 0xff);
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}
}

/*
    880000 and 880002 might be video related,
    see sub @ 68k:002e5e where they are written if the screen is set to inverted.
    Also, irq code at 22c4 :
    - 880000 & 00, execute irq code
    - 880000 & 01, scroll?
    - 880000 & 03, crash
*/

WRITE16_MEMBER(tecmosys_state::unk880000_w)
{
	COMBINE_DATA(&m_880000regs[offset]);

	switch( offset )
	{
		case 0x00/2:
			break; // global x scroll for sprites?

		case 0x02/2:
			break; // global y scroll for sprites

		case 0x08/2:
			m_spritelist = data & 0x3; // which of the 4 spritelists to use (buffering)
			break;

		case 0x22/2:
			machine().watchdog_reset();
			//logerror( "watchdog_w( %06x, %04x ) @ %06x\n", (offset * 2)+0x880000, data, space.device().safe_pc() );
			break;

		default:
			logerror( "unk880000_w( %06x, %04x ) @ %06x\n", (offset * 2)+0x880000, data, space.device().safe_pc() );
			break;
	}
}

READ16_MEMBER(tecmosys_state::unk880000_r)
{
	//UINT16 ret = m_880000regs[offset];

	logerror( "unk880000_r( %06x ) @ %06x = %04x\n", (offset * 2 ) +0x880000, space.device().safe_pc(), m_880000regs[offset] );

	/* this code allows scroll regs to be updated, but tkdensho at least resets perodically */

	switch( offset )
	{
		case 0:
			if ( m_screen->vpos() >= 240) return 0;
			else return 1;

		default:
			return 0;
	}
}

READ16_MEMBER(tecmosys_state::eeprom_r)
{
	return ((m_eeprom->do_read() & 0x01) << 11);
}

WRITE16_MEMBER(tecmosys_state::eeprom_w)
{
	if ( ACCESSING_BITS_8_15 )
	{
		m_eeprom->di_write((data & 0x0800) >> 11);
		m_eeprom->cs_write((data & 0x0200) ? ASSERT_LINE : CLEAR_LINE );
		m_eeprom->clk_write((data & 0x0400) ? CLEAR_LINE: ASSERT_LINE );
	}
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, tecmosys_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM // work ram
	AM_RANGE(0x210000, 0x210001) AM_READNOP // single byte overflow on stack defined as 0x210000
	AM_RANGE(0x300000, 0x300fff) AM_RAM_WRITE(bg0_tilemap_w) AM_SHARE("bg0tilemap_ram") // bg0 ram
	AM_RANGE(0x301000, 0x3013ff) AM_RAM_WRITE(bg0_tilemap_lineram_w) AM_SHARE("bg0_lineram")// bg0 linescroll? (guess)

	AM_RANGE(0x400000, 0x400fff) AM_RAM_WRITE(bg1_tilemap_w) AM_SHARE("bg1tilemap_ram") // bg1 ram
	AM_RANGE(0x401000, 0x4013ff) AM_RAM_WRITE(bg1_tilemap_lineram_w) AM_SHARE("bg1_lineram")// bg1 linescroll? (guess)

	AM_RANGE(0x500000, 0x500fff) AM_RAM_WRITE(bg2_tilemap_w) AM_SHARE("bg2tilemap_ram") // bg2 ram
	AM_RANGE(0x501000, 0x5013ff) AM_RAM_WRITE(bg2_tilemap_lineram_w) AM_SHARE("bg2_lineram") // bg2 linescroll? (guess)

	AM_RANGE(0x700000, 0x703fff) AM_RAM_WRITE(fg_tilemap_w) AM_SHARE("fgtilemap_ram") // fix ram
	AM_RANGE(0x800000, 0x80ffff) AM_RAM AM_SHARE("spriteram") // obj ram
	AM_RANGE(0x880000, 0x88000b) AM_READ(unk880000_r)
	AM_RANGE(0x900000, 0x907fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette") // AM_WRITEONLY // obj pal

	//AM_RANGE(0x980000, 0x9807ff) AM_WRITEONLY // bg pal
	//AM_RANGE(0x980800, 0x980fff) AM_WRITE(paletteram_xGGGGGRRRRRBBBBB_word_w) AM_SHARE("paletteram") // fix pal
	// the two above are as tested by the game code, I've only rolled them into one below to get colours to show right.
	AM_RANGE(0x980000, 0x980fff) AM_RAM_WRITE(tilemap_paletteram16_xGGGGGRRRRRBBBBB_word_w) AM_SHARE("tmap_palette")

	AM_RANGE(0x880000, 0x88002f) AM_WRITE(unk880000_w ) AM_SHARE("880000regs")  // 10 byte dta@88000c, 880022=watchdog?
	AM_RANGE(0xa00000, 0xa00001) AM_WRITE(eeprom_w  )
	AM_RANGE(0xa80000, 0xa80005) AM_WRITEONLY AM_SHARE("a80000regs")    // a80000-3 scroll? a80004 inverted ? 3 : 0
	AM_RANGE(0xb00000, 0xb00005) AM_WRITEONLY AM_SHARE("b00000regs")    // b00000-3 scrool?, b00004 inverted ? 3 : 0
	AM_RANGE(0xb80000, 0xb80001) AM_READWRITE(prot_status_r, prot_status_w)
	AM_RANGE(0xc00000, 0xc00005) AM_WRITEONLY AM_SHARE("c00000regs")    // c00000-3 scroll? c00004 inverted ? 13 : 10
	AM_RANGE(0xc80000, 0xc80005) AM_WRITEONLY AM_SHARE("c80000regs")    // c80000-3 scrool? c80004 inverted ? 3 : 0
	AM_RANGE(0xd00000, 0xd00001) AM_READ_PORT("P1")
	AM_RANGE(0xd00002, 0xd00003) AM_READ_PORT("P2")
	AM_RANGE(0xd80000, 0xd80001) AM_READ(eeprom_r)
	AM_RANGE(0xe00000, 0xe00001) AM_WRITE(sound_w )
	AM_RANGE(0xe80000, 0xe80001) AM_WRITE(prot_data_w)
	AM_RANGE(0xf00000, 0xf00001) AM_READ(sound_r)
	AM_RANGE(0xf80000, 0xf80001) AM_READ(prot_data_r)
ADDRESS_MAP_END


WRITE8_MEMBER(tecmosys_state::z80_bank_w)
{
	membank("bank1")->set_entry(data);
}

WRITE8_MEMBER(tecmosys_state::oki_bank_w)
{
	UINT8 upperbank = (data & 0x30) >> 4;
	UINT8 lowerbank = (data & 0x03) >> 0;
	UINT8* region = memregion("oki")->base();

	memcpy( region+0x00000, region+0x80000 + lowerbank * 0x20000, 0x20000  );
	memcpy( region+0x20000, region+0x80000 + upperbank * 0x20000, 0x20000  );
}

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, tecmosys_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xe000, 0xf7ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, tecmosys_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("ymf", ymf262_device, read, write)
	AM_RANGE(0x10, 0x10) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0x20, 0x20) AM_WRITE(oki_bank_w)
	AM_RANGE(0x30, 0x30) AM_WRITE(z80_bank_w)
	AM_RANGE(0x40, 0x40) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x50, 0x50) AM_WRITE(soundlatch2_byte_w)
	AM_RANGE(0x60, 0x61) AM_DEVREADWRITE("ymz", ymz280b_device, read, write)
ADDRESS_MAP_END



static INPUT_PORTS_START( deroon )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )      PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )    PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )    PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )   PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )          PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )          PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )          PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 )          PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )      PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )   PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )          PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )          PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )          PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 )          PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



static const gfx_layout gfxlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4},
	{ 0*4*8, 1*4*8, 2*4*8, 3*4*8, 4*4*8, 5*4*8, 6*4*8, 7*4*8},
	8*8*4
};

static const gfx_layout gfxlayout2 =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4,
		8*8*4*1+0*4, 8*8*4*1+1*4, 8*8*4*1+2*4, 8*8*4*1+3*4, 8*8*4*1+4*4, 8*8*4*1+5*4,8*8*4*1+6*4, 8*8*4*1+7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		8*8*4*2+0*32, 8*8*4*2+1*32, 8*8*4*2+2*32, 8*8*4*2+3*32, 8*8*4*2+4*32, 8*8*4*2+5*32, 8*8*4*2+6*32, 8*8*4*2+7*32 },
	128*8
};

static GFXDECODE_START( tecmosys )
	GFXDECODE_ENTRY( "gfx2", 0, gfxlayout,   0x4400, 0x40 )
	GFXDECODE_ENTRY( "gfx3", 0, gfxlayout2,  0x4000, 0x40 )
	GFXDECODE_ENTRY( "gfx4", 0, gfxlayout2,  0x4000, 0x40 )
	GFXDECODE_ENTRY( "gfx5", 0, gfxlayout2,  0x4000, 0x40 )

GFXDECODE_END



WRITE_LINE_MEMBER(tecmosys_state::sound_irq)
{
	/* IRQ */
	m_audiocpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}

void tecmosys_state::machine_start()
{
	membank("bank1")->configure_entries(0, 16, memregion("audiocpu")->base(), 0x4000);

	save_item(NAME(m_device_read_ptr));
	save_item(NAME(m_device_status));
	save_item(NAME(m_device_value));
}

static MACHINE_CONFIG_START( deroon, tecmosys_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", tecmosys_state,  irq1_line_hold)
	MCFG_WATCHDOG_VBLANK_INIT(400) // guess

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_16MHz/2 )
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(io_map)


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", tecmosys)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")
	MCFG_EEPROM_SERIAL_ENABLE_STREAMING()

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_AFTER_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(57.4458)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(3000))
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(tecmosys_state, screen_update)

	MCFG_PALETTE_ADD("palette", 0x4000+0x800)
	MCFG_PALETTE_FORMAT(xGGGGGRRRRRBBBBB)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymf", YMF262, XTAL_14_31818MHz)
	MCFG_YMF262_IRQ_HANDLER(WRITELINE(tecmosys_state, sound_irq))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.00)
	MCFG_SOUND_ROUTE(2, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(3, "rspeaker", 1.00)

	MCFG_OKIM6295_ADD("oki", XTAL_16MHz/8, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)

	MCFG_SOUND_ADD("ymz", YMZ280B, XTAL_16_9344MHz)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.30)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.30)
MACHINE_CONFIG_END


ROM_START( deroon )
	ROM_REGION( 0x100000, "maincpu", 0 ) // Main Program
	ROM_LOAD16_BYTE( "t001.upau1", 0x00000, 0x80000, CRC(14b92c18) SHA1(b47b8c828222a3f7c0fe9271899bd38171d972fb) )
	ROM_LOAD16_BYTE( "t002.upal1", 0x00001, 0x80000, CRC(0fb05c68) SHA1(5140592e15414770fb46d5ac9ba8f76e3d4ab323) )

	ROM_REGION( 0x040000, "audiocpu", 0 ) // Sound Program
	ROM_LOAD( "t003.uz1", 0x000000, 0x040000, CRC(8bdfafa0) SHA1(c0cf3eb7a65d967958fe2aace171859b0faf7753) )

	ROM_REGION( 0x2200, "cpu2", 0 ) // MCU is a 68HC11A8 with 8k ROM, 512 bytes EEPROM
	ROM_LOAD( "deroon_68hc11a8.rom",    0x0000, 0x2000, NO_DUMP )
	ROM_LOAD( "deroon_68hc11a8.eeprom", 0x2000, 0x0200, NO_DUMP )

	ROM_REGION( 0x2000000, "gfx1", ROMREGION_ERASE00 ) // Sprites (non-tile based)
	/* all these roms need verifying, they could be half size */

	ROM_LOAD16_BYTE( "t101.uah1", 0x0000000, 0x200000, CRC(74baf845) SHA1(935d2954ba227a894542be492654a2750198e1bc) )
	ROM_LOAD16_BYTE( "t102.ual1", 0x0000001, 0x200000, CRC(1a02c4a3) SHA1(5155eeaef009fc9a9f258e3e54ca2a7f78242df5) )
	/*                            0x8000000, 0x400000 - no rom loaded here, these gfx are 4bpp */
	ROM_LOAD16_BYTE( "t103.ubl1", 0x0800001, 0x400000, CRC(84e7da88) SHA1(b5c3234f33bb945cc9762b91db087153a0589cfb) )
	/*                            0x1000000, 0x400000 - no rom loaded here, these gfx are 4bpp */
	ROM_LOAD16_BYTE( "t104.ucl1", 0x1000001, 0x200000, CRC(66eb611a) SHA1(64435d35677fea3c06fdb03c670f3f63ee481c02) )

	ROM_REGION( 0x100000, "gfx2", 0 ) // 8x8 4bpp tiles
	ROM_LOAD( "t301.ubd1", 0x000000, 0x100000, CRC(8b026177) SHA1(3887856bdaec4d9d3669fe3bc958ef186fbe9adb) )

	ROM_REGION( 0x100000, "gfx3", ROMREGION_ERASE00) // 16x16 4bpp tiles
	/* not used? */

	ROM_REGION( 0x100000, "gfx4", ROMREGION_ERASE00 ) // 16x16 4bpp tiles
	ROM_LOAD( "t201.ubb1", 0x000000, 0x100000, CRC(d5a087ac) SHA1(5098160ce7719d93e3edae05f6edd317d4c61f0d) )

	ROM_REGION( 0x100000, "gfx5", ROMREGION_ERASE00 ) // 16x16 4bpp tiles
	ROM_LOAD( "t202.ubc1", 0x000000, 0x100000, CRC(f051dae1) SHA1(f5677c07fe644b3838657370f0309fb09244c619) )

	ROM_REGION( 0x200000, "ymz", 0 ) // YMZ280B Samples
	ROM_LOAD( "t401.uya1", 0x000000, 0x200000, CRC(92111992) SHA1(ae27e11ae76dec0b9892ad32e1a8bf6ab11f2e6c) )

	ROM_REGION( 0x100000, "oki", 0 ) // M6295 Samples
	ROM_LOAD( "t501.uad1", 0x080000, 0x080000, CRC(2fbcfe27) SHA1(f25c830322423f0959a36955edb563a6150f2142) )
ROM_END

ROM_START( tkdensho )
	ROM_REGION( 0x600000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "aeprge-2.pal", 0x00000, 0x80000, CRC(25e453d6) SHA1(9c84e2af42eff5cc9b14c1759d5bab42fa7bb663) )
	ROM_LOAD16_BYTE( "aeprgo-2.pau", 0x00001, 0x80000, CRC(22d59510) SHA1(5ade482d6ab9a22df2ee8337458c22cfa9045c73) )

	ROM_REGION( 0x040000, "audiocpu", 0 ) // Sound Program
	ROM_LOAD( "aesprg-2.z1", 0x000000, 0x020000, CRC(43550ab6) SHA1(2580129ef8ebd9295249175de4ba985c752e06fe) )
	ROM_RELOAD(              0x020000, 0x020000) // for banks

	ROM_REGION( 0x2200, "cpu2", 0 ) // MCU is a 68HC11A8 with 8k ROM, 512 bytes EEPROM
	ROM_LOAD( "tkdensho_68hc11a8.rom",    0x0000, 0x2000, NO_DUMP )
	ROM_LOAD( "tkdensho_68hc11a8.eeprom", 0x2000, 0x0200, NO_DUMP )

	ROM_REGION( 0x4000000, "gfx1", ROMREGION_ERASE00 ) // Graphics - mostly (maybe all?) not tile based
	ROM_LOAD16_BYTE( "ae100h.ah1",    0x0000000, 0x0400000, CRC(06be252b) SHA1(08d1bb569fd2e66e2c2f47da7780b31945232e62) )
	ROM_LOAD16_BYTE( "ae100.al1",     0x0000001, 0x0400000, CRC(009cdff4) SHA1(fd88f07313d14fd4429b09a1e8d6b595df3b98e5) )
	ROM_LOAD16_BYTE( "ae101h.bh1",    0x0800000, 0x0400000, CRC(f2469eff) SHA1(ba49d15cc7949437ba9f56d9b425a5f0e62137df) )
	ROM_LOAD16_BYTE( "ae101.bl1",     0x0800001, 0x0400000, CRC(db7791bb) SHA1(1fe40b747b7cee7a9200683192b1d60a735a0446) )
	ROM_LOAD16_BYTE( "ae102h.ch1",    0x1000000, 0x0200000, CRC(f9d2a343) SHA1(d141ac0b20be587e77a576ef78f15d269d9c84e5) )
	ROM_LOAD16_BYTE( "ae102.cl1",     0x1000001, 0x0200000, CRC(681be889) SHA1(8044ca7cbb325e6dcadb409f91e0c01b88a1bca7) )
	ROM_LOAD16_BYTE( "ae104.el1",     0x2000001, 0x0400000, CRC(e431b798) SHA1(c2c24d4f395bba8c78a45ecf44009a830551e856) )
	ROM_LOAD16_BYTE( "ae105.fl1",     0x2800001, 0x0400000, CRC(b7f9ebc1) SHA1(987f664072b43a578b39fa6132aaaccc5fe5bfc2) )
	ROM_LOAD16_BYTE( "ae106.gl1",     0x3000001, 0x0200000, CRC(7c50374b) SHA1(40865913125230122072bb13f46fb5fb60c088ea) )

	ROM_REGION( 0x080000, "gfx2", 0 ) // 8x8 4bpp tiles
	ROM_LOAD( "ae300w36.bd1",  0x000000, 0x0080000, CRC(e829f29e) SHA1(e56bfe2669ed1d1ae394c644def426db129d97e3) )

	ROM_REGION( 0x100000, "gfx3", 0 ) // 16x16 4bpp tiles
	ROM_LOAD( "ae200w74.ba1",  0x000000, 0x0100000, CRC(c1645041) SHA1(323670a6aa2a4524eb968cc0b4d688098ffeeb12) )

	ROM_REGION( 0x100000, "gfx4", 0 ) // 16x16 4bpp tiles
	ROM_LOAD( "ae201w75.bb1",  0x000000, 0x0100000, CRC(3f63bdff) SHA1(0d3d57fdc0ec4bceef27c11403b3631d23abadbf) )

	ROM_REGION( 0x100000, "gfx5", 0 ) // 16x16 4bpp tiles
	ROM_LOAD( "ae202w76.bc1",  0x000000, 0x0100000, CRC(5cc857ca) SHA1(2553fb5220433acc15dfb726dc064fe333e51d88) )

	ROM_REGION( 0x400000, "ymz", 0 ) // YMZ280B Samples
	ROM_LOAD( "ae400t23.ya1", 0x000000, 0x200000, CRC(c6ffb043) SHA1(e0c6c5f6b840f63c9a685a2c3be66efa4935cbeb) )
	ROM_LOAD( "ae401t24.yb1", 0x200000, 0x200000, CRC(d83f1a73) SHA1(412b7ac9ff09a984c28b7d195330d78c4aac3dc5) )

	ROM_REGION( 0x100000, "oki", 0 ) // M6295 Samples
	ROM_LOAD( "ae500w07.ad1", 0x080000, 0x080000, CRC(3734f92c) SHA1(048555b5aa89eaf983305c439ba08d32b4a1bb80) )
ROM_END

ROM_START( tkdenshoa )
	ROM_REGION( 0x600000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "aeprge.pal", 0x00000, 0x80000, CRC(17a209ff) SHA1(b5dbea9868cbb89d4e27bf19fdb616ac256985b4) )
	ROM_LOAD16_BYTE( "aeprgo.pau", 0x00001, 0x80000, CRC(d265e6a1) SHA1(f39d8ce115f197a660f5210b2483108854eb12a9) )

	ROM_REGION( 0x040000, "audiocpu", 0 ) // Sound Program
	ROM_LOAD( "aesprg-2.z1", 0x000000, 0x020000, CRC(43550ab6) SHA1(2580129ef8ebd9295249175de4ba985c752e06fe) )
	ROM_RELOAD(              0x020000, 0x020000) // for banks

	ROM_REGION( 0x2200, "cpu2", 0 ) // MCU is a 68HC11A8 with 8k ROM, 512 bytes EEPROM
	ROM_LOAD( "tkdensho_68hc11a8.rom",    0x0000, 0x2000, NO_DUMP )
	ROM_LOAD( "tkdensho_68hc11a8.eeprom", 0x2000, 0x0200, NO_DUMP )

	ROM_REGION( 0x4000000, "gfx1", ROMREGION_ERASE00 ) // Graphics - mostly (maybe all?) not tile based
	ROM_LOAD16_BYTE( "ae100h.ah1",    0x0000000, 0x0400000, CRC(06be252b) SHA1(08d1bb569fd2e66e2c2f47da7780b31945232e62) )
	ROM_LOAD16_BYTE( "ae100.al1",     0x0000001, 0x0400000, CRC(009cdff4) SHA1(fd88f07313d14fd4429b09a1e8d6b595df3b98e5) )
	ROM_LOAD16_BYTE( "ae101h.bh1",    0x0800000, 0x0400000, CRC(f2469eff) SHA1(ba49d15cc7949437ba9f56d9b425a5f0e62137df) )
	ROM_LOAD16_BYTE( "ae101.bl1",     0x0800001, 0x0400000, CRC(db7791bb) SHA1(1fe40b747b7cee7a9200683192b1d60a735a0446) )
	ROM_LOAD16_BYTE( "ae102h.ch1",    0x1000000, 0x0200000, CRC(f9d2a343) SHA1(d141ac0b20be587e77a576ef78f15d269d9c84e5) )
	ROM_LOAD16_BYTE( "ae102.cl1",     0x1000001, 0x0200000, CRC(681be889) SHA1(8044ca7cbb325e6dcadb409f91e0c01b88a1bca7) )
	ROM_LOAD16_BYTE( "ae104.el1",     0x2000001, 0x0400000, CRC(e431b798) SHA1(c2c24d4f395bba8c78a45ecf44009a830551e856) )
	ROM_LOAD16_BYTE( "ae105.fl1",     0x2800001, 0x0400000, CRC(b7f9ebc1) SHA1(987f664072b43a578b39fa6132aaaccc5fe5bfc2) )
	ROM_LOAD16_BYTE( "ae106.gl1",     0x3000001, 0x0200000, CRC(7c50374b) SHA1(40865913125230122072bb13f46fb5fb60c088ea) )

	ROM_REGION( 0x080000, "gfx2", 0 ) // 8x8 4bpp tiles
	ROM_LOAD( "ae300w36.bd1",  0x000000, 0x0080000, CRC(e829f29e) SHA1(e56bfe2669ed1d1ae394c644def426db129d97e3) )

	ROM_REGION( 0x100000, "gfx3", 0 ) // 16x16 4bpp tiles
	ROM_LOAD( "ae200w74.ba1",  0x000000, 0x0100000, CRC(c1645041) SHA1(323670a6aa2a4524eb968cc0b4d688098ffeeb12) )

	ROM_REGION( 0x100000, "gfx4", 0 ) // 16x16 4bpp tiles
	ROM_LOAD( "ae201w75.bb1",  0x000000, 0x0100000, CRC(3f63bdff) SHA1(0d3d57fdc0ec4bceef27c11403b3631d23abadbf) )

	ROM_REGION( 0x100000, "gfx5", 0 ) // 16x16 4bpp tiles
	ROM_LOAD( "ae202w76.bc1",  0x000000, 0x0100000, CRC(5cc857ca) SHA1(2553fb5220433acc15dfb726dc064fe333e51d88) )

	ROM_REGION( 0x400000, "ymz", 0 ) // YMZ280B Samples
	ROM_LOAD( "ae400t23.ya1", 0x000000, 0x200000, CRC(c6ffb043) SHA1(e0c6c5f6b840f63c9a685a2c3be66efa4935cbeb) )
	ROM_LOAD( "ae401t24.yb1", 0x200000, 0x200000, CRC(d83f1a73) SHA1(412b7ac9ff09a984c28b7d195330d78c4aac3dc5) )

	ROM_REGION( 0x100000, "oki", 0 ) // M6295 Samples
	ROM_LOAD( "ae500w07.ad1", 0x080000, 0x080000, CRC(3734f92c) SHA1(048555b5aa89eaf983305c439ba08d32b4a1bb80) )
ROM_END

void tecmosys_state::descramble()
{
	UINT8 *gfxsrc  = memregion( "gfx1" )->base();
	size_t srcsize = memregion( "gfx1" )->bytes();
	int i;

	for (i=0; i < srcsize; i+=4)
	{
		UINT8 tmp[4];

		tmp[2] = ((gfxsrc[i+0]&0xf0)>>0) | ((gfxsrc[i+1]&0xf0)>>4); //  0, 1, 2, 3   8, 9,10,11
		tmp[3] = ((gfxsrc[i+0]&0x0f)<<4) | ((gfxsrc[i+1]&0x0f)<<0); //  4, 5, 6, 7, 12,13,14,15
		tmp[0] = ((gfxsrc[i+2]&0xf0)>>0) | ((gfxsrc[i+3]&0xf0)>>4); // 16,17,18,19, 24,25,26,27
		tmp[1] = ((gfxsrc[i+2]&0x0f)<<4) | ((gfxsrc[i+3]&0x0f)>>0); // 20,21,22,23, 28,29,30,31

		gfxsrc[i+0] = tmp[0];
		gfxsrc[i+1] = tmp[1];
		gfxsrc[i+2] = tmp[2];
		gfxsrc[i+3] = tmp[3];
	}
}

DRIVER_INIT_MEMBER(tecmosys_state,deroon)
{
	descramble();
	prot_init(0); // machine/tecmosys.c
}

DRIVER_INIT_MEMBER(tecmosys_state,tkdensho)
{
	descramble();
	prot_init(1);
}

DRIVER_INIT_MEMBER(tecmosys_state,tkdensha)
{
	descramble();
	prot_init(2);
}

GAME( 1995, deroon,           0, deroon, deroon, tecmosys_state, deroon,     ROT0, "Tecmo", "Deroon DeroDero", MACHINE_SUPPORTS_SAVE )
GAME( 1996, tkdensho,         0, deroon, deroon, tecmosys_state, tkdensho,   ROT0, "Tecmo", "Toukidenshou - Angel Eyes (VER. 960614)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, tkdenshoa, tkdensho, deroon, deroon, tecmosys_state, tkdensha,   ROT0, "Tecmo", "Toukidenshou - Angel Eyes (VER. 960427)", MACHINE_SUPPORTS_SAVE )

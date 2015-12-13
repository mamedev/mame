// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                            -= Jaleco Mega System 1 =-

                    driver by   Luca Elia (l.elia@tin.it)


To enter service mode in some games press service1+F3.


Year + Game                       System    Protection
---------------------------------------------------------------------
88  Legend of Makai (World) /       Z
    Makai Densetsu  (Japan)         Z
    P-47  (World) /                 A
    P-47  (Japan) /                 A
    P-47  (Japan, Export)           A
    Kick Off (Japan)                A
    Takeda Shingen (Japan)          A             Encryption (key 1)
    Ninja Kazan (World) /           A       Yes + Encryption (key 1)
    Iga Ninjyutsuden (Japan)        A       Yes + Encryption (key 1)
89  Astyanax          (World) /     A       Yes + Encryption (key 2)
    The Lord of King  (Japan)       A       Yes + Encryption (key 2)
    Hachoo!                         A       Yes + Encryption (key 2)
    Jitsuryoku!! Pro Yakyuu (Japan) A       Yes + Encryption (key 2)
    Plus Alpha                      A       Yes + Encryption (key 2)
    Saint Dragon                    A       Yes + Encryption (key 1)
90  RodLand  (World) /              A             Encryption (key 3)
    RodLand  (Japan)                A             Encryption (key 2)
    R&T (Prototype?)                A             Encryption (key 2)
    Phantasm        (Japan) /       A             Encryption (key 1)
91  Avenging Spirit (World)         B       Inputs
    Earth Defense Force             B       Inputs
    64th Street  (World) /          C       Inputs
    64th Street  (Japan)            C       Inputs
92  Soldam (Japan)                  A             Encryption (key 2)
    Big Striker                     C       Inputs
93  Chimera Beast                   C       Inputs
    Cybattler                       C       Inputs
    Hayaoshi Quiz Ouza Ketteisen    B       Inputs
    Peek-a-Boo!                     D       Inputs
---------------------------------------------------------------------

NOTE: Chimera Beast is the only game missing a dump of its priority PROM


Hardware    Main CPU    Sound CPU   Sound Chips
-----------------------------------------------------------
MS1 - Z     68000       Z80         YM2203c
MS1 - A     68000       68000       YM2151      2xOKI-M6295
MS1 - B     68000       68000       YM2151      2xOKI-M6295
MS1 - C     68000       68000       YM2151      2xOKI-M6295
MS1 - D     68000       -           -             OKI-M6295
-----------------------------------------------------------



Main CPU    RW      MS1-A/Z         MS1-B           MS1-C           MS1-D
-----------------------------------------------------------------------------------
ROM         R   000000-03ffff   000000-03ffff   000000-07ffff   000000-03ffff
                                080000-0bffff
Video Regs   W  084000-0843ff   044000-0443ff   0c0000-0cffff   0c0000-0cffff
Palette     RW  088000-0887ff   048000-0487ff   0f8000-0f87ff   0d8000-0d87ff
Object RAM  RW  08e000-08ffff   04e000-04ffff   0d2000-0d3fff   0ca000-0cbfff
Scroll 0    RW  090000-093fff   050000-053fff   0e0000-0e3fff   0d0000-0d3fff
Scroll 1    RW  094000-097fff   054000-057fff   0e8000-0ebfff   0e8000-0ebfff
Scroll 2    RW  098000-09bfff   058000-05bfff   0f0000-0f3fff   -
Work RAM    RW  0f0000-0fffff*  060000-07ffff*  1f0000-1fffff*  1f0000-1fffff
Input Ports R   080000-080009   0e0000-0e0001** 0d8000-d80001** 100000-100001**
-----------------------------------------------------------------------------------
*  Some games use mirror addresses
** Through protection.



Sound CPU       RW      MS1-A           MS1-B           MS1-C           MS1-D
-----------------------------------------------------------------------------------
ROM         R       000000-01ffff       000000-01ffff       000000-01ffff       No Sound CPU
Latch #1        R       040000-040001       <           060000-060001
Latch #2         W      060000-060001       <           <
2151 reg         W      080000-080001       <           <
2151 data        W      080002-080003       <           <
2151 status     R       080002-080003       <           <
6295 #1 data         W      0a0000-0a0003       <           <
6295 #1 status      R       0a0000-0a0001       <           <
6295 #2 data         W      0c0000-0c0003       <           <
6295 #2 status      R       0c0000-0c0001       <           <
RAM         RW      0f0000-0f3fff       0e0000-0effff?      <
-----------------------------------------------------------------------------------


                                Issues / To Do
                                --------------

- There is a 512 byte PROM in the video section (differs by game) that
  controls the priorities. This prom is currently missing for one game,
  so we have to use fake data for it (Chimera Beast).

- Making the M6295 status register return 0 fixes the music tempo in
  avspirit, 64street, astyanax etc. but makes most of the effects in
  hachoo disappear! Define SOUND_HACK to 0 to turn this hack off
  This seems to be some Jaleco magic at work (strange protection?). The
  bootleg version of rodlandj has one instruction patched out to do exactly
  the same thing that we are doing (ignoring the 6295 status).

- Understand properly how irqs truly works, kazan / iganinju is (again) broken.

- 64street: player characters in attract mode doesn't move at all, protection?
  they move on the real PCB

- tshingen: unemulated mosaic effect when killing enemies with the flashing sword.
  See https://youtu.be/m4ZH0v8UqWs
  The effect can be tested in e.g. stdragon and p47 test mode:
  See https://youtu.be/zo3FTCqkNBc and https://youtu.be/dEqH017YBzw

- Understand a handful of unknown bits in video regs

- R&T really does have scrambled sound effects on the PCB, those two ROMs being
  ones which even still had their original labels.  Possibly a prototype, only
  one known to exist.  ROM17 is missing on the board, not sure if this is
  intentional, is the data for the 'secondary' set of levels stored in 17/18?
  This game has no alternate levels mode either.  Socketed encryption chip is
  unusual.

***************************************************************************/

#define SYS_A_CPU_CLOCK     (XTAL_12MHz / 2)    /* clock for main 68000 */
#define SYS_B_CPU_CLOCK     XTAL_8MHz       /* clock for main 68000 */
#define SYS_C_CPU_CLOCK     (XTAL_24MHz / 2)    /* clock for main 68000 */
#define SYS_D_CPU_CLOCK     XTAL_8MHz       /* clock for main 68000 */
#define SOUND_CPU_CLOCK     XTAL_7MHz       /* clock for sound 68000 */
#define OKI4_SOUND_CLOCK    XTAL_4MHz

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/2203intf.h"
#include "sound/2151intf.h"
#include "machine/jalcrpt.h"
#include "includes/megasys1.h"


MACHINE_RESET_MEMBER(megasys1_state,megasys1)
{
	m_ignore_oki_status = 1;    /* ignore oki status due 'protection' */
	m_ip_select = 0;    /* reset protection */
	m_mcu_hs = 0;
}

MACHINE_RESET_MEMBER(megasys1_state,megasys1_hachoo)
{
	m_ignore_oki_status = 0;    /* strangely hachoo need real oki status */
	m_ip_select = 0;    /* reset protection */
	m_mcu_hs = 0;
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

/***************************************************************************
                        [ Main CPU - System A / Z ]
***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(megasys1_state::megasys1A_scanline)
{
	int scanline = param;

	// irq 1 is vblank (confirmed by Saint Dragon "press start" behaviour), 2 & 3 unknown
	
	if(scanline == 240) // vblank-out irq
		m_maincpu->set_input_line(1, HOLD_LINE);

	if(scanline == 0)
		m_maincpu->set_input_line(2, HOLD_LINE);

	// RTE in stdragon
	if(scanline == 128)
		m_maincpu->set_input_line(3, HOLD_LINE);
}

static ADDRESS_MAP_START( megasys1A_map, AS_PROGRAM, 16, megasys1_state )
	ADDRESS_MAP_GLOBAL_MASK(0xfffff)
	AM_RANGE(0x000000, 0x05ffff) AM_ROM
	AM_RANGE(0x080000, 0x080001) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x080002, 0x080003) AM_READ_PORT("P1")
	AM_RANGE(0x080004, 0x080005) AM_READ_PORT("P2")
	AM_RANGE(0x080006, 0x080007) AM_READ_PORT("DSW")
	AM_RANGE(0x080008, 0x080009) AM_READ(soundlatch2_word_r)    /* from sound cpu */
	AM_RANGE(0x084000, 0x0843ff) AM_RAM_WRITE(megasys1_vregs_A_w) AM_SHARE("vregs")
	AM_RANGE(0x088000, 0x0887ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x08e000, 0x08ffff) AM_RAM AM_SHARE("objectram")
	AM_RANGE(0x090000, 0x093fff) AM_RAM_WRITE(megasys1_scrollram_0_w) AM_SHARE("scrollram.0")
	AM_RANGE(0x094000, 0x097fff) AM_RAM_WRITE(megasys1_scrollram_1_w) AM_SHARE("scrollram.1")
	AM_RANGE(0x098000, 0x09bfff) AM_RAM_WRITE(megasys1_scrollram_2_w) AM_SHARE("scrollram.2")
	AM_RANGE(0x0f0000, 0x0fffff) AM_RAM_WRITE(ms1_ram_w) AM_SHARE("ram")
ADDRESS_MAP_END


/***************************************************************************
                            [ Main CPU - System B ]
***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(megasys1_state::megasys1B_scanline)
{
	int scanline = param;

	if(scanline == 240) // vblank-out irq
		m_maincpu->set_input_line(4, HOLD_LINE);

	if(scanline == 0)
		m_maincpu->set_input_line(2, HOLD_LINE);

	if(scanline == 128)
		m_maincpu->set_input_line(1, HOLD_LINE);
}


/*           Read the input ports, through a protection device:

 ip_select_values must contain the 5 codes sent to the protection device
 in order to obtain the status of the following 5 input ports:

         Coins  Player1     Player2     DSW1        DSW2

 in that order.         */

READ16_MEMBER(megasys1_state::ip_select_r)
{
	int i;

//  Coins   P1      P2      DSW1    DSW2
//  57      53      54      55      56      < 64street
//  37      35      36      33      34      < avspirit
//  58      54      55      56      57      < bigstrik
//  56      52      53      54      55      < cybattlr
//  20      21      22      23      24      < edf
//  51      52      53      54      55      < hayaosi1


	/* f(x) = ((x*x)>>4)&0xFF ; f(f($D)) == 6 */
	if ((m_ip_select & 0xF0) == 0xF0) return 0x000D;

	for (i = 0; i < 5; i++) if (m_ip_select == m_ip_select_values[i]) break;

	switch (i)
	{
			case 0 :    return m_io_system->read();
			case 1 :    return m_io_p1->read();
			case 2 :    return m_io_p2->read();
			case 3 :    return m_io_dsw1->read();
			case 4 :    return m_io_dsw2->read();
			default  :  return 0x0006;
	}
}

WRITE16_MEMBER(megasys1_state::ip_select_w)
{
	COMBINE_DATA(&m_ip_select);
	m_maincpu->set_input_line(2, HOLD_LINE);
}


static ADDRESS_MAP_START( megasys1B_map, AS_PROGRAM, 16, megasys1_state )
	ADDRESS_MAP_GLOBAL_MASK(0xfffff)
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x044000, 0x0443ff) AM_RAM_WRITE(megasys1_vregs_A_w) AM_SHARE("vregs")
	AM_RANGE(0x048000, 0x0487ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x04e000, 0x04ffff) AM_RAM AM_SHARE("objectram")
	AM_RANGE(0x050000, 0x053fff) AM_RAM_WRITE(megasys1_scrollram_0_w) AM_SHARE("scrollram.0")
	AM_RANGE(0x054000, 0x057fff) AM_RAM_WRITE(megasys1_scrollram_1_w) AM_SHARE("scrollram.1")
	AM_RANGE(0x058000, 0x05bfff) AM_RAM_WRITE(megasys1_scrollram_2_w) AM_SHARE("scrollram.2")
	AM_RANGE(0x060000, 0x07ffff) AM_RAM_WRITE(ms1_ram_w) AM_SHARE("ram")
	AM_RANGE(0x080000, 0x0bffff) AM_ROM
	AM_RANGE(0x0e0000, 0x0e0001) AM_READWRITE(ip_select_r,ip_select_w)
ADDRESS_MAP_END


/***************************************************************************
                            [ Main CPU - System C ]
***************************************************************************/


#define INTERRUPT_NUM_C INTERRUPT_NUM_B
#define interrupt_C     interrupt_B

WRITE16_MEMBER(megasys1_state::ms1_ram_w )
{
	// DON'T use COMBINE_DATA
	// byte writes end up mirroring in both bytes of the word like nmk16.c
	// 64th Street and Chimera Beast rely on this for attract inputs

	m_ram[offset] = data;
//	if (mem_mask != 0xffff) printf("byte write to RAM %04x %04x %04x\n", offset, data, mem_mask);

}

static ADDRESS_MAP_START( megasys1C_map, AS_PROGRAM, 16, megasys1_state )
	ADDRESS_MAP_GLOBAL_MASK(0x1fffff)
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x0c0000, 0x0cffff) AM_READWRITE(megasys1_vregs_C_r,megasys1_vregs_C_w) AM_SHARE("vregs")
	AM_RANGE(0x0d2000, 0x0d3fff) AM_RAM AM_SHARE("objectram")
	AM_RANGE(0x0d8000, 0x0d8001) AM_READWRITE(ip_select_r,ip_select_w)
	// 64th Street actively uses 0xe4*** for breakable objects.
	AM_RANGE(0x0e0000, 0x0e3fff) AM_MIRROR(0x4000) AM_RAM_WRITE(megasys1_scrollram_0_w) AM_SHARE("scrollram.0")
	AM_RANGE(0x0e8000, 0x0ebfff) AM_MIRROR(0x4000) AM_RAM_WRITE(megasys1_scrollram_1_w) AM_SHARE("scrollram.1")
	AM_RANGE(0x0f0000, 0x0f3fff) AM_MIRROR(0x4000) AM_RAM_WRITE(megasys1_scrollram_2_w) AM_SHARE("scrollram.2")
	AM_RANGE(0x0f8000, 0x0f87ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x1c0000, 0x1cffff) AM_MIRROR(0x30000) AM_RAM_WRITE(ms1_ram_w) AM_SHARE("ram") //0x1f****, Cybattler reads attract mode inputs at 0x1d****
ADDRESS_MAP_END


/***************************************************************************
                            [ Main CPU - System D ]
***************************************************************************/

INTERRUPT_GEN_MEMBER(megasys1_state::megasys1D_irq)
{
	device.execute().set_input_line(2, HOLD_LINE);
}

static ADDRESS_MAP_START( megasys1D_map, AS_PROGRAM, 16, megasys1_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x0c0000, 0x0c9fff) AM_RAM_WRITE(megasys1_vregs_D_w) AM_SHARE("vregs")
	AM_RANGE(0x0ca000, 0x0cbfff) AM_RAM AM_SHARE("objectram")
	AM_RANGE(0x0d0000, 0x0d3fff) AM_RAM_WRITE(megasys1_scrollram_1_w) AM_SHARE("scrollram.1")
	AM_RANGE(0x0d4000, 0x0d7fff) AM_RAM_WRITE(megasys1_scrollram_2_w) AM_SHARE("scrollram.2")
	AM_RANGE(0x0d8000, 0x0d87ff) AM_MIRROR(0x3000) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x0e0000, 0x0e0001) AM_READ_PORT("DSW")
	AM_RANGE(0x0e8000, 0x0ebfff) AM_RAM_WRITE(megasys1_scrollram_0_w) AM_SHARE("scrollram.0")
	AM_RANGE(0x0f0000, 0x0f0001) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x0f8000, 0x0f8001) AM_DEVREADWRITE8("oki1", okim6295_device, read, write, 0x00ff)
//  AM_RANGE(0x100000, 0x100001) // protection
	AM_RANGE(0x1f0000, 0x1fffff) AM_RAM /*_WRITE(ms1_ram_w)*/ AM_SHARE("ram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( megasys1D_oki_map, AS_0, 8, megasys1_state )
	AM_RANGE(0x00000, 0x1ffff) AM_ROM
	AM_RANGE(0x20000, 0x3ffff) AM_ROMBANK("okibank")
ADDRESS_MAP_END

/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

/*
                            [ Sound CPU interrupts ]

    [MS1-A]
        astyanax                all rte
        hachoo                  all reset the program, but the status
                                register is set to 2700
        iganinju                all rte
        p47 & p47j              all rte
        phantasm                all rte (4 is different, but rte)
        plusalph                all rte
        rodland & rodlandj      all rte (4 is different, but rte)
        stdragon                4]read & store sound command and echo to main cpu
                                rest: rte
    [MS1-B]
        avspirit                all rte (4 is different, but rte)
        edf                     all rte (4 is different, but rte)

    [MS1-C]
        64street                all rte (4 is different, but rte)
        chimerab                all rte
        cybattlr
            1;3;5-7]400 busy loop
            2]40c   read & store sound command and echo to main cpu
            4]446   rte


 These games almost always don't use the interrupts to drive the music
 tempo (cybattlr and stdragon do!) but use the YM2151 timers instead
 (they poll the status register). Since those timers are affected by
 the YM2151 clock, it's this latter that ultimately decides the music
 tempo.

 Note that some games' music is severely slowed down and out of sync
 (avspirit, 64street) by the fact that the game waits for some samples
 to be played entirely (M6295 status register polled) but they take
 too much time (and raising the M6295 clock rate would, on the other
 hand, screw the pitch of the samples)

 A temporary fix is to make the status of this chip return 0...
 unfortunately, this trick makes most of the effects disappear in at
 least one game: hachoo!

 IRQ 4 comes from the YM2151.  This is confirmed by jitsupro, which
 runs at a much slower timer rate than the other games and formerly
 required it's own machine driver to get interrupts at around the
 right speed.  Now with the 2151 driving all games have the proper
 tempo with no hacks.

*/

/* YM2151 IRQ */
WRITE_LINE_MEMBER(megasys1_state::sound_irq)
{
	if (state)
		m_audiocpu->set_input_line(4, HOLD_LINE);
}

READ8_MEMBER(megasys1_state::oki_status_1_r)
{
	if (m_ignore_oki_status == 1)
		return 0;
	else
		return m_oki1->read_status();
}

READ8_MEMBER(megasys1_state::oki_status_2_r)
{
	if (m_ignore_oki_status == 1)
		return 0;
	else
		return m_oki2->read_status();
}
/***************************************************************************
                            [ Sound CPU - System A ]
***************************************************************************/


static ADDRESS_MAP_START( megasys1A_sound_map, AS_PROGRAM, 16, megasys1_state )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0x040000, 0x040001) AM_READ(soundlatch_word_r)
	AM_RANGE(0x060000, 0x060001) AM_WRITE(soundlatch2_word_w)   // to main cpu
	AM_RANGE(0x080000, 0x080003) AM_DEVREADWRITE8("ymsnd", ym2151_device, read, write, 0x00ff)
	AM_RANGE(0x0a0000, 0x0a0001) AM_READ8(oki_status_1_r, 0x00ff)
	AM_RANGE(0x0a0000, 0x0a0003) AM_DEVWRITE8("oki1", okim6295_device, write, 0x00ff)
	AM_RANGE(0x0c0000, 0x0c0001) AM_READ8(oki_status_2_r, 0x00ff)
	AM_RANGE(0x0c0000, 0x0c0003) AM_DEVWRITE8("oki2", okim6295_device, write, 0x00ff)
	AM_RANGE(0x0e0000, 0x0fffff) AM_RAM
ADDRESS_MAP_END


/***************************************************************************
                        [ Sound CPU - System B / C ]
***************************************************************************/


static ADDRESS_MAP_START( megasys1B_sound_map, AS_PROGRAM, 16, megasys1_state )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0x040000, 0x040001) AM_READWRITE(soundlatch_word_r,soundlatch2_word_w) /* from/to main cpu */
	AM_RANGE(0x060000, 0x060001) AM_READWRITE(soundlatch_word_r,soundlatch2_word_w) /* from/to main cpu */
	AM_RANGE(0x080000, 0x080003) AM_DEVREADWRITE8("ymsnd", ym2151_device, read, write, 0x00ff)
	AM_RANGE(0x0a0000, 0x0a0001) AM_READ8(oki_status_1_r, 0x00ff)
	AM_RANGE(0x0a0000, 0x0a0003) AM_DEVWRITE8("oki1", okim6295_device, write, 0x00ff)
	AM_RANGE(0x0c0000, 0x0c0001) AM_READ8(oki_status_2_r, 0x00ff)
	AM_RANGE(0x0c0000, 0x0c0003) AM_DEVWRITE8("oki2", okim6295_device, write, 0x00ff)
	AM_RANGE(0x0e0000, 0x0effff) AM_RAM
ADDRESS_MAP_END


/***************************************************************************
                        [ Sound CPU - System Z ]
***************************************************************************/



static ADDRESS_MAP_START( z80_sound_map, AS_PROGRAM, 8, megasys1_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xe000, 0xe000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xf000, 0xf000) AM_WRITENOP /* ?? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( z80_sound_io_map, AS_IO, 8, megasys1_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE("ymsnd", ym2203_device, read, write)
ADDRESS_MAP_END


/*************************************
 *
 *  Generic port definitions
 *
 *************************************/

static INPUT_PORTS_START( megasys1_generic )
	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* Reserve 1P */
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* Reserve 2P */
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/* Coinage DSWs */
//  1]  01-41 02-31 03-21 07-11 06-12 05-13 04-14 00-FC * 2
//  2]  04-31 02-21 07-11 03-12 05-13 01-14 06-15 00-FC
//      00-41 20-31 10-21 38-11 18-12 28-13 08-14 30-15

#define COINAGE_8BITS \
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coin_A ) )\
	PORT_DIPSETTING(      0x0007, DEF_STR( 4C_1C ) )\
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )\
	PORT_DIPSETTING(      0x0009, DEF_STR( 2C_1C ) )\
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_1C ) )\
/*  PORT_DIPSETTING(      0x0005, DEF_STR( 1C_1C ) )*/\
/*  PORT_DIPSETTING(      0x0004, DEF_STR( 1C_1C ) )*/\
/*  PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) )*/\
/*  PORT_DIPSETTING(      0x0002, DEF_STR( 1C_1C ) )*/\
/*  PORT_DIPSETTING(      0x0001, DEF_STR( 1C_1C ) )*/\
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_3C ) )\
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_2C ) )\
	PORT_DIPSETTING(      0x000d, DEF_STR( 1C_3C ) )\
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_4C ) )\
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_5C ) )\
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_6C ) )\
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )\
	PORT_DIPNAME( 0x00f0, 0x00f0, DEF_STR( Coin_B ) )\
	PORT_DIPSETTING(      0x0070, DEF_STR( 4C_1C ) )\
	PORT_DIPSETTING(      0x0080, DEF_STR( 3C_1C ) )\
	PORT_DIPSETTING(      0x0090, DEF_STR( 2C_1C ) )\
	PORT_DIPSETTING(      0x00f0, DEF_STR( 1C_1C ) )\
/*  PORT_DIPSETTING(      0x0050, DEF_STR( 1C_1C ) )*/\
/*  PORT_DIPSETTING(      0x0040, DEF_STR( 1C_1C ) )*/\
/*  PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )*/\
/*  PORT_DIPSETTING(      0x0020, DEF_STR( 1C_1C ) )*/\
/*  PORT_DIPSETTING(      0x0010, DEF_STR( 1C_1C ) )*/\
	PORT_DIPSETTING(      0x0060, DEF_STR( 2C_3C ) )\
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_2C ) )\
	PORT_DIPSETTING(      0x00d0, DEF_STR( 1C_3C ) )\
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_4C ) )\
	PORT_DIPSETTING(      0x00b0, DEF_STR( 1C_5C ) )\
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_6C ) )\
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )

/*************************************
 *
 *  Game-specific port definitions
 *
 *************************************/

static INPUT_PORTS_START( 64street )
	PORT_INCLUDE( megasys1_generic )

	PORT_START("DSW1")
	COINAGE_8BITS

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0060, 0x0020, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0040, "1" )
	PORT_DIPSETTING(      0x0060, "2" )
	PORT_DIPSETTING(      0x0020, "3" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_SERVICE( 0x0080, IP_ACTIVE_LOW )
INPUT_PORTS_END

static INPUT_PORTS_START( astyanax )
	PORT_INCLUDE( megasys1_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)

	PORT_MODIFY("P2")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("DSW")
	PORT_DIPUNUSED( 0x0001, 0x0001 ) // according to manual
	PORT_DIPUNUSED( 0x0002, 0x0002 ) // according to manual
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(      0x0004, "30k 70k 110k then every 30k" )
	PORT_DIPSETTING(      0x0000, "50k 100k then every 40k" )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0008, "2" )
	PORT_DIPSETTING(      0x0018, "3" )
	PORT_DIPSETTING(      0x0010, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Swap 1P/2P Controls" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(      0x0300, DEF_STR( 1C_1C ) )    // 1_2 shown in test mode
//  PORT_DIPSETTING(      0x0500, DEF_STR( 1C_1C ) )    // 1_3
//  PORT_DIPSETTING(      0x0100, DEF_STR( 1C_1C ) )    // 1_4
//  PORT_DIPSETTING(      0x0600, DEF_STR( 1C_1C ) )    // 1_5
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(      0x1800, DEF_STR( 1C_1C ) )    // 1_2 shown in test mode
//  PORT_DIPSETTING(      0x2800, DEF_STR( 1C_1C ) )    // 1_3
//  PORT_DIPSETTING(      0x0800, DEF_STR( 1C_1C ) )    // 1_4
//  PORT_DIPSETTING(      0x3000, DEF_STR( 1C_1C ) )    // 1_5
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Test Mode" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( avspirit )
	PORT_INCLUDE( 64street )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Test Mode" ) // freeze & slow motion via P1 + P2 start
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( phantasm )
	PORT_INCLUDE( megasys1_generic )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Test Mode" ) // freeze & slow motion via P1 + P2 start
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0080, IP_ACTIVE_LOW )

	PORT_DIPNAME( 0x0f00, 0x0f00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0900, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0f00, DEF_STR( 1C_1C ) )
/*  PORT_DIPSETTING(      0x0500, DEF_STR( 1C_1C ) )*/
/*  PORT_DIPSETTING(      0x0400, DEF_STR( 1C_1C ) )*/
/*  PORT_DIPSETTING(      0x0300, DEF_STR( 1C_1C ) )*/
/*  PORT_DIPSETTING(      0x0200, DEF_STR( 1C_1C ) )*/
/*  PORT_DIPSETTING(      0x0100, DEF_STR( 1C_1C ) )*/
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0d00, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0b00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf000, 0xf000, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x7000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x9000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0xf000, DEF_STR( 1C_1C ) )
/*  PORT_DIPSETTING(      0x5000, DEF_STR( 1C_1C ) )*/
/*  PORT_DIPSETTING(      0x4000, DEF_STR( 1C_1C ) )*/
/*  PORT_DIPSETTING(      0x3000, DEF_STR( 1C_1C ) )*/
/*  PORT_DIPSETTING(      0x2000, DEF_STR( 1C_1C ) )*/
/*  PORT_DIPSETTING(      0x1000, DEF_STR( 1C_1C ) )*/
	PORT_DIPSETTING(      0x6000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0xd000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0xb000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
INPUT_PORTS_END

static INPUT_PORTS_START( bigstrik )
	PORT_INCLUDE( 64street )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)

	PORT_MODIFY("P2")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x0006, 0x0006, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0018, 0x0018, "Time" )
	PORT_DIPSETTING(      0x0000, "Very Short" )
	PORT_DIPSETTING(      0x0010, "Short" )
	PORT_DIPSETTING(      0x0018, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0008, "Long" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "1 Credit 2 Play" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( chimerab )
	PORT_INCLUDE( megasys1_generic )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0060, 0x0020, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0040, "1" )
	PORT_DIPSETTING(      0x0060, "2" )
	PORT_DIPSETTING(      0x0020, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_SERVICE( 0x0080, IP_ACTIVE_LOW )

	PORT_START("DSW2")
	COINAGE_8BITS
INPUT_PORTS_END

static INPUT_PORTS_START( cybattlr )
	PORT_INCLUDE( megasys1_generic )

	PORT_START("DSW1")      /*1fd2d9.b, !1fd009.b */
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0080, IP_ACTIVE_LOW )

	PORT_START("DSW2")      /* 1fd2d7.b, !1fd007.b */
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Disable Instruction Screen" ) /* Manual states this dip is "Unused" */
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0018, 0x00018, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, "Up to Mission 4" )
	PORT_DIPSETTING(      0x0008, "Not on Final Mission" )
	PORT_DIPSETTING(      0x0018, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Intro Music" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( edf )
	PORT_INCLUDE( megasys1_generic )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x000040, 0x000040, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x000000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x000040, DEF_STR( On ) )
	PORT_SERVICE( 0x000080, IP_ACTIVE_LOW )

	PORT_START("DSW2")          /* 0x66007.b */
	PORT_DIPNAME( 0x0007, 0x0007, "DSW-B bits 2-0" )
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x0001, "1" )
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0004, "4" )
	PORT_DIPSETTING(      0x0005, "5" )
	PORT_DIPSETTING(      0x0006, "6" )
	PORT_DIPSETTING(      0x0007, "7" )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0008, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( hachoo )
	PORT_INCLUDE( megasys1_generic )

	PORT_START("DSW")
	PORT_DIPUNUSED( 0x0001, 0x0001 ) /* Manual states dips 1-4 & 6 are "Unused" */
	PORT_DIPUNUSED( 0x0002, 0x0002 )
	PORT_DIPUNUSED( 0x0004, 0x0004 )
	PORT_DIPUNUSED( 0x0008, 0x0008 )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Harder ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPUNUSED( 0x0040, 0x0040 )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x8000, 0x8000 )
INPUT_PORTS_END

static INPUT_PORTS_START( kazan )
	PORT_INCLUDE( hachoo )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0003, "2" )
	PORT_DIPSETTING(      0x0001, "3" )
	PORT_DIPSETTING(      0x0002, "4" )
	PORT_DIPSETTING(      0x0000, "Infinite (Cheat)" )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(      0x0004, "50k" )
	PORT_DIPSETTING(      0x0000, "200k" )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Cocktail ) )

	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Freeze Screen (Cheat)")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( jitsupro )
	PORT_INCLUDE( hachoo )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)

	PORT_MODIFY("P2")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, "2 Player Innings per Credit" )
	PORT_DIPSETTING(      0x0001, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x0002, 0x0002, "Difficulty for Catching Ball" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Normal ) )   // $200-140
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) ) // $400-140
	PORT_DIPNAME( 0x003c, 0x003c, "Franchise (Increase Batting Power)" )
	PORT_DIPSETTING(      0x003c, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0038, "G"  )
	PORT_DIPSETTING(      0x0034, "D"  )
	PORT_DIPSETTING(      0x0030, "C"  ) /* I hope the Japanese Baseball crowd knows what these leagues are ;-) */
	PORT_DIPSETTING(      0x002c, "S"  )
	PORT_DIPSETTING(      0x0028, "W (B)"  )
	PORT_DIPSETTING(      0x0024, "T"  )
	PORT_DIPSETTING(      0x0020, "L"  )
	PORT_DIPSETTING(      0x001c, "Br (Bw)"  )
	PORT_DIPSETTING(      0x0018, "F"  )
	PORT_DIPSETTING(      0x0014, "H" )
	PORT_DIPSETTING(      0x0010, "O (M)" )
	PORT_DIPSETTING(      0x000c, "Bu" )
//  PORT_DIPSETTING(      0x0008, "13" )
//  PORT_DIPSETTING(      0x0004, "14" )
//  PORT_DIPSETTING(      0x0000, "15" )
	PORT_DIPNAME( 0x0040, 0x0040, "Scroll Is Based On" )
	PORT_DIPSETTING(      0x0040, "Shadow of Baseball" )
	PORT_DIPSETTING(      0x0000, "The Baseball Itself" )

	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( kickoff )
	PORT_INCLUDE( megasys1_generic )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, "Time" )  // -> !f0082.w
	PORT_DIPSETTING(      0x0003, "3'" )
	PORT_DIPSETTING(      0x0002, "4'" )
	PORT_DIPSETTING(      0x0001, "5'" )
	PORT_DIPSETTING(      0x0000, "6'" )
	PORT_DIPUNUSED( 0x0004, 0x0004 ) /* Manual states dips 3 & 4 are "Unused" */
	PORT_DIPUNUSED( 0x0008, 0x0008 )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Harder ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Controls ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Trackball ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Joystick ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPUNUSED( 0x0800, 0x0800 )
	PORT_DIPUNUSED( 0x1000, 0x1000 )
	PORT_DIPNAME( 0x2000, 0x2000, "Freeze Screen (Cheat)" )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Text" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Japanese ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )  // show "Japan Only" warning
INPUT_PORTS_END

static INPUT_PORTS_START( lomakai )
	PORT_INCLUDE( megasys1_generic )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0002, "4" )
	PORT_DIPSETTING(      0x0001, "5" )
	PORT_DIPUNUSED( 0x0004, 0x0004 )
	PORT_DIPUNUSED( 0x0008, 0x0008 )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Difficulty ) )   // boss enemy damage bar
	PORT_DIPSETTING(      0x0030, DEF_STR( Easy ) )         // 6 dots
	PORT_DIPSETTING(      0x0020, DEF_STR( Normal ) )       // 8 dots
	PORT_DIPSETTING(      0x0010, DEF_STR( Hard ) )         // 9 dots
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )      // 12 dots
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Invulnerability (Cheat)")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( p47 )
	PORT_INCLUDE( lomakai )

	PORT_MODIFY("DSW")   /* Manual states dips 3 & 4 are "Unused" */
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Hardest ) )
	PORT_DIPUNUSED( 0x0040, 0x0040 ) /* Manual states this dip is "Unused" */

	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( plusalph )
	PORT_INCLUDE( hachoo )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0002, "4" )
	PORT_DIPSETTING(      0x0001, "5" )
	PORT_DIPSETTING(      0x0000, "Infinite (Cheat)" )
	PORT_DIPNAME( 0x0004, 0x0004, "Hyper Laser Beams" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0004, "3" )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(      0x0008, "70k and every 200k")     /* As stated in manual, but was 70K & Every 130K */
	PORT_DIPSETTING(      0x0000, "100k and 300k Only")     /* As stated in manual, but was 100K & Every 200K */
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Cocktail ) )

	PORT_DIPNAME( 0x8000, 0x8000, "Freeze Screen (Cheat)")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( rodland )
	PORT_INCLUDE( hachoo )

	PORT_MODIFY("DSW")   /* Manual states dips 1 & 2 are "Unused" */
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0004, "2" )
	PORT_DIPSETTING(      0x000c, "3" )
	PORT_DIPSETTING(      0x0008, "4" )
	PORT_DIPSETTING(      0x0000, "Infinite (Cheat)")
	PORT_DIPNAME( 0x0010, 0x0010, "Default episode" )
	PORT_DIPSETTING(      0x0010, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPNAME( 0x0060, 0x0060, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Hardest ) )

	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x8000, IP_ACTIVE_LOW )
INPUT_PORTS_END

static INPUT_PORTS_START( stdragon )
	PORT_INCLUDE( lomakai )

	PORT_MODIFY("DSW")  /* Manual states dips 3 & 4 are "Unused" */
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )

	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x8000, 0x8000 )    // used?
INPUT_PORTS_END

static INPUT_PORTS_START( soldam )
	PORT_INCLUDE( lomakai )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy )   )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Hard )   )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hardest ))
	PORT_DIPNAME( 0x000c, 0x000c, "Games To Play (Vs)" )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x000c, "2" )
	PORT_DIPSETTING(      0x0008, "3" )
	PORT_DIPSETTING(      0x0004, "4" )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Credits To Start (Vs)" )
	PORT_DIPSETTING(      0x0020, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPNAME( 0x0040, 0x0040, "Credits To Continue (Vs)" )
	PORT_DIPSETTING(      0x0040, "1" )
	PORT_DIPSETTING(      0x0000, "2" )

	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_SERVICE( 0x8000, IP_ACTIVE_LOW )
INPUT_PORTS_END

static INPUT_PORTS_START( tshingen )
	PORT_INCLUDE( hachoo )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)

	PORT_MODIFY("P2")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0003, 0x0001, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0003, "2" )
	PORT_DIPSETTING(      0x0001, "3" )
	PORT_DIPSETTING(      0x0002, "4" )
	PORT_DIPSETTING(      0x0000, "Infinite (Cheat)" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(      0x000c, "20k" )
	PORT_DIPSETTING(      0x0004, "30k" )
	PORT_DIPSETTING(      0x0008, "40k" )
	PORT_DIPSETTING(      0x0000, "50k" )
	PORT_DIPNAME( 0x0030, 0x0010, DEF_STR( Difficulty ) )   // damage when hit
	PORT_DIPSETTING(      0x0030, DEF_STR( Easy ) )         // 0
	PORT_DIPSETTING(      0x0010, DEF_STR( Normal )  )      // 1
	PORT_DIPSETTING(      0x0020, DEF_STR( Hard ) )         // 2
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )      // 3
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( hayaosi1 )
	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(3)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	COINAGE_8BITS

	PORT_START("DSW2")          /* 0x69353.b */
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Unknown 2-2" )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Points To Win" )
	PORT_DIPSETTING(      0x0000, "10" )
	PORT_DIPSETTING(      0x0020, "15" )
	PORT_DIPNAME( 0x0040, 0x0040, "Unknown 2-6" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Unknown 2-7" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( peekaboo )
	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN3 )        // called "service"
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN4 )        // called "test"
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON3 )      // called "stage clear"
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON4 )      // called "option"
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x00ff, 0x0080, IPT_PADDLE ) PORT_PLAYER(1) PORT_MINMAX(0x0018,0x00e0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_CENTERDELTA(0)

	PORT_START("P2")
	PORT_BIT( 0x00ff, 0x0080, IPT_PADDLE ) PORT_PLAYER(2) PORT_MINMAX(0x0018,0x00e0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_COCKTAIL

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )       // 1f0358<-!
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hardest ) )
	PORT_SERVICE( 0x0004, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0008, 0x0008, "Movement" )                  // 1f0392<-!
	PORT_DIPSETTING(      0x0008, "Paddles" )
	PORT_DIPSETTING(      0x0000, "Buttons" )
	PORT_DIPNAME( 0x0030, 0x0030, "Nudity" )                    // 1f0356<-!
	PORT_DIPSETTING(      0x0030, "Female and Male (Full)" )
	PORT_DIPSETTING(      0x0020, "Female (Full)" )
	PORT_DIPSETTING(      0x0010, "Female (Partial)" )
	PORT_DIPSETTING(      0x0000, DEF_STR( None ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Cabinet ) )          // 1f006a<-!
	PORT_DIPSETTING(      0x0040, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Number of controllers" )     // 1f0074<-!
	PORT_DIPSETTING(      0x0080, "1" )
	PORT_DIPSETTING(      0x0000, "2" )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) )      // 1f0354<-
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Flip_Screen ) )      // 1f0022/6e<-!
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

/*************************************
 *
 *  Peek-a-Boo! Custom I/O
 *
 *************************************/


/* Read the input ports, through a protection device */
READ16_MEMBER(megasys1_state::protection_peekaboo_r)
{
	switch (m_protection_val)
	{
		case 0x02:  return 0x03;
		case 0x51:  return m_io_p1->read();
		case 0x52:  return m_io_p2->read();
		default:    return m_protection_val;
	}
}

WRITE16_MEMBER(megasys1_state::protection_peekaboo_w)
{
	COMBINE_DATA(&m_protection_val);

	if ((m_protection_val & 0x90) == 0x90)
		membank("okibank")->set_entry(m_protection_val & 7);

	m_maincpu->set_input_line(4, HOLD_LINE);
}

/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout tilelayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1)   },
	{ STEP8(0,4)   },
	{ STEP8(0,4*8) },
	8*8*4
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1)   },
	{ STEP8(8*8*4*0,4), STEP8(8*8*4*2,4) },
	{ STEP16(0,4*8) },
	16*16*4
};

static GFXDECODE_START( Z )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,   256*0, 16 )   // [0] Scroll 0
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,   256*2, 16 )   // [1] Scroll 1
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout, 256*1, 16 )   // [2] Sprites
GFXDECODE_END

static GFXDECODE_START( ABC )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,   256*0, 16 )   // [0] Scroll 0
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,   256*1, 16 )   // [1] Scroll 1
	GFXDECODE_ENTRY( "gfx3", 0, tilelayout,   256*2, 16 )   // [2] Scroll 2 (unused in system D)
	GFXDECODE_ENTRY( "gfx4", 0, spritelayout, 256*3, 16 )   // [3] Sprites
GFXDECODE_END

/*************************************
 *
 *  Machine Driver Macros
 *
 *************************************/

/***************************************************************************

                        [  Mega System 1 A,B and C ]

                          2x68000 2xM6295 1xYM2151

***************************************************************************/

/* Provided by Jim Hernandez: 3.5MHz for FM, 30KHz (!) for ADPCM */

static MACHINE_CONFIG_START( system_A, megasys1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, SYS_A_CPU_CLOCK) /* 6MHz verified */
	MCFG_CPU_PROGRAM_MAP(megasys1A_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", megasys1_state, megasys1A_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("audiocpu", M68000, SOUND_CPU_CLOCK) /* 7MHz verified */
	MCFG_CPU_PROGRAM_MAP(megasys1A_sound_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(120000))

	MCFG_MACHINE_RESET_OVERRIDE(megasys1_state,megasys1)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(megasys1_state, screen_update_megasys1)
	MCFG_SCREEN_VBLANK_DRIVER(megasys1_state, screen_eof_megasys1)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ABC)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)
	MCFG_PALETTE_INIT_OWNER(megasys1_state,megasys1)
	MCFG_VIDEO_START_OVERRIDE(megasys1_state,megasys1)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", SOUND_CPU_CLOCK/2) /* 3.5MHz (7MHz / 2) verified */
	MCFG_YM2151_IRQ_HANDLER(WRITELINE(megasys1_state,sound_irq))
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.80)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.80)

	MCFG_OKIM6295_ADD("oki1", OKI4_SOUND_CLOCK, OKIM6295_PIN7_HIGH) /* 4MHz verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.30)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.30)

	MCFG_OKIM6295_ADD("oki2", OKI4_SOUND_CLOCK, OKIM6295_PIN7_HIGH) /* 4MHz verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.30)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.30)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( system_A_hachoo, system_A )
	MCFG_MACHINE_RESET_OVERRIDE(megasys1_state,megasys1_hachoo)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( system_B, system_A )

	/* basic machine hardware */

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(SYS_B_CPU_CLOCK) /* 8MHz */
	MCFG_CPU_PROGRAM_MAP(megasys1B_map)
	MCFG_TIMER_MODIFY("scantimer")
	MCFG_TIMER_DRIVER_CALLBACK(megasys1_state, megasys1B_scanline)

	MCFG_CPU_MODIFY("audiocpu")
	MCFG_CPU_PROGRAM_MAP(megasys1B_sound_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( system_Bbl, megasys1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, SYS_B_CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(megasys1B_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", megasys1_state, megasys1B_scanline, "screen", 0, 1)

	MCFG_MACHINE_RESET_OVERRIDE(megasys1_state,megasys1)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(megasys1_state, screen_update_megasys1)
	MCFG_SCREEN_VBLANK_DRIVER(megasys1_state, screen_eof_megasys1)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ABC)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)
	MCFG_PALETTE_INIT_OWNER(megasys1_state,megasys1)
	MCFG_VIDEO_START_OVERRIDE(megasys1_state,megasys1)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	/* just the one OKI, used for sound and music */
	MCFG_OKIM6295_ADD("oki1", OKI4_SOUND_CLOCK, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.30)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.30)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( system_B_hayaosi1, system_B )

	/* basic machine hardware */

	MCFG_OKIM6295_REPLACE("oki1", 2000000, OKIM6295_PIN7_HIGH) /* correct speed, but unknown OSC + divider combo */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.30)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.30)

	MCFG_OKIM6295_REPLACE("oki2", 2000000, OKIM6295_PIN7_HIGH) /* correct speed, but unknown OSC + divider combo */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.30)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.30)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( system_C, system_A )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(SYS_C_CPU_CLOCK) /* 12MHz */
	MCFG_CPU_PROGRAM_MAP(megasys1C_map)
	MCFG_TIMER_MODIFY("scantimer")
	MCFG_TIMER_DRIVER_CALLBACK(megasys1_state, megasys1B_scanline)

	MCFG_CPU_MODIFY("audiocpu")
	MCFG_CPU_PROGRAM_MAP(megasys1B_sound_map)
MACHINE_CONFIG_END


/***************************************************************************

                            [ Mega System 1 D ]

                              1x68000 1xM6295

                                OSC: 8MHz

***************************************************************************/


static MACHINE_CONFIG_START( system_D, megasys1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, SYS_D_CPU_CLOCK)    /* 8MHz */
	MCFG_CPU_PROGRAM_MAP(megasys1D_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", megasys1_state,  megasys1D_irq)

	MCFG_MACHINE_RESET_OVERRIDE(megasys1_state,megasys1)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(megasys1_state, screen_update_megasys1)
	MCFG_SCREEN_VBLANK_DRIVER(megasys1_state, screen_eof_megasys1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ABC)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(RRRRRGGGGGBBBBBx)
	MCFG_PALETTE_INIT_OWNER(megasys1_state,megasys1)
	MCFG_VIDEO_START_OVERRIDE(megasys1_state,megasys1)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki1", SYS_D_CPU_CLOCK/4, OKIM6295_PIN7_HIGH)    /* 2MHz (8MHz / 4) */
	MCFG_DEVICE_ADDRESS_MAP(AS_0, megasys1D_oki_map)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END




/***************************************************************************

                            [  Mega System 1 Z ]

                             68000+Z80 1xYM2203

                            OSC:    5, 12 MHz

***************************************************************************/


WRITE_LINE_MEMBER(megasys1_state::irqhandler)
{
	m_audiocpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}

static MACHINE_CONFIG_START( system_Z, megasys1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, SYS_A_CPU_CLOCK) /* 6MHz (12MHz / 2) */
	MCFG_CPU_PROGRAM_MAP(megasys1A_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", megasys1_state, megasys1A_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("audiocpu", Z80, 3000000) /* OSC 12MHz divided by 4 ??? */
	MCFG_CPU_PROGRAM_MAP(z80_sound_map)
	MCFG_CPU_IO_MAP(z80_sound_io_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(megasys1_state, screen_update_megasys1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", Z)
	MCFG_PALETTE_ADD("palette", 768)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(megasys1_state,megasys1)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, 1500000)
	MCFG_YM2203_IRQ_HANDLER(WRITELINE(megasys1_state, irqhandler))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

/***************************************************************************

                            [ 64th Street ]

It runs on 1 MC68000P12 and 1 MC6800P10
For the sound It has 1 YM 2151 and 1 YM 3012

(World version)
interrupts: 1] 10eac:   disabled while b6c4=6 (10fb6 test)
                        if (8b1c)   8b1c<-0
                            color cycle
                            copies 800 bytes 98da->8008

            2] 10f28:   switch b6c4
                        0   RTE
                        2   10f44:  M[b6c2]<-d8000; b6c4<-4
                        4   10f6c:  next b6c2 & d8000.  if (b6c2>A) b6c2,4<-0
                                                        else        b6c4  <-2
                        6   10f82: b6c6<-(d8001) b6c7<-FF (test)

            4] 10ed0:   disabled while b6c4=6 (10fb6 test)
                        watchdog 8b1e
                        many routines...
                        b6c2<-0

13ca    print a string: a7->screen disp.l(base=f0004),src.l
13ea    print a string: a1->(chars)*
1253c   hw test (table of tests at 125c6)       *TRAP#D*
125f8   mem test (table of mem tests at 126d4)
1278e   input test (table of tests at 12808)
128a8   sound test  12a08   crt test
12aca   dsw test (b68e.w = dswa.b|dswb.b)

ff8b1e.w    incremented by int4, when >= b TRAP#E (software watchdog error)
ff9df8.w    *** level ***

***************************************************************************/


ROM_START( 64street )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* Main CPU Code */
	ROM_LOAD16_BYTE( "64th_03.rom", 0x000000, 0x040000, CRC(ed6c6942) SHA1(f610b31548ed4889a43d77be286b9bfabf700064) )
	ROM_LOAD16_BYTE( "64th_02.rom", 0x000001, 0x040000, CRC(0621ed1d) SHA1(97d3e84cced23865157c5a15cbf5b7671c1dbae1) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "64th_08.rom", 0x000000, 0x010000, CRC(632be0c1) SHA1(626073037249d96ac70b2d11b2dd72b22bac49c7) )
	ROM_LOAD16_BYTE( "64th_07.rom", 0x000001, 0x010000, CRC(13595d01) SHA1(e730a530ca232aab883217fa12804075cb2aa640) )

	ROM_REGION( 0x1000, "mcu", 0 ) /* MCU Internal Code, M50747? */
	ROM_LOAD( "64street.mcu", 0x000000, 0x1000, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "64th_01.rom", 0x000000, 0x080000, CRC(06222f90) SHA1(52b6cb88b9d2209c16d1633c83c0224b6ebf29dc) )

	ROM_REGION( 0x80000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "64th_06.rom", 0x000000, 0x080000, CRC(2bfcdc75) SHA1(f49f92f1ff58dccf72e05ecf80761c7b65a25ba3) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "64th_09.rom", 0x000000, 0x020000, CRC(a4a97db4) SHA1(1179457a6f33b3b44fac6056f6245f3aaae6afd5) )

	ROM_REGION( 0x100000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "64th_05.rom", 0x000000, 0x080000, CRC(a89a7020) SHA1(be36e58e9314688ee39249944c5a6c201e0249ee) )
	ROM_LOAD( "64th_04.rom", 0x080000, 0x080000, CRC(98f83ef6) SHA1(e9b72487695ac7cdc4fbf595389c4b8781ed207e) )

	ROM_REGION( 0x40000, "oki1", 0 )        /* Samples */
	ROM_LOAD( "64th_11.rom", 0x000000, 0x020000, CRC(b0b8a65c) SHA1(b7e42d9083d0bbfe160fc73a7317d696e90d83d6) )

	ROM_REGION( 0x40000, "oki2", 0 )        /* Samples */
	ROM_LOAD( "64th_10.rom", 0x000000, 0x040000, CRC(a3390561) SHA1(f86d5c61e3e80d30408535c2203940ca1e95ac18) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM */
	ROM_LOAD( "pr91009.12",  0x0000, 0x0200, CRC(c69423d6) SHA1(ba9644a9899df2d73a5a16bf7ceef1954c2e25f3) ) // same as pr-91044 on hayaosi1
ROM_END


ROM_START( 64streetj )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* Main CPU Code */
	ROM_LOAD16_BYTE( "91105-3.bin", 0x000000, 0x040000, CRC(a211a83b) SHA1(423d8f273f1520f6a37f1255bb2d343a6bbd790a) )
	ROM_LOAD16_BYTE( "91105-2.bin", 0x000001, 0x040000, CRC(27c1f436) SHA1(d7936523549cfcd99ba98c6776ebd225b245867b) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "64th_08.rom", 0x000000, 0x010000, CRC(632be0c1) SHA1(626073037249d96ac70b2d11b2dd72b22bac49c7) )
	ROM_LOAD16_BYTE( "64th_07.rom", 0x000001, 0x010000, CRC(13595d01) SHA1(e730a530ca232aab883217fa12804075cb2aa640) )

	ROM_REGION( 0x1000, "mcu", 0 ) /* MCU Internal Code, m50747? */
	ROM_LOAD( "64street.mcu", 0x000000, 0x1000, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "64th_01.rom", 0x000000, 0x080000, CRC(06222f90) SHA1(52b6cb88b9d2209c16d1633c83c0224b6ebf29dc) )

	ROM_REGION( 0x80000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "64th_06.rom", 0x000000, 0x080000, CRC(2bfcdc75) SHA1(f49f92f1ff58dccf72e05ecf80761c7b65a25ba3) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "64th_09.rom", 0x000000, 0x020000, CRC(a4a97db4) SHA1(1179457a6f33b3b44fac6056f6245f3aaae6afd5) )

	ROM_REGION( 0x100000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "64th_05.rom", 0x000000, 0x080000, CRC(a89a7020) SHA1(be36e58e9314688ee39249944c5a6c201e0249ee) )
	ROM_LOAD( "64th_04.rom", 0x080000, 0x080000, CRC(98f83ef6) SHA1(e9b72487695ac7cdc4fbf595389c4b8781ed207e) )

	ROM_REGION( 0x40000, "oki1", 0 )        /* Samples */
	ROM_LOAD( "64th_11.rom", 0x000000, 0x020000, CRC(b0b8a65c) SHA1(b7e42d9083d0bbfe160fc73a7317d696e90d83d6) )

	ROM_REGION( 0x40000, "oki2", 0 )        /* Samples */
	ROM_LOAD( "64th_10.rom", 0x000000, 0x040000, CRC(a3390561) SHA1(f86d5c61e3e80d30408535c2203940ca1e95ac18) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM */
	ROM_LOAD( "pr91009.12",  0x0000, 0x0200, CRC(c69423d6) SHA1(ba9644a9899df2d73a5a16bf7ceef1954c2e25f3) ) // same as pr-91044 on hayaosi1
ROM_END


/***************************************************************************

                    [ The Astyanax ] / [ The Lord of King ]

interrupts: 1] 1aa  2] 1b4

***************************************************************************/


ROM_START( astyanax )
	ROM_REGION( 0x60000, "maincpu", 0 )     /* Main CPU Code */
	ROM_LOAD16_BYTE( "astyan2.bin", 0x00000, 0x20000, CRC(1b598dcc) SHA1(f6b733d9b0e81226eb784aaddda1791e3e95b816) )
	ROM_LOAD16_BYTE( "astyan1.bin", 0x00001, 0x20000, CRC(1a1ad3cf) SHA1(e094b4528e6f36eb30bfc148f2ad50d876e9280a) )
	ROM_LOAD16_BYTE( "astyan3.bin", 0x40000, 0x10000, CRC(097b53a6) SHA1(80952b2e685cefa8dd7c31b1ec54c4de924a84eb) )
	ROM_LOAD16_BYTE( "astyan4.bin", 0x40001, 0x10000, CRC(1e1cbdb2) SHA1(5d076233d5ed6fdd9f0ecf64453325c14d33e879) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "astyan5.bin",  0x000000, 0x010000, CRC(11c74045) SHA1(00310a08a1c9a08050004e39b111b940142f8dea) )
	ROM_LOAD16_BYTE( "astyan6.bin",  0x000001, 0x010000, CRC(eecd4b16) SHA1(2078e900b53347aad008a8ce7191f4e5541d4df0) )

	ROM_REGION( 0x1000, "mcu", 0 ) /* M50747 MCU Code */
	ROM_LOAD( "m50747", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "astyan11.bin", 0x000000, 0x020000, CRC(5593fec9) SHA1(8fa5bfa8921c6f03ddf485276207978e345887d5) )
	ROM_LOAD( "astyan12.bin", 0x020000, 0x020000, CRC(e8b313ec) SHA1(ee690e284ab05db858aad4f0a0b24681c14f93c8) )
	ROM_LOAD( "astyan13.bin", 0x040000, 0x020000, CRC(5f3496c6) SHA1(56f2beb2a6224cd77ee7fb56ed2685b78271c27a) )
	ROM_LOAD( "astyan14.bin", 0x060000, 0x020000, CRC(29a09ec2) SHA1(6eab93fdc3491da54ca4da8a2a04a095b85aee57) )

	ROM_REGION( 0x80000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "astyan15.bin", 0x000000, 0x020000, CRC(0d316615) SHA1(64e6c2a836ba91e17277c7a9fd65cfb6faa88c04) )
	ROM_LOAD( "astyan16.bin", 0x020000, 0x020000, CRC(ba96e8d9) SHA1(da1e8bfc2861df267b11991ddc9329101a6ad0c8) )
	ROM_LOAD( "astyan17.bin", 0x040000, 0x020000, CRC(be60ba06) SHA1(205fb6fe3fe27ef40346c2ccd61168477cd83ac1) )
	ROM_LOAD( "astyan18.bin", 0x060000, 0x020000, CRC(3668da3d) SHA1(4e8328851b26985b3a2d56d398411fbaeae8012c) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "astyan19.bin", 0x000000, 0x020000, CRC(98158623) SHA1(e9088d0d4b8c07bd21398f68966cb8633716a9b7) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "astyan20.bin", 0x000000, 0x020000, CRC(c1ad9aa0) SHA1(b19bc564ccb3fdb06300a64ccd672aace734393f) )
	ROM_LOAD( "astyan21.bin", 0x020000, 0x020000, CRC(0bf498ee) SHA1(9d7e25e97cec6056d7b9abd36f5a1e4c162b70a3) )
	ROM_LOAD( "astyan22.bin", 0x040000, 0x020000, CRC(5f04d9b1) SHA1(1f58a49a01129f8ef921f3bc284c299393213af3) )
	ROM_LOAD( "astyan23.bin", 0x060000, 0x020000, CRC(7bd4d1e7) SHA1(888a2224cca41db19d84da344db657d924dcc019) )

	ROM_REGION( 0x40000, "oki1", 0 )        /* Samples */
	ROM_LOAD( "astyan9.bin",  0x000000, 0x020000, CRC(a10b3f17) SHA1(6b548d99f0c10f15d14f1a14d494f3348ea8e484) )
	ROM_LOAD( "astyan10.bin", 0x020000, 0x020000, CRC(4f704e7a) SHA1(10a3cabb087b065fb28d2838c476125e051cbbf8) )

	ROM_REGION( 0x40000, "oki2", 0 )        /* Samples */
	ROM_LOAD( "astyan7.bin",  0x000000, 0x020000, CRC(319418cc) SHA1(4056948e12f2191eecd2f4140a7de4340ab1554f) )
	ROM_LOAD( "astyan8.bin",  0x020000, 0x020000, CRC(5e5d2a22) SHA1(fc039d804fdcb8ce089e4436260d64408640b264) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM */
	ROM_LOAD( "rd.bpr",       0x0000, 0x0200, CRC(85b30ac4) SHA1(b03f577ceb0f26b67453ffa52ef61fea76a93184) )
ROM_END


ROM_START( lordofk )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* Main CPU Code */
	ROM_LOAD16_BYTE( "lokj02.bin", 0x00000, 0x20000, CRC(0d7f9b4a) SHA1(551f237cd60e3b9aa339e393a672b08645b043cc) )
	ROM_LOAD16_BYTE( "lokj01.bin", 0x00001, 0x20000, CRC(bed3cb93) SHA1(ad4dbdacded60289ebf245111ce4543151b9456a) )
	ROM_LOAD16_BYTE( "lokj03.bin", 0x40000, 0x20000, CRC(d8702c91) SHA1(bdf0ed1f116b0c8589a5b6c61e6f441b5afa38cb) )
	ROM_LOAD16_BYTE( "lokj04.bin", 0x40001, 0x20000, CRC(eccbf8c9) SHA1(f37fb6a536f6344d6d68c8193de4db5d70b29c0a) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "astyan5.bin",  0x000000, 0x010000, CRC(11c74045) SHA1(00310a08a1c9a08050004e39b111b940142f8dea) )
	ROM_LOAD16_BYTE( "astyan6.bin",  0x000001, 0x010000, CRC(eecd4b16) SHA1(2078e900b53347aad008a8ce7191f4e5541d4df0) )

	ROM_REGION( 0x1000, "mcu", 0 ) /* M50747 MCU Code */
	ROM_LOAD( "m50747", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "astyan11.bin", 0x000000, 0x020000, CRC(5593fec9) SHA1(8fa5bfa8921c6f03ddf485276207978e345887d5) )
	ROM_LOAD( "astyan12.bin", 0x020000, 0x020000, CRC(e8b313ec) SHA1(ee690e284ab05db858aad4f0a0b24681c14f93c8) )
	ROM_LOAD( "astyan13.bin", 0x040000, 0x020000, CRC(5f3496c6) SHA1(56f2beb2a6224cd77ee7fb56ed2685b78271c27a) )
	ROM_LOAD( "astyan14.bin", 0x060000, 0x020000, CRC(29a09ec2) SHA1(6eab93fdc3491da54ca4da8a2a04a095b85aee57) )

	ROM_REGION( 0x80000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "astyan15.bin", 0x000000, 0x020000, CRC(0d316615) SHA1(64e6c2a836ba91e17277c7a9fd65cfb6faa88c04) )
	ROM_LOAD( "astyan16.bin", 0x020000, 0x020000, CRC(ba96e8d9) SHA1(da1e8bfc2861df267b11991ddc9329101a6ad0c8) )
	ROM_LOAD( "astyan17.bin", 0x040000, 0x020000, CRC(be60ba06) SHA1(205fb6fe3fe27ef40346c2ccd61168477cd83ac1) )
	ROM_LOAD( "astyan18.bin", 0x060000, 0x020000, CRC(3668da3d) SHA1(4e8328851b26985b3a2d56d398411fbaeae8012c) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "astyan19.bin", 0x000000, 0x020000, CRC(98158623) SHA1(e9088d0d4b8c07bd21398f68966cb8633716a9b7) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "astyan20.bin", 0x000000, 0x020000, CRC(c1ad9aa0) SHA1(b19bc564ccb3fdb06300a64ccd672aace734393f) )
	ROM_LOAD( "astyan21.bin", 0x020000, 0x020000, CRC(0bf498ee) SHA1(9d7e25e97cec6056d7b9abd36f5a1e4c162b70a3) )
	ROM_LOAD( "astyan22.bin", 0x040000, 0x020000, CRC(5f04d9b1) SHA1(1f58a49a01129f8ef921f3bc284c299393213af3) )
	ROM_LOAD( "astyan23.bin", 0x060000, 0x020000, CRC(7bd4d1e7) SHA1(888a2224cca41db19d84da344db657d924dcc019) )

	ROM_REGION( 0x40000, "oki1", 0 )        /* Samples */
	ROM_LOAD( "astyan9.bin",  0x000000, 0x020000, CRC(a10b3f17) SHA1(6b548d99f0c10f15d14f1a14d494f3348ea8e484) )
	ROM_LOAD( "astyan10.bin", 0x020000, 0x020000, CRC(4f704e7a) SHA1(10a3cabb087b065fb28d2838c476125e051cbbf8) )

	ROM_REGION( 0x40000, "oki2", 0 )        /* Samples */
	ROM_LOAD( "astyan7.bin",  0x000000, 0x020000, CRC(319418cc) SHA1(4056948e12f2191eecd2f4140a7de4340ab1554f) )
	ROM_LOAD( "astyan8.bin",  0x020000, 0x020000, CRC(5e5d2a22) SHA1(fc039d804fdcb8ce089e4436260d64408640b264) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM */
	ROM_LOAD( "rd.bpr",       0x0000, 0x0200, CRC(85b30ac4) SHA1(b03f577ceb0f26b67453ffa52ef61fea76a93184) )
ROM_END


/***************************************************************************

                    [ Avenging Spirit ] / [ Phantasm ]

2 TMP68000
1 YM2151
1 OSC 8Mhz
1 OSC 12Mhz
1 OSC 7 Mhz

 Name    Location
 ----------------------
 9        23G        27C1001
 1        5F          "
 2        3F          "
 3        1J          "
 4        1K          "
 5        5B         27C020
 6        3B          "

(Avspirit)
interrupts: 2,3, 5,6,7]     move.w  $e0000.l, $78e9e.l
                            andi.w  #$ff, $78e9e.l
            4] 78b20 software watchdog (78ea0 enables it)


fd6     reads e0000 (values FF,06,34,35,36,37)
ffa     e0000<-6 test

79584.w *** level ***

1] E9C
2] ED4
3] F4C      rte
4-7] ED2    rte

***************************************************************************/


ROM_START( avspirit )
	ROM_REGION( 0xc0000, "maincpu", 0 )     /* Main CPU Code: 00000-3ffff & 80000-bffff */
	ROM_LOAD16_BYTE( "spirit05.rom",  0x000000, 0x020000, CRC(b26a341a) SHA1(5ff5b7d3aa73cc7cea7b6e8cc2ba55f4cd9b52e5) )
	ROM_CONTINUE (                    0x080000, 0x020000 )
	ROM_LOAD16_BYTE(  "spirit06.rom", 0x000001, 0x020000, CRC(609f71fe) SHA1(ab1bfe211763fb855477645267223e7fd4d6b6da) )
	ROM_CONTINUE (                    0x080001, 0x020000 )

	ROM_REGION( 0x40000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "spirit01.rom",  0x000000, 0x020000, CRC(d02ec045) SHA1(465b61d89ca06e7e0a42c42efb6919c964ad0f93) )
	ROM_LOAD16_BYTE( "spirit02.rom",  0x000001, 0x020000, CRC(30213390) SHA1(9334978d3568b36215ed29789501f7cbaf6651ea) )

	ROM_REGION( 0x1000, "mcu", 0 ) /* MCU Internal Code */
	ROM_LOAD( "m50747", 0x000000, 0x1000, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "spirit12.rom",  0x000000, 0x080000, CRC(728335d4) SHA1(bbf13378ac0bff5e732eb30081b421ed89d12fa2) )

	ROM_REGION( 0x80000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "spirit11.rom",  0x000000, 0x080000, CRC(7896f6b0) SHA1(f09c1592aaa34eb5b7fe096ad4ccdcb155a5cadd) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "spirit09.rom",  0x000000, 0x020000, CRC(0c37edf7) SHA1(4074377f756b231b905b9b6a087c6d6ad3d49f52) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "spirit10.rom",  0x000000, 0x080000, CRC(2b1180b3) SHA1(6d62b6bd73b9dd23670a0683f28609be29ac1d98) )

	ROM_REGION( 0x40000, "oki1", 0 )        /* Samples */
	ROM_LOAD( "spirit14.rom",  0x000000, 0x040000, CRC(13be9979) SHA1(828ae745867e25834e51d08308b4ab5d8e80f2c8) )

	ROM_REGION( 0x40000, "oki2", 0 )        /* Samples */
	ROM_LOAD( "spirit13.rom",  0x000000, 0x040000, CRC(05bc04d9) SHA1(b903edf39393cad2b4b6b58b10651304793aaa3e) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM */
	ROM_LOAD( "ph.bin",        0x0000, 0x0200, CRC(8359650a) SHA1(97d0105f06c64340fb19a541db03481a7e0b5e05) )
ROM_END


ROM_START( phantasm )
	ROM_REGION( 0x60000, "maincpu", 0 )     /* Main CPU Code */
	ROM_LOAD16_BYTE( "phntsm02.bin", 0x000000, 0x020000, CRC(d96a3584) SHA1(3ae62c5785b6249f1921d914c1f094bcf850d8d1) )
	ROM_LOAD16_BYTE( "phntsm01.bin", 0x000001, 0x020000, CRC(a54b4b87) SHA1(92745c53d8550189c3b0ce55be9027447817a2dc) )
	ROM_LOAD16_BYTE( "phntsm03.bin", 0x040000, 0x010000, CRC(1d96ce20) SHA1(2fb79160ea0dd18b5713691e4cf195d27ac4e3c3) )
	ROM_LOAD16_BYTE( "phntsm04.bin", 0x040001, 0x010000, CRC(dc0c4994) SHA1(c3c72336b5032ef237490b095d3270de5803738c) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "phntsm05.bin", 0x000000, 0x010000, CRC(3b169b4a) SHA1(81c46fc94887c0cea363848b5c831dcf3b5b76de) )
	ROM_LOAD16_BYTE( "phntsm06.bin", 0x000001, 0x010000, CRC(df2dfb2e) SHA1(b2542fa478917d44dffcf9e11ff7eaac6019676d) )

	/* No MCU */

	ROM_REGION( 0x80000, "gfx1", 0 ) /* Scroll 0 */
//  ROM_LOAD( "phntsm14.bin",  0x000000, 0x080000, CRC(728335d4) SHA1(bbf13378ac0bff5e732eb30081b421ed89d12fa2) )
	ROM_LOAD( "spirit12.rom",  0x000000, 0x080000, CRC(728335d4) SHA1(bbf13378ac0bff5e732eb30081b421ed89d12fa2) )

	ROM_REGION( 0x80000, "gfx2", 0 ) /* Scroll 1 */
//  ROM_LOAD( "phntsm18.bin",  0x000000, 0x080000, CRC(7896f6b0) SHA1(f09c1592aaa34eb5b7fe096ad4ccdcb155a5cadd) )
	ROM_LOAD( "spirit11.rom",  0x000000, 0x080000, CRC(7896f6b0) SHA1(f09c1592aaa34eb5b7fe096ad4ccdcb155a5cadd) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* Scroll 2 */
//  ROM_LOAD( "phntsm19.bin",  0x000000, 0x020000, CRC(0c37edf7) SHA1(4074377f756b231b905b9b6a087c6d6ad3d49f52) )
	ROM_LOAD( "spirit09.rom",  0x000000, 0x020000, CRC(0c37edf7) SHA1(4074377f756b231b905b9b6a087c6d6ad3d49f52) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* Sprites */
//  ROM_LOAD( "phntsm23.bin",  0x000000, 0x080000, CRC(2b1180b3) SHA1(6d62b6bd73b9dd23670a0683f28609be29ac1d98) )
	ROM_LOAD( "spirit10.rom",  0x000000, 0x080000, CRC(2b1180b3) SHA1(6d62b6bd73b9dd23670a0683f28609be29ac1d98) )

	ROM_REGION( 0x40000, "oki1", 0 )        /* Samples */
//  ROM_LOAD( "phntsm10.bin", 0x000000, 0x040000, CRC(13be9979) SHA1(828ae745867e25834e51d08308b4ab5d8e80f2c8) )
	ROM_LOAD( "spirit14.rom", 0x000000, 0x040000, CRC(13be9979) SHA1(828ae745867e25834e51d08308b4ab5d8e80f2c8) )

	ROM_REGION( 0x40000, "oki2", 0 )        /* Samples */
//  ROM_LOAD( "phntsm08.bin", 0x000000, 0x040000, CRC(05bc04d9) SHA1(b903edf39393cad2b4b6b58b10651304793aaa3e) )
	ROM_LOAD( "spirit13.rom", 0x000000, 0x040000, CRC(05bc04d9) SHA1(b903edf39393cad2b4b6b58b10651304793aaa3e) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM */
	ROM_LOAD( "ph.bin",        0x0000, 0x0200, CRC(8359650a) SHA1(97d0105f06c64340fb19a541db03481a7e0b5e05) )
ROM_END


/***************************************************************************

Monky Elf

PCB Layout
----------

|---------------------------------------------|
|     1   62256   62256  62256  62256     8   |
|     2    3      4                           |
| M6295     68000                         9   |
| M6295                                       |
|YM2151                                   10  |
|                                             |
|J  6116                                      |
|A  6116                                      |
|M                                       12MHz|
|M                                            |
|A                                            |
|                                             |
|                                             |
|                                  62256      |
|     62256   62256                62256      |
|      5        6    6264                     |
|DSW1    68000       6264                     |
|                                  62256      |
|DSW2         16MHz      7         62256      |
|---------------------------------------------|
Notes:
     68000 clocks 8.000MHz [16/2]
     YM2151 clock 3.000MHz [12/4]
     M6295 clocks 3.000MHz [12/4] pin 7 high

***************************************************************************/


ROM_START( monkelf )
	ROM_REGION( 0xc0000, "maincpu", 0 )     /* Main CPU Code: 00000-3ffff & 80000-bffff */
	ROM_LOAD16_BYTE( "6",  0x000000, 0x020000, CRC(40b80914) SHA1(103dd3531b6b270e0d756801ff5ac69db5c6b82f) )
	ROM_CONTINUE (                   0x080000, 0x020000 )
	ROM_LOAD16_BYTE(  "5", 0x000001, 0x020000, CRC(6c45465d) SHA1(ae30c3f14617ffe99622a019eb64880ac14bf7cf) )
	ROM_CONTINUE (                   0x080001, 0x020000 )

	ROM_REGION( 0x40000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "4",  0x000000, 0x020000, CRC(d02ec045) SHA1(465b61d89ca06e7e0a42c42efb6919c964ad0f93) )
	ROM_LOAD16_BYTE( "3",  0x000001, 0x020000, CRC(30213390) SHA1(9334978d3568b36215ed29789501f7cbaf6651ea) )

	ROM_REGION( 0x80000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "8",  0x000000, 0x080000, CRC(728335d4) SHA1(bbf13378ac0bff5e732eb30081b421ed89d12fa2) )

	ROM_REGION( 0x80000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "9",  0x000000, 0x080000, CRC(7896f6b0) SHA1(f09c1592aaa34eb5b7fe096ad4ccdcb155a5cadd) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "10",  0x000000, 0x020000, CRC(0c37edf7) SHA1(4074377f756b231b905b9b6a087c6d6ad3d49f52) )

	ROM_REGION( 0x80000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "7",  0x000000, 0x080000, CRC(2b1180b3) SHA1(6d62b6bd73b9dd23670a0683f28609be29ac1d98) )

	ROM_REGION( 0x40000, "oki1", 0 )        /* Samples */
	ROM_LOAD( "1",  0x000000, 0x040000, CRC(13be9979) SHA1(828ae745867e25834e51d08308b4ab5d8e80f2c8) )

	ROM_REGION( 0x40000, "oki2", 0 )        /* Samples */
	ROM_LOAD( "2",  0x000000, 0x040000, CRC(05bc04d9) SHA1(b903edf39393cad2b4b6b58b10651304793aaa3e) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM */
//  ROM_LOAD( "ph.bin",        0x0000, 0x0200, CRC(8359650a) SHA1(97d0105f06c64340fb19a541db03481a7e0b5e05) )
	// the bootleg prom is a different format, i don't know how to use it
	ROM_LOAD( "82s147",        0x0000, 0x0200, CRC(547eccc0) SHA1(44dd92e899a7852d2fd937b7d45519315b8b4d4f) )
ROM_END


/***************************************************************************

                            [ Big Striker ]

PCB: RB-91105A EB911009-20045

Some eproms are rev 1.0 (7,8,10,11)

Some are rev 1.1 (2,3,9)

This PCB use 68K CPU (2x)

Chrystal : 7.000 Mhz + 24.000 Mhz

Sound : Ym 2151 (Stereo)

Note: RAM is ff0000-ffffff while sprites live in 1f8000-1f87ff

interrupts: 1]
            2]
            4]

$885c/e.w   *** time (BCD) ***

***************************************************************************/


ROM_START( bigstrik )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* Main CPU Code */
	ROM_LOAD16_BYTE( "91105v11.3", 0x000000, 0x020000, CRC(5d6e08ec) SHA1(4b80a5073cd0b0142cad094816b935d750ac11fb) )
	ROM_LOAD16_BYTE( "91105v11.2", 0x000001, 0x020000, CRC(2120f05b) SHA1(a769cf8c3a4fa6a3f604edf45ce6db35979826cb) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "91105v10.8", 0x000000, 0x010000, CRC(7dd69ece) SHA1(e8dc3cbce8cb3f549384cd114f8fc0e6c72462f3) )
	ROM_LOAD16_BYTE( "91105v10.7", 0x000001, 0x010000, CRC(bc2c1508) SHA1(110dece929f9b452eb287c736d394d1022a09d75) )

	ROM_REGION( 0x1000, "mcu", 0 ) /* MCU Internal Code, M50747 */
	ROM_LOAD( "bigstrik.mcu", 0x000000, 0x1000, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "91021-01.1",   0x000000, 0x080000, CRC(f1945858) SHA1(3ed3881d3a93f34de5a15c287e076db209477259) )

	ROM_REGION( 0x80000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "91021-03.6",   0x000000, 0x080000, CRC(e88821e5) SHA1(60ea179db5e958b721eee71e26398e4ea8b8457e) )

	ROM_REGION( 0x20000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "91105v11.9", 0x000000, 0x020000, CRC(7be1c50c) SHA1(1dd81a61deeda3866e3f1ca82384f5f1b2efac08) )

	ROM_REGION( 0x100000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "91021-02.5",   0x000000, 0x080000, CRC(199819ca) SHA1(2f85cb3a8fa12faab379377c9a5ce3add30e6abf) )

	ROM_REGION( 0x40000, "oki1", 0 )        /* Samples */
	ROM_LOAD( "91105v10.11", 0x000000, 0x040000, CRC(0ef8fd43) SHA1(c226db63d9427ba024e7c41d5518c8895b45feaa)  )

	ROM_REGION( 0x40000, "oki2", 0 )        /* Samples */
	ROM_LOAD( "91105v10.10", 0x000000, 0x040000, CRC(d273a92a) SHA1(9f94bb7a60dfc7158871c9239d72832ca7b8ad09)  )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM */
	ROM_LOAD( "82s131.12",      0x0000, 0x0200, CRC(4b00fccf) SHA1(61682a595e604772b0adf6446d265a04719a36cc) )
ROM_END


/***************************************************************************

                            [ Chimera Beast ]

interrupts: 1,3]
            2, 5,6]
            4]

Note: This game was a prototype

***************************************************************************/


ROM_START( chimerab )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* Main CPU Code */
	ROM_LOAD16_BYTE( "prg3.bin", 0x000000, 0x040000, CRC(70f1448f) SHA1(60aaee1cf7aa15ffa4962d947747b0ae7cdcfd8a) )
	ROM_LOAD16_BYTE( "prg2.bin", 0x000001, 0x040000, CRC(821dbb85) SHA1(df204db38995ff4c898b8a0121834ec1b84b215c) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "prg8.bin", 0x000000, 0x010000, CRC(a682b1ca) SHA1(66f5d5a73f5e8cba87eac09c55eee59117d94f7b) )
	ROM_LOAD16_BYTE( "prg7.bin", 0x000001, 0x010000, CRC(83b9982d) SHA1(68e7d344ebfffe19822c4cf9f7b13cb51f23537a) )

	ROM_REGION( 0x1000, "cpu2", 0 ) /* MCU Internal Code, m50747? */
	ROM_LOAD( "chimerab.mcu", 0x000000, 0x1000, NO_DUMP )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "s1.bin",   0x000000, 0x080000, CRC(e4c2ac77) SHA1(db4bff3c02f22cc59a67b103fd176f4d88531f93) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "s2.bin",   0x000000, 0x080000, CRC(fafb37a5) SHA1(e36c4d18209add696982e36e84397ec51b9a9e7e) )

	ROM_REGION( 0x020000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "scr3.bin", 0x000000, 0x020000, CRC(5fe38a83) SHA1(0492be1a92baacb80ed5bdc0167beda3e9163d76) )

	ROM_REGION( 0x100000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "b2.bin",   0x000000, 0x080000, CRC(6e7f1778) SHA1(ac93f56c998f28e3f453fcdbf85f3217c9ae97de) )
	ROM_LOAD( "b1.bin",   0x080000, 0x080000, CRC(29c0385e) SHA1(4416cb17d3121ec00bceff2614dc424a359f127a) )

	ROM_REGION( 0x040000, "oki1", 0 )       /* Samples */
	ROM_LOAD( "voi11.bin", 0x000000, 0x040000, CRC(14b3afe6) SHA1(6d8659d0fc6980ffc1661702fc787737448dce9d) )

	ROM_REGION( 0x040000, "oki2", 0 )       /* Samples */
	ROM_LOAD( "voi10.bin", 0x000000, 0x040000, CRC(67498914) SHA1(8d89fa90f38fd102b15f26f71491ea833ec32cb2) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM */
	ROM_LOAD( "prom",         0x0000, 0x0200, NO_DUMP )
ROM_END


/***************************************************************************

                                [ Cybattler ]

 Cybattler (JPN Ver.)
 (c)1991 Jaleco
 RB-91105A EB91009-20045-1
 CPU  :TMP68000P-12,MC68000P10
 Sound:YM2151,OKI M6295 x2
 OSC  :24.000MHz,7.000MHz,4.000MHz

 01.1         [1109337f]    MR91028-01
 2_VER10.2    [2ed14c50]    RB91105A 2 Ver1.0
 3_VER10.3    [bee20587]    RB91105A 3 Ver1.0
 02.4         [882825db]    MR91028-02
 03.5         [4cd49f58]    MR91028-03
 04.6         [0c91798e]    MR91028-04
 7_VER10.7    [85d219d7]    RB91105A 7 Ver1.0
 8_VER10.8    [bf7b3558]    RB91105A 8 Ver1.0
 9_VER10.9    [37b1f195]    RB91105A 9 Ver1.0
 10_VER10.10  [8af95eed]    RB91105A 10 Ver1.0
 11_VER10.11  [59d62d1f]    RB91105A 11 Ver1.0

 PR-91028.12  [cfe90082]

interrupts: 1,3]    408
            2, 5,6] 498
                    1fd2c2.w routine index:
                    0:  4be>    1fd2c0.w <- d8000
                    2:  4ca>    1fd2d0+(1fd2c4.w) <- d8000. next
                    4:  4ee>    1fd2c4.w += 2.
                                            S   P1  P2  DB  DA
                                d8000 <-    56  52  53  55  54
                                1fd000+     00  02  04  06  08
                                depending on 1fd2c4.        previous
                    6:  4be again

            4]      452

c2208 <- 1fd040 (layers enable)
c2200 <- 1fd042 (sprite control)
c2308 <- 1fd046 (screen control)
c2004 <- 1fd054 (scroll 0 ctrl) c2000 <- 1fd220 (scroll 0 x)    c2002 <- 1fd222 (scroll 1 y)
c200c <- 1fd05a (scroll 1 ctrl) c2008 <- 1fd224 (scroll 1 x)    c200a <- 1fd226 (scroll 2 y)
c2104 <- 1fd060 (scroll 2 ctrl) c2100 <- 1fd228 (scroll 2 x)    c2102 <- 1fd22a (scroll 3 y)

1f0010.w    *** level (0,1,..) ***
1fb044.l    *** score / 10 ***

***************************************************************************/


ROM_START( cybattlr )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* Main CPU Code */
	ROM_LOAD16_BYTE( "cb_03.rom", 0x000000, 0x040000, CRC(bee20587) SHA1(3c1d546c63a3d6f8a63b7dee1c8e99a7091d774d) )
	ROM_LOAD16_BYTE( "cb_02.rom", 0x000001, 0x040000, CRC(2ed14c50) SHA1(4ed01ea5c5e59c3c012d9a4d5257be78220758c1) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "cb_08.rom", 0x000000, 0x010000, CRC(bf7b3558) SHA1(6046b965d61560e0227437f00f1ff1f7dbc16232) )
	ROM_LOAD16_BYTE( "cb_07.rom", 0x000001, 0x010000, CRC(85d219d7) SHA1(a9628efc5eddefad739363ff0b2f37a2d095df86) )

	ROM_REGION( 0x1000, "mcu", 0 ) /* MCU Internal Code, m50747? */
	ROM_LOAD( "cybattlr.mcu", 0x000000, 0x1000, NO_DUMP )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "cb_m01.rom", 0x000000, 0x080000, CRC(1109337f) SHA1(ab294d87c9b4eb54401da5ad6ea171e4c0a700b5) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "cb_m04.rom", 0x000000, 0x080000, CRC(0c91798e) SHA1(63747adcf24146fdb0f59bd4dfd6ac6300eeafc3) )

	ROM_REGION( 0x020000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "cb_09.rom",  0x000000, 0x020000, CRC(37b1f195) SHA1(06be0e2ec2649e82183925554a5025c1c7a09137) )

	ROM_REGION( 0x100000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "cb_m03.rom", 0x000000, 0x080000, CRC(4cd49f58) SHA1(a455a27edec8b6f92c64636a10624ab5ccefa5e0) )
	ROM_LOAD( "cb_m02.rom", 0x080000, 0x080000, CRC(882825db) SHA1(06ab0f9ee60614ce22d32b27ab28fcaa0d8de66f) )

	ROM_REGION( 0x040000, "oki1", 0 )       /* Samples */
	ROM_LOAD( "cb_11.rom", 0x000000, 0x040000, CRC(59d62d1f) SHA1(48363df066e7967b28887253108dc8cb124637f4) )

	ROM_REGION( 0x040000, "oki2", 0 )       /* Samples */
	ROM_LOAD( "cb_10.rom", 0x000000, 0x040000, CRC(8af95eed) SHA1(42949d67986303370f3b663106f278f828955a4b) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM */
	ROM_LOAD( "pr-91028.12",  0x0000, 0x0200, CRC(cfe90082) SHA1(b59991ec7b3e83ba645b709547e5b4cbe03c0f11) )
ROM_END


/***************************************************************************

                         [ Earth Defense Force ]

 2 TMP68000
 1 YM2151
 1 OSC 12Mhz
 1 OSC 7Mhz
 1 OSC 8Mhz

interrupts: 2,3]    543C>   move.w  $e0000.l,   $60da6.l
                            move.w  #$ffff,     $60da8.l
            4,5,6]  5928 +  move.w  #$ffff,     $60010.l

89e         (a7)+ -> 44000.w & 6000e.w
8cc         (a7)+ -> 44204.w ; 4420c.w ; 4400c.w
fc0         (a7)+ -> 58000 (string)

616f4.w     *** lives ***
60d8a.w     *** level(1..) ***

***************************************************************************/


ROM_START( edf )
	ROM_REGION( 0xc0000, "maincpu", 0 )     /* Main CPU Code: 00000-3ffff & 80000-bffff */
	ROM_LOAD16_BYTE( "edf5.b5",  0x000000, 0x020000, CRC(105094d1) SHA1(e962164836756bc20c2b5dc0032042a0219e82d8) )
	ROM_CONTINUE (               0x080000, 0x020000 )
	ROM_LOAD16_BYTE( "edf_06.rom",  0x000001, 0x020000, CRC(94da2f0c) SHA1(ae6aef03d61d244a857a9dc824be230c35f4c978) )
	ROM_CONTINUE (                  0x080001, 0x020000 )

	ROM_REGION( 0x40000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "edf1.f5",  0x000000, 0x020000, CRC(2290ea19) SHA1(64c9394bd4d5569d68833d2e57abaf2f1af5be97) )
	ROM_LOAD16_BYTE( "edf2.f3",  0x000001, 0x020000, CRC(ce93643e) SHA1(686bf0ec104af8c97624a782e0d60afe170fd945) )

	ROM_REGION( 0x1000, "mcu", 0 ) /* MCU Internal Code, 64 pin DIP surface scratched, m50747? */
	ROM_LOAD( "edf.mcu", 0x000000, 0x1000, NO_DUMP )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "edf_m04.rom",  0x000000, 0x080000, CRC(6744f406) SHA1(3b8f13ca968456186d9ad61f34611b7eab62ea86) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "edf_m05.rom",  0x000000, 0x080000, CRC(6f47e456) SHA1(823baa9dc4cb2425c64e9332c6ed4678e49d0c7b) )

	ROM_REGION( 0x020000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "edf_09.rom",   0x000000, 0x020000, CRC(96e38983) SHA1(a4fb94f15d9a9f7df1645be66fe3e179d0ebf765) )

	ROM_REGION( 0x080000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "edf_m03.rom",  0x000000, 0x080000, CRC(ef469449) SHA1(bc591e56c5478383eb4bd29f16133c6ba407c22f) )

	ROM_REGION( 0x040000, "oki1", 0 )       /* Samples */
	ROM_LOAD( "edf_m02.rom",  0x000000, 0x040000, CRC(fc4281d2) SHA1(67ea324ff359a5d9e7538c08865b5eeebd16704b) )

	ROM_REGION( 0x040000, "oki2", 0 )       /* Samples */
	ROM_LOAD( "edf_m01.rom",  0x000000, 0x040000, CRC(9149286b) SHA1(f6c66c5cd50b72c4d401a263c65a8d4ef8cf9221) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM  (N82S131N compatible type PROM) */
	ROM_LOAD( "rd.20n",    0x0000, 0x0200, CRC(1d877538) SHA1(a5be0dc65dcfc36fbba10d1fddbe155e24b6122f) )
ROM_END


ROM_START( edfa )
	ROM_REGION( 0xc0000, "maincpu", 0 )     /* Main CPU Code: 00000-3ffff & 80000-bffff */
	ROM_LOAD16_BYTE( "5.b5", 0x00000, 0x20000, CRC(6edd3c53) SHA1(53fd42f417be7ca57bd941abe343e2730a7b3ba9) )
	ROM_CONTINUE (           0x80000, 0x20000 )
	ROM_LOAD16_BYTE( "6.b3", 0x00001, 0x20000, CRC(4d8bfa8f) SHA1(9d61f035e7c73a26b5de5380030c511eebeb7ece) )
	ROM_CONTINUE (           0x80001, 0x20000 )

	// rest from edf:

	ROM_REGION( 0x40000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "edf1.f5",  0x000000, 0x020000, CRC(2290ea19) SHA1(64c9394bd4d5569d68833d2e57abaf2f1af5be97) )
	ROM_LOAD16_BYTE( "edf2.f3",  0x000001, 0x020000, CRC(ce93643e) SHA1(686bf0ec104af8c97624a782e0d60afe170fd945) )

	ROM_REGION( 0x1000, "mcu", 0 ) /* MCU Internal Code, 64 pin DIP surface scratched, m50747? */
	ROM_LOAD( "edf.mcu", 0x000000, 0x1000, NO_DUMP )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "edf_m04.rom",  0x000000, 0x080000, CRC(6744f406) SHA1(3b8f13ca968456186d9ad61f34611b7eab62ea86) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "edf_m05.rom",  0x000000, 0x080000, CRC(6f47e456) SHA1(823baa9dc4cb2425c64e9332c6ed4678e49d0c7b) )

	ROM_REGION( 0x020000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "edf_09.rom",   0x000000, 0x020000, CRC(96e38983) SHA1(a4fb94f15d9a9f7df1645be66fe3e179d0ebf765) )

	ROM_REGION( 0x080000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "edf_m03.rom",  0x000000, 0x080000, CRC(ef469449) SHA1(bc591e56c5478383eb4bd29f16133c6ba407c22f) )

	ROM_REGION( 0x040000, "oki1", 0 )       /* Samples */
	ROM_LOAD( "edf_m02.rom",  0x000000, 0x040000, CRC(fc4281d2) SHA1(67ea324ff359a5d9e7538c08865b5eeebd16704b) )

	ROM_REGION( 0x040000, "oki2", 0 )       /* Samples */
	ROM_LOAD( "edf_m01.rom",  0x000000, 0x040000, CRC(9149286b) SHA1(f6c66c5cd50b72c4d401a263c65a8d4ef8cf9221) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM  (N82S131N compatible type PROM) */
	ROM_LOAD( "rd.20n",    0x0000, 0x0200, CRC(1d877538) SHA1(a5be0dc65dcfc36fbba10d1fddbe155e24b6122f) )
ROM_END


ROM_START( edfu )
	ROM_REGION( 0xc0000, "maincpu", 0 )     /* Main CPU Code: 00000-3ffff & 80000-bffff */
	ROM_LOAD16_BYTE( "edf5.b5",  0x000000, 0x020000, CRC(105094d1) SHA1(e962164836756bc20c2b5dc0032042a0219e82d8) )
	ROM_CONTINUE (               0x080000, 0x020000 )
	ROM_LOAD16_BYTE( "edf6.b3",  0x000001, 0x020000, CRC(4797de97) SHA1(dcfcc376a49853c938d772808efe421ba4ba24da) )
	ROM_CONTINUE (               0x080001, 0x020000 )

	// rest from edf:

	ROM_REGION( 0x40000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "edf1.f5",  0x000000, 0x020000, CRC(2290ea19) SHA1(64c9394bd4d5569d68833d2e57abaf2f1af5be97) )
	ROM_LOAD16_BYTE( "edf2.f3",  0x000001, 0x020000, CRC(ce93643e) SHA1(686bf0ec104af8c97624a782e0d60afe170fd945) )

	ROM_REGION( 0x1000, "mcu", 0 ) /* MCU Internal Code, 64 pin DIP surface scratched, m50747? */
	ROM_LOAD( "edf.mcu", 0x000000, 0x1000, NO_DUMP )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "edf_m04.rom",  0x000000, 0x080000, CRC(6744f406) SHA1(3b8f13ca968456186d9ad61f34611b7eab62ea86) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "edf_m05.rom",  0x000000, 0x080000, CRC(6f47e456) SHA1(823baa9dc4cb2425c64e9332c6ed4678e49d0c7b) )

	ROM_REGION( 0x020000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "edf_09.rom",   0x000000, 0x020000, CRC(96e38983) SHA1(a4fb94f15d9a9f7df1645be66fe3e179d0ebf765) )

	ROM_REGION( 0x080000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "edf_m03.rom",  0x000000, 0x080000, CRC(ef469449) SHA1(bc591e56c5478383eb4bd29f16133c6ba407c22f) )

	ROM_REGION( 0x040000, "oki1", 0 )       /* Samples */
	ROM_LOAD( "edf_m02.rom",  0x000000, 0x040000, CRC(fc4281d2) SHA1(67ea324ff359a5d9e7538c08865b5eeebd16704b) )

	ROM_REGION( 0x040000, "oki2", 0 )       /* Samples */
	ROM_LOAD( "edf_m01.rom",  0x000000, 0x040000, CRC(9149286b) SHA1(f6c66c5cd50b72c4d401a263c65a8d4ef8cf9221) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM  (N82S131N compatible type PROM) */
	ROM_LOAD( "rd.20n",    0x0000, 0x0200, CRC(1d877538) SHA1(a5be0dc65dcfc36fbba10d1fddbe155e24b6122f) )
ROM_END


ROM_START( edfbl )
	ROM_REGION( 0xc0000, "maincpu", 0 )     /* Main CPU Code: 00000-3ffff & 80000-bffff */
	ROM_LOAD16_BYTE( "02.bin",  0x000000, 0x020000, CRC(19a0dfa0) SHA1(acd020fa42de9cd98e51fe92377a46846859797b) )
	ROM_CONTINUE (               0x080000, 0x020000 )
	ROM_LOAD16_BYTE( "01.bin",  0x000001, 0x020000, CRC(fc893ad0) SHA1(6d7be560e2343f3943f52ccdae7bd255b7720b6e) )
	ROM_CONTINUE (                  0x080001, 0x020000 )

	/* no 2nd 68k on this bootleg, is there a PIC? */

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "07.bin",  0x000000, 0x040000, CRC(4495c228) SHA1(2193561e193e696c66f27fa186f27ffbbdcb1826) )
	ROM_LOAD( "06.bin",  0x040000, 0x040000, CRC(3e37f226) SHA1(b789c1d2159f54d7464239e111bd729e0582b89b) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "03.bin",  0x000000, 0x040000, CRC(eea24345) SHA1(1ed690eb62b28cf6bbcb6fec7e8e39daaa340af3) )
	ROM_LOAD( "04.bin",  0x040000, 0x040000, CRC(2cfe9439) SHA1(c953f1cf16be444eef3dc389305733ac351559b6) )

	ROM_REGION( 0x020000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "05.bin",   0x000000, 0x020000, CRC(96e38983) SHA1(a4fb94f15d9a9f7df1645be66fe3e179d0ebf765) )

	ROM_REGION( 0x080000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "09.bin",  0x000000, 0x040000, CRC(e89d27c0) SHA1(b95d7988f13c578f501dc6cf2a5109dbef2a4d6c) )
	ROM_LOAD( "08.bin",  0x040000, 0x040000, CRC(603ac969) SHA1(193144080d2cb5536980e5f0f7173fba470ab79f) )

	ROM_REGION( 0x040000, "oki1", 0 ) /* Samples - non-banked sfx? */
	ROM_LOAD( "12.bin",  0x000000, 0x010000, CRC(e645f447) SHA1(5de3acc32a2211995ed1e9b4577063124b0db45a) )

	ROM_REGION( 0x080000, "okibanks", 0 ) /* Samples - banked music? */
	ROM_LOAD( "11.bin",  0x000000, 0x040000, CRC(5a8896cb) SHA1(ffa529acc1842868d51c22acf2b6b5a6aa1a79b2) )
	ROM_LOAD( "10.bin",  0x040000, 0x040000, CRC(baa7c91b) SHA1(1f4d240a4059fad1d09d624275dfe2dffe950a47) )

	ROM_REGION( 0x0200, "proms", 0 ) /* the bootleg has an 82s131 prom like the original, but it isn't confirmed to be the same yet */
	ROM_LOAD( "rd.20n",    0x0000, 0x0200, CRC(1d877538) SHA1(a5be0dc65dcfc36fbba10d1fddbe155e24b6122f) )
ROM_END


/***************************************************************************

                                [ Hachoo! ]

 2 Mc68000
 1 OSC 12Mhz
 1 OSC 7Mhz
 1 OSC 4Mhz
 1 YM2151
 1 YM3012

***************************************************************************/


ROM_START( hachoo )
	ROM_REGION( 0x60000, "maincpu", 0 )     /* Main CPU Code */
	ROM_LOAD16_BYTE( "hacho02.rom", 0x000000, 0x020000, CRC(49489c27) SHA1(21c31e1b41ca6c7e78803e5a2e7c49f7b885d0e3) )
	ROM_LOAD16_BYTE( "hacho01.rom", 0x000001, 0x020000, CRC(97fc9515) SHA1(192660061af6a5bddccf7cfffcbfa368c4030de9) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "hacho05.rom", 0x000000, 0x010000, CRC(6271f74f) SHA1(2fe0f8adf3cdafe13a9107c36f24f1a525d06a05) )
	ROM_LOAD16_BYTE( "hacho06.rom", 0x000001, 0x010000, CRC(db9e743c) SHA1(77a3691b48eed389bfcdead5f307415dce47247e) )

	ROM_REGION( 0x1000, "mcu", 0 ) /* M50747 MCU Code */
	ROM_LOAD( "m50747", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "hacho14.rom", 0x000000, 0x080000, CRC(10188483) SHA1(43bf08ac777c42351b04e2c35b1a119f524b4388) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "hacho15.rom", 0x000000, 0x020000, CRC(e559347e) SHA1(1d71c83f4946af80083bbd059e55c2d57f2f9647) )
	ROM_LOAD( "hacho16.rom", 0x020000, 0x020000, CRC(105fd8b5) SHA1(41aafcf6e29417a39ca0945f47a90646da2cbf3c) )
	ROM_LOAD( "hacho17.rom", 0x040000, 0x020000, CRC(77f46174) SHA1(81d923069191c153773aaeb2d0eab6ab0076a386) )
	ROM_LOAD( "hacho18.rom", 0x060000, 0x020000, CRC(0be21111) SHA1(45beb7e9f6cfe56893e0c5b052a1922e3d73275b) )

	ROM_REGION( 0x020000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "hacho19.rom", 0x000000, 0x020000, CRC(33bc9de3) SHA1(8bbfda0fea742177e00dd5fff226f85233537cb3) )

	ROM_REGION( 0x080000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "hacho20.rom", 0x000000, 0x020000, CRC(2ae2011e) SHA1(f294ebfd87816c7b179fcaba3869e3402b2560a9) )
	ROM_LOAD( "hacho21.rom", 0x020000, 0x020000, CRC(6dcfb8d5) SHA1(a478fea81acf1f317fe82ec84d4d21227db7432b) )
	ROM_LOAD( "hacho22.rom", 0x040000, 0x020000, CRC(ccabf0e0) SHA1(3b9d95d8dee6155b484d85cc3f12e20a8ae3c9be) )
	ROM_LOAD( "hacho23.rom", 0x060000, 0x020000, CRC(ff5f77aa) SHA1(e9fc71ac3499ee5b4636a3bdf1f3fbbe2623b0db) )

	ROM_REGION( 0x040000, "oki1", 0 )       /* Samples */
	ROM_LOAD( "hacho09.rom", 0x000000, 0x020000, CRC(e9f35c90) SHA1(1a1dd6a7777bbad1475ad65f8797818c9b4f0937) )
	ROM_LOAD( "hacho10.rom", 0x020000, 0x020000, CRC(1aeaa188) SHA1(40827435c948a2fd448137eb3f8c33fc84da3b82) )

	ROM_REGION( 0x040000, "oki2", 0 )       /* Samples */
	ROM_LOAD( "hacho07.rom", 0x000000, 0x020000, CRC(06e6ca7f) SHA1(a15a1b754b0d47285a023ecfc4b762ab592f8262) )
	ROM_LOAD( "hacho08.rom", 0x020000, 0x020000, CRC(888a6df1) SHA1(71d70633ecf7255287e55e92f8d2f186fe58f4b4) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM */
	ROM_LOAD( "ht.bin",      0x0000, 0x0200, CRC(85302b15) SHA1(8184c1184a71706cdb981e3c4f90a08521413e72) )
ROM_END


/***************************************************************************

Hayaoshi Quiz Ouza Ketteisen
(c)1991 Jaleco

JALECO ED9075
EB90004-20027

CPU: HD68000PS8 x2
MCU: M50747? (labeled "MO-91044")
Sound: YM2151 YM3012 M6295x2
Custom: GS-9000401 (44pin QFP)
        GS-9000403 (44pin QFP, x2)
        GS-9000404 (44pin QFP)
        GS-9000405 (80pin QFP, x3)
        GS-9000406 (80pin QFP, x3)
        GS-9000407 (80pin QFP)

ROMs:
1 - near 68000 (actual label is ???????N?C?Y[1])
2 /            (actual label is ???????N?C?Y[2])

3 - near 6295 (actual label is ???????N?C?Y[3])
4 /           (actual label is ???????N?C?Y[4])

5 - near 68000 (actual label is ???????N?C?Y[5] Ver1.1)
6 /            (actual label is ???????N?C?Y[6] Ver1.1)

7  - near customs (actual label is ???????N?C?Y[7])
8  |              (actual label is ???????N?C?Y[8])
9  |              (actual label is ???????N?C?Y[9])
10 /              (actual label is ???????N?C?Y[10])

PR-91044 (82S131N)

***************************************************************************/


ROM_START( hayaosi1 )
	ROM_REGION( 0xc0000, "maincpu", 0 )     /* Main CPU Code: 00000-3ffff & 80000-bffff */
	ROM_LOAD16_BYTE( "5", 0x000000, 0x020000, CRC(eaf38fab) SHA1(0f9cd6e674668a86d2bb54228b50217c934e96af) )
	ROM_CONTINUE (                  0x080000, 0x020000 )
	ROM_LOAD16_BYTE( "6", 0x000001, 0x020000, CRC(341f8057) SHA1(958d9fc870bc13a9c1720d21776b5239db771ce2) )
	ROM_CONTINUE (                  0x080001, 0x020000 )

	ROM_REGION( 0x40000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "1", 0x00000, 0x20000, CRC(b088b27e) SHA1(198e2520ce4f9b19ea108e09ff00f7e27768f290) )
	ROM_LOAD16_BYTE( "2", 0x00001, 0x20000, CRC(cebc7b16) SHA1(18b166560ffff7c43cec3d52e4b2da79256dfb2e) )

	ROM_REGION( 0x1000, "mcu", 0 ) /* MCU Internal Code, M50747 */
	ROM_LOAD( "mo-91044.mcu", 0x000000, 0x1000, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_ERASEFF) /* Scroll 0 */
	ROM_LOAD( "7", 0x000000, 0x80000, CRC(3629c455) SHA1(c216b600750861b073062c165f36e6949db10d78) )

	ROM_REGION( 0x80000, "gfx2", ROMREGION_ERASEFF ) /* Scroll 1 */
	ROM_LOAD( "8", 0x000000, 0x80000, CRC(15f0b2a3) SHA1(48080de7818bd1c4ac6a7cd81aa86b69bdda2668) )

	ROM_REGION( 0x20000, "gfx3", ROMREGION_ERASEFF ) /* Scroll 2 */
	ROM_LOAD( "9",  0x000000, 0x20000, CRC(64d5b95e) SHA1(793714b2b049afd1cb66c888545cb8379c702010) )

	ROM_REGION( 0x80000, "gfx4", ROMREGION_ERASEFF ) /* Sprites */
	ROM_LOAD( "10", 0x000000, 0x80000, CRC(593e93d6) SHA1(db449b45301e3f7c26e0dfe1f4cf8293ae7dfdaa) )

	ROM_REGION( 0x40000, "oki1", 0 )        /* Samples */
	ROM_LOAD( "3", 0x000000, 0x40000, CRC(f3f5787a) SHA1(5e0416726de7b78583c9e1eb7944a41d307a9308) )

	ROM_REGION( 0x40000, "oki2", 0 )        /* Samples */
	ROM_LOAD( "4", 0x000000, 0x40000, CRC(ac3f9bd2) SHA1(7856f40daa30de9077e68a5ea977ec39c044c2f8) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM  (N82S131N compatible type BPROM) */
	ROM_LOAD( "pr-91044",  0x0000, 0x0200, CRC(c69423d6) SHA1(ba9644a9899df2d73a5a16bf7ceef1954c2e25f3) )
ROM_END


/***************************************************************************

                    [ Ninja Kazan ] \ [ Iga Ninjyutsuden ]

interrupts: 1] 420(does nothing)
            2] 500
            3] 410(it doesn't save registers on the stack!!)

f0004.l     *** hi score (BCD) ***
f000c.l     *** score (BCD) ***
f002a.w     *** lives ***
f010c.w     credits


***************************************************************************/


ROM_START( kazan )
	ROM_REGION( 0x60000, "maincpu", 0 )     /* Main CPU Code */
	ROM_LOAD16_BYTE( "kazan.2",    0x000000, 0x020000, CRC(072aa3d6) SHA1(49fd03d72f647dcda140d0a507f23a80911427e1) )
	ROM_LOAD16_BYTE( "kazan.1",    0x000001, 0x020000, CRC(b9801e2d) SHA1(72f0ca6da5177625073ee2687ddba3647af5e9e8) )
	ROM_LOAD16_BYTE( "iga_03.bin", 0x040000, 0x010000, CRC(de5937ad) SHA1(d3039e5391feb925ea10f33a1363bf3ffc1ebb3d) )
	ROM_LOAD16_BYTE( "iga_04.bin", 0x040001, 0x010000, CRC(afaf0480) SHA1(b8d0ec859a94941650bdd2b01e98d054d49fef67) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "iga_05.bin", 0x000000, 0x010000, CRC(13580868) SHA1(bfcd11b294b64af81a0403a3e9370c42a9859b6b) )
	ROM_LOAD16_BYTE( "iga_06.bin", 0x000001, 0x010000, CRC(7904d5dd) SHA1(4cd9fdab601a90c997a041a9f7966a9a233e897b) )

	ROM_REGION( 0x1000, "mcu", 0 ) /* M50747 MCU Code */
	ROM_LOAD( "m50747", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "kazan.11", 0x000000, 0x020000, CRC(08e54137) SHA1(1e3298a896ae0de64f0fc2dab6b32c8bf875f50b) )
	ROM_LOAD( "kazan.12", 0x020000, 0x020000, CRC(e89d58bd) SHA1(a4f2530fb544af48f66b3402c5162639745ab11d) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "kazan.15", 0x000000, 0x020000, CRC(48b28aa9) SHA1(9430f5dd8c6b75e59f0a5ae933c645a07a56d183) )
	ROM_LOAD( "kazan.16", 0x020000, 0x020000, CRC(07eab526) SHA1(97f6898a7992e9606c78c01a09102b3080146013) )
	ROM_LOAD( "kazan.17", 0x040000, 0x020000, CRC(617269ea) SHA1(93c62d4ce01add4eec1d392a0b25ab6d60d9788d) )
	ROM_LOAD( "kazan.18", 0x060000, 0x020000, CRC(52fc1b4b) SHA1(42d1971d35e8d91631a2b6b883dcee975cf9fbca) )

	ROM_REGION( 0x020000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "kazan.19", 0x000000, 0x010000, CRC(b3a9a4ae) SHA1(bccef0f6ea17c2f0f8d61da4d174389084252d13) )

	ROM_REGION( 0x080000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "kazan.20", 0x000000, 0x020000, CRC(ee5819d8) SHA1(44be00a64c42d724e3c3c5e48cbb5144b7c7c13f) )
	ROM_LOAD( "kazan.21", 0x020000, 0x020000, CRC(abf14d39) SHA1(6c84498e7ace56947b04b46341b2ab9b4aea5bb8) )
	ROM_LOAD( "kazan.22", 0x040000, 0x020000, CRC(646933c4) SHA1(583094c6969de95f70f88901f3ef2c279b467334) )
	ROM_LOAD( "kazan.23", 0x060000, 0x020000, CRC(0b531aee) SHA1(7aa97ada48e8a99bd2345efe41c45b82cb2d48e2) )

	ROM_REGION( 0x040000, "oki1", 0 )       /* Samples */
	ROM_LOAD( "kazan.9",  0x000000, 0x020000, CRC(5c28bd2d) SHA1(95d70a30118dfd2649f8d1f726a89e61233b4ae1) )
	ROM_LOAD( "kazan.10", 0x020000, 0x010000, CRC(cd6c7978) SHA1(efbf20eebeea67e8ace385b508372bf70b6ac8bc) )

	ROM_REGION( 0x040000, "oki2", 0 )       /* Samples */
	ROM_LOAD( "kazan.7",  0x000000, 0x020000, CRC(42f228f8) SHA1(6bef1269da5f4bdc56f6a37fff423f71450ac49c) )
	ROM_LOAD( "kazan.8",  0x020000, 0x020000, CRC(ebd1c883) SHA1(36cb08b7ce29326ae1694d8c7088408cdf399f27) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM */
	ROM_LOAD( "kazan.14m",    0x0000, 0x0200, CRC(85b30ac4) SHA1(b03f577ceb0f26b67453ffa52ef61fea76a93184) )
ROM_END


ROM_START( iganinju )
	ROM_REGION( 0x60000, "maincpu", 0 )     /* Main CPU Code */
	ROM_LOAD16_BYTE( "iga_02.bin", 0x000000, 0x020000, CRC(bd00c280) SHA1(d4e074bb25fc7295b1a39aa22e966cf471a6789f) )
	ROM_LOAD16_BYTE( "iga_01.bin", 0x000001, 0x020000, CRC(fa416a9e) SHA1(c81405037366c93754d8eed1c70128091f9b3e3f) )
	ROM_LOAD16_BYTE( "iga_03.bin", 0x040000, 0x010000, CRC(de5937ad) SHA1(d3039e5391feb925ea10f33a1363bf3ffc1ebb3d) )
	ROM_LOAD16_BYTE( "iga_04.bin", 0x040001, 0x010000, CRC(afaf0480) SHA1(b8d0ec859a94941650bdd2b01e98d054d49fef67) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "iga_05.bin", 0x000000, 0x010000, CRC(13580868) SHA1(bfcd11b294b64af81a0403a3e9370c42a9859b6b) )
	ROM_LOAD16_BYTE( "iga_06.bin", 0x000001, 0x010000, CRC(7904d5dd) SHA1(4cd9fdab601a90c997a041a9f7966a9a233e897b) )

	ROM_REGION( 0x1000, "mcu", 0 ) /* M50747 MCU Code */
	ROM_LOAD( "m50747", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "iga_14.bin", 0x000000, 0x040000, CRC(c707d513) SHA1(b0067a444385809a7dfd11fea27b1add318d5225) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "iga_18.bin", 0x000000, 0x080000, CRC(6c727519) SHA1(5a05f82ff6fe2835f72607be52290b6ae32640c8) )

	ROM_REGION( 0x020000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "iga_19.bin", 0x000000, 0x020000, CRC(98a7e998) SHA1(603ec7696cf50f873265a0956bc01aa2cf6448f7) )

	ROM_REGION( 0x080000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "iga_23.bin", 0x000000, 0x080000, CRC(fb58c5f4) SHA1(530e32dbe46dfe7d19fc48b77c1544679d40ff59) )

	ROM_REGION( 0x040000, "oki1", 0 )       /* Samples */
	ROM_LOAD( "iga_10.bin", 0x000000, 0x040000, CRC(67a89e0d) SHA1(9c15e1b5e15f3f95f302d7223828bed0d0173347) )

	ROM_REGION( 0x040000, "oki2", 0 )       /* Samples */
	ROM_LOAD( "iga_08.bin", 0x000000, 0x040000, CRC(857dbf60) SHA1(e700b307aa481a57180a4529e2ce4326574e128e) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM */
	ROM_LOAD( "iga.131",    0x0000, 0x0200, CRC(1d877538) SHA1(a5be0dc65dcfc36fbba10d1fddbe155e24b6122f) )
ROM_END


/***************************************************************************

                        [ Jitsuryoku!! Pro Yakyuu ]

(JPN Ver.)
(c)1989 Jaleco
Mega-System
MB-8842
A-Type
CPU  :TMP68000P-8 x2
Sound:YM2151,YM3012
OSC  :12.000MHz,7.000MHz

Sub
MB-M02A (EB-88003-3001-1)
Sound:OKI M6295
OSC  :4.000MHz
Other:JALECO GS-88000

BS.BPR       [85b30ac4] (82S131)

***************************************************************************/


ROM_START( jitsupro )
	ROM_REGION( 0x60000, "maincpu", 0 )     /* Main CPU Code */
	ROM_LOAD16_BYTE( "jp_2.bin", 0x000000, 0x020000, CRC(5d842ff2) SHA1(69032601c0e67c5c78fad1cb2bb4f1b59014fe5a) )
	ROM_LOAD16_BYTE( "jp_1.bin", 0x000001, 0x020000, CRC(0056edec) SHA1(529a5181f7d791930e238bc115daeae1ab9a63ad) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "jp_5.bin", 0x000000, 0x010000, CRC(84454e9e) SHA1(a506d44349a670e57d9dba3ec6a9de2597ba2cdb) ) // 11xxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "jp_6.bin", 0x000001, 0x010000, CRC(1fa9b75b) SHA1(d0e3640333f737658542ed4a8758d62f6d64ae05) ) // 11xxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x1000, "mcu", 0 ) /* M50747 MCU Code */
	ROM_LOAD( "m50747", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "jp_14.bin", 0x000000, 0x080000, CRC(db112abf) SHA1(fd8c510934241b7923660acca6122ca3e63bf934) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "jp_18.bin", 0x000000, 0x080000, CRC(3ed855e3) SHA1(c68fffe42aa134480fce37d8d7e0aa336d97f898) )

	ROM_REGION( 0x020000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "jp_19.bin", 0x000000, 0x020000, CRC(ff59111f) SHA1(caf78b5657c1b1f99b0de440862618d3d961ea18) )

	ROM_REGION( 0x080000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "jp_23.bin", 0x000000, 0x080000, CRC(275f48bd) SHA1(449c19247d4956f5eecdd5352e24e31685bd448d) )

	ROM_REGION( 0x040000, "oki1", 0 )       /* Samples */
	ROM_LOAD( "jp_10.bin", 0x000000, 0x040000, CRC(178e43c0) SHA1(9c3d5a10f0f7a9d3f2d5dfaba6495d5cd8e94c4d) )   // FIRST AND SECOND HALF IDENTICAL
	ROM_CONTINUE(          0x000000, 0x040000             )

	ROM_REGION( 0x040000, "oki2", 0 )       /* Samples */
	ROM_LOAD( "jp_8.bin",  0x000000, 0x040000, CRC(eca67632) SHA1(9f91081a26bd98fd79d5ddc6413b8a32006bb05f) )   // FIRST AND SECOND HALF IDENTICAL
	ROM_CONTINUE(          0x000000, 0x040000             )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM  (N82S131N compatible type BPROM) */
	ROM_LOAD( "bs.bpr",    0x0000, 0x0200, CRC(85b30ac4) SHA1(b03f577ceb0f26b67453ffa52ef61fea76a93184) )
ROM_END


/***************************************************************************

                            [ Kick Off ]

 2 TMP68000
 1 YM2151
 1 OSC 7Mhz
 1 OSC 4Mhz
 1 OSC 12Mhz

WARNING: The sound CPU writes and read in the 9000-ffff area

interrupts: 1-2]    rte
            3]      timer
            4-7]    loop forever

f0128/a.w   *** Time (minutes/seconds BCD) ***
f012c/e.w   *** Goals (P1/P2) ***

Notes:
    * Coin B and Test are ignored
    * The alternate control method (selectable through a DSW)
      isn't implemented: the program tests the low 4 bits of
      the joystick inputs ($80002, $80004) but not the buttons.
      I can't get the players to move
    * Pressing P1 or P2 Start while the game boots pops up
      a rudimental sprites or tiles browser

***************************************************************************/


ROM_START( kickoff )
	ROM_REGION( 0x60000, "maincpu", 0 )     /* Main CPU Code */
	ROM_LOAD16_BYTE( "kioff03.rom", 0x000000, 0x010000, CRC(3b01be65) SHA1(110b4e02053073c0315aba1eca8c19afe5fafb33) )
	ROM_LOAD16_BYTE( "kioff01.rom", 0x000001, 0x010000, CRC(ae6e68a1) SHA1(aac54e13dd33420712a869e6f46fb9b94fde9e34) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "kioff09.rom", 0x000000, 0x010000, CRC(1770e980) SHA1(0c9dd30765432c64bc6c320c0948c471b52ae084) )
	ROM_LOAD16_BYTE( "kioff19.rom", 0x000001, 0x010000, CRC(1b03bbe4) SHA1(ef778712c293af15bda37f0425892023747ec479) )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "kioff05.rom", 0x000000, 0x020000, CRC(e7232103) SHA1(4bb72fb835ab491cf5b58a34af4e2a767703320c) )
	ROM_LOAD( "kioff06.rom", 0x020000, 0x020000, CRC(a0b3cb75) SHA1(4840177d84e825c39e2e8252c75f0c1aab156b19) )
	ROM_LOAD( "kioff07.rom", 0x040000, 0x020000, CRC(ed649919) SHA1(e8955c0dc2d1546d875a16fc9d8595ed4a507539) )
	ROM_LOAD( "kioff10.rom", 0x060000, 0x020000, CRC(fd739fec) SHA1(1442d5ef7b8fbaa0c9f71c12ce993626364d2e1a) )

	ROM_REGION( 0x080000, "gfx2", ROMREGION_ERASEFF ) /* Scroll 1 */
	// scroll 1 is unused

	ROM_REGION( 0x020000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "kioff16.rom", 0x000000, 0x020000, CRC(22c46314) SHA1(e56161d4145042fc2524b12c31c5b99166c1019b) )

	ROM_REGION( 0x080000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "kioff27.rom", 0x000000, 0x020000, CRC(ca221ae2) SHA1(77ba20536620949f3a172205f4d7275c9771a5a9) )
	ROM_LOAD( "kioff18.rom", 0x020000, 0x020000, CRC(d7909ada) SHA1(3bdbf21c2815f0242974e42d04c3b5e356ce583f) )
	ROM_LOAD( "kioff17.rom", 0x040000, 0x020000, CRC(f171559e) SHA1(9052f0aff07f0c4a013766bda1f9fffcfa682b29) )
	ROM_LOAD( "kioff26.rom", 0x060000, 0x020000, CRC(2a90df1b) SHA1(518d959d9557afa05da8d7bce590ff46bd6fe367) )

	ROM_REGION( 0x040000, "oki1", 0 )       /* Samples */
	ROM_LOAD( "kioff20.rom", 0x000000, 0x020000, CRC(5c28bd2d) SHA1(95d70a30118dfd2649f8d1f726a89e61233b4ae1) )
	ROM_LOAD( "kioff21.rom", 0x020000, 0x020000, CRC(195940cf) SHA1(5b1880a576046dae32cf1fd48cd4e8830649b7f7) )

	ROM_REGION( 0x040000, "oki2", 0 )       /* Samples */
	// same rom for 2 oki chips ?? Unlikely
	ROM_LOAD( "kioff20.rom", 0x000000, 0x020000, CRC(5c28bd2d) SHA1(95d70a30118dfd2649f8d1f726a89e61233b4ae1) )
	ROM_LOAD( "kioff21.rom", 0x020000, 0x020000, CRC(195940cf) SHA1(5b1880a576046dae32cf1fd48cd4e8830649b7f7) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM */
	ROM_LOAD( "kick.bin",    0x0000, 0x0200, CRC(85b30ac4) SHA1(b03f577ceb0f26b67453ffa52ef61fea76a93184) )
ROM_END


/***************************************************************************

                            [ Legend of Makai ]

 1 Tmp 68000
 1 Z80
 1 YM2203c
 1 Osc 5Mhz
 1 Osc 12Mhz

***************************************************************************/

ROM_START( lomakai )
	ROM_REGION( 0x60000, "maincpu", 0 )     /* Main CPU Code */
	ROM_LOAD16_BYTE( "lom_30.rom", 0x000000, 0x020000, CRC(ba6d65b8) SHA1(4c83e57c977b2be82a99a4a61ab8fd5f7099ae38) )
	ROM_LOAD16_BYTE( "lom_20.rom", 0x000001, 0x020000, CRC(56a00dc2) SHA1(5d97f89d384e12d70cbb5aabd6ce309e5cfb5497) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Sound CPU Code (Z80) */
	ROM_LOAD( "lom_01.rom",  0x0000, 0x10000, CRC(46e85e90) SHA1(905899346f7cd91e76d0e303258149c3d16604e0) )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "lom_05.rom", 0x000000, 0x020000, CRC(d04fc713) SHA1(b04bf71b93aa7fe5680c9ab1bf346416d75b511f) )

	ROM_REGION( 0x010000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "lom_08.rom", 0x000000, 0x010000, CRC(bdb15e67) SHA1(6f8e06d294cf7cfbbf77c4013a6e02a942300f72) )

	ROM_REGION( 0x020000, "gfx3", 0 ) /* Sprites */
	ROM_LOAD( "lom_06.rom", 0x000000, 0x020000, CRC(f33b6eed) SHA1(92da5b278c59bcbe4fdaf408bf1cfd8d6cafde85) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Unknown PROMs */
	ROM_LOAD( "makaiden.9",  0x0000, 0x0100, CRC(3567065d) SHA1(e111e40f9400512b3e088842d87462b00b450b8d) )
	ROM_LOAD( "makaiden.10", 0x0100, 0x0100, CRC(e6709c51) SHA1(f5cd4f0454c1a71a5b0006b098f9e76c2d8a27d2) )
ROM_END


ROM_START( makaiden )
	ROM_REGION( 0x60000, "maincpu", 0 )     /* Main CPU Code */
	ROM_LOAD16_BYTE( "makaiden.3a", 0x000000, 0x020000, CRC(87cf81d1) SHA1(c4410a86a01c683368dbc3daca61e21931885650) )
	ROM_LOAD16_BYTE( "makaiden.2a", 0x000001, 0x020000, CRC(d40e0fea) SHA1(0f8a0440f63f52508ab44c3a8eb5b7f03ccca49d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Sound CPU Code (Z80) */
	ROM_LOAD( "lom_01.rom",  0x0000, 0x10000, CRC(46e85e90) SHA1(905899346f7cd91e76d0e303258149c3d16604e0) )

	ROM_REGION( 0x020000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "lom_05.rom", 0x000000, 0x020000, CRC(d04fc713) SHA1(b04bf71b93aa7fe5680c9ab1bf346416d75b511f) )

	ROM_REGION( 0x010000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "makaiden.8", 0x000000, 0x010000, CRC(a7f623f9) SHA1(f893fcaedd9144823f3fa10706dd6cd8ac51cdb3) )

	ROM_REGION( 0x020000, "gfx3", 0 ) /* Sprites */
	ROM_LOAD( "lom_06.rom", 0x000000, 0x020000, CRC(f33b6eed) SHA1(92da5b278c59bcbe4fdaf408bf1cfd8d6cafde85) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Unknown PROMs */
	ROM_LOAD( "makaiden.9",  0x0000, 0x0100, CRC(3567065d) SHA1(e111e40f9400512b3e088842d87462b00b450b8d) )
	ROM_LOAD( "makaiden.10", 0x0100, 0x0100, CRC(e6709c51) SHA1(f5cd4f0454c1a71a5b0006b098f9e76c2d8a27d2) )
ROM_END


/***************************************************************************

                             [ P - 47 ]

(Japan version)
interrupts: 1]  53e     2] 540

517a        print word string: (a6)+,(a5)+$40. FFFF ends
5dbc        print string(s) to (a1)+$40: a6-> len.b,x.b,y.b,(chars.b)*
726a        prints screen
7300        ram test
7558        ip test
75e6(7638 loop) sound test
    84300.w     <-f1002.w   ?portrait F/F on(0x0100)/off(0x0000)
    84308.w     <-f1004.w   sound code

7736(7eb4 loop) scroll 0 test
    9809c       color
    980a0       hscroll
    980a4       vscroll
    980a8       charsize

    7e1e        prepare screen
    7e84        get user input
    7faa        vhscroll
    80ce        print value.l from a0

785c(78b8 loop) obj check 1     84000.w <-0x0E  84100.w <-0x101
    9804c   size
    98050   number      (0e.w bit 11-0)
    98054   color code  (08.w bit 2-0)
    98058   H flip      (08.w bit 6)
    9805c   V flip      (08.w bit 7)
    98060   priority    (08.w bit 3)
    98064   mosaic      (08.w bit 11-8)
    98068   mosaic sol. (08.w bit 12)

7afe(7cfe loop) obj check 2     84000.w <-0x0f  84100.w <-0x00
    9804a   obj num (a4-8e000)/8
    9804e   H-rev   a4+02.w
    98052   V-rev   a4+04.w
    98056   CG-rev  a4+06.w
    9805a   Rem.Eff bit   4 of 84100
    98060   Rem.Num bit 3-0 of 84100 (see 7dd4)

TRAP#2      pause?
f0104.w     *** initial lives ***
f002a/116.w <-!80000
f0810.w     <-!80002
f0c00.w     <-!80004
f0018.w     *** level ***


***************************************************************************/

ROM_START( p47 )
	ROM_REGION( 0x60000, "maincpu", 0 )     /* Main CPU Code */
	ROM_LOAD16_BYTE( "p47us3.bin", 0x000000, 0x020000, CRC(022e58b8) SHA1(87db59e409977358d9a7b689f2d69bef056328d9) )
	ROM_LOAD16_BYTE( "p47us1.bin", 0x000001, 0x020000, CRC(ed926bd8) SHA1(5cf3e7b9b23667eaa8ebcff0803a7b881c7b83cf) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "p47j_9.bin",  0x000000, 0x010000, CRC(ffcf318e) SHA1(c675968c931a7e8e00ae83e49e8cef3fd193da57) )
	ROM_LOAD16_BYTE( "p47j_19.bin", 0x000001, 0x010000, CRC(adb8c12e) SHA1(31590b037133f81a52779dbd4f2b5ac5b59198ae) )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "p47j_5.bin",  0x000000, 0x020000, CRC(fe65b65c) SHA1(b13902bf3b469c06d0646c49ddf211f16cb5e5c3) )
	ROM_LOAD( "p47j_6.bin",  0x020000, 0x020000, CRC(e191d2d2) SHA1(d494c652953f5c8dcd8c8b696a011d085d335fea) )
	ROM_LOAD( "p47j_7.bin",  0x040000, 0x020000, CRC(f77723b7) SHA1(2f95ea5e55bc21c4e9a760f102f2dc13b9ca6cf1) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "p47j_23.bin", 0x000000, 0x020000, CRC(6e9bc864) SHA1(f56ea2dd638a8f6952796535eb549ddd55573bcf) )
	ROM_RELOAD(              0x020000, 0x020000 )   /* why? */
	ROM_LOAD( "p47j_12.bin", 0x040000, 0x020000, CRC(5268395f) SHA1(de0cba1e7a7d4acc27467d1b553e8f39bea7282e) )

	ROM_REGION( 0x020000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "p47us16.bin", 0x000000, 0x010000, CRC(5a682c8f) SHA1(0910025e2ee068e5a1fe7f2daae64c9112ab1de6) ) // "phantom" instead of "freedom" in the logo

	ROM_REGION( 0x080000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "p47j_27.bin", 0x000000, 0x020000, CRC(9e2bde8e) SHA1(8cac74c8177a6953b78c6fbf734dfee5da8fc961) )
	ROM_LOAD( "p47j_18.bin", 0x020000, 0x020000, CRC(29d8f676) SHA1(6af5ec9aa96ea67c2c95bcca2164afc128e84a31) )
	ROM_LOAD( "p47j_26.bin", 0x040000, 0x020000, CRC(4d07581a) SHA1(768693e1fcb822b8284ba14c9a5c3d6b00f73383) )
	ROM_RELOAD(              0x060000, 0x020000 )   /* why? */

	ROM_REGION( 0x040000, "oki1", 0 )       /* Samples */
	ROM_LOAD( "p47j_20.bin", 0x000000, 0x020000, CRC(2ed53624) SHA1(2b8ed16cffb6179587e7f01fcbcc30ed436d7afa) )
	ROM_LOAD( "p47j_21.bin", 0x020000, 0x020000, CRC(6f56b56d) SHA1(30f386870411ff0e65684a8d8e6d4afb9125718a) )

	ROM_REGION( 0x040000, "oki2", 0 )       /* Samples */
	ROM_LOAD( "p47j_10.bin", 0x000000, 0x020000, CRC(b9d79c1e) SHA1(315dbed9b7cc289b383c95e6c94267682324154c) )
	ROM_LOAD( "p47j_11.bin", 0x020000, 0x020000, CRC(fa0d1887) SHA1(d24c17806669f5b12527b36bc9c10fd16222e23c) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM  (N82S131N compatible type BPROM) */
	ROM_LOAD( "p-47.14m",    0x0000, 0x0200, CRC(1d877538) SHA1(a5be0dc65dcfc36fbba10d1fddbe155e24b6122f) )
ROM_END


ROM_START( p47j )
	ROM_REGION( 0x60000, "maincpu", 0 )     /* Main CPU Code */
	ROM_LOAD16_BYTE( "p47j_3.bin", 0x000000, 0x020000, CRC(11c655e5) SHA1(a2bfd6538ac81a5f20fa77460ba045584313413a) )
	ROM_LOAD16_BYTE( "p47j_1.bin", 0x000001, 0x020000, CRC(0a5998de) SHA1(9f474c6c9b125fc7c41a44dbaacf3ba3800df8b5) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "p47j_9.bin",  0x000000, 0x010000, CRC(ffcf318e) SHA1(c675968c931a7e8e00ae83e49e8cef3fd193da57) )
	ROM_LOAD16_BYTE( "p47j_19.bin", 0x000001, 0x010000, CRC(adb8c12e) SHA1(31590b037133f81a52779dbd4f2b5ac5b59198ae) )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "p47j_5.bin",  0x000000, 0x020000, CRC(fe65b65c) SHA1(b13902bf3b469c06d0646c49ddf211f16cb5e5c3) )
	ROM_LOAD( "p47j_6.bin",  0x020000, 0x020000, CRC(e191d2d2) SHA1(d494c652953f5c8dcd8c8b696a011d085d335fea) )
	ROM_LOAD( "p47j_7.bin",  0x040000, 0x020000, CRC(f77723b7) SHA1(2f95ea5e55bc21c4e9a760f102f2dc13b9ca6cf1) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "p47j_23.bin", 0x000000, 0x020000, CRC(6e9bc864) SHA1(f56ea2dd638a8f6952796535eb549ddd55573bcf) )
	ROM_RELOAD(              0x020000, 0x020000 )   /* why? */
	ROM_LOAD( "p47j_12.bin", 0x040000, 0x020000, CRC(5268395f) SHA1(de0cba1e7a7d4acc27467d1b553e8f39bea7282e) )

	ROM_REGION( 0x020000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "p47j_16.bin", 0x000000, 0x010000, CRC(30e44375) SHA1(62a4bb217b6aad5fd4760a0f4999cb63559549a5) )

	ROM_REGION( 0x080000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "p47j_27.bin", 0x000000, 0x020000, CRC(9e2bde8e) SHA1(8cac74c8177a6953b78c6fbf734dfee5da8fc961) )
	ROM_LOAD( "p47j_18.bin", 0x020000, 0x020000, CRC(29d8f676) SHA1(6af5ec9aa96ea67c2c95bcca2164afc128e84a31) )
	ROM_LOAD( "p47j_26.bin", 0x040000, 0x020000, CRC(4d07581a) SHA1(768693e1fcb822b8284ba14c9a5c3d6b00f73383) )
	ROM_RELOAD(              0x060000, 0x020000 )   /* why? */

	ROM_REGION( 0x040000, "oki1", 0 )       /* Samples */
	ROM_LOAD( "p47j_20.bin", 0x000000, 0x020000, CRC(2ed53624) SHA1(2b8ed16cffb6179587e7f01fcbcc30ed436d7afa) )
	ROM_LOAD( "p47j_21.bin", 0x020000, 0x020000, CRC(6f56b56d) SHA1(30f386870411ff0e65684a8d8e6d4afb9125718a) )

	ROM_REGION( 0x040000, "oki2", 0 )       /* Samples */
	ROM_LOAD( "p47j_10.bin", 0x000000, 0x020000, CRC(b9d79c1e) SHA1(315dbed9b7cc289b383c95e6c94267682324154c) )
	ROM_LOAD( "p47j_11.bin", 0x020000, 0x020000, CRC(fa0d1887) SHA1(d24c17806669f5b12527b36bc9c10fd16222e23c) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM  (N82S131N compatible type BPROM) */
	ROM_LOAD( "p-47.14m",    0x0000, 0x0200, CRC(1d877538) SHA1(a5be0dc65dcfc36fbba10d1fddbe155e24b6122f) )
ROM_END


/***************************************************************************

P-47 (Japan, Export)

MB 8843 sub-board with "P-47 (B)TYPE" sticker.
The program ROMs are labelled "JALECO EXPORT P-47 #".
Extra EPROM labelled "JALECO EXPORT 17".
It contains enemy sprites without the German "Iron Cross" emblem.

***************************************************************************/

ROM_START( p47je )
	ROM_REGION( 0x60000, "maincpu", 0 )     /* Main CPU Code */
	ROM_LOAD16_BYTE( "export_p-47_3.rom2", 0x000000, 0x020000, CRC(37185412) SHA1(02c4c7dcc448d9ac85a699bd2cee9a060ad9e088) )
	ROM_LOAD16_BYTE( "export_p-47_1.rom1", 0x000001, 0x020000, CRC(3925dd4f) SHA1(687bac19e5786d09addb313123f2c32d9601c0ff) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "p47j_9.bin",  0x000000, 0x010000, CRC(ffcf318e) SHA1(c675968c931a7e8e00ae83e49e8cef3fd193da57) )
	ROM_LOAD16_BYTE( "p47j_19.bin", 0x000001, 0x010000, CRC(adb8c12e) SHA1(31590b037133f81a52779dbd4f2b5ac5b59198ae) )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "p47j_5.bin",  0x000000, 0x020000, CRC(fe65b65c) SHA1(b13902bf3b469c06d0646c49ddf211f16cb5e5c3) )
	ROM_LOAD( "p47j_6.bin",  0x020000, 0x020000, CRC(e191d2d2) SHA1(d494c652953f5c8dcd8c8b696a011d085d335fea) )
	ROM_LOAD( "p47j_7.bin",  0x040000, 0x020000, CRC(f77723b7) SHA1(2f95ea5e55bc21c4e9a760f102f2dc13b9ca6cf1) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "p47j_23.bin", 0x000000, 0x020000, CRC(6e9bc864) SHA1(f56ea2dd638a8f6952796535eb549ddd55573bcf) )
	ROM_RELOAD(              0x020000, 0x020000 )   /* why? */
	ROM_LOAD( "p47j_12.bin", 0x040000, 0x020000, CRC(5268395f) SHA1(de0cba1e7a7d4acc27467d1b553e8f39bea7282e) )

	ROM_REGION( 0x020000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "p47j_16.bin", 0x000000, 0x010000, CRC(30e44375) SHA1(62a4bb217b6aad5fd4760a0f4999cb63559549a5) )

	ROM_REGION( 0x080000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "p47j_27.bin",     0x000000, 0x020000, CRC(9e2bde8e) SHA1(8cac74c8177a6953b78c6fbf734dfee5da8fc961) )
	ROM_LOAD( "p47j_18.bin",     0x020000, 0x020000, CRC(29d8f676) SHA1(6af5ec9aa96ea67c2c95bcca2164afc128e84a31) )
	ROM_LOAD( "export_17.rom15", 0x040000, 0x020000, CRC(b6c2e241) SHA1(54c9cc9e858a3060117acc0128ea7e759d255a67) ) // German "Iron Cross" emblem removed from enemies
	ROM_LOAD( "p47j_26.bin",     0x060000, 0x020000, CRC(4d07581a) SHA1(768693e1fcb822b8284ba14c9a5c3d6b00f73383) )

	ROM_REGION( 0x040000, "oki1", 0 )       /* Samples */
	ROM_LOAD( "p47j_20.bin", 0x000000, 0x020000, CRC(2ed53624) SHA1(2b8ed16cffb6179587e7f01fcbcc30ed436d7afa) )
	ROM_LOAD( "p47j_21.bin", 0x020000, 0x020000, CRC(6f56b56d) SHA1(30f386870411ff0e65684a8d8e6d4afb9125718a) )

	ROM_REGION( 0x040000, "oki2", 0 )       /* Samples */
	ROM_LOAD( "p47j_10.bin", 0x000000, 0x020000, CRC(b9d79c1e) SHA1(315dbed9b7cc289b383c95e6c94267682324154c) )
	ROM_LOAD( "p47j_11.bin", 0x020000, 0x020000, CRC(fa0d1887) SHA1(d24c17806669f5b12527b36bc9c10fd16222e23c) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM  (N82S131N compatible type BPROM) */
	ROM_LOAD( "p-47.14m",    0x0000, 0x0200, CRC(1d877538) SHA1(a5be0dc65dcfc36fbba10d1fddbe155e24b6122f) )
ROM_END


/***************************************************************************

                            [ Peek-a-Boo! ]

Jaleco board no. PB-92127A

  CPU: Motorola 68000P10
Sound: OKI M6295
  OSC: 8Mhz
  MCU: MO-90233 (unknown type with internal rom)

interrupts:
    1]      506>    rte
    2]      50a>    move.w  #$ffff, $1f0006.l
                    jsr     $46e0.l             rte
    3]      51c>    rte
    4]      520>    move.w  #$ffff, $1f000a.l   rte
    5-7]    53c>    rte

3832    Show error (d7 = ram segment where error occurred)
        1 after d8000 ok. 3 after e0000&d0000 ok. 4 after ram&rom ok

003E5E: 0000 3E72   [0] Color Ram
003E62: 0000 3E86   [1] Video Ram
003E66: 0000 3E9A   [2] Sprite Ram
003E6A: 0000 3EB0   [3] Work Ram
003E6E: 0000 3EC4   [4] ROM

000000-03ffff   rom (3f760 chksum)
1f0000-1fffff   ram
0d0000-0d3fff   text
0d8000-0d87ff   palette (+200 = text palette)
0e8000-0ebfff   layer
0e0000-0e0001   2 dips, 1f003a<-!
0f0000-0f0001   2 controls
0f8000-0f8001   ???

010000-010001   protection\watchdog;
    fb -> fb
    9x ->   0       watchdog reset?
            else    samples bank?
                    $1ff010 = sample
                    $1ff014 = bank = sample - $22 (33DC: 1 1 2 3 4 5 6 6 6 6)
                        samples:    bank:
                        $00-21      0
                        $22-2b      1-6
000000-01ffff
020000-03ffff   banked

    51 -> paddle p1
    52 -> paddle p2
    4bba waits for 1f000a to go !0, then clears 1f000a (int 4)
    4bca waits (100000) & FF == 3
    sequence $81, $71, $67 written


Scroll x,y,ctrl:
c2000<-1f0010       c2002<-1f0014       c2004<-1f000c

Scroll x,y,ctrl:
c2008<-1f0018       c200a<-1f001c       c200c<-1f000e

Layers ctrl:
c2208<-1f0024<<8 + 1f0026       c2308<-1f0022 | 1f002c

Sprite bank + ??
c2108<-1f005a + 1f0060 + 1f0062 + 1f0068

Sprite ctrl:
c2200<-0

1f0000.w    routine index, table at $fae:
    0: 4E40
    1: 4EC2
    2: 4F2C
    3: 4F70
    4: 4FBC
    5: 533A
    6: 5382
    7: 556E

1f003c/40   paddle p1/p2
1f0260/4.l  *** p1/p2 score/10 (BCD) ***
1f02e6/8.w  *** p1/p2 current lives ***
            Bonus lives:    20K  100K  250K  500K 1000K
1f02ee      current player (0/1)
1f0380      hi score


***************************************************************************/


ROM_START( peekaboo )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 68000 CPU Code */
	ROM_LOAD16_BYTE( "j3", 0x000000, 0x020000, CRC(f5f4cf33) SHA1(f135f2b627347255bb0811e9a4a213e3b447c199) )
	ROM_LOAD16_BYTE( "j2", 0x000001, 0x020000, CRC(7b3d430d) SHA1(8b48101929da4938a61dfd0eda845368c4184831) )

	ROM_REGION( 0x1000, "mcu", 0 ) /* MCU Internal Code, M50747 */
	ROM_LOAD( "mo-90233.mcu", 0x000000, 0x1000, NO_DUMP )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "5",       0x000000, 0x080000, CRC(34fa07bb) SHA1(0f688acf302fd56701ee4fcc1d692adb7bf86ce4) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "4",       0x000000, 0x020000, CRC(f037794b) SHA1(235c278121921b234a27835284be80c136e6409b) )

	ROM_REGION( 0x020000, "gfx3", ROMREGION_ERASEFF ) /* Scroll 2 */
	// Unused

	ROM_REGION( 0x080000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "1",       0x000000, 0x080000, CRC(5a444ecf) SHA1(38a7a6e91d0635a7f82a1c9a04efe1586ed3d856) )

	ROM_REGION( 0x100000, "oki1", 0 )       /* Samples */
	ROM_LOAD( "peeksamp.124", 0x000000, 0x100000, CRC(e1206fa8) SHA1(339d5a4fa2af7fb4ab2e9c6c66f4848fa8774832) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM */
	ROM_LOAD( "priority.69",    0x000000, 0x200, CRC(b40bff56) SHA1(39c95eed79328ef2df754988db83e07909e848f8) )
ROM_END

ROM_START( peekaboou )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 68000 CPU Code */
	ROM_LOAD16_BYTE( "pb92127a_3_ver1.0.ic29", 0x000000, 0x020000, CRC(4603176a) SHA1(bbdc3fa439b32bdaaef5ca374af89e25fc4d9c1a) )
	ROM_LOAD16_BYTE( "pb92127a_2_ver1.0.ic28", 0x000001, 0x020000, CRC(7bf4716b) SHA1(f2c0bfa32426c9816d9d3fbd73560566a497912d) )

	ROM_REGION( 0x1000, "mcu", 0 ) /* MCU Internal Code, M50747 */
	ROM_LOAD( "mo-90233.mcu", 0x000000, 0x1000, NO_DUMP )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "5",       0x000000, 0x080000, CRC(34fa07bb) SHA1(0f688acf302fd56701ee4fcc1d692adb7bf86ce4) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "4",       0x000000, 0x020000, CRC(f037794b) SHA1(235c278121921b234a27835284be80c136e6409b) )

	ROM_REGION( 0x020000, "gfx3", ROMREGION_ERASEFF ) /* Scroll 2 */
	// Unused

	ROM_REGION( 0x080000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "1",       0x000000, 0x080000, CRC(5a444ecf) SHA1(38a7a6e91d0635a7f82a1c9a04efe1586ed3d856) )

	ROM_REGION( 0x100000, "oki1", 0 )       /* Samples */
	ROM_LOAD( "peeksamp.124", 0x000000, 0x100000, CRC(e1206fa8) SHA1(339d5a4fa2af7fb4ab2e9c6c66f4848fa8774832) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM */
	ROM_LOAD( "priority.69",    0x000000, 0x200, CRC(b40bff56) SHA1(39c95eed79328ef2df754988db83e07909e848f8) )
ROM_END

/***************************************************************************

                            [ Plus Alpha ]
                          (aka Flight Alpha)

f2ef8.w     bombs
f309e.w     *** lives       ***
f30a4.l     *** score (BCD) ***

***************************************************************************/


ROM_START( plusalph )
	ROM_REGION( 0x60000, "maincpu", 0 )     /* Main CPU Code */
	ROM_LOAD16_BYTE( "pa-rom2.bin", 0x000000, 0x020000, CRC(33244799) SHA1(686fb7fa8a6c25b5aff78bc509f35c69492d7d1e) )
	ROM_LOAD16_BYTE( "pa-rom1.bin", 0x000001, 0x020000, CRC(a32fdcae) SHA1(c2315a7142e5499e9325f5a8361cb25e83747a3e) )
	ROM_LOAD16_BYTE( "pa-rom3.bin", 0x040000, 0x010000, CRC(1b739835) SHA1(3aaa9545a7f578a9775311dcd44504870f3b1544) )
	ROM_LOAD16_BYTE( "pa-rom4.bin", 0x040001, 0x010000, CRC(ff760e80) SHA1(dd06306a516a2d5e49cf8f2343ddc26405b309a9) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "pa-rom5.bin", 0x000000, 0x010000, CRC(ddc2739b) SHA1(dee31660428baea44c73dec238ed7f39a6771fe6) )
	ROM_LOAD16_BYTE( "pa-rom6.bin", 0x000001, 0x010000, CRC(f6f8a167) SHA1(60d5c9db18d8f6704b68ccde5d026174679cec36) )

	ROM_REGION( 0x1000, "mcu", 0 ) /* M50747 MCU Code */
	ROM_LOAD( "m50747", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "pa-rom11.bin", 0x000000, 0x020000, CRC(eb709ae7) SHA1(434c9da3c79a97ddd9be77908ce65e9efe6c8106) )
	ROM_LOAD( "pa-rom12.bin", 0x020000, 0x020000, CRC(cacbc350) SHA1(328094a5d067775871f7d69b4c20c40e46b0eaba) )
	ROM_LOAD( "pa-rom13.bin", 0x040000, 0x020000, CRC(fad093dd) SHA1(98a42f91d66b990e18f845edc01c23568cbc7a83) )
	ROM_LOAD( "pa-rom14.bin", 0x060000, 0x020000, CRC(d3676cd1) SHA1(b805216af1a65c2f19a82aaf3775bbbceb065c7e) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "pa-rom15.bin", 0x000000, 0x020000, CRC(8787735b) SHA1(9083061ce6de98a611558fc59d1ec37aefc2a2e0) )
	ROM_LOAD( "pa-rom16.bin", 0x020000, 0x020000, CRC(a06b813b) SHA1(de4fb0f92f9178c2b5f2750d17cfc3da126b23a4) )
	ROM_LOAD( "pa-rom17.bin", 0x040000, 0x020000, CRC(c6b38a4b) SHA1(2529ee47324edcdf4cd34c159524f37da08e1099) )
	/* empty place */

	ROM_REGION( 0x020000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "pa-rom19.bin", 0x000000, 0x010000, CRC(39ef193c) SHA1(93f417a36732ca76d566f2ff2c9ff62e5679da08) )

	ROM_REGION( 0x080000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "pa-rom20.bin", 0x000000, 0x020000, CRC(86c557a8) SHA1(a6dfb2aeda07639448a4135d1336019214030fc9) )
	ROM_LOAD( "pa-rom21.bin", 0x020000, 0x020000, CRC(81140a88) SHA1(f31ae208623bdb152888e1ebdd2515f9bcc2fb44) )
	ROM_LOAD( "pa-rom22.bin", 0x040000, 0x020000, CRC(97e39886) SHA1(89161ae647c835ff24fcb8676f6e1228f9a1fa10) )
	ROM_LOAD( "pa-rom23.bin", 0x060000, 0x020000, CRC(0383fb65) SHA1(31fa9c9ba57bf3c9ee8e3f5b1b4e28c1a3591a4b) )

	ROM_REGION( 0x040000, "oki1", 0 )       /* Samples */
	ROM_LOAD( "pa-rom9.bin",  0x000000, 0x020000, CRC(065364bd) SHA1(bacb268b1c76c286e89eb823d8c3477ec5f2516c) )
	ROM_LOAD( "pa-rom10.bin", 0x020000, 0x020000, CRC(395df3b2) SHA1(6f69b573e997ba4bb5aabf745843921f0866d209) )

	ROM_REGION( 0x040000, "oki2", 0 )       /* Samples */
	ROM_LOAD( "pa-rom7.bin",  0x000000, 0x020000, CRC(9f5d800e) SHA1(c0a7bdedc8a1294b8d29f7f6007542ea006e70cb) )
	ROM_LOAD( "pa-rom8.bin",  0x020000, 0x020000, CRC(ae007750) SHA1(b48784fd6bcf205296e3e5b59c258f76da5c2d1b) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM */
	ROM_LOAD( "prom.14m",     0x0000, 0x0200, CRC(1d877538) SHA1(a5be0dc65dcfc36fbba10d1fddbe155e24b6122f) )
ROM_END


/***************************************************************************

                            [ RodLand ]

 2 TMP 68K
 1 YM2151
 1 YM3014
 1 OSC 7 Mhz
 1 OSC 4 Mhz
 1 OSC 12 Mhz

 Rod Land (Alt JPN Ver.)
 (c)1988 Jaleco
 Mega-System
 A-Type
 MB-M02A (EB-88003-3001-1)

(World version)
interrupts: 1] 418->3864: rts   2] 420: move.w #-1,f0010; jsr 3866  3] rte

213da   print test error (20c12 = string address 0-4)

f0018->84200    f0020->84208    f0028->84008
f001c->84202    f0024->8420a    f002c->8400a
f0012->84204    f0014->8420c    f0016->8400c

7fe     d0.w -> 84000.w & f000e.w
81a     d0/d1/d2 & $D -> 84204 / 8420c /8400c

***************************************************************************/


ROM_START( rodland )
	ROM_REGION( 0x60000, "maincpu", 0 )     /* Main CPU Code */
	ROM_LOAD16_BYTE( "JALECO_ROD_LAND_2.ROM2", 0x000000, 0x020000, CRC(c7e00593) SHA1(055b7bcabf90ed6d5edc2797d0f85a5d49b8693b) )
	ROM_LOAD16_BYTE( "JALECO_ROD_LAND_1.ROM1", 0x000001, 0x020000, CRC(2e748ca1) SHA1(285414af11aad36f3bd7020365ff90eb696d2de3) )
	ROM_LOAD16_BYTE( "JALECO_ROD_LAND_3.ROM3", 0x040000, 0x010000, CRC(62fdf6d7) SHA1(ffde7e7f5b3b548bc980b9dee767f693046ecab2) )
	ROM_LOAD16_BYTE( "JALECO_ROD_LAND_4.ROM4", 0x040001, 0x010000, CRC(44163c86) SHA1(1c56d79531af0312e7cd3dc66cf61b55dd1a6e51) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "JALECO_ROD_LAND_5.ROM5", 0x000000, 0x010000, CRC(c1617c28) SHA1(1b3440055c083b74270fe06b5f42e7d1337efeca) )
	ROM_LOAD16_BYTE( "JALECO_ROD_LAND_6.ROM6", 0x000001, 0x010000, CRC(663392b2) SHA1(99052639e934d1ca18888c9c7fa061c1d3508fd4) )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "LH534H31.ROM14", 0x000000, 0x080000, CRC(8201e1bb) SHA1(3304100dcab7b67cee021869a50f4295c8635814) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "LH534H32.ROM18", 0x000000, 0x080000, CRC(f3b30ca6) SHA1(f2f88c24a009b6695f7548aebd37b25d1fd19892) )

	ROM_REGION( 0x020000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "LH2311J0.ROM19", 0x000000, 0x020000, CRC(124d7e8f) SHA1(d7885a10085cc3389bd0e26e9d54adb8929218c0) )

	ROM_REGION( 0x080000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "LH534H33.ROM23", 0x000000, 0x080000, CRC(936db174) SHA1(4dfb2c31bc4bbf659184fe18e320d19f326b3ec5) )

	ROM_REGION( 0x040000, "oki1", 0 )       /* Samples */
	ROM_LOAD( "LH5321T5.ROM10", 0x000000, 0x040000, CRC(e1d1cd99) SHA1(6604111d37455c1bd59c1469d9ee7841e7dec913) )

	ROM_REGION( 0x040000, "oki2", 0 )       /* Samples */
	ROM_LOAD( "S202000DR.ROM8", 0x000000, 0x040000, CRC(8a49d3a7) SHA1(68cb8cf2753b39c253d0edaa8ef2c54fd1f6ebe5) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM */
	ROM_LOAD( "PS89013A.M14",    0x0000, 0x0200, CRC(8914e72d) SHA1(80a664471f14c8ed8544a5e226fdca425ab3c657) )
ROM_END


ROM_START( rodlandj )
	ROM_REGION( 0x60000, "maincpu", 0 )     /* Main CPU Code */
	ROM_LOAD16_BYTE( "JALECO_ROD_LAND_2.ROM2", 0x000000, 0x020000, CRC(b1d2047e) SHA1(75d282b7614c5f4b76ab44e34fea9e87ab8b992c) )
	ROM_LOAD16_BYTE( "JALECO_ROD_LAND_1.ROM1", 0x000001, 0x020000, CRC(3c47c2a3) SHA1(62e66a2f53aeacf92551ba64ae4ce14c2e982bb0) )
	ROM_LOAD16_BYTE( "JALECO_ROD_LAND_3.ROM3", 0x040000, 0x010000, CRC(c5b1075f) SHA1(a8bcc0e9dbb4b731bc0b7e5a8e0efc3d142505b9) )
	ROM_LOAD16_BYTE( "JALECO_ROD_LAND_4.ROM4", 0x040001, 0x010000, CRC(9ec61048) SHA1(71b6af054a528af04e23affff635a9358537cd3b) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "JALECO_ROD_LAND_5.ROM5", 0x000000, 0x010000, CRC(c1617c28) SHA1(1b3440055c083b74270fe06b5f42e7d1337efeca) )
	ROM_LOAD16_BYTE( "JALECO_ROD_LAND_6.ROM6", 0x000001, 0x010000, CRC(663392b2) SHA1(99052639e934d1ca18888c9c7fa061c1d3508fd4) )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "LH534H31.ROM14", 0x000000, 0x080000, CRC(8201e1bb) SHA1(3304100dcab7b67cee021869a50f4295c8635814) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "LH534H32.ROM18", 0x000000, 0x080000, CRC(f3b30ca6) SHA1(f2f88c24a009b6695f7548aebd37b25d1fd19892) )

	ROM_REGION( 0x020000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "LH2311J0.ROM19", 0x000000, 0x020000, CRC(124d7e8f) SHA1(d7885a10085cc3389bd0e26e9d54adb8929218c0) )

	ROM_REGION( 0x080000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "LH534H33.ROM23", 0x000000, 0x080000, CRC(936db174) SHA1(4dfb2c31bc4bbf659184fe18e320d19f326b3ec5) )

	ROM_REGION( 0x040000, "oki1", 0 )       /* Samples */
	ROM_LOAD( "LH5321T5.ROM10", 0x000000, 0x040000, CRC(e1d1cd99) SHA1(6604111d37455c1bd59c1469d9ee7841e7dec913) )

	ROM_REGION( 0x040000, "oki2", 0 )       /* Samples */
	ROM_LOAD( "S202000DR.ROM8", 0x000000, 0x040000, CRC(8a49d3a7) SHA1(68cb8cf2753b39c253d0edaa8ef2c54fd1f6ebe5) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM */
	ROM_LOAD( "PS89013A.M14",    0x0000, 0x0200, CRC(8914e72d) SHA1(80a664471f14c8ed8544a5e226fdca425ab3c657) )
ROM_END

/* probably a prototype, original JP key and unscrambled ROMs, incorrect
   audio matches PCB */
ROM_START( rittam )
	ROM_REGION( 0x60000, "maincpu", 0 )     /* Main CPU Code */
	ROM_LOAD16_BYTE( "2.ROM2", 0x000000, 0x020000, CRC(93085af2) SHA1(e49dc1e62c1cec75f192ac4608f69c4361ad739a) )
	ROM_LOAD16_BYTE( "R+T_1.ROM1", 0x000001, 0x020000, CRC(20446c34) SHA1(10753b8c3826468f42c5b1da8cfa60658db60401) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "JALECO_5.ROM5", 0x000000, 0x010000, CRC(ea6600ec) SHA1(392e782a266c5997331df75b15211bced8efb47c) )
	ROM_LOAD16_BYTE( "JALECO_6.ROM6", 0x000001, 0x010000, CRC(51c3c0bc) SHA1(97d6b49d1816cd02ea50ae5f7909a84e9ca8b06f) )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "11.ROM11", 0x000000, 0x020000, CRC(ad2bf897) SHA1(3c449bef7f82aa1d111932361c83ae6661f3bee7) )
	ROM_LOAD( "12.ROM12", 0x020000, 0x020000, CRC(d0224ed6) SHA1(aa1701b248e9be120a001032052c693bf29c386a) )
	ROM_LOAD( "13.ROM13", 0x040000, 0x020000, CRC(b1d5d423) SHA1(df0e34797826f4458a26992a84bdd1e790a942d9) )
	ROM_LOAD( "14.ROM14", 0x060000, 0x020000, CRC(20f8c361) SHA1(9e644041de89b279ed4e2420ac938849c42242f6) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "15.ROM15", 0x000000, 0x020000, CRC(90bc97ac) SHA1(bdd3ce2214e99ce6c66982cf21ce0641fbcfeb6d) )
	ROM_LOAD( "16.ROM16", 0x020000, 0x020000, CRC(e38750aa) SHA1(b231835c204d33c05a854d8450cfd334102a45be) )
	// ROM17 not populated - not sure why, missing?
	ROM_LOAD( "18.ROM18", 0x060000, 0x020000, CRC(57ccf24f) SHA1(8d480093359ebea8e053810ad834b5b1f893bb77) )

	ROM_REGION( 0x020000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "19.ROM19", 0x000000, 0x020000, CRC(6daa1081) SHA1(400cfa302b7d7238b966462c4d9272e8b8dad6f1) )

	ROM_REGION( 0x080000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "R+T_20.ROM20", 0x000000, 0x020000, CRC(23bc2b0b) SHA1(2aa85b0aa56de367ba8f9c79494b242d1d0db11c) )
	ROM_LOAD( "21.ROM21", 0x020000, 0x020000, CRC(9d2b0ec4) SHA1(b589697948ba400da061bfa7ac199b35245f6426) )
	ROM_LOAD( "22.ROM22", 0x040000, 0x020000, CRC(bba2e2cf) SHA1(d718ecf65ad974a981e7f851781c2a83943a4e6e) )
	ROM_LOAD( "23.ROM23", 0x060000, 0x020000, CRC(05536a18) SHA1(6cc1417d91985bf92dbd0db822dde005a7dc001d) )

	ROM_REGION( 0x040000, "oki1", 0 )       /* Samples */
	ROM_LOAD( "JALECO_9.ROM9", 0x000000, 0x020000, CRC(065364bd) SHA1(bacb268b1c76c286e89eb823d8c3477ec5f2516c) )
	ROM_LOAD( "JALECO_10.ROM10", 0x020000, 0x020000, CRC(395df3b2) SHA1(6f69b573e997ba4bb5aabf745843921f0866d209) )

	ROM_REGION( 0x040000, "oki2", 0 )       /* Samples */
	ROM_LOAD( "JALECO_7.ROM7", 0x000000, 0x020000, CRC(76fd879f) SHA1(a2169e2efa0c8e804f7d2fac32c655f1379d95e1) )
	ROM_LOAD( "JALECO_8.ROM8", 0x020000, 0x020000, CRC(a771ab00) SHA1(be547b296ee3fcc0ab7339f2c99d1039ceb3b5bb) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM */
	ROM_LOAD( "PS89013A.M14",    0x0000, 0x0200, CRC(8914e72d) SHA1(80a664471f14c8ed8544a5e226fdca425ab3c657) )
ROM_END


/* 100% identical to rodlandj, but not encrypted */
ROM_START( rodlandjb )
	ROM_REGION( 0x60000, "maincpu", 0 )     /* Main CPU Code */
	ROM_LOAD16_BYTE( "rl19.bin", 0x000000, 0x010000, CRC(028de21f) SHA1(04c88a0138dd119655b4a8a965617781a9a6ff71) )
	ROM_LOAD16_BYTE( "rl17.bin", 0x000001, 0x010000, CRC(9c720046) SHA1(8543f0942863b4aa5329572dd1f374ea18c29851) )
	ROM_LOAD16_BYTE( "rl20.bin", 0x020000, 0x010000, CRC(3f536d07) SHA1(cfcf47c42677fae204b3a7d70786d157279ba6e5) )
	ROM_LOAD16_BYTE( "rl18.bin", 0x020001, 0x010000, CRC(5aa61717) SHA1(3292cdafc10b412c06addc3c6d4e39ee70ff06e8) )
	ROM_LOAD16_BYTE( "rl_3.bin", 0x040000, 0x010000, CRC(c5b1075f) SHA1(a8bcc0e9dbb4b731bc0b7e5a8e0efc3d142505b9) )
	ROM_LOAD16_BYTE( "rl_4.bin", 0x040001, 0x010000, CRC(9ec61048) SHA1(71b6af054a528af04e23affff635a9358537cd3b) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "rl02.bin", 0x000000, 0x010000, CRC(d26eae8f) SHA1(1c6d514e6d006f78fa7b24d18a3eb4c5a4c5cbce) )
	ROM_LOAD16_BYTE( "rl01.bin", 0x000001, 0x010000, CRC(04cf24bc) SHA1(e754cce3c83a7088daf90e753fbb0df9ef7fc9be) )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "LH534H31.ROM14", 0x000000, 0x080000, CRC(8201e1bb) SHA1(3304100dcab7b67cee021869a50f4295c8635814) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "LH534H32.ROM18", 0x000000, 0x080000, CRC(f3b30ca6) SHA1(f2f88c24a009b6695f7548aebd37b25d1fd19892) )

	ROM_REGION( 0x020000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "LH2311J0.ROM19", 0x000000, 0x020000, CRC(124d7e8f) SHA1(d7885a10085cc3389bd0e26e9d54adb8929218c0) )

	ROM_REGION( 0x080000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "LH534H33.ROM23", 0x000000, 0x080000, CRC(936db174) SHA1(4dfb2c31bc4bbf659184fe18e320d19f326b3ec5) )

	ROM_REGION( 0x040000, "oki1", 0 )       /* Samples */
	ROM_LOAD( "LH5321T5.ROM10", 0x000000, 0x040000, CRC(e1d1cd99) SHA1(6604111d37455c1bd59c1469d9ee7841e7dec913) )

	ROM_REGION( 0x040000, "oki2", 0 )       /* Samples */
	ROM_LOAD( "S202000DR.ROM8", 0x000000, 0x040000, CRC(8a49d3a7) SHA1(68cb8cf2753b39c253d0edaa8ef2c54fd1f6ebe5) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM */
	ROM_LOAD( "PS89013A.M14",    0x0000, 0x0200, CRC(8914e72d) SHA1(80a664471f14c8ed8544a5e226fdca425ab3c657) )
ROM_END


/***************************************************************************

                            [ Saint Dragon ]

            *** Press coin on startup to enter test mode ***

interrupts: 1] rte  2] 620  3] 5e6

***************************************************************************/


ROM_START( stdragon )
	ROM_REGION( 0x60000, "maincpu", 0 )     /* Main CPU Code */
	ROM_LOAD16_BYTE( "jsd-02.bin", 0x000000, 0x020000, CRC(cc29ab19) SHA1(e145eeb01fad313e300f0c614c0e7a5c1d75d7d9) )
	ROM_LOAD16_BYTE( "jsd-01.bin", 0x000001, 0x020000, CRC(67429a57) SHA1(f3c20fabed97ac5c2fe3e891f9c8c86478453a6c) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "jsd-05.bin", 0x000000, 0x010000, CRC(8c04feaa) SHA1(57e86fd88dc72d123a41f0dee80a16be38ac2e81) )
	ROM_LOAD16_BYTE( "jsd-06.bin", 0x000001, 0x010000, CRC(0bb62f3a) SHA1(68d9f161ba2568f8e046b1a40127bbb973d7a884) )

	ROM_REGION( 0x1000, "mcu", 0 ) /* M50747 MCU Code */
	ROM_LOAD( "m50747", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "jsd-11.bin", 0x000000, 0x020000, CRC(2783b7b1) SHA1(4edde596cf26afb33b247cf5b1420d86f8f0c104) )
	ROM_LOAD( "jsd-12.bin", 0x020000, 0x020000, CRC(89466ab7) SHA1(8de42f2828e48e4fe3c6d078f6b9d48498933d72) )
	ROM_LOAD( "jsd-13.bin", 0x040000, 0x020000, CRC(9896ae82) SHA1(953e79558d66f7bfff893a7b69450ae23f8d16a4) )
	ROM_LOAD( "jsd-14.bin", 0x060000, 0x020000, CRC(7e8da371) SHA1(0bed102bdd4dfca23ca636c7de67da1a9107eb5a) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "jsd-15.bin", 0x000000, 0x020000, CRC(e296bf59) SHA1(15361b2c2df391656e47b815731f5f03a6a8a7a6) )
	ROM_LOAD( "jsd-16.bin", 0x020000, 0x020000, CRC(d8919c06) SHA1(643f04911ea035db7ddb593f5bf1be364f645fe1) )
	ROM_LOAD( "jsd-17.bin", 0x040000, 0x020000, CRC(4f7ad563) SHA1(ef06e0c8dc29b75cc7c449a227576be442163a6e) )
	ROM_LOAD( "jsd-18.bin", 0x060000, 0x020000, CRC(1f4da822) SHA1(fe81fb93c1aa938425b6bfca2bb829dda2d86b4a) )

	ROM_REGION( 0x020000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "jsd-19.bin", 0x000000, 0x010000, CRC(25ce807d) SHA1(64accb923e9727093790c8ae8296e9ff2d04af06) )

	ROM_REGION( 0x080000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "jsd-20.bin", 0x000000, 0x020000, CRC(2c6e93bb) SHA1(6130611a0a4067ced1b646438c49387d8cea0c98) )
	ROM_LOAD( "jsd-21.bin", 0x020000, 0x020000, CRC(864bcc61) SHA1(21420a4804311fb381a9b05068d60d518fdc67db) )
	ROM_LOAD( "jsd-22.bin", 0x040000, 0x020000, CRC(44fe2547) SHA1(193f487993e10e4a00e50d7febc9e43f3d217038) )
	ROM_LOAD( "jsd-23.bin", 0x060000, 0x020000, CRC(6b010e1a) SHA1(7d056330a16725999c7b6662eb6ddeea0a4e446c) )

	ROM_REGION( 0x040000, "oki1", 0 )       /* Samples */
	ROM_LOAD( "jsd-09.bin", 0x000000, 0x020000, CRC(e366bc5a) SHA1(c97bc1f25357366b4ff1343dfc9d0808a2630b28) )
	ROM_LOAD( "jsd-10.bin", 0x020000, 0x020000, CRC(4a8f4fe6) SHA1(4f13f0149aa29b7cbddcd782f043bb71b3d27ede) )

	ROM_REGION( 0x040000, "oki2", 0 )       /* Samples */
	ROM_LOAD( "jsd-07.bin", 0x000000, 0x020000, CRC(6a48e979) SHA1(617281d9fe3c3927f94bf2f66d0a08923a92a6ab) )
	ROM_LOAD( "jsd-08.bin", 0x020000, 0x020000, CRC(40704962) SHA1(4efd8c4d406600aa486c8b84b6f9882cca5970a4) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM */
	ROM_LOAD( "prom.14m",    0x0000, 0x0200, CRC(1d877538) SHA1(a5be0dc65dcfc36fbba10d1fddbe155e24b6122f) )
ROM_END

/***************************************************************************

Saint Dragon alternate set

This romset comes from an original pcb. Game differences are: none.

Hardware info:
Jaleco Mega System 1-A

Rom definition:
jsda-01,jsda-02 main program
E71-14 (jsd-11,jsd-12,jsd-13,jsd-14)* background
E72-18 (jsd-15,jsd-16,jsd-17,jsd-18)* foreground
E73-23 (jsd-20,jsd-21,jsd-22,jsd-23)* sprites

*The 128k gfx roms were merged into 512k roms.
Rest of roms are identical to existing set
Roms are 27c101,62304

Dumped by tirino73

***************************************************************************/

ROM_START( stdragona )
	ROM_REGION( 0x60000, "maincpu", 0 )     /* Main CPU Code */
	ROM_LOAD16_BYTE( "jsda-02.bin", 0x000000, 0x020000, CRC(d65d4154) SHA1(f77886590a092743c829fb52b5de0ca8ef51c122) )
	ROM_LOAD16_BYTE( "jsda-01.bin", 0x000001, 0x020000, CRC(c40c8ee1) SHA1(346b16519f35d7bdb283d87f6f89f54d3b7eefe2) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "jsd-05.bin", 0x000000, 0x010000, CRC(8c04feaa) SHA1(57e86fd88dc72d123a41f0dee80a16be38ac2e81) )
	ROM_LOAD16_BYTE( "jsd-06.bin", 0x000001, 0x010000, CRC(0bb62f3a) SHA1(68d9f161ba2568f8e046b1a40127bbb973d7a884) )

	ROM_REGION( 0x1000, "mcu", 0 ) /* M50747 MCU Code */
	ROM_LOAD( "m50747", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Scroll 0 - scrambled */
	ROM_LOAD( "e71-14.bin", 0x000000, 0x080000, CRC(8e26ff92) SHA1(06985056027facb1d3df08cf04277492c1be6102) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "e72-18.bin", 0x000000, 0x080000, CRC(0b234711) SHA1(1c5a8db28cef84434c526eab9cf9c4c123cebeea) )

	ROM_REGION( 0x020000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "jsd-19.bin", 0x000000, 0x010000, CRC(25ce807d) SHA1(64accb923e9727093790c8ae8296e9ff2d04af06) )

	ROM_REGION( 0x080000, "gfx4", 0 ) /* Sprites - scrambled */
	ROM_LOAD( "e73-23.bin", 0x000000, 0x080000,  CRC(00ca3e04) SHA1(ea11007fc8e0b4fa702f24dd740bc0194624836c) )

	ROM_REGION( 0x040000, "oki1", 0 )       /* Samples */
	ROM_LOAD( "jsd-09.bin", 0x000000, 0x020000, CRC(e366bc5a) SHA1(c97bc1f25357366b4ff1343dfc9d0808a2630b28) )
	ROM_LOAD( "jsd-10.bin", 0x020000, 0x020000, CRC(4a8f4fe6) SHA1(4f13f0149aa29b7cbddcd782f043bb71b3d27ede) )

	ROM_REGION( 0x040000, "oki2", 0 )       /* Samples */
	ROM_LOAD( "jsd-07.bin", 0x000000, 0x020000, CRC(6a48e979) SHA1(617281d9fe3c3927f94bf2f66d0a08923a92a6ab) )
	ROM_LOAD( "jsd-08.bin", 0x020000, 0x020000, CRC(40704962) SHA1(4efd8c4d406600aa486c8b84b6f9882cca5970a4) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM */
	ROM_LOAD( "prom.14m",    0x0000, 0x0200, CRC(1d877538) SHA1(a5be0dc65dcfc36fbba10d1fddbe155e24b6122f) )
ROM_END

/***************************************************************************

Bootleg version of Saint Dragon. Two PCBs connected by two flat cables.
Sound section can host two oki chips (and roms) but only one is populated.
No ASICs just logic chips.

- ROMs A-19 and A-20 are fitted 'piggy backed' with one pin
  from A-20 bent out and wired to a nearby TTL.

- Stage 5 has some of its background graphics corrupted.
  Don't know if it is a PCB issue or designed like that.

***************************************************************************/

ROM_START( stdragonb )
	ROM_REGION( 0x60000, "maincpu", 0 )     /* Main CPU Code */
	ROM_LOAD16_BYTE( "a-4.bin", 0x00000, 0x10000, CRC(c58fe5c2) SHA1(221767e995e05b076e256b1818c4b5d85f58e7e6) )
	ROM_LOAD16_BYTE( "a-2.bin", 0x00001, 0x10000, CRC(46a7cdbb) SHA1(b90a0c10a5e7584e565f61b7bb143fb5800ae039) )
	ROM_LOAD16_BYTE( "a-3.bin", 0x20000, 0x10000, CRC(f6a268c4) SHA1(106184fb18ad8018e9a4aad383c7243c254bfab1) )
	ROM_LOAD16_BYTE( "a-1.bin", 0x20001, 0x10000, CRC(0fb439bd) SHA1(ab596cee4d14f9d805c065d826f36298c6486975) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "b-20.bin", 0x00000, 0x10000, CRC(8c04feaa) SHA1(57e86fd88dc72d123a41f0dee80a16be38ac2e81) ) // = jsd-05
	ROM_LOAD16_BYTE( "b-19.bin", 0x00001, 0x10000, CRC(0bb62f3a) SHA1(68d9f161ba2568f8e046b1a40127bbb973d7a884) ) // = jsd-06

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "a-15.bin", 0x00000, 0x10000, CRC(42f7d2cd) SHA1(7518b2d1d92a1c48e6d8ae0723cfa76ac67fa2b9) ) // ~= jsd-11 [1/2]
	ROM_LOAD( "a-16.bin", 0x10000, 0x10000, CRC(4f519a97) SHA1(fc7c9f6756f9b6c8fa96c2eea61066859120ad3a) ) // ~= jsd-11 [2/2]
	ROM_LOAD( "a-14.bin", 0x20000, 0x10000, CRC(d8ba8d4c) SHA1(47c179e46f329c32f09ba539c742633f390fc449) ) // ~= jsd-12 [1/2]

	ROM_LOAD( "a-18.bin", 0x40000, 0x10000, CRC(5e35f269) SHA1(54b3108f819056582c3e85432faa6c31dd706cbe) ) // ~= jsd-13 [1/2]
	ROM_LOAD( "a-19.bin", 0x50000, 0x10000, CRC(b818db20) SHA1(f60b041a7934fb3d1ebf8fcdf12121e33734c6ae) ) // ~= jsd-13 [2/2]
	ROM_LOAD( "a-17.bin", 0x60000, 0x10000, CRC(0f6094f9) SHA1(952976c7e019536b8d718ce7c6ed5e6a643b4070) ) // ~= jsd-14 [1/2]
	ROM_LOAD( "a-20.bin", 0x70000, 0x10000, CRC(e8849b15) SHA1(2c18f56da4d26ca7112d9bd720f26e9cce078eb7) ) // ~= jsd-14 [2/2]

	ROM_REGION( 0x080000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "a-9.bin",  0x00000, 0x10000, CRC(135c2e0e) SHA1(052b29c7060117c7e3e6c7c7826c129775564f87) ) // = jsd-15 [1/2]
	ROM_LOAD( "a-10.bin", 0x10000, 0x10000, CRC(19cec47a) SHA1(b90600b39e4c54e1405be27740e8c55b18681632) ) // = jsd-15 [2/2]
	ROM_LOAD( "a-5.bin",  0x20000, 0x10000, CRC(da4ca7bf) SHA1(f472ce7f474a56779dd3bbd729d908494e94c91c) ) // = jsd-16 [1/2]
	ROM_LOAD( "a-6.bin",  0x30000, 0x10000, CRC(9d9b6470) SHA1(a6433687b1b13517e249138dac1b088ff0bcd2ff) ) // = jsd-16 [2/2]
	ROM_LOAD( "a-12.bin", 0x40000, 0x10000, CRC(22382b5f) SHA1(e177368bf1e02a57d4284362804e1ba5a39cfb35) ) // = jsd-17 [1/2]
	ROM_LOAD( "a-11.bin", 0x50000, 0x10000, CRC(26c2494d) SHA1(224aabd2e431f490bc9e06682ee279e7ca3a7df7) ) // = jsd-17 [2/2]
	ROM_LOAD( "a-7.bin",  0x60000, 0x10000, CRC(cee3a6f7) SHA1(3829591a6724b080435e9d008ff51faf69ebcd71) ) // = jsd-18 [1/2]
	ROM_LOAD( "a-8.bin",  0x70000, 0x10000, CRC(883b99bb) SHA1(820afda20ba2b66ac89a5982178aa5b5f6e2bd74) ) // = jsd-18 [2/2]

	ROM_REGION( 0x020000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "a-13.bin", 0x000000, 0x08000, CRC(9e487aa1) SHA1(6d418467bc865a7069b5a9eb0707d23ce821ad28) ) // = jsd-19 [1/2]

	ROM_REGION( 0x080000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "a-22.bin", 0x00000, 0x10000, CRC(c7ee6d89) SHA1(45bba446dc5821222c09957380d74993310cb3a1) ) // ~= jsd-20 [1/2]
	ROM_LOAD( "a-23.bin", 0x10000, 0x10000, CRC(79552709) SHA1(2e5120efcc0afc46642561b269f410498f6f5bef) ) // ~= jsd-20 [2/2]
	ROM_LOAD( "a-25.bin", 0x20000, 0x10000, CRC(d8926711) SHA1(56c2f25e21eacd4fb779fa04ffd06de937c557ef) ) // ~= jsd-21 [1/2]
	ROM_LOAD( "a-26.bin", 0x30000, 0x10000, CRC(41d76447) SHA1(cfced91518859b93b77c9097f0b44adef66c8683) ) // ~= jsd-21 [2/2]
	ROM_LOAD( "a-21.bin", 0x40000, 0x10000, CRC(5af84bd5) SHA1(a0b4dd69c8e0e2f38f67d42dcadb1254299ab649) ) // ~= jsd-22 [1/2]
	ROM_LOAD( "a-24.bin", 0x50000, 0x10000, CRC(09ae3173) SHA1(6c5c49297319decf530f3c0930d5146836d425b1) ) // ~= jsd-22 [2/2]
	ROM_LOAD( "a-27.bin", 0x60000, 0x10000, CRC(c9049e98) SHA1(d24775704a4898293522ea5c2a901c6f457dce75) ) // ~= jsd-23 [1/2]
	ROM_LOAD( "a-28.bin", 0x70000, 0x10000, CRC(b4d12106) SHA1(08018251d10c0f5410779fa68cf95c87ba89ea56) ) // ~= jsd-23 [2/2]

	ROM_REGION( 0x040000, "oki1", ROMREGION_ERASE00 )       /* Samples */
	// unpopulated

	ROM_REGION( 0x040000, "oki2", 0 )       /* Samples */
	ROM_LOAD( "a-29.bin", 0x00000, 0x10000, CRC(0049aa65) SHA1(29efff074e0fd23eb3cc9ccd3a0eae0acc812e39) ) // = jsd-07 [1/2]
	ROM_LOAD( "a-30.bin", 0x10000, 0x10000, CRC(05bce2c7) SHA1(4aaf5156bafb3451492c5053d7d75994a72f8738) ) // = jsd-07 [2/2]
	ROM_LOAD( "b-17.bin", 0x20000, 0x10000, CRC(3e4e34d3) SHA1(3cda83d8f9e9108acbace717f167cccb8adc5b90) ) // = jsd-08 [1/2]
	ROM_LOAD( "b-18.bin", 0x30000, 0x10000, CRC(738a6643) SHA1(d41a0eaf076847d63a9a23db16a99627ec118f97) ) // = jsd-08 [2/2]

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM */
	ROM_LOAD( "prom.14m", 0x0000, 0x0200, CRC(1d877538) SHA1(a5be0dc65dcfc36fbba10d1fddbe155e24b6122f) ) // from parent
ROM_END


/***************************************************************************

                                [ Soldam ]

f00c2.l *** score/10 (BCD) ***

The country code is at ROM address $3a9d, copied to RAM address
f0025: 0 = japan, 1 = USA. Change f0025 to 1 to have all the
text in english.

***************************************************************************/


ROM_START( soldam )
	ROM_REGION( 0x60000, "maincpu", 0 )     /* Main CPU Code */
	ROM_LOAD16_BYTE( "2ver1j.bin",0x000000, 0x020000, CRC(45444b07) SHA1(d991dd52904671fbd8cfcfe07c956d9fd45b3470) )
	ROM_LOAD16_BYTE( "1euro.bin", 0x000001, 0x020000, CRC(9f9da28a) SHA1(8ce9cd72d12cf66f0b1611ec3933383d2995e5f2) )
	ROM_LOAD16_BYTE( "3ver1.bin", 0x040000, 0x010000, CRC(c5382a07) SHA1(5342775f2925772e23bb460e88cd2b7e524e57fa) )
	ROM_LOAD16_BYTE( "4ver1.bin", 0x040001, 0x010000, CRC(1df7816f) SHA1(7c069470ec0e884eae5a52581f2be17d9e692105) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "5ver1.bin", 0x000000, 0x010000, CRC(d1019a67) SHA1(32d77914a67c009bf1bb397772f195594f7cc03f) )
	ROM_LOAD16_BYTE( "6ver1.bin", 0x000001, 0x010000, CRC(3ed219b4) SHA1(afffa5596027181ae94488d54d6266f8a7ead180) )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "14ver1.bin", 0x000000, 0x080000, CRC(73c90610) SHA1(044462fd41e8502ba57f814c61db6f0cfb1cc18b) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "18ver1.bin", 0x000000, 0x080000, CRC(e91a1afd) SHA1(66a686d61de105ee571fbdabc774ed0b848bf426) )

	ROM_REGION( 0x020000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "19ver1.bin", 0x000000, 0x020000, CRC(38465da1) SHA1(461fc0d81b711d0646dc366c057da66d4b8c6e23) )

	ROM_REGION( 0x080000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "23ver1.bin", 0x000000, 0x080000, CRC(0ca09432) SHA1(c9b12d564032c2a668e18ba95fd71ab540e798ce) )

	ROM_REGION( 0x040000, "oki1", 0 )       /* Samples */
	ROM_LOAD( "10ver1.bin", 0x000000, 0x040000, CRC(8d5613bf) SHA1(eee217dd2ab64d86b7f5eda55a3c331d862c079e) )

	ROM_REGION( 0x040000, "oki2", 0 )       /* Samples */
	ROM_LOAD( "8ver1.bin",  0x000000, 0x040000, CRC(fcd36019) SHA1(f4edb55bd62b697c5a73c461008e764c2f16956b) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM */
	ROM_LOAD( "pr-91023.m14",   0x0000, 0x0200, CRC(8914e72d) SHA1(80a664471f14c8ed8544a5e226fdca425ab3c657) )
ROM_END


ROM_START( soldamj )
	ROM_REGION( 0x60000, "maincpu", 0 )     /* Main CPU Code */
	ROM_LOAD16_BYTE( "soldam2.bin", 0x000000, 0x020000, CRC(c73d29e4) SHA1(2a6bffd6835506a0a1449047dec69445d2242fca) )
	ROM_LOAD16_BYTE( "soldam1.bin", 0x000001, 0x020000, CRC(e7cb0c20) SHA1(7b1adf439cd4022ec110ec18359fb50ac137f253) )
	ROM_LOAD16_BYTE( "3ver1.bin",   0x040000, 0x010000, CRC(c5382a07) SHA1(5342775f2925772e23bb460e88cd2b7e524e57fa) )
	ROM_LOAD16_BYTE( "4ver1.bin",   0x040001, 0x010000, CRC(1df7816f) SHA1(7c069470ec0e884eae5a52581f2be17d9e692105) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "5ver1.bin", 0x000000, 0x010000, CRC(d1019a67) SHA1(32d77914a67c009bf1bb397772f195594f7cc03f) )
	ROM_LOAD16_BYTE( "6ver1.bin", 0x000001, 0x010000, CRC(3ed219b4) SHA1(afffa5596027181ae94488d54d6266f8a7ead180) )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "soldam14.bin", 0x000000, 0x080000, CRC(26cea54a) SHA1(00beb1fe2973daf8bab288a0cb9d5fff26a00415) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "soldam18.bin", 0x000000, 0x080000, CRC(7d8e4712) SHA1(d16455648dcba467336e51daac8b23e463a74230) )

	ROM_REGION( 0x020000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "19ver1.bin", 0x000000, 0x020000, CRC(38465da1) SHA1(461fc0d81b711d0646dc366c057da66d4b8c6e23) )

	ROM_REGION( 0x080000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "23ver1.bin", 0x000000, 0x080000, CRC(0ca09432) SHA1(c9b12d564032c2a668e18ba95fd71ab540e798ce) )

	ROM_REGION( 0x040000, "oki1", 0 )       /* Samples */
	ROM_LOAD( "10ver1.bin", 0x000000, 0x040000, CRC(8d5613bf) SHA1(eee217dd2ab64d86b7f5eda55a3c331d862c079e) )

	ROM_REGION( 0x040000, "oki2", 0 )       /* Samples */
	ROM_LOAD( "8ver1.bin",  0x000000, 0x040000, CRC(fcd36019) SHA1(f4edb55bd62b697c5a73c461008e764c2f16956b) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM */
	ROM_LOAD( "pr-91023.m14",   0x0000, 0x0200, CRC(8914e72d) SHA1(80a664471f14c8ed8544a5e226fdca425ab3c657) )
ROM_END


/***************************************************************************

                            [ Takeda Shingen ]

***************************************************************************/


ROM_START( tshingena )
	ROM_REGION( 0x60000, "maincpu", 0 )     /* Main CPU Code */
	ROM_LOAD16_BYTE( "takeda2.bin", 0x000000, 0x020000, CRC(6ddfc9f3) SHA1(0ce1b8eae31453db0b2081717d7dbda9ea7d5a60) )
	ROM_LOAD16_BYTE( "takeda1.bin", 0x000001, 0x020000, CRC(1afc6b7d) SHA1(b56da1b8c5b417a88a2952491c2d5472bb783945) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "takeda5.bin", 0x000000, 0x010000, CRC(fbdc51c0) SHA1(bc6036c556275f7eccd7741d23437a98b0aa13bb) )
	ROM_LOAD16_BYTE( "takeda6.bin", 0x000001, 0x010000, CRC(8fa65b69) SHA1(23a2d60435f235366f877ac79ac1506a99cfae9c) )

	ROM_REGION( 0x1000, "mcu", 0 ) /* M50747 MCU Code */
	ROM_LOAD( "m50747", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "takeda11.bin", 0x000000, 0x020000, CRC(bf0b40a6) SHA1(3634b8700b6cfb71d3796847eab50fd2714d4726) )
	ROM_LOAD( "takeda12.bin", 0x020000, 0x020000, CRC(07987d89) SHA1(54f0fcbac6ec9c27b70a04a192db2874d38e91d8) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "takeda15.bin", 0x000000, 0x020000, CRC(4c316b79) SHA1(1e17cb061e3d06427bef3e8c222f2a7cc80743ff) )
	ROM_LOAD( "takeda16.bin", 0x020000, 0x020000, CRC(ceda9dd6) SHA1(2339cbb3d10fae6cba32b6455511aad60f4e481a) )
	ROM_LOAD( "takeda17.bin", 0x040000, 0x020000, CRC(3d4371dc) SHA1(2a6c34eb9952492f9554e5c810f015496ac205c1) )

	ROM_REGION( 0x020000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "takeda19.bin", 0x000000, 0x010000, CRC(2ca2420d) SHA1(0e9f2f3d8ea2be07193e5a5fd37256a3887e7a2f) )

	ROM_REGION( 0x080000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "takeda20.bin", 0x000000, 0x020000, CRC(1bfd636f) SHA1(d56eb7538195930ea3cf16788e3128a6262675e3) )
	ROM_LOAD( "takeda21.bin", 0x020000, 0x020000, CRC(12fb006b) SHA1(1dae7f3bdbfefb3424d07572c398e4edd274a4ae) )
	ROM_LOAD( "takeda22.bin", 0x040000, 0x020000, CRC(b165b6ae) SHA1(b987cdb5a71882d2495ac449a78705ba996943a7) )
	ROM_LOAD( "takeda23.bin", 0x060000, 0x020000, CRC(37cb9214) SHA1(b98c361cd8b19b85cd16f040c415550b3750ab9e) )

	ROM_REGION( 0x040000, "oki1", 0 )       /* Samples */
	ROM_LOAD( "takeda9.bin",  0x000000, 0x020000, CRC(db7f3f4f) SHA1(2667bab60abe49b3a9ce4ff63948e274578062b7) )
	ROM_LOAD( "takeda10.bin", 0x020000, 0x020000, CRC(c9959d71) SHA1(afbb756b3b4730262055f80995bd4ba5f9031d26) )

	ROM_REGION( 0x040000, "oki2", 0 )       /* Samples */
	ROM_LOAD( "shing_07.rom",  0x000000, 0x020000, CRC(c37ecbdc) SHA1(fcae2bbc4a10ee0fa18c9a850ae451c0600ea98c) )
	ROM_LOAD( "shing_08.rom",  0x020000, 0x020000, CRC(36d56c8c) SHA1(391f8c6b3ee605ce846f1862b0d3b14694dbd556) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM */
	ROM_LOAD( "ts.bpr",        0x0000, 0x0200, CRC(85b30ac4) SHA1(b03f577ceb0f26b67453ffa52ef61fea76a93184) )
ROM_END


ROM_START( tshingen )
	ROM_REGION( 0x60000, "maincpu", 0 )     /* Main CPU Code */
	ROM_LOAD16_BYTE( "shing_02.rom", 0x000000, 0x020000, CRC(d9ab5b78) SHA1(c7622ec11a636dc7a6bcad02556a98aa0a9fb043) )
	ROM_LOAD16_BYTE( "shing_01.rom", 0x000001, 0x020000, CRC(a9d2de20) SHA1(b53205722ae19305a1c373abbbac4fbcbcb0b0f0) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Sound CPU Code */
	ROM_LOAD16_BYTE( "takeda5.bin", 0x000000, 0x010000, CRC(fbdc51c0) SHA1(bc6036c556275f7eccd7741d23437a98b0aa13bb) )
	ROM_LOAD16_BYTE( "takeda6.bin", 0x000001, 0x010000, CRC(8fa65b69) SHA1(23a2d60435f235366f877ac79ac1506a99cfae9c) )

	ROM_REGION( 0x1000, "mcu", 0 ) /* M50747 MCU Code */
	ROM_LOAD( "m50747", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* Scroll 0 */
	ROM_LOAD( "takeda11.bin", 0x000000, 0x020000, CRC(bf0b40a6) SHA1(3634b8700b6cfb71d3796847eab50fd2714d4726) )
	ROM_LOAD( "shing_12.rom", 0x020000, 0x020000, CRC(5e4adedb) SHA1(0b67af2913e1f15d0a9d81e7b22d26a1011fa160) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* Scroll 1 */
	ROM_LOAD( "shing_15.rom", 0x000000, 0x020000, CRC(9db18233) SHA1(d323035505159e420430d178b4fa3a972aed80cd) )
	ROM_LOAD( "takeda16.bin", 0x020000, 0x020000, CRC(ceda9dd6) SHA1(2339cbb3d10fae6cba32b6455511aad60f4e481a) )
	ROM_LOAD( "takeda17.bin", 0x040000, 0x020000, CRC(3d4371dc) SHA1(2a6c34eb9952492f9554e5c810f015496ac205c1) )

	ROM_REGION( 0x020000, "gfx3", 0 ) /* Scroll 2 */
	ROM_LOAD( "shing_19.rom", 0x000000, 0x010000, CRC(97282d9d) SHA1(c977fb6cebbb9f4097b3c25f2b7f5cb2440c191b) )

	ROM_REGION( 0x080000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD( "shing_20.rom", 0x000000, 0x020000, CRC(7f6f8384) SHA1(77ccd8ca3cf35040e9cc8c78b2489864f60be699) )
	ROM_LOAD( "takeda21.bin", 0x020000, 0x020000, CRC(12fb006b) SHA1(1dae7f3bdbfefb3424d07572c398e4edd274a4ae) )
	ROM_LOAD( "takeda22.bin", 0x040000, 0x020000, CRC(b165b6ae) SHA1(b987cdb5a71882d2495ac449a78705ba996943a7) )
	ROM_LOAD( "takeda23.bin", 0x060000, 0x020000, CRC(37cb9214) SHA1(b98c361cd8b19b85cd16f040c415550b3750ab9e) )

	ROM_REGION( 0x040000, "oki1", 0 )       /* Samples */
	ROM_LOAD( "takeda9.bin",  0x000000, 0x020000, CRC(db7f3f4f) SHA1(2667bab60abe49b3a9ce4ff63948e274578062b7) )
	ROM_LOAD( "takeda10.bin", 0x020000, 0x020000, CRC(c9959d71) SHA1(afbb756b3b4730262055f80995bd4ba5f9031d26) )

	ROM_REGION( 0x040000, "oki2", 0 )       /* Samples */
	ROM_LOAD( "shing_07.rom",  0x000000, 0x020000, CRC(c37ecbdc) SHA1(fcae2bbc4a10ee0fa18c9a850ae451c0600ea98c) )
	ROM_LOAD( "shing_08.rom",  0x020000, 0x020000, CRC(36d56c8c) SHA1(391f8c6b3ee605ce846f1862b0d3b14694dbd556) )

	ROM_REGION( 0x0200, "proms", 0 )        /* Priority PROM */
	ROM_LOAD( "ts.bpr",        0x0000, 0x0200, CRC(85b30ac4) SHA1(b03f577ceb0f26b67453ffa52ef61fea76a93184) )
ROM_END


void megasys1_state::rodland_gfx_unmangle(const char *region)
{
	UINT8 *rom = memregion(region)->base();
	int size = memregion(region)->bytes();
	int i;

	/* data lines swap: 76543210 -> 64537210 */
	for (i = 0;i < size;i++)
		rom[i] =   (rom[i] & 0x27)
				| ((rom[i] & 0x80) >> 4)
				| ((rom[i] & 0x48) << 1)
				| ((rom[i] & 0x10) << 2);

	dynamic_buffer buffer(size);

	memcpy(&buffer[0],rom,size);

	/* address lines swap: ..dcba9876543210 -> ..acb8937654d210 */
	for (i = 0;i < size;i++)
	{
		int a =    (i &~0x2508)
				| ((i & 0x2000) >> 10)
				| ((i & 0x0400) << 3)
				| ((i & 0x0100) << 2)
				| ((i & 0x0008) << 5);
		rom[i] = buffer[a];
	}
}

void megasys1_state::jitsupro_gfx_unmangle(const char *region)
{
	UINT8 *rom = memregion(region)->base();
	int size = memregion(region)->bytes();
	int i;

	/* data lines swap: 76543210 -> 43576210 */
	for (i = 0;i < size;i++)
		rom[i] =   BITSWAP8(rom[i],0x4,0x3,0x5,0x7,0x6,0x2,0x1,0x0);

	dynamic_buffer buffer(size);

	memcpy(&buffer[0],rom,size);

	/* address lines swap: fedcba9876543210 -> fe8cb39d7654a210 */
	for (i = 0;i < size;i++)
	{
		int a = (i & ~0xffff) |
	BITSWAP16(i,0xf,0xe,0x8,0xc,0xb,0x3,0x9,0xd,0x7,0x6,0x5,0x4,0xa,0x2,0x1,0x0);

		rom[i] = buffer[a];
	}
}

void megasys1_state::stdragona_gfx_unmangle(const char *region)
{
	UINT8 *rom = memregion(region)->base();
	int size = memregion(region)->bytes();
	int i;

	/* data lines swap: 76543210 -> 37564210 */
	for (i = 0;i < size;i++)
		rom[i] =   BITSWAP8(rom[i],3,7,5,6,4,2,1,0);

	dynamic_buffer buffer(size);

	memcpy(&buffer[0],rom,size);

	/* address lines swap: fedcba9876543210 -> fe3cbd9a76548210 */
	for (i = 0;i < size;i++)
	{
		int a = (i & ~0xffff) |
	BITSWAP16(i,0xf,0xe,0x3,0xc,0xb,0xd,0x9,0xa,0x7,0x6,0x5,0x4,0x8,0x2,0x1,0x0);

		rom[i] = buffer[a];
	}
}

/*************************************
 *
 *  Game-specific driver inits
 *
 *************************************/

/*
    MCU handshake sequence:
    the M50747 MCU can overlay 0x40 bytes of data inside the ROM space.
    The offset where this happens is given by m68k to MCU write [0x8/2] << 6.
    For example stdragon writes 0x33e -> maps at 0xcf80-0xcfbf while stdragona writes 0x33f -> maps at 0xcfc0-0xcfff.
    Note: stdragona forgets to turn off the overlay before the ROM check in service mode (hence it reports an error).
*/

#define MCU_HS_LOG 0

#define MCU_HS_SEQ(_1_,_2_,_3_,_4_) \
	(m_mcu_hs_ram[0/2] == _1_ && \
		m_mcu_hs_ram[2/2] == _2_ && \
		m_mcu_hs_ram[4/2] == _3_ && \
		m_mcu_hs_ram[6/2] == _4_)

DRIVER_INIT_MEMBER(megasys1_state,64street)
{
//  UINT16 *ROM = (UINT16 *) memregion("maincpu")->base();
//  ROM[0x006b8/2] = 0x6004;        // d8001 test
//  ROM[0x10EDE/2] = 0x6012;        // watchdog

	m_ip_select_values[0] = 0x57;
	m_ip_select_values[1] = 0x53;
	m_ip_select_values[2] = 0x54;
	m_ip_select_values[3] = 0x55;
	m_ip_select_values[4] = 0x56;
}

READ16_MEMBER(megasys1_state::megasys1A_mcu_hs_r)
{
	if(m_mcu_hs && ((m_mcu_hs_ram[8/2] << 6) & 0x3ffc0) == ((offset*2) & 0x3ffc0))
	{
		if(MCU_HS_LOG && !space.debugger_access())
			printf("MCU HS R (%04x) <- [%02x]\n",mem_mask,offset*2);

		return 0x889e;
	}

	return m_rom_maincpu[offset];
}

WRITE16_MEMBER(megasys1_state::megasys1A_mcu_hs_w)
{
	// following is hachoo, other games differs slightly
	// R 0x5f0, if bit 0 == 0 then skips hs seq (debug?)
	// [0/2]: 0x00ff
	// [2/2]: 0x0055
	// [4/2]: 0x00aa
	// [6/2]: 0x0000
	// [8/2]: 0x0fff
	// R 0x5f0, if bit 1 == 0 then goes further (debug again?)
	// R 0x3ffc0, compares with seed 0x889e

	COMBINE_DATA(&m_mcu_hs_ram[offset]);

	if(MCU_HS_SEQ(0x00ff,0x0055,0x00aa,0x0000) && offset == 0x8/2)
		m_mcu_hs = 1;
	else
		m_mcu_hs = 0;

	if(MCU_HS_LOG && !space.debugger_access())
		printf("MCU HS W %04x (%04x) -> [%02x]\n",data,mem_mask,offset*2);
}

DRIVER_INIT_MEMBER(megasys1_state,astyanax)
{
	astyanax_rom_decode(machine(), "maincpu");
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00000, 0x3ffff, read16_delegate(FUNC(megasys1_state::megasys1A_mcu_hs_r),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x20000, 0x20009, write16_delegate(FUNC(megasys1_state::megasys1A_mcu_hs_w),this));
}

DRIVER_INIT_MEMBER(megasys1_state,avspirit)
{
	m_ip_select_values[0] = 0x37;
	m_ip_select_values[1] = 0x35;
	m_ip_select_values[2] = 0x36;
	m_ip_select_values[3] = 0x33;
	m_ip_select_values[4] = 0x34;

	// has twice less RAM
	m_maincpu->space(AS_PROGRAM).unmap_readwrite(0x060000, 0x06ffff);
	m_maincpu->space(AS_PROGRAM).install_ram(0x070000, 0x07ffff, m_ram);
}

DRIVER_INIT_MEMBER(megasys1_state,bigstrik)
{
	m_ip_select_values[0] = 0x58;
	m_ip_select_values[1] = 0x54;
	m_ip_select_values[2] = 0x55;
	m_ip_select_values[3] = 0x56;
	m_ip_select_values[4] = 0x57;
}

DRIVER_INIT_MEMBER(megasys1_state,chimerab)
{
	/* same as cybattlr */
	m_ip_select_values[0] = 0x56;
	m_ip_select_values[1] = 0x52;
	m_ip_select_values[2] = 0x53;
	m_ip_select_values[3] = 0x54;
	m_ip_select_values[4] = 0x55;
}

DRIVER_INIT_MEMBER(megasys1_state,cybattlr)
{
	m_ip_select_values[0] = 0x56;
	m_ip_select_values[1] = 0x52;
	m_ip_select_values[2] = 0x53;
	m_ip_select_values[3] = 0x54;
	m_ip_select_values[4] = 0x55;
}

DRIVER_INIT_MEMBER(megasys1_state,edf)
{
	m_ip_select_values[0] = 0x20;
	m_ip_select_values[1] = 0x21;
	m_ip_select_values[2] = 0x22;
	m_ip_select_values[3] = 0x23;
	m_ip_select_values[4] = 0x24;
}

READ16_MEMBER(megasys1_state::edfbl_input_r)
{
	ioport_port *in_names[] = { m_io_system, m_io_p1, m_io_p2, m_io_dsw1, m_io_dsw2 };
	UINT16 res;

	res = 0;

	switch(offset)
	{
		case 0x02/2:
		case 0x04/2:
		case 0x06/2:
		case 0x08/2:
		case 0x0a/2: res = in_names[offset-1]->read(); break;
	}

	return res;
}

DRIVER_INIT_MEMBER(megasys1_state,edfbl)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xe0000, 0xe000f, read16_delegate(FUNC(megasys1_state::edfbl_input_r),this));
	//m_maincpu->space(AS_PROGRAM).install_legacy_write_handler(*m_oki1, 0xe000e, 0xe000f, FUNC(soundlatch_byte_w));
}

DRIVER_INIT_MEMBER(megasys1_state,hayaosi1)
{
	m_ip_select_values[0] = 0x51;
	m_ip_select_values[1] = 0x52;
	m_ip_select_values[2] = 0x53;
	m_ip_select_values[3] = 0x54;
	m_ip_select_values[4] = 0x55;
}

READ16_MEMBER(megasys1_state::iganinju_mcu_hs_r)
{
	if(m_mcu_hs && ((m_mcu_hs_ram[8/2] << 6) & 0x3ffc0) == ((offset*2) & 0x3ffc0))
	{
		if(MCU_HS_LOG && !space.debugger_access())
			printf("MCU HS R (%04x) <- [%02x]\n",mem_mask,offset*2);

		return 0x835d;
	}

	return m_rom_maincpu[offset];
}

WRITE16_MEMBER(megasys1_state::iganinju_mcu_hs_w)
{
	// [0/2]: 0x0000
	// [2/2]: 0x0055
	// [4/2]: 0x00aa
	// [6/2]: 0x00ff
	// [8/2]: 0x0bc0
	// expects 0x835d to be read at 0x2f000, does hs sequence until that happens

	COMBINE_DATA(&m_mcu_hs_ram[offset]);

	if(MCU_HS_SEQ(0x0000,0x0055,0x00aa,0x00ff) && offset == 0x8/2)
		m_mcu_hs = 1;
	else
		m_mcu_hs = 0;

	if(MCU_HS_LOG && !space.debugger_access())
		printf("MCU HS W %04x (%04x) -> [%02x]\n",data,mem_mask,offset*2);
}

DRIVER_INIT_MEMBER(megasys1_state,iganinju)
{
	phantasm_rom_decode(machine(), "maincpu");

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00000, 0x3ffff, read16_delegate(FUNC(megasys1_state::iganinju_mcu_hs_r),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x2f000, 0x2f009, write16_delegate(FUNC(megasys1_state::iganinju_mcu_hs_w),this));

	//m_rom_maincpu[0x00006e/2] = 0x0420; // the only game that does
										// not like lev 3 interrupts
}

// jitsupro writes oki commands to both the lsb and msb; it works because of byte smearing
WRITE16_MEMBER(megasys1_state::okim6295_both_1_w)
{
	m_oki1->write_command(data & 0xff);
}
WRITE16_MEMBER(megasys1_state::okim6295_both_2_w)
{
	m_oki2->write_command(data & 0xff);
}

DRIVER_INIT_MEMBER(megasys1_state,jitsupro)
{
	astyanax_rom_decode(machine(), "maincpu");      // Code

	jitsupro_gfx_unmangle("gfx1");   // Gfx
	jitsupro_gfx_unmangle("gfx4");
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00000, 0x3ffff, read16_delegate(FUNC(megasys1_state::megasys1A_mcu_hs_r),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x20000, 0x20009, write16_delegate(FUNC(megasys1_state::megasys1A_mcu_hs_w),this));

	m_audiocpu->space(AS_PROGRAM).install_write_handler(0xa0000, 0xa0003, write16_delegate(FUNC(megasys1_state::okim6295_both_1_w),this));
	m_audiocpu->space(AS_PROGRAM).install_write_handler(0xc0000, 0xc0003, write16_delegate(FUNC(megasys1_state::okim6295_both_2_w),this));
}

DRIVER_INIT_MEMBER(megasys1_state,peekaboo)
{
	UINT8 *ROM = memregion("oki1")->base();
	memory_bank *okibank = membank("okibank");

	okibank->configure_entry(7, &ROM[0x20000]);
	okibank->configure_entries(0, 7, &ROM[0x20000], 0x20000);

	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x100000, 0x100001, read16_delegate(FUNC(megasys1_state::protection_peekaboo_r),this), write16_delegate(FUNC(megasys1_state::protection_peekaboo_w),this));
}

DRIVER_INIT_MEMBER(megasys1_state,phantasm)
{
	phantasm_rom_decode(machine(), "maincpu");
}

DRIVER_INIT_MEMBER(megasys1_state,rodland)
{
	rodland_gfx_unmangle("gfx1");
	rodland_gfx_unmangle("gfx4");

	rodland_rom_decode(machine(), "maincpu");
}

DRIVER_INIT_MEMBER(megasys1_state,rodlandj)
{
	rodland_gfx_unmangle("gfx1");
	rodland_gfx_unmangle("gfx4");

	astyanax_rom_decode(machine(), "maincpu");
}

DRIVER_INIT_MEMBER(megasys1_state,rodlandjb)
{
	rodland_gfx_unmangle("gfx1");
	rodland_gfx_unmangle("gfx4");
}

DRIVER_INIT_MEMBER(megasys1_state,rittam)
{
	astyanax_rom_decode(machine(), "maincpu");
}

READ16_MEMBER(megasys1_state::soldamj_spriteram16_r)
{
	return m_spriteram[offset];
}

WRITE16_MEMBER(megasys1_state::soldamj_spriteram16_w)
{
	if (offset < 0x800/2)   COMBINE_DATA(&m_spriteram[offset]);
}

DRIVER_INIT_MEMBER(megasys1_state,soldamj)
{
	astyanax_rom_decode(machine(), "maincpu");
	/* Sprite RAM is mirrored */
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x8c000, 0x8cfff, read16_delegate(FUNC(megasys1_state::soldamj_spriteram16_r),this), write16_delegate(FUNC(megasys1_state::soldamj_spriteram16_w),this));
}

DRIVER_INIT_MEMBER(megasys1_state,soldam)
{
	phantasm_rom_decode(machine(), "maincpu");
	/* Sprite RAM is mirrored */
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x8c000, 0x8cfff, read16_delegate(FUNC(megasys1_state::soldamj_spriteram16_r),this), write16_delegate(FUNC(megasys1_state::soldamj_spriteram16_w),this));
}


READ16_MEMBER(megasys1_state::stdragon_mcu_hs_r)
{
	if(m_mcu_hs && ((m_mcu_hs_ram[8/2] << 6) & 0x3ffc0) == ((offset*2) & 0x3ffc0))
	{
		if(MCU_HS_LOG && !space.debugger_access())
			printf("MCU HS R (%04x) <- [%02x]\n",mem_mask,offset*2);

		return 0x835d;
	}

	return m_rom_maincpu[offset];
}

WRITE16_MEMBER(megasys1_state::stdragon_mcu_hs_w)
{
	COMBINE_DATA(&m_mcu_hs_ram[offset]);

	if(MCU_HS_SEQ(0x0000,0x0055,0x00aa,0x00ff) && offset == 0x8/2)
		m_mcu_hs = 1;
	else
		m_mcu_hs = 0;

	if(MCU_HS_LOG && !space.debugger_access())
		printf("MCU HS W %04x (%04x) -> [%02x]\n",data,mem_mask,offset*2);
}


DRIVER_INIT_MEMBER(megasys1_state,stdragon)
{
	phantasm_rom_decode(machine(), "maincpu");
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00000, 0x3ffff, read16_delegate(FUNC(megasys1_state::stdragon_mcu_hs_r),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x23ff0, 0x23ff9, write16_delegate(FUNC(megasys1_state::stdragon_mcu_hs_w),this));
}

DRIVER_INIT_MEMBER(megasys1_state,stdragona)
{
	phantasm_rom_decode(machine(), "maincpu");

	stdragona_gfx_unmangle("gfx1");
	stdragona_gfx_unmangle("gfx4");

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x00000, 0x3ffff, read16_delegate(FUNC(megasys1_state::stdragon_mcu_hs_r),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x23ff0, 0x23ff9, write16_delegate(FUNC(megasys1_state::stdragon_mcu_hs_w),this));
}

DRIVER_INIT_MEMBER(megasys1_state,stdragonb)
{
	stdragona_gfx_unmangle("gfx1");
	stdragona_gfx_unmangle("gfx4");
}

READ16_MEMBER(megasys1_state::monkelf_input_r)
{
	ioport_port *in_names[] = { m_io_p1, m_io_p2, m_io_dsw1, m_io_dsw2, m_io_system };
	UINT16 res;

	res = 0xffff;

	switch(offset)
	{
		case 0x02/2:
		case 0x04/2:
		case 0x06/2:
		case 0x08/2:
		case 0x0a/2: res = in_names[offset-1]->read(); break;
	}

	return res;
}

DRIVER_INIT_MEMBER(megasys1_state,monkelf)
{
	DRIVER_INIT_CALL(avspirit);

	m_rom_maincpu[0x00744/2] = 0x4e71; // weird check, 0xe000e R is a port-based trap?

	m_maincpu->space(AS_PROGRAM).install_read_handler(0xe0000, 0xe000f, read16_delegate(FUNC(megasys1_state::monkelf_input_r),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x44000, 0x443ff, write16_delegate(FUNC(megasys1_state::megasys1_vregs_monkelf_w),this));

	// convert bootleg priority format to standard
	{
		int i;
		UINT8 *ROM = memregion("proms")->base();

		for (i = 0x1fe; i >= 0; i -= 2) {
			ROM[i+0] = ROM[i+1] = (ROM[i/2] >> 4) & 0x0f;
		}
	}

	megasys1_priority_create();
}

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

// Type Z
GAME( 1988, lomakai,  0,        system_Z,          lomakai,  driver_device,  0,        ROT0,   "Jaleco", "Legend of Makai (World)", 0 )
GAME( 1988, makaiden, lomakai,  system_Z,          lomakai,  driver_device,  0,        ROT0,   "Jaleco", "Makai Densetsu (Japan)", 0 )

// Type A
GAME( 1988, p47,      0,        system_A,          p47,      driver_device,  0,        ROT0,   "Jaleco", "P-47 - The Phantom Fighter (World)", 0 )
GAME( 1988, p47j,     p47,      system_A,          p47,      driver_device,  0,        ROT0,   "Jaleco", "P-47 - The Freedom Fighter (Japan)", 0 )
GAME( 1988, p47je,    p47,      system_A,          p47,      driver_device,  0,        ROT0,   "Jaleco", "P-47 - The Freedom Fighter (Japan, Export)", 0 )
GAME( 1988, kickoff,  0,        system_A,          kickoff,  driver_device,  0,        ROT0,   "Jaleco", "Kick Off (Japan)", 0 )
GAME( 1988, tshingen, 0,        system_A,          tshingen, megasys1_state, phantasm, ROT0,   "Jaleco", "Shingen Samurai-Fighter (Japan, English)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1988, tshingena,tshingen, system_A,          tshingen, megasys1_state, phantasm, ROT0,   "Jaleco", "Takeda Shingen (Japan, Japanese)", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1988, kazan,    0,        system_A,          kazan,    megasys1_state, iganinju, ROT0,   "Jaleco", "Ninja Kazan (World)", 0 )
GAME( 1988, iganinju, kazan,    system_A,          kazan,    megasys1_state, iganinju, ROT0,   "Jaleco", "Iga Ninjyutsuden (Japan)", 0 )
GAME( 1989, astyanax, 0,        system_A,          astyanax, megasys1_state, astyanax, ROT0,   "Jaleco", "The Astyanax", 0 )
GAME( 1989, lordofk,  astyanax, system_A,          astyanax, megasys1_state, astyanax, ROT0,   "Jaleco", "The Lord of King (Japan)", 0 )
GAME( 1989, hachoo,   0,        system_A_hachoo,   hachoo,   megasys1_state, astyanax, ROT0,   "Jaleco", "Hachoo!", 0 )
GAME( 1989, jitsupro, 0,        system_A,          jitsupro, megasys1_state, jitsupro, ROT0,   "Jaleco", "Jitsuryoku!! Pro Yakyuu (Japan)", 0 )
GAME( 1989, plusalph, 0,        system_A,          plusalph, megasys1_state, astyanax, ROT270, "Jaleco", "Plus Alpha", 0 )
GAME( 1989, stdragon, 0,        system_A,          stdragon, megasys1_state, stdragon, ROT0,   "Jaleco", "Saint Dragon (set 1)", 0 )
GAME( 1989, stdragona,stdragon, system_A,          stdragon, megasys1_state, stdragona,ROT0,   "Jaleco", "Saint Dragon (set 2)", 0 )
GAME( 1989, stdragonb,stdragon, system_A,          stdragon, megasys1_state, stdragonb,ROT0,   "bootleg","Saint Dragon (bootleg)", 0 )
GAME( 1990, rodland,  0,        system_A,          rodland,  megasys1_state, rodland,  ROT0,   "Jaleco", "Rod-Land (World)", 0 )
GAME( 1990, rodlandj, rodland,  system_A,          rodland,  megasys1_state, rodlandj, ROT0,   "Jaleco", "Rod-Land (Japan)", 0 )
GAME( 1990, rittam,   rodland,  system_A,          rodland,  megasys1_state, rittam,   ROT0,   "Jaleco", "R&T (Rod-Land prototype?)", 0 )
GAME( 1990, rodlandjb,rodland,  system_A,          rodland,  megasys1_state,  rodlandjb,        ROT0,   "bootleg","Rod-Land (Japan bootleg)", 0 )
GAME( 1990, phantasm, avspirit, system_A,          phantasm, megasys1_state, phantasm, ROT0,   "Jaleco", "Phantasm (Japan)", 0 )
GAME( 1992, soldam,   0,        system_A,          soldam,   megasys1_state, soldam,   ROT0,   "Jaleco", "Soldam", 0 )
GAME( 1992, soldamj,  soldam,   system_A,          soldam,   megasys1_state, soldamj,  ROT0,   "Jaleco", "Soldam (Japan)", 0 )

// Type B
GAME( 1991, avspirit, 0,        system_B,          avspirit, megasys1_state, avspirit, ROT0,   "Jaleco", "Avenging Spirit", 0 )
GAME( 1990, monkelf,  avspirit, system_B,          avspirit, megasys1_state, monkelf,  ROT0,   "bootleg","Monky Elf (Korean bootleg of Avenging Spirit)", 0 )
GAME( 1991, edf,      0,        system_B,          edf,      megasys1_state, edf,      ROT0,   "Jaleco", "E.D.F. : Earth Defense Force (set 1)", 0 )
GAME( 1991, edfa,     edf,      system_B,          edf,      megasys1_state, edf,      ROT0,   "Jaleco", "E.D.F. : Earth Defense Force (set 2)", 0 )
GAME( 1991, edfu,     edf,      system_B,          edf,      megasys1_state, edf,      ROT0,   "Jaleco", "E.D.F. : Earth Defense Force (North America)", 0 )
GAME( 1991, edfbl,    edf,      system_Bbl,        edf,      megasys1_state, edfbl,    ROT0,   "bootleg","E.D.F. : Earth Defense Force (bootleg)", MACHINE_NO_SOUND )
GAME( 1993, hayaosi1, 0,        system_B_hayaosi1, hayaosi1, megasys1_state, hayaosi1, ROT0,   "Jaleco", "Hayaoshi Quiz Ouza Ketteisen - The King Of Quiz", MACHINE_IMPERFECT_GRAPHICS )

// Type C
GAME( 1991, 64street, 0,        system_C,          64street, megasys1_state, 64street, ROT0,   "Jaleco", "64th. Street - A Detective Story (World)", 0 )
GAME( 1991, 64streetj,64street, system_C,          64street, megasys1_state, 64street, ROT0,   "Jaleco", "64th. Street - A Detective Story (Japan)", 0 )
GAME( 1992, bigstrik, 0,        system_C,          bigstrik, megasys1_state, bigstrik, ROT0,   "Jaleco", "Big Striker", 0 )
GAME( 1993, chimerab, 0,        system_C,          chimerab, megasys1_state, chimerab, ROT0,   "Jaleco", "Chimera Beast (prototype)", 0 )
GAME( 1993, cybattlr, 0,        system_C,          cybattlr, megasys1_state, cybattlr, ROT90,  "Jaleco", "Cybattler", 0 )

// Type D
GAME( 1993, peekaboo, 0,        system_D,          peekaboo, megasys1_state, peekaboo, ROT0,   "Jaleco", "Peek-a-Boo!", 0 )
GAME( 1993, peekaboou,peekaboo, system_D,          peekaboo, megasys1_state, peekaboo, ROT0,   "Jaleco", "Peek-a-Boo! (North America, ver 1.0)", 0 )

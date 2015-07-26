// license:BSD-3-Clause
// copyright-holders:Victor Trucco,Steve Ellenoff,Phil Stroffolino,Tatsuyuki Satoh,Tomasz Slanina,Nicola Salmoria
/***************************************************************************

Ping Pong King  (c) Taito 1985
Gladiator       (c) Taito 1986 - developed by Allumer

Credits:
- Victor Trucco: original emulation and MAME driver
- Steve Ellenoff: YM2203 Sound, ADPCM Sound, dip switch fixes, high score save,
          input port patches, panning fix, sprite banking,
          Golden Castle Rom Set Support
- Phil Stroffolino: palette, sprites, misc video driver fixes
- Tatsuyuki Satoh: YM2203 sound improvements, NEC 8741 simulation, ADPCM with MC6809
- Tomasz Slanina   preliminary Ping Pong King driver
- Nicola Salmoria  clean up

special thanks to:
- Camilty for precious hardware information and screenshots
- Jason Richmond for hardware information and misc. notes
- Joe Rounceville for schematics
- and everyone else who's offered support along the way!


***************************************************************************

PING PONG KING
TAITO 1985
M6100094A

-------------
P0-003


X Q0_10 Q0_9 Q0_8 Q0_7 Q0_6 6116 Q0_5 Q0_4 Q0_3 Q1_2 Q1_1
                                                 Q1
                                   Z80B
                                       8251
 2009 2009 2009
               AQ-001
          Q2
              2116
              2116
                                 AQ-003
-------------
P1-004


 QO_15 TMM2009 Q0_14 Q0_13 Q0_12     2128 2128


                                                           SW1
                                                    AQ-004

                                              SW2          SW3
   6809 X Q0_19 Q0_18
                           Z80  Q0_17 Q0_16 2009 AQ-003 YM2203


***************************************************************************

Gladiator Memory Map
--------------------

Main CPU (Z80)
--------------
The address decoding is done by two PROMs, Q3 @ 2B and Q4 @ 5S.

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
00xxxxxxxxxxxxxx R   xxxxxxxx ROM 1F    program ROM
010xxxxxxxxxxxxx R   xxxxxxxx ROM 1D    program ROM
011xxxxxxxxxxxxx R   xxxxxxxx ROM 1A    program ROM (paged)
10xxxxxxxxxxxxxx R   xxxxxxxx ROM 1C    program ROM (paged)
110000xxxxxxxxxx R/W xxxxxxxx RAM 4S    sprite RAM [1]
110001xxxxxxxxxx R/W xxxxxxxx RAM 4U    sprite RAM
110010xxxxxxxxxx R/W xxxxxxxx RAM 4T    sprite RAM
110011000-------   W xxxxxxxx VCSA2     fg relative scroll Y?
110011001-------   W ------xx           fg tile bank select
110011001-------   W -----x--           bg scroll X msb
110011001-------   W ----x---           fg scroll X msb
110011001-------   W ---x----           bg tile bank select
110011001-------   W --x-----           blank screen?
11001101--------   W xxxxxxxx VCSA1     fg scroll X
11001110--------   W xxxxxxxx           fg+bg scroll Y?
11001111--------   W xxxxxxxx VCSA3     bg scroll X
11010xxxxxxxxxxx R/W xxxxxxxx CCS       palette RAM
11011xxxxxxxxxxx R/W xxxxxxxx VCS1      bg tilemap RAM
11100xxxxxxxxxxx R/W xxxxxxxx VCS2      bg tilemap RAM
11101xxxxxxxxxxx R/W xxxxxxxx VCS3      fg tilemap RAM
11110xxxxxxxxxxx R/W xxxxxxxx RAM 1H    work RAM (battery backed)
11111-----------              n.c.

[1] only the first 256 bytes of each RAM actually contain sprite data (two buffers
    of 128 bytes).

I/O ports:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
--------000--000   W -------x OBJACS ?  select active sprite buffer
--------000--001   W -------x OBJCGBK   sprite bank select
--------000--010   W -------x PR.BK     ROM bank select
--------000--011   W -------x NMIFG     NMI enable/acknowledge (NMI not used by Gladiator)
--------000--100   W -------x SRST      reset sound CPU
--------000--101   W -------x CBK0      unknown
--------000--110   W -------x LOBJ      unknown (related to sprites)
--------000--111   W -------x REVERS    flip screen
--------001-----              n.c.
--------010-----              n.c.
--------011-----              n.c.
--------100----x R/W xxxxxxxx CIOMCS    8741 #0 (communication with 2nd Z80, DIPSW1)
--------101----- R/W          ARST      watchdog reset
--------110----x R/W xxxxxxxx SI/.CS    8251 (serial communication for debug purposes)
--------111-----              n.c.


Sound CPU (Z80)
---------------

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
00xxxxxxxxxxxxxx R   xxxxxxxx ROM 6F    program ROM
01xxxxxxxxxxxxxx R   xxxxxxxx ROM 6E    program ROM
10----xxxxxxxxxx R/W xxxxxxxx RAM 6D    work RAM
11--------------              n.c.

I/O ports:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
--------000----x R/W xxxxxxxx OPNCS     YM2203 (portA: SSRST, unknown; portB: DIPSW3)
--------001----x R/W xxxxxxxx CIOSCS    8741 #1 (communication with main Z80, DIPSW2)
--------010----- R/W -------- INTCS     irq enable/acknowledge
--------011----x R/W xxxxxxxx AX1       8741 #2 (digital inputs, coins)
--------100----x R/W xxxxxxxx AX2       8741 #3 (digital inputs)
--------101--xxx   W -------x FCS       control filters in sound output section
--------110-----              n.c.
--------111-----   W xxxxxxxx ADCS      command to 6809


Third CPU (6809)
----------------

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
0000------------              n.c.
0001------------   W ----xxxx           MSM5205 data
0001------------   W ---x----           MSM5205 clock
0001------------   W --x-----           MSM5205 reset
0001------------   W -x------ ADBR      ROM bank select
0010------------ R   xxxxxxxx           command from Z80
0011------------              n.c.
01xxxxxxxxxxxxxx R   xxxxxxxx ROM 5P    program ROM (banked)
10xxxxxxxxxxxxxx R   xxxxxxxx ROM 5N    program ROM (banked)
11xxxxxxxxxxxxxx R   xxxxxxxx ROM 5M    program ROM (banked)


***************************************************************************

Notes:
------
- The fg tilemap is a 1bpp layer which selects the second palette bank when
  active, so it could be used for some cool effects. Gladiator just sets the
  whole palette to white so we can just treat it as a monochromatic layer.
- tilemap Y scroll is not implemented because the game doesn't use it so I can't
  verify it's right.

TODO:
-----
- gladiatr_irq_patch_w, which triggers irq on the second CPU, is a kludge. It
  shouldn't work that way, that address should actually reset the second CPU
  (but the main CPU never asserts the line). The schematics are too fuzzy to
  understand what should trigger irq on the second CPU. Just using a vblank
  interrupt doesn't work, probably because the CPU expects interrupts to only
  begin happening when the main CPU has finished the self test.
- YM2203 mixing problems (loss of bass notes)
- YM2203 some sound effects just don't sound correct
- Audio Filter Switch not hooked up (might solve YM2203 mixing issue)
- Ports 60,61,80,81 not fully understood yet...
- The four 8741 ROMs are available but not used.


***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/tait8741.h"
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"
#include "sound/msm5205.h"
#include "machine/nvram.h"
#include "includes/gladiatr.h"


/*Rom bankswitching*/
WRITE8_MEMBER(gladiatr_state::gladiatr_bankswitch_w)
{
	membank("bank1")->set_entry(data & 0x01);
}


READ8_MEMBER(gladiatr_state::gladiator_dsw1_r )
{
	int orig = ioport("DSW1")->read()^0xff;

	return BITSWAP8(orig, 0,1,2,3,4,5,6,7);
}

READ8_MEMBER(gladiatr_state::gladiator_dsw2_r )
{
	int orig = ioport("DSW2")->read()^0xff;

	return BITSWAP8(orig, 2,3,4,5,6,7,1,0);
}

READ8_MEMBER(gladiatr_state::gladiator_controls_r )
{
	int coins = 0;

	if(ioport("COINS")->read() & 0xc0 ) coins = 0x80;
	switch(offset)
	{
	case 0x01: /* start button , coins */
		return ioport("IN0")->read() | coins;
	case 0x02: /* Player 1 Controller , coins */
		return ioport("IN1")->read() | coins;
	case 0x04: /* Player 2 Controller , coins */
		return ioport("IN2")->read() | coins;
	}
	/* unknown */
	return 0;
}

READ8_MEMBER(gladiatr_state::gladiator_button3_r )
{
	switch(offset)
	{
	case 0x01: /* button 3 */
		return ioport("IN3")->read();
	}
	/* unknown */
	return 0;
}

MACHINE_RESET_MEMBER(gladiatr_state,gladiator)
{
	/* 6809 bank memory set */
	{
		membank("bank2")->set_entry(0);
		m_audiocpu->reset();
	}
}

/* YM2203 port B handler (output) */
WRITE8_MEMBER(gladiatr_state::gladiator_int_control_w)
{
	/* bit 7   : SSRST = sound reset ? */
	/* bit 6-1 : N.C.                  */
	/* bit 0   : ??                    */
}
/* YM2203 IRQ */
WRITE_LINE_MEMBER(gladiatr_state::gladiator_ym_irq)
{
	/* NMI IRQ is not used by gladiator sound program */
	m_subcpu->set_input_line(INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE);
}

/*Sound Functions*/
WRITE8_MEMBER(gladiatr_state::gladiator_adpcm_w)
{
	/* bit6 = bank offset */
	membank("bank2")->set_entry((data & 0x40) ? 1 : 0);

	m_msm->data_w(data);         /* bit0..3  */
	m_msm->reset_w(BIT(data, 5)); /* bit 5    */
	m_msm->vclk_w (BIT(data, 4)); /* bit4     */
}

WRITE8_MEMBER(gladiatr_state::gladiator_cpu_sound_command_w)
{
	soundlatch_byte_w(space,0,data);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

READ8_MEMBER(gladiatr_state::gladiator_cpu_sound_command_r)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	return soundlatch_byte_r(space,0);
}

WRITE8_MEMBER(gladiatr_state::gladiatr_flipscreen_w)
{
	flip_screen_set(data & 1);
}


#if 1
/* !!!!! patch to IRQ timing for 2nd CPU !!!!! */
WRITE8_MEMBER(gladiatr_state::gladiatr_irq_patch_w)
{
	m_subcpu->set_input_line(0, HOLD_LINE);
}
#endif







WRITE8_MEMBER(gladiatr_state::ppking_qx0_w)
{
	if(!offset)
	{
		m_data2=data;
		m_flag2=1;
	}
}

WRITE8_MEMBER(gladiatr_state::ppking_qx1_w)
{
	if(!offset)
	{
		m_data1=data;
		m_flag1=1;
	}
}

WRITE8_MEMBER(gladiatr_state::ppking_qx2_w){ }

WRITE8_MEMBER(gladiatr_state::ppking_qx3_w){ }

READ8_MEMBER(gladiatr_state::ppking_qx2_r){ return machine().rand(); }

READ8_MEMBER(gladiatr_state::ppking_qx3_r){ return machine().rand()&0xf; }

READ8_MEMBER(gladiatr_state::ppking_qx0_r)
{
	if(!offset)
			return m_data1;
	else
		return m_flag2;
}

READ8_MEMBER(gladiatr_state::ppking_qx1_r)
{
	if(!offset)
		return m_data2;
	else
		return m_flag1;
}

MACHINE_RESET_MEMBER(gladiatr_state,ppking)
{
	m_data1 = m_data2 = 0;
	m_flag1 = m_flag2 = 1;
}

static ADDRESS_MAP_START( ppking_cpu1_map, AS_PROGRAM, 8, gladiatr_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xcbff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xcc00, 0xcfff) AM_WRITE(ppking_video_registers_w)
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(paletteram_w) AM_SHARE("paletteram")
	AM_RANGE(0xd800, 0xdfff) AM_RAM_WRITE(videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xe800, 0xefff) AM_RAM_WRITE(textram_w) AM_SHARE("textram")
	AM_RANGE(0xf000, 0xf7ff) AM_RAM AM_SHARE("nvram") /* battery backed RAM */
ADDRESS_MAP_END


static ADDRESS_MAP_START( ppking_cpu3_map, AS_PROGRAM, 8, gladiatr_state )
	AM_RANGE(0x2000, 0x2fff) AM_ROM
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( ppking_cpu1_io, AS_IO, 8, gladiatr_state )
//  ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xc000, 0xc000) AM_WRITE(spritebuffer_w)
	AM_RANGE(0xc004, 0xc004) AM_NOP // WRITE(ppking_irq_patch_w)
	AM_RANGE(0xc09e, 0xc09f) AM_READ(ppking_qx0_r) AM_WRITE(ppking_qx0_w)
	AM_RANGE(0xc0bf, 0xc0bf) AM_NOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( ppking_cpu2_io, AS_IO, 8, gladiatr_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE("ymsnd", ym2203_device, read, write)
	AM_RANGE(0x20, 0x21) AM_READ(ppking_qx1_r) AM_WRITE(ppking_qx1_w)
	AM_RANGE(0x40, 0x40) AM_READNOP
	AM_RANGE(0x60, 0x61) AM_READWRITE(ppking_qx2_r,ppking_qx2_w)
	AM_RANGE(0x80, 0x81) AM_READWRITE(ppking_qx3_r,ppking_qx3_w)
ADDRESS_MAP_END




static ADDRESS_MAP_START( gladiatr_cpu1_map, AS_PROGRAM, 8, gladiatr_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xcbff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xcc00, 0xcfff) AM_WRITE(gladiatr_video_registers_w)
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(paletteram_w) AM_SHARE("paletteram")
	AM_RANGE(0xd800, 0xdfff) AM_RAM_WRITE(videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xe800, 0xefff) AM_RAM_WRITE(textram_w) AM_SHARE("textram")
	AM_RANGE(0xf000, 0xf7ff) AM_RAM AM_SHARE("nvram") /* battery backed RAM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpu2_map, AS_PROGRAM, 8, gladiatr_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( gladiatr_cpu3_map, AS_PROGRAM, 8, gladiatr_state )
	AM_RANGE(0x1000, 0x1fff) AM_WRITE(gladiator_adpcm_w)
	AM_RANGE(0x2000, 0x2fff) AM_READ(gladiator_cpu_sound_command_r)
	AM_RANGE(0x4000, 0xffff) AM_ROMBANK("bank2")
ADDRESS_MAP_END


static ADDRESS_MAP_START( gladiatr_cpu1_io, AS_IO, 8, gladiatr_state )
//  ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xc000, 0xc000) AM_WRITE(spritebuffer_w)
	AM_RANGE(0xc001, 0xc001) AM_WRITE(gladiatr_spritebank_w)
	AM_RANGE(0xc002, 0xc002) AM_WRITE(gladiatr_bankswitch_w)
	AM_RANGE(0xc004, 0xc004) AM_WRITE(gladiatr_irq_patch_w) /* !!! patch to 2nd CPU IRQ !!! */
	AM_RANGE(0xc007, 0xc007) AM_WRITE(gladiatr_flipscreen_w)
	AM_RANGE(0xc09e, 0xc09f) AM_DEVREADWRITE("taito8741", taito8741_4pack_device, read_0, write_0)
	AM_RANGE(0xc0bf, 0xc0bf) AM_NOP // watchdog_reset_w doesn't work
ADDRESS_MAP_END

static ADDRESS_MAP_START( gladiatr_cpu2_io, AS_IO, 8, gladiatr_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE("ymsnd", ym2203_device, read, write)
	AM_RANGE(0x20, 0x21) AM_DEVREADWRITE("taito8741", taito8741_4pack_device, read_1, write_1)
	AM_RANGE(0x40, 0x40) AM_NOP // WRITE(sub_irq_ack_w)
	AM_RANGE(0x60, 0x61) AM_DEVREADWRITE("taito8741", taito8741_4pack_device, read_2, write_2)
	AM_RANGE(0x80, 0x81) AM_DEVREADWRITE("taito8741", taito8741_4pack_device, read_3, write_3)
	AM_RANGE(0xa0, 0xa7) AM_NOP // filters on sound output
	AM_RANGE(0xe0, 0xe0) AM_WRITE(gladiator_cpu_sound_command_w)
ADDRESS_MAP_END



static INPUT_PORTS_START( gladiatr )
	PORT_START("DSW1")      /* (8741-0 parallel port)*/
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x00, "After 4 Stages" )        PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Continues ) )
	PORT_DIPSETTING(    0x04, "Ends" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:4")   /*NOTE: Actual manual has these settings reversed(typo?)! */
	PORT_DIPSETTING(    0x00, "Only at 100000" )
	PORT_DIPSETTING(    0x00, "Every 100000" )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")      /* (8741-1 parallel port) - Dips 6 Unused */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )        /* Listed as "Unused" */
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")      /* (YM2203 port B) - Dips 5,6,7 Unused */
	PORT_DIPNAME( 0x01, 0x01, "Invulnerability (Cheat)")    PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Memory Backup" )     PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Clear" )
	PORT_DIPNAME( 0x0c, 0x0c, "Starting Stage" )        PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW3:5" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW3:6" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW3:7" )        /* Listed as "Unused" */
	PORT_SERVICE_DIPLOC(   0x80, IP_ACTIVE_LOW, "SW3:8" )

	PORT_START("IN0")   /*(8741-3 parallel port 1) */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* COINS */

	PORT_START("COINS") /*(8741-3 parallel port bit7) */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)

	PORT_START("IN1")   /* (8741-3 parallel port 2) */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* COINS */

	PORT_START("IN2")   /* (8741-3 parallel port 4) */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* COINS */

	PORT_START("IN3")   /* (8741-2 parallel port 1) */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

/*******************************************************************/

static const gfx_layout charlayout  =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout tilelayout  =
{
	8,8,
	RGN_FRAC(1,2),
	3,
	{ 4, RGN_FRAC(1,2), RGN_FRAC(1,2)+4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8
};

static const gfx_layout spritelayout  =
{
	16,16,
	RGN_FRAC(1,2),
	3,
	{ 4, RGN_FRAC(1,2), RGN_FRAC(1,2)+4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8
};

static GFXDECODE_START( ppking )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout, 0, 32 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout, 0x100, 32 )
GFXDECODE_END

static GFXDECODE_START( gladiatr )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0x200, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,   0x000, 32 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout, 0x100, 32 )
GFXDECODE_END



READ8_MEMBER(gladiatr_state::ppking_f1_r)
{
	return machine().rand();
}

static MACHINE_CONFIG_START( ppking, gladiatr_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_12MHz/2) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(ppking_cpu1_map)
	MCFG_CPU_IO_MAP(ppking_cpu1_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gladiatr_state,  irq0_line_hold)

	MCFG_CPU_ADD("sub", Z80, XTAL_12MHz/4) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(cpu2_map)
	MCFG_CPU_IO_MAP(ppking_cpu2_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gladiatr_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", M6809, XTAL_12MHz/16) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(ppking_cpu3_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	MCFG_MACHINE_RESET_OVERRIDE(gladiatr_state,ppking)
	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(gladiatr_state, screen_update_ppking)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ppking)
	MCFG_PALETTE_ADD("palette", 1024)

	MCFG_VIDEO_START_OVERRIDE(gladiatr_state,ppking)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, XTAL_12MHz/8) /* verified on pcb */
	MCFG_AY8910_PORT_A_READ_CB(READ8(gladiatr_state, ppking_f1_r))
	MCFG_AY8910_PORT_B_READ_CB(READ8(gladiatr_state, ppking_f1_r))
	MCFG_SOUND_ROUTE(0, "mono", 0.60)
	MCFG_SOUND_ROUTE(1, "mono", 0.60)
	MCFG_SOUND_ROUTE(2, "mono", 0.60)
	MCFG_SOUND_ROUTE(3, "mono", 0.50)

	MCFG_SOUND_ADD("msm", MSM5205, XTAL_455kHz) /* verified on pcb */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_SEX_4B)  /* vclk input mode    */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( gladiatr, gladiatr_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_12MHz/2) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(gladiatr_cpu1_map)
	MCFG_CPU_IO_MAP(gladiatr_cpu1_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gladiatr_state,  irq0_line_hold)

	MCFG_CPU_ADD("sub", Z80, XTAL_12MHz/4) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(cpu2_map)
	MCFG_CPU_IO_MAP(gladiatr_cpu2_io)

	MCFG_CPU_ADD("audiocpu", M6809, XTAL_12MHz/16) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(gladiatr_cpu3_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_MACHINE_RESET_OVERRIDE(gladiatr_state,gladiator)
	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_TAITO8741_ADD("taito8741")
	MCFG_TAITO8741_MODES(TAITO8741_MASTER,TAITO8741_SLAVE,TAITO8741_PORT,TAITO8741_PORT)
	MCFG_TAITO8741_CONNECT(1,0,0,0)
	MCFG_TAITO8741_PORT_HANDLERS(READ8(gladiatr_state,gladiator_dsw1_r),READ8(gladiatr_state,gladiator_dsw2_r),READ8(gladiatr_state,gladiator_button3_r),READ8(gladiatr_state,gladiator_controls_r))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(gladiatr_state, screen_update_gladiatr)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", gladiatr)
	MCFG_PALETTE_ADD("palette", 1024)

	MCFG_VIDEO_START_OVERRIDE(gladiatr_state,gladiatr)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, XTAL_12MHz/8) /* verified on pcb */
	MCFG_YM2203_IRQ_HANDLER(WRITELINE(gladiatr_state, gladiator_ym_irq))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW3")) /* port B read */
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(gladiatr_state, gladiator_int_control_w)) /* port A write */
	MCFG_SOUND_ROUTE(0, "mono", 0.60)
	MCFG_SOUND_ROUTE(1, "mono", 0.60)
	MCFG_SOUND_ROUTE(2, "mono", 0.60)
	MCFG_SOUND_ROUTE(3, "mono", 0.50)

	MCFG_SOUND_ADD("msm", MSM5205, XTAL_455kHz) /* verified on pcb */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_SEX_4B)  /* vclk input mode    */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( ppking )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "q1_1.a1",        0x00000, 0x2000, CRC(b74b2718) SHA1(29833439211076873324ccfa5897eb1e6aa9d134) )
	ROM_LOAD( "q1_2.b1",        0x02000, 0x2000, CRC(1b1e4cd4) SHA1(34c6cf5e0775c0c834dda34a3a2a4685465daa8e) )
	ROM_LOAD( "q0_3.c1",        0x04000, 0x2000, CRC(6a7acf8e) SHA1(06d37e813605f507ea1c720764fc554e58defdf8) )
	ROM_LOAD( "q0_4.d1",        0x06000, 0x2000, CRC(b83eb6d5) SHA1(f112d3c0d701977dcc5c312ad74d78b44882201b) )
	ROM_LOAD( "q0_5.e1",        0x08000, 0x4000, CRC(4d2007e2) SHA1(973ef0e6ff6065b753402489a3d10a9b68164969) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "q0_17.6f",       0x0000, 0x2000, CRC(f7fe0d24) SHA1(6dcb23aa7fc08fc892a8b3843ccb982997c20571) )
	ROM_LOAD( "q0_16.6e",       0x4000, 0x2000, CRC(b1e32588) SHA1(13c74479238a34a08e249f9120b42a52d80f8274) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "q0_19.5n",       0x0c000, 0x2000, CRC(4bcf896d) SHA1(f587a66fcc63e989742ce2d5f4cf2bb464987038) )
	ROM_LOAD( "q0_18.5m",       0x0e000, 0x2000, CRC(89ba64f8) SHA1(fa01316ea744b4277ee64d5f14cb6d7e3a949f2b) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "q0_15.1r",       0x00000, 0x2000, CRC(fbd33219) SHA1(78b9bb327ededaa818d26c41c5e8fd1c041ef142) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "q0_12.1j",       0x00000, 0x2000, CRC(b1a44482) SHA1(84cc40976aa9b015a9f970a878bbde753651b3ba) ) /* plane 3 */
	/* space to unpack plane 3 */
	ROM_LOAD( "q0_13.1k",       0x04000, 0x2000, CRC(468f35e6) SHA1(8e28481910663fe525cefd4ad406468b7736900e) ) /* planes 1,2 */
	ROM_LOAD( "q0_14.1m",       0x06000, 0x2000, CRC(eed04a7f) SHA1(d139920889653c33ded38a85510789380dd0aa9e) ) /* planes 1,2 */

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "q0_6.k1",        0x00000, 0x2000, CRC(bb3d666c) SHA1(a689c7a1e75b916d69f396db7c4688ac355c2aff) ) /* plane 3 */
	ROM_LOAD( "q0_7.l1",        0x02000, 0x2000, CRC(16a2550e) SHA1(adb54b70a6db660b5f29ad66da02afd8e99884bb) ) /* plane 3 */
	/* space to unpack plane 3 */
	ROM_LOAD( "q0_8.m1",        0x08000, 0x2000, CRC(41235b22) SHA1(4d9702efe0ea320dab7c0d889f4d03f196b32661) ) /* planes 1,2 */
	ROM_LOAD( "q0_9.p1",        0x0a000, 0x2000, CRC(74cc94b2) SHA1(2cb981ecb2487dfa5c0974e036106fc06c2c1880) ) /* planes 1,2 */
	ROM_LOAD( "q0_10.r1",       0x0c000, 0x2000, CRC(af438cc6) SHA1(cf79c8d3f2a0c489a756b341f8df747f6654764b) ) /* planes 1,2 */
	/* empty! */

	ROM_REGION( 0x040, "proms", 0 )
	ROM_LOAD( "q1",             0x0000, 0x0020, CRC(cca9ae7b) SHA1(e18416fbe2a5b09db749cc9a14fa89186ed8f4ba) )
	ROM_LOAD( "q2",             0x0020, 0x0020, CRC(da952b5e) SHA1(0863ad8fdcf69435a7455cd345d3d0af0b0c0a0a) )
ROM_END


ROM_START( gladiatr )
	ROM_REGION( 0x1c000, "maincpu", 0 )
	ROM_LOAD( "qb0_5",          0x00000, 0x4000, CRC(25b19efb) SHA1(c41344278f6c7f3d6527aced3e459ed1ba86dea5) )
	ROM_LOAD( "qb0_4",          0x04000, 0x2000, CRC(347ec794) SHA1(51100f9fef2e96f00e94fce709eed6583b01a2eb) )
	ROM_LOAD( "qb0_1",          0x10000, 0x2000, CRC(040c9839) SHA1(8c0d9a246847461a59eb5e6a53a94218e701d6c3) )
	ROM_CONTINUE(               0x16000, 0x2000 )
	ROM_LOAD( "qc0_3",          0x12000, 0x4000, CRC(8d182326) SHA1(f0af3757c2cf9e1e8035272567adee6efc733319) )
	ROM_CONTINUE(               0x18000, 0x4000 )

	ROM_REGION( 0x10000, "sub", 0 ) /* Code for the 2nd CPU */
	ROM_LOAD( "qb0_17",         0x0000, 0x4000, CRC(e78be010) SHA1(157231d858d13a006b57a4ab419368168e64edb7) )

	ROM_REGION( 0x28000, "audiocpu", 0 )  /* 6809 Code & ADPCM data */
	ROM_LOAD( "qb0_20",         0x10000, 0x4000, CRC(15916eda) SHA1(6558bd2ae6f14d630ae93e66ce7d09be33870cce) )
	ROM_CONTINUE(               0x1c000, 0x4000 )
	ROM_LOAD( "qb0_19",         0x14000, 0x4000, CRC(79caa7ed) SHA1(57adc8429ad016c4da41deda6b7b6fe36de5a225) )
	ROM_CONTINUE(               0x20000, 0x4000 )
	ROM_LOAD( "qb0_18",         0x18000, 0x4000, CRC(e9591260) SHA1(e427aa10c683fbeb98171f6d1820781d21075a24) )
	ROM_CONTINUE(               0x24000, 0x4000 )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "qc0_15",         0x00000, 0x2000, CRC(a7efa340) SHA1(f87e061b8e4d8cd0834fab301779a8493549419b) ) /* (monochrome) */

	ROM_REGION( 0x20000, "gfx2", 0 )    /* tiles */
	ROM_LOAD( "qb0_12",         0x00000, 0x8000, CRC(0585d9ac) SHA1(e3cb07e9dc5ec2fcfa0c90294d32f0b751f67752) ) /* plane 3 */
	/* space to unpack plane 3 */
	ROM_LOAD( "qb0_13",         0x10000, 0x8000, CRC(a6bb797b) SHA1(852e9993270e5557c1a0350007d0beaec5ca6286) ) /* planes 1,2 */
	ROM_LOAD( "qb0_14",         0x18000, 0x8000, CRC(85b71211) SHA1(81545cd168da4a707e263fdf0ee9902e3a13ba93) ) /* planes 1,2 */

	ROM_REGION( 0x30000, "gfx3", 0 )    /* sprites */
	ROM_LOAD( "qc1_6",          0x00000, 0x4000, CRC(651e6e44) SHA1(78ce576e6c29e43d590c42f0d4926cff82fd0268) ) /* plane 3 */
	ROM_LOAD( "qc2_7",          0x04000, 0x8000, CRC(c992c4f7) SHA1(3263973474af07c8b93c4ec97924568848cb7201) ) /* plane 3 */
	/* space to unpack plane 3 */
	ROM_LOAD( "qc0_8",          0x18000, 0x4000, CRC(1c7ffdad) SHA1(b224fd4cce078186f22e6393a38c7a2d84dc0066) ) /* planes 1,2 */
	ROM_LOAD( "qc1_9",          0x1c000, 0x4000, CRC(01043e03) SHA1(6a6dddc0a036873135dceaa989e757bdd2455ae7) ) /* planes 1,2 */
	ROM_LOAD( "qc1_10",         0x20000, 0x8000, CRC(364cdb58) SHA1(4d8548f9dfa9d105dd277c61cf3d56583a5ebbcb) ) /* planes 1,2 */
	ROM_LOAD( "qc2_11",         0x28000, 0x8000, CRC(c9fecfff) SHA1(7c13ace4293fbfab7fe924b7b24c498d8cefc7ac) ) /* planes 1,2 */

	ROM_REGION( 0x00040, "proms", 0 )   /* unused */
	ROM_LOAD( "q3.2b",          0x00000, 0x0020, CRC(6a7c3c60) SHA1(5125bfeb03752c8d76b140a4e74d5cac29dcdaa6) ) /* address decoding */
	ROM_LOAD( "q4.5s",          0x00020, 0x0020, CRC(e325808e) SHA1(5fd92ad4eff24f6ccf2df19d268a6cafba72202e) )
ROM_END

ROM_START( ogonsiro )
	ROM_REGION( 0x1c000, "maincpu", 0 )
	ROM_LOAD( "qb0_5",          0x00000, 0x4000, CRC(25b19efb) SHA1(c41344278f6c7f3d6527aced3e459ed1ba86dea5) )
	ROM_LOAD( "qb0_4",          0x04000, 0x2000, CRC(347ec794) SHA1(51100f9fef2e96f00e94fce709eed6583b01a2eb) )
	ROM_LOAD( "qb0_1",          0x10000, 0x2000, CRC(040c9839) SHA1(8c0d9a246847461a59eb5e6a53a94218e701d6c3) )
	ROM_CONTINUE(               0x16000, 0x2000 )
	ROM_LOAD( "qb0_3",          0x12000, 0x4000, CRC(d6a342e7) SHA1(96274ae3bda4679108a25fcc514b625552abda30) )
	ROM_CONTINUE(               0x18000, 0x4000 )

	ROM_REGION( 0x10000, "sub", 0 ) /* Code for the 2nd CPU */
	ROM_LOAD( "qb0_17",         0x0000, 0x4000, CRC(e78be010) SHA1(157231d858d13a006b57a4ab419368168e64edb7) )

	ROM_REGION( 0x28000, "audiocpu", 0 )  /* 6809 Code & ADPCM data */
	ROM_LOAD( "qb0_20",         0x10000, 0x4000, CRC(15916eda) SHA1(6558bd2ae6f14d630ae93e66ce7d09be33870cce) )
	ROM_CONTINUE(               0x1c000, 0x4000 )
	ROM_LOAD( "qb0_19",         0x14000, 0x4000, CRC(79caa7ed) SHA1(57adc8429ad016c4da41deda6b7b6fe36de5a225) )
	ROM_CONTINUE(               0x20000, 0x4000 )
	ROM_LOAD( "qb0_18",         0x18000, 0x4000, CRC(e9591260) SHA1(e427aa10c683fbeb98171f6d1820781d21075a24) )
	ROM_CONTINUE(               0x24000, 0x4000 )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "qb0_15",         0x00000, 0x2000, CRC(5e1332b8) SHA1(fab6e2c7ea9bc94c1245bf759b4004a70c57d666) ) /* (monochrome) */

	ROM_REGION( 0x20000, "gfx2", 0 )    /* tiles */
	ROM_LOAD( "qb0_12",         0x00000, 0x8000, CRC(0585d9ac) SHA1(e3cb07e9dc5ec2fcfa0c90294d32f0b751f67752) ) /* plane 3 */
	/* space to unpack plane 3 */
	ROM_LOAD( "qb0_13",         0x10000, 0x8000, CRC(a6bb797b) SHA1(852e9993270e5557c1a0350007d0beaec5ca6286) ) /* planes 1,2 */
	ROM_LOAD( "qb0_14",         0x18000, 0x8000, CRC(85b71211) SHA1(81545cd168da4a707e263fdf0ee9902e3a13ba93) ) /* planes 1,2 */

	ROM_REGION( 0x30000, "gfx3", 0 )    /* sprites */
	ROM_LOAD( "qb0_6",          0x00000, 0x4000, CRC(1a2bc769) SHA1(498861f4d0cffeaff90609c8000c921a114756b6) ) /* plane 3 */
	ROM_LOAD( "qb0_7",          0x04000, 0x8000, CRC(4b677bd9) SHA1(3314ef58ff5307faf0ecd8f99950d43d571c91a6) ) /* plane 3 */
	/* space to unpack plane 3 */
	ROM_LOAD( "qc0_8",          0x18000, 0x4000, CRC(1c7ffdad) SHA1(b224fd4cce078186f22e6393a38c7a2d84dc0066) ) /* planes 1,2 */
	ROM_LOAD( "qb0_9",          0x1c000, 0x4000, CRC(38f5152d) SHA1(fbb7b13a625999807d180a3212e6e12870629438) ) /* planes 1,2 */
	ROM_LOAD( "qb0_10",         0x20000, 0x8000, CRC(87ab6cc4) SHA1(50bc1108ff5609c0e7dad615e92e16eb72b7bc03) ) /* planes 1,2 */
	ROM_LOAD( "qb0_11",         0x28000, 0x8000, CRC(25eaa4ff) SHA1(3547fc600a617ba7fe5240a7830edb90230b6c51) ) /* planes 1,2 */

	ROM_REGION( 0x00040, "proms", 0 ) /* unused */
	ROM_LOAD( "q3.2b",          0x00000, 0x0020, CRC(6a7c3c60) SHA1(5125bfeb03752c8d76b140a4e74d5cac29dcdaa6) ) /* address decoding */
	ROM_LOAD( "q4.5s",          0x00020, 0x0020, CRC(e325808e) SHA1(5fd92ad4eff24f6ccf2df19d268a6cafba72202e) )
ROM_END

ROM_START( greatgur )
	ROM_REGION( 0x1c000, "maincpu", 0 )
	ROM_LOAD( "qb0_5",          0x00000, 0x4000, CRC(25b19efb) SHA1(c41344278f6c7f3d6527aced3e459ed1ba86dea5) )
	ROM_LOAD( "qb0_4",          0x04000, 0x2000, CRC(347ec794) SHA1(51100f9fef2e96f00e94fce709eed6583b01a2eb) )
	ROM_LOAD( "qb0_1",          0x10000, 0x2000, CRC(040c9839) SHA1(8c0d9a246847461a59eb5e6a53a94218e701d6c3) )
	ROM_CONTINUE(               0x16000, 0x2000 )
	ROM_LOAD( "qb0_3",          0x12000, 0x4000, CRC(d6a342e7) SHA1(96274ae3bda4679108a25fcc514b625552abda30) )
	ROM_CONTINUE(               0x18000, 0x4000 )

	ROM_REGION( 0x10000, "sub", 0 ) /* Code for the 2nd CPU */
	ROM_LOAD( "qb0_17",         0x0000, 0x4000, CRC(e78be010) SHA1(157231d858d13a006b57a4ab419368168e64edb7) )

	ROM_REGION( 0x28000, "audiocpu", 0 )  /* 6809 Code & ADPCM data */
	ROM_LOAD( "qb0_20",         0x10000, 0x4000, CRC(15916eda) SHA1(6558bd2ae6f14d630ae93e66ce7d09be33870cce) )
	ROM_CONTINUE(               0x1c000, 0x4000 )
	ROM_LOAD( "qb0_19",         0x14000, 0x4000, CRC(79caa7ed) SHA1(57adc8429ad016c4da41deda6b7b6fe36de5a225) )
	ROM_CONTINUE(               0x20000, 0x4000 )
	ROM_LOAD( "qb0_18",         0x18000, 0x4000, CRC(e9591260) SHA1(e427aa10c683fbeb98171f6d1820781d21075a24) )
	ROM_CONTINUE(               0x24000, 0x4000 )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "qb0_15",         0x00000, 0x2000, CRC(5e1332b8) SHA1(fab6e2c7ea9bc94c1245bf759b4004a70c57d666) ) /* (monochrome) */

	ROM_REGION( 0x20000, "gfx2", 0 )    /* tiles */
	ROM_LOAD( "qb0_12",         0x00000, 0x8000, CRC(0585d9ac) SHA1(e3cb07e9dc5ec2fcfa0c90294d32f0b751f67752) ) /* plane 3 */
	/* space to unpack plane 3 */
	ROM_LOAD( "qb0_13",         0x10000, 0x8000, CRC(a6bb797b) SHA1(852e9993270e5557c1a0350007d0beaec5ca6286) ) /* planes 1,2 */
	ROM_LOAD( "qb0_14",         0x18000, 0x8000, CRC(85b71211) SHA1(81545cd168da4a707e263fdf0ee9902e3a13ba93) ) /* planes 1,2 */

	ROM_REGION( 0x30000, "gfx3", 0 )    /* sprites */
	ROM_LOAD( "qc0_06.bin",     0x00000, 0x4000, CRC(96b20201) SHA1(212270d3ba72974f22e96744c752860cc5ffba5b) ) /* plane 3 */
	ROM_LOAD( "qc0_07.bin",     0x04000, 0x8000, CRC(9e89fa8f) SHA1(b133ae2ac62f43a7a51fa0d1a023a4f95fef2996) ) /* plane 3 */
	/* space to unpack plane 3 */
	ROM_LOAD( "qc0_8",          0x18000, 0x4000, CRC(1c7ffdad) SHA1(b224fd4cce078186f22e6393a38c7a2d84dc0066) ) /* planes 1,2 */
	ROM_LOAD( "qc0_09.bin",     0x1c000, 0x4000, CRC(204cd385) SHA1(e7a8720feeac8ced581d72190345daed5750379f) ) /* planes 1,2 */
	ROM_LOAD( "qc1_10",         0x20000, 0x8000, CRC(364cdb58) SHA1(4d8548f9dfa9d105dd277c61cf3d56583a5ebbcb) ) /* planes 1,2 */
	ROM_LOAD( "qc1_11.bin",     0x28000, 0x8000, CRC(b2aabbf5) SHA1(9eb4d80f38a30f6e45231a9bfd1aff7a124c6ee9) ) /* planes 1,2 */

	ROM_REGION( 0x00040, "proms", 0 ) /* unused */
	ROM_LOAD( "q3.2b",          0x00000, 0x0020, CRC(6a7c3c60) SHA1(5125bfeb03752c8d76b140a4e74d5cac29dcdaa6) ) /* address decoding */
	ROM_LOAD( "q4.5s",          0x00020, 0x0020, CRC(e325808e) SHA1(5fd92ad4eff24f6ccf2df19d268a6cafba72202e) )

	ROM_REGION( 0x0400, "user1", 0 ) /* ROMs for the four 8741 (not emulated yet) */
	ROM_LOAD( "gladcctl.1",     0x00000, 0x0400, CRC(b30d225f) SHA1(f383286530975c440589c276aa8c46fdfe5292b6) )
	ROM_LOAD( "gladccpu.2",     0x00000, 0x0400, CRC(1d02cd5f) SHA1(f7242039788c66a1d91b01852d7d447330b847c4) )
	ROM_LOAD( "gladucpu.17",    0x00000, 0x0400, CRC(3c5ca4c6) SHA1(0d8c2e1c2142ada11e30cfb9a48663386fee9cb8) )
	ROM_LOAD( "gladcsnd.18",    0x00000, 0x0400, CRC(3c5ca4c6) SHA1(0d8c2e1c2142ada11e30cfb9a48663386fee9cb8) )
ROM_END

ROM_START( gcastle )
	ROM_REGION( 0x1c000, "maincpu", 0 )
	ROM_LOAD( "gc0_5",          0x00000, 0x4000, BAD_DUMP CRC(25b19efb) SHA1(c41344278f6c7f3d6527aced3e459ed1ba86dea5) ) // not dumped, placeholder taken from ogonsiro
	ROM_LOAD( "gc0_4",          0x04000, 0x2000, BAD_DUMP CRC(347ec794) SHA1(51100f9fef2e96f00e94fce709eed6583b01a2eb) ) // "
	ROM_LOAD( "gc0_1",          0x10000, 0x2000, BAD_DUMP CRC(040c9839) SHA1(8c0d9a246847461a59eb5e6a53a94218e701d6c3) ) // "
	ROM_CONTINUE(               0x16000, 0x2000 )
	ROM_LOAD( "gc0_3",          0x12000, 0x4000, BAD_DUMP CRC(d6a342e7) SHA1(96274ae3bda4679108a25fcc514b625552abda30) ) // "
	ROM_CONTINUE(               0x18000, 0x4000 )

	ROM_REGION( 0x10000, "sub", 0 ) /* Code for the 2nd CPU */
	ROM_LOAD( "qb0_17",         0x0000, 0x4000, CRC(e78be010) SHA1(157231d858d13a006b57a4ab419368168e64edb7) )

	ROM_REGION( 0x28000, "audiocpu", 0 )  /* 6809 Code & ADPCM data */
	ROM_LOAD( "qb0_20",         0x10000, 0x4000, CRC(15916eda) SHA1(6558bd2ae6f14d630ae93e66ce7d09be33870cce) )
	ROM_CONTINUE(               0x1c000, 0x4000 )
	ROM_LOAD( "qb0_19",         0x14000, 0x4000, CRC(79caa7ed) SHA1(57adc8429ad016c4da41deda6b7b6fe36de5a225) )
	ROM_CONTINUE(               0x20000, 0x4000 )
	ROM_LOAD( "qb0_18",         0x18000, 0x4000, CRC(e9591260) SHA1(e427aa10c683fbeb98171f6d1820781d21075a24) )
	ROM_CONTINUE(               0x24000, 0x4000 )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "qb0_15",         0x00000, 0x2000, CRC(5e1332b8) SHA1(fab6e2c7ea9bc94c1245bf759b4004a70c57d666) ) /* (monochrome) */

	ROM_REGION( 0x20000, "gfx2", 0 )    /* tiles */
	ROM_LOAD( "qb0_12",         0x00000, 0x8000, CRC(0585d9ac) SHA1(e3cb07e9dc5ec2fcfa0c90294d32f0b751f67752) ) /* plane 3 */
	/* space to unpack plane 3 */
	ROM_LOAD( "qb0_13",         0x10000, 0x8000, CRC(a6bb797b) SHA1(852e9993270e5557c1a0350007d0beaec5ca6286) ) /* planes 1,2 */
	ROM_LOAD( "qb0_14",         0x18000, 0x8000, CRC(85b71211) SHA1(81545cd168da4a707e263fdf0ee9902e3a13ba93) ) /* planes 1,2 */

	ROM_REGION( 0x30000, "gfx3", 0 )    /* sprites */
	ROM_LOAD( "gc1_6",          0x00000, 0x4000, CRC(94f49be2) SHA1(adc9f38469d32eee5906b37289245df062b134b4) ) /* plane 3 */
	ROM_LOAD( "gc2_7",          0x04000, 0x8000, CRC(bb2cb454) SHA1(3cac1716a5c90953117deadcc3eba02000cda7c0) ) /* plane 3 */
	/* space to unpack plane 3 */
	ROM_LOAD( "qc0_8",          0x18000, 0x4000, CRC(1c7ffdad) SHA1(b224fd4cce078186f22e6393a38c7a2d84dc0066) ) /* planes 1,2 */
	ROM_LOAD( "gc1_9",          0x1c000, 0x4000, CRC(69b977fd) SHA1(9d05200e2b57577f7f353853cbbaa59bfd7a2804) ) /* planes 1,2 */
	ROM_LOAD( "qb0_10",         0x20000, 0x8000, CRC(87ab6cc4) SHA1(50bc1108ff5609c0e7dad615e92e16eb72b7bc03) ) /* planes 1,2 */
	ROM_LOAD( "gc2_11",         0x28000, 0x8000, CRC(5c512365) SHA1(f6b283ed7ec6f530b9c0f2e1e29c1a766af20a1e) ) /* planes 1,2 */

	ROM_REGION( 0x00040, "proms", 0 ) /* unused */
	ROM_LOAD( "q3.2b",          0x00000, 0x0020, CRC(6a7c3c60) SHA1(5125bfeb03752c8d76b140a4e74d5cac29dcdaa6) ) /* address decoding */
	ROM_LOAD( "q4.5s",          0x00020, 0x0020, CRC(e325808e) SHA1(5fd92ad4eff24f6ccf2df19d268a6cafba72202e) )
ROM_END


void gladiatr_state::swap_block(UINT8 *src1,UINT8 *src2,int len)
{
	int i,t;

	for (i = 0;i < len;i++)
	{
		t = src1[i];
		src1[i] = src2[i];
		src2[i] = t;
	}
}

DRIVER_INIT_MEMBER(gladiatr_state,gladiatr)
{
	UINT8 *rom;
	int i,j;

	rom = memregion("gfx2")->base();
	// unpack 3bpp graphics
	for (j = 3; j >= 0; j--)
	{
		for (i = 0; i < 0x2000; i++)
		{
			rom[i+(2*j+1)*0x2000] = rom[i+j*0x2000] >> 4;
			rom[i+2*j*0x2000] = rom[i+j*0x2000];
		}
	}
	// sort data
	swap_block(rom + 0x14000, rom + 0x18000, 0x4000);


	rom = memregion("gfx3")->base();
	// unpack 3bpp graphics
	for (j = 5; j >= 0; j--)
	{
		for (i = 0; i < 0x2000; i++)
		{
			rom[i+(2*j+1)*0x2000] = rom[i+j*0x2000] >> 4;
			rom[i+2*j*0x2000] = rom[i+j*0x2000];
		}
	}
	// sort data
	swap_block(rom + 0x1a000, rom + 0x1c000, 0x2000);
	swap_block(rom + 0x22000, rom + 0x28000, 0x2000);
	swap_block(rom + 0x26000, rom + 0x2c000, 0x2000);
	swap_block(rom + 0x24000, rom + 0x28000, 0x4000);

	membank("bank1")->configure_entries(0, 2, memregion("maincpu")->base() + 0x10000, 0x6000);
	membank("bank2")->configure_entries(0, 2, memregion("audiocpu")->base() + 0x10000, 0xc000);
	
	/* make sure bank is valid in cpu-reset */
	membank("bank2")->set_entry(0);
}


READ8_MEMBER(gladiatr_state::ppking_f6a3_r)
{
	if(space.device().safe_pcbase()==0x8e)
		m_nvram[0x6a3]=1;

	return m_nvram[0x6a3];
}

DRIVER_INIT_MEMBER(gladiatr_state,ppking)
{
	UINT8 *rom;
	int i,j;

	rom = memregion("gfx2")->base();
	// unpack 3bpp graphics
	for (i = 0; i < 0x2000; i++)
	{
		rom[i+0x2000] = rom[i] >> 4;
	}

	rom = memregion("gfx3")->base();
	// unpack 3bpp graphics
	for (j = 1; j >= 0; j--)
	{
		for (i = 0; i < 0x2000; i++)
		{
			rom[i+(2*j+1)*0x2000] = rom[i+j*0x2000] >> 4;
			rom[i+2*j*0x2000] = rom[i+j*0x2000];
		}
	}
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xf6a3,0xf6a3,read8_delegate(FUNC(gladiatr_state::ppking_f6a3_r),this));

	save_item(NAME(m_data1));
	save_item(NAME(m_data2));
}



GAME( 1985, ppking,   0,        ppking,   0,        gladiatr_state, ppking,   ROT90, "Taito America Corporation", "Ping-Pong King", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
GAME( 1986, gladiatr, 0,        gladiatr, gladiatr, gladiatr_state, gladiatr, ROT0,  "Allumer / Taito America Corporation", "Gladiator (US)", GAME_SUPPORTS_SAVE )
GAME( 1986, ogonsiro, gladiatr, gladiatr, gladiatr, gladiatr_state, gladiatr, ROT0,  "Allumer / Taito Corporation", "Ougon no Shiro (Japan)", GAME_SUPPORTS_SAVE )
GAME( 1986, greatgur, gladiatr, gladiatr, gladiatr, gladiatr_state, gladiatr, ROT0,  "Allumer / Taito Corporation", "Great Gurianos (Japan?)", GAME_SUPPORTS_SAVE )
GAME( 1986, gcastle,  gladiatr, gladiatr, gladiatr, gladiatr_state, gladiatr, ROT0,  "Allumer / Taito Corporation", "Golden Castle (prototype?)", GAME_SUPPORTS_SAVE ) // incomplete dump

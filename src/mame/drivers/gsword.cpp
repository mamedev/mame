// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff,Jarek Parchanski,Vas Crabb
/*
    Great Swordsman (Taito) 1984
    Joshi Volleyball (Taito) 1983

TODO:

-joshi volleyball
   -The incomplete graphic
   -The implementation of DAC sound ?
   -MCU hookup is incomplete
   -The true interrupt circuit of SUB CPU
   -unknown ROM (BANK ROM of sub-cpu ?)

Credits:
- Steve Ellenoff: Original emulation and Mame driver
- Jarek Parchanski: Dip Switch Fixes, Color improvements, ADPCM Interface code
- Tatsuyuki Satoh: sound improvements, NEC 8741 emulation, adpcm improvements,
            josvollyvall 8741 emulation
- Charlie Miltenberger: sprite colors improvements & precious hardware
            information and screenshots

Trick:
If you want fight with ODILION swordsman patch program for 1st CPU
at these addresses, otherwise you won't never fight with him.

        ROM[0x2256] = 0
        ROM[0x2257] = 0
        ROM[0x2258] = 0
        ROM[0x2259] = 0
        ROM[0x225A] = 0


There are 3 Z80s and two AY-3-8910s..

Prelim memory map (last updated 6/15/98)
*****************************************
GS1     z80 Main Code   (8K)    0000-1FFF
Gs2     z80 Game Data   (8K)    2000-3FFF
Gs3     z80 Game Data   (8K)    4000-5FFF
Gs4     z80 Game Data   (8K)    6000-7FFF
Gs5     z80 Game Data   (4K)    8000-8FFF
Gs6     Sprites         (8K)
Gs7     Sprites         (8K)
Gs8     Sprites         (8K)
Gs10    Tiles           (8K)
Gs11    Tiles           (8K)
Gs12    3rd z80 CPU &   (8K)
        ADPCM Samples?
Gs13    ADPCM Samples?  (8K)
Gs14    ADPCM Samples?  (8K)
Gs15    2nd z80 CPU     (8K)    0000-1FFF
Gs16    2nd z80 Data    (8K)    2000-3FFF
*****************************************

**********
*Main Z80*
**********

    9000 - 9fff Work Ram
        982e - 982e Free play
        98e0 - 98e0 Coin Input
        98e1 - 98e1 Player 1 Controls
        98e2 - 98e2 Player 2 Controls
        9c00 - 9c30 (Hi score - Scores)
        9c78 - 9cd8 (Hi score - Names)
        9e00 - 9e7f Sprites in working ram!
        9e80 - 9eff Sprite X & Y in working ram!

    a000 - afff Sprite RAM & Video Attributes
        a000 - a37F ???
        a380 - a77F Sprite Tile #s
        a780 - a7FF Sprite Y & X positions
        a980 - a980 Background Tile Bank Select
        ab00 - ab00 Background Tile Y-Scroll register
        ab80 - abff Sprite Attributes(X & Y Flip)

    b000 - b7ff Screen RAM
    b800 - ffff not used?!

PORTS:
7e 8741-#0 data port
7f 8741-#0 command / status port

*************
*2nd Z80 CPU*
*************
0000 - 3FFF ROM CODE
4000 - 43FF WORK RAM

write
6000 adpcm sound command for 3rd CPU

PORTS:
00 8741-#2 data port
01 8741-#2 command / status port
40 8741-#1 data port
41 8741-#1 command / status port

read:
60 fake port #0 ?
61 ay8910-#0 read port
data / ay8910-#0 read
80 fake port #1 ?
81 ay8910-#1 read port

write:
60 ay8910-#0 control port
61 ay8910-#0 data port
80 ay8910-#1 control port
81 ay8910-#1 data port
   ay8910-A  : NMI control ?
a0 unknown
e0 unknown (watch dog?)

*************
*3rd Z80 CPU*
*************
0000-5fff ROM

read:
a000 adpcm sound command

write:
6000 MSM5205 reset and data

*************
I8741 communication data

reg: 0->1 (main->2nd) /     : (1->0) 2nd->main :
 0 : DSW.2 (port)           : DSW.1(port)
 1 : DSW.1                  : DSW.2
 2 : IN0 / sound error code :
 3 : IN1 / ?                :
 4 : IN2                    :
 4 : IN3                    :
 5 :                        :
 6 :                        : DSW0?
 7 :                        : ?

gsword notes:

There are two 8041 MCUs and one 8741 MCU:
* One connected to the main CPU for communicating with the sub CPU and
  reading DIP switch C.
* One connected to the sub CPU for communicating with the main CPU and
  reading DIP switches A and B.
* One connected to the sub CPU for reading player, start and coin
  inputs, and driving the coin counter outputs.
* TODO: confirm which MCU is connected to which CPU.

So far, only the AA-016 MCU has been dumped successfully.  There's no
reason to believe that the AA-017 MCU runs a different program.  Other
Allumer-developed games are known to have different silkscreen on
identical parts.  It's not clear which of the MCUs (AA-013 at 5A, AA-016
at 9C and AA-017 at 9G) plays which role.

The AA-016 MCU program is divided into several parts, each entirely
contained in a single program page.  The initialisation/self test, mode
selection, and the main loop of the communication mode are in page 0;
the subroutines implementing communication mode are in page 1; the
input/coin handling main loop and host communiction code is in page 2;
finally, the subroutines that implement coin handling are in page 3.
This suggests componentes were assigned to different developers who were
each given exclusive use of a page to simplify integration.  There was
insufficient space in page 3 for the program checksum fragment, so the
coin handling subroutines are not checksummed, despite plenty of zero-
filled space elsewhere in the ROM.

At least two Great Swordsman boards have been seen with a UVEPROM-based
D8741A-8 MCU for AA-103 at 5A.  This suggests the developers made some
last-minute change to the code that only affected one of the three use
cases.  The AA-016 program contains complete code for communication and
input/coin handling, so it would have to be some change in behaviour.
One possibility is that they decided they needed to ensure the coin
handling code is checksummed, and rearranged or refactored the code to
allow this.

The clock inputs for the MCUs are unknown - the values used are taken
from gladiatr, where a superficially similar arrangement is used.  It
would be good if we could get frequency measurements for all the clock
inputs:
* Master clock input to each MCU on XTAL 1 (DIP pin 2, PLCC pin 3)
* Serial clock input to communication MCUs on T0 (DIP pin 1, PLCC pin 2)

Communication between MCUs is working - the handlers for parity error
($187), premature end of data ($194) and excess data bits are not being
hit.  This confirms that the MCUs are synchronising correctly and the
serial clock is not excessively fast in relation to the MCU core clock.

The I/O MCU is correctly reading inputs, counting coins, driving coin
counters, and driving coin counters.  However, a problem elsewhere is
causing the main program to not reliably register credits.  It registers
credits occasionally, and once you start a game, it's playable - player
inputs work fine.

There are problems with sound.  Many effects aren't playing or are cut
off almost immediately.  It's not clear why - possibly due to some of
the I/O that isn't understood.

+-----+----------------+----------------+----------------+
| Pin | MCU 1          | MCU 2          | MCU 3          |
+-----+----------------+----------------+----------------+
| P10 | to MCU 2 P10   | to MCU 1 P10   | P1 right       |
| P11 | to MCU 2 T1    | to MCU 1 T1    | P1 left        |
| P12 | unknown        | DSW A 3        | unknown        |
| P13 | unknown        | DSW A 4        | P1 lower       |
| P14 | unknown        | DSW A 5        | P1 raise       |
| P15 | unknown        | unknown        | P1 middle      |
| P16 | unknown        | unknown        | 1P start       |
| P17 | unknown        | unknown        | 2P start       |
+-----+----------------+----------------+----------------+
| P20 | DSW C 1?       | DSW B 1?       | P2 right       |
| P21 | DSW C 2?       | DSW B 2?       | P2 left        |
| P22 | DSW C 3        | DSW B 3        | unknown        |
| P23 | DSW C 4        | DSW B 4        | P2 lower       |
| P24 | DSW C 5        | DSW B 5        | P2 raise       |
| P25 | DSW C 6        | DSW B 6        | P2 middle      |
| P26 | DSW C 7        | DSW B 7        | coin counter 1 |
| P27 | DSW C 8?       | DSW B 8        | coin counter 2 |
+-----+----------------+----------------+----------------+
| T0  | serial clock   | serial clock   | coin chute 1   |
| T1  | from MCU 2 P11 | from MCU 1 P11 | coin chute 2   |
+-----+----------------+----------------+----------------+

The communication MCUs transfer data bidirectionally on their P10 pins
which are tied together.  An MCU indicates that it is sending data by
pulling P11 low.  This is connected to T1 on the other MCU.  The data
clock is supplied to both communication MCUs on T0.  Outputs are set
after detecting a rising edge on T0, and inputs are read after detecting
a falling edge on T0.

A frame consists to five bytes send LSB first.  Each byte is followed by
an odd parity bit.  Useful addresses for debugging communication:
* $12A: Function for sending a frame from $21..$25
* $159: Function for receiving a frame to $29..$2D
* $187: Receive parity error
* $194: Premature end-of-data
* $199: Excess data

The I/O MCU handles rejecting coin pulses that are too long or too short
and converting coins to credits.  The duration is measured in terms of
rate that the sub CPU polls the MCU at.  The minimum coin pulse duration
is always two polling periods, while the maximum is programmable.  The
coins/credit settings for each coin input are programmable, but only the
first one can be set with DIP switches (the second one is always set for
1 coin/1 credit).

During the setup phase, the sub CPU writes setup instructions to the I/O
MCU's control port.  Each byte has an instruction in the high three bits
and a value in the low five bits.  Instructions recognised are:
* 0x00: Set coin 1 maximum pulse duration ($2E, default 6)
* 0x20: Set coin 2 maximum pulse duration ($2F, default 6)
* 0x40: Set coin 1 coins per credit ($35, default 1)
* 0x60: Set coin 2 coins per credit ($3D, default 1)
* 0x80: Set coin 1 credits per coin ($33, default 1)
* 0xa0: Set coin 2 credits per coin ($3B, default 1)
* 0xc0: Set value at $2A (never read, default $08)
* 0xe0: Swap nybbles and set value at $29 (never read, default $80)

The setup phase is terminated by writing a command to the I/O MCU's data
port.  The MCU return a data byte depending on the low three bits of the
command:
* xxxxxxx1 or xxxxx000: Credit flag and start buttons
* xxxxxx10: Credit flag and P1 controls
* xxxxx100: Credit flag and P2 controls

The MCU keeps an internal credit counter ($2D).  Each time the MCU
receives a polling command, if the credit counter is non-zero it will be
decremented and bit 7 of the response will be set; if the credit counter
is zero, but 7 of the response will be clear.

After terminating the setup phase, any byte written to the data port
will cause the MCU to check the coin inputs, update the credit counter
if necessary, and return the desired data.  A byte written to the
control port after the setup phase terminates causes the MCU to read a
byte of program memory from page 2 using the received byte as the
offset, twos-complement it, return it, and then immediately re-execute
the previous command received on the data port.

The I/O MCU indicates status via the user-defined status flags.  It sets
them to 0000 while waiting for a command, or 1111 while processing a
command.

MCU 2 P15, P16 and P17 seem to have some effect, although it's not clear
what they're supposed to do.  Tying them low during startup prevents the
sub CPU from correctly programming the I/O MCU.  It's not clear whether
pins corresponding to unused DIP switches have some other purpose. Fake
DIP switches have been included to make it easier to mess with some of
the unknown I/O.

The audio CPU seems to have some unmapped peripheral(s) around $FEB0.

******************************************/

#include "emu.h"
#include "includes/gsword.h"

#include "cpu/mcs48/mcs48.h"
#include "cpu/z80/z80.h"
#include "machine/clock.h"
#include "machine/i8255.h"
#include "screen.h"
#include "speaker.h"



void gsword_state_base::machine_start()
{
	save_item(NAME(m_fake8910_0));
	save_item(NAME(m_fake8910_1));
}

void gsword_state_base::machine_reset()
{
}

WRITE8_MEMBER(gsword_state_base::ay8910_control_port_0_w)
{
	m_ay0->address_w(data);
	m_fake8910_0 = data;
}
WRITE8_MEMBER(gsword_state_base::ay8910_control_port_1_w)
{
	m_ay1->address_w(data);
	m_fake8910_1 = data;
}

READ8_MEMBER(gsword_state_base::fake_0_r)
{
	return m_fake8910_0+1;
}
READ8_MEMBER(gsword_state_base::fake_1_r)
{
	return m_fake8910_1+1;
}


/* CPU 2 memory hack */
/* (402E) timeout upcount must be under 0AH                         */
/* (4004,4005) clear down counter , if (4004,4005)==0 then (402E)=0 */
READ8_MEMBER(gsword_state::hack_r)
{
	u8 const data = m_cpu2_ram[offset + 4];

	/*if(offset==1)osd_printf_debug("CNT %02X%02X\n",m_cpu2_ram[5],m_cpu2_ram[4]); */

	/* speedup timeout count down */
	if (m_protect_hack)
	{
		switch(offset)
		{
		case 0: return data & 0x7f;
		case 1: return 0x00;
		}
	}
	return data;
}

WRITE8_MEMBER(gsword_state::nmi_set_w)
{
/*  osd_printf_debug("AY write %02X\n",data);*/

	m_protect_hack = (data & 0x80) ? false : true;
#if 0
	/* An actual circuit isn't known. */
	/* write ff,02,ff,fe, 17 x 0d,0f */
	m_nmi_enable = ((data>>7) & (data&1) &1) == 0;


#else
	switch(data)
	{
	case 0xff:
		m_nmi_enable = false; /* NMI must be disabled */
		break;
	case 0x02:
		m_nmi_enable = false; /* ANY */
		break;
	case 0x0d:
		m_nmi_enable = true;
		break;
	case 0x0f:
		m_nmi_enable = true; /* NMI must be enabled */
		break;
	case 0xfe:
		m_nmi_enable = true; /* NMI must be enabled */
		break;
	}
	/* bit1= nmi disable , for ram check */
	logerror("NMI control %02x\n",data);
#endif
}

WRITE8_MEMBER(gsword_state::sound_command_w)
{
	m_soundlatch->write(data);
	m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

WRITE8_MEMBER(gsword_state::adpcm_data_w)
{
	m_msm->write_data(data & 0x0f); // bit 0..3
	m_msm->reset_w(BIT(data, 5));   // bit 5
	m_msm->vclk_w(BIT(data, 4));    // bit 4
}

READ8_MEMBER(gsword_state::mcu2_p1_r)
{
	// P10 is tied to P10 on MCU1, P11 is used to drive T1 on MCU1
	// assume the low bits of DIP switch A aren't connected at all
	return (m_dsw0->read() & 0xfc) | 0x02 | BIT(m_mcu1_p1, 0);
}

WRITE8_MEMBER(gsword_state::mcu3_p2_w)
{
	machine().bookkeeping().coin_counter_w(0, BIT(~data, 6));
	machine().bookkeeping().coin_counter_w(1, BIT(~data, 7));
}

INTERRUPT_GEN_MEMBER(gsword_state::sound_interrupt)
{
	if (m_nmi_enable)
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void gsword_state::init_gsword()
{
#if 0
	uint8_t *ROM2 = memregion("sub")->base();
	ROM2[0x1da] = 0xc3; /* patch for rom self check */

	ROM2[0x71e] = 0;    /* patch for sound protection or time out function */
	ROM2[0x71f] = 0;
#endif
#if 1
	/* hack for sound protection or time out function */
	m_subcpu->space(AS_PROGRAM).install_read_handler(0x4004, 0x4005, read8_delegate(FUNC(gsword_state::hack_r),this));
#endif
}

void gsword_state::init_gsword2()
{
#if 0
	uint8_t *ROM2 = memregion("sub")->base();

	ROM2[0x1da] = 0xc3; /* patch for rom self check */
	ROM2[0x726] = 0;    /* patch for sound protection or time out function */
	ROM2[0x727] = 0;
#endif
#if 1
	/* hack for sound protection or time out function */
	m_subcpu->space(AS_PROGRAM).install_read_handler(0x4004, 0x4005, read8_delegate(FUNC(gsword_state::hack_r),this));
#endif
}

void gsword_state::machine_start()
{
	gsword_state_base::machine_start();

	save_item(NAME(m_protect_hack));
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_tclk_val));
	save_item(NAME(m_mcu1_p1));
	save_item(NAME(m_mcu2_p1));
}

void gsword_state::machine_reset()
{
	gsword_state_base::machine_reset();

	m_protect_hack = false;
	m_nmi_enable = false;
}


READ8_MEMBER(josvolly_state::mcu1_p1_r)
{
	// the two MCUs appear to have port 1 tied together
	return m_mcu1_p1 & m_mcu2_p1;
}

READ8_MEMBER(josvolly_state::mcu1_p2_r)
{
	// p27 needs to float high for the MCU to start in the right mode
	// p20 and p21 drive the test inputs of the other MCU
	// if DIPSW1:8 is allowed to pull p27 low, the game won't even boot to test mode
	// DIPSW1:1 and DIPSW1:2 are shown in test mode, but switching them on will break comms
	return 0x80U | ioport("DSW1")->read();
}

READ8_MEMBER(josvolly_state::mcu2_p1_r)
{
	// the two MCUs appear to have port 1 tied together
	return m_mcu1_p1 & m_mcu2_p1;
}

READ8_MEMBER(josvolly_state::mcu2_p2_r)
{
	// p27 needs to be tied low for the MCU to start in the right mode
	return 0x7fU & ioport("DSW2")->read();
}

WRITE8_MEMBER(josvolly_state::cpu2_nmi_enable_w)
{
	m_cpu2_nmi_enable = true;
}

WRITE8_MEMBER(josvolly_state::cpu2_irq_clear_w)
{
	m_audiocpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}

WRITE8_MEMBER(josvolly_state::mcu1_p1_w)
{
	if (data != m_mcu1_p1)
	{
		logerror("mcu1 p1 = 0x%02x\n", data);
		m_mcu1_p1 = data;
	}
}

WRITE8_MEMBER(josvolly_state::mcu1_p2_w)
{
	if (data != m_mcu1_p2)
	{
		logerror("mcu1 p2 = 0x%02x\n", data);

		// the second CPU somehow gets an NMI when data is available
		// it's probably implemented by the logic arrays somehow
		// this is just a hacky guess at how it works
		if (m_cpu2_nmi_enable && (data & (data ^ m_mcu1_p2) & 0x01))
		{
			m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
			m_cpu2_nmi_enable = false;
		}

		m_mcu1_p2 = data;
	}
}

WRITE8_MEMBER(josvolly_state::mcu2_p1_w)
{
	if (data != m_mcu2_p1)
	{
		logerror("mcu2 p1 = 0x%02x\n", data);
		m_mcu2_p1 = data;
	}
}

WRITE8_MEMBER(josvolly_state::mcu2_p2_w)
{
	logerror("mcu2 p2 = 0x%02x\n", data);
}

void josvolly_state::machine_start()
{
	gsword_state_base::machine_start();

	save_item(NAME(m_cpu2_nmi_enable));
	save_item(NAME(m_mcu1_p1));
	save_item(NAME(m_mcu1_p2));
	save_item(NAME(m_mcu2_p1));
}

void josvolly_state::machine_reset()
{
	gsword_state_base::machine_reset();

	m_cpu2_nmi_enable = false;
	m_mcu1_p1 = 0xffU;
	m_mcu1_p2 = 0xffU;
	m_mcu2_p1 = 0xffU;
}


void gsword_state_base::cpu1_map(address_map &map)
{
	map(0x0000, 0x8fff).rom();
	map(0x9000, 0x9fff).ram();
	map(0xa000, 0xa37f).ram();
	map(0xa380, 0xa3ff).ram().share("spritetile_ram");
	map(0xa400, 0xa77f).ram();
	map(0xa780, 0xa7ff).ram().share("spritexy_ram");
	map(0xa980, 0xa980).w(FUNC(gsword_state_base::charbank_w));
	map(0xaa80, 0xaa80).w(FUNC(gsword_state_base::videoctrl_w));   /* flip screen, char palette bank */
	map(0xab00, 0xab00).w(FUNC(gsword_state_base::scroll_w));
	map(0xab80, 0xabff).writeonly().share("spriteattram");
	map(0xb000, 0xb7ff).readonly().w(FUNC(gsword_state_base::videoram_w)).share("videoram");
}


void gsword_state::cpu1_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x7e, 0x7f).rw("mcu1", FUNC(upi41_cpu_device::upi41_master_r), FUNC(upi41_cpu_device::upi41_master_w));
}

void gsword_state::cpu2_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x43ff).ram().share("cpu2_ram");
	map(0x6000, 0x6000).w(FUNC(gsword_state::sound_command_w));
}

void gsword_state::cpu2_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw("mcu3", FUNC(upi41_cpu_device::upi41_master_r), FUNC(upi41_cpu_device::upi41_master_w));
	map(0x40, 0x41).rw("mcu2", FUNC(upi41_cpu_device::upi41_master_r), FUNC(upi41_cpu_device::upi41_master_w));
	map(0x60, 0x60).rw(FUNC(gsword_state::fake_0_r), FUNC(gsword_state::ay8910_control_port_0_w));
	map(0x61, 0x61).rw(m_ay0, FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x80, 0x80).rw(FUNC(gsword_state::fake_1_r), FUNC(gsword_state::ay8910_control_port_1_w));
	map(0x81, 0x81).rw(m_ay1, FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));

	map(0xe0, 0xe0).nopr(); /* ?? */
	map(0xa0, 0xa0).nopw(); /* ?? */
	map(0xe0, 0xe0).nopw(); /* watchdog? */
}

void gsword_state::cpu3_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8000).w(FUNC(gsword_state::adpcm_data_w));
	map(0xa000, 0xa000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}


void josvolly_state::josvolly_cpu1_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x7e, 0x7f).rw("mcu1", FUNC(upi41_cpu_device::upi41_master_r), FUNC(upi41_cpu_device::upi41_master_w));
}

void josvolly_state::josvolly_cpu2_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x43ff).ram().share("cpu2_ram");

	/* NEC D8255A with silkscreen removed and replaced with "AA 007" */
	map(0x8000, 0x8003).rw("aa_007", FUNC(i8255_device::read), FUNC(i8255_device::write));

//  map(0x6000, 0x6000) AM_WRITE(adpcm_soundcommand_w)
	map(0xA000, 0xA001).rw("mcu2", FUNC(upi41_cpu_device::upi41_master_r), FUNC(upi41_cpu_device::upi41_master_w));
}

void josvolly_state::josvolly_cpu2_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw(FUNC(josvolly_state::fake_0_r), FUNC(josvolly_state::ay8910_control_port_0_w));
	map(0x01, 0x01).rw(m_ay0, FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x40, 0x40).rw(FUNC(josvolly_state::fake_1_r), FUNC(josvolly_state::ay8910_control_port_1_w));
	map(0x41, 0x41).rw(m_ay1, FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));

	map(0x81, 0x81).w(FUNC(josvolly_state::cpu2_nmi_enable_w));
	map(0xC1, 0xC1).w(FUNC(josvolly_state::cpu2_irq_clear_w));
}


static INPUT_PORTS_START( gsword )
	PORT_START("MCU1.P1") // TODO: fake port for debugging - should be removed
	PORT_BIT( 0x03, 0x02, IPT_UNUSED ) // these bits are used for communication
	PORT_DIPNAME( 0x04, 0x04, "MCU1.P12" )
	PORT_DIPSETTING(    0x04, DEF_STR(Off) )
	PORT_DIPSETTING(    0x00, DEF_STR(On) )
	PORT_DIPNAME( 0x08, 0x08, "MCU1.P13" )
	PORT_DIPSETTING(    0x08, DEF_STR(Off) )
	PORT_DIPSETTING(    0x00, DEF_STR(On) )
	PORT_DIPNAME( 0x10, 0x10, "MCU1.P14" )
	PORT_DIPSETTING(    0x10, DEF_STR(Off) )
	PORT_DIPSETTING(    0x00, DEF_STR(On) )
	PORT_DIPNAME( 0x20, 0x20, "MCU1.P15" )
	PORT_DIPSETTING(    0x20, DEF_STR(Off) )
	PORT_DIPSETTING(    0x00, DEF_STR(On) )
	PORT_DIPNAME( 0x40, 0x40, "MCU1.P16" )
	PORT_DIPSETTING(    0x40, DEF_STR(Off) )
	PORT_DIPSETTING(    0x00, DEF_STR(On) )
	PORT_DIPNAME( 0x80, 0x80, "MCU1.P17" )
	PORT_DIPSETTING(    0x80, DEF_STR(Off) )
	PORT_DIPSETTING(    0x00, DEF_STR(On) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_DIPNAME( 0x04, 0x04, "MCU3.P12" ) // TODO: fake DIP switch for debugging - should be removed
	PORT_DIPSETTING(    0x04, DEF_STR(Off) )
	PORT_DIPSETTING(    0x00, DEF_STR(On) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_DIPNAME( 0x04, 0x04, "MCU3.P22" ) // TODO: fake DIP switch for debugging - should be removed
	PORT_DIPSETTING(    0x04, DEF_STR(Off) )
	PORT_DIPSETTING(    0x00, DEF_STR(On) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED ) // P26 and P27 are outputs for coin counters

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )           PORT_DIPLOCATION("A:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) )           PORT_DIPLOCATION("A:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("A:3,4,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, "1 Coin/10 Credits" )
	// FIXME: these three DIP switches are physically present, but may not be connected to anything - turning them on seems to cause problems
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )          PORT_DIPLOCATION("A:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("A:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("A:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )           PORT_DIPLOCATION("B:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) )           PORT_DIPLOCATION("B:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Fencing Difficulty" )        PORT_DIPLOCATION("B:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x10, "Kendo Difficulty" )          PORT_DIPLOCATION("B:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x20, "Roman Difficulty" )          PORT_DIPLOCATION("B:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("B:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Lives ) )            PORT_DIPLOCATION("B:8")
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x00, "255 (Cheat)" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )           PORT_DIPLOCATION("C:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) )           PORT_DIPLOCATION("C:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("C:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("C:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x30, 0x30, "First Stage" )               PORT_DIPLOCATION("C:5,6")
	PORT_DIPSETTING(    0x30, "Fencing" )
	PORT_DIPSETTING(    0x20, "Kendo" )
	PORT_DIPSETTING(    0x10, "Roman" )
	PORT_DIPSETTING(    0x00, "Kendo" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("C:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )           PORT_DIPLOCATION("C:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( josvolly )
	PORT_START("IN0")       /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW , IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW , IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW , IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW , IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")       /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")       /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")      /* DSW1 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )   PORT_DIPLOCATION("DIPSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) )   PORT_DIPLOCATION("DIPSW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, "982E" )              PORT_DIPLOCATION("DIPSW1:3,4")
	PORT_DIPSETTING(    0x0c, "0" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x30, 0x30, "982A" )              PORT_DIPLOCATION("DIPSW1:5,6")
	PORT_DIPSETTING(    0x00, "60" )
	PORT_DIPSETTING(    0x10, "90" )
	PORT_DIPSETTING(    0x20, "120" )
	PORT_DIPSETTING(    0x30, "150" )
	PORT_DIPNAME( 0x40, 0x40, "TEST_MODE" )         PORT_DIPLOCATION("DIPSW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )   PORT_DIPLOCATION("DIPSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")      /* DSW2 */
	PORT_DIPNAME( 0x01, 0x01, "982C" )              PORT_DIPLOCATION("DIPSW2:8")
	PORT_DIPSETTING(    0x01, "0" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("DIPSW2:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("DIPSW2:6,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("DIPSW2:4,3")
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x40, 0x40, "9827" )              PORT_DIPLOCATION("DIPSW2:2")
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )   PORT_DIPLOCATION("DIPSW2:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout gsword_text =
{
	8,8,    /* 8x8 characters */
	1024,   /* 1024 characters */
	2,      /* 2 bits per pixel */
	{ 0, 4 },   /* the two bitplanes for 4 pixels are packed into one byte */
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8    /* every char takes 16 bytes */
};

static const gfx_layout gsword_sprites1 =
{
	16,16,   /* 16x16 sprites */
	64*2,    /* 128 sprites */
	2,       /* 2 bits per pixel */
	{ 0, 4 },   /* the two bitplanes for 4 pixels are packed into one byte */
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8     /* every sprite takes 64 bytes */
};

static const gfx_layout gsword_sprites2 =
{
	32,32,    /* 32x32 sprites */
	64,       /* 64 sprites */
	2,       /* 2 bits per pixel */
	{ 0, 4 }, /* the two bitplanes for 4 pixels are packed into one byte */
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3,
			64*8+0, 64*8+1, 64*8+2, 64*8+3, 72*8+0, 72*8+1, 72*8+2, 72*8+3,
			80*8+0, 80*8+1, 80*8+2, 80*8+3, 88*8+0, 88*8+1, 88*8+2, 88*8+3},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8,
			128*8, 129*8, 130*8, 131*8, 132*8, 133*8, 134*8, 135*8,
			160*8, 161*8, 162*8, 163*8, 164*8, 165*8, 166*8, 167*8 },
	64*8*4    /* every sprite takes (64*8=16x6)*4) bytes */
};

static GFXDECODE_START( gfx_gsword )
	GFXDECODE_ENTRY( "gfx1", 0, gsword_text,         0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, gsword_sprites1,  64*4, 64 )
	GFXDECODE_ENTRY( "gfx3", 0, gsword_sprites2,  64*4, 64 )
GFXDECODE_END


void gsword_state::gsword(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(18'000'000)/6); /* verified on pcb */
	m_maincpu->set_addrmap(AS_PROGRAM, &gsword_state::cpu1_map);
	m_maincpu->set_addrmap(AS_IO, &gsword_state::cpu1_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(gsword_state::irq0_line_hold));

	Z80(config, m_subcpu, XTAL(18'000'000)/6);  /* verified on pcb */
	m_subcpu->set_addrmap(AS_PROGRAM, &gsword_state::cpu2_map);
	m_subcpu->set_addrmap(AS_IO, &gsword_state::cpu2_io_map);
	m_subcpu->set_periodic_int(FUNC(gsword_state::sound_interrupt), attotime::from_hz(4*60));

	Z80(config, m_audiocpu, XTAL(18'000'000)/6);    /* verified on pcb */
	m_audiocpu->set_addrmap(AS_PROGRAM, &gsword_state::cpu3_map);

	upi41_cpu_device &mcu1(I8041(config, "mcu1", 12'000'000/2));        // clock unknown, using value from gladiatr
	mcu1.p1_in_cb().set([this] () { return ioport("MCU1.P1")->read() | BIT(m_mcu2_p1, 0); });
	mcu1.p1_out_cb().set([this] (uint8_t data) { m_mcu1_p1 = data; });
	mcu1.p2_in_cb().set_ioport("DSW2");
	mcu1.t0_in_cb().set([this] () { return m_tclk_val ? 1 : 0; });      // serial clock
	mcu1.t1_in_cb().set([this] () { return BIT(m_mcu2_p1, 1); });       // from P11 on other MCU

	upi41_cpu_device &mcu2(I8041(config, "mcu2", 12'000'000/2));        // clock unknown, using value from gladiatr
	mcu2.p1_in_cb().set(FUNC(gsword_state::mcu2_p1_r));
	mcu2.p1_out_cb().set([this] (uint8_t data) { m_mcu2_p1 = data; });
	mcu2.p2_in_cb().set_ioport("DSW1");
	mcu2.t0_in_cb().set([this] () { return m_tclk_val ? 1 : 0; });      // serial clock
	mcu2.t1_in_cb().set([this] () { return BIT(m_mcu1_p1, 1); });       // from P11 on other MCU

	upi41_cpu_device &mcu3(I8041(config, "mcu3", 12'000'000/2));        // clock unknown, using value from gladiatr
	mcu3.p1_in_cb().set_ioport("IN0");
	mcu3.p2_in_cb().set_ioport("IN1");
	mcu3.p2_out_cb().set(FUNC(gsword_state::mcu3_p2_w));
	mcu3.t0_in_cb().set_ioport("COINS").bit(0);
	mcu3.t1_in_cb().set_ioport("COINS").bit(1);

	// clock unknown, using value from gladiatr
	CLOCK(config, "tclk", 12'000'000/8/128/2).signal_handler().set([this] (int state) { m_tclk_val = state != 0; });

	// lazy way to ensure communication works
	config.m_perfect_cpu_quantum = subtag("mcu1");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(gsword_state::screen_update_gsword));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gsword);
	PALETTE(config, m_palette, FUNC(gsword_state::gsword_palette), 64*4 + 64*4, 256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	AY8910(config, m_ay0, XTAL(18'000'000)/12).add_route(ALL_OUTPUTS, "mono", 0.30); // Clock verified on PCB

	AY8910(config, m_ay1, 1500000);
	m_ay1->port_a_write_callback().set(FUNC(gsword_state::nmi_set_w));
	m_ay1->add_route(ALL_OUTPUTS, "mono", 0.30);

	msm5205_device &msm(MSM5205(config, "msm", XTAL(400'000))); /* verified on pcb */
	msm.set_prescaler_selector(msm5205_device::SEX_4B);  /* vclk input mode    */
	msm.add_route(ALL_OUTPUTS, "mono", 0.60);
}

void josvolly_state::josvolly(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 18000000/4);     /* ? */
	m_maincpu->set_addrmap(AS_PROGRAM, &josvolly_state::cpu1_map);
	m_maincpu->set_addrmap(AS_IO, &josvolly_state::josvolly_cpu1_io_map);
	m_maincpu->set_periodic_int(FUNC(josvolly_state::irq0_line_hold), attotime::from_hz(2*60));

	Z80(config, m_audiocpu, 12000000/4);    /* ? */
	m_audiocpu->set_addrmap(AS_PROGRAM, &josvolly_state::josvolly_cpu2_map);
	m_audiocpu->set_addrmap(AS_IO, &josvolly_state::josvolly_cpu2_io_map);
	m_audiocpu->set_vblank_int("screen", FUNC(josvolly_state::irq0_line_assert));

	upi41_cpu_device &mcu1(I8741(config, "mcu1", 18000000/2)); /* ? */
	mcu1.p1_in_cb().set(FUNC(josvolly_state::mcu1_p1_r));
	mcu1.p1_out_cb().set(FUNC(josvolly_state::mcu1_p1_w));
	mcu1.p2_in_cb().set(FUNC(josvolly_state::mcu1_p2_r));
	mcu1.p2_out_cb().set(FUNC(josvolly_state::mcu1_p2_w));

	upi41_cpu_device &mcu2(I8741(config, "mcu2", 12000000/2)); /* ? */
	mcu2.p1_in_cb().set(FUNC(josvolly_state::mcu2_p1_r));
	mcu2.p1_out_cb().set(FUNC(josvolly_state::mcu2_p1_w));
	mcu2.p2_in_cb().set(FUNC(josvolly_state::mcu2_p2_r));
	mcu2.p2_out_cb().set(FUNC(josvolly_state::mcu2_p2_w));
	// TEST0 and TEST1 are driven by P20 and P21 on the other MCU
	mcu2.t0_in_cb().set("mcu1", FUNC(upi41_cpu_device::p2_r)).bit(0);
	mcu2.t1_in_cb().set("mcu1", FUNC(upi41_cpu_device::p2_r)).bit(1);

	i8255_device &ppi(I8255(config, "aa_007"));
	ppi.in_pa_callback().set_ioport("IN1");   // 1PL
	ppi.in_pb_callback().set_ioport("IN2");   // 2PL / ACK
	ppi.in_pc_callback().set_ioport("IN0");   // START

	// the second MCU polls the first MCU's outputs, so it needs tight sync
	config.m_perfect_cpu_quantum = subtag("mcu2");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(josvolly_state::screen_update_gsword));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gsword);
	PALETTE(config, m_palette, FUNC(josvolly_state::josvolly_palette), 64*4 + 64*4, 256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	AY8910(config, m_ay0, 1500000).add_route(ALL_OUTPUTS, "mono", 0.30);
	AY8910(config, m_ay1, 1500000).add_route(ALL_OUTPUTS, "mono", 0.30);

#if 0
	MSM5205(config, "msm", 384000).add_route(ALL_OUTPUTS, "mono", 0.60);
#endif
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

// ac10-* ROM labels were written using a typewriter. The board is a Taito original however.
ROM_START( gsword )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ac10-01.2c",   0x0000, 0x2000, CRC(511b9389) SHA1(d24a083e812663522a06138dcc3aa60e48d27434) )
	ROM_LOAD( "ac1-2.2d",     0x2000, 0x2000, CRC(d772accf) SHA1(08028c6f026c118cc375ecff5c24dcb549475633) )
	ROM_LOAD( "ac10-03.2e",   0x4000, 0x2000, CRC(413a0ce6) SHA1(3dde7889db9f449aec5a05a4a3d27e12786df869) )
	ROM_LOAD( "ac1-4.2f",     0x6000, 0x2000, CRC(ca9d206d) SHA1(887eedc4e10218bf149c84399edd5d1e32c85051) )
	ROM_LOAD( "ac1-5.2h",     0x8000, 0x1000, CRC(2a892326) SHA1(a2cd91263714480c2569d3bbc73d62d222175e89) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "ac10-15.5h",   0x0000, 0x2000, CRC(b74e9d43) SHA1(d6e9e05e2e652c9d467dba1f1501d2a7ec8f851c) )
	ROM_LOAD( "ac0-16.7h",    0x2000, 0x2000, CRC(10accc10) SHA1(311961bfe852582a9c66aaecf9bc4c8f0ac7fccf) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    // 64K for 3nd z80
	ROM_LOAD( "ac10-12.3a",   0x0000, 0x2000, CRC(56eac59f) SHA1(22bde858ddcafad3f731030c39fd525458ecdbdd) )
	ROM_LOAD( "ac10-13.4a",   0x2000, 0x2000, CRC(3a920eaa) SHA1(256fafda0d522dee993b6840e60532f11a705345) )
	ROM_LOAD( "ac10-14.3d",   0x4000, 0x2000, CRC(819db933) SHA1(5e8b10d94ca6ba608a074bd5f30f14b95122fe85) )
	ROM_LOAD( "ac10-17.4d",   0x6000, 0x2000, CRC(87817985) SHA1(370399a4622958829ca6d1545e614b121f09c2c0) )

	ROM_REGION( 0x0400, "mcu1", 0 )    // D8741A-8
	ROM_LOAD( "aa-013.5a",    0x0000, 0x0400, CRC(e546aa52) SHA1(b8197c836713b1ace8ecd8238e645405c929364f) )

	ROM_REGION( 0x0400, "mcu2", 0 )    // 8041AH
	ROM_LOAD( "aa-016.9c",    0x0000, 0x0400, CRC(e546aa52) SHA1(b8197c836713b1ace8ecd8238e645405c929364f) )

	ROM_REGION( 0x0400, "mcu3", 0 )    // 8041AH
	ROM_LOAD( "aa-017.9g",    0x0000, 0x0400, CRC(e546aa52) SHA1(b8197c836713b1ace8ecd8238e645405c929364f) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "ac1-10.9n",    0x0000, 0x2000, CRC(517c571b) SHA1(05572a8ea416922da50143936fda9ba038f0b91e) )    // tiles
	ROM_LOAD( "ac1-11.9p",    0x2000, 0x2000, CRC(7a1d8a3a) SHA1(3f90be9ddba3cf7a879fd69ac67c2b67fd63b9ee) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "ac1-6.9e",     0x0000, 0x2000, CRC(1b0a3cb7) SHA1(0b0f17b9844d7310b46110559e09cfc3b50bb38b) )    // sprites

	ROM_REGION( 0x4000, "gfx3", 0 )
	ROM_LOAD( "ac0-7.9f",     0x0000, 0x2000, CRC(ef5f28c6) SHA1(85d943e5c5136d9458118f676b0c79fcf3aaf0c4) )
	ROM_LOAD( "ac0-8.9h",     0x2000, 0x2000, CRC(46824b30) SHA1(f6880b1c31ae795e3781d16ee96145df1db60328) )

	ROM_REGION( 0x0360, "proms", 0 )
	ROM_LOAD( "ac0-1.11c",    0x0000, 0x0100, CRC(5c4b2adc) SHA1(0a6fdd60bdbd56bb7573147e4a976e5d0ddf43b5) )    // palette low bits
	ROM_LOAD( "ac0-2.11cd",   0x0100, 0x0100, CRC(966bda66) SHA1(05439508113b3e51a16ee87d3f4691aa8901ebcb) )    // palette high bits
	ROM_LOAD( "ac0-3.8c",     0x0200, 0x0100, CRC(dae13f77) SHA1(d4d105542955e806311987dd3c4ffce1e13caf91) )    // sprite lookup table
	ROM_LOAD( "003.4e",       0x0300, 0x0020, CRC(43a548b8) SHA1(d01529d7f8f5101232cdf3490fdb2c61bf179181) )    // address decoder? not used
	ROM_LOAD( "004.4d",       0x0320, 0x0020, CRC(43a548b8) SHA1(d01529d7f8f5101232cdf3490fdb2c61bf179181) )    // address decoder? not used
	ROM_LOAD( "005.3h",       0x0340, 0x0020, CRC(e8d6dec0) SHA1(d15cba9a4b24255d41046b15c2409391ab13ce95) )    // address decoder? not used
ROM_END

ROM_START( gsword2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ac1-1.2c",     0x0000, 0x2000, CRC(565c4d9e) SHA1(17b86e86ab95aeb458b8368c8c04666a1ccd9eee) )
	ROM_LOAD( "ac1-2.2d",     0x2000, 0x2000, CRC(d772accf) SHA1(08028c6f026c118cc375ecff5c24dcb549475633) )
	ROM_LOAD( "ac1-3.2e",     0x4000, 0x2000, CRC(2cee1871) SHA1(df099209c56f2807e4fdb83c625368f5e7e583e5) )
	ROM_LOAD( "ac1-4.2f",     0x6000, 0x2000, CRC(ca9d206d) SHA1(887eedc4e10218bf149c84399edd5d1e32c85051) )
	ROM_LOAD( "ac1-5.2h",     0x8000, 0x1000, CRC(2a892326) SHA1(a2cd91263714480c2569d3bbc73d62d222175e89) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "ac0-15.5h",    0x0000, 0x2000, CRC(1aa4690e) SHA1(7b0dbc38f3e6af2c9efa44b6759a3cdd9adc992d) )
	ROM_LOAD( "ac0-16.7h",    0x2000, 0x2000, CRC(10accc10) SHA1(311961bfe852582a9c66aaecf9bc4c8f0ac7fccf) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    // 64K for 3nd z80
	ROM_LOAD( "ac0-12.3a",    0x0000, 0x2000, CRC(a6589068) SHA1(9385abe2449c5c5bac8f49d2afd140acea1791c3) )
	ROM_LOAD( "ac0-13.4a",    0x2000, 0x2000, CRC(4ee79796) SHA1(3353625903f63910a18fae0a9568a96d75592328) )
	ROM_LOAD( "ac0-14.3d",    0x4000, 0x2000, CRC(455364b6) SHA1(ebabf077d1ba113c13e7620d61720ed141acb5ad) )
	// 6000-7fff empty

	ROM_REGION( 0x0400, "mcu1", 0 )    // D8741A-8
	ROM_LOAD( "aa-013.5a",    0x0000, 0x0400, CRC(e546aa52) SHA1(b8197c836713b1ace8ecd8238e645405c929364f) )

	ROM_REGION( 0x0400, "mcu2", 0 )    // 8041AH
	ROM_LOAD( "aa-016.9c",    0x0000, 0x0400, CRC(e546aa52) SHA1(b8197c836713b1ace8ecd8238e645405c929364f) )

	ROM_REGION( 0x0400, "mcu3", 0 )    // 8041AH
	ROM_LOAD( "aa-017.9g",    0x0000, 0x0400, CRC(e546aa52) SHA1(b8197c836713b1ace8ecd8238e645405c929364f) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "ac1-10.9n",    0x0000, 0x2000, CRC(517c571b) SHA1(05572a8ea416922da50143936fda9ba038f0b91e) )    // tiles
	ROM_LOAD( "ac1-11.9p",    0x2000, 0x2000, CRC(7a1d8a3a) SHA1(3f90be9ddba3cf7a879fd69ac67c2b67fd63b9ee) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "ac1-6.9e",     0x0000, 0x2000, CRC(1b0a3cb7) SHA1(0b0f17b9844d7310b46110559e09cfc3b50bb38b) )    // sprites

	ROM_REGION( 0x4000, "gfx3", 0 )
	ROM_LOAD( "ac0-7.9f",     0x0000, 0x2000, CRC(ef5f28c6) SHA1(85d943e5c5136d9458118f676b0c79fcf3aaf0c4) )
	ROM_LOAD( "ac0-8.9h",     0x2000, 0x2000, CRC(46824b30) SHA1(f6880b1c31ae795e3781d16ee96145df1db60328) )

	ROM_REGION( 0x0360, "proms", 0 )
	ROM_LOAD( "ac0-1.11c",    0x0000, 0x0100, CRC(5c4b2adc) SHA1(0a6fdd60bdbd56bb7573147e4a976e5d0ddf43b5) )    // palette low bits
	ROM_LOAD( "ac0-2.11cd",   0x0100, 0x0100, CRC(966bda66) SHA1(05439508113b3e51a16ee87d3f4691aa8901ebcb) )    // palette high bits
	ROM_LOAD( "ac0-3.8c",     0x0200, 0x0100, CRC(dae13f77) SHA1(d4d105542955e806311987dd3c4ffce1e13caf91) )    // sprite lookup table
	ROM_LOAD( "003.4e",       0x0300, 0x0020, CRC(43a548b8) SHA1(d01529d7f8f5101232cdf3490fdb2c61bf179181) )    // address decoder? not used
	ROM_LOAD( "004.4d",       0x0320, 0x0020, CRC(43a548b8) SHA1(d01529d7f8f5101232cdf3490fdb2c61bf179181) )    // address decoder? not used
	ROM_LOAD( "005.3h",       0x0340, 0x0020, CRC(e8d6dec0) SHA1(d15cba9a4b24255d41046b15c2409391ab13ce95) )    // address decoder? not used
ROM_END

ROM_START( josvolly )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "aa2-1.2c",     0x0000, 0x2000, CRC(27f740a5) SHA1(3e038386e743fdf718e795a944ff4b631a492958) )
	ROM_LOAD( "aa1-2.2d",     0x2000, 0x2000, CRC(3e02e3e1) SHA1(cc0aee321cf5232438cd6e38635c9060056ad361) )
	ROM_LOAD( "aa0-3.2e",     0x4000, 0x2000, CRC(72843ffe) SHA1(fe70727bbcb0622df81eca2969c1a85398767479) )
	ROM_LOAD( "aa1-4.2f",     0x6000, 0x2000, CRC(22c1466e) SHA1(d86093903e473252c35170e35d7f9ee34194086d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "aa3-12.2h",    0x0000, 0x1000, CRC(3796bbf6) SHA1(8741f556ddb06e7779d1e8abc3d06688881f8269) )
	ROM_LOAD( "aa0-13.2j",    0x2000, 0x2000, CRC(58cc89ac) SHA1(9785ec27e593b3e249da7a1b6b025c6d573e28f9) )

	ROM_REGION( 0x04000, "user1", 0 )   // music data and samples - not sure where it's mapped
	ROM_LOAD( "aa0-14.4j",    0x0000, 0x2000, CRC(436fe91f) SHA1(feb29501090c6db911e13ce6e9935ba004b0ce7e) )

	ROM_REGION( 0x400, "mcu1", 0 )
	ROM_LOAD( "aa003.bin",    0x0000, 0x400, CRC(68b399d9) SHA1(053482d12c2b714c23fc80ad0589a2afd258a5a6) )

	ROM_REGION( 0x400, "mcu2", 0 )
	ROM_LOAD( "aa008.bin",    0x0000, 0x400, CRC(68b399d9) SHA1(053482d12c2b714c23fc80ad0589a2afd258a5a6) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "aa0-10.9n",    0x0000, 0x2000, CRC(207c4f42) SHA1(4cf2922d55cfc9e68cc07c3252ea3b5619b8aca5) )    // tiles */
	ROM_LOAD( "aa1-11.9p",    0x2000, 0x1000, CRC(c130464a) SHA1(9d23577b8aaaffeefff3d8f93668d1b2bd0ba3d9) )
	ROM_RELOAD(               0x3000, 0x1000 ) // title screen data is actually read from here

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "aa0-6.9e",     0x0000, 0x2000, CRC(c2c2401a) SHA1(ef987d53d9e502277086f39b455174d3539572e6) )    // sprites */

	ROM_REGION( 0x4000, "gfx3", 0 )
	ROM_LOAD( "aa0-7.9f",     0x0000, 0x2000, CRC(da836231) SHA1(209723778b705dba8206b56c3b8f0996f02ba8d5) )
	ROM_LOAD( "aa0-8.9h",     0x2000, 0x2000, CRC(a0426d57) SHA1(d029408e005ea57f4902c081203f3d3980a5f927) )

	ROM_REGION( 0x0460, "proms", 0 )
	ROM_LOAD( "a1.10k",       0x0000, 0x0100, CRC(09f7b56a) SHA1(9b82d1d4ebab14b366dc0ca95c933e37811ac155) )    // palette red?
	ROM_LOAD( "a2.9k",        0x0100, 0x0100, CRC(852eceac) SHA1(6ed7011b45cf767d6503b92d29a14a7b8e099a76) )    // palette green?
	ROM_LOAD( "a3.9j",        0x0200, 0x0100, CRC(1312718b) SHA1(4a7d7eae4d8ea085eead46758832fddac7aff0b0) )    // palette blue?
	ROM_LOAD( "a4.8c",        0x0300, 0x0100, CRC(1dcec967) SHA1(4d36842c2fd929a6508a58bc8ea7e0372296e575) )    // sprite lookup table
	ROM_LOAD( "003.4e",       0x0400, 0x0020, CRC(43a548b8) SHA1(d01529d7f8f5101232cdf3490fdb2c61bf179181) )    // address decoder? not used
	ROM_LOAD( "004.4d",       0x0420, 0x0020, CRC(43a548b8) SHA1(d01529d7f8f5101232cdf3490fdb2c61bf179181) )    // address decoder? not used
	ROM_LOAD( "005.3h",       0x0440, 0x0020, CRC(e8d6dec0) SHA1(d15cba9a4b24255d41046b15c2409391ab13ce95) )    // address decoder? not used
ROM_END


GAME( 1983, josvolly, 0,      josvolly, josvolly, josvolly_state, empty_init,   ROT90, "Allumer / Taito Corporation", "Joshi Volleyball",         MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, gsword,   0,      gsword,   gsword,   gsword_state,   init_gsword,  ROT0,  "Allumer / Taito Corporation", "Great Swordsman (World?)", MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, gsword2,  gsword, gsword,   gsword,   gsword_state,   init_gsword2, ROT0,  "Allumer / Taito Corporation", "Great Swordsman (Japan?)", MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

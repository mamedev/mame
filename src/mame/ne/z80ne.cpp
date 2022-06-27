// license:BSD-3-Clause
// copyright-holders:Roberto Lavarone
/******************************************************************************
    Nuova Elettronica Z80NE system driver

    Preliminary driver

    Roberto Lavarone, 2009-01-05

    Thanks go to:
        Roberto Bazzano: www.z80ne.com

    Z80NE memory map

        LX.382 CPU board
            range     short     description
            0000-03FF RAM
            8000-83FF EPROM     firmware

        LX.383 HEX keyboard and display controller for LX.384 hex keyboard and display
            range     short     description
            F0-F7     I/O       7-segment LED dual-port RAM write
            F0        I/O       keyboard read
            F8        I/O       enable single step for next instruction

        LX.385 Cassette interface
            range     short     description
            EE        I/O       UART Data Read/Write
            EF        I/O       UART Status/Control - Cassette Tape Control

        LX.392 32K Memory expansion board
            range     short     description


        LX.389 Printer Interface
            range     short     description
            02-03
            06-07
            0A-0B
            0E-0F
            12-13
            16-17
            1A-1B
            1E-1F

        LX.548 16K Basic eprom
            range     short     description
            0000-3FFF EPROM     firmware

        LX.388 Video Interface
            range     short     description
            EC00-EDFF RAM
            EA        I/O       keyboard
            EB        I/O       video retrace

        LX.529 Graphics Video and Printer Interface
            range     short     description
            80        I/O       PIO 0 port A data (ram 0)
            81        I/O       PIO 0 port A control (ram 0)
            82        I/O       PIO 0 port B data (printer)
            83        I/O       PIO 0 port B control (printer)
            84        I/O       PIO 1 port A data (ram 1)
            85        I/O       PIO 1 port A control (ram 1)
            86        I/O       PIO 1 port B data (keyboard)
            87        I/O       PIO 1 port B control (keyboard)
            88        I/O       PIO 2 port A data (ram 2)
            89        I/O       PIO 2 port A control (ram 2)
            8A        I/O       PIO 2 port B data (printer busy + 40/80 video chars)
            8B        I/O       PIO 2 port B control (printer busy + 40/40 video chars)
            8C        I/O       SY6845 address and status register
            8D        I/O       SY6845 data register
            8E        I/O       RAM 3 character attribute
            8F        I/O       beeper

        LX.390 Floppy Interface
            range     short     description
            F000-F3FF EPROM     firmware
            D0        I/O       command/status register
            D1        I/O       trace register
            D2        I/O       sector register
            D3        I/O       data register (write only if controller idle)
            D6        I/O       drive select / drive side one select
            D7        I/O       data register (write always)

        LX.394-395 EPROM Programmer
            range     short     description
            9000-9FFF EPROM     EPROM to be written
            8400-8FFF EPROM     firmware

Quick Instructions:
  Z80NE:
  - Use 0-F to enter an address (or use mouse in artwork)
  - Hold CTRL press 0 to show data at that address (CTRL cannot be held with mouse)
  - Use 0-F to enter data
  - CTRL 0 to advance to next address
  - CTRL 2 to view/change CPU registers
  - CTRL 0 change register, advance to next
  Z80NET
  - In Machine Configuration, select your preferred baud rate, and reboot machine.
  - CTRL 6 to load a tape, press A or B to choose tape device, choose Play.
  - After this, click any key, enter 1000, CTRL 4 to run.
  - CTRL 5 to save
  - All tapes in software list (except bioritmi & tapebas) are BASIC programs.
  - To use tapebas, load side 1 in the normal way, run it, select side 2 for cas 0, play, the rest loads
  - The Basic is bilingual - ENG for English, ITA for Italian, so enter ENG.
  - Then for any Basic program in software list: CLOAD hit Play RUN
  Z80NETB
  - BASIC-only, English only. A version of TRS80 Level II Basic. Not compatible with software list.
  Z80NETF
  - There is a choice of 5 bioses, via the Machine Configuration menu
  - EP382:  same as Z80NET.
  - EP548:  same as Z80NETB.
  - EP390:  for swlist-item "basic55k". Press B, from the menu select 2, runs. Type ENG for English.
            It can load and run tapes from the swlist, although it seems to often have load errors.
  - EP1390: requires a floppy to boot from. Disks marked as NE-DOS 1.5 should work.
  - EP2390: uses ports 8x, not emulated, not working. For NE-DOS G.1

Natural Keyboard and Paste:
    - The hexpad keys conflict with the keyboard keys, therefore Natural Keyboard is not
      supported. Paste is meant for Z80NE only.

    - Test Paste:
      N1000=11=22=33=44=55=66=77=88=99=N1000=
      Press Ctrl+0 to review the data.


*********************************************************************************************************/

/* Core includes */
#include "emu.h"
#include "includes/z80ne.h"

#include "cpu/z80/z80.h"
#include "machine/ram.h"

#include "softlist_dev.h"
#include "speaker.h"

#include "formats/dmk_dsk.h"

/* Layout */
#include "z80ne.lh"
#include "z80net.lh"
#include "z80netb.lh"
#include "z80netf.lh"


/******************************************************************************
 Memory Maps
******************************************************************************/


/* LX.382 CPU Board RAM */
/* LX.382 CPU Board EPROM */
void z80ne_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).ram().share("mainram");
	map(0x8000, 0x83ff).rom().region("maincpu",0);
}

void z80net_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).ram().share("mainram");
	map(0x8000, 0x83ff).rom().region("maincpu",0);
	map(0x8400, 0xebff).ram();
	map(0xec00, 0xedff).ram().share("videoram"); /* (6847) */
	map(0xee00, 0xffff).ram();
}

void z80netb_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0xebff).ram();
	map(0xec00, 0xedff).ram().share("videoram"); /* (6847) */
	map(0xee00, 0xffff).ram();
}

void z80ne_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xee, 0xee).rw(m_uart, FUNC(ay31015_device::receive), FUNC(ay31015_device::transmit));
	map(0xef, 0xef).rw(FUNC(z80ne_state::lx385_ctrl_r), FUNC(z80ne_state::lx385_ctrl_w));
	map(0xf0, 0xff).rw(FUNC(z80ne_state::lx383_r), FUNC(z80ne_state::lx383_w));
}

void z80net_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xea, 0xea).r(FUNC(z80net_state::lx387_data_r));
	map(0xeb, 0xeb).r(FUNC(z80net_state::lx388_read_field_sync));
	map(0xee, 0xee).rw(m_uart, FUNC(ay31015_device::receive), FUNC(ay31015_device::transmit));
	map(0xef, 0xef).rw(FUNC(z80net_state::lx385_ctrl_r), FUNC(z80net_state::lx385_ctrl_w));
	map(0xf0, 0xff).rw(FUNC(z80net_state::lx383_r), FUNC(z80net_state::lx383_w));
}

void z80netf_state::mem_map(address_map &map)
{
	map(0x0000, 0x03ff).bankrw("bank1");
	map(0x0400, 0x3fff).bankrw("bank2");
	map(0x4000, 0x7fff).ram();
	map(0x8000, 0x83ff).bankrw("bank3");
	map(0x8400, 0xdfff).ram();
	map(0xec00, 0xedff).ram().share("videoram"); /* (6847) */
	map(0xf000, 0xf3ff).bankrw("bank4");
}

void z80netf_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xd0, 0xd7).rw(FUNC(z80netf_state::lx390_fdc_r), FUNC(z80netf_state::lx390_fdc_w));
	map(0xea, 0xea).r(FUNC(z80netf_state::lx387_data_r));
	map(0xeb, 0xeb).r(FUNC(z80netf_state::lx388_read_field_sync));
	map(0xee, 0xee).rw(m_uart, FUNC(ay31015_device::receive), FUNC(ay31015_device::transmit));
	map(0xef, 0xef).rw(FUNC(z80netf_state::lx385_ctrl_r), FUNC(z80netf_state::lx385_ctrl_w));
	map(0xf0, 0xff).rw(FUNC(z80netf_state::lx383_r), FUNC(z80netf_state::lx383_w));
}



/******************************************************************************
 Input Ports
******************************************************************************/


static INPUT_PORTS_START( z80ne )
	/* LX.384 Hex Keyboard and Display */
	/*
	 * In natural mode the CTRL key is mapped on shift
	 */
	PORT_START("ROW0")          /* IN0 keys row 0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 0") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('=') // set address, increment
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 1") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('-') // ?
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 2") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('R') // registers
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 3") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('T') // single-step
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 4") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('X') // go
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 5") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('L') // load
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 6") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('S') // save
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 7") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('K') // reserved for future expansion

	PORT_START("ROW1")          /* IN1 keys row 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("CTRL")          /* CONTROL key */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("RST")           /* RESET key */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 Reset")  PORT_CODE(KEYCODE_F3) PORT_CHANGED_MEMBER(DEVICE_SELF, z80ne_state, z80ne_reset, 0) PORT_CHAR('N')

	/* Settings - need to reboot after altering these */
	PORT_START("LX.385")
	PORT_CONFNAME(0x07, 0x04, "LX.385 Cassette: P1,P3 Data Rate")
	PORT_CONFSETTING( 0x01, "A-B: 300 bps")
	PORT_CONFSETTING( 0x02, "A-C: 600 bps")
	PORT_CONFSETTING( 0x04, "A-D: 1200 bps")
	PORT_CONFNAME( 0x08, 0x00, "LX.385: P4 Parity Check")
	PORT_CONFSETTING( 0x00, "Parity Check Enabled")
	PORT_CONFSETTING( 0x08, "Parity Check Disabled")

INPUT_PORTS_END


static INPUT_PORTS_START( z80net )

	PORT_INCLUDE( z80ne )

	/* LX.387 Keyboard BREAK key */
	PORT_START("LX387_BRK")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Break") PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(END)) PORT_CHANGED_MEMBER(DEVICE_SELF, z80net_state, z80net_nmi, 0)

	/* LX.387 Keyboard (Encoded by KR2376) */

	PORT_START("X0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD )                               PORT_CHAR('_')

	PORT_START("X3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)          PORT_CHAR('0')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('`') PORT_CHAR('@')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(8)                   PORT_NAME("Del")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)                  PORT_NAME("CR")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_CHAR(10) PORT_NAME("LF")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X4")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')

	PORT_START("X5")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)          PORT_CHAR('c') PORT_CHAR('G')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("X6")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')

	PORT_START("X7")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')

	PORT_START("MODIFIERS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Shift Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_TOGGLE

INPUT_PORTS_END


static INPUT_PORTS_START( z80netf )

PORT_INCLUDE( z80net )

	/* Settings */
	PORT_START("CONFIG")
	PORT_CONFNAME(0x07, 0x01, "Boot EPROM")
	PORT_CONFSETTING(   0x01, "EP382  Hex Monitor")
	PORT_CONFSETTING(   0x02, "EP548  16k BASIC")
	PORT_CONFSETTING(   0x03, "EP390  Boot Loader for 5.5k floppy BASIC")
	PORT_CONFSETTING(   0x04, "EP1390 Boot Loader for NE DOS 1.0/1.5")
	PORT_CONFSETTING(   0x05, "EP2390 Boot Loader for NE DOS G.1")
	PORT_BIT(0xf8, 0xf8, IPT_UNUSED)

INPUT_PORTS_END




/******************************************************************************
 Machine Drivers
******************************************************************************/
#if 0
static const uint32_t lx388palette[] =
{
	rgb_t(0x00, 0xff, 0x00), /* GREEN */
	rgb_t(0x00, 0xff, 0x00), /* YELLOW in original, here GREEN */
	rgb_t(0x00, 0x00, 0xff), /* BLUE */
	rgb_t(0xff, 0x00, 0x00), /* RED */
	rgb_t(0xff, 0xff, 0xff), /* BUFF */
	rgb_t(0x00, 0xff, 0xff), /* CYAN */
	rgb_t(0xff, 0x00, 0xff), /* MAGENTA */
	rgb_t(0xff, 0x80, 0x00), /* ORANGE */

	rgb_t(0x00, 0x20, 0x00), /* BLACK in original, here DARK green */
	rgb_t(0x00, 0xff, 0x00), /* GREEN */
	rgb_t(0x00, 0x00, 0x00), /* BLACK */
	rgb_t(0xff, 0xff, 0xff), /* BUFF */

	rgb_t(0x00, 0x20, 0x00), /* ALPHANUMERIC DARK GREEN */
	rgb_t(0x00, 0xff, 0x00), /* ALPHANUMERIC BRIGHT GREEN */
	rgb_t(0x40, 0x10, 0x00), /* ALPHANUMERIC DARK ORANGE */
	rgb_t(0xff, 0xc4, 0x18)  /* ALPHANUMERIC BRIGHT ORANGE */
};
#endif

void z80ne_state::floppy_formats(format_registration &fr)
{
	fr.add_fm_containers();
	fr.add(FLOPPY_DMK_FORMAT);
}

static void z80ne_floppies(device_slot_interface &device)
{
	device.option_add("sssd", FLOPPY_525_SSSD);
}

void z80ne_state::z80ne(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, Z80NE_CPU_SPEED_HZ);
	m_maincpu->set_addrmap(AS_PROGRAM, &z80ne_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &z80ne_state::io_map);

	AY31015(config, m_uart);

	CLOCK(config, m_uart_clock, 4800);
	m_uart_clock->signal_handler().set(FUNC(z80ne_state::lx385_uart_tx_clock_w));
	m_uart_clock->signal_handler().append(m_uart, FUNC(ay31015_device::write_rcp));

	SPEAKER(config, "mono").front_center();

	CASSETTE(config, m_cassette1);
	m_cassette1->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette1->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette1->set_interface("z80ne_cass");

	CASSETTE(config, m_cassette2);
	m_cassette2->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette2->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette2->set_interface("z80ne_cass");

	config.set_default_layout(layout_z80ne);

	// all known tapes require LX.388 expansion
	//SOFTWARE_LIST(config, "cass_list").set_original("z80ne_cass");
}

void z80net_state::lx387(machine_config &config)
{
	KR2376_ST(config, m_lx387_kr2376, 50000);
	m_lx387_kr2376->x<0>().set_ioport("X0");
	m_lx387_kr2376->x<1>().set_ioport("X1");
	m_lx387_kr2376->x<2>().set_ioport("X2");
	m_lx387_kr2376->x<3>().set_ioport("X3");
	m_lx387_kr2376->x<4>().set_ioport("X4");
	m_lx387_kr2376->x<5>().set_ioport("X5");
	m_lx387_kr2376->x<6>().set_ioport("X6");
	m_lx387_kr2376->x<7>().set_ioport("X7");
	m_lx387_kr2376->shift().set(FUNC(z80net_state::lx387_shift_r));
	m_lx387_kr2376->control().set(FUNC(z80net_state::lx387_control_r));
}

void z80net_state::z80net(machine_config &config)
{
	z80ne(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &z80net_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &z80net_state::io_map);

	lx387(config);

	/* video hardware */
	SCREEN(config, "lx388", SCREEN_TYPE_RASTER);

	MC6847_PAL(config, m_vdg, 4.433619_MHz_XTAL);
	m_vdg->set_screen("lx388");
	m_vdg->input_callback().set(FUNC(z80net_state::lx388_mc6847_videoram_r));
	// AG = GND, GM2 = GND, GM1 = GND, GM0 = GND, CSS = GND
	// other lines not connected

	config.set_default_layout(layout_z80net);

	SOFTWARE_LIST(config, "cass_list").set_original("z80ne_cass");
}

void z80netb_state::z80netb(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, Z80NE_CPU_SPEED_HZ);
	m_maincpu->set_addrmap(AS_PROGRAM, &z80netb_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &z80netb_state::io_map);

	AY31015(config, m_uart);

	CLOCK(config, m_uart_clock, 4800);
	m_uart_clock->signal_handler().set(FUNC(z80netb_state::lx385_uart_tx_clock_w));
	m_uart_clock->signal_handler().append(m_uart, FUNC(ay31015_device::write_rcp));

	SPEAKER(config, "mono").front_center();

	CASSETTE(config, m_cassette1);
	m_cassette1->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette1->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette1->set_interface("z80ne_cass");

	CASSETTE(config, m_cassette2);
	m_cassette2->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette2->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette2->set_interface("z80ne_cass");

	lx387(config);

	/* video hardware */
	SCREEN(config, "lx388", SCREEN_TYPE_RASTER);

	MC6847_PAL(config, m_vdg, 4.433619_MHz_XTAL);
	m_vdg->set_screen("lx388");
	m_vdg->input_callback().set(FUNC(z80netb_state::lx388_mc6847_videoram_r));
	// AG = GND, GM2 = GND, GM1 = GND, GM0 = GND, CSS = GND
	// other lines not connected

	config.set_default_layout(layout_z80netb);

	// none of the software is compatible
	//SOFTWARE_LIST(config, "cass_list").set_original("z80ne_cass");
}

void z80netf_state::z80netf(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, Z80NE_CPU_SPEED_HZ);
	m_maincpu->set_addrmap(AS_PROGRAM, &z80netf_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &z80netf_state::io_map);

	AY31015(config, m_uart);

	CLOCK(config, m_uart_clock, 4800);
	m_uart_clock->signal_handler().set(FUNC(z80netf_state::lx385_uart_tx_clock_w));
	m_uart_clock->signal_handler().append(m_uart, FUNC(ay31015_device::write_rcp));

	SPEAKER(config, "mono").front_center();

	CASSETTE(config, m_cassette1);
	m_cassette1->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette1->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette1->set_interface("z80ne_cass");

	CASSETTE(config, m_cassette2);
	m_cassette2->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette2->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette2->set_interface("z80ne_cass");

	lx387(config);

	/* video hardware */
	SCREEN(config, "lx388", SCREEN_TYPE_RASTER);

	MC6847_PAL(config, m_vdg, 4.433619_MHz_XTAL);
	m_vdg->set_screen("lx388");
	m_vdg->input_callback().set(FUNC(z80netf_state::lx388_mc6847_videoram_r));
	// AG = GND, GM2 = GND, GM1 = GND, GM0 = GND, CSS = GND
	// other lines not connected

	FD1771(config, m_wd1771, 2_MHz_XTAL / 2);
	FLOPPY_CONNECTOR(config, "wd1771:0", z80ne_floppies, "sssd", z80ne_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "wd1771:1", z80ne_floppies, "sssd", z80ne_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "wd1771:2", z80ne_floppies, nullptr,   z80ne_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "wd1771:3", z80ne_floppies, nullptr,   z80ne_state::floppy_formats);

	config.set_default_layout(layout_z80netf);

	/* internal ram */
	RAM(config, m_ram).set_default_size("56K").set_default_value(0x00);

	SOFTWARE_LIST(config, "cass_list").set_original("z80ne_cass");
	SOFTWARE_LIST(config, "flop_list").set_original("z80ne_flop");
}

/******************************************************************************
 ROM Definitions
******************************************************************************/


ROM_START( z80ne )
	ROM_REGION(0x0400, "maincpu", 0)
	ROM_LOAD( "ep382.ic5", 0x0000, 0x0400, CRC(55818366) SHA1(adcac04b83c09265517b7bafbc2f5f665d751bec) )
ROM_END

ROM_START( z80net )
	ROM_REGION(0x0400, "maincpu", 0)
	ROM_LOAD( "ep382.ic5", 0x0000, 0x0400, CRC(55818366) SHA1(adcac04b83c09265517b7bafbc2f5f665d751bec) )
ROM_END

ROM_START( z80netb )
/*
 * 16k Basic
 */
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD( "548-1.ic1", 0x0000, 0x0800, CRC(868cad39) SHA1(0ea8af010786a080f823a879a4211f5712d260da) )
	ROM_LOAD( "548-2.ic2", 0x0800, 0x0800, CRC(ac297d99) SHA1(ccf31d3f9d02c3b68a0ee3be4984424df0e83ab0) )
	ROM_LOAD( "548-3.ic3", 0x1000, 0x0800, CRC(9c1fe511) SHA1(ff5b6e49a137c2ff9cb760c39bfd85ce4b52bb7d) )
	ROM_LOAD( "548-4.ic4", 0x1800, 0x0800, CRC(cb5e0de3) SHA1(0beaa8927faaf61f6c3fc0ea1d3d5670f901aae3) )
	ROM_LOAD( "548-5.ic5", 0x2000, 0x0800, CRC(0bd4559c) SHA1(e736a3124819ffb43e96a8114cd188f18d538053) )
	ROM_LOAD( "548-6.ic6", 0x2800, 0x0800, CRC(6d663034) SHA1(57588be4e360658dbb313946d7a608e36c1fdd68) )
	ROM_LOAD( "548-7.ic7", 0x3000, 0x0800, CRC(0bab06c0) SHA1(d52f1519c798e91f25648e996b1db174d90ce0f5) )
	ROM_LOAD( "548-8.ic8", 0x3800, 0x0800, CRC(f381b594) SHA1(2de7a8941ba48d463974c73d62e994d3cbe2868d) )
ROM_END

ROM_START( z80netf )
	ROM_REGION(0x5000, "maincpu", 0)
	/* ep548 banked at 0x0000 - 0x3FFF */
	ROM_LOAD(  "548-1.ic1", 0x0000, 0x0800, CRC(868cad39) SHA1(0ea8af010786a080f823a879a4211f5712d260da) )
	ROM_LOAD(  "548-2.ic2", 0x0800, 0x0800, CRC(ac297d99) SHA1(ccf31d3f9d02c3b68a0ee3be4984424df0e83ab0) )
	ROM_LOAD(  "548-3.ic3", 0x1000, 0x0800, CRC(9c1fe511) SHA1(ff5b6e49a137c2ff9cb760c39bfd85ce4b52bb7d) )
	ROM_LOAD(  "548-4.ic4", 0x1800, 0x0800, CRC(cb5e0de3) SHA1(0beaa8927faaf61f6c3fc0ea1d3d5670f901aae3) )
	ROM_LOAD(  "548-5.ic5", 0x2000, 0x0800, CRC(0bd4559c) SHA1(e736a3124819ffb43e96a8114cd188f18d538053) )
	ROM_LOAD(  "548-6.ic6", 0x2800, 0x0800, CRC(6d663034) SHA1(57588be4e360658dbb313946d7a608e36c1fdd68) )
	ROM_LOAD(  "548-7.ic7", 0x3000, 0x0800, CRC(0bab06c0) SHA1(d52f1519c798e91f25648e996b1db174d90ce0f5) )
	ROM_LOAD(  "548-8.ic8", 0x3800, 0x0800, CRC(f381b594) SHA1(2de7a8941ba48d463974c73d62e994d3cbe2868d) )

	/* ep382 - 8000 */
	ROM_LOAD(  "ep382.ic5", 0x4000, 0x0400, CRC(55818366) SHA1(adcac04b83c09265517b7bafbc2f5f665d751bec) )
	/* ep390 - F000 */
	ROM_LOAD(  "ep390.ic6", 0x4400, 0x0400, CRC(e4dd7de9) SHA1(523caa97112a9e67cc078c1a70ceee94ec232093) )
	/* ep1390 - F000 */
	ROM_LOAD( "ep1390.ic6", 0x4800, 0x0400, CRC(dc2cbc1d) SHA1(e23418b8f8261a17892f3a73ec09c72bb02e1d0b) )
	/* ep2390 - F000 */
	ROM_LOAD( "ep2390.ic6", 0x4C00, 0x0400, CRC(28d28eee) SHA1(b80f75c1ac4905ae369ecbc9b9ce120cc85502ed) )
ROM_END

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY               FULLNAME                      FLAGS
COMP( 1980, z80ne,   0,      0,      z80ne,   z80ne,   z80ne_state,   init_z80ne, "Nuova Elettronica",  "Z80NE",                      MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1980, z80net,  z80ne,  0,      z80net,  z80net,  z80net_state,  init_z80ne, "Nuova Elettronica",  "Z80NE + LX.388",             MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1980, z80netb, z80ne,  0,      z80netb, z80net,  z80netb_state, init_z80ne, "Nuova Elettronica",  "Z80NE + LX.388 + Basic 16k", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1980, z80netf, z80ne,  0,      z80netf, z80netf, z80netf_state, empty_init, "Nuova Elettronica",  "Z80NE + LX.388 + LX.390",    MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )

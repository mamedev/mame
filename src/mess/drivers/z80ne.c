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


******************************************************************************/

/* Core includes */
#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/z80ne.h"
#include "imagedev/flopdrv.h"
#include "formats/dmk_dsk.h"
#include "machine/ram.h"

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
static ADDRESS_MAP_START( z80ne_mem, AS_PROGRAM, 8, z80ne_state )
	AM_RANGE( 0x0000, 0x03ff ) AM_RAMBANK("bank1")
	AM_RANGE( 0x0400, 0x7fff ) AM_RAM
	AM_RANGE( 0x8000, 0x83ff ) AM_ROMBANK("bank2")
	AM_RANGE( 0x8400, 0xffff ) AM_READNOP AM_WRITENOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( z80net_mem, AS_PROGRAM, 8, z80ne_state )
	AM_RANGE( 0x0000, 0x03ff ) AM_RAMBANK("bank1")
	AM_RANGE( 0x0400, 0x7fff ) AM_RAM
	AM_RANGE( 0x8000, 0x83ff ) AM_ROMBANK("bank2")
	AM_RANGE( 0x8400, 0xebff ) AM_RAM
	AM_RANGE( 0xec00, 0xedff ) AM_RAM AM_SHARE("videoram") /* (6847) */
	AM_RANGE( 0xee00, 0xffff ) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( z80netb_mem, AS_PROGRAM, 8, z80ne_state )
	AM_RANGE( 0x0000, 0x3fff ) AM_ROM
	AM_RANGE( 0x4000, 0xebff ) AM_RAM
	AM_RANGE( 0xec00, 0xedff ) AM_RAM AM_SHARE("videoram") /* (6847) */
	AM_RANGE( 0xee00, 0xffff ) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( z80ne_io, AS_IO, 8, z80ne_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xee, 0xee) AM_READWRITE(lx385_data_r, lx385_data_w )
	AM_RANGE(0xef, 0xef) AM_READWRITE(lx385_ctrl_r, lx385_ctrl_w )
	AM_RANGE(0xf0, 0xff) AM_READWRITE(lx383_r, lx383_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( z80net_io, AS_IO, 8, z80ne_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xea, 0xea) AM_READ(lx388_data_r )
	AM_RANGE(0xeb, 0xeb) AM_READ(lx388_read_field_sync )
	AM_RANGE(0xee, 0xee) AM_READWRITE(lx385_data_r, lx385_data_w )
	AM_RANGE(0xef, 0xef) AM_READWRITE(lx385_ctrl_r, lx385_ctrl_w )
	AM_RANGE(0xf0, 0xff) AM_READWRITE(lx383_r, lx383_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( z80netf_mem, AS_PROGRAM, 8, z80ne_state )
	AM_RANGE( 0x0000, 0x03ff ) AM_RAMBANK("bank1")
	AM_RANGE( 0x0400, 0x3fff ) AM_RAMBANK("bank2")
	AM_RANGE( 0x4000, 0x7fff ) AM_RAM
	AM_RANGE( 0x8000, 0x83ff ) AM_RAMBANK("bank3")
	AM_RANGE( 0x8400, 0xdfff ) AM_RAM
	AM_RANGE( 0xe000, 0xebff ) AM_READNOP AM_WRITENOP
	AM_RANGE( 0xec00, 0xedff ) AM_RAM AM_SHARE("videoram") /* (6847) */
	AM_RANGE( 0xee00, 0xefff ) AM_READNOP AM_WRITENOP
	AM_RANGE( 0xf000, 0xf3ff ) AM_RAMBANK("bank4")
	AM_RANGE( 0xf400, 0xffff ) AM_READNOP AM_WRITENOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( z80netf_io, AS_IO, 8, z80ne_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xd0, 0xd7) AM_READWRITE(lx390_fdc_r, lx390_fdc_w)
	AM_RANGE(0xea, 0xea) AM_READ(lx388_data_r )
	AM_RANGE(0xeb, 0xeb) AM_READ(lx388_read_field_sync )
	AM_RANGE(0xee, 0xee) AM_READWRITE(lx385_data_r, lx385_data_w )
	AM_RANGE(0xef, 0xef) AM_READWRITE(lx385_ctrl_r, lx385_ctrl_w )
	AM_RANGE(0xf0, 0xff) AM_READWRITE(lx383_r, lx383_w )
ADDRESS_MAP_END



/******************************************************************************
 Input Ports
******************************************************************************/


static INPUT_PORTS_START( z80ne )
/* LX.384 Hex Keyboard and Display */
/*
 * In natural mode the CTRL key is mapped on shift
 */
PORT_START("ROW0")          /* IN0 keys row 0 */
PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 0") PORT_CODE(KEYCODE_0)          //PORT_CHAR('0') PORT_CHAR('=')
PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 1") PORT_CODE(KEYCODE_1)          //PORT_CHAR('1') PORT_CHAR('!')
PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 2") PORT_CODE(KEYCODE_2)          //PORT_CHAR('2') PORT_CHAR('"')
PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 3") PORT_CODE(KEYCODE_3)          //PORT_CHAR('3') PORT_CHAR(0x00a3) // ??
PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 4") PORT_CODE(KEYCODE_4)          //PORT_CHAR('4') PORT_CHAR('$')
PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 5") PORT_CODE(KEYCODE_5)          //PORT_CHAR('5') PORT_CHAR('%')
PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 6") PORT_CODE(KEYCODE_6)          //PORT_CHAR('6') PORT_CHAR('&')
PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 7") PORT_CODE(KEYCODE_7)          //PORT_CHAR('7') PORT_CHAR('/')

PORT_START("ROW1")          /* IN1 keys row 1 */
PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 8") PORT_CODE(KEYCODE_8)          //PORT_CHAR('8') PORT_CHAR('(')
PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 9") PORT_CODE(KEYCODE_9)          //PORT_CHAR('9') PORT_CHAR(')')
PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 A") PORT_CODE(KEYCODE_A)          //PORT_CHAR('a') PORT_CHAR('A')
PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 B") PORT_CODE(KEYCODE_B)          //PORT_CHAR('b') PORT_CHAR('B')
PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 C") PORT_CODE(KEYCODE_C)          //PORT_CHAR('c') PORT_CHAR('C')
PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 D") PORT_CODE(KEYCODE_D)          //PORT_CHAR('d') PORT_CHAR('D')
PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 E") PORT_CODE(KEYCODE_E)          //PORT_CHAR('e') PORT_CHAR('E')
PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 F") PORT_CODE(KEYCODE_F)          //PORT_CHAR('f') PORT_CHAR('F')

PORT_START("CTRL")          /* CONTROL key */
PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) //PORT_CHAR(UCHAR_SHIFT_1)
PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )

PORT_START("RST")           /* RESET key */
PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LX.384 Reset")  PORT_CODE(KEYCODE_F3) PORT_CHANGED_MEMBER(DEVICE_SELF, z80ne_state, z80ne_reset, NULL)

/* Settings */
PORT_START("LX.385")
PORT_CONFNAME(0x07, 0x01    , "LX.385 Cassette: P1,P3 Data Rate")
PORT_CONFSETTING( 0x01, "A-B: 300 bps")
PORT_CONFSETTING( 0x02, "A-C: 600 bps")
PORT_CONFSETTING( 0x04, "A-D: 1200 bps")
PORT_CONFNAME( 0x08, 0x00, "LX.385: P4 Parity Check")
PORT_CONFSETTING( 0x00, "Parity Check Enabled")
PORT_CONFSETTING( 0x08, "Parity Check Disabled")

INPUT_PORTS_END


static INPUT_PORTS_START( z80net )

PORT_INCLUDE( z80ne )

/* LX.388 Keyboard BREAK key */
PORT_START("LX388_BRK")
PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Break") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT)) PORT_CHANGED_MEMBER(DEVICE_SELF, z80ne_state, z80ne_nmi, NULL)

/* LX.388 Keyboard (Encoded by KR2376) */

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
PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Alpha Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))

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
static const UINT32 lx388palette[] =
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
	rgb_t(0xff, 0xc4, 0x18)      /* ALPHANUMERIC BRIGHT ORANGE */
};
#endif

FLOPPY_FORMATS_MEMBER( z80ne_state::floppy_formats )
	FLOPPY_DMK_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( z80ne_floppies )
	SLOT_INTERFACE("sssd", FLOPPY_525_SSSD)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( z80ne, z80ne_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("z80ne", Z80, Z80NE_CPU_SPEED_HZ)
	MCFG_CPU_PROGRAM_MAP(z80ne_mem)
	MCFG_CPU_IO_MAP(z80ne_io)

	MCFG_MACHINE_START_OVERRIDE(z80ne_state,z80ne)
	MCFG_MACHINE_RESET_OVERRIDE(z80ne_state,z80ne)

	MCFG_DEVICE_ADD( "ay_3_1015", AY31015, 0 )
	MCFG_AY31015_TX_CLOCK(4800.0)
	MCFG_AY31015_RX_CLOCK(4800.0)

	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED)

	MCFG_CASSETTE_ADD( "cassette2" )
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED)

	MCFG_DEFAULT_LAYOUT(layout_z80ne)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("32K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( z80net, z80ne )

	MCFG_CPU_MODIFY("z80ne")
	MCFG_CPU_PROGRAM_MAP(z80net_mem)
	MCFG_CPU_IO_MAP(z80net_io)

	MCFG_MACHINE_START_OVERRIDE(z80ne_state, z80net )
	MCFG_MACHINE_RESET_OVERRIDE(z80ne_state, z80net )

	MCFG_DEVICE_ADD("lx388_kr2376", KR2376, 50000)

	/* video hardware */
	MCFG_SCREEN_MC6847_PAL_ADD("lx388", "mc6847")

	MCFG_DEVICE_ADD("mc6847", MC6847_PAL, XTAL_4_433619MHz)
	MCFG_MC6847_INPUT_CALLBACK(READ8(z80ne_state, lx388_mc6847_videoram_r))
	// AG = GND, GM2 = GND, GM1 = GND, GM0 = GND, CSS = GND
	// other lines not connected

	MCFG_DEFAULT_LAYOUT(layout_z80net)

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("32K")
	MCFG_RAM_EXTRA_OPTIONS("1K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( z80netb, z80ne_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("z80ne", Z80, Z80NE_CPU_SPEED_HZ)
	MCFG_CPU_PROGRAM_MAP(z80netb_mem)
	MCFG_CPU_IO_MAP(z80net_io)

	MCFG_MACHINE_START_OVERRIDE(z80ne_state,z80netb)
	MCFG_MACHINE_RESET_OVERRIDE(z80ne_state,z80netb)

	MCFG_DEVICE_ADD( "ay_3_1015", AY31015, 0 )
	MCFG_AY31015_TX_CLOCK(4800.0)
	MCFG_AY31015_RX_CLOCK(4800.0)

	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED)

	MCFG_CASSETTE_ADD( "cassette2" )
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED)

	MCFG_DEVICE_ADD("lx388_kr2376", KR2376, 50000)

	/* video hardware */
	MCFG_SCREEN_MC6847_PAL_ADD("lx388", "mc6847")

	MCFG_DEVICE_ADD("mc6847", MC6847_PAL, XTAL_4_433619MHz)
	MCFG_MC6847_INPUT_CALLBACK(READ8(z80ne_state, lx388_mc6847_videoram_r))
	// AG = GND, GM2 = GND, GM1 = GND, GM0 = GND, CSS = GND
	// other lines not connected

	MCFG_DEFAULT_LAYOUT(layout_z80netb)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("32K")
	MCFG_RAM_EXTRA_OPTIONS("1K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( z80netf, z80ne_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("z80ne", Z80, Z80NE_CPU_SPEED_HZ)
	MCFG_CPU_PROGRAM_MAP(z80netf_mem)
	MCFG_CPU_IO_MAP(z80netf_io)

	MCFG_MACHINE_START_OVERRIDE(z80ne_state,z80netf)
	MCFG_MACHINE_RESET_OVERRIDE(z80ne_state,z80netf)

	MCFG_DEVICE_ADD( "ay_3_1015", AY31015, 0 )
	MCFG_AY31015_TX_CLOCK(4800.0)
	MCFG_AY31015_RX_CLOCK(4800.0)

	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED)

	MCFG_CASSETTE_ADD( "cassette2" )
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED)

	MCFG_DEVICE_ADD("lx388_kr2376", KR2376, 50000)

	/* video hardware */
	MCFG_SCREEN_MC6847_PAL_ADD("lx388", "mc6847")

	MCFG_DEVICE_ADD("mc6847", MC6847_PAL, XTAL_4_433619MHz)
	MCFG_MC6847_INPUT_CALLBACK(READ8(z80ne_state, lx388_mc6847_videoram_r))
	// AG = GND, GM2 = GND, GM1 = GND, GM0 = GND, CSS = GND
	// other lines not connected

	MCFG_FD1771_ADD("wd1771", XTAL_2MHz / 2)
	MCFG_FLOPPY_DRIVE_ADD("wd1771:0", z80ne_floppies, "sssd", z80ne_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("wd1771:1", z80ne_floppies, "sssd", z80ne_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("wd1771:2", z80ne_floppies, NULL,   z80ne_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("wd1771:3", z80ne_floppies, NULL,   z80ne_state::floppy_formats)

	MCFG_DEFAULT_LAYOUT(layout_z80netf)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("56K")
MACHINE_CONFIG_END

/******************************************************************************
 ROM Definitions
******************************************************************************/


ROM_START( z80ne )
	ROM_REGION(0x20000, "z80ne", 0)
	ROM_LOAD( "ep382.ic5", 0x14000, 0x0400, CRC(61bc5f39) SHA1(a93779a598736302a2fdd94be2fb0bbddea7a72c) )
ROM_END

ROM_START( z80net )
	ROM_REGION(0x20000, "z80ne", 0)
	ROM_LOAD( "ep382.ic5", 0x14000, 0x0400, CRC(61bc5f39) SHA1(a93779a598736302a2fdd94be2fb0bbddea7a72c) )
ROM_END

ROM_START( z80netb )
/*
 * 16k Basic
 */
	ROM_REGION(0x10000, "z80ne", 0)
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
	ROM_REGION(0x20000, "z80ne", 0) /* 64k for code  64k for banked code */
	/* ep548 banked at 0x0000 - 0x3FFF */
	ROM_LOAD(  "548-1.ic1", 0x10000, 0x0800, CRC(868cad39) SHA1(0ea8af010786a080f823a879a4211f5712d260da) )
	ROM_LOAD(  "548-2.ic2", 0x10800, 0x0800, CRC(ac297d99) SHA1(ccf31d3f9d02c3b68a0ee3be4984424df0e83ab0) )
	ROM_LOAD(  "548-3.ic3", 0x11000, 0x0800, CRC(9c1fe511) SHA1(ff5b6e49a137c2ff9cb760c39bfd85ce4b52bb7d) )
	ROM_LOAD(  "548-4.ic4", 0x11800, 0x0800, CRC(cb5e0de3) SHA1(0beaa8927faaf61f6c3fc0ea1d3d5670f901aae3) )
	ROM_LOAD(  "548-5.ic5", 0x12000, 0x0800, CRC(0bd4559c) SHA1(e736a3124819ffb43e96a8114cd188f18d538053) )
	ROM_LOAD(  "548-6.ic6", 0x12800, 0x0800, CRC(6d663034) SHA1(57588be4e360658dbb313946d7a608e36c1fdd68) )
	ROM_LOAD(  "548-7.ic7", 0x13000, 0x0800, CRC(0bab06c0) SHA1(d52f1519c798e91f25648e996b1db174d90ce0f5) )
	ROM_LOAD(  "548-8.ic8", 0x13800, 0x0800, CRC(f381b594) SHA1(2de7a8941ba48d463974c73d62e994d3cbe2868d) )

	/* ep382 - banked at 0x0000 - 0x03FF */
	ROM_LOAD(  "ep382.ic5", 0x14000, 0x0400, CRC(61bc5f39) SHA1(a93779a598736302a2fdd94be2fb0bbddea7a72c) )

	/* ep390 - banked at 0x0000 - 0x03FF */
	ROM_LOAD(  "ep390.ic6", 0x14400, 0x0400, CRC(e4dd7de9) SHA1(523caa97112a9e67cc078c1a70ceee94ec232093) )
	/* ep1390 - banked at 0x0000 - 0x03FF */
	ROM_LOAD( "ep1390.ic6", 0x14800, 0x0400, CRC(dc2cbc1d) SHA1(e23418b8f8261a17892f3a73ec09c72bb02e1d0b) )
	/* ep2390 - banked at 0x0000 - 0x03FF */
	ROM_LOAD( "ep2390.ic6", 0x14C00, 0x0400, CRC(28d28eee) SHA1(b80f75c1ac4905ae369ecbc9b9ce120cc85502ed) )
ROM_END

/*    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT     INIT     COMPANY               FULLNAME                      FLAGS */
COMP( 1980, z80ne,    0,        0,      z80ne,    z80ne, z80ne_state,    z80ne,   "Nuova Elettronica",  "Z80NE",                      MACHINE_NO_SOUND_HW)
COMP( 1980, z80net,   z80ne,    0,      z80net,   z80net, z80ne_state,   z80net,  "Nuova Elettronica",  "Z80NE + LX.388",             MACHINE_NO_SOUND_HW)
COMP( 1980, z80netb,  z80ne,    0,      z80netb,  z80net, z80ne_state,   z80netb, "Nuova Elettronica",  "Z80NE + LX.388 + Basic 16k", MACHINE_NO_SOUND_HW)
COMP( 1980, z80netf,  z80ne,    0,      z80netf,  z80netf, z80ne_state,  z80netf, "Nuova Elettronica",  "Z80NE + LX.388 + LX.390",    MACHINE_NO_SOUND_HW)

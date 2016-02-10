// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

Visual 1050

PCB Layout
----------

REV B-1

|---------------------------------------------------------------------------------------------------|
|                                                               |----------|                        |
|           9216                                                |   ROM0   |        LS74            |
|                                                               |----------|                        |
--                      LS00    7406            LS255   LS393                       4164    4164    |
||      |-----------|                                           |----------|                        |
||      |   WD1793  |   LS14    7407    -       LS393   LS74    |  8251A   |        4164    4164    |
||      |-----------|                   |                       |----------|                        |
|C                                      |                                           4164    4164    |
|N      7406    LS00    LS74    LS14    |C      LS20    LS08    LS75                                |
|1                                      |N                                          4164    4164    |
||                                      |5                                                          |
||      7406    LS195           LS244   |       LS04    LS32    LS08                4164    4164    |
||                                      -                                                           |
||      |------------|  |--------|                                                  4164    4164    |
-- BAT  |   8255A    |  |  8214  |              LS139   LS32    LS138   LS17                        |
--      |------------|  |--------|      -                                           4164    4164    |
||                                      |C                                                          |
||      |--------|      LS00    LS00    |N      LS00    LS32    LS138               4164    4164    |
|C      | 8251A  |                      |6                                                          |
|N      |--------|      |------------|  -                       |------------|                      |
|2                      |   8255A    |                          |    Z80A    |                      |
||      1488 1489  RTC  |------------|          LS00    LS32    |------------|      LS257   LS257   |
||                                                                                                  |
--                                                              |------------|      |------------|  |
|                                                       LS04    |   8255A    |      |   8255A    |  |
|                                                               |------------|      |------------|  |
|                                                                                                   |
|                                               7404                                LS257   LS257   |
|                                                   16MHz                                           |
|       LS04    LS74    LS257                                                       9016    9016    |
|                                                                                                   |
--      LS74    LS04    LS74    LS163                                               9016    9016    |
||                                                                                                  |
||                                                                                  9016    9016    |
||      LS02    LS163   LS74    7404                            7404                                |
||                                                                                  9016    9016    |
||                                                                                                  |
|C      LS244   LS10    LS257                   LS362           |----------|        9016    9016    |
|N                                                              |   ROM1   |                        |
|3      LS245   7404    LS273                   LS32            |----------|        9016    9016    |
||                                                                                                  |
||      7407            LS174                   LS32                                9016    9016    |
||              15.36MHz                                                                            |
||                                                              |------------|      9016    9016    |
--                      LS09    LS04            LS12            |    6502    |                      |
|                                                               |------------|      9016    9016    |
--                                                                                                  |
|C              LS175   LS86    LS164           LS164                               9016    9016    |
|N                                                              |------------|                      |
|4                                                              |    6845    |      LS253   LS253   |
--      7426    LS02    LS74    LS00            LS164           |------------|                      |
|                                                                                   LS253   LS253   |
|                       REV B-1 W/O 1059            S/N 492                                         |
|---------------------------------------------------------------------------------------------------|

Notes:
    All IC's shown.

    ROM0    - "IC 244-032 V1.0"
    ROM1    - "IC 244-033 V1.0"
    Z80A    - Zilog Z8400APS Z80A CPU
    6502    - Synertek SY6502A CPU
    4164    - NEC D4164-2 64Kx1 Dynamic RAM
    9016    - AMD AM9016EPC 16Kx1 Dynamic RAM
    8214    - NEC uPB8214C Priority Interrupt Control Unit
    8255A   - NEC D8255AC-5 Programmable Peripheral Interface
    8251A   - NEC D8251AC Programmable Communication Interface
    WD1793  - Mitsubishi MB8877 Floppy Disc Controller
    9216    - SMC FDC9216 Floppy Disk Data Separator
    1488    - Motorola MC1488 Quad Line EIA-232D Driver
    1489    - Motorola MC1489 Quad Line Receivers
    6845    - Hitachi HD46505SP CRT Controller
    RTC     - OKI MSM58321RS Real Time Clock
    BAT     - 3.4V battery
    CN1     - parallel connector
    CN2     - serial connector
    CN3     - winchester connector
    CN4     - monitor connector
    CN5     - floppy data connector
    CN6     - floppy power connector

*/

/*

    Using the hard disk
    -------------------

    Use the chdman utility to create a Tandon TM501 (5MB) or CMI CM-5412 (10MB) hard disk image:

    $ chdman createhd -chs 306,2,32 -ss 256 -o tm501.chd
    $ chdman createhd -chs 306,4,32 -ss 256 -o cm5412.chd

    Start the Visual 1050 emulator with the floppy and hard disk images mounted:

    $ mess v1050 -flop1 cpm3:flop2 -hard tm501.chd
    $ mess v1050 -flop1 cpm3:flop2 -hard cm5412.chd

    Start the Winchester Format Program from the CP/M prompt:

    A>fmtwinch

    Enter Y to continue.
    Enter A for 5MB, or B for 10MB hard disk.
    Enter C to start formatting.

    Once the formatting is complete, the CP/M system files need to be copied over to the hard disk:

    A>copysys

    Enter source drive name "a" and press RETURN.
    Enter target drive name "c" and press RETURN.
    Enter "y" at the prompt for CPM3.SYS.
    Enter "y" at the prompt for CCP.COM.
    Press RETURN to return to CP/M.

    You can now boot from the hard disk with:

    $ mess v1050 -hard tm501.chd
    $ mess v1050 -hard cm5412.chd

    Or skip all of the above and use the preformatted images in the software list:

    $ mess v1050 -hard cpm3hd5
    $ mess v1050 -hard cpm3hd10

*/

/*

    TODO:

    - floppy 1 is broken
    - write to banked RAM at 0x0000-0x1fff when ROM is active
    - real keyboard w/i8049
    - keyboard beeper (NE555 wired in strange mix of astable/monostable modes)

*/

#include "includes/v1050.h"
#include "bus/rs232/rs232.h"
#include "softlist.h"

void v1050_state::set_interrupt(UINT8 mask, int state)
{
	if (state)
	{
		m_int_state |= mask;
	}
	else
	{
		m_int_state &= ~mask;
	}

	m_pic->r_w(~(m_int_state & m_int_mask));
}

void v1050_state::bankswitch()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	int bank = (m_bank >> 1) & 0x03;

	if (BIT(m_bank, 0))
	{
		program.install_readwrite_bank(0x0000, 0x1fff, "bank1");
		membank("bank1")->set_entry(bank);
	}
	else
	{
		program.install_read_bank(0x0000, 0x1fff, "bank1");
		program.unmap_write(0x0000, 0x1fff);
		membank("bank1")->set_entry(3);
	}

	membank("bank2")->set_entry(bank);

	if (bank == 2)
	{
		program.unmap_readwrite(0x4000, 0xbfff);
	}
	else
	{
		program.install_readwrite_bank(0x4000, 0x7fff, "bank3");
		program.install_readwrite_bank(0x8000, 0xbfff, "bank4");
		membank("bank3")->set_entry(bank);
		membank("bank4")->set_entry(bank);
	}

	membank("bank5")->set_entry(bank);
}

// Keyboard HACK

static const UINT8 V1050_KEYCODES[4][12][8] =
{
	{   // unshifted
		{ 0xc0, 0xd4, 0xd8, 0xdc, 0xe0, 0xe4, 0xe8, 0xec },
		{ 0xf0, 0xfc, 0x90, 0xf4, 0xf8, 0x94, 0xc4, 0xc8 },
		{ 0xcc, 0xd0, 0x1b, 0x31, 0x32, 0x33, 0x34, 0x35 },
		{ 0x36, 0x37, 0x38, 0x39, 0x30, 0x2d, 0x3d, 0x60 },
		{ 0x08, 0x88, 0x8c, 0x71, 0x77, 0x65, 0x72, 0x74 },
		{ 0x79, 0x75, 0x69, 0x6f, 0x70, 0x5b, 0x5d, 0x0d },
		{ 0x7f, 0x00, 0x80, 0x61, 0x73, 0x64, 0x66, 0x67 },
		{ 0x68, 0x6a, 0x6b, 0x6c, 0x3b, 0x27, 0x5c, 0x84 },
		{ 0x00, 0x7a, 0x78, 0x63, 0x76, 0x62, 0x6e, 0x6d },
		{ 0x2c, 0x2e, 0x2f, 0x00, 0x0a, 0x20, 0x81, 0x82 },
		{ 0xb7, 0xb8, 0xb9, 0xad, 0xb4, 0xb5, 0xb6, 0xac },
		{ 0xb1, 0xb2, 0xb3, 0x83, 0xb0, 0xae, 0x00, 0x00 },
	},

	{   // shifted
		{ 0xc1, 0xd5, 0xd9, 0xdd, 0xe1, 0xe5, 0xe9, 0xed },
		{ 0xf1, 0xfd, 0x91, 0xf5, 0xf9, 0x95, 0xc5, 0xc9 },
		{ 0xcd, 0xd1, 0x1b, 0x21, 0x40, 0x23, 0x24, 0x25 },
		{ 0x5e, 0x26, 0x2a, 0x28, 0x29, 0x5f, 0x2b, 0x7e },
		{ 0x08, 0x89, 0x8d, 0x51, 0x57, 0x45, 0x52, 0x54 },
		{ 0x59, 0x55, 0x49, 0x4f, 0x50, 0x7b, 0x7d, 0x0d },
		{ 0x7f, 0x00, 0x80, 0x41, 0x53, 0x44, 0x46, 0x47 },
		{ 0x48, 0x4a, 0x4b, 0x4c, 0x3a, 0x22, 0x7c, 0x85 },
		{ 0x00, 0x5a, 0x58, 0x43, 0x56, 0x42, 0x4e, 0x4d },
		{ 0x3c, 0x3e, 0x3f, 0x0a, 0x20, 0x81, 0x82, 0xb7 },
		{ 0xb8, 0xb9, 0xad, 0xb4, 0xb5, 0xb6, 0xac, 0xb1 },
		{ 0xb2, 0xb3, 0x83, 0xb0, 0xa0, 0x00, 0x00, 0x00 },
	},

	{   // control
		{ 0xc2, 0xd6, 0xda, 0xde, 0xe2, 0xe6, 0xea, 0xee },
		{ 0xf2, 0xfe, 0x92, 0xf6, 0xfa, 0x96, 0xc6, 0xca },
		{ 0xce, 0xd2, 0x1b, 0x31, 0x32, 0x33, 0x34, 0x35 },
		{ 0x36, 0x37, 0x38, 0x39, 0x30, 0x2d, 0x3d, 0x00 },
		{ 0x08, 0x8a, 0x8e, 0x11, 0x17, 0x05, 0x12, 0x14 },
		{ 0x19, 0x15, 0x09, 0x0f, 0x10, 0x1b, 0x1d, 0x0d },
		{ 0x7f, 0x00, 0x80, 0x01, 0x13, 0x04, 0x06, 0x07 },
		{ 0x08, 0x0a, 0x0b, 0x0c, 0x3b, 0x27, 0x1c, 0x86 },
		{ 0x00, 0x1a, 0x18, 0x03, 0x16, 0x02, 0x0e, 0x0d },
		{ 0x2c, 0x2e, 0x2f, 0x0a, 0x20, 0x81, 0x82, 0xb7 },
		{ 0xb8, 0xb9, 0xad, 0xb4, 0xb5, 0xb6, 0xac, 0xb1 },
		{ 0xb2, 0xb3, 0x83, 0xb0, 0xae, 0x00, 0x00, 0x00 },
	},

	{   // shift & control
		{ 0xc3, 0xd7, 0xdb, 0xdf, 0xe3, 0xe7, 0xeb, 0xef },
		{ 0xf3, 0xff, 0x93, 0xf7, 0xfb, 0x97, 0xc7, 0xcb },
		{ 0xcf, 0xd3, 0x1b, 0x21, 0x00, 0x23, 0x24, 0x25 },
		{ 0x1e, 0x26, 0x2a, 0x28, 0x29, 0x1f, 0x2b, 0x1e },
		{ 0x08, 0x8b, 0x8f, 0x11, 0x17, 0x05, 0x12, 0x14 },
		{ 0x19, 0x15, 0x09, 0x0f, 0x10, 0x1b, 0x1d, 0x0d },
		{ 0x7f, 0x00, 0x80, 0x01, 0x13, 0x04, 0x06, 0x07 },
		{ 0x08, 0x0a, 0x0b, 0x0c, 0x3a, 0x22, 0x1c, 0x87 },
		{ 0x00, 0x1a, 0x18, 0x03, 0x16, 0x02, 0x0e, 0x0d },
		{ 0x3c, 0x3e, 0x3f, 0x0a, 0x20, 0x81, 0x82, 0xb7 },
		{ 0xb8, 0xb9, 0xad, 0xb4, 0xb5, 0xb6, 0xac, 0xb1 },
		{ 0xb2, 0xb3, 0x83, 0xb0, 0xae, 0x00, 0x00, 0x00 },
	}
};

void v1050_state::scan_keyboard()
{
	static const char *const keynames[] = { "ROW0", "ROW1", "ROW2", "ROW3", "ROW4", "ROW5", "ROW6", "ROW7", "ROW8", "ROW9", "ROW10", "ROW11" };
	int table = 0, row, col;
	int keydata = 0xff;

	UINT8 line_mod = ioport("ROW12")->read();

	if((line_mod & 0x07) && (line_mod & 0x18))
	{
		table = 3;  // shift & control
	}
	else if (line_mod & 0x07)
	{
		table = 1; // shifted
	}
	else if (line_mod & 0x18)
	{
		table = 2; // ctrl
	}

	// scan keyboard
	for (row = 0; row < 12; row++)
	{
		UINT8 data = ioport(keynames[row])->read();

		for (col = 0; col < 8; col++)
		{
			if (!BIT(data, col))
			{
				// latch key data
				keydata = V1050_KEYCODES[table][row][col];

				if (m_keydata != keydata)
				{
					m_keydata = keydata;
					m_keyavail = 1;

					set_interrupt(INT_KEYBOARD, 1);
					return;
				}
			}
		}
	}

	m_keydata = keydata;
}

TIMER_DEVICE_CALLBACK_MEMBER(v1050_state::v1050_keyboard_tick)
{
	scan_keyboard();
}

READ8_MEMBER( v1050_state::kb_data_r )
{
	m_keyavail = 0;

	set_interrupt(INT_KEYBOARD, 0);

	return m_keydata;
}

READ8_MEMBER( v1050_state::kb_status_r )
{
	UINT8 val = m_uart_kb->status_r(space, 0);

	return val | (m_keyavail ? 0x02 : 0x00);
}

// Z80 Read/Write Handlers

WRITE8_MEMBER( v1050_state::v1050_i8214_w )
{
	m_pic->b_w((data >> 1) & 0x07);
	m_pic->sgs_w(BIT(data, 4));
}

READ8_MEMBER( v1050_state::vint_clr_r )
{
	set_interrupt(INT_VSYNC, 0);

	return 0xff;
}

WRITE8_MEMBER( v1050_state::vint_clr_w )
{
	set_interrupt(INT_VSYNC, 0);
}

READ8_MEMBER( v1050_state::dint_clr_r )
{
	set_interrupt(INT_DISPLAY, 0);

	return 0xff;
}

WRITE8_MEMBER( v1050_state::dint_clr_w )
{
	set_interrupt(INT_DISPLAY, 0);
}

WRITE8_MEMBER( v1050_state::bank_w )
{
	m_bank = data;

	bankswitch();
}

// SY6502A Read/Write Handlers

WRITE8_MEMBER( v1050_state::dint_w )
{
	set_interrupt(INT_DISPLAY, 1);
}

WRITE8_MEMBER( v1050_state::dvint_clr_w )
{
	m_subcpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}

WRITE8_MEMBER( v1050_state::sasi_data_w )
{
	m_sasi_data = data;

	if (m_sasi_data_enable)
	{
		m_sasi_data_out->write(m_sasi_data);
	}
}

WRITE_LINE_MEMBER( v1050_state::write_sasi_io )
{
	m_sasi_ctrl_in->write_bit4(state);

	m_sasi_data_enable = state;

	if (m_sasi_data_enable)
	{
		m_sasi_data_out->write(m_sasi_data);
	}
	else
	{
		m_sasi_data_out->write(0);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(v1050_state::sasi_ack_tick)
{
	m_sasibus->write_ack(0);
}

TIMER_DEVICE_CALLBACK_MEMBER(v1050_state::sasi_rst_tick)
{
	m_sasibus->write_rst(0);
}

WRITE8_MEMBER( v1050_state::sasi_ctrl_w )
{
	/*

	    bit     description

	    0       SEL
	    1       ACK
	    2
	    3
	    4
	    5
	    6
	    7       RST

	*/

	m_sasibus->write_sel(BIT(data, 0));

	if (BIT(data, 1))
	{
		// send acknowledge pulse
		m_sasibus->write_ack(1);

		m_timer_ack->adjust(attotime::from_nsec(100));
	}

	if (BIT(data, 7))
	{
		// send reset pulse
		m_sasibus->write_rst(1);

		m_timer_rst->adjust(attotime::from_nsec(100));
	}
}

// Memory Maps

static ADDRESS_MAP_START( v1050_mem, AS_PROGRAM, 8, v1050_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_RAMBANK("bank1")
	AM_RANGE(0x2000, 0x3fff) AM_RAMBANK("bank2")
	AM_RANGE(0x4000, 0x7fff) AM_RAMBANK("bank3")
	AM_RANGE(0x8000, 0xbfff) AM_RAMBANK("bank4")
	AM_RANGE(0xc000, 0xffff) AM_RAMBANK("bank5")
ADDRESS_MAP_END

static ADDRESS_MAP_START( v1050_io, AS_IO, 8, v1050_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x84, 0x87) AM_DEVREADWRITE(I8255A_DISP_TAG, i8255_device, read, write)
//  AM_RANGE(0x88, 0x88) AM_DEVREADWRITE(I8251A_KB_TAG, i8251_device, data_r, data_w)
//  AM_RANGE(0x89, 0x89) AM_DEVREADWRITE(I8251A_KB_TAG, i8251_device, status_r, control_w)
	AM_RANGE(0x88, 0x88) AM_READ(kb_data_r) AM_DEVWRITE(I8251A_KB_TAG, i8251_device, data_w)
	AM_RANGE(0x89, 0x89) AM_READ(kb_status_r) AM_DEVWRITE(I8251A_KB_TAG, i8251_device, control_w)
	AM_RANGE(0x8c, 0x8c) AM_DEVREADWRITE(I8251A_SIO_TAG, i8251_device, data_r, data_w)
	AM_RANGE(0x8d, 0x8d) AM_DEVREADWRITE(I8251A_SIO_TAG, i8251_device, status_r, control_w)
	AM_RANGE(0x90, 0x93) AM_DEVREADWRITE(I8255A_MISC_TAG, i8255_device, read, write)
	AM_RANGE(0x94, 0x97) AM_DEVREADWRITE(MB8877_TAG, mb8877_t, read, write)
	AM_RANGE(0x9c, 0x9f) AM_DEVREADWRITE(I8255A_RTC_TAG, i8255_device, read, write)
	AM_RANGE(0xa0, 0xa0) AM_READWRITE(vint_clr_r, vint_clr_w)
	AM_RANGE(0xb0, 0xb0) AM_READWRITE(dint_clr_r, dint_clr_w)
	AM_RANGE(0xc0, 0xc0) AM_WRITE(v1050_i8214_w)
	AM_RANGE(0xd0, 0xd0) AM_WRITE(bank_w)
	AM_RANGE(0xe0, 0xe0) AM_WRITE(sasi_data_w) AM_DEVREAD("scsi_data_in", input_buffer_device, read)
	AM_RANGE(0xe1, 0xe1) AM_DEVREAD("scsi_ctrl_in", input_buffer_device, read) AM_WRITE(sasi_ctrl_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( v1050_crt_mem, AS_PROGRAM, 8, v1050_state )
	AM_RANGE(0x0000, 0x7fff) AM_READWRITE(videoram_r, videoram_w) AM_SHARE("video_ram")
	AM_RANGE(0x8000, 0x8000) AM_DEVWRITE(H46505_TAG, mc6845_device, address_w)
	AM_RANGE(0x8001, 0x8001) AM_DEVREADWRITE(H46505_TAG, mc6845_device, register_r, register_w)
	AM_RANGE(0x9000, 0x9003) AM_DEVREADWRITE(I8255A_M6502_TAG, i8255_device, read, write)
	AM_RANGE(0xa000, 0xa000) AM_READWRITE(attr_r, attr_w)
	AM_RANGE(0xb000, 0xb000) AM_WRITE(dint_w)
	AM_RANGE(0xc000, 0xc000) AM_WRITE(dvint_clr_w)
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END

// Input Ports

static INPUT_PORTS_START( v1050 )
	PORT_START("ROW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) // HELP
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F5)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F6)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F7)

	PORT_START("ROW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F8)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F9)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F10)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F11)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F12)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F13)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F14)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F15)

	PORT_START("ROW2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) // F16
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) // F17
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')

	PORT_START("ROW3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('`') PORT_CHAR('~')

	PORT_START("ROW4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BACKSPACE") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BREAK") PORT_CODE(KEYCODE_PAUSE)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')

	PORT_START("ROW5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')

	PORT_START("ROW6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')

	PORT_START("ROW7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SCRLOCK)

	PORT_START("ROW8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) // SHIFT
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')

	PORT_START("ROW9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) // SHIFT
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LINE FEED") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_NUMLOCK) // DOWN
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_NUMLOCK) // UP

	PORT_START("ROW10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad -") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START("ROW11")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad +") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("ROW12")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LEFT SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RIGHT SHIFT") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LEFT CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RIGHT CTRL") PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))
INPUT_PORTS_END

// 8214 Interface

WRITE_LINE_MEMBER(v1050_state::pic_int_w)
{
	if (state == ASSERT_LINE)
	{
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
	}
}

// Display 8255A Interface

WRITE8_MEMBER(v1050_state::disp_ppi_pc_w)
{
	m_ppi_6502->pc2_w(BIT(data, 6));
	m_ppi_6502->pc4_w(BIT(data, 7));
}

WRITE8_MEMBER(v1050_state::m6502_ppi_pc_w)
{
	m_ppi_disp->pc2_w(BIT(data, 7));
	m_ppi_disp->pc4_w(BIT(data, 6));
}

// Miscellanous 8255A Interface

WRITE8_MEMBER( v1050_state::misc_ppi_pa_w )
{
	/*

	    bit     signal      description

	    PA0     f_ds<0>     drive 0 select
	    PA1     f_ds<1>     drive 1 select
	    PA2     f_ds<2>     drive 2 select
	    PA3     f_ds<3>     drive 3 select
	    PA4     f_side_1    floppy side select
	    PA5     f_pre_comp  precompensation
	    PA6     f_motor_on* floppy motor
	    PA7     f_dden*     double density select

	*/

	// floppy drive select
	floppy_image_device *floppy = nullptr;

	if (!BIT(data, 0)) floppy = m_floppy0->get_device();
	if (!BIT(data, 1)) floppy = m_floppy1->get_device();
	if (!BIT(data, 2)) floppy = m_floppy2->get_device();
	if (!BIT(data, 3)) floppy = m_floppy3->get_device();

	m_fdc->set_floppy(floppy);

	// floppy side select
	if (floppy) floppy->ss_w(BIT(data, 4));

	// floppy motor
	if (floppy) floppy->mon_w(BIT(data, 6));

	// density select
	m_fdc->dden_w(BIT(data, 7));
}

WRITE_LINE_MEMBER(v1050_state::write_centronics_busy)
{
	m_centronics_busy = state;
}

WRITE_LINE_MEMBER(v1050_state::write_centronics_perror)
{
	m_centronics_perror = state;
}

READ8_MEMBER(v1050_state::misc_ppi_pc_r)
{
	/*

	    bit     signal      description

	    PC0     pr_strobe   printer strobe
	    PC1     f_int_enb   floppy interrupt enable
	    PC2     baud_sel_a
	    PC3     baud_sel_b
	    PC4     pr_busy*    printer busy
	    PC5     pr_pe*      printer paper end
	    PC6
	    PC7

	*/

	UINT8 data = 0;

	data |= m_centronics_busy << 4;
	data |= m_centronics_perror << 5;

	return data;
}

void v1050_state::set_baud_sel(int baud_sel)
{
	if (baud_sel != m_baud_sel)
	{
		int divider = 1;

		switch (baud_sel)
		{
		case 0:
			divider = 13 * 16; // 19200
			break;

		case 1:
			divider = 13 * 8; // 38400
			break;

		case 2:
			divider = 8; // 500000
			break;

		case 3:
			divider = 13 * 2; // 153600
			break;
		}

		m_clock_sio->set_clock_scale((double) 1 / divider);

		m_baud_sel = baud_sel;
	}
}

WRITE8_MEMBER( v1050_state::misc_ppi_pc_w )
{
	/*

	    bit     signal      description

	    PC0     pr_strobe   printer strobe
	    PC1     f_int_enb   floppy interrupt enable
	    PC2     baud_sel_a
	    PC3     baud_sel_b
	    PC4     pr_busy*    printer busy
	    PC5     pr_pe*      printer paper end
	    PC6
	    PC7

	*/

	// printer strobe
	m_centronics->write_strobe(BIT(data, 0));

	// floppy interrupt enable
	m_f_int_enb = BIT(data, 1);
	update_fdc();

	// baud select
	set_baud_sel((data >> 2) & 0x03);
}

// Real Time Clock 8255A Interface

WRITE8_MEMBER( v1050_state::rtc_ppi_pb_w )
{
	/*

	    bit     signal      description

	    PB0                 RS-232
	    PB1                 Winchester
	    PB2                 keyboard
	    PB3                 floppy disk interrupt
	    PB4                 vertical interrupt
	    PB5                 display interrupt
	    PB6                 expansion B
	    PB7                 expansion A

	*/

	m_int_mask = data;
}

READ8_MEMBER( v1050_state::rtc_ppi_pa_r )
{
	return m_rtc_ppi_pa;
}

WRITE8_MEMBER( v1050_state::rtc_ppi_pa_w )
{
	m_rtc->d0_w((data >> 0) & 1);
	m_rtc->d1_w((data >> 1) & 1);
	m_rtc->d2_w((data >> 2) & 1);
	m_rtc->d3_w((data >> 3) & 1);
}

READ8_MEMBER( v1050_state::rtc_ppi_pc_r )
{
	/*

	    bit     signal      description

	    PC0
	    PC1
	    PC2
	    PC3                 clock busy
	    PC4
	    PC5
	    PC6
	    PC7

	*/

	return m_rtc_ppi_pc;
}

WRITE8_MEMBER( v1050_state::rtc_ppi_pc_w )
{
	/*

	    bit     signal      description

	    PC0
	    PC1
	    PC2
	    PC3
	    PC4                 clock address write
	    PC5                 clock data write
	    PC6                 clock data read
	    PC7                 clock device select

	*/

	m_rtc->address_write_w(BIT(data, 4));
	m_rtc->write_w(BIT(data, 5));
	m_rtc->read_w(BIT(data, 6));
	m_rtc->cs2_w(BIT(data, 7));
}

// Keyboard 8251A Interface

WRITE_LINE_MEMBER(v1050_state::write_keyboard_clock)
{
	m_uart_kb->write_txc(state);
	m_uart_kb->write_rxc(state);
}

WRITE_LINE_MEMBER( v1050_state::kb_rxrdy_w )
{
	set_interrupt(INT_KEYBOARD, state);
}

// Serial 8251A Interface

WRITE_LINE_MEMBER(v1050_state::write_sio_clock)
{
	m_uart_sio->write_txc(state);
	m_uart_sio->write_rxc(state);
}

WRITE_LINE_MEMBER( v1050_state::sio_rxrdy_w )
{
	m_rxrdy = state;

	set_interrupt(INT_RS_232, m_rxrdy || m_txrdy);
}

WRITE_LINE_MEMBER( v1050_state::sio_txrdy_w )
{
	m_txrdy = state;

	set_interrupt(INT_RS_232, m_rxrdy || m_txrdy);
}

// MB8877 Interface

void v1050_state::update_fdc()
{
	if (m_f_int_enb)
	{
		set_interrupt(INT_FLOPPY, m_fdc_irq ? 1 : 0);
		m_maincpu->set_input_line(INPUT_LINE_NMI, m_fdc_drq ? ASSERT_LINE : CLEAR_LINE);
	}
	else
	{
		set_interrupt(INT_FLOPPY, 0);
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	}
}

// disk format: 80 tracks, 1 head, 10 sectors, 512 bytes sector length, first sector id 1

static SLOT_INTERFACE_START( v1050_floppies )
	SLOT_INTERFACE( "525ssqd", FLOPPY_525_SSQD ) // Teac FD 55E-02-U
	SLOT_INTERFACE( "525qd", FLOPPY_525_QD ) // Teac FD 55-FV-35-U
SLOT_INTERFACE_END

WRITE_LINE_MEMBER( v1050_state::fdc_intrq_w )
{
	m_fdc_irq = state;

	update_fdc();
}

WRITE_LINE_MEMBER( v1050_state::fdc_drq_w )
{
	m_fdc_drq = state;

	update_fdc();
}


// Machine Initialization

IRQ_CALLBACK_MEMBER(v1050_state::v1050_int_ack)
{
	UINT8 vector = 0xf0 | (m_pic->a_r() << 1);

	//logerror("Interrupt Acknowledge Vector: %02x\n", vector);

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);

	return vector;
}

void v1050_state::machine_start()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	// initialize I8214
	m_pic->etlg_w(1);
	m_pic->inte_w(1);

	// initialize RTC
	m_rtc->cs1_w(1);

	// setup memory banking
	UINT8 *ram = m_ram->pointer();

	membank("bank1")->configure_entries(0, 2, ram, 0x10000);
	membank("bank1")->configure_entry(2, ram + 0x1c000);
	membank("bank1")->configure_entry(3, m_rom->base());

	program.install_readwrite_bank(0x2000, 0x3fff, "bank2");
	membank("bank2")->configure_entries(0, 2, ram + 0x2000, 0x10000);
	membank("bank2")->configure_entry(2, ram + 0x1e000);

	program.install_readwrite_bank(0x4000, 0x7fff, "bank3");
	membank("bank3")->configure_entries(0, 2, ram + 0x4000, 0x10000);

	program.install_readwrite_bank(0x8000, 0xbfff, "bank4");
	membank("bank4")->configure_entries(0, 2, ram + 0x8000, 0x10000);

	program.install_readwrite_bank(0xc000, 0xffff, "bank5");
	membank("bank5")->configure_entries(0, 3, ram + 0xc000, 0);

	bankswitch();

	// register for state saving
	save_item(NAME(m_int_mask));
	save_item(NAME(m_int_state));
	save_item(NAME(m_f_int_enb));
	save_item(NAME(m_fdc_irq));
	save_item(NAME(m_fdc_drq));
	save_item(NAME(m_keylatch));
	save_item(NAME(m_keydata));
	save_item(NAME(m_keyavail));
	save_item(NAME(m_rxrdy));
	save_item(NAME(m_txrdy));
	save_item(NAME(m_baud_sel));
	save_item(NAME(m_bank));
	save_item(NAME(m_centronics_busy));
	save_item(NAME(m_centronics_perror));
}

void v1050_state::machine_reset()
{
	m_bank = 0;
	bankswitch();

	set_baud_sel(0);

	m_fdc->reset();
}

// Machine Driver

static MACHINE_CONFIG_START( v1050, v1050_state )
	// basic machine hardware
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_16MHz/4)
	MCFG_CPU_PROGRAM_MAP(v1050_mem)
	MCFG_CPU_IO_MAP(v1050_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(v1050_state,v1050_int_ack)

	MCFG_QUANTUM_PERFECT_CPU(Z80_TAG)

	MCFG_CPU_ADD(M6502_TAG, M6502, XTAL_15_36MHz/16)
	MCFG_CPU_PROGRAM_MAP(v1050_crt_mem)
	MCFG_QUANTUM_PERFECT_CPU(M6502_TAG)

	// keyboard HACK
	MCFG_TIMER_DRIVER_ADD_PERIODIC("keyboard", v1050_state, v1050_keyboard_tick, attotime::from_hz(60))

	// video hardware
	MCFG_FRAGMENT_ADD(v1050_video)

	// devices
	MCFG_DEVICE_ADD(UPB8214_TAG, I8214, XTAL_16MHz/4)
	MCFG_I8214_IRQ_CALLBACK(WRITELINE(v1050_state, pic_int_w))

	MCFG_DEVICE_ADD(MSM58321RS_TAG, MSM58321, XTAL_32_768kHz)
	MCFG_MSM58321_D0_HANDLER(WRITELINE(v1050_state, rtc_ppi_pa_0_w))
	MCFG_MSM58321_D1_HANDLER(WRITELINE(v1050_state, rtc_ppi_pa_1_w))
	MCFG_MSM58321_D2_HANDLER(WRITELINE(v1050_state, rtc_ppi_pa_2_w))
	MCFG_MSM58321_D3_HANDLER(WRITELINE(v1050_state, rtc_ppi_pa_3_w))
	MCFG_MSM58321_BUSY_HANDLER(WRITELINE(v1050_state, rtc_ppi_pc_3_w))

	MCFG_DEVICE_ADD(I8255A_DISP_TAG, I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(DEVREAD8(I8255A_M6502_TAG, i8255_device, pb_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(v1050_state, disp_ppi_pc_w))

	MCFG_DEVICE_ADD(I8255A_MISC_TAG, I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(v1050_state, misc_ppi_pa_w))
	MCFG_I8255_OUT_PORTB_CB(DEVWRITE8("cent_data_out", output_latch_device, write))
	MCFG_I8255_IN_PORTC_CB(READ8(v1050_state,misc_ppi_pc_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(v1050_state,misc_ppi_pc_w))

	MCFG_DEVICE_ADD(I8255A_RTC_TAG, I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(v1050_state, rtc_ppi_pa_r))
	MCFG_I8255_OUT_PORTA_CB(WRITE8(v1050_state, rtc_ppi_pa_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(v1050_state, rtc_ppi_pb_w))
	MCFG_I8255_IN_PORTC_CB(READ8(v1050_state, rtc_ppi_pc_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(v1050_state, rtc_ppi_pc_w))

	MCFG_DEVICE_ADD(I8255A_M6502_TAG, I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(DEVREAD8(I8255A_DISP_TAG, i8255_device, pb_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(v1050_state, m6502_ppi_pc_w))

	MCFG_DEVICE_ADD(I8251A_KB_TAG, I8251, 0/*XTAL_16MHz/8,*/)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE(V1050_KEYBOARD_TAG, v1050_keyboard_device, si_w))
	MCFG_I8251_RXRDY_HANDLER(WRITELINE(v1050_state, kb_rxrdy_w))

	MCFG_DEVICE_ADD(CLOCK_KB_TAG, CLOCK, XTAL_16MHz/4/13/8)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(v1050_state, write_keyboard_clock))

	// keyboard
	MCFG_DEVICE_ADD(V1050_KEYBOARD_TAG, V1050_KEYBOARD, 0)
	MCFG_V1050_KEYBOARD_OUT_TX_HANDLER(DEVWRITELINE(I8251A_KB_TAG, i8251_device, write_rxd))

	MCFG_DEVICE_ADD(I8251A_SIO_TAG, I8251, 0/*XTAL_16MHz/8,*/)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_rts))
	MCFG_I8251_RXRDY_HANDLER(WRITELINE(v1050_state, sio_rxrdy_w))
	MCFG_I8251_TXRDY_HANDLER(WRITELINE(v1050_state, sio_txrdy_w))

	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(I8251A_SIO_TAG, i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE(I8251A_SIO_TAG, i8251_device, write_dsr))

	MCFG_DEVICE_ADD(CLOCK_SIO_TAG, CLOCK, XTAL_16MHz/4)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(v1050_state, write_sio_clock))

	MCFG_MB8877_ADD(MB8877_TAG, XTAL_16MHz/16)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(v1050_state, fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(v1050_state, fdc_drq_w))
	MCFG_FLOPPY_DRIVE_ADD(MB8877_TAG":0", v1050_floppies, "525qd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(MB8877_TAG":1", v1050_floppies, "525qd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(MB8877_TAG":2", v1050_floppies, nullptr,    floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(MB8877_TAG":3", v1050_floppies, nullptr,    floppy_image_device::default_floppy_formats)

	// SASI bus
	MCFG_DEVICE_ADD(SASIBUS_TAG, SCSI_PORT, 0)
	MCFG_SCSI_DATA_INPUT_BUFFER("scsi_data_in")
	MCFG_SCSI_REQ_HANDLER(DEVWRITELINE("scsi_ctrl_in", input_buffer_device, write_bit0)) MCFG_DEVCB_XOR(1)
	MCFG_SCSI_BSY_HANDLER(DEVWRITELINE("scsi_ctrl_in", input_buffer_device, write_bit1))
	MCFG_SCSI_MSG_HANDLER(DEVWRITELINE("scsi_ctrl_in", input_buffer_device, write_bit2))
	MCFG_SCSI_CD_HANDLER(DEVWRITELINE("scsi_ctrl_in", input_buffer_device, write_bit3))
	MCFG_SCSI_IO_HANDLER(WRITELINE(v1050_state, write_sasi_io)) MCFG_DEVCB_XOR(1) // bit4
	MCFG_SCSIDEV_ADD(SASIBUS_TAG ":" SCSI_PORT_DEVICE1, "harddisk", S1410, SCSI_ID_0)

	MCFG_SCSI_OUTPUT_LATCH_ADD("scsi_data_out", SASIBUS_TAG)
	MCFG_DEVICE_ADD("scsi_data_in", INPUT_BUFFER, 0)
	MCFG_DEVICE_ADD("scsi_ctrl_in", INPUT_BUFFER, 0)

	MCFG_TIMER_DRIVER_ADD(TIMER_ACK_TAG, v1050_state, sasi_ack_tick)
	MCFG_TIMER_DRIVER_ADD(TIMER_RST_TAG, v1050_state, sasi_rst_tick)

	// software lists
	MCFG_SOFTWARE_LIST_ADD("flop_list", "v1050_flop")
	MCFG_SOFTWARE_LIST_ADD("hdd_list", "v1050_hdd")

	// printer
	MCFG_CENTRONICS_ADD(CENTRONICS_TAG, centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(v1050_state, write_centronics_busy))
	MCFG_CENTRONICS_PERROR_HANDLER(WRITELINE(v1050_state, write_centronics_perror))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
MACHINE_CONFIG_END

// ROMs

ROM_START( v1050 )
	ROM_REGION( 0x10000, Z80_TAG, 0 )
	ROM_LOAD( "e244-032 rev 1.2.u86", 0x0000, 0x2000, CRC(46f847a7) SHA1(374db7a38a9e9230834ce015006e2f1996b9609a) )

	ROM_REGION( 0x10000, M6502_TAG, 0 )
	ROM_LOAD( "e244-033 rev 1.1.u77", 0xe000, 0x2000, CRC(c0502b66) SHA1(bc0015f5b14f98110e652eef9f7c57c614683be5) )
ROM_END

// System Drivers

//    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   INIT    COMPANY                     FULLNAME        FLAGS
COMP( 1983, v1050,  0,      0,      v1050,  v1050, driver_device,   0,      "Visual Technology Inc",    "Visual 1050", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND | MACHINE_IMPERFECT_KEYBOARD )

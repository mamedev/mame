/***************************************************************************

    drivers/bw2.c

    Bondwell BW 2

    - Z80L CPU 4MHz
    - 64KB RAM, expandable to 224KB RAM.
    - 4KB System ROM
    - MSM6255 LCD controller, 640x200 pixels
    - 16KB Video RAM
    - TMS2797 FDC controller
    - 8251 USART serial interface
    - 8253 PIT timer
    - 8255 PPI

  http://www.thebattles.net/bondwell/

  http://www.vintage-computer.com/bondwell2.shtml

  http://www2.okisemi.com/site/productscatalog/displaydrivers/availabledocuments/Intro-7090.html


    TODO:

    - modem card

***************************************************************************/

#include "includes/bw2.h"

int bw2_state::get_ramdisk_size()
{
	return ioport("RAMCARD")->read() * 256;
}

/* Memory */

void bw2_state::bankswitch(UINT8 data)
{
	/*

        Y0  /RAM1   Memory bank 1
        Y1  /VRAM   Video memory
        Y2  /RAM2   Memory bank 2
        Y3  /RAM3   Memory bank 3
        Y4  /RAM4   Memory bank 4
        Y5  /RAM5   Memory bank 5
        Y6  /RAM6   Memory bank 6
        Y7  /ROM    ROM

    */

	address_space *program = m_maincpu->space(AS_PROGRAM);

	int max_ram_bank = 0;

	m_bank = data & 0x07;

	switch (m_ram->size())
	{
	case 64 * 1024:
		max_ram_bank = BANK_RAM1;
		break;

	case 96 * 1024:
		max_ram_bank = BANK_RAM2;
		break;

	case 128 * 1024:
		max_ram_bank = BANK_RAM3;
		break;

	case 160 * 1024:
		max_ram_bank = BANK_RAM4;
		break;

	case 192 * 1024:
		max_ram_bank = BANK_RAM5;
		break;

	case 224 * 1024:
		max_ram_bank = BANK_RAM6;
		break;
	}

	switch (m_bank)
	{
	case BANK_RAM1:
		program->install_readwrite_bank(0x0000, 0x7fff, "bank1");
		break;

	case BANK_VRAM:
		program->install_readwrite_bank(0x0000, 0x3fff, 0, 0x4000, "bank1");
		break;

	case BANK_RAM2:
	case BANK_RAM3:
	case BANK_RAM4:
	case BANK_RAM5:
	case BANK_RAM6:
		if (m_bank > max_ram_bank)
		{
			program->unmap_readwrite(0x0000, 0x7fff);
		}
		else
		{
			program->install_readwrite_bank(0x0000, 0x7fff, "bank1");
		}
		break;

	case BANK_ROM:
		program->install_read_bank(0x0000, 0x7fff, "bank1");
		program->unmap_write(0x0000, 0x7fff);
		break;
	}

	membank("bank1")->set_entry(m_bank);
}

void bw2_state::ramcard_bankswitch(UINT8 data)
{
	/*

        Y0  /RAM1   Memory bank 1
        Y1  /VRAM   Video memory
        Y2  /RAM2   RAMCARD ROM
        Y3  /RAM3   Memory bank 3
        Y4  /RAM4   Memory bank 4
        Y5  /RAM5   RAMCARD RAM
        Y6  /RAM6   Memory bank 6
        Y7  /ROM    ROM

    */

	address_space *program = m_maincpu->space(AS_PROGRAM);

	int max_ram_bank = BANK_RAM1;

	m_bank = data & 0x07;

	switch (m_ram->size())
	{
	case 64 * 1024:
	case 96 * 1024:
		max_ram_bank = BANK_RAM1;
		break;

	case 128 * 1024:
		max_ram_bank = BANK_RAM3;
		break;

	case 160 * 1024:
		max_ram_bank = BANK_RAM4;
		break;

	case 192 * 1024:
	case 224 * 1024:
		max_ram_bank = BANK_RAM6;
		break;
	}

	switch (m_bank)
	{
	case BANK_RAM1:
	case BANK_RAMCARD_RAM:
		program->install_readwrite_bank(0x0000, 0x7fff, "bank1");
		break;

	case BANK_VRAM:
		program->install_readwrite_bank(0x0000, 0x3fff, 0, 0x4000, "bank1");
		break;

	case BANK_RAM3:
	case BANK_RAM4:
	case BANK_RAM6:
		if (m_bank > max_ram_bank)
		{
			program->unmap_readwrite(0x0000, 0x7fff);
		}
		else
		{
			program->install_readwrite_bank(0x0000, 0x7fff, "bank1");
		}
		break;

	case BANK_RAMCARD_ROM:
		program->install_read_bank(0x0000, 0x3fff, 0, 0x4000, "bank1");
		program->unmap_write(0x0000, 0x3fff, 0, 0x4000);
		break;

	case BANK_ROM:
		program->install_read_bank(0x0000, 0x7fff, "bank1");
		program->unmap_write(0x0000, 0x7fff);
		break;
	}

	membank("bank1")->set_entry(m_bank);
}

WRITE8_MEMBER( bw2_state::ramcard_bank_w )
{
	address_space *program = m_maincpu->space(AS_PROGRAM);

	UINT8 ramcard_bank = data & 0x0f;
	UINT32 bank_offset = ramcard_bank * 0x8000;

	if ((get_ramdisk_size() == 256) && (ramcard_bank > 7))
	{
		program->unmap_readwrite(0x0000, 0x7fff);
	}
	else
	{
		program->install_readwrite_bank(0x0000, 0x7fff, "bank1");
	}

	membank("bank1")->configure_entry(BANK_RAMCARD_RAM, m_ramcard_ram + bank_offset);
	membank("bank1")->set_entry(m_bank);
}

/* Floppy */

WRITE_LINE_MEMBER( bw2_state::fdc_drq_w )
{
	if (state)
	{
		if (m_maincpu->state_int(Z80_HALT))
		{
			device_set_input_line(m_maincpu, INPUT_LINE_NMI, HOLD_LINE);
		}
	}
	else
	{
		device_set_input_line(m_maincpu, INPUT_LINE_NMI, CLEAR_LINE);
	}
}

static ADDRESS_MAP_START( bw2_mem, AS_PROGRAM, 8, bw2_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_RAMBANK("bank1")
	AM_RANGE(0x8000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( bw2_io, AS_IO, 8, bw2_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x00, 0x03 ) AM_DEVREADWRITE(I8255A_TAG, i8255_device, read, write)
	AM_RANGE( 0x10, 0x13 ) AM_DEVREADWRITE_LEGACY(PIT8253_TAG, pit8253_r, pit8253_w)
	AM_RANGE( 0x20, 0x21 ) AM_DEVREADWRITE(MSM6255_TAG, msm6255_device, read, write)
//  AM_RANGE( 0x30, 0x3f ) SLOT
	AM_RANGE( 0x40, 0x40 ) AM_DEVREADWRITE(I8251_TAG, i8251_device, data_r, data_w)
	AM_RANGE( 0x41, 0x41 ) AM_DEVREADWRITE(I8251_TAG, i8251_device, status_r, control_w)
	AM_RANGE( 0x50, 0x50 ) AM_DEVWRITE(CENTRONICS_TAG, centronics_device, write)
	AM_RANGE( 0x60, 0x63 ) AM_DEVREADWRITE_LEGACY(WD2797_TAG, wd17xx_r, wd17xx_w)
//  AM_RANGE( 0x70, 0x7f ) MODEMSEL
ADDRESS_MAP_END

/*
  Keyboard matrix
        X0    X1    X2    X3    X4    X5    X6    X7
     +-----+-----+-----+-----+-----+-----+-----+-----+
  Y9 |CAPS |     |     |     |     |     |     |     |
     +-----+-----+-----+-----+-----+-----+-----+-----+
  Y8 |SHIFT|     |SPACE|  Z  |  A  |  Q  | 2 " | F1  |
     +-----+-----+-----+-----+-----+-----+-----+-----+
  Y7 |CTRL | - = | \ | |  X  |  S  |  W  | 3 # | F2  |
     +-----+-----+-----+-----+-----+-----+-----+-----+
  Y6 | @ ` |  P  | UP  |  C  |  D  |  E  | 4 $ | F3  |
     +-----+-----+-----+-----+-----+-----+-----+-----+
  Y5 | 1 ! |     |     |  V  |  F  |  R  | 5 % | F4  |
     +-----+-----+-----+-----+-----+-----+-----+-----+
  Y4 | ESC |     |     |  B  |  G  |  T  | 6 & | F5  |
     +-----+-----+-----+-----+-----+-----+-----+-----+
  Y3 | TAB | : * |ENTER|  N  |  H  |  Y  | 7 ' | F6  |
     +-----+-----+-----+-----+-----+-----+-----+-----+
  Y2 |DOWN | ^ ~ | [ { |  M  |  J  |  U  | 8 ( | F7  |
     +-----+-----+-----+-----+-----+-----+-----+-----+
  Y1 |RIGHT| ; + | ] } | , < |  K  |  I  | 9 ) | F8  |
     +-----+-----+-----+-----+-----+-----+-----+-----+
  Y0 |LEFT | BS  | / ? | . > |  L  |  O  | 0 _ | DEL |
     +-----+-----+-----+-----+-----+-----+-----+-----+
*/

static INPUT_PORTS_START( bw2 )
	PORT_START("ROW0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('_')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(DEL))

	PORT_START("ROW1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))

	PORT_START("ROW2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))

	PORT_START("ROW3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR(UCHAR_MAMEKEY(TAB))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))

	PORT_START("ROW4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))

	PORT_START("ROW5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))

	PORT_START("ROW6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))

	PORT_START("ROW7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))

	PORT_START("ROW8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))

	PORT_START("ROW9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("BAUD")
	PORT_CONFNAME( 0x05, 0x05, "Baud rate")
	PORT_CONFSETTING( 0x00, "9600 baud" )
	PORT_CONFSETTING( 0x01, "4800 baud" )
	PORT_CONFSETTING( 0x02, "2400 baud" )
	PORT_CONFSETTING( 0x03, "1200 baud" )
	PORT_CONFSETTING( 0x04, "600 baud" )
	PORT_CONFSETTING( 0x05, "300 baud" )

	PORT_START("RAMCARD")
	PORT_CONFNAME( 0x03, 0x00, "RAMCARD")
	PORT_CONFSETTING( 0x00, DEF_STR( None ) )
	PORT_CONFSETTING( 0x01, "256 KB" )
	PORT_CONFSETTING( 0x02, "512 KB" )
INPUT_PORTS_END

/* PPI */

WRITE8_MEMBER( bw2_state::ppi_pa_w )
{
	/*

        PA0     KB0 Keyboard line select 0
        PA1     KB1 Keyboard line select 1
        PA2     KB2 Keyboard line select 2
        PA3     KB3 Keyboard line select 3
        PA4     /DS0 Drive select 0
        PA5     /DS1 Drive select 1
        PA6     Select RS232 connector
        PA7     /STROBE to centronics printer

    */

	m_kb_row = data & 0x0f;

	if (BIT(data, 4))
	{
		m_drive = 0;

		wd17xx_set_drive(m_fdc, m_drive);
	}

	if (BIT(data, 5))
	{
		m_drive = 1;

		wd17xx_set_drive(m_fdc, m_drive);
	}

	m_centronics->strobe_w(BIT(data, 7));
}

READ8_MEMBER( bw2_state::ppi_pb_r )
{
	/*

        PB0     Keyboard column status of selected line
        PB1     Keyboard column status of selected line
        PB2     Keyboard column status of selected line
        PB3     Keyboard column status of selected line
        PB4     Keyboard column status of selected line
        PB5     Keyboard column status of selected line
        PB6     Keyboard column status of selected line
        PB7     Keyboard column status of selected line

    */

	static const char *const rownames[] = { "ROW0", "ROW1", "ROW2", "ROW3", "ROW4", "ROW5", "ROW6", "ROW7", "ROW8", "ROW9" };

	UINT8 data = 0xff;

	if (m_kb_row <= 9)
	{
		data = ioport(rownames[m_kb_row])->read();
	}

	return data;
}

WRITE8_MEMBER( bw2_state::ppi_pc_w )
{
	/*

        PC0     Memory bank select
        PC1     Memory bank select
        PC2     Memory bank select
        PC3     Not connected

    */

	if (get_ramdisk_size() > 0)
	{
		ramcard_bankswitch(data & 0x07);
	}
	else
	{
		bankswitch(data & 0x07);
	}
}

READ8_MEMBER( bw2_state::ppi_pc_r )
{
	/*

        PC4     BUSY from centronics printer
        PC5     M/FDBK motor feedback
        PC6     RLSD Carrier detect from RS232
        PC7     /PROT Write protected disk

    */

	UINT8 data = 0;

	data |= m_centronics->busy_r() << 4;
	data |= m_mfdbk << 5;

	data |= floppy_wpt_r(m_drive ? m_floppy1 : m_floppy0) << 7;

	return data;
}

static I8255A_INTERFACE( ppi_intf )
{
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(bw2_state, ppi_pa_w),
	DEVCB_DRIVER_MEMBER(bw2_state, ppi_pb_r),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(bw2_state, ppi_pc_r),
	DEVCB_DRIVER_MEMBER(bw2_state, ppi_pc_w),
};

/* PIT */

WRITE_LINE_MEMBER( bw2_state::pit_out0_w )
{
	m_uart->transmit_clock();
	m_uart->receive_clock();
}

WRITE_LINE_MEMBER( bw2_state::mtron_w )
{
	m_mtron = state;
	m_mfdbk = !state;

	floppy_mon_w(m_floppy0, m_mtron);
	floppy_mon_w(m_floppy1, m_mtron);

	floppy_drive_set_ready_state(m_floppy0, 1, 1);
	floppy_drive_set_ready_state(m_floppy1, 1, 1);
}

static const struct pit8253_config pit_intf =
{
	{
		{
			XTAL_16MHz/4,	/* 8251 USART TXC, RXC */
			DEVCB_LINE_VCC,
			DEVCB_DRIVER_LINE_MEMBER(bw2_state, pit_out0_w)
		},
		{
			11000,		/* LCD controller */
			DEVCB_LINE_VCC,
			DEVCB_DEVICE_LINE(PIT8253_TAG, pit8253_clk2_w)
		},
		{
			0,		/* Floppy /MTRON */
			DEVCB_LINE_VCC,
			DEVCB_DRIVER_LINE_MEMBER(bw2_state, mtron_w)
		}
	}
};


/* Video */

static PALETTE_INIT( bw2 )
{
    palette_set_color_rgb(machine, 0, 0xa5, 0xad, 0xa5);
    palette_set_color_rgb(machine, 1, 0x31, 0x39, 0x10);
}

static MSM6255_CHAR_RAM_READ( bw2_charram_r )
{
	bw2_state *state = device->machine().driver_data<bw2_state>();

	return state->m_video_ram[ma & 0x3fff];
}

static MSM6255_INTERFACE( lcdc_intf )
{
	SCREEN_TAG,
	0,
	bw2_charram_r,
};

static LEGACY_FLOPPY_OPTIONS_START(bw2)
	LEGACY_FLOPPY_OPTION(bw2, "dsk", "BW2 340K disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([1])
		TRACKS([80])
		SECTORS([17])
		SECTOR_LENGTH([256])
		FIRST_SECTOR_ID([0]))
	LEGACY_FLOPPY_OPTION(bw2, "dsk", "BW2 360K disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([1])
		TRACKS([80])
		SECTORS([18])
		SECTOR_LENGTH([256])
		FIRST_SECTOR_ID([0]))
LEGACY_FLOPPY_OPTIONS_END

static const floppy_interface bw2_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_3_5_SSDD, // Teac FD-35
	LEGACY_FLOPPY_OPTIONS_NAME(bw2),
	"floppy_3_5",
	NULL
};

static const wd17xx_interface fdc_intf =
{
	DEVCB_LINE_GND,
	DEVCB_CPU_INPUT_LINE(Z80_TAG, INPUT_LINE_IRQ0),
	DEVCB_DRIVER_LINE_MEMBER(bw2_state, fdc_drq_w),
	{ FLOPPY_0, FLOPPY_1, NULL, NULL }
};

/* Machine */

void bw2_state::machine_start()
{
	/* allocate memory */
	m_video_ram = auto_alloc_array(machine(), UINT8, BW2_VIDEORAM_SIZE);
	m_ramcard_ram = auto_alloc_array(machine(), UINT8, BW2_RAMCARD_SIZE);

	/* memory banking */
	membank("bank1")->configure_entry(BANK_RAM1, m_ram->pointer());
	membank("bank1")->configure_entry(BANK_VRAM, m_video_ram);
	membank("bank1")->configure_entry(BANK_ROM, memregion("ic1")->base());

	/* register for state saving */
	save_item(NAME(m_kb_row));
	save_pointer(NAME(m_ramcard_ram), BW2_RAMCARD_SIZE);
	save_item(NAME(m_bank));
	save_item(NAME(m_drive));
	save_item(NAME(m_mtron));
	save_item(NAME(m_mfdbk));
	save_pointer(NAME(m_video_ram), BW2_VIDEORAM_SIZE);
}

void bw2_state::machine_reset()
{
	address_space *io = machine().device(Z80_TAG)->memory().space(AS_IO);

	if (get_ramdisk_size() > 0)
	{
		// RAMCARD installed

		membank("bank1")->configure_entry(BANK_RAMCARD_ROM, memregion("ramcard")->base());
		membank("bank1")->configure_entries(BANK_RAM3, 2, m_ram->pointer() + 0x8000, 0x8000);
		membank("bank1")->configure_entry(BANK_RAMCARD_RAM, m_ramcard_ram);
		membank("bank1")->configure_entry(BANK_RAM6, m_ram->pointer() + 0x18000);

		io->install_write_handler(0x30, 0x30, 0, 0x0f, write8_delegate(FUNC(bw2_state::ramcard_bank_w), this), 0);
	}
	else
	{
		// no RAMCARD

		membank("bank1")->configure_entries(BANK_RAM2, 5, m_ram->pointer() + 0x8000, 0x8000);

		io->unmap_write(0x30, 0x30, 0, 0x0f);
	}

	membank("bank1")->set_entry(BANK_ROM);
}

static MACHINE_CONFIG_START( bw2, bw2_state )
	// basic machine hardware
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_16MHz/4)
	MCFG_CPU_PROGRAM_MAP(bw2_mem)
	MCFG_CPU_IO_MAP(bw2_io)

	// video hardware
	MCFG_SCREEN_ADD( SCREEN_TAG, LCD )
	MCFG_SCREEN_REFRESH_RATE( 60 )
	MCFG_SCREEN_UPDATE_DEVICE( MSM6255_TAG, msm6255_device, screen_update )
	MCFG_SCREEN_SIZE( 640, 200 )
	MCFG_SCREEN_VISIBLE_AREA( 0, 640-1, 0, 200-1 )
	MCFG_DEFAULT_LAYOUT( layout_lcd )

	MCFG_PALETTE_LENGTH( 2 )
	MCFG_PALETTE_INIT( bw2 )

	// devices
	MCFG_PIT8253_ADD(PIT8253_TAG, pit_intf)
	MCFG_I8255A_ADD(I8255A_TAG, ppi_intf)
	MCFG_MSM6255_ADD(MSM6255_TAG, XTAL_16MHz, lcdc_intf)
	MCFG_CENTRONICS_PRINTER_ADD(CENTRONICS_TAG, standard_centronics)
	MCFG_I8251_ADD(I8251_TAG, default_i8251_interface)
	MCFG_WD2797_ADD(WD2797_TAG, fdc_intf)
	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(bw2_floppy_interface)

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop_list","bw2")

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
	MCFG_RAM_EXTRA_OPTIONS("96K,128K,160K,192K,224K")
MACHINE_CONFIG_END

/***************************************************************************

  System driver(s)

***************************************************************************/

ROM_START( bw2 )
	ROM_REGION(0x10000, "ic1", 0)
	ROM_SYSTEM_BIOS(0, "20", "BW 2 v2.0")
	ROMX_LOAD("bw2-20.ic8", 0x0000, 0x1000, CRC(86f36471) SHA1(a3e2ba4edd50ff8424bb0675bdbb3b9f13c04c9d), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "12", "BW 2 v1.2")
	ROMX_LOAD("bw2-12.ic8", 0x0000, 0x1000, CRC(0ab42d10) SHA1(430b232631eee9b715151b8d191b7eb9449ac513), ROM_BIOS(2))

	ROM_REGION(0x4000, "ramcard", 0)
	ROM_LOAD("ramcard-10.bin", 0x0000, 0x4000, CRC(68cde1ba) SHA1(a776a27d64f7b857565594beb63aa2cd692dcf04))
ROM_END

/*    YEAR  NAME    PARENT  COMPAT  MACHINE   INPUT   INIT    COMPANY              FULLNAME  FLAGS */
COMP( 1985, bw2,    0,      0,      bw2,      bw2, driver_device,    0,	"Bondwell Holding",  "Bondwell Model 2",   GAME_NO_SOUND )

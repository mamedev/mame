// license:BSD-3-Clause
// copyright-holders:Curt Coder
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

***************************************************************************/

#include "emu.h"
#include "includes/bw2.h"
#include "bus/rs232/rs232.h"
#include "screen.h"
#include "softlist.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

enum
{
	RAM1 = 0,
	VRAM,
	RAM2,
	RAM3,
	RAM4,
	RAM5,
	RAM6,
	ROM
};

#define HAS_KB_OF_RAM(_kb) \
	(m_ram->size() >= (_kb * 1024))


//**************************************************************************
//  ADDRESS DECODING
//**************************************************************************

//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( bw2_state::read )
{
	int rom = 1, vram = 1, ram1 = 1, ram2 = 1, ram3 = 1, ram4 = 1, ram5 = 1, ram6 = 1;

	uint8_t data = 0xff;

	switch (m_bank)
	{
	case RAM1: ram1 = 0; break;
	case VRAM: vram = 0; break;
	case RAM2: ram2 = 0; break;
	case RAM3: ram3 = 0; break;
	case RAM4: ram4 = 0; break;
	case RAM5: ram5 = 0; break;
	case RAM6: ram6 = 0; break;
	case ROM: rom = 0; break;
	}

	if (offset < 0x8000)
	{
		if (!rom)
			data = m_rom->base()[offset & 0x3fff];

		if (!vram)
			data = m_video_ram[offset & 0x3fff];

		if (!ram1)
			data = m_ram->pointer()[offset];

		if (!ram2 && HAS_KB_OF_RAM(96))
			data = m_ram->pointer()[0x10000 | offset];

		if (!ram3 && HAS_KB_OF_RAM(128))
			data = m_ram->pointer()[0x18000 | offset];

		if (!ram4 && HAS_KB_OF_RAM(160))
			data = m_ram->pointer()[0x20000 | offset];

		if (!ram5 && HAS_KB_OF_RAM(192))
			data = m_ram->pointer()[0x28000 | offset];

		if (!ram6 && HAS_KB_OF_RAM(224))
			data = m_ram->pointer()[0x30000 | offset];
	}
	else
	{
		data = m_ram->pointer()[offset];
	}

	return m_exp->cd_r(offset, data, ram2, ram3, ram4, ram5, ram6);
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( bw2_state::write )
{
	int vram = 1, ram1 = 1, ram2 = 1, ram3 = 1, ram4 = 1, ram5 = 1, ram6 = 1;

	switch (m_bank)
	{
	case RAM1: ram1 = 0; break;
	case VRAM: vram = 0; break;
	case RAM2: ram2 = 0; break;
	case RAM3: ram3 = 0; break;
	case RAM4: ram4 = 0; break;
	case RAM5: ram5 = 0; break;
	case RAM6: ram6 = 0; break;
	}

	if (offset < 0x8000)
	{
		if (!vram)
			m_video_ram[offset & 0x3fff] = data;

		if (!ram1)
			m_ram->pointer()[offset] = data;

		if (!ram2 && HAS_KB_OF_RAM(96))
			m_ram->pointer()[0x10000 | offset] = data;

		if (!ram3 && HAS_KB_OF_RAM(128))
			m_ram->pointer()[0x18000 | offset] = data;

		if (!ram4 && HAS_KB_OF_RAM(160))
			m_ram->pointer()[0x20000 | offset] = data;

		if (!ram5 && HAS_KB_OF_RAM(192))
			m_ram->pointer()[0x28000 | offset] = data;

		if (!ram6 && HAS_KB_OF_RAM(224))
			m_ram->pointer()[0x30000 | offset] = data;
	}
	else
	{
		m_ram->pointer()[offset] = data;
	}

	m_exp->cd_w(offset, data, ram2, ram3, ram4, ram5, ram6);
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( bw2_mem )
//-------------------------------------------------

void bw2_state::bw2_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).rw(FUNC(bw2_state::read), FUNC(bw2_state::write));
}


//-------------------------------------------------
//  ADDRESS_MAP( bw2_io )
//-------------------------------------------------

void bw2_state::bw2_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x03).rw(I8255A_TAG, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x10, 0x13).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x20, 0x21).m(m_lcdc, FUNC(msm6255_device::map));
	map(0x30, 0x3f).rw(m_exp, FUNC(bw2_expansion_slot_device::slot_r), FUNC(bw2_expansion_slot_device::slot_w));
	map(0x40, 0x41).rw(m_uart, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x50, 0x50).w("cent_data_out", FUNC(output_latch_device::bus_w));
	map(0x60, 0x63).rw(m_fdc, FUNC(wd2797_device::read), FUNC(wd2797_device::write));
	map(0x70, 0x7f).rw(m_exp, FUNC(bw2_expansion_slot_device::modsel_r), FUNC(bw2_expansion_slot_device::modsel_w));
}


//-------------------------------------------------
//  ADDRESS_MAP( lcdc_map )
//-------------------------------------------------

void bw2_state::lcdc_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x3fff).ram().share("videoram");
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( bw2 )
//-------------------------------------------------

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
	PORT_START("Y0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('_')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(DEL))

	PORT_START("Y1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))

	PORT_START("Y2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))

	PORT_START("Y3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR(UCHAR_MAMEKEY(TAB))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))

	PORT_START("Y4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))

	PORT_START("Y5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))

	PORT_START("Y6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))

	PORT_START("Y7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))

	PORT_START("Y8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))

	PORT_START("Y9")
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
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

WRITE_LINE_MEMBER( bw2_state::write_centronics_busy )
{
	m_centronics_busy = state;
}

//-------------------------------------------------
//  I8255A interface
//-------------------------------------------------

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

	// keyboard
	m_kb = data & 0x0f;

	// drive select
	m_floppy = nullptr;
	if (BIT(data, 4)) m_floppy = m_floppy0->get_device();
	if (BIT(data, 5)) m_floppy = m_floppy1->get_device();

	m_fdc->set_floppy(m_floppy);
	if (m_floppy) m_floppy->mon_w(m_mtron);

	// centronics strobe
	m_centronics->write_strobe(BIT(data, 7));
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

	uint8_t data = 0xff;

	if (m_kb < 10)
	{
		data = m_y[m_kb]->read();
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

	m_bank = data & 0x07;
}

READ8_MEMBER( bw2_state::ppi_pc_r )
{
	/*

	    PC4     BUSY from centronics printer
	    PC5     M/FDBK motor feedback
	    PC6     RLSD Carrier detect from RS232
	    PC7     /PROT Write protected disk

	*/

	uint8_t data = 0;

	// centronics busy
	data |= m_centronics_busy << 4;

	// floppy motor
	data |= m_mfdbk << 5;

	// write protect
	if (m_floppy) data |= m_floppy->wpt_r() << 7;

	return data;
}

//-------------------------------------------------
//  pit8253_config pit_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( bw2_state::mtron_w )
{
	m_mtron = state;
	m_mfdbk = !state;

	if (m_floppy) m_floppy->mon_w(m_mtron);
}

//-------------------------------------------------
//  floppy_format_type floppy_formats
//-------------------------------------------------

WRITE_LINE_MEMBER( bw2_state::fdc_drq_w )
{
	if (state)
	{
		if (m_maincpu->state_int(Z80_HALT))
		{
			m_maincpu->set_input_line(INPUT_LINE_NMI, HOLD_LINE);
		}
	}
	else
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	}
}

FLOPPY_FORMATS_MEMBER( bw2_state::floppy_formats )
	FLOPPY_BW2_FORMAT
FLOPPY_FORMATS_END

static void bw2_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD); // Teac FD-35
}



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************


void bw2_state::bw2_palette(palette_device &palette) const
{
	palette.set_pen_color(0, 0xa5, 0xad, 0xa5);
	palette.set_pen_color(1, 0x31, 0x39, 0x10);
}


//-------------------------------------------------
//  MACHINE_START( bw2 )
//-------------------------------------------------

void bw2_state::machine_start()
{
	// register for state saving
	save_item(NAME(m_kb));
	save_item(NAME(m_bank));
	save_item(NAME(m_mtron));
	save_item(NAME(m_mfdbk));
	save_item(NAME(m_centronics_busy));
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( bw2 )
//-------------------------------------------------

void bw2_state::bw2(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &bw2_state::bw2_mem);
	m_maincpu->set_addrmap(AS_IO, &bw2_state::bw2_io);

	// video hardware
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_screen_update(MSM6255_TAG, FUNC(msm6255_device::screen_update));
	screen.set_size(640, 200);
	screen.set_visarea(0, 640-1, 0, 200-1);
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(bw2_state::bw2_palette), 2);

	// devices
	PIT8253(config, m_pit, 0);
	m_pit->set_clk<0>(16_MHz_XTAL / 4); // 8251 USART TXC, RXC
	m_pit->out_handler<0>().set(m_uart, FUNC(i8251_device::write_txc));
	m_pit->out_handler<0>().append(m_uart, FUNC(i8251_device::write_rxc));
	m_pit->set_clk<1>(11000); // LCD controller
	m_pit->out_handler<1>().set(m_pit, FUNC(pit8253_device::write_clk2));
	m_pit->set_clk<2>(0); // Floppy /MTRON
	m_pit->out_handler<2>().set(FUNC(bw2_state::mtron_w));

	i8255_device &ppi(I8255A(config, I8255A_TAG));
	ppi.out_pa_callback().set(FUNC(bw2_state::ppi_pa_w));
	ppi.in_pb_callback().set(FUNC(bw2_state::ppi_pb_r));
	ppi.in_pc_callback().set(FUNC(bw2_state::ppi_pc_r));
	ppi.out_pc_callback().set(FUNC(bw2_state::ppi_pc_w));

	MSM6255(config, m_lcdc, 16_MHz_XTAL);
	m_lcdc->set_addrmap(0, &bw2_state::lcdc_map);
	m_lcdc->set_screen(SCREEN_TAG);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(bw2_state::write_centronics_busy));

	output_latch_device &latch(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(latch);

	I8251(config, m_uart, 0);
	m_uart->txd_handler().set(RS232_TAG, FUNC(rs232_port_device::write_txd));
	m_uart->dtr_handler().set(RS232_TAG, FUNC(rs232_port_device::write_dtr));
	m_uart->rts_handler().set(RS232_TAG, FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, RS232_TAG, default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_uart, FUNC(i8251_device::write_rxd));
	rs232.dsr_handler().set(m_uart, FUNC(i8251_device::write_dsr));

	WD2797(config, m_fdc, 16_MHz_XTAL / 16);
	m_fdc->intrq_wr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_fdc->drq_wr_callback().set(FUNC(bw2_state::fdc_drq_w));

	FLOPPY_CONNECTOR(config, WD2797_TAG":0", bw2_floppies, "35dd", bw2_state::floppy_formats);
	FLOPPY_CONNECTOR(config, WD2797_TAG":1", bw2_floppies, nullptr, bw2_state::floppy_formats);
	BW2_EXPANSION_SLOT(config, m_exp, 16_MHz_XTAL, bw2_expansion_cards, nullptr);

	// software list
	SOFTWARE_LIST(config, "flop_list").set_original("bw2");

	// internal ram
	RAM(config, RAM_TAG).set_default_size("64K").set_extra_options("96K,128K,160K,192K,224K");
}



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( bw2 )
//-------------------------------------------------

ROM_START( bw2 )
	ROM_REGION( 0x1000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS( "v20" )
	ROM_SYSTEM_BIOS( 0, "v12", "BW 2 v1.2" )
	ROMX_LOAD( "bw2-12.ic8", 0x0000, 0x1000, CRC(0ab42d10) SHA1(430b232631eee9b715151b8d191b7eb9449ac513), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v20", "BW 2 v2.0" )
	ROMX_LOAD( "bw2-20.ic8", 0x0000, 0x1000, CRC(86f36471) SHA1(a3e2ba4edd50ff8424bb0675bdbb3b9f13c04c9d), ROM_BIOS(1) )
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT   CLASS      INIT        COMPANY             FULLNAME            FLAGS
COMP( 1985, bw2,  0,      0,      bw2,     bw2,    bw2_state, empty_init, "Bondwell Holding", "Bondwell Model 2", MACHINE_NO_SOUND_HW )

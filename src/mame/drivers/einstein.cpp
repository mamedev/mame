// license:GPL-2.0+
// copyright-holders:Kevin Thacker, Dirk Best, Phill Harvey-Smith
/******************************************************************************

    Tatung Einstein

    TMS9129 VDP Graphics
        16k ram

    Z80 CPU (4 MHz)

    Z80 CTC (4 MHz)
        channel 0 is serial transmit clock
        channel 1 is serial receive clock
        trigger for channel 0,1 and 2 is a 2 MHz clock
        trigger for channel 3 is the terminal count of channel 2

    Intel 8251 Serial (2 MHz clock?)

    WD1770 Floppy Disc controller
        density is fixed, 4 drives and double sided supported

    AY-3-8910 PSG (2 MHz)
        port A and port B are connected to the keyboard. Port A is keyboard
        line select, Port B is data.

    printer connected to port A of PIO. /ACK from printer is connected to /ASTB.
    D7-D0 of PIO port A is printer data lines.
    ARDY of PIO is connected to /STROBE on printer.

    user port is port B of PIO
    keyboard connected to port A and port B of PSG

    TODO:
    - The ADC is not emulated!
    - printer emulation needs checking!

    Many thanks to Chris Coxall for the schematics of the TC-01, the dump of the
    system rom and a dump of a Xtal boot disc.

    Many thanks to Andrew Dunipace for his help with the 80-column card
    and Speculator hardware (Spectrum emulator).

    Kevin Thacker [MESS driver]


    2011-Mar-14, Phill Harvey-Smith.
        Having traced out the circuit of the TK02 80 coumn card, I have changed the
        emulation to match what the hardware does, the emulation was mostly correct,
        just some minor issues with the addressing of the VRAM, and bit 0 of the
        status register is the latched output of the 6845 DE, and not vblank.

        Also added defines to stop the log being flooded with keyboard messages :)

 ******************************************************************************/

#include "emu.h"
#include "includes/einstein.h"

#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "bus/einstein/userport/userport.h"
#include "machine/clock.h"
#include "machine/z80pio.h"
#include "sound/ay8910.h"
#include "softlist.h"
#include "speaker.h"


#define VERBOSE_KEYBOARD    0
#define VERBOSE_DISK        0


/***************************************************************************
    KEYBOARD
***************************************************************************/

/* refresh keyboard data. It is refreshed when the keyboard line is written */
void einstein_state::einstein_scan_keyboard()
{
	uint8_t data = 0xff;

	if (!BIT(m_keyboard_line, 0)) data &= m_line[0]->read();
	if (!BIT(m_keyboard_line, 1)) data &= m_line[1]->read();
	if (!BIT(m_keyboard_line, 2)) data &= m_line[2]->read();
	if (!BIT(m_keyboard_line, 3)) data &= m_line[3]->read();
	if (!BIT(m_keyboard_line, 4)) data &= m_line[4]->read();
	if (!BIT(m_keyboard_line, 5)) data &= m_line[5]->read();
	if (!BIT(m_keyboard_line, 6)) data &= m_line[6]->read();
	if (!BIT(m_keyboard_line, 7)) data &= m_line[7]->read();

	m_keyboard_data = data;
}

TIMER_DEVICE_CALLBACK_MEMBER(einstein_state::einstein_keyboard_timer_callback)
{
	/* re-scan keyboard */
	einstein_scan_keyboard();

	/* if /fire1 or /fire2 is 0, signal a fire interrupt */
	if ((m_buttons->read() & 0x03) != 0)
	{
		m_interrupt |= EINSTEIN_FIRE_INT;
	}

	/* keyboard data changed? */
	if (m_keyboard_data != 0xff)
	{
		/* generate interrupt */
		m_interrupt |= EINSTEIN_KEY_INT;
	}
}

WRITE8_MEMBER(einstein_state::einstein_keyboard_line_write)
{
	if (VERBOSE_KEYBOARD)
		logerror("einstein_keyboard_line_write: %02x\n", data);

	m_keyboard_line = data;

	/* re-scan the keyboard */
	einstein_scan_keyboard();
}

READ8_MEMBER(einstein_state::einstein_keyboard_data_read)
{
	/* re-scan the keyboard */
	einstein_scan_keyboard();

	if (VERBOSE_KEYBOARD)
		logerror("einstein_keyboard_data_read: %02x\n", m_keyboard_data);

	return m_keyboard_data;
}


/***************************************************************************
    FLOPPY DRIVES
***************************************************************************/

WRITE8_MEMBER(einstein_state::einstein_drsel_w)
{
	if (VERBOSE_DISK)
		logerror("%s: einstein_drsel_w %02x\n", machine().describe_context(), data);

	floppy_image_device *floppy = nullptr;

	if (BIT(data, 0)) floppy = m_floppy[0]->get_device();
	if (BIT(data, 1)) floppy = m_floppy[1]->get_device();
	if (BIT(data, 2)) floppy = m_floppy[2]->get_device();
	if (BIT(data, 3)) floppy = m_floppy[3]->get_device();

	if (floppy)
		floppy->ss_w(BIT(data, 4));

	m_fdc->set_floppy(floppy);
}


/***************************************************************************
    UART
***************************************************************************/

WRITE_LINE_MEMBER(einstein_state::einstein_serial_transmit_clock)
{
	m_uart->write_txc(state);
}

WRITE_LINE_MEMBER(einstein_state::einstein_serial_receive_clock)
{
	m_uart->write_rxc(state);
}


/***************************************************************************
    MEMORY BANKING
***************************************************************************/

void einstein_state::einstein_page_rom()
{
	m_bank1->set_base(m_rom_enabled ? m_region_bios->base() : m_ram->pointer());
}

/* writing to this port is a simple trigger, and switches between RAM and ROM */
WRITE8_MEMBER(einstein_state::einstein_rom_w)
{
	m_rom_enabled ^= 1;
	einstein_page_rom();
}


/***************************************************************************
    INTERRUPTS
***************************************************************************/

/* int priority */
/* keyboard int->ctc/adc->pio */
static const z80_daisy_config einstein_daisy_chain[] =
{
	{ "keyboard_daisy" },
	{ IC_I058 },
	{ "adc_daisy" },
	{ IC_I063 },
	{ "fire_daisy" },
	{ nullptr }
};

WRITE_LINE_MEMBER(einstein_state::write_centronics_busy)
{
	m_centronics_busy = state;
}

WRITE_LINE_MEMBER(einstein_state::write_centronics_perror)
{
	m_centronics_perror = state;
}

WRITE_LINE_MEMBER(einstein_state::write_centronics_fault)
{
	m_centronics_fault = state;
}

READ8_MEMBER(einstein_state::einstein_kybintmsk_r)
{
	uint8_t data = 0;

	/* clear key int. a read of this I/O port will do this or a reset */
	m_interrupt &= ~EINSTEIN_KEY_INT;

	/* bit 0 and 1: fire buttons on the joysticks */
	data |= m_buttons->read();

	/* bit 2 to 4: printer status */
	data |= m_centronics_busy << 2;
	data |= m_centronics_perror << 3;
	data |= m_centronics_fault << 4;

	/* bit 5 to 7: graph, control and shift key */
	data |= m_extra->read();

	if(VERBOSE_KEYBOARD)
		logerror("%s: einstein_kybintmsk_r %02x\n", machine().describe_context(), data);

	return data;
}

WRITE8_MEMBER(einstein_state::einstein_kybintmsk_w)
{
	logerror("%s: einstein_kybintmsk_w %02x\n", machine().describe_context(), data);

	/* set mask from bit 0 */
	if (data & 0x01)
	{
		logerror("key int is disabled\n");
		m_interrupt_mask &= ~EINSTEIN_KEY_INT;
	}
	else
	{
		logerror("key int is enabled\n");
		m_interrupt_mask |= EINSTEIN_KEY_INT;
	}
}

/* writing to this I/O port sets the state of the mask; D0 is used */
/* writing 0 enables the /ADC interrupt */
WRITE8_MEMBER(einstein_state::einstein_adcintmsk_w)
{
	logerror("%s: einstein_adcintmsk_w %02x\n", machine().describe_context(), data);

	if (data & 0x01)
	{
		logerror("adc int is disabled\n");
		m_interrupt_mask &= ~EINSTEIN_ADC_INT;
	}
	else
	{
		logerror("adc int is enabled\n");
		m_interrupt_mask |= EINSTEIN_ADC_INT;
	}
}

/* writing to this I/O port sets the state of the mask; D0 is used */
/* writing 0 enables the /FIRE interrupt */
WRITE8_MEMBER(einstein_state::einstein_fire_int_w)
{
	logerror("%s: einstein_fire_int_w %02x\n", machine().describe_context(), data);

	if (data & 0x01)
	{
		logerror("fire int is disabled\n");
		m_interrupt_mask &= ~EINSTEIN_FIRE_INT;
	}
	else
	{
		logerror("fire int is enabled\n");
		m_interrupt_mask |= EINSTEIN_FIRE_INT;
	}
}


/***************************************************************************
    MACHINE EMULATION
***************************************************************************/

void einstein_state::machine_start()
{
	// setup expansion slot
	m_pipe->set_program_space(&m_maincpu->space(AS_PROGRAM));
	m_pipe->set_io_space(&m_maincpu->space(AS_IO));
}

void einstein_state::machine_reset()
{
	/* initialize memory mapping */
	m_bank2->set_base(m_ram->pointer());
	m_bank3->set_base(m_ram->pointer() + 0x8000);
	m_rom_enabled = 1;
	einstein_page_rom();

	/* a reset causes the fire int, adc int, keyboard int mask
	to be set to 1, which causes all these to be DISABLED */
	m_interrupt = 0;
	m_interrupt_mask = 0;
}


/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

static ADDRESS_MAP_START( einstein_mem, AS_PROGRAM, 8, einstein_state )
	AM_RANGE(0x0000, 0x07fff) AM_READ_BANK("bank1") AM_WRITE_BANK("bank2")
	AM_RANGE(0x8000, 0x0ffff) AM_RAMBANK("bank3")
ADDRESS_MAP_END

/* The I/O ports are decoded into 8 blocks using address lines A3 to A7 */
static ADDRESS_MAP_START( einstein_io, AS_IO, 8, einstein_state )
	ADDRESS_MAP_UNMAP_HIGH
	/* block 0, ay8910 psg */
	AM_RANGE(0x02, 0x02) AM_MIRROR(0xff04) AM_DEVREADWRITE(IC_I030, ay8910_device, data_r, address_w)
	AM_RANGE(0x03, 0x03) AM_MIRROR(0xff04) AM_DEVWRITE(IC_I030, ay8910_device, data_w)
	/* block 1, tms9928a vdp */
	AM_RANGE(0x08, 0x08) AM_MIRROR(0xff06) AM_DEVREADWRITE("tms9929a", tms9929a_device, vram_read, vram_write)
	AM_RANGE(0x09, 0x09) AM_MIRROR(0xff06) AM_DEVREADWRITE("tms9929a", tms9929a_device, register_read, register_write)
	/* block 2, i8251 uart */
	AM_RANGE(0x10, 0x10) AM_MIRROR(0xff06) AM_DEVREADWRITE(IC_I060, i8251_device, data_r, data_w)
	AM_RANGE(0x11, 0x11) AM_MIRROR(0xff06) AM_DEVREADWRITE(IC_I060, i8251_device, status_r, control_w)
	/* block 3, wd1770 floppy controller */
	AM_RANGE(0x18, 0x1b) AM_MIRROR(0xff04) AM_DEVREADWRITE(IC_I042, wd1770_device, read, write)
	/* block 4, internal controls */
	AM_RANGE(0x20, 0x20) AM_MIRROR(0xff00) AM_READWRITE(einstein_kybintmsk_r, einstein_kybintmsk_w)
	AM_RANGE(0x21, 0x21) AM_MIRROR(0xff00) AM_WRITE(einstein_adcintmsk_w)
	AM_RANGE(0x23, 0x23) AM_MIRROR(0xff00) AM_WRITE(einstein_drsel_w)
	AM_RANGE(0x24, 0x24) AM_MIRROR(0xff00) AM_WRITE(einstein_rom_w)
	AM_RANGE(0x25, 0x25) AM_MIRROR(0xff00) AM_WRITE(einstein_fire_int_w)
	/* block 5, z80ctc */
	AM_RANGE(0x28, 0x2b) AM_MIRROR(0xff04) AM_DEVREADWRITE(IC_I058, z80ctc_device, read, write)
	/* block 6, z80pio */
	AM_RANGE(0x30, 0x33) AM_MIRROR(0xff04) AM_DEVREADWRITE(IC_I063, z80pio_device, read_alt, write_alt)
#if 0
	/* block 7, adc */
	AM_RANGE(0x38, 0x38) AM_MIRROR(0xff07) AM_DEVREADWRITE_LEGACY(IC_I050, adc0844_r, adc0844_w)
#endif
ADDRESS_MAP_END


/***************************************************************************
    INPUT PORTS
***************************************************************************/

static INPUT_PORTS_START( einstein )
	PORT_START("LINE0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BREAK") PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F0") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LEFT") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR(0xA3)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(0xBA) PORT_CHAR(0xBD)    // is \xBA correct for double vertical bar || ?
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('@')

	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F5))

	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DELETE") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('-')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("UP") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F4))

	PORT_START("LINE4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F3))

	PORT_START("LINE5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F2))

	PORT_START("LINE6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F1))

	PORT_START("LINE7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F6") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F6))

	PORT_START("EXTRA")
	PORT_BIT(0x1f, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("GRPH")    PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CONTROL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT")   PORT_CODE(KEYCODE_LSHIFT)   PORT_CODE(KEYCODE_RSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)

	/* fire buttons for analogue joysticks */
	PORT_START("BUTTONS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Joystick 1 Button 1") PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Joystick 2 Button 1") PORT_PLAYER(2)
	PORT_BIT(0xfc, IP_ACTIVE_HIGH, IPT_UNUSED)

	/* analog joystick 1 x axis */
	PORT_START("JOY1_X")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_MINMAX(1,0xff) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(1) PORT_REVERSE

	/* analog joystick 1 y axis */
	PORT_START("JOY1_Y")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_MINMAX(1,0xff) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH) PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH) PORT_PLAYER(1) PORT_REVERSE

	/* analog joystick 2 x axis */
	PORT_START("JOY2_X")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_MINMAX(1,0xff) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(2) PORT_REVERSE

	/* analog joystick 2 Y axis */
	PORT_START("JOY2_Y")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_MINMAX(1,0xff) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH) PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH) PORT_PLAYER(2) PORT_REVERSE
INPUT_PORTS_END


/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

static SLOT_INTERFACE_START( einstein_floppies )
	SLOT_INTERFACE("3ss", TEAC_FD_30A)
	SLOT_INTERFACE("3ds", FLOPPY_3_DSDD)
	SLOT_INTERFACE("525ssqd", FLOPPY_525_SSQD)
	SLOT_INTERFACE("525qd", FLOPPY_525_QD)
	SLOT_INTERFACE("35ssdd", FLOPPY_35_SSDD)
	SLOT_INTERFACE("35dd", FLOPPY_35_DD)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( einstein )
	/* basic machine hardware */
	MCFG_CPU_ADD(IC_I001, Z80, XTAL_X002 / 2)
	MCFG_CPU_PROGRAM_MAP(einstein_mem)
	MCFG_CPU_IO_MAP(einstein_io)
	MCFG_Z80_DAISY_CHAIN(einstein_daisy_chain)

	/* this is actually clocked at the system clock 4 MHz, but this would be too fast for our
	driver. So we update at 50Hz and hope this is good enough. */
	MCFG_TIMER_DRIVER_ADD_PERIODIC("keyboard", einstein_state, einstein_keyboard_timer_callback, attotime::from_hz(50))

	MCFG_DEVICE_ADD(IC_I063, Z80PIO, XTAL_X002 / 2)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE(IC_I001, INPUT_LINE_IRQ0))
	MCFG_Z80PIO_OUT_PA_CB(DEVWRITE8("cent_data_out", output_latch_device, write))
	MCFG_Z80PIO_OUT_ARDY_CB(DEVWRITELINE("centronics", centronics_device, write_strobe))
	MCFG_Z80PIO_IN_PB_CB(DEVREAD8("user", einstein_userport_device, read))
	MCFG_Z80PIO_OUT_PB_CB(DEVWRITE8("user", einstein_userport_device, write))
	MCFG_Z80PIO_OUT_BRDY_CB(DEVWRITELINE("user", einstein_userport_device, brdy_w))

	MCFG_DEVICE_ADD(IC_I058, Z80CTC, XTAL_X002 / 2)
	MCFG_Z80CTC_INTR_CB(INPUTLINE(IC_I001, INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(WRITELINE(einstein_state, einstein_serial_transmit_clock))
	MCFG_Z80CTC_ZC1_CB(WRITELINE(einstein_state, einstein_serial_receive_clock))
	MCFG_Z80CTC_ZC2_CB(DEVWRITELINE(IC_I058, z80ctc_device, trg3))

	MCFG_CLOCK_ADD("ctc_trigger", XTAL_X002 / 4)
	MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE(IC_I058, z80ctc_device, trg0))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE(IC_I058, z80ctc_device, trg1))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE(IC_I058, z80ctc_device, trg2))

	/* Einstein daisy chain support for non-Z80 devices */
	MCFG_DEVICE_ADD("keyboard_daisy", EINSTEIN_KEYBOARD_DAISY, 0)
	MCFG_DEVICE_ADD("adc_daisy", EINSTEIN_ADC_DAISY, 0)
	MCFG_DEVICE_ADD("fire_daisy", EINSTEIN_FIRE_DAISY, 0)

	/* video hardware */
	MCFG_DEVICE_ADD( "tms9929a", TMS9929A, XTAL_10_738635MHz / 2 )
	MCFG_TMS9928A_VRAM_SIZE(0x4000) /* 16k RAM, provided by IC i040 and i041 */
	MCFG_TMS9928A_SET_SCREEN("screen")
	MCFG_TMS9928A_SCREEN_ADD_PAL("screen")
	MCFG_SCREEN_UPDATE_DEVICE("tms9929a", tms9929a_device, screen_update)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(IC_I030, AY8910, XTAL_X002 / 4)
	MCFG_AY8910_PORT_B_READ_CB(READ8(einstein_state, einstein_keyboard_data_read))
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(einstein_state, einstein_keyboard_line_write))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	/* printer */
	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER(DEVWRITELINE(IC_I063, z80pio_device, strobe_a))
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(einstein_state, write_centronics_busy))
	MCFG_CENTRONICS_PERROR_HANDLER(WRITELINE(einstein_state, write_centronics_perror))
	MCFG_CENTRONICS_FAULT_HANDLER(WRITELINE(einstein_state, write_centronics_fault))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")

	/* uart */
	MCFG_DEVICE_ADD(IC_I060, I8251, 0)

	MCFG_WD1770_ADD(IC_I042, XTAL_X002)

	MCFG_FLOPPY_DRIVE_ADD(IC_I042 ":0", einstein_floppies, "3ss", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(IC_I042 ":1", einstein_floppies, "3ss", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(IC_I042 ":2", einstein_floppies, "525qd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(IC_I042 ":3", einstein_floppies, "525qd", floppy_image_device::default_floppy_formats)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("disk_list","einstein")

	/* RAM is provided by 8k DRAM ICs i009, i010, i011, i012, i013, i014, i015 and i016 */
	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")

	// tatung pipe connector
	MCFG_TATUNG_PIPE_ADD("pipe")

	// user port
	MCFG_EINSTEIN_USERPORT_ADD("user")
	MCFG_EINSTEIN_USERPORT_BSTB_HANDLER(DEVWRITELINE(IC_I063, z80pio_device, strobe_b))
MACHINE_CONFIG_END


/***************************************************************************
    ROM DEFINITIONS
***************************************************************************/

/* There are two sockets, i023 and i024, each either a 2764 or 27128
 * only i023 is used by default and fitted with the 8k bios (called MOS).
 *
 * We are missing dumps of version MOS 1.1, possibly of 1.0 if it exists.
 */
ROM_START( einstein )
	ROM_REGION(0x8000, "bios", 0)
	/* i023 */
	ROM_SYSTEM_BIOS(0,  "mos12",  "MOS 1.2")
	ROMX_LOAD("mos12.i023", 0, 0x2000, CRC(ec134953) SHA1(a02125d8ebcda48aa784adbb42a8b2d7ef3a4b77), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1,  "mos121",  "MOS 1.21")
	ROMX_LOAD("mos121.i023", 0, 0x2000, CRC(a746eeb6) SHA1(f75aaaa777d0fd92225acba291f6bf428b341d3e), ROM_BIOS(2))
	ROM_RELOAD(0x2000, 0x2000)
	/* i024 */
	ROM_FILL(0x4000, 0x4000, 0xff)
ROM_END

ROM_START( einst256 )
	ROM_REGION(0x8000, "bios", 0)
	ROM_LOAD("tc256.rom", 0x0000, 0x4000, CRC(ef8dad88) SHA1(eb2102d3bef572db7161c26a7c68a5fcf457b4d0) )
ROM_END


/***************************************************************************
    GAME DRIVERS
***************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     STATE           INIT  COMPANY   FULLNAME          FLAGS
COMP( 1984, einstein, 0,      0,      einstein, einstein, einstein_state, 0,    "Tatung", "Einstein TC-01", 0 )
COMP( 1984, einst256, 0,      0,      einstein, einstein, einstein_state, 0,    "Tatung", "Einstein 256",   MACHINE_NOT_WORKING )

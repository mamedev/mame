// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    Philips VG-5000mu

    Driver by Sandro Ronco with help from Daniel Coulom

    05/2010 (Sandro Ronco)
     - EF9345 video controller
     - keyboard input ports
    05/2009 Skeleton driver.

    Known issues:
     - Support the K7 filetype for ease of usage, but as read only.

    Information ( see the very informative http://vg5k.free.fr/ ):
     - Variants: Radiola VG5000 and Schneider VG5000
     - CPU: Zilog Z80 running at 4MHz
     - ROM: 18KB (16 KB BASIC + 2 KB charset )
     - RAM: 24 KB
     - Video: SGS Thomson EF9345 processor
            - Text mode: 25 rows x 40 columns
            - Character matrix: 8 x 10
            - ASCII characters set, 128 graphics mode characters, 192 user characters.
            - Graphics mode: not available within basic, only semi graphic is available.
            - Colors: 8
     - Sound: Synthesizer, 4 Octaves
     - Keyboard: 63 keys AZERTY, Caps Lock, CTRL key to access 33 BASIC instructions
     - I/O: Tape recorder connector (1200/2400 bauds), SCART connector to TV (RGB),
       External PSU (VU0022) connector, Bus connector (2x25 pins)
     - There are 2 versions of the VG5000 ROM, one with Basic v1.0,
       contained in two 8 KB ROMs, and one with Basic 1.1, contained in
       a single 16 KB ROM.
     - RAM: 24 KB (3 x 8 KB) type SRAM D4168C, more precisely:
         2 x 8 KB, used by the system
         1 x 8 KB, used by the video processor
     - Memory Map:
         $0000 - $3fff  BASIC + monitor
         $4000 - $47cf  Screen
         $47d0 - $7fff  reserved area for BASIC, variables, etc
         $8000 - $bfff  Memory Expansion 16K or ROM cart
         $c000 - $ffff  Memory Expansion 32K or ROM cart
     - This computer was NOT MSX-compatible!


****************************************************************************/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "imagedev/printer.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "video/ef9345.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "formats/vg5k_cas.h"


namespace {

class vg5k_state : public driver_device
{
public:
	vg5k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ef9345(*this, "ef9345")
		, m_dac(*this, "dac")
		, m_printer(*this, "printer")
		, m_cassette(*this, "cassette")
		, m_ram(*this, RAM_TAG)
	{ }

	void vg5k(machine_config &config);

	void init_vg5k();

	DECLARE_INPUT_CHANGED_MEMBER(delta_button);

private:
	required_device<z80_device> m_maincpu;
	required_device<ef9345_device> m_ef9345;
	required_device<dac_bit_interface> m_dac;
	required_device<printer_image_device> m_printer;
	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;

	offs_t m_ef9345_offset = 0;
	uint8_t m_printer_latch = 0;
	uint8_t m_printer_signal = 0;
	emu_timer *m_z80_irq_clear_timer = nullptr;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void z80_m1_w(uint8_t data);
	uint8_t printer_state_r();
	void printer_state_w(uint8_t data);
	void printer_data_w(uint8_t data);
	void ef9345_offset_w(uint8_t data);
	uint8_t ef9345_io_r();
	void ef9345_io_w(uint8_t data);
	uint8_t cassette_r();
	void cassette_w(uint8_t data);
	TIMER_CALLBACK_MEMBER(z80_irq_clear);
	TIMER_DEVICE_CALLBACK_MEMBER(z80_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(vg5k_scanline);
	void vg5k_io(address_map &map) ATTR_COLD;
	void vg5k_mem(address_map &map) ATTR_COLD;
};

void vg5k_state::z80_m1_w(uint8_t data)
{
	// Leverage the refresh callback of the Z80 emulator to pretend
	// the second T state of the M1 cycle didn't happen.
	// This simulates the WAIT line asserted at that moment, as
	// the current implementation of the Z80 doesn't handle the WAIT
	// line at that moment.
	m_maincpu->adjust_icount(-1);
}

uint8_t vg5k_state::printer_state_r()
{
	return (m_printer->is_ready() ? 0x00 : 0xff);
}

void vg5k_state::printer_state_w(uint8_t data)
{
	// Character is emitted on a rising edge.
	if (!BIT(m_printer_signal, 0) && BIT(data, 0)) {
		m_printer->output(m_printer_latch);
	}
	m_printer_signal = data;
}


void vg5k_state::printer_data_w(uint8_t data)
{
	m_printer_latch = data;
}


void vg5k_state::ef9345_offset_w(uint8_t data)
{
	m_ef9345_offset = data;
}


uint8_t vg5k_state::ef9345_io_r()
{
	return m_ef9345->data_r(m_ef9345_offset);
}


void vg5k_state::ef9345_io_w(uint8_t data)
{
	m_ef9345->data_w(m_ef9345_offset, data);
}


uint8_t vg5k_state::cassette_r()
{
	double level = m_cassette->input();

	return (level > 0.03) ? 0xff : 0x00;
}


void vg5k_state::cassette_w(uint8_t data)
{
	m_dac->write(BIT(data, 3));
	m_cassette->change_state(BIT(data, 1) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED , CASSETTE_MASK_MOTOR);

	if (BIT(data, 1)) {
		if (BIT(data, 0)) {
			m_cassette->output(+1);
		} else {
			m_cassette->output(-1);
		}
	} else {
		m_cassette->output(0);
	}
}


void vg5k_state::vg5k_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).ram();
	map(0x8000, 0xffff).noprw(); /* messram expansion memory */
}

void vg5k_state::vg5k_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);

	/* joystick */
	map(0x07, 0x07).portr("JOY0");
	map(0x08, 0x08).portr("JOY1");

	/* printer */
	map(0x10, 0x10).rw(FUNC(vg5k_state::printer_state_r), FUNC(vg5k_state::printer_state_w));
	map(0x11, 0x11).w(FUNC(vg5k_state::printer_data_w));

	/* keyboard */
	map(0x80, 0x80).portr("ROW1");
	map(0x81, 0x81).portr("ROW2");
	map(0x82, 0x82).portr("ROW3");
	map(0x83, 0x83).portr("ROW4");
	map(0x84, 0x84).portr("ROW5");
	map(0x85, 0x85).portr("ROW6");
	map(0x86, 0x86).portr("ROW7");
	map(0x87, 0x87).portr("ROW8");

	/* EF9345 */
	map(0x8f, 0x8f).w(FUNC(vg5k_state::ef9345_offset_w));
	map(0xcf, 0xcf).rw(FUNC(vg5k_state::ef9345_io_r), FUNC(vg5k_state::ef9345_io_w));

	/* cassette */
	map(0xaf, 0xaf).rw(FUNC(vg5k_state::cassette_r), FUNC(vg5k_state::cassette_w));
}

/* Input ports */
static INPUT_PORTS_START( vg5k )
	PORT_START("ROW1")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_HOME)                                   PORT_NAME("LIST EFFE")      PORT_CHAR(UCHAR_MAMEKEY(F1))
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)       PORT_NAME("MAJ")            PORT_CHAR(UCHAR_SHIFT_1)
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LEFT)                                                               PORT_CHAR(UCHAR_MAMEKEY(LEFT))
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RIGHT)                                                              PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DOWN)                                                               PORT_CHAR(UCHAR_MAMEKEY(DOWN))
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)   PORT_NAME("CTRL ACCENT")    PORT_CHAR(UCHAR_SHIFT_2)
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_INSERT)                                 PORT_NAME("INSC INSL")      PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_START("ROW2")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2)                                     PORT_NAME("RUN STOP")       PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CHAR(UCHAR_MAMEKEY(PAUSE))
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)                                                                  PORT_CHAR('q') PORT_CHAR('Q')
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)                                  PORT_NAME("ESPACE")         PORT_CHAR(' ')
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CAPSLOCK)                               PORT_NAME("LCK")            PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER)                                  PORT_NAME("RET")            PORT_CHAR(13)
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_UP)                                                                 PORT_CHAR(UCHAR_MAMEKEY(UP))
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)                                                                  PORT_CHAR('a') PORT_CHAR('A')
	PORT_START("ROW3")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)                                                                  PORT_CHAR('w') PORT_CHAR('W')
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)                                                                  PORT_CHAR('x') PORT_CHAR('X')
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)                                                                  PORT_CHAR('c') PORT_CHAR('C')
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)                                                                  PORT_CHAR('v') PORT_CHAR('V')
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)                                                                  PORT_CHAR('b') PORT_CHAR('B')
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)                                                                  PORT_CHAR('1') PORT_CHAR('#') PORT_CHAR(0xe2)   // â
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)                                                                  PORT_CHAR(':') PORT_CHAR('*')
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)                                                                  PORT_CHAR('z') PORT_CHAR('Z')
	PORT_START("ROW4")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)                                                                  PORT_CHAR('s') PORT_CHAR('S')
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)                                                                  PORT_CHAR('e') PORT_CHAR('E')
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)                                                                  PORT_CHAR('3') PORT_CHAR('"') PORT_CHAR(0xea)   // ê
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)                                                                  PORT_CHAR('4') PORT_CHAR(0xa3) PORT_CHAR(0xef)  // £ ï
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)                                                                  PORT_CHAR('5') PORT_CHAR('$') PORT_CHAR(0xf9)   // ù
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)                                                                  PORT_CHAR('6') PORT_CHAR('%') PORT_CHAR(0xf4)   // ô
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)                                                                  PORT_CHAR('2') PORT_CHAR('!') PORT_CHAR(0xe9)   // é
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)                                                               PORT_CHAR(';') PORT_CHAR('@')
	PORT_START("ROW5")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB)                                                                PORT_CHAR(0xf7) PORT_CHAR('_')                  // ÷
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)                                                              PORT_CHAR(0xd7) PORT_CHAR('|')                  // ×
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)                                                                  PORT_CHAR('f') PORT_CHAR('F')
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)                                                                  PORT_CHAR('g') PORT_CHAR('G')
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)                                                                  PORT_CHAR('u') PORT_CHAR('U')
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)                                                                  PORT_CHAR('i') PORT_CHAR('I')
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)                                                                  PORT_CHAR('o') PORT_CHAR('O')
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)                                                                  PORT_CHAR('p') PORT_CHAR('P')
	PORT_START("ROW6")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)                                                                  PORT_CHAR('0') PORT_CHAR(')') PORT_CHAR(0xe0)   // à
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)                                                              PORT_CHAR(']') PORT_CHAR('[')
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)                                                              PORT_CHAR(0x2026) PORT_CHAR(0x3c0)              // … π
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)                                                              PORT_CHAR(',') PORT_CHAR('/')
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)                                                                  PORT_CHAR('7') PORT_CHAR('&') PORT_CHAR(0xe8)   // è
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)                                                                  PORT_CHAR('8') PORT_CHAR('\'') PORT_CHAR(0xfb)  // û
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)                                                                  PORT_CHAR('9') PORT_CHAR('(') PORT_CHAR(0xe7)   // ç
	PORT_START("ROW7")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE)                                                              PORT_CHAR('-') PORT_CHAR('?')
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH)                                                          PORT_CHAR('+') PORT_CHAR('.')
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)                                                                  PORT_CHAR('r') PORT_CHAR('R')
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)                                                                  PORT_CHAR('t') PORT_CHAR('T')
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)                                                                  PORT_CHAR('y') PORT_CHAR('Y')
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE)                                                          PORT_CHAR('<') PORT_CHAR('>')
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE)                             PORT_NAME("PRT")            PORT_CHAR(UCHAR_MAMEKEY(PRTSCR))
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)                                                                  PORT_CHAR('d') PORT_CHAR('D')
	PORT_START("ROW8")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)                                                             PORT_CHAR('=') PORT_CHAR('^')
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSPACE)                              PORT_NAME("EFFC EFFL")      PORT_CHAR(8) PORT_CHAR(UCHAR_MAMEKEY(DEL))
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)                                                                  PORT_CHAR('n') PORT_CHAR('N')
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)                                                                  PORT_CHAR('j') PORT_CHAR('J')
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)                                                                  PORT_CHAR('h') PORT_CHAR('H')
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)                                                                  PORT_CHAR('k') PORT_CHAR('K')
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)                                                                  PORT_CHAR('l') PORT_CHAR('L')
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)                                                              PORT_CHAR('m') PORT_CHAR('M')
	PORT_START("JOY0")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1)
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(1)
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("JOY1")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2)
		PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
		PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2)
		PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2)
		PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(2)
		PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
		PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
		PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("direct")
		PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)        PORT_CODE(KEYCODE_END)                              PORT_NAME("DELTA")          PORT_CHANGED_MEMBER(DEVICE_SELF, vg5k_state, delta_button, 0)
INPUT_PORTS_END


TIMER_CALLBACK_MEMBER(vg5k_state::z80_irq_clear)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}


TIMER_DEVICE_CALLBACK_MEMBER(vg5k_state::z80_irq)
{
	m_maincpu->set_input_line(0, ASSERT_LINE);

	m_z80_irq_clear_timer->adjust(attotime::from_usec(100));
}

TIMER_DEVICE_CALLBACK_MEMBER(vg5k_state::vg5k_scanline)
{
	m_ef9345->update_scanline((uint16_t)param);
}

INPUT_CHANGED_MEMBER(vg5k_state::delta_button)
{
	// The yellow Delta key on the keyboard is wired so that it asserts directly the NMI line of the Z80.
	if (!newval) {
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}
}


void vg5k_state::machine_start()
{
	save_item(NAME(m_ef9345_offset));
	save_item(NAME(m_printer_latch));
	save_item(NAME(m_printer_signal));

	m_z80_irq_clear_timer = timer_alloc(FUNC(vg5k_state::z80_irq_clear), this);
}

void vg5k_state::machine_reset()
{
	m_ef9345_offset = 0;
	m_printer_latch = 0;
	m_printer_signal = 0;
}


/* F4 Character Displayer */
static const gfx_layout vg5k_charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	/* y offsets */
	{ 0, 8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_vg5k )
	GFXDECODE_ENTRY( "ef9345", 0x2000, vg5k_charlayout, 0, 4 )
GFXDECODE_END

void vg5k_state::init_vg5k()
{
	uint8_t *FNT = memregion("ef9345")->base();
	uint16_t dest = 0x2000;

	/* Unscramble the chargen rom as the format is too complex for gfxdecode to handle unaided */
	for (uint16_t a = 0; a < 8192; a+=4096)
		for (uint16_t b = 0; b < 2048; b+=64)
			for (uint16_t c = 0; c < 4; c++)
				for (uint16_t d = 0; d < 64; d+=4)
					FNT[dest++]=FNT[a|b|c|d];


	/* install expansion memory*/
	address_space &program = m_maincpu->space(AS_PROGRAM);
	uint8_t *ram = m_ram->pointer();
	uint16_t ram_size = m_ram->size();

	if (ram_size > 0x4000)
		program.install_ram(0x8000, 0x3fff + ram_size, ram);
}


void vg5k_state::vg5k(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &vg5k_state::vg5k_mem);
	m_maincpu->set_addrmap(AS_IO, &vg5k_state::vg5k_io);
	m_maincpu->refresh_cb().set(FUNC(vg5k_state::z80_m1_w));

	TIMER(config, "vg5k_scanline").configure_scanline(FUNC(vg5k_state::vg5k_scanline), "screen", 0, 10);

	TIMER(config, "irq_timer").configure_periodic(FUNC(vg5k_state::z80_irq), attotime::from_msec(20));

	EF9345(config, m_ef9345, 0);
	m_ef9345->set_palette_tag("palette");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("ef9345", FUNC(ef9345_device::screen_update));
	screen.set_size(336, 300);
	screen.set_visarea(00, 336-1, 00, 270-1);

	GFXDECODE(config, "gfxdecode", "palette", gfx_vg5k);
	PALETTE(config, "palette").set_entries(8);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.125);

	/* cassette */
	CASSETTE(config, m_cassette);
	m_cassette->set_formats(vg5k_cassette_formats);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(0, "speaker", 0.05);
	m_cassette->set_interface("vg5k_cass");

	/* printer */
	PRINTER(config, m_printer, 0);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("16K").set_extra_options("32K,48K");

	/* Software lists */
	SOFTWARE_LIST(config, "cass_list").set_original("vg5k");
}

/* ROM definition */
ROM_START( vg5k )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v11", "BASIC v1.1")
	ROMX_LOAD( "vg5k11.bin", 0x0000, 0x4000, CRC(a6f4a0ea) SHA1(58eccce33cc21fc17bc83921018f531b8001eda3), ROM_BIOS(0) )  // dumped from a Philips VG-5000.
	ROM_SYSTEM_BIOS(1, "v10", "BASIC v1.0")
	ROMX_LOAD( "vg5k10.bin", 0x0000, 0x4000, BAD_DUMP CRC(57983260) SHA1(5ad1787a6a597b5c3eedb7c3704b649faa9be4ca), ROM_BIOS(1) )

	ROM_REGION( 0x4000, "ef9345", 0 )
	ROM_LOAD( "charset.rom", 0x0000, 0x2000, BAD_DUMP CRC(b2f49eb3) SHA1(d0ef530be33bfc296314e7152302d95fdf9520fc) )                // from dcvg5k
ROM_END

} // anonymous namespace


/* Driver */
//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT       COMPANY    FULLNAME   FLAGS
COMP( 1984, vg5k, 0,      0,      vg5k,    vg5k,  vg5k_state, init_vg5k, "Philips", "VG-5000", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )

// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert
/***************************************************************************

    ITT 3030

    When the machine is started you get a black screen.
    Hold down B until the cursor appears. It will then boot from the floppy.

    ToDo:
    - Check Beeper
    - finish hooking up keyboard
    - serial port
    - daisy chain
    - ...


    CPU Board, all ICs shown:

    |-----------------------------------------------------------------|
    |                                                                 |
    |    74LS640N          Z80_Combo            74LS138N              |
    |                                                       74LS00N   |
    |    74LS240N                               74LS74AN              |
    |                                                       74LS00N   |
    |C   74LS240N          Z80_CPU              74LS240N             C|
    |N                                                      74LS74AN N|
    |1   74LS241N                               74LS240N             2|
    |                      ROM_1      74LS20N               74LS38N   |
    |    74LS240N                               74LS240N              |
    |                                 74LS04N               74LS02N   |
    |    74LS138N                               74LS74AN              |
    |                                                       74LS175N  |
    |    75154N        74LS156N                 74LS00N               |
    |                                                       74LS123N  |
    |    75150P 75150P 74LS175N   X1  74LS00N   74LS132N              |
    |-----------------------------------------------------------------|

    Z80_Combo:  Mostek MK3886 Z80 Combo Chip, Serial, Timer, 256 bytes RAM, Interrupt Controller
    Z80_CPU:    Zilog Z80A CPU
    ROM_1:      NEC D2716D marked "BOOTV1.2"
    X1:         Crystal 4,194 MHz
    CN1:        Bus Connector
    CN2:        Memory Board Connector

----------------------------------------------------------------------------------

    Video / Keyboard Combination board, all ICs shown:

    |-----------------------------------------------------------------|
    |                                                                 |
    |         X1     74276N      MCU_1                  74LS85N       |
    |                                                                 |
    |    74LS138N    74LS240                            74LS240N      |
    |                            75LS257AN  74LS166AN                 |
    |    74LS08N     74LS85N                            74LS241N      |
    |                            75LS257AN  ROM_1                    C|
    |    74LS132N    74LS32N                            74LS240N     N|
    |                            75LS257AN                           1|
    |    74LS10N     74LS08N                            74LS240N      |
    |                            75LS257AN  RAM_1                     |
    |    74LS163AN   74LS173AN                          74LS374N      |
    |                                                                 |
    |    74LS86N     74LS240N                           74LS640N      |
    |                            Video_1                              |
    |    74LS74AN    74LS240N                           74LS640N      |
    |-----------------------------------------------------------------|

    X1:     Crystal 6 MHz
    MCU_1:  NEC D8741AD marked "V1.1 3030"
    ROM_1:  MBM 2716 marked "GB 136-0"
    RAM_1:  NEC D4016D
    Video_1 Video-IC SND5027E, compatible with TMS9927

----------------------------------------------------------------------------------

    Floppy Controller board, all ICs shown

    |-----------------------------------------------------------------|
    |                                                                 |
    |  X1   74LS51N    F    74LS74AN   74LS02N        MC4044P         |
    |                  D                        567                   |
    |     74LS04N      C    74LS00N    74LS01N  :::   MC4024P         |
    |                                                                 |
    |   74LS00N        1    74LS74AN   74LS74AN       74LS14N        C|
    |                  7                                             N|
    |  74LS240N        9    74LS161N   74LS393N   74LS74AN           1|
    |                  1                                              |
    |   74LS132N            74LS14N    74LS14N    74LS374N            |
    |                                                                 |
    |    74LS123N   74LS04N  74LS163N   74LS14N    74LS241N           |
    |                                                                 |
    |     74LS393N   74LS138  74LS175N   74LS85N    74LS645N          |
    |                                                                 |
    |-----------------------------------------------------------------|

    X1:     Crystal 8 MHz
    FDC:    Siemens SAB1791-02P
    567:    Jumper Pad (emtpy)

----------------------------------------------------------------------------------

    256K RAM board, all ICs shown:

    |-----------------------------------------------------------------|
    |                                                                 |
    |   HM4864P   HM4864P   HM4864P   HM4864P       74LS245N          |
    |                                                                 |
    |   HM4864P   HM4864P   HM4864P   HM4864P       P     74LS14N     |
    |                                               R                 |
    |   HM4864P   HM4864P   HM4864P   HM4864P       M     74LS00N     |
    |                                                                C|
    |   HM4864P   HM4864P   HM4864P   HM4864P       AM         A     N|
    |                                               29         M     1|
    |   HM4864P   HM4864P   HM4864P   HM4864P       66         2      |
    |                                               PC         9      |
    |   HM4864P   HM4864P   HM4864P   HM4864P                  6      |
    |                                               AM         4      |
    |   HM4864P   HM4864P   HM4864P   HM4864P       29         8      |
    |                                               66         P      |
    |   HM4864P   HM4864P   HM4864P   HM4864P       PC         C      |
    |                                                      SN7474N    |
    |-----------------------------------------------------------------|

    PRM:    N82S129F 1K Bipolar PROM
            AM2966PC: Octal Dynamic Memory Drivers with Three-State Outputs
            AM29648PC
    CN1:    Connector to CN2 of Z80 CPU card

----------------------------------------------------------------------------------

    Parallel I/O board, all ICs shown:

    |-------------------------------------|                                                                 |
    |                                     |
    |  74   74                            |
    |  LS   LS        Z80A PIO            |
    |  00   14                            |
    |   N    N                            |
    |                                     |
    |                                     |
    |  74   74        74   74   D4   74   |
    |  LS   LS        LS   LS   I3   LS   |
    |  13   14        24   85   P2   64   |
    |  2N    N        1N    N    1   0N   |
    |                                     |
    |             CN1                     |
    |                                     |
    |             74LS00N                 |
    |-------------------------------------|

    CN1: Bus connector
    DIP: 4x DIP current setting: off-on-on-off, sets the address for the parallel port

----------------------------------------------------------------------------------

Beeper Circuit, all ICs shown:

    |---------------------------|                                                                 |
    |                           |
    |   BEEP       74LS132N     |
    | R1                        |
    |              74LS14N      |
    |                           |
    |   74LS132N   74LS193N     |
    |                           |
    |   74LS74AN   74LS165N     |
    |            CN1            |
    |---------------------------|

    CN1: Connector to mainboard
    R1:  looks like a potentiometer
    BEEP: Beeper ... touted in the manual as "Hupe" ... i.e. "horn" :)

----------------------------------------------------------------------------------

    Other boards and extensions mentioned in the manual:
    - S100 bus adapter board
    - IEEE 488 bus adapter board
    - 64K memory board
    - 8086 CPU board
    - external harddisk
    - TV adapter B/W (TV, Save/Load from Audio Cassette) with PROM/RAM/BASIC-Module with 16K or 32K RAM
    - TV adapter color with connection to Video / Keyboard combination card
    - Monitor adapters B/W and color
    - Video / Keyboard interface 2 with grayscale, 8 colors, loadable character set, blinking
    - Graphics Adapter with 16 colours, hi-res 512x256 pixels
    - RTC
    - Arithmetics chip

***************************************************************************/


#include "emu.h"

#include "cpu/mcs48/mcs48.h"        //Keyboard MCU ... talks to the 8278 on the keyboard circuit
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/bankdev.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "sound/beep.h"
#include "video/tms9927.h"          //Display hardware

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "formats/itt3030_dsk.h"

#include "utf8.h"


namespace {

#define MAIN_CLOCK XTAL_4.194MHz

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class itt3030_state : public driver_device
{
public:
	itt3030_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_kbdmcu(*this, "kbdmcu")
		, m_ram(*this, "mainram")
		, m_crtc(*this, "crt5027")
		, m_48kbank(*this, "lowerbank")
		, m_fdc (*this, "fdc")
		, m_floppy(*this, "fdc:%u", 0)
		, m_beep(*this, "beeper")
		, m_keyrows(*this, "ROW.%u", 0)
		, m_vram(*this, "vram")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
	{ }

	void itt3030(machine_config &config);

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t vsync_r();
	void beep_w(uint8_t data);
	void bank_w(uint8_t data);
	int kbd_matrix_r();
	void kbd_matrix_w(uint8_t data);
	uint8_t kbd_port2_r();
	void kbd_port2_w(uint8_t data);
	uint8_t fdc_r(offs_t offset);
	void fdc_w(offs_t offset, uint8_t data);
	uint8_t fdc_stat_r();
	void fdc_cmd_w(uint8_t data);
	static void itt3030_floppy_formats(format_registration &fr);

	void fdcirq_w(int state);
	void fdcdrq_w(int state);
	void fdchld_w(int state);
	void itt3030_palette(palette_device &palette) const;

	void itt3030_io(address_map &map) ATTR_COLD;
	void itt3030_map(address_map &map) ATTR_COLD;
	void lower48_map(address_map &map) ATTR_COLD;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<i8741a_device> m_kbdmcu;
	required_device<ram_device> m_ram;
	required_device<crt5027_device> m_crtc;
	required_device<address_map_bank_device> m_48kbank;
	required_device<fd1791_device> m_fdc;
	required_device_array<floppy_connector, 3> m_floppy;
	required_device<beep_device> m_beep;

	required_ioport_array<16> m_keyrows;

	// shared pointers
	required_shared_ptr<uint8_t> m_vram;

	uint8_t m_kbdclk = 0, m_kbdread = 0, m_kbdport2 = 0;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	floppy_image_device *m_curfloppy = nullptr;
	bool m_fdc_irq = false, m_fdc_drq = false, m_fdc_hld = false;
};

//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

// The lower 48K is switchable among the first 48K of each of 8 48K banks numbered 0-7 or "bank 8" which is the internal ROM and VRAM
// The upper 16K is always the top 16K of the first bank, F5 can set this to 32K
// Port F6 bits 7-5 select banks 0-7, bit 4 enables bank 8

void itt3030_state::itt3030_map(address_map &map)
{
	map(0x0000, 0xbfff).m(m_48kbank, FUNC(address_map_bank_device::amap8));
}

void itt3030_state::lower48_map(address_map &map)
{
	map(0x60000, 0x607ff).rom().region("maincpu", 0);   // begin "page 8"
	map(0x60800, 0x60fff).rom().region("maincpu", 0);
	map(0x61000, 0x610ff).ram().mirror(0x100);  // only 256 bytes, but ROM also clears 11xx?
	map(0x63000, 0x63fff).ram().share("vram");
}

void itt3030_state::itt3030_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x20, 0x2f).rw(m_crtc, FUNC(crt5027_device::read), FUNC(crt5027_device::write));
	map(0x30, 0x31).rw(m_kbdmcu, FUNC(i8741a_device::upi41_master_r), FUNC(i8741a_device::upi41_master_w));
	map(0x32, 0x32).w(FUNC(itt3030_state::beep_w));
	map(0x35, 0x35).r(FUNC(itt3030_state::vsync_r));
	map(0x50, 0x53).rw(FUNC(itt3030_state::fdc_r), FUNC(itt3030_state::fdc_w));
	map(0x54, 0x54).rw(FUNC(itt3030_state::fdc_stat_r), FUNC(itt3030_state::fdc_cmd_w));
	map(0xf6, 0xf6).w(FUNC(itt3030_state::bank_w));
}


//**************************************************************************
//  INPUTS
//**************************************************************************

int itt3030_state::kbd_matrix_r()
{
	return m_kbdread;
}

void itt3030_state::kbd_matrix_w(uint8_t data)
{
//  printf("matrix_w: %02x (col %d row %d clk %d)\n", data, m_kbdcol, m_kbdrow, (data & 0x80) ? 1 : 0);

	if ((data & 0x80) && (!m_kbdclk))
	{
		const ioport_value tmp_read = m_keyrows[(data >> 3) & 0xf]->read() & (1 << (data & 0x7));
		m_kbdread = (tmp_read != 0) ? 1 : 0;
	}

	m_kbdclk = (data & 0x80) ? 1 : 0;
}

// bit 2 is UPI-41 host IRQ to Z80
void itt3030_state::kbd_port2_w(uint8_t data)
{
	m_kbdport2 = data;
}

uint8_t itt3030_state::kbd_port2_r()
{
	return m_kbdport2;
}


//**************************************************************************
//  KEYBOARD
//**************************************************************************

static INPUT_PORTS_START( itt3030 )
	PORT_START("ROW.0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')

	PORT_START("ROW.1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')

	PORT_START("ROW.2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F6") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('/')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')

	PORT_START("ROW.3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F7") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')

	PORT_START("ROW.4")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift Lock") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("ROW.5")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')

	PORT_START("ROW.6")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')   // actually UK pound symbol
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')

	PORT_START("ROW.7")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')

	PORT_START("ROW.8")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(R)") PORT_CODE(KEYCODE_F12) PORT_CHAR('=')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('~')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL) PORT_CHAR('^')    // PC doesn't have 3 keys to the right of L, so we sub DEL for the 3rd one
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("ROW.9")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(CL)") PORT_CODE(KEYCODE_F13) PORT_CHAR(4)    // produces control-D always
	PORT_BIT(0x001e, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ROW.10")
	PORT_BIT(0x001f, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ROW.11")
	PORT_BIT(0x001f, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ROW.12")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F8") PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR(';')

	PORT_START("ROW.13")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_F9)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('=')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR(':')

	PORT_START("ROW.14")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_F10)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('@') PORT_CHAR('?')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('<') PORT_CHAR('>')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('-') PORT_CHAR('`')

	PORT_START("ROW.15")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_F11) PORT_CHAR(27)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('/') PORT_CHAR('\\')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('+') PORT_CHAR('*')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_MAMEKEY(RSHIFT))
INPUT_PORTS_END


//**************************************************************************
//  VIDEO
//**************************************************************************

uint8_t itt3030_state::vsync_r()
{
	uint8_t ret = 0;

	if (m_screen->vblank())
	{
		ret |= 0xc0;    // set both bits 6 and 7 if vblank
	}

	if (m_screen->hblank())
	{
		ret |= 0x80;    // set only bit 7 if hblank
	}

	return ret;
}

void itt3030_state::bank_w(uint8_t data)
{
	int bank = 8;

	if (BIT(data, 4))
	{
		bank = (BIT(data, 5) << 2) | (BIT(data, 6) << 1) | BIT(data, 7);
	}

	//  printf("bank_w: new value %02x, m_bank %x, bank %x\n", data, m_bank, bank);

	m_48kbank->set_bank(bank);
}

uint32_t itt3030_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int start = m_crtc->upscroll_offset();
	for(int y = 0; y < 24; y++ )
	{
		int vramy = (start + y) % 24;
		for(int x = 0; x < 80; x++ )
		{
			uint8_t code = m_vram[x + vramy*128];
			int invert = code & 0x80 ? 1 : 0;
			code &= 0x7f;
			m_gfxdecode->gfx(invert)->opaque(bitmap,cliprect,  code , 0, 0,0, x*8,y*12);
		}
	}

	return 0;
}

static const gfx_layout charlayout =
{
	8, 16,              /* 8x16 characters */
	128,                /* 128 characters */
	1,                /* 1 bits per pixel */
	{0},                /* no bitplanes; 1 bit per pixel */
	{7, 6, 5, 4, 3, 2, 1, 0},
	{ 0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8,
		8*8,  9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    /* size of one char */
};

static GFXDECODE_START( gfx_itt3030 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 1 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 1 )
GFXDECODE_END

void itt3030_state::itt3030_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t::black());
	palette.set_pen_color(1, rgb_t(215, 229, 82));
	palette.set_pen_color(2, rgb_t::black());
}

//**************************************************************************
//  SOUND
//**************************************************************************

void itt3030_state::beep_w(uint8_t data)
{
	m_beep->set_state(data&1);
}

//**************************************************************************
//  FLOPPY
//**************************************************************************

void itt3030_state::fdcirq_w(int state)
{
	m_fdc_irq = state;
}


void itt3030_state::fdcdrq_w(int state)
{
	m_fdc_drq = state;
}

void itt3030_state::fdchld_w(int state)
{
	m_fdc_hld = state;
}

/*
    7 Data Request (DRQ - inverted 1791-Signal)
    6 Interrupt Request (INTRQ - 1791-Signal)
    5 Head Load (HLD - inverted 1791-Signal)
    4 Ready 3 (Drive 3 ready)
    3 Ready 2 (Drive 2 ready)
    2 Ready l (Drive 1 ready)
    1 Write protect (the disk in the selected drive is write protected)
    0 HLT (Halt signal during head load and track change)
*/
uint8_t itt3030_state::fdc_stat_r()
{
	uint8_t res = 0;
	floppy_image_device *floppy1 = m_floppy[0] ? m_floppy[0]->get_device() : nullptr;
	floppy_image_device *floppy2 = m_floppy[1] ? m_floppy[1]->get_device() : nullptr;
	floppy_image_device *floppy3 = m_floppy[2] ? m_floppy[2]->get_device() : nullptr;

	res = m_fdc_drq ? 0x80 : 0x00;
	res |= m_fdc_irq ? 0x40 : 0x00;
	res |= m_fdc_hld ? 0x00 : 0x20;
	if (floppy3) res |= !floppy3->ready_r() ? 0x10 : 0;
	if (floppy2) res |= !floppy2->ready_r() ? 0x08 : 0;
	if (floppy1) res |= !floppy1->ready_r() ? 0x04 : 0;
	if (m_curfloppy) res |= m_curfloppy->wpt_r() ? 0x02 : 0;

	return res;
}

/* As far as we can tell, the mess of ttl de-inverts the bus */
uint8_t itt3030_state::fdc_r(offs_t offset)
{
	return m_fdc->read(offset) ^ 0xff;
}

void itt3030_state::fdc_w(offs_t offset, uint8_t data)
{
	m_fdc->write(offset, data ^ 0xff);
}

/*
    7 SEL1 - Select drive 1
    6 SEL2 - Select drive 2
    5 SEL3 - Select drive 3
    4 MOTOR - Motor on
    3 DOOR - Drive door lock drives 1 + 2 (not possible with all drives)
    2 SIDESEL - Select disk side
    1 KOMP - write comp on/off
    0 RG J - Change separator stage to read
*/
void itt3030_state::fdc_cmd_w(uint8_t data)
{
	floppy_image_device *floppy = nullptr;

	logerror("%02x to fdc_cmd_w: motor %d side %d\n", data, (data & 0x10)>>4, (data & 4)>>2);

	// select drive
	if (data & 0x80)
	{
		floppy = m_floppy[0] ? m_floppy[0]->get_device() : nullptr;
	}
	else if (data & 0x40)
	{
		floppy = m_floppy[1] ? m_floppy[1]->get_device() : nullptr;
	}
	else if (data & 0x20)
	{
		floppy = m_floppy[2] ? m_floppy[2]->get_device() : nullptr;
	}

	// selecting a new drive?
	if (floppy != m_curfloppy)
	{
		m_fdc->set_floppy(floppy);
		m_curfloppy = floppy;
	}

	if (floppy != nullptr)
	{
		// side select
		floppy->ss_w((data & 4) ? 1 : 0);

		// motor control (active low)
		floppy->mon_w((data & 0x10) ? 0 : 1);
	}
}

//**************************************************************************
//  FLOPPY - Drive definitions
//**************************************************************************

void itt3030_state::itt3030_floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_ITT3030_FORMAT);
}


static void itt3030_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("525qd", FLOPPY_525_QD);
}




//**************************************************************************
//  MACHINE
//**************************************************************************

void itt3030_state::machine_start()
{
	save_item(NAME(m_kbdread));
	m_48kbank->space(AS_PROGRAM).install_ram(0, m_ram->size() - 16384, m_ram->pointer());
	m_maincpu->space(AS_PROGRAM).install_ram(0xc000, 0xffff, m_ram->pointer() + m_ram->size() - 16384);
	m_gfxdecode->gfx(1)->set_colorbase(1);

	m_kbdclk = 0;   // must be initialized here b/c mcs48_reset() causes write of 0xff to all ports
}

void itt3030_state::machine_reset()
{
	m_48kbank->set_bank(8);
	m_kbdread = 1;
	m_kbdclk = 1;
	m_fdc_irq = m_fdc_drq = m_fdc_hld = 0;
	m_curfloppy = nullptr;
}

void itt3030_state::itt3030(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &itt3030_state::itt3030_map);
	m_maincpu->set_addrmap(AS_IO, &itt3030_state::itt3030_io);

	// Schematics + i8278 datasheet says:
	// Port 1 goes to the keyboard matrix.
	// bits 0-2 select bit to read back, bits 3-6 choose column to read from, bit 7 clocks the process (rising edge strobes the row, falling edge reads the data)
	// T0 is the key matrix return
	// pin 23 is the UPI-41 host IRQ line, it's unknown how it's connected to the Z80
	I8741A(config, m_kbdmcu, 6_MHz_XTAL);
	m_kbdmcu->t0_in_cb().set(FUNC(itt3030_state::kbd_matrix_r));
	m_kbdmcu->p1_out_cb().set(FUNC(itt3030_state::kbd_matrix_w));
	m_kbdmcu->p2_in_cb().set(FUNC(itt3030_state::kbd_port2_r));
	m_kbdmcu->p2_out_cb().set(FUNC(itt3030_state::kbd_port2_w));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(250));
	m_screen->set_screen_update(FUNC(itt3030_state::screen_update));
	m_screen->set_size(80*8, 24*12);
	m_screen->set_visarea(0, 80*8-1, 0, 24*12-1);
	m_screen->set_palette(m_palette);

	/* devices */
	ADDRESS_MAP_BANK(config, "lowerbank").set_map(&itt3030_state::lower48_map).set_options(ENDIANNESS_LITTLE, 8, 20, 0xc000);

	CRT5027(config, m_crtc, 6_MHz_XTAL / 8).set_char_width(8);

	FD1791(config, m_fdc, 20_MHz_XTAL / 20);
	m_fdc->intrq_wr_callback().set(FUNC(itt3030_state::fdcirq_w));
	m_fdc->drq_wr_callback().set(FUNC(itt3030_state::fdcdrq_w));
	m_fdc->hld_wr_callback().set(FUNC(itt3030_state::fdchld_w));
	FLOPPY_CONNECTOR(config, "fdc:0", itt3030_floppies, "525qd", itt3030_state::itt3030_floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", itt3030_floppies, "525qd", itt3030_state::itt3030_floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:2", itt3030_floppies, "525qd", itt3030_state::itt3030_floppy_formats);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_itt3030);

	PALETTE(config, m_palette, FUNC(itt3030_state::itt3030_palette), 3);

	/* internal ram */
	RAM(config, "mainram").set_default_size("256K");

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 3250).add_route(ALL_OUTPUTS, "mono", 1.00);

	SOFTWARE_LIST(config, "flop_list").set_original("itt3030");
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( itt3030 )
	ROM_REGION( 0x0800, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "bootv1.2.bin", 0x0000, 0x0800, CRC(90279d45) SHA1(a39a3f31f4f98980b1ef50805870837fbf72261d))
	ROM_REGION( 0x0800, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD( "gb136-0.bin", 0x0000, 0x0800, CRC(6a3895a8) SHA1(f3b977ffa2f54c346521c9ef034830de8f404621))
	ROM_REGION( 0x0400, "kbdmcu", ROMREGION_ERASE00 )
	ROM_LOAD( "8741ad.bin", 0x0000, 0x0400, CRC(cabf4394) SHA1(e5d1416b568efa32b578ca295a29b7b5d20c0def))
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

COMP( 1982, itt3030, 0, 0, itt3030, itt3030, itt3030_state, empty_init, "ITT RFA", "ITT3030", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

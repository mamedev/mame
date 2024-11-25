// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

A business-only clone of the TRS-80 Model III.

All controllers are built-in. Disk drives are required. It can
boot from any size floppy or from a hard disk. All the drives are
in external enclosures. External connections are:
- 5.25 floppy connector (up to 4 drives)
- 8 floppy connector (up to 4 drives)
- HD connector (1 only, gives total of 9 drives)
- Serial A
- Serial B
- Expansion I/O

All configuration comes from the floppy image. This includes setting up
the CRTC, the character generator, the SIO and the keyboard.

There was a Technical Manual, but I was unable to find a copy, and so
everything in here is based upon what could be gleaned from the schematic.

There's IMD floppy images available, but it's unknown what kind of drive
is needed. I was not able to boot anything.

Organisation of memory:
- There's 128K of RAM, split into 32k sections. Bits 6,7 of 7FC control the
  banking. According to the disassembly, 0x80 selects A1 and B1, while 0x40
  selects A1 and A2. I presume that the 64K banks are A and B, and their 32K
  halves are 1 and 2. It's unknown what combinations 0x00 and 0xC0 select.

- Overlaid on this is a special 4K-sized bank which holds the ROM, the video
  RAM and all the devices. This bank can be anywhere in memory on a 4K boundary
  chosen by writing to x7DC. Further, the 4K bank has some unassigned areas,
  allowing the underlying main ram to shine through. It's unknown what happens
  if you attempt to write to ROM. The bank is initially at address 0-FFF.

NOTE on character generation:
- There's no character generator ROM. All definitions are in a 6116 RAM at
  an unknown address within the 4K bank. Therefore nothing can be seen until
  a successful boot from disk, which contains the font data, and also does
  initialisation of the CRTC. Video RAM resides in another 6116, which is
  also in the 4K bank.

Booting:
- A sector is read into main RAM starting at 0x400. Then it is executed and
  the process of booting can begin. In TRS-80 compatible-mode (a disk program),
  the 4K bank is moved to 0x3000.

***************************************************************************


To Do: Almost everything.
Status: Beeps every so often. Unable to read the disk.


***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
//#include "machine/ram.h"
#include "imagedev/floppy.h"
#include "imagedev/snapquik.h"
#include "machine/z80sio.h"
#include "machine/z80pio.h"
#include "machine/com8116.h"
#include "machine/msm5832.h"
//#include "bus/rs232/rs232.h"
#include "machine/wd_fdc.h"
#include "sound/beep.h"
#include "machine/timer.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "formats/imd_dsk.h"
//#include "formats/trs80_dsk.h"
//#include "formats/dmk_dsk.h"

#include "utf8.h"


namespace {

class max80_state : public driver_device
{
public:
	max80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
//      , m_p_chargen(*this, "chargen")
//      , m_vram(*this, "videoram")
		, m_palette(*this, "palette")
		, m_pio(*this, "pio")
		, m_crtc(*this, "crtc")
		, m_uart(*this, "uart")
		, m_brg(*this, "brg")
		, m_beep(*this, "beeper")
		, m_beep_timer(*this, "beep_timer")
		, m_rtc(*this, "rtc")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_floppy2(*this, "fdc:2")
		, m_floppy3(*this, "fdc:3")
		, m_io_keyboard(*this, "LINE%u", 0U)
//      , m_mainram(*this, RAM_TAG)
	{ }

	void max80(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	static void floppy_formats(format_registration &fr);
	[[maybe_unused]] u8 keyboard_r(offs_t offset);
	void beep_w(offs_t offset, u8 data);
	void mode_w(offs_t offset, u8 data);
	void drive_w(offs_t offset, u8 data);
	u8 fdc_status_r(offs_t offset);
	u8 pio_pa_r(offs_t offset);
	void pio_pa_w(offs_t offset, u8 data);
	void pio_pb_w(offs_t offset, u8 data);

	void intrq_w(int state);
	void drq_w(int state);
	MC6845_UPDATE_ROW(crtc_update_row);
	TIMER_DEVICE_CALLBACK_MEMBER(beep_timer);

	void mem_map(address_map &map) ATTR_COLD;

	u8 m_size_store = 0U;
	bool m_fdc_drq = 0;
	bool m_fdc_int = 0;
	bool m_allow_nmi = 0;
	u8 m_mode = 0U;
	floppy_image_device *m_floppy = 0;
	required_device<cpu_device> m_maincpu;
	//required_region_ptr<u8> m_p_chargen;
	//required_shared_ptr<u8> m_p_vram;
	required_device<palette_device> m_palette;
	required_device<z80pio_device> m_pio;
	required_device<mc6845_device> m_crtc;
	required_device<z80sio_device> m_uart;
	required_device<com8116_device> m_brg;
	required_device<beep_device> m_beep;
	required_device<timer_device> m_beep_timer;
	required_device<msm5832_device> m_rtc;
	required_device<mb8876_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;
	required_ioport_array<8> m_io_keyboard;
//  optional_device<ram_device>                 m_mainram;
};


void max80_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).ram();
	map(0x0000, 0x01ff).rom();
	//map(0x3800, 0x3bff).r(FUNC(max80_state::keyboard_r));
	//map(0x3c00, 0x3fff).ram().share(m_p_videoram);
	map(0x07d0, 0x07d3).w(m_brg, FUNC(com8116_device::str_w));  // W0
	map(0x07d4, 0x07d7).w(m_brg, FUNC(com8116_device::stt_w));  // W1
	map(0x07d8, 0x07db).w(FUNC(max80_state::drive_w));       // W2
	map(0x07dc, 0x07df).w(FUNC(max80_state::mode_w));   // 0x30 to move 4k area to 3000-3FFF   //W3
	map(0x07e0, 0x07e0).mirror(2).w(m_crtc, FUNC(mc6845_device::address_w));    // OUT 0
	map(0x07e1, 0x07e1).mirror(2).w(m_crtc, FUNC(mc6845_device::register_w));   // OUT 0
	map(0x07e4, 0x07e7).rw(m_uart, FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));  // IN1,OUT1
	map(0x07ec, 0x07ef).rw(m_fdc, FUNC(mb8876_device::read), FUNC(mb8876_device::write));  // IN3,OUT3
	//map(0x07f0, 0x07f3).r   // udata
	map(0x07f4, 0x07f7).r(FUNC(max80_state::fdc_status_r));
	map(0x07f8, 0x07fb).portr("BOOT");   // IN6
	map(0x07f8, 0x07fb).w(FUNC(max80_state::beep_w));
	map(0x07fc, 0x07ff).rw(m_pio, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt)); // IN7,OUT7
}

static INPUT_PORTS_START( max80 )
	PORT_START("LINE0")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_INSERT)     PORT_CHAR('@')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')

	PORT_START("LINE1")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("LINE2")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')

	PORT_START("LINE3")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("\\") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("_") PORT_CODE(KEYCODE_DEL)        PORT_CHAR('_') PORT_CHAR(127)

	PORT_START("LINE4")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD)          PORT_CHAR('0')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD)          PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD)          PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD)          PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("LINE5")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD)          PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD)          PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME(": *") PORT_CODE(KEYCODE_MINUS)        PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("; +") PORT_CODE(KEYCODE_COLON)        PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA)        PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("- =") PORT_CODE(KEYCODE_EQUALS)       PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH)        PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("LINE6")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(13)
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("Clear") PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_END)        PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)        PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_BACKSPACE)   PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')

	PORT_START("LINE7")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x40, 0x00, IPT_UNUSED)
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("CTL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)

	PORT_START("BOOT")   // lower 3 dips decide the boot device. Other 5 dips are unknown. Manual shows them all set low.
	PORT_DIPNAME( 0x07, 0x02, "Boot device")
	PORT_DIPSETTING(    0x00, "None")
	PORT_DIPSETTING(    0x01, "Floppy 5")
	PORT_DIPSETTING(    0x02, "Floppy 8")
	PORT_DIPSETTING(    0x03, "HD 5 UVC")
	PORT_DIPSETTING(    0x04, "HD 8 UVC")
	PORT_DIPSETTING(    0x05, "FD 5 UVC")
	PORT_DIPSETTING(    0x06, "FD 8 UVC")
	PORT_DIPSETTING(    0x07, "HD SASI")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


/*************************************
 *
 *              Port handlers.
 *
 *************************************/


TIMER_DEVICE_CALLBACK_MEMBER(max80_state::beep_timer)
{
	m_beep->set_state(0);
}

void max80_state::drive_w(offs_t offset, u8 data)
{
	m_floppy = nullptr;

	if (BIT(data, 0)) m_floppy = m_floppy0->get_device();
	if (BIT(data, 1)) m_floppy = m_floppy1->get_device();
	if (BIT(data, 2)) m_floppy = m_floppy2->get_device();
	if (BIT(data, 3)) m_floppy = m_floppy3->get_device();

	m_fdc->set_floppy(m_floppy);

	if (m_floppy)
	{
		m_floppy->mon_w(0);
		m_floppy->ss_w(BIT(data, 4));
	}

	m_fdc->dden_w(!BIT(data, 6));
	m_fdc->set_unscaled_clock(BIT(data, 5) ? 2000000 : 1000000);
	m_allow_nmi = BIT(data, 7);
}


void max80_state::intrq_w(int state)
{
	m_fdc_int = state;
}

void max80_state::drq_w(int state)
{
	m_fdc_drq = state;
	m_maincpu->set_input_line(INPUT_LINE_NMI, (state && m_allow_nmi) ? ASSERT_LINE : CLEAR_LINE);
}

u8 max80_state::keyboard_r(offs_t offset)
{
	u8 i, result = 0;

	for (i = 0; i < 8; i++)
		if (BIT(offset, i))
			result |= m_io_keyboard[i]->read();

	return result;
}

u8 max80_state::fdc_status_r(offs_t offset)
{
	u8 data = 0xfc | int(m_fdc_drq) | (m_fdc_int << 1);
	return data;
}

void max80_state::beep_w(offs_t offset, u8 data)
{
	m_beep->set_state(1);
	m_beep_timer->adjust(attotime::from_msec(150));
}

void max80_state::mode_w(offs_t offset, u8 data)
{
	// bit 0, disable rom
	// bit 1, disable i/o
	// bit 2, enable video access from cpu
	// bit 3, enable wide characters
	// bits 4-7, move rom and i/o to the block starting with x000, where x = BIT(data,4,4)
	m_mode = data;
}

u8 max80_state::pio_pa_r(offs_t offset)
{
	return m_rtc->data_r();
}

void max80_state::pio_pa_w(offs_t offset, u8 data)
{
	m_rtc->data_w(data & 15);
	// Memory banking of the 64k rams
	// if subsequent memory address of >=0x8000, bits 6/7 are used, else bits 4/5.
	// High bit switches 64k banks. Low bit does something with 32k, but not sure what.
}

void max80_state::pio_pb_w(offs_t offset, u8 data)
{
	m_rtc->address_w(data & 15);
	m_rtc->write_w(BIT(data, 4));
	m_rtc->read_w(BIT(data, 5));
	m_rtc->hold_w(BIT(data, 6));
}

/*************************************
 *  Machine              *
 *************************************/

void max80_state::machine_start()
{
//  save_item(NAME(m_irq));
//  save_item(NAME(m_size_store));
//  save_item(NAME(m_drq_off));
//  save_item(NAME(m_intrq_off));

}

void max80_state::machine_reset()
{
	m_size_store = 0xff;
	m_fdc_drq = false;
	m_fdc_int = false;
	m_floppy = nullptr;
}

MC6845_UPDATE_ROW( max80_state::crtc_update_row )
{
#if 0
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u32 *p = &bitmap.pix(y);

	for (u16 x = 0; x < x_count; x++)
	{
		u16 mem = (ma + x) & 0x7ff;
		u8 chr = m_vram[mem];
		u8 gfx = m_p_chargen[(chr<<4) | ra] ^ ((x == cursor_x) ? 0xff : 0);

		/* Display a scanline of a character (8 pixels) */
		*p++ = palette[BIT(gfx, 7)];
		*p++ = palette[BIT(gfx, 6)];
		*p++ = palette[BIT(gfx, 5)];
		*p++ = palette[BIT(gfx, 4)];
		*p++ = palette[BIT(gfx, 3)];
		*p++ = palette[BIT(gfx, 2)];
		*p++ = palette[BIT(gfx, 1)];
		*p++ = palette[BIT(gfx, 0)];
	}
#endif
}


void max80_state::floppy_formats(format_registration &fr)
{
	fr.add(FLOPPY_IMD_FORMAT);
	//fr.add(FLOPPY_JV3_FORMAT);
	//fr.add(FLOPPY_DMK_FORMAT);
	//fr.add(FLOPPY_JV1_FORMAT);
}

static void max80_floppies(device_slot_interface &device)
{
	// Available images get rejected with 40-track drives
	//device.option_add("40t_sd", FLOPPY_525_SSSD);
	//device.option_add("40t_dd", FLOPPY_525_DD);
	device.option_add("80t_qd", FLOPPY_525_QD);
	device.option_add("8ssdd", FLOPPY_8_SSDD);
	device.option_add("8dsdd", FLOPPY_8_DSDD);
}


void max80_state::max80(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 15'200'000 / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &max80_state::mem_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2000)); // not accurate
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));
	screen.set_size(640, 480);
	screen.set_visarea(0, 639, 0, 479);

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	HD6845S(config, m_crtc, 15'200'000 / 8);   // HD46505
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(max80_state::crtc_update_row));

	// devices
	MB8876(config, m_fdc, 8_MHz_XTAL / 4);
	m_fdc->intrq_wr_callback().set(FUNC(max80_state::intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(max80_state::drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", max80_floppies, "8ssdd", max80_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", max80_floppies, nullptr, max80_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:2", max80_floppies, nullptr, max80_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:3", max80_floppies, nullptr, max80_state::floppy_formats).enable_sound(true);

	Z80PIO(config, m_pio, 15'200'000 / 3);
	m_pio->in_pa_callback().set(FUNC(max80_state::pio_pa_r));
	m_pio->out_pa_callback().set(FUNC(max80_state::pio_pa_w));
	m_pio->out_pb_callback().set(FUNC(max80_state::pio_pb_w));

	COM8116(config, m_brg, 5'068'800);   // A few gates wired up as an oscillator. Frequency guessed.
	m_brg->fr_handler().set(m_uart, FUNC(z80sio_device::rxca_w));
	m_brg->ft_handler().set(m_uart, FUNC(z80sio_device::rxcb_w));

	MSM5832(config, m_rtc, 32.768_kHz_XTAL);

	Z80SIO(config, m_uart, 15'200'000 / 3);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 800).add_route(ALL_OUTPUTS, "mono", 0.50);
	TIMER(config, m_beep_timer).configure_generic(FUNC(max80_state::beep_timer));
}


/***************************************************************************

  Game driver(s)

***************************************************************************/


ROM_START(max80)
	ROM_REGION(0x0200, "maincpu",0)
	ROM_LOAD("max80.e12", 0x0000, 0x0200, CRC(cf316f25) SHA1(78663711c6100a67ef18382284565feda2bbbf77) )
ROM_END

} // anonymous namespace


//    YEAR  NAME      PARENT    COMPAT    MACHINE   INPUT     CLASS          INIT             COMPANY          FULLNAME               FLAGS
COMP( 1982, max80,    0,        trs80l2,  max80,    max80,    max80_state, empty_init,    "Lobo Systems",      "MAX-80",        MACHINE_NOT_WORKING ) //| MACHINE_SUPPORTS_SAVE )

// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************************

CADR I/O Board emulation

The I/O Board makes the keyboard, mouse, and chaosnet available on the Unibus.

Implementation based on description of the operation, not schematics.


TODO:
- Chaosnet
  - Sys300/301 needs a small hack to continue booting - timing issue with keyboard controller combined with bug in cadr code?
    The initial 'release' message is sent too soon by the keyboard? or is there communication with other hardware/mcu
    before the data is presented to the cpu.
    wpdset 3ff435,1,r
    wpdset 3ff430,1,rw, wait for write 0090, read 0090
    wpdset 3ff425,1,r, wait for read from 3ff425, MD=4
    and continue
- General purpose I/O
- Serial I/O
- Where is the REPEAT key mapped? And how should it be used?
- Caps lock is not working
- The keyboard and I/O board actually communicate through a serial connection.
- Is there another MCU on the I/O board to communicate with the keyboard??
- How are chaosnet packet checksums calculated?
  The chaosnet documentation mentions 16 bit CRC checksums, usim uses internet checksum.

**********************************************************************************/
#include "emu.h"
#include "cadr_iob.h"

#include "speaker.h"


//#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"



namespace {

static constexpr u16 IRQ_VECTOR_KEYBOARD = 0xb0;
static constexpr u16 IRQ_VECTOR_MOUSE = 0xb4;
static constexpr u16 IRQ_VECTOR_CHAOSNET = 0xb8;
static constexpr u16 IRQ_VECTOR_CLOCK = 0xbc;
static constexpr u16 IRQ_VECTOR_CHAOS_TRANSMIT = 0x100;
static constexpr u16 IRQ_VECTOR_CHAOS_RECEIVE = 0x104;

static constexpr int CSR_REMOTE_MOUSE_ENABLE_BIT = 0;
static constexpr int CSR_MOUSE_IRQ_ENABLE_BIT = 1;
static constexpr int CSR_KEYBOARD_IRQ_ENABLE_BIT = 2;
static constexpr int CSR_CLOCK_IRQ_ENABLE_BIT = 3;
static constexpr int CSR_MOUSE_READY_BIT = 4;
static constexpr u16 CSR_MOUSE_READY = 1 << CSR_MOUSE_READY_BIT;
static constexpr int CSR_KEYBOARD_READY_BIT = 5;
static constexpr u16 CSR_KEYBOARD_READY = 1 << CSR_KEYBOARD_READY_BIT;
static constexpr int CSR_CLOCK_READY_BIT = 6;
static constexpr u16 CSR_CLOCK_READY = 1 << CSR_CLOCK_READY_BIT;

static constexpr int CHAOSNET_TIMER_IRQ_ENABLE_BIT = 0;
static constexpr u16 CHAOSNET_TIMER_IRQ_ENABLE = 1 << CHAOSNET_TIMER_IRQ_ENABLE_BIT;
static constexpr int CHAOSNET_LOOPBACK_BIT = 1;
static constexpr u16 CHAOSNET_LOOKBACK = 1 << CHAOSNET_LOOPBACK_BIT;
static constexpr int CHAOSNET_ANY_DESTINATION_BIT = 2;
static constexpr u16 CHAOSNET_ANY_DESTINATION = 1 << CHAOSNET_ANY_DESTINATION_BIT;
static constexpr int CHAOSNET_RESET_RECEIVE_BIT = 3;
static constexpr int CHAOSNET_RECEIVE_IRQ_ENABLE_BIT = 4;
static constexpr u16 CHAOSNET_RECEIVE_IRQ_ENABLE = 1 << CHAOSNET_RECEIVE_IRQ_ENABLE_BIT;
static constexpr int CHAOSNET_TRANSMIT_IRQ_ENABLE_BIT = 5;
static constexpr u16 CHAOSNET_TRANSMIT_IRQ_ENABLE = 1 << CHAOSNET_TRANSMIT_IRQ_ENABLE_BIT;
static constexpr int CHAOSNET_TRANSMIT_ABORTED_BIT = 6;
static constexpr u16 CHAOSNET_TRANSMIT_ABORTED = 1 << CHAOSNET_TRANSMIT_ABORTED_BIT;
static constexpr int CHAOSNET_TRANSMIT_DONE_BIT = 7;
static constexpr u16 CHAOSNET_TRANSMIT_DONE = 1 << CHAOSNET_TRANSMIT_DONE_BIT;
static constexpr int CHAOSNET_RESET_TRANSMIT_BIT = 8;
static constexpr u16 CHAOSNET_LOST_COUNT = 0x1e00;
static constexpr int CHAOSNET_RESET_BIT = 13;
static constexpr u16 CHAOSNET_RESET = 1 << CHAOSNET_RESET_BIT;
static constexpr int CHAOSNET_RECEIVE_DONE_BIT = 15;
static constexpr u16 CHAOSNET_RECEIVE_DONE = 1 << CHAOSNET_RECEIVE_DONE_BIT;

} // anonymous namespace


DEFINE_DEVICE_TYPE(CADR_IOB, cadr_iob_device, "cadr_iob", "CADR I/O Board")


cadr_iob_device::cadr_iob_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, CADR_IOB, tag, owner, clock)
	, m_i8748(*this, "i8748")
	, m_irq_vector_cb(*this)
	, m_keyboard(*this, "KEY.%u", 0)
	, m_mouse_x(*this, "MOUSE_X")
	, m_mouse_y(*this, "MOUSE_Y")
	, m_mouse_buttons(*this, "MOUSE_BUTTONS")
	, m_my_chaos_address(*this, "MY_CHAOS_ADDRESS")
	, m_speaker(*this, "speaker")
{
}


void cadr_iob_device::device_add_mconfig(machine_config &config)
{
	I8748(config, m_i8748, 6_MHz_XTAL);
	m_i8748->bus_in_cb().set(FUNC(cadr_iob_device::mcu_bus_r));
	m_i8748->bus_out_cb().set(FUNC(cadr_iob_device::mcu_bus_w));
	m_i8748->p1_out_cb().set(FUNC(cadr_iob_device::mcu_p1_w));
	m_i8748->p2_in_cb().set(FUNC(cadr_iob_device::mcu_p2_r));

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00);
}


ROM_START(cadr_keyboard)
	ROM_REGION(0x400, "i8748", 0)
	ROM_LOAD("cadr_kb_8748_3-7-83.bin", 0x0, 0x400, CRC(8467ad0e) SHA1(cefbbd8f0afb86bd0157041f868fa70b38848582))
ROM_END


const tiny_rom_entry *cadr_iob_device::device_rom_region() const
{
	return ROM_NAME(cadr_keyboard);
}


static INPUT_PORTS_START(keyboard)
	PORT_START("KEY.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN) // f2 02 00
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ⅱ")// f2 02 02
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ⅳ") // f2 02 04
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MODE LOCK") // f2 02 06
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN) // f2 02 08
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L.SUPER") // f2 02 0a
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN) // f2 02 0c
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN) // f2 02 0e

	PORT_START("KEY.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN) // f2 02 10
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$') // 4 f2 02 12
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') // R f2 02 14
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') // F f2 02 16
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') // V f2 02 18
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ALT LOCK") // f2 02 1a
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN) // f2 02 1c
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) // f2 02 1e

	PORT_START("KEY.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2) // f2 02 20
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(": ±") // f2 02 22
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t') // f2 02 24
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RUB OUT") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) // f2 02 26
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L.SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1) // f2 02 28
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R.SHIFT") PORT_CODE(KEYCODE_RSHIFT) // f2 02 2a
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RCONTROL) // f2 02 2c
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN) // f2 02 2e

	PORT_START("KEY.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("HOLD OUTPUT") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7)) // f2 02 30
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*') // f2 02 32
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') // f2 02 34
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') // f2 02 36
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<') // f2 02 38
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R.GREEK") // f2 02 3a
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LINE") // f2 02 3c
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('\\') PORT_CHAR('|') // f2 02 3e

	PORT_START("KEY.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TERMINAL") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2)) // f2 02 40
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN) // f2 02 42
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("NETWORK") // f2 02 44
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN) // f2 02 46
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L.GREEK") // f2 02 48
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L.META") // f2 02 4a
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("STATUS") // f2 02 4c
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RESUME") PORT_CODE(KEYCODE_F11) PORT_CHAR(UCHAR_MAMEKEY(F11)) // f2 02 4e

	PORT_START("KEY.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CLEAR SCREEN") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6)) // Refreshes screen, f2 02 50
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^') // f2 02 52
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') // f2 02 54
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') // f2 02 56
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N') // f2 02 58
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN) // f2 02 5a
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN) // f2 02 5c
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN) // f2 02 5e

	PORT_START("KEY.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN) // f2 02 60
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@') // f2 02 62
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') // f2 02 64
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') // f2 02 66
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') // f2 02 68
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R.SUPER") // f2 02 6a
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN) // f2 02 6c
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ABORT") PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9)) // f2 02 6e

	PORT_START("KEY.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN) // f2 02 70
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') // also (, f2 02 72
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') // f2 02 74
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') // f2 02 76
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>') // f2 02 78
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN) // f2 02 7a
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN) // f2 02 7c
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('`') PORT_CHAR('~') // f2 02 7e

	PORT_START("KEY.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MACRO") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1)) // f2 02 80
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ⅰ") // f2 02 82
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ⅲ") // f2 02 84
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN) // f2 02 86
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L.TOP") // f2 02 88
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN) // f2 02 8a
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP)) // f2 02 8c
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CALL") PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F12)) // CALL f2 02 8e

	PORT_START("KEY.9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CLEAR INPUT") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5)) // f2 02 90
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%') // f2 02 92
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') // f2 02 94
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') // f2 02 96
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')  // f2 02 98
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN) // f2 02 9a
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("HELP") // f2 02 9c
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) // f2 02 9e

	PORT_START("KEY.10")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("QUOTE") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3)) // f2 02 a0
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!') // f2 02 a2
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') // f2 02 a4
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') // f2 02 a6
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') // f2 02 a8
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) // f2 02 aa
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+') // f2 02 ac
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN) // f2 02 ae

	PORT_START("KEY.11")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN) // f2 02 b0
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_') // f2 02 b2
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('(') // f2 02 b4
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') // also ", f2 02 b6
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') // f2 02 b8
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN) // f2 02 ba
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) // f2 02 bc
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR(')') // f2 02 be

	PORT_START("KEY.12")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN) // f2 02 c0
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SYSTEM") // f2 02 c2
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN) // f2 02 c4
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ALT MODE") // f2 02 c6
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN) // f2 02 c8
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L.HYPER") // f2 02 ca
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('}') // also >, f2 02 cc
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN) // f2 02 ce

	PORT_START("KEY.13")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN) // f2 02 d0
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&') // f2 02 d2
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') // f2 02 d4
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') // f2 02 d6
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M') // f2 02 d8
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R.TOP") // f2 02 da
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("END") // f2 02 dc
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DELETE") PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(DEL)) // f2 02 de

	PORT_START("KEY.14")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("OVER STRIKE") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4)) // f2 02 e0
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#') // f2 02 e2
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') // f2 02 e4
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') // f2 02 e6
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') // f2 02 e8
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R.META") // f2 02 ea
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('{') // also <, f2 02 ec
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("BREAK") PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10)) // keyboard break f2 02 ee

	PORT_START("KEY.15")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("STOP OUTPUT") PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8)) // STOP-OUTPUT f2 02 f0
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') // also ),  f2 02 f2
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') // f2 02 f4
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(':') PORT_CHAR(';') // f2 02 f6
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?') // f2 02 f8
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R.HYPER") // f2 02 fa
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) // f2 02 fc
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN) // f2 02 fe


	PORT_START("MOUSE_X")
	PORT_BIT(0xfff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(50) PORT_KEYDELTA(0) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(cadr_iob_device::mouse_changed), 1)

	PORT_START("MOUSE_Y")
	PORT_BIT(0xfff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(50) PORT_KEYDELTA(0) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(cadr_iob_device::mouse_changed), 2)

	PORT_START("MOUSE_BUTTONS")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Left mouse button")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Middle mouse button")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Right mouse button")


	PORT_START("MY_CHAOS_ADDRESS")
	PORT_DIPNAME(0xf000, 0x0000, "net high nibble")
	PORT_DIPSETTING(0x0000, "0")
	PORT_DIPSETTING(0x1000, "1")
	PORT_DIPSETTING(0x2000, "2")
	PORT_DIPSETTING(0x3000, "3")
	PORT_DIPSETTING(0x4000, "4")
	PORT_DIPSETTING(0x5000, "5")
	PORT_DIPSETTING(0x6000, "6")
	PORT_DIPSETTING(0x7000, "7")
	PORT_DIPSETTING(0x8000, "8")
	PORT_DIPSETTING(0x9000, "9")
	PORT_DIPSETTING(0xa000, "a")
	PORT_DIPSETTING(0xb000, "b")
	PORT_DIPSETTING(0xc000, "c")
	PORT_DIPSETTING(0xd000, "d")
	PORT_DIPSETTING(0xe000, "e")
	PORT_DIPSETTING(0xf000, "f")
	PORT_DIPNAME(0x0f00, 0x0400, "net low nibble")
	PORT_DIPSETTING(0x0000, "0")
	PORT_DIPSETTING(0x0100, "1")
	PORT_DIPSETTING(0x0200, "2")
	PORT_DIPSETTING(0x0300, "3")
	PORT_DIPSETTING(0x0400, "4")
	PORT_DIPSETTING(0x0500, "5")
	PORT_DIPSETTING(0x0600, "6")
	PORT_DIPSETTING(0x0700, "7")
	PORT_DIPSETTING(0x0800, "8")
	PORT_DIPSETTING(0x0900, "9")
	PORT_DIPSETTING(0x0a00, "a")
	PORT_DIPSETTING(0x0b00, "b")
	PORT_DIPSETTING(0x0c00, "c")
	PORT_DIPSETTING(0x0d00, "d")
	PORT_DIPSETTING(0x0e00, "e")
	PORT_DIPSETTING(0x0f00, "f")
	PORT_DIPNAME(0x00f0, 0x0020, "host high nibble")
	PORT_DIPSETTING(0x0000, "0")
	PORT_DIPSETTING(0x0010, "1")
	PORT_DIPSETTING(0x0020, "2")
	PORT_DIPSETTING(0x0030, "3")
	PORT_DIPSETTING(0x0040, "4")
	PORT_DIPSETTING(0x0050, "5")
	PORT_DIPSETTING(0x0060, "6")
	PORT_DIPSETTING(0x0070, "7")
	PORT_DIPSETTING(0x0080, "8")
	PORT_DIPSETTING(0x0090, "9")
	PORT_DIPSETTING(0x00a0, "a")
	PORT_DIPSETTING(0x00b0, "b")
	PORT_DIPSETTING(0x00c0, "c")
	PORT_DIPSETTING(0x00d0, "d")
	PORT_DIPSETTING(0x00e0, "e")
	PORT_DIPSETTING(0x00f0, "f")
	PORT_DIPNAME(0x000f, 0x0002, "host low nibble")
	PORT_DIPSETTING(0x0000, "0")
	PORT_DIPSETTING(0x0001, "1")
	PORT_DIPSETTING(0x0002, "2")
	PORT_DIPSETTING(0x0003, "3")
	PORT_DIPSETTING(0x0004, "4")
	PORT_DIPSETTING(0x0005, "5")
	PORT_DIPSETTING(0x0006, "6")
	PORT_DIPSETTING(0x0007, "7")
	PORT_DIPSETTING(0x0008, "8")
	PORT_DIPSETTING(0x0009, "9")
	PORT_DIPSETTING(0x000a, "a")
	PORT_DIPSETTING(0x000b, "b")
	PORT_DIPSETTING(0x000c, "c")
	PORT_DIPSETTING(0x000d, "d")
	PORT_DIPSETTING(0x000e, "e")
	PORT_DIPSETTING(0x000f, "f")
INPUT_PORTS_END


ioport_constructor cadr_iob_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(keyboard);
}


void cadr_iob_device::device_start()
{
	save_item(NAME(m_p1));
	save_item(NAME(m_bus));
	save_item(NAME(m_keyboard_data));
	save_item(NAME(m_csr));
	save_item(NAME(m_speaker_data));
	save_item(NAME(m_microsecond_clock_buffer));
	save_item(NAME(m_clock));
	save_item(NAME(m_chaos_csr));
	save_item(NAME(m_chaos_transmit_pointer));
	save_item(NAME(m_chaos_transmit_buffer));
	save_item(NAME(m_chaos_receive_size));
	save_item(NAME(m_chaos_receive_pointer));
	save_item(NAME(m_chaos_receive_buffer));
	save_item(NAME(m_chaos_receive_bit_count));

	m_clock_timer = timer_alloc(FUNC(cadr_iob_device::clock_callback), this);
	m_transmit_timer = timer_alloc(FUNC(cadr_iob_device::transmit_callback), this);
}


TIMER_CALLBACK_MEMBER(cadr_iob_device::clock_callback)
{
	m_csr |= CSR_CLOCK_READY;
	if (BIT(m_csr, CSR_CLOCK_IRQ_ENABLE_BIT))
	{
		m_irq_vector_cb(IRQ_VECTOR_CLOCK);
	}
}


void cadr_iob_device::device_reset()
{
	m_p1 = 0;
	m_bus = 0xffffffff;
	m_csr = 0;
	m_microsecond_clock_buffer = 0;
	m_clock = 0;
	m_chaos_csr = CHAOSNET_TRANSMIT_DONE;
	m_speaker_data = 0;
	m_speaker->level_w(m_speaker_data);
	m_chaos_transmit_pointer = 0;
	m_chaos_receive_pointer = 0;
	m_chaos_receive_size = 0;
	m_chaos_receive_bit_count = 0xffff;
}


void cadr_iob_device::chaos_transmit_start()
{
	// Add our source address
	m_chaos_transmit_buffer[m_chaos_transmit_pointer] = m_my_chaos_address->read();
	m_chaos_transmit_pointer = (m_chaos_transmit_pointer + 1) % CHAOS_BUFFER_SIZE;
	// TODO Add checksum
	m_chaos_transmit_buffer[m_chaos_transmit_pointer] = 0xDEAD;
	m_chaos_transmit_pointer = (m_chaos_transmit_pointer + 1) % CHAOS_BUFFER_SIZE;

	LOG("Starting transmit of packet\n");
	LOG("Transmitting packet data");
	for (int i = 0; i < m_chaos_transmit_pointer; i++)
	{
		LOG(" %04x", m_chaos_transmit_buffer[i]);
	}
	LOG("\n");

	// TODO set proper timing
	m_transmit_timer->adjust(attotime::from_nsec(500));
}


TIMER_CALLBACK_MEMBER(cadr_iob_device::transmit_callback)
{
	if (BIT(m_chaos_csr, CHAOSNET_LOOPBACK_BIT))
	{
		for (int i = 0; i < m_chaos_transmit_pointer; i++)
		{
			m_chaos_receive_buffer[i] = m_chaos_transmit_buffer[i];
		}

		m_chaos_receive_size = m_chaos_transmit_pointer;
		m_chaos_receive_bit_count = (m_chaos_receive_size * 16) - 1;
		m_chaos_receive_pointer = 0;

		m_chaos_csr |= CHAOSNET_RECEIVE_DONE;
		if (BIT(m_chaos_csr, CHAOSNET_RECEIVE_IRQ_ENABLE_BIT))
		{
			m_irq_vector_cb(IRQ_VECTOR_CHAOS_RECEIVE);
		}
	}
	else
	{
		// TODO transmit the data to the network
	}

	m_chaos_csr |= CHAOSNET_TRANSMIT_DONE;
	m_chaos_transmit_pointer = 0;
	if (BIT(m_chaos_csr, CHAOSNET_TRANSMIT_IRQ_ENABLE_BIT))
	{
		m_irq_vector_cb(IRQ_VECTOR_CHAOS_TRANSMIT);
	}

}


void cadr_iob_device::write(offs_t offset, u16 data)
{
	switch (offset)
	{
	case 0x04:
		m_speaker_data ^= 1;
		m_speaker->level_w(m_speaker_data);
		break;
	case 0x05:
		m_csr = (m_csr & 0xf0) | (data & 0x0f);
		break;
	case 0x0a:
		m_csr &= ~CSR_CLOCK_READY;
		m_clock = data;
		m_clock_timer->adjust(attotime::from_msec(m_clock << 4));
		break;
	case 0x0b: // general purpose i/o
		break;
	case 0x10: // chaosnet csr
		// --x----- -------- Reset receiver, transmitter and interrupt
		// -------x -------- Clear transmitter
		// -------- --x----- Transmit interrupt enable
		// -------- ---x---- Receive interrupt enable
		// -------- ----x--- Clear receiver
		// -------- -----x-- Match any destination
		// -------- ------x- Loopback
		// -------- -------x Timer interrupt enable (not implemented)
		{
			const u16 bits_to_store = CHAOSNET_RECEIVE_IRQ_ENABLE | CHAOSNET_TRANSMIT_IRQ_ENABLE |
				CHAOSNET_ANY_DESTINATION | CHAOSNET_LOOKBACK | CHAOSNET_TIMER_IRQ_ENABLE;
			m_chaos_csr = (m_chaos_csr & ~bits_to_store) | (data & bits_to_store);
		}
		if (BIT(data, CHAOSNET_RESET_RECEIVE_BIT))
		{
			m_chaos_receive_pointer = 0;
			m_chaos_receive_size = 0;
			m_chaos_csr = (m_chaos_csr & ~CHAOSNET_RECEIVE_DONE);
			m_chaos_csr = (m_chaos_csr & ~CHAOSNET_LOST_COUNT);
		}
		if (BIT(data, CHAOSNET_TRANSMIT_IRQ_ENABLE_BIT))
		{
			m_chaos_csr |= CHAOSNET_TRANSMIT_DONE;
		}
		if (BIT(data, CHAOSNET_RESET_TRANSMIT_BIT))
		{
			m_transmit_timer->adjust(attotime::never);
			m_chaos_csr |= CHAOSNET_TRANSMIT_DONE;
		}
		if (BIT(data, CHAOSNET_RESET_BIT))
		{
			m_chaos_receive_pointer = 0;
			m_chaos_receive_size = 0;
			m_transmit_timer->adjust(attotime::never);
			m_chaos_csr |= CHAOSNET_TRANSMIT_DONE;
			m_chaos_csr = m_chaos_csr & ~(CHAOSNET_RESET | CHAOSNET_RECEIVE_DONE | CHAOSNET_RECEIVE_IRQ_ENABLE | CHAOSNET_TRANSMIT_IRQ_ENABLE);
		}
		break;
	case 0x11: // store word in transmit buffer
		m_chaos_transmit_buffer[m_chaos_transmit_pointer] = data;
		m_chaos_transmit_pointer = (m_chaos_transmit_pointer + 1) % CHAOS_BUFFER_SIZE;
		m_chaos_csr &= ~CHAOSNET_TRANSMIT_DONE;
		break;
	default:
		break;
	}
}


u16 cadr_iob_device::read(offs_t offset)
{
	switch (offset)
	{
	case 0x00: // keyboard low
		m_csr &= ~CSR_KEYBOARD_READY;
		return m_keyboard_data & 0xffff;
	case 0x01: // keyboard high
		return m_keyboard_data >> 16;
	case 0x02: // mouse y
		// 0------- --------
		// -x------ -------- - head switch
		// --x----- -------- - middle switch
		// ---x---- -------- - tail switch
		// ----xxxx xxxxxxxx - Y position of the mouse
		m_csr &= ~CSR_MOUSE_READY;
		return ((m_mouse_buttons->read() & 0x07) << 12) | (m_mouse_y->read() & 0xfff);
	case 0x03: // mouse x
		// xx------ -------- - raw Y encoder inputs
		// --xx---- -------- - raw X encoder inputs
		// ----xxxx xxxxxxxx - X position of the mouse
		return m_mouse_x->read() & 0xfff;
	case 0x04: // beeper
		m_speaker_data ^= 1;
		m_speaker->level_w(m_speaker_data);
		break;
	case 0x05:
		return m_csr;
	case 0x08: // microsecond counter low
		m_microsecond_clock_buffer = machine().time().as_ticks(1e6);
		return m_microsecond_clock_buffer & 0xffff;
	case 0x09: // microsecond counter high
		return m_microsecond_clock_buffer >> 16;
	case 0x0a: // 60hz clock
		return machine().time().as_ticks(60) & 0xffff;
	case 0x0b: // general purpose i/o
		break;
	case 0x10: // chaos net csr 764140
		// x------- -------- Receive done
		// -x------ -------- CRC error
		// ---xxxx- -------- Lost count
		// -------- x------- Transmit done
		// -------- -x------ Transmit aborted
		// -------- --x----- Transmit interrupt enable
		// -------- ---x---- Receive interrupt enable
		// -------- -----x-- Match any destination
		// -------- ------x- Loopback
		// -------- -------x Timer interrupt enable (not implemented)
		return m_chaos_csr;
	case 0x11: // chaos net my address
		return m_my_chaos_address->read();
	case 0x12: // next word from receive buffer
		{
			const u16 data = m_chaos_receive_buffer[m_chaos_receive_pointer];
			m_chaos_receive_pointer = (m_chaos_receive_pointer + 1) % CHAOS_BUFFER_SIZE;
			return data;
		}
	case 0x13: // count of bits remaining in the receive buffer
		if (m_chaos_receive_pointer < m_chaos_receive_size)
		{
			return m_chaos_receive_bit_count;
		}
		return 0xffff;
	case 0x15: // host number of this interface
		chaos_transmit_start();
		return m_my_chaos_address->read();
	}
	return 0xffff;
}


INPUT_CHANGED_MEMBER(cadr_iob_device::mouse_changed)
{
	m_csr |= 0x10;
	if (BIT(m_csr, CSR_MOUSE_IRQ_ENABLE_BIT))
	{
		m_irq_vector_cb(IRQ_VECTOR_MOUSE);
	}
}


u8 cadr_iob_device::mcu_bus_r()
{
	// bit 0 is checked by mcu (data/clock?)
	return 0xff;
}


void cadr_iob_device::mcu_bus_w(u8 data)
{
	switch (m_p1 & 0x60)
	{
	case 0x20:
		m_bus = (m_bus & 0xffff00ff) | (data << 8);
		break;
	case 0x40:
		m_bus = (m_bus & 0xffffff00) | data;
		m_keyboard_data = (m_bus >> 1) & 0xffffff;
		m_csr |= CSR_KEYBOARD_READY;
		if (BIT(m_csr, CSR_KEYBOARD_IRQ_ENABLE_BIT))
		{
			m_irq_vector_cb(IRQ_VECTOR_KEYBOARD);
		}
		break;
	case 0x60:
		m_bus = (m_bus & 0xff00ffff) | (data << 16);
		break;
	}
}


void cadr_iob_device::mcu_p1_w(u8 data)
{
	// Sequence when a key is pressed:
	// - bits 5 and 6 get set, r4 is output to bus,
	// - bit 6 is reset, r3 is output to bus,
	// - bits 5 and 4 are reset, bit 6 is set, r2 is output to bus 
	// examples:
	// 64, bus, 24, bus, 04, 44, bus
	// 70, bus, 30, bus, 10, 50, bus
	// 65: 11, bus, 01, bus, 10, bus

	// Lowest bit is changed separately, to latch data for reading on p2?
	m_p1 = data;
}


u8 cadr_iob_device::mcu_p2_r()
{
	if (!BIT(m_p1, 0))
	{
		return m_keyboard[(m_p1 >> 1) & 0x0f]->read();
	}
	return 0xff;
}

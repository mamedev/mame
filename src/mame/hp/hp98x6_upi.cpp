// license:BSD-3-Clause
// copyright-holders:F. Ulivi

// **** High level emulation of 8041 UPI in HP98X6 systems ****
//
// Configuration jumpers in RAM_POS_CFG_JUMPERS register:
//
// |  Bit | Meaning                                           |
// |------+---------------------------------------------------|
// |    7 | ID PROM is installed                              |
// |    6 | UPI revision                                      |
// |    5 | 0: HP-HIL keyboards not supported                 |
// | 4..1 | -                                                 |
// |    0 | Large (0) or small (1) keyboard                   |
//
// Sequence to read ID PROM:
//
// 68k          UPI
// ===========================================
// Cmd C1 ->    Set ID PROM address = 0
// Cmd 01 ->    Read PROM byte @00, address++
//              <- Byte @00
// Cmd 01 ->    Read PROM byte @01, address++
//              <- Byte @01
// ...
// Cmd C0 ->    Stop reading ID PROM
//
// TODO:
// - Find if FHS and delay timers are self canceling

#include "emu.h"
#include "hp98x6_upi.h"

#include "speaker.h"

// Debugging
#define VERBOSE 0
#include "logmacro.h"


// Bit manipulation
namespace {
	template<typename T> constexpr T BIT_MASK(unsigned n)
	{
		return (T)1U << n;
	}

	template<typename T> void BIT_CLR(T& w , unsigned n)
	{
		w &= ~BIT_MASK<T>(n);
	}

	template<typename T> void BIT_SET(T& w , unsigned n)
	{
		w |= BIT_MASK<T>(n);
	}
}

// Device type definition
DEFINE_DEVICE_TYPE(HP98X6_UPI, hp98x6_upi_device, "hp98x6_upi", "UPI of HP98x6 systems")

// Positions in internal RAM
enum : uint8_t {
	RAM_POS_0_R0,
	RAM_POS_0_R1,
	RAM_POS_0_R2_FLAGS1,
	RAM_POS_0_R3_CURR_KEY,
	RAM_POS_0_R4_FLAGS2,
	RAM_POS_0_R5_FLAGS3,
	RAM_POS_0_R6_ROLLOVER,
	RAM_POS_0_R7_KEY_DOWN,
	RAM_POS_RST_DEB_CNT = 0x10,
	RAM_POS_CFG_JUMPERS,
	RAM_POS_LNG_JUMPERS,
	RAM_POS_OUT_BUFF_1,
	RAM_POS_OUT_BUFF_2,
	RAM_POS_OUT_BUFF_3,
	RAM_POS_OUT_BUFF_4,
	RAM_POS_OUT_BUFF_5,
	RAM_POS_1_R0,
	RAM_POS_1_R1,
	RAM_POS_1_R2,
	RAM_POS_1_R3_TIMER_STS,
	RAM_POS_1_R4_RPG_COUNT,
	RAM_POS_1_R5_W_PTR,
	RAM_POS_1_R6_R_PTR,
	RAM_POS_1_R7_6CTR,
	RAM_POS_HIGH_RAM_START = 0x20,
	RAM_POS_AR_WAIT = 0x20,
	RAM_POS_AR_TIMER,
	RAM_POS_AR_RATE,
	RAM_POS_BEEP_TIMER,
	RAM_POS_BEEP_FREQ,
	RAM_POS_RPG_TIMER,
	RAM_POS_RPG_INT_RATE,
	RAM_POS_TIMER_INT,
	RAM_POS_READING_PROM,       // Not in HP doc, arbitrarily used for PROM reading
	RAM_POS_PROM_ADDR,          // Not in HP doc, arbitrarily used for PROM reading
	RAM_POS_TOD_1 = 0x2d,
	RAM_POS_TOD_2,
	RAM_POS_TOD_3,
	RAM_POS_DAY_1,
	RAM_POS_DAY_2,
	RAM_POS_FHS_1,
	RAM_POS_FHS_2,
	RAM_POS_MATCH_1,
	RAM_POS_MATCH_2,
	RAM_POS_MATCH_3,
	RAM_POS_DELAY_1,
	RAM_POS_DELAY_2,
	RAM_POS_DELAY_3,
	RAM_POS_CYCLE_1,
	RAM_POS_CYCLE_2,
	RAM_POS_CYCLE_3,
	RAM_POS_CYCLE_SAVE_1,
	RAM_POS_CYCLE_SAVE_2,
	RAM_POS_CYCLE_SAVE_3
};

// Clocks per 10 ms
constexpr unsigned CLOCKS_PER_10MS = 50000;

// Delays (in device clocks)
// They are all multiples of 15 as that's the basic instruction cycle of 8041
constexpr unsigned POR_DELAY1 = 105;
constexpr unsigned POR_DELAY2 = 100005; // This is a wild guess, real 8041 is mostly checksumming its ROM here
constexpr unsigned RESET_DELAY = 7500;
constexpr unsigned INPUT_DELAY = 495;   // Sometimes 68k doesn't like instantaneous consumption of cmd/data bytes

// Bits in status register
constexpr unsigned STATUS_F1_BIT = 3;
constexpr unsigned STATUS_F0_BIT = 2;
constexpr unsigned STATUS_IBF_BIT = 1;
constexpr unsigned STATUS_OBF_BIT = 0;
constexpr uint8_t ST_PSI = 0x10;
constexpr uint8_t ST_TIMER = 0x20;
constexpr uint8_t ST_REQUESTED_DATA = 0x40;
constexpr uint8_t ST_POST_OK = 0x70;
constexpr uint8_t ST_KEY = 0x80;
constexpr uint8_t ST_RPG = 0xc0;

// Bits in RAM_POS_0_R2_FLAGS1
constexpr uint8_t R2_FLAGS1_DEB_MASK = 0x07;
constexpr uint8_t R2_FLAGS1_DEB_INIT = 0x02;
constexpr unsigned R2_FLAGS1_FHS_BIT = 3;
constexpr unsigned R2_FLAGS1_CYCLE_BIT = 4;
constexpr unsigned R2_FLAGS1_DELAY_BIT = 5;
constexpr unsigned R2_FLAGS1_MATCH_BIT = 6;
constexpr unsigned R2_FLAGS1_BEEP_BIT = 7;

// Bits in RAM_POS_0_R4_FLAGS2
constexpr unsigned R4_FLAGS2_AR_BIT = 5;
constexpr unsigned R4_FLAGS2_FHS_MASK_BIT = 4;
constexpr unsigned R4_FLAGS2_PSI_MASK_BIT = 3;
constexpr unsigned R4_FLAGS2_TMR_MASK_BIT = 2;
constexpr unsigned R4_FLAGS2_RST_MASK_BIT = 1;
constexpr unsigned R4_FLAGS2_KEY_MASK_BIT = 0;

// Bits in RAM_POS_0_R5_FLAGS3
constexpr unsigned R5_FLAGS3_FHS_BIT = 7;
constexpr unsigned R5_FLAGS3_READ_BIT = 5;
constexpr unsigned R5_FLAGS3_RPG_BIT = 4;
constexpr unsigned R5_FLAGS3_USER_TMR_BIT = 3;
constexpr unsigned R5_FLAGS3_PSI_BIT = 2;
constexpr unsigned R5_FLAGS3_CTRL_UP_BIT = 1;
constexpr unsigned R5_FLAGS3_SHIFT_UP_BIT = 0;

// Bits in RAM_POS_CFG_JUMPERS
constexpr unsigned CFG_JUMPERS_PROM_BIT = 7;

// Bits in RAM_POS_1_R3_TIMER_STS
constexpr unsigned R3_TIMER_STS_MATCH_BIT = 7;
constexpr unsigned R3_TIMER_STS_DELAY_BIT = 6;
constexpr unsigned R3_TIMER_STS_CYCLE_BIT = 5;
constexpr uint8_t R3_TIMER_STS_INT_MASK = 0xe0;
constexpr uint8_t R3_TIMER_STS_MISSED_CYCLES_MASK = 0x1f;

// Commands
constexpr uint8_t CMD_MASK = 0xe0;
constexpr uint8_t CMD_PARAM = 0x1f;
constexpr uint8_t CMD_RD_RAM_LOW = 0x00;
constexpr uint8_t CMD_RD_RAM_HIGH = 0x20;
constexpr uint8_t CMD_SET_INT_MASK = 0x40;
constexpr uint8_t CMD_WR_RAM_HIGH = 0xa0;
constexpr uint8_t CMD_RD_PROM_STOP = 0xc0;
constexpr uint8_t CMD_RD_PROM_START = 0xc1;

// Scancode range
constexpr uint8_t SCANCODE_NONE = 0;
constexpr uint8_t MIN_SCANCODE = 0x18;
constexpr uint8_t MAX_SCANCODE = 0x7f;

// Various constants
constexpr uint8_t POST_BYTE = 0x8e;
constexpr uint8_t TOD_1_ONE_DAY = 0;    // One day is 0x83d600 (8'640'000) in TOD counts
constexpr uint8_t TOD_2_ONE_DAY = 0xd6;
constexpr uint8_t TOD_3_ONE_DAY = 0x83;
constexpr unsigned BEEP_SCALING = 15 * 64 * 64;
constexpr uint8_t PAUSE_SCANCODE = 0x38;    // Reset is Shift+Pause

hp98x6_upi_device::hp98x6_upi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
: device_t(mconfig, HP98X6_UPI, tag, owner, clock)
	, m_keys(*this, "KEY%u", 0)
	, m_shift(*this, "KEY_SHIFT")
	, m_dial(*this, "DIAL")
	, m_beep(*this, "beep")
	, m_10ms_timer(*this, "timer")
	, m_delay_timer(*this, "dly")
	, m_input_delay_timer(*this, "inp_dly")
	, m_idprom(*this, "idprom")
	, m_irq1_write_func(*this)
	, m_irq7_write_func(*this)
{
}

uint8_t hp98x6_upi_device::read(offs_t offset)
{
	uint8_t res;

	if (BIT(offset, 0)) {
		res = m_status;
	} else {
		res = m_data_out;
		BIT_CLR(m_status, STATUS_OBF_BIT);
		m_irq1_write_func(false);
		try_output();
	}
	return res;
}

void hp98x6_upi_device::write(offs_t offset, uint8_t data)
{
	m_data_in = data;
	if (!BIT(m_status, STATUS_IBF_BIT)) {
		m_ready = false;
		m_input_delay_timer->adjust(clocks_to_attotime(INPUT_DELAY));
		BIT_SET(m_status, STATUS_IBF_BIT);
	}
	if (BIT(offset, 0)) {
		BIT_SET(m_status, STATUS_F1_BIT);
	} else {
		BIT_CLR(m_status, STATUS_F1_BIT);
	}
}

void hp98x6_upi_device::device_add_mconfig(machine_config &config)
{
	// Beep
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 0).add_route(ALL_OUTPUTS, "mono", 1.00);

	TIMER(config, m_10ms_timer).configure_periodic(FUNC(hp98x6_upi_device::ten_ms), clocks_to_attotime(CLOCKS_PER_10MS));
	TIMER(config, m_delay_timer).configure_generic(FUNC(hp98x6_upi_device::delay));
	TIMER(config, m_input_delay_timer).configure_generic(FUNC(hp98x6_upi_device::input_delay));
}

#define IOP_MASK(x) BIT_MASK<ioport_value>((x))

static INPUT_PORTS_START(hp98x6_upi_ports)
	PORT_START("KEY0")
	PORT_BIT(IOP_MASK(0) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                             // 0,0: N/U
	PORT_BIT(IOP_MASK(1) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                             // 0,1: N/U
	PORT_BIT(IOP_MASK(2) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                             // 0,2: N/U
	PORT_BIT(IOP_MASK(3) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_NAME("CAPS LOCK")                                        // 0,3: Caps lock
	PORT_BIT(IOP_MASK(4) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_NAME("k3")                        // 0,4: k3
	PORT_BIT(IOP_MASK(5) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("INSLN")                                                                        // 0,5: Insln
	PORT_BIT(IOP_MASK(6) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("EDIT")                                                                         // 0,6: Edit
	PORT_BIT(IOP_MASK(7) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("PAUSE")                                                                        // 0,7: Pause
	PORT_BIT(IOP_MASK(8) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))                                  // 0,8: KP 1
	PORT_BIT(IOP_MASK(9) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))                                  // 0,9: KP 7
	PORT_BIT(IOP_MASK(10) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')                                       // 0,10: 1
	PORT_BIT(IOP_MASK(11) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')                                       // 0,11: 9
	PORT_BIT(IOP_MASK(12) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')                                   // 0,12: , <
	PORT_BIT(IOP_MASK(13) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')                                       // 0,13: Q
	PORT_BIT(IOP_MASK(14) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')                                       // 0,14: A
	PORT_BIT(IOP_MASK(15) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')                                       // 0,15: Z
	PORT_BIT(IOP_MASK(16) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                            // 1,0: N/U
	PORT_BIT(IOP_MASK(17) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                            // 1,1: N/U
	PORT_BIT(IOP_MASK(18) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                            // 1,2: N/U
	PORT_BIT(IOP_MASK(19) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')                                                   // 1,3: Tab
	PORT_BIT(IOP_MASK(20) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_NAME("k4")                       // 1,4: k4
	PORT_BIT(IOP_MASK(21) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("DELLN")                                                                       // 1,5: Delln
	PORT_BIT(IOP_MASK(22) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("ALPHA")                                                                       // 1,6: Alpha
	PORT_BIT(IOP_MASK(23) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("ENTER") PORT_CHAR(13)                                // 1,7: Enter
	PORT_BIT(IOP_MASK(24) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))                                 // 1,8: KP 2
	PORT_BIT(IOP_MASK(25) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))                                 // 1,9: KP 8
	PORT_BIT(IOP_MASK(26) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')                                       // 1,10: 2
	PORT_BIT(IOP_MASK(27) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')                                       // 1,11: 0
	PORT_BIT(IOP_MASK(28) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')                                    // 1,12: .
	PORT_BIT(IOP_MASK(29) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')                                       // 1,13: W
	PORT_BIT(IOP_MASK(30) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')                                       // 1,14: S
	PORT_BIT(IOP_MASK(31) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')                                       // 1,15: X

	PORT_START("KEY1")
	PORT_BIT(IOP_MASK(0) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                             // 2,0: N/U
	PORT_BIT(IOP_MASK(1) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                             // 2,1: N/U
	PORT_BIT(IOP_MASK(2) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                             // 2,2: N/U
	PORT_BIT(IOP_MASK(3) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("k0")                                                                           // 2,3: k0
	PORT_BIT(IOP_MASK(4) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))                                    // 2,4: Down
	PORT_BIT(IOP_MASK(5) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("RECALL")                                                                       // 2,5: Recall
	PORT_BIT(IOP_MASK(6) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("GRAPHICS")                                                                     // 2,6: Graphics
	PORT_BIT(IOP_MASK(7) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("CONTINUE")                                                                     // 2,7: Continue
	PORT_BIT(IOP_MASK(8) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))                                  // 2,8: KP 3
	PORT_BIT(IOP_MASK(9) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))                                  // 2,9: KP 9
	PORT_BIT(IOP_MASK(10) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')                                       // 2,10: 3
	PORT_BIT(IOP_MASK(11) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')                                   // 2,11: -
	PORT_BIT(IOP_MASK(12) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')                                   // 2,12: /
	PORT_BIT(IOP_MASK(13) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')                                       // 2,13: E
	PORT_BIT(IOP_MASK(14) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')                                       // 2,14: D
	PORT_BIT(IOP_MASK(15) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')                                       // 2,15: C
	PORT_BIT(IOP_MASK(16) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                            // 3,0: N/U
	PORT_BIT(IOP_MASK(17) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                            // 3,1: N/U
	PORT_BIT(IOP_MASK(18) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                            // 3,2: N/U
	PORT_BIT(IOP_MASK(19) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_NAME("k1")                       // 3,3: k1
	PORT_BIT(IOP_MASK(20) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))                                       // 3,4: Up
	PORT_BIT(IOP_MASK(21) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("INSCHR")                                                                      // 3,5: Inschr
	PORT_BIT(IOP_MASK(22) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("STEP")                                                                        // 3,6: Step
	PORT_BIT(IOP_MASK(23) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("EXECUTE")                                                                     // 3,7: Execute
	PORT_BIT(IOP_MASK(24) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))                         // 3,8: KP -
	PORT_BIT(IOP_MASK(25) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))                         // 3,9: KP /
	PORT_BIT(IOP_MASK(26) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')                                       // 3,10: 4
	PORT_BIT(IOP_MASK(27) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')                                  // 3,11: =
	PORT_BIT(IOP_MASK(28) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')                                                  // 3,12: Space
	PORT_BIT(IOP_MASK(29) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')                                       // 3,13: R
	PORT_BIT(IOP_MASK(30) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')                                       // 3,14: F
	PORT_BIT(IOP_MASK(31) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')                                       // 3,15: V

	PORT_START("KEY2")
	PORT_BIT(IOP_MASK(0) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                             // 4,0: N/U
	PORT_BIT(IOP_MASK(1) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                             // 4,1: N/U
	PORT_BIT(IOP_MASK(2) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                             // 4,2: N/U
	PORT_BIT(IOP_MASK(3) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_NAME("k2")                        // 4,3: k2
	PORT_BIT(IOP_MASK(4) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8)) PORT_NAME("k8")                        // 4,4: k4
	PORT_BIT(IOP_MASK(5) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("DELCHR")                                                                       // 4,5: Delchr
	PORT_BIT(IOP_MASK(6) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("CLRLN")                                                                        // 4,6: Clrln
	PORT_BIT(IOP_MASK(7) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))                                  // 4,7: KP 0
	PORT_BIT(IOP_MASK(8) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))                                  // 4,8: KP 4
	PORT_BIT(IOP_MASK(9) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("Keypad E")                                                                     // 4,9: KP E
	PORT_BIT(IOP_MASK(10) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')                                       // 4,10: 5
	PORT_BIT(IOP_MASK(11) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')                               // 4,11: [
	PORT_BIT(IOP_MASK(12) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')                                       // 4,12: O
	PORT_BIT(IOP_MASK(13) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')                                       // 4,13: T
	PORT_BIT(IOP_MASK(14) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')                                       // 4,14: G
	PORT_BIT(IOP_MASK(15) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')                                       // 4,15: B
	PORT_BIT(IOP_MASK(16) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                            // 5,0: N/U
	PORT_BIT(IOP_MASK(17) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                            // 5,1: N/U
	PORT_BIT(IOP_MASK(18) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                            // 5,2: N/U
	PORT_BIT(IOP_MASK(19) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_NAME("k5")                       // 5,3: k5
	PORT_BIT(IOP_MASK(20) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9)) PORT_NAME("k9")                       // 5,4: k9
	PORT_BIT(IOP_MASK(21) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_END) PORT_NAME("CLR TO END")                                           // 5,5: Clr->endif
	PORT_BIT(IOP_MASK(22) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("RESULT")                                                                      // 5,6: Result
	PORT_BIT(IOP_MASK(23) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))                             // 5,7: KP .
	PORT_BIT(IOP_MASK(24) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))                                 // 5,8: KP 5
	PORT_BIT(IOP_MASK(25) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("Keypad (")                                                                    // 5,9: KP (
	PORT_BIT(IOP_MASK(26) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')                                       // 5,10: 6
	PORT_BIT(IOP_MASK(27) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')                              // 5,11: ]
	PORT_BIT(IOP_MASK(28) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')                                       // 5,12: P
	PORT_BIT(IOP_MASK(29) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')                                       // 5,13: Y
	PORT_BIT(IOP_MASK(30) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')                                       // 5,14: H
	PORT_BIT(IOP_MASK(31) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')                                       // 5,15: N

	PORT_START("KEY3")
	PORT_BIT(IOP_MASK(0) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                             // 6,0: N/U
	PORT_BIT(IOP_MASK(1) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                             // 6,1: N/U
	PORT_BIT(IOP_MASK(2) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                             // 6,2: N/U
	PORT_BIT(IOP_MASK(3) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6)) PORT_NAME("k6")                        // 6,3: k6
	PORT_BIT(IOP_MASK(4) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))                                    // 6,4: Left
	PORT_BIT(IOP_MASK(5) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)                                                 // 6,5: Back space
	PORT_BIT(IOP_MASK(6) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("PRT ALL")                                                                      // 6,6: Prtall
	PORT_BIT(IOP_MASK(7) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA_PAD) PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD))                          // 6,7: KP ,
	PORT_BIT(IOP_MASK(8) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))                                  // 6,8: KP 6
	PORT_BIT(IOP_MASK(9) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("Keypad )")                                                                     // 6,9: KP )
	PORT_BIT(IOP_MASK(10) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')                                       // 6,10: 7
	PORT_BIT(IOP_MASK(11) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')                                   // 6,11: ;
	PORT_BIT(IOP_MASK(12) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')                                       // 6,12: K
	PORT_BIT(IOP_MASK(13) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')                                       // 6,13: U
	PORT_BIT(IOP_MASK(14) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')                                       // 6,14: J
	PORT_BIT(IOP_MASK(15) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                            // 6,15: N/U
	PORT_BIT(IOP_MASK(16) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                            // 7,0: N/U
	PORT_BIT(IOP_MASK(17) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                            // 7,1: N/U
	PORT_BIT(IOP_MASK(18) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                            // 7,2: N/U
	PORT_BIT(IOP_MASK(19) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7)) PORT_NAME("k7")                       // 7,3: k7
	PORT_BIT(IOP_MASK(20) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))                                 // 7,4: Right
	PORT_BIT(IOP_MASK(21) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("RUN")                                                                         // 7,5: Run
	PORT_BIT(IOP_MASK(22) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("CLR I/O")                                                                     // 7,6: Clr I/O
	PORT_BIT(IOP_MASK(23) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))                           // 7,7: KP +
	PORT_BIT(IOP_MASK(24) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))                           // 7,8: KP *
	PORT_BIT(IOP_MASK(25) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_NAME("Keypad ^")                                                                    // 7.9: KP ^
	PORT_BIT(IOP_MASK(26) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')                                       // 7,10: 8
	PORT_BIT(IOP_MASK(27) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"')                                  // 7,11: '
	PORT_BIT(IOP_MASK(28) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')                                       // 7,12: L
	PORT_BIT(IOP_MASK(29) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')                                       // 7,13: I
	PORT_BIT(IOP_MASK(30) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')                                       // 7,14: M
	PORT_BIT(IOP_MASK(31) , IP_ACTIVE_HIGH , IPT_UNUSED)                                                                                            // 7,15: N/U

	PORT_START("KEY_SHIFT")
	PORT_BIT(IOP_MASK(0) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1) // Left and right Shift
	PORT_BIT(IOP_MASK(1) , IP_ACTIVE_HIGH , IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2) // Control

	PORT_START("DIAL")
	PORT_BIT(0xff, 0x00, IPT_DIAL) PORT_SENSITIVITY(30) PORT_KEYDELTA(15)

INPUT_PORTS_END

ioport_constructor hp98x6_upi_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(hp98x6_upi_ports);
}

void hp98x6_upi_device::device_start()
{
	save_item(NAME(m_ram));
	save_item(NAME(m_data_in));
	save_item(NAME(m_data_out));
	save_item(NAME(m_status));
	save_item(NAME(m_ready));
}

void hp98x6_upi_device::device_reset()
{
	m_irq1_write_func(true);
	m_irq7_write_func(false);

	m_last_dial = m_dial->read();
	m_beep->set_state(0);
	m_ram[RAM_POS_0_R2_FLAGS1] = 0;
	// Mask out all interrupts
	m_ram[RAM_POS_0_R4_FLAGS2] =
		BIT_MASK<uint8_t>(R4_FLAGS2_FHS_MASK_BIT) |
		BIT_MASK<uint8_t>(R4_FLAGS2_PSI_MASK_BIT) |
		BIT_MASK<uint8_t>(R4_FLAGS2_TMR_MASK_BIT) |
		BIT_MASK<uint8_t>(R4_FLAGS2_RST_MASK_BIT) |
		BIT_MASK<uint8_t>(R4_FLAGS2_KEY_MASK_BIT);
	m_ram[RAM_POS_0_R5_FLAGS3] = 0;
	m_ram[RAM_POS_0_R6_ROLLOVER] = SCANCODE_NONE;
	m_ram[RAM_POS_0_R7_KEY_DOWN] = SCANCODE_NONE;
	// Assume RESET key is down
	m_ram[RAM_POS_RST_DEB_CNT] = R2_FLAGS1_DEB_INIT;
	// PROM is present if m_idprom != nullptr
	m_ram[RAM_POS_CFG_JUMPERS] = bool(m_idprom) ? BIT_MASK<uint8_t>(CFG_JUMPERS_PROM_BIT) : 0;
	// Assume US English
	m_ram[RAM_POS_LNG_JUMPERS] = 0;
	m_ram[RAM_POS_1_R3_TIMER_STS] = 0;
	m_ram[RAM_POS_1_R4_RPG_COUNT] = 0;
	m_ram[RAM_POS_1_R5_W_PTR] = 0;
	m_ram[RAM_POS_READING_PROM] = 0;

	// Note that RAM is not cleared to avoid losing TOD after an UPI reset

	m_status = 0;
	m_ready = false;
	m_fsm_state = fsm_st::ST_POR_TEST1;
	m_delay_timer->adjust(clocks_to_attotime(POR_DELAY1));
	m_input_delay_timer->reset();
}

TIMER_DEVICE_CALLBACK_MEMBER(hp98x6_upi_device::ten_ms)
{
	ten_ms_update_key();
	ten_ms_update_dial();
	ten_ms_update_timers();
	ten_ms_update_beep();
	try_output();
}

TIMER_DEVICE_CALLBACK_MEMBER(hp98x6_upi_device::delay)
{
	switch (m_fsm_state) {
	case fsm_st::ST_POR_TEST1:
		m_irq1_write_func(false);
		m_fsm_state = fsm_st::ST_POR_TEST2;
		m_delay_timer->adjust(clocks_to_attotime(POR_DELAY2));
		break;

	case fsm_st::ST_POR_TEST2:
		write_ob_st(POST_BYTE, ST_POST_OK);
		m_fsm_state = fsm_st::ST_IDLE;
		update_fsm();
		break;

	case fsm_st::ST_RESETTING:
		// Send NMI to 68k
		m_irq7_write_func(true);
		// F0 = 0 means "NMI from RESET key"
		BIT_CLR(m_status, STATUS_F0_BIT);
		m_fsm_state = fsm_st::ST_IDLE;
		update_fsm();
		break;

	default:
		break;
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(hp98x6_upi_device::input_delay)
{
	m_ready = true;
	update_fsm();
}

void hp98x6_upi_device::write_ob_st(uint8_t data, uint8_t st)
{
	LOG("UPI out %02x ST=%02x\n", data, st);
	m_data_out = data;
	BIT_SET(m_status, STATUS_OBF_BIT);
	m_irq1_write_func(true);
	m_status = (m_status & 0x0f) | (st & 0xf0);
}

bool hp98x6_upi_device::read_ib(uint8_t &data)
{
	data = m_data_in;
	bool res = BIT(m_status, STATUS_IBF_BIT);
	BIT_CLR(m_status, STATUS_IBF_BIT);
	return res;
}

void hp98x6_upi_device::update_fsm()
{
	uint8_t in_data;

	// Check for incoming command or data
	if (m_fsm_state == fsm_st::ST_IDLE &&
		m_ready &&
		read_ib(in_data)) {
		if (BIT(m_status, STATUS_F1_BIT)) {
			// Command
			LOG("UPI cmd %02x\n", in_data);
			decode_cmd(in_data);
		} else if (m_ram[RAM_POS_1_R5_W_PTR]) {
			// Data
			LOG("UPI data %02x\n", in_data);
			uint8_t w_ptr = m_ram[RAM_POS_1_R5_W_PTR];
			if (w_ptr >= RAM_POS_TOD_1 && w_ptr <= RAM_POS_TOD_3) {
				// Data written to TOD are summed in, not just stored
				uint8_t inc[] = { 0, 0, 0 };
				inc[w_ptr - RAM_POS_TOD_1] = in_data;
				(void)add_to_ctr(RAM_POS_TOD_1, 3, inc);
			} else {
				m_ram[w_ptr] = in_data;
			}
			// Additional actions triggered by writing at the end of
			// various counters
			switch (w_ptr) {
			case RAM_POS_BEEP_FREQ:
				// Start/stop beep
				if (in_data != 0) {
					m_beep->set_clock((clock() * in_data) / BEEP_SCALING);
					m_beep->set_state(1);
					BIT_SET(m_ram[RAM_POS_0_R2_FLAGS1], R2_FLAGS1_BEEP_BIT);
				} else {
					m_beep->set_state(0);
					BIT_CLR(m_ram[RAM_POS_0_R2_FLAGS1], R2_FLAGS1_BEEP_BIT);
				}
				break;
			case RAM_POS_RPG_INT_RATE:
				m_ram[RAM_POS_RPG_TIMER] = m_ram[RAM_POS_RPG_INT_RATE];
				break;
			case RAM_POS_FHS_2:
				BIT_SET(m_ram[RAM_POS_0_R2_FLAGS1], R2_FLAGS1_FHS_BIT);
				break;
			case RAM_POS_MATCH_3:
				BIT_SET(m_ram[RAM_POS_0_R2_FLAGS1], R2_FLAGS1_MATCH_BIT);
				break;
			case RAM_POS_DELAY_3:
				BIT_SET(m_ram[RAM_POS_0_R2_FLAGS1], R2_FLAGS1_DELAY_BIT);
				break;
			case RAM_POS_CYCLE_3:
				BIT_SET(m_ram[RAM_POS_0_R2_FLAGS1], R2_FLAGS1_CYCLE_BIT);
				m_ram[RAM_POS_CYCLE_SAVE_1] = m_ram[RAM_POS_CYCLE_1];
				m_ram[RAM_POS_CYCLE_SAVE_2] = m_ram[RAM_POS_CYCLE_2];
				m_ram[RAM_POS_CYCLE_SAVE_3] = m_ram[RAM_POS_CYCLE_3];
				break;
			}

			// Increment write pointer
			w_ptr++;
			if (w_ptr > 0x3f) {
				LOG("Write pointer overflow\n");
			} else {
				m_ram[RAM_POS_1_R5_W_PTR] = w_ptr;
			}
		} else {
			// Not writing, data not expected
			LOG("Unexpected data %02x\n", in_data);
		}
		// Try to send anything that's waiting to be sent
		try_output();
	}
}

void hp98x6_upi_device::decode_cmd(uint8_t cmd)
{
	// Not writing to RAM unless we get the write command
	m_ram[RAM_POS_1_R5_W_PTR] = 0;

	// Decode command categories first
	switch (cmd & CMD_MASK) {
	case CMD_RD_RAM_LOW:
		// 000x'xxxx
		// Read low RAM
		m_ram[RAM_POS_1_R6_R_PTR] = cmd & CMD_PARAM;
		// Read from ID PROM when enabled
		if (m_ram[RAM_POS_READING_PROM] &&
			m_ram[RAM_POS_1_R6_R_PTR] == RAM_POS_0_R1) {
			m_ram[RAM_POS_0_R1] = m_idprom[m_ram[RAM_POS_PROM_ADDR]];
			LOG("PROM @%02x=%02x\n", m_ram[RAM_POS_PROM_ADDR], m_ram[RAM_POS_0_R1]);
			m_ram[RAM_POS_PROM_ADDR]++;
		}
		BIT_SET(m_ram[RAM_POS_0_R5_FLAGS3], R5_FLAGS3_READ_BIT);
		break;
	case CMD_RD_RAM_HIGH:
		// 001x'xxxx
		// Read high RAM
		{
			unsigned idx = RAM_POS_HIGH_RAM_START - 4 + (cmd & CMD_PARAM);
			m_ram[RAM_POS_OUT_BUFF_1] = m_ram[idx++];
			m_ram[RAM_POS_OUT_BUFF_2] = m_ram[idx++];
			m_ram[RAM_POS_OUT_BUFF_3] = m_ram[idx++];
			m_ram[RAM_POS_OUT_BUFF_4] = m_ram[idx++];
			m_ram[RAM_POS_OUT_BUFF_5] = m_ram[idx];
		}
		break;
	case CMD_SET_INT_MASK:
		// 010x'xxxx
		// Set interrupt mask
		m_ram[RAM_POS_0_R4_FLAGS2] &= ~CMD_PARAM;
		m_ram[RAM_POS_0_R4_FLAGS2] |= (cmd & CMD_PARAM);
		try_fhs_output();
		break;
	case CMD_WR_RAM_HIGH:
		// 101x'xxxx
		// Write high RAM
		m_ram[RAM_POS_1_R5_W_PTR] = RAM_POS_HIGH_RAM_START + (cmd & CMD_PARAM);
		break;
	default:
		// Decode single commands
		switch (cmd) {
		case CMD_RD_PROM_START:
			// 1100'0001
			// Start reading ID PROM
			if (bool(m_idprom)) {
				LOG("Start PROM read\n");
				m_ram[RAM_POS_READING_PROM] = 1;
				m_ram[RAM_POS_PROM_ADDR] = 0;
			} else {
				LOG("Attempt to read from non-existing PROM\n");
			}
			break;
		case CMD_RD_PROM_STOP:
			// 1100'0000
			// Stop reading ID PROM
			LOG("Stop PROM read\n");
			m_ram[RAM_POS_READING_PROM] = 0;
			break;
		default:
			LOG("Unknown UPI cmd %02x\n", cmd);
			break;
		}
	}

	// Then decode write commands that require additional actions
	switch (m_ram[RAM_POS_1_R5_W_PTR]) {
	case RAM_POS_TOD_1:
		// Set time-of-day
		m_ram[RAM_POS_TOD_1] = 0;
		m_ram[RAM_POS_TOD_2] = 0;
		m_ram[RAM_POS_TOD_3] = 0;
		break;
	case RAM_POS_DAY_1 - 1:
		// Set day
		m_ram[RAM_POS_1_R5_W_PTR] = RAM_POS_DAY_1;
		break;
	case RAM_POS_FHS_1:
		// Set FHS
		BIT_CLR(m_ram[RAM_POS_0_R2_FLAGS1], R2_FLAGS1_FHS_BIT);
		BIT_CLR(m_ram[RAM_POS_0_R5_FLAGS3], R5_FLAGS3_FHS_BIT);
		m_irq7_write_func(false);
		break;
	case RAM_POS_MATCH_1:
		BIT_CLR(m_ram[RAM_POS_0_R2_FLAGS1], R2_FLAGS1_MATCH_BIT);
		BIT_CLR(m_ram[RAM_POS_1_R3_TIMER_STS], R3_TIMER_STS_MATCH_BIT);
		if ((m_ram[RAM_POS_1_R3_TIMER_STS] & R3_TIMER_STS_INT_MASK) == 0) {
			BIT_CLR(m_ram[RAM_POS_0_R5_FLAGS3], R5_FLAGS3_USER_TMR_BIT);
		}
		break;
	case RAM_POS_DELAY_1:
		BIT_CLR(m_ram[RAM_POS_0_R2_FLAGS1], R2_FLAGS1_DELAY_BIT);
		BIT_CLR(m_ram[RAM_POS_1_R3_TIMER_STS], R3_TIMER_STS_DELAY_BIT);
		if ((m_ram[RAM_POS_1_R3_TIMER_STS] & R3_TIMER_STS_INT_MASK) == 0) {
			BIT_CLR(m_ram[RAM_POS_0_R5_FLAGS3], R5_FLAGS3_USER_TMR_BIT);
		}
		break;
	case RAM_POS_CYCLE_1:
		BIT_CLR(m_ram[RAM_POS_0_R2_FLAGS1], R2_FLAGS1_CYCLE_BIT);
		m_ram[RAM_POS_1_R3_TIMER_STS] &= ~(R3_TIMER_STS_MISSED_CYCLES_MASK |
											 BIT_MASK<uint8_t>(R3_TIMER_STS_CYCLE_BIT));
		if ((m_ram[RAM_POS_1_R3_TIMER_STS] & R3_TIMER_STS_INT_MASK) == 0) {
			BIT_CLR(m_ram[RAM_POS_0_R5_FLAGS3], R5_FLAGS3_USER_TMR_BIT);
		}
		break;
	}
}

bool hp98x6_upi_device::add_to_ctr(unsigned ram_idx, unsigned len, const uint8_t *op)
{
	unsigned carry = 0;

	for (unsigned i = ram_idx; i < ram_idx + len; i++) {
		unsigned sum = unsigned(m_ram[i]) + carry + *op++;
		if (sum < 256) {
			carry = 0;
			m_ram[i] = uint8_t(sum);
		} else {
			carry = 1;
			m_ram[i] = uint8_t(sum - 256);
		}
	}

	return bool(carry);
}

void hp98x6_upi_device::ten_ms_update_key()
{
	// Acquire shift & control (note that bits in R5_FLAGS3 are inverted)
	if (BIT(m_shift->read(), 0)) {
		BIT_CLR(m_ram[RAM_POS_0_R5_FLAGS3], R5_FLAGS3_SHIFT_UP_BIT);
	} else {
		BIT_SET(m_ram[RAM_POS_0_R5_FLAGS3], R5_FLAGS3_SHIFT_UP_BIT);
	}
	if (BIT(m_shift->read(), 1)) {
		BIT_CLR(m_ram[RAM_POS_0_R5_FLAGS3], R5_FLAGS3_CTRL_UP_BIT);
	} else {
		BIT_SET(m_ram[RAM_POS_0_R5_FLAGS3], R5_FLAGS3_CTRL_UP_BIT);
	}

	// Scan keyboard
	ioport_value keys[4];
	acquire_keys(keys);

	for (uint8_t idx = MIN_SCANCODE; idx <= MAX_SCANCODE; idx++) {
		if (is_key_down(keys, idx)) {
			// Check for RESET key combo
			if (!BIT(m_ram[RAM_POS_0_R5_FLAGS3], R5_FLAGS3_SHIFT_UP_BIT) &&
				idx == PAUSE_SCANCODE) {
				if (m_ram[RAM_POS_RST_DEB_CNT] == 0 &&
					!BIT(m_ram[RAM_POS_0_R4_FLAGS2], R4_FLAGS2_RST_MASK_BIT)) {
					// RESET key pressed: start delay to interrupt 68k
					LOG("Reset pressed\n");
					m_fsm_state = fsm_st::ST_RESETTING;
					m_delay_timer->adjust(clocks_to_attotime(RESET_DELAY));
					// Reset cancels FHS timer
					BIT_CLR(m_ram[RAM_POS_0_R2_FLAGS1], R2_FLAGS1_FHS_BIT);
					BIT_CLR(m_ram[RAM_POS_0_R5_FLAGS3], R5_FLAGS3_FHS_BIT);
				}
				m_ram[RAM_POS_RST_DEB_CNT] = R2_FLAGS1_DEB_INIT;
			} else if (idx != m_ram[RAM_POS_0_R7_KEY_DOWN]) {
				if (m_ram[RAM_POS_0_R7_KEY_DOWN] == SCANCODE_NONE) {
					// New key is down
					LOG("Key %02x\n", idx);
					set_new_key(idx);
				} else if (m_ram[RAM_POS_0_R6_ROLLOVER] == SCANCODE_NONE) {
					// Save roll-over key
					LOG("Rollover key %02x\n", idx);
					m_ram[RAM_POS_0_R6_ROLLOVER] = idx;
				}
			} else {
				// Key kept down, reload debounce counter
				m_ram[RAM_POS_0_R2_FLAGS1] = (m_ram[RAM_POS_0_R2_FLAGS1] & ~R2_FLAGS1_DEB_MASK) | R2_FLAGS1_DEB_INIT;
			}
		}
	}

	// Debounce RESET key
	if (m_ram[RAM_POS_RST_DEB_CNT] != 0) {
		m_ram[RAM_POS_RST_DEB_CNT]--;
	}

	// Debounce, auto-repeat and roll-over
	if (m_ram[RAM_POS_0_R7_KEY_DOWN] != SCANCODE_NONE) {
		// Do debouncing
		m_ram[RAM_POS_0_R2_FLAGS1]--;
		if ((m_ram[RAM_POS_0_R2_FLAGS1] & R2_FLAGS1_DEB_MASK) != 0) {
			// Key still down
			m_ram[RAM_POS_AR_TIMER]++;
			if (m_ram[RAM_POS_AR_TIMER] == 0) {
				// Repeat key if A/R is enabled
				m_ram[RAM_POS_AR_TIMER] = m_ram[RAM_POS_AR_RATE];
				if (m_ram[RAM_POS_AR_RATE]) {
					LOG("A/R key %02x\n", m_ram[RAM_POS_0_R7_KEY_DOWN]);
					BIT_SET(m_ram[RAM_POS_0_R4_FLAGS2], R4_FLAGS2_AR_BIT);
				}
			}
			// Key is up & debounced, get roll-over key (if any)
		} else if (m_ram[RAM_POS_0_R6_ROLLOVER] != SCANCODE_NONE) {
			LOG("Shift rollover key %02x\n", m_ram[RAM_POS_0_R6_ROLLOVER]);
			set_new_key(m_ram[RAM_POS_0_R6_ROLLOVER]);
			m_ram[RAM_POS_0_R6_ROLLOVER] = SCANCODE_NONE;
		} else {
			m_ram[RAM_POS_0_R7_KEY_DOWN] = SCANCODE_NONE;
			BIT_CLR(m_ram[RAM_POS_0_R4_FLAGS2], R4_FLAGS2_AR_BIT);
		}
	}
}

void hp98x6_upi_device::ten_ms_update_dial()
{
	ioport_value dial = m_dial->read();
	if (dial != m_last_dial) {
		uint8_t diff = uint8_t(m_last_dial) - uint8_t(dial);
		int diff_int = int(diff);
		// Sign extension
		if (BIT(diff_int, 7)) {
			diff_int -= 256;
		}
		LOG("DIAL %d\n", diff_int);
		int pos = int(m_ram[RAM_POS_1_R4_RPG_COUNT]);
		// Sign extension
		if (BIT(pos, 7)) {
			pos -= 256;
		}
		pos += diff_int;
		// clamp pos between [-128 .. +127]
		if (pos < -128) {
			pos = -128;
		} else if (pos > 127) {
			pos = 127;
		}

		m_ram[RAM_POS_1_R4_RPG_COUNT] = uint8_t(pos);
		m_last_dial = dial;
	}

	if (--m_ram[RAM_POS_RPG_TIMER] == 0) {
		// Dial timer expired
		m_ram[RAM_POS_RPG_TIMER] = m_ram[RAM_POS_RPG_INT_RATE];
		if (m_ram[RAM_POS_1_R4_RPG_COUNT]) {
			// Time to send dial movement to 68k
			BIT_SET(m_ram[RAM_POS_0_R5_FLAGS3], R5_FLAGS3_RPG_BIT);
		}
	}
}

void hp98x6_upi_device::ten_ms_update_timers()
{
	// 10-ms timer is up
	BIT_SET(m_ram[RAM_POS_0_R5_FLAGS3], R5_FLAGS3_PSI_BIT);

	// Advance TOD
	uint8_t one[] = { 1, 0, 0 };
	(void)add_to_ctr(RAM_POS_TOD_1, 3, one);
	// Check for one full day of counting
	if (m_ram[RAM_POS_TOD_3] > TOD_3_ONE_DAY ||
		(m_ram[RAM_POS_TOD_3] == TOD_3_ONE_DAY &&
		 (m_ram[RAM_POS_TOD_2] > TOD_2_ONE_DAY ||
		  (m_ram[RAM_POS_TOD_2] == TOD_2_ONE_DAY &&
		   m_ram[RAM_POS_TOD_1] >= TOD_1_ONE_DAY)))) {
		// Clear TOD and advance DAY counter
		m_ram[RAM_POS_TOD_1] = 0;
		m_ram[RAM_POS_TOD_2] = 0;
		m_ram[RAM_POS_TOD_3] = 0;
		(void)add_to_ctr(RAM_POS_DAY_1, 2, one);
	}

	// Check if "match" timer matches TOD
	if (BIT(m_ram[RAM_POS_0_R2_FLAGS1], R2_FLAGS1_MATCH_BIT) &&
		m_ram[RAM_POS_TOD_1] == m_ram[RAM_POS_MATCH_1] &&
		m_ram[RAM_POS_TOD_2] == m_ram[RAM_POS_MATCH_2] &&
		m_ram[RAM_POS_TOD_3] == m_ram[RAM_POS_MATCH_3]) {
		BIT_SET(m_ram[RAM_POS_0_R5_FLAGS3], R5_FLAGS3_USER_TMR_BIT);
		BIT_SET(m_ram[RAM_POS_1_R3_TIMER_STS], R3_TIMER_STS_MATCH_BIT);
		// Match timer is self-canceling
		BIT_CLR(m_ram[RAM_POS_0_R2_FLAGS1], R2_FLAGS1_MATCH_BIT);
	}

	// Update FHS timer
	if (BIT(m_ram[RAM_POS_0_R2_FLAGS1], R2_FLAGS1_FHS_BIT) &&
		add_to_ctr(RAM_POS_FHS_1, 2, one)) {
		BIT_SET(m_ram[RAM_POS_0_R5_FLAGS3], R5_FLAGS3_FHS_BIT);
		// Is the FHS timer self-canceling?
		try_fhs_output();
	}

	// Update cyclic timer
	if (BIT(m_ram[RAM_POS_0_R2_FLAGS1], R2_FLAGS1_CYCLE_BIT) &&
		add_to_ctr(RAM_POS_CYCLE_1, 3, one)) {
		if (BIT(m_ram[RAM_POS_1_R3_TIMER_STS], R3_TIMER_STS_CYCLE_BIT) &&
			(m_ram[RAM_POS_1_R3_TIMER_STS] & R3_TIMER_STS_MISSED_CYCLES_MASK) != R3_TIMER_STS_MISSED_CYCLES_MASK) {
			// Increment number of missed cycle interrupts
			m_ram[RAM_POS_1_R3_TIMER_STS]++;
		}
		BIT_SET(m_ram[RAM_POS_1_R3_TIMER_STS], R3_TIMER_STS_CYCLE_BIT);
		BIT_SET(m_ram[RAM_POS_0_R5_FLAGS3], R5_FLAGS3_USER_TMR_BIT);
		// Reload timer
		m_ram[RAM_POS_CYCLE_1] = m_ram[RAM_POS_CYCLE_SAVE_1];
		m_ram[RAM_POS_CYCLE_2] = m_ram[RAM_POS_CYCLE_SAVE_2];
		m_ram[RAM_POS_CYCLE_3] = m_ram[RAM_POS_CYCLE_SAVE_3];
	}

	// Update delay timer
	if (BIT(m_ram[RAM_POS_0_R2_FLAGS1], R2_FLAGS1_DELAY_BIT) &&
		add_to_ctr(RAM_POS_DELAY_1, 3, one)) {
		BIT_SET(m_ram[RAM_POS_0_R5_FLAGS3], R5_FLAGS3_USER_TMR_BIT);
		BIT_SET(m_ram[RAM_POS_1_R3_TIMER_STS], R3_TIMER_STS_DELAY_BIT);
		// Is the delay timer self-canceling?
	}
}

void hp98x6_upi_device::ten_ms_update_beep()
{
	if (BIT(m_ram[RAM_POS_0_R2_FLAGS1], R2_FLAGS1_BEEP_BIT) &&
		++m_ram[RAM_POS_BEEP_TIMER] == 0) {
		m_beep->set_state(0);
		BIT_CLR(m_ram[RAM_POS_0_R2_FLAGS1], R2_FLAGS1_BEEP_BIT);
	}
}

void hp98x6_upi_device::set_new_key(uint8_t scancode)
{
	m_ram[RAM_POS_0_R7_KEY_DOWN] = scancode;
	// Trigger output of key
	BIT_SET(m_ram[RAM_POS_0_R4_FLAGS2], R4_FLAGS2_AR_BIT);
	// Set A/R delay counter
	m_ram[RAM_POS_AR_TIMER] = m_ram[RAM_POS_AR_WAIT];
	// Load debounce counter
	m_ram[RAM_POS_0_R2_FLAGS1] = (m_ram[RAM_POS_0_R2_FLAGS1] & ~R2_FLAGS1_DEB_MASK) | R2_FLAGS1_DEB_INIT;
}

void hp98x6_upi_device::acquire_keys(ioport_value input[4])
{
	// Search for longest sequence among pressed keys
	int max_len = 0;
	unsigned n_pressed = 0;
	for (unsigned i = 0; i < 4; i++) {
		input[i] = m_keys[i]->read();
		auto w = input[i];
		while (w) {
			auto mask = BIT_MASK<ioport_value>(31 - count_leading_zeros_32(w));
			auto len = m_keys[i]->field(mask)->seq().length();
			if (len > max_len) {
				max_len = len;
			}
			w &= ~mask;
			n_pressed++;
		}
	}
	// Filter out pressed keys with sequences shorter than the longest one
	if (n_pressed > 1) {
		for (unsigned i = 0; i < 4; i++) {
			auto w = input[i];
			while (w) {
				auto mask = BIT_MASK<ioport_value>(31 - count_leading_zeros_32(w));
				auto len = m_keys[i]->field(mask)->seq().length();
				if (len < max_len) {
					input[i] &= ~mask;
				}
				w &= ~mask;
			}
		}
	}
}

bool hp98x6_upi_device::is_key_down(const ioport_value input[4], uint8_t idx)
{
	unsigned row = idx % 8;
	unsigned col = idx / 8;
	return BIT(input[row / 2], col + ((row & 1) << 4));
}

uint8_t hp98x6_upi_device::encode_shift_ctrl(uint8_t st) const
{
	if (BIT(m_ram[RAM_POS_0_R5_FLAGS3], R5_FLAGS3_CTRL_UP_BIT)) {
		BIT_SET(st, 5);
	}
	if (BIT(m_ram[RAM_POS_0_R5_FLAGS3], R5_FLAGS3_SHIFT_UP_BIT)) {
		BIT_SET(st, 4);
	}
	return st;
}

void hp98x6_upi_device::try_output()
{
	if (!BIT(m_status, STATUS_OBF_BIT)) {
		// Key
		if (BIT(m_ram[RAM_POS_0_R4_FLAGS2], R4_FLAGS2_AR_BIT) &&
			!BIT(m_ram[RAM_POS_0_R4_FLAGS2], R4_FLAGS2_KEY_MASK_BIT)) {
			uint8_t st = encode_shift_ctrl(ST_KEY);
			write_ob_st(m_ram[RAM_POS_0_R7_KEY_DOWN], st);
			BIT_CLR(m_ram[RAM_POS_0_R4_FLAGS2], R4_FLAGS2_AR_BIT);
			// If key output is not possible now, autorepeat bit is not cleared
			// to try again at a later time
		} else {
			// PSI and/or timers
			bool psi = BIT(m_ram[RAM_POS_0_R5_FLAGS3], R5_FLAGS3_PSI_BIT) &&
				!BIT(m_ram[RAM_POS_0_R4_FLAGS2], R4_FLAGS2_PSI_MASK_BIT);
			bool timer = BIT(m_ram[RAM_POS_0_R5_FLAGS3], R5_FLAGS3_USER_TMR_BIT) &&
				!BIT(m_ram[RAM_POS_0_R4_FLAGS2], R4_FLAGS2_TMR_MASK_BIT);

			if (psi || timer) {
				uint8_t st = (psi ? ST_PSI : 0) | (timer ? ST_TIMER : 0);
				write_ob_st(m_ram[RAM_POS_1_R3_TIMER_STS], st);
				if (psi) {
					BIT_CLR(m_ram[RAM_POS_0_R5_FLAGS3], R5_FLAGS3_PSI_BIT);
				}
				if (timer) {
					BIT_CLR(m_ram[RAM_POS_0_R5_FLAGS3], R5_FLAGS3_USER_TMR_BIT);
					m_ram[RAM_POS_1_R3_TIMER_STS] = 0;
				}
			} else if (BIT(m_ram[RAM_POS_0_R5_FLAGS3], R5_FLAGS3_RPG_BIT) &&
					   !BIT(m_ram[RAM_POS_0_R4_FLAGS2], R4_FLAGS2_KEY_MASK_BIT)) {
				// Dial position
				uint8_t st = encode_shift_ctrl(ST_RPG);
				write_ob_st(m_ram[RAM_POS_1_R4_RPG_COUNT], st);
				m_ram[RAM_POS_1_R4_RPG_COUNT] = 0;
				BIT_CLR(m_ram[RAM_POS_0_R5_FLAGS3], R5_FLAGS3_RPG_BIT);
			} else if (BIT(m_ram[RAM_POS_0_R5_FLAGS3], R5_FLAGS3_READ_BIT)) {
				// Read data
				write_ob_st(m_ram[m_ram[RAM_POS_1_R6_R_PTR]], ST_REQUESTED_DATA);
				BIT_CLR(m_ram[RAM_POS_0_R5_FLAGS3], R5_FLAGS3_READ_BIT);
			}
		}
	}
}

void hp98x6_upi_device::try_fhs_output()
{
	// Check if FHS NMI interrupt is to be raised
	if (BIT(m_ram[RAM_POS_0_R5_FLAGS3], R5_FLAGS3_FHS_BIT) &&
		!BIT(m_ram[RAM_POS_0_R4_FLAGS2], R4_FLAGS2_FHS_MASK_BIT)) {
		LOG("NMI from FHS\n");
		BIT_CLR(m_ram[RAM_POS_0_R5_FLAGS3], R5_FLAGS3_FHS_BIT);
		// F0 = 1 means "NMI from FHS timer"
		BIT_SET(m_status, STATUS_F0_BIT);
		m_irq7_write_func(true);
	}
}

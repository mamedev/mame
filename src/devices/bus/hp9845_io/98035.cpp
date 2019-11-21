// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    98035.cpp

    98035 module (Real time clock)

    This module has two main functions: it is a battery-backed real
    time clock and a timer/counter module.
    It is based on a HP Nanoprocessor CPU having 2K of firmware ROM
    and 256 bytes of RAM. This processor parses and executes commands
    that are sent by a HP98xx system and returns the results. All
    I/O happens (mostly) through ASCII strings.
    A 1 MHz crystal provides both the clock to the CPU (it runs at 500 kHz)
    and a periodic 1 kHz interrupt.
    When main power is removed, time is counted by an unnamed "clock chip"
    that is powered by a NiCd battery. It appears that HP had such an
    hard time finding the clock chip that they had to adapt one
    made for counting and displaying time on a 7-segment display. This
    chip was probably manufactured for alarm clocks and/or wristwatches.
    Nanoprocessor parses the digits on the "display" and translates
    them into a standard month-day-hours-minutes-seconds time.
    The biggest limit of the clock chip is that it lacks a counter
    for current year (and so the module cannot recognize leap years).
    For further info on this module see also:
    http://www.hp9825.com/html/real-time_clock.html

    Here's what I know about the clock chip.
    * Packaged in a 24-pin DIP
    * Runs on a 2.4V NiCd battery
    * Keeps time with a standard 32.768 kHz oscillator
    * Drives a 3 1/2 digit LED display with 7-segment digits
    * Multiplexes digits with a 32768 Hz / 64 clock
    * Has 3 buttons to read/set time (READ, SET & CHG)
    * Counts month, day, hour, minute & seconds (no year)
    * Doesn't support leap years
    * On Tony Duell's schematics the chip is marked "AC5954"

    All my attempts to find something like a datasheet of this chip
    failed.

    This driver emulates the clock chip with a FSM that reacts to
    "short" and "long" pressings of keys. Here's a summary of the FSM.

    | State | Key pressed | New state | Display                        |
    |-------+-------------+-----------+--------------------------------|
    | OFF   |             |           | Blank, no multiplexing         |
    |       | Short READ  | HHMM      |                                |
    |       | Short SET   | HH        |                                |
    | HHMM  |             |           | "HH:mm" (Hours 1-12 and mins.) |
    |       |             |           | On real chip it probably       |
    |       |             |           | returns to OFF after a couple  |
    |       |             |           | of seconds.                    |
    |       | Long READ   | SS        |                                |
    | SS    |             |           | "  :SS" (just seconds)         |
    |       | Short SET   | HH        |                                |
    |       | Long SET    | OFF       |                                |
    |       | READ + CHG  |           | seconds++                      |
    | HH    |             |           | "HH: A/P" (hours & AM/PM)      |
    |       | Short SET   | MIN       |                                |
    |       | Long SET    | OFF       |                                |
    |       | READ + CHG  |           | hours++                        |
    | MIN   |             |           | "  :mm" (just minutes)         |
    |       | Short SET   | MON       |                                |
    |       | Long SET    | OFF       |                                |
    |       | READ + CHG  |           | minutes++                      |
    | MON   |             |           | "MM:  " (just month)           |
    |       | Short SET   | DOM       |                                |
    |       | Long SET    | OFF       |                                |
    |       | READ + CHG  |           | month++                        |
    | DOM   |             |           | "  :DD" (just day of month)    |
    |       | Short SET   | OFF       |                                |
    |       | Long SET    | OFF       |                                |
    |       | READ + CHG  |           | day++                          |

    The main reference for this module is this manual:
    HP, 98035A Real Time Clock Installation and Operation Manual

*********************************************************************/

#include "emu.h"
#include "98035.h"
#include "coreutil.h"

// Debugging
#define VERBOSE 0
#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)

#define BIT_MASK(n) (1U << (n))

// Macros to clear/set single bits
#define BIT_CLR(w , n)  ((w) &= ~BIT_MASK(n))
#define BIT_SET(w , n)  ((w) |= BIT_MASK(n))

// Frequency of digit multiplexing in clock chip
#define DIGIT_MUX_FREQ  (XTAL(32'768) / 64)

// Duration of key presses
#define KEY_PRESS_SHORT 1   // 1.95 ms
#define KEY_PRESS_LONG  512 // 1 s

// Mask of keys in m_clock_keys
#define KEY_READ_MASK   1
#define KEY_SET_MASK    2
#define KEY_CHG_MASK    4

// Timers
enum {
	MSEC_TMR_ID,
	CLOCK_TMR_ID
};

// 7-segment display
// Mapping of 7 segments on NP input port is as follows:
// Bit  Segment
// ============
//  7   N/U (1)
//  6   N/U (1)
//  5   Seg "G"
//  4   Seg "F"
//  3   Seg "E"
//  2   Seg "C"
//  1   Seg "B"
//  0   Seg "A"
//
// Segment "D" is not mapped as it's not needed to tell decimal digits apart.
// A segment is ON when its bit is "0".
#define SEVEN_SEG_OFF   0xff    // All segments off
#define SEVEN_SEG_A     0xc0    // "A"
#define SEVEN_SEG_P     0xc4    // "P"
static const uint8_t dec_2_seven_segs[] = {
	0xe0,   // 0
	0xf9,   // 1
	0xd4,   // 2
	0xd8,   // 3
	0xc9,   // 4
	0xca,   // 5
	0xc2,   // 6
	0xf8,   // 7
	0xc0,   // 8
	0xc8    // 9
};

hp98035_io_card_device::hp98035_io_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig , HP98035_IO_CARD , tag , owner , clock),
	  device_hp9845_io_interface(mconfig, *this),
	  device_rtc_interface(mconfig , *this),
	  m_cpu(*this , "np")
{
}

hp98035_io_card_device::~hp98035_io_card_device()
{
}

static INPUT_PORTS_START(hp98035_port)
	PORT_HP9845_IO_SC(9)
INPUT_PORTS_END

ioport_constructor hp98035_io_card_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(hp98035_port);
}

void hp98035_io_card_device::device_start()
{
	save_item(NAME(m_np_ram));
	save_item(NAME(m_ram_addr));
	save_item(NAME(m_ram_data_in));
	save_item(NAME(m_dc));
	save_item(NAME(m_np_irq));
	save_item(NAME(m_flg));
	save_item(NAME(m_inten));
	save_item(NAME(m_intflag));
	save_item(NAME(m_irq));
	save_item(NAME(m_idr_full));
	save_item(NAME(m_idr));
	save_item(NAME(m_odr));
	save_item(NAME(m_clock_1s_div));
	//save_item(NAME(m_clock_state));
	save_item(NAME(m_clock_digits));
	save_item(NAME(m_clock_mux));
	save_item(NAME(m_clock_segh));
	save_item(NAME(m_clock_keys));
	save_item(NAME(m_prev_clock_keys));
	save_item(NAME(m_clock_key_cnt));

	m_msec_timer = timer_alloc(MSEC_TMR_ID);
	m_clock_timer = timer_alloc(CLOCK_TMR_ID);
}

void hp98035_io_card_device::device_reset()
{
	m_idr_full = false;
	m_idr = 0;
	m_odr = 0;
	sts_w(true);
	set_flg(true);

	attotime period(attotime::from_msec(1));
	m_msec_timer->adjust(period , 0 , period);

	period = attotime::from_hz(DIGIT_MUX_FREQ);
	m_clock_timer->adjust(period , 0 , period);

	half_init();
}

void hp98035_io_card_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == MSEC_TMR_ID) {
		// On real hw there's a full 4-bit decimal counter, but only the LSB is used to
		// generate interrupts
		m_np_irq = !m_np_irq;
		update_dc();
	} else if (id == CLOCK_TMR_ID) {
		// Update digit multiplexer
		if (m_clock_state == CLOCK_OFF) {
			m_clock_mux = 0;
		} else {
			m_clock_mux <<= 1;
			if ((m_clock_mux & 7) == 0) {
				m_clock_mux = 1;
			}
		}
		// Act on clock chip "keys"
		if (m_clock_keys == 0 || m_clock_keys != m_prev_clock_keys) {
			m_clock_key_cnt = 0;
			if (m_clock_keys == 0 && m_clock_state == CLOCK_HHMM) {
				// Keys released in HHMM state -> turn display off
				// In real hw there is probably 1 s delay
				m_clock_state = CLOCK_OFF;
				regen_clock_image();
			}
		} else if (m_clock_key_cnt < KEY_PRESS_LONG) {
			m_clock_key_cnt++;
			if (m_clock_key_cnt == KEY_PRESS_SHORT) {
				/// Short key press
				clock_short_press();
			} else if (m_clock_key_cnt == KEY_PRESS_LONG) {
				// Long key press
				clock_long_press();
			}
		}
		m_prev_clock_keys = m_clock_keys;
		// Count seconds
		m_clock_1s_div++;
		if (m_clock_1s_div >= DIGIT_MUX_FREQ.value()) {
			m_clock_1s_div = 0;
			advance_seconds();
			regen_clock_image();
		}
	}
}

READ16_MEMBER(hp98035_io_card_device::reg_r)
{
	uint16_t res;

	switch (offset) {
	case 0:
		// R4: ODR
		res = ~m_odr & 0xff;
		break;

	case 1:
		// R5: Status register
		res = 0x20;
		if (m_inten) {
			BIT_SET(res , 7);
		}
		if (m_intflag) {
			BIT_SET(res , 1);
		}
		if (!BIT(m_dc , 5)) {
			BIT_SET(res , 0);
		}
		break;

	default:
		res = 0;
		break;
	}

	LOG(("read R%u=%04x\n" , offset + 4 , res));
	return res;
}

WRITE16_MEMBER(hp98035_io_card_device::reg_w)
{
	bool new_inten;

	switch (offset) {
	case 0:
		// R4: IDR
		m_idr = (uint8_t)(~data);
		m_idr_full = true;
		break;

	case 1:
		// R5: interrupt enable
		new_inten = BIT(data , 7);
		if (!m_inten && new_inten) {
			m_intflag = true;
		}
		m_inten = new_inten;
		update_irq();
		break;

	case 3:
		// R7: trigger
		set_flg(false);
		break;
	}

	LOG(("write R%u=%04x\n" , offset + 4 , data));
}

WRITE8_MEMBER(hp98035_io_card_device::ram_addr_w)
{
	m_ram_addr = data;
}

READ8_MEMBER(hp98035_io_card_device::ram_data_r)
{
	return m_np_ram[ m_ram_addr ];
}

WRITE8_MEMBER(hp98035_io_card_device::ram_addr_data_w)
{
	m_ram_addr = data;
	m_np_ram[ m_ram_addr ] = m_ram_data_in;
}

WRITE8_MEMBER(hp98035_io_card_device::ram_data_w)
{
	m_ram_data_in = data;
}

WRITE8_MEMBER(hp98035_io_card_device::clock_key_w)
{
	m_clock_keys = data & 7;
}

READ8_MEMBER(hp98035_io_card_device::clock_digit_r)
{
	switch (m_clock_mux) {
	case 1:
		return m_clock_digits[ 0 ];

	case 2:
		return m_clock_digits[ 1 ];

	case 4:
		return m_clock_digits[ 2 ];

	default:
		return SEVEN_SEG_OFF;
	}
}

WRITE8_MEMBER(hp98035_io_card_device::odr_w)
{
	m_odr = data;
	set_flg(true);
}

READ8_MEMBER(hp98035_io_card_device::idr_r)
{
	set_flg(true);
	m_idr_full = false;
	return m_idr;
}

READ8_MEMBER(hp98035_io_card_device::np_status_r)
{
	// Bit 2 = 0: use US date format
	uint8_t res = 0x03;

	if (!m_intflag) {
		BIT_SET(res, 7);
	}
	if (!m_inten) {
		BIT_SET(res, 6);
	}
	if (!m_flg) {
		BIT_SET(res, 5);
	}
	if (m_idr_full) {
		BIT_SET(res, 4);
	}
	if (!m_irq) {
		BIT_SET(res, 3);
	}
	return res;
}

WRITE8_MEMBER(hp98035_io_card_device::clear_np_irq_w)
{
	m_np_irq = false;
	update_dc();
}

READ8_MEMBER(hp98035_io_card_device::clock_mux_r)
{
	// External input lines are always active (bits 7-4)
	uint8_t res = 0xf0 | m_clock_mux;

	if (m_clock_mux == 4 && m_clock_segh) {
		BIT_SET(res, 3);
	}
	return res;
}

WRITE8_MEMBER(hp98035_io_card_device::set_irq_w)
{
	m_irq = true;
	update_irq();
}

READ8_MEMBER(hp98035_io_card_device::clr_inten_r)
{
	m_intflag = false;
	m_inten = false;
	update_irq();

	return 0xff;
}

WRITE8_MEMBER(hp98035_io_card_device::clr_inten_w)
{
	m_intflag = false;
	m_inten = false;
	update_irq();
}

WRITE8_MEMBER(hp98035_io_card_device::dc_w)
{
	if (data != m_dc) {
		//LOG(("DC=%02x\n" , data));
		m_dc = data;
		update_dc();
	}
}

void hp98035_io_card_device::half_init()
{
	m_inten = false;
	m_intflag = false;
	m_irq = false;
	update_irq();
	m_np_irq = false;
	update_dc();

	m_clock_1s_div = 0;
	m_clock_state = CLOCK_OFF;
	m_clock_mux = 0;
	regen_clock_image();
}

void hp98035_io_card_device::set_flg(bool value)
{
	m_flg = value;
	flg_w(m_flg);
}

void hp98035_io_card_device::update_irq()
{
	if (!m_inten) {
		m_irq = false;
	}
	irq_w(m_inten && m_irq);
}

void hp98035_io_card_device::update_dc()
{
	m_cpu->set_input_line(0 , m_np_irq && BIT(m_dc , HP_NANO_IE_DC));
}

void hp98035_io_card_device::set_lhs_digits(unsigned v)
{
	if (v < 10) {
		m_clock_segh = false;
	} else {
		v -= 10;
		m_clock_segh = true;
	}
	m_clock_digits[ 2 ] = dec_2_seven_segs[ v ];
}

void hp98035_io_card_device::set_rhs_digits(unsigned v)
{
	m_clock_digits[ 0 ] = dec_2_seven_segs[ v % 10 ];
	m_clock_digits[ 1 ] = dec_2_seven_segs[ v / 10 ];
}

void hp98035_io_card_device::regen_clock_image()
{
	int tmp;
	bool pm;

	switch (m_clock_state) {
	case CLOCK_OFF:
		m_clock_digits[ 0 ] = SEVEN_SEG_OFF;
		m_clock_digits[ 1 ] = SEVEN_SEG_OFF;
		m_clock_digits[ 2 ] = SEVEN_SEG_OFF;
		m_clock_segh = false;
		break;

	case CLOCK_HHMM:
		tmp = get_clock_register(RTC_HOUR);
		if (tmp == 0) {
			tmp = 12;
		} else if (tmp > 12) {
			tmp -= 12;
		}
		set_lhs_digits(tmp);
		set_rhs_digits(get_clock_register(RTC_MINUTE));
		break;

	case CLOCK_SS:
		m_clock_segh = false;
		m_clock_digits[ 2 ] = SEVEN_SEG_OFF;
		set_rhs_digits(get_clock_register(RTC_SECOND));
		break;

	case CLOCK_HH:
		tmp = get_clock_register(RTC_HOUR);
		pm = tmp >= 12;
		if (tmp == 0) {
			tmp = 12;
		} else if (tmp > 12) {
			tmp -= 12;
		}
		set_lhs_digits(tmp);
		m_clock_digits[ 1 ] = SEVEN_SEG_OFF;
		m_clock_digits[ 0 ] = pm ? SEVEN_SEG_P : SEVEN_SEG_A;
		break;

	case CLOCK_MIN:
		m_clock_segh = false;
		m_clock_digits[ 2 ] = SEVEN_SEG_OFF;
		set_rhs_digits(get_clock_register(RTC_MINUTE));
		break;

	case CLOCK_MON:
		tmp = get_clock_register(RTC_MONTH);
		set_lhs_digits(tmp);
		m_clock_digits[ 0 ] = SEVEN_SEG_OFF;
		m_clock_digits[ 1 ] = SEVEN_SEG_OFF;
		break;

	case CLOCK_DOM:
		m_clock_segh = false;
		m_clock_digits[ 2 ] = SEVEN_SEG_OFF;
		set_rhs_digits(get_clock_register(RTC_DAY));
		break;

	default:
		m_clock_state = CLOCK_OFF;
		break;
	}
	LOG(("St=%d segh=%d %02x:%02x:%02x\n" , m_clock_state ,  m_clock_segh , m_clock_digits[ 2 ] ,
		 m_clock_digits[ 1 ] , m_clock_digits[ 0 ]));
}

void hp98035_io_card_device::clock_short_press()
{
	LOG(("Short press:%u\n" , m_clock_keys));

	bool regen = false;
	int tmp;

	switch (m_clock_state) {
	case CLOCK_OFF:
		if (m_clock_keys == KEY_READ_MASK) {
			m_clock_state = CLOCK_HHMM;
			regen = true;
		} else if (m_clock_keys == KEY_SET_MASK) {
			m_clock_state = CLOCK_HH;
			regen = true;
		}
		break;

	case CLOCK_SS:
		if (m_clock_keys == KEY_SET_MASK) {
			m_clock_state = CLOCK_HH;
			regen = true;
		} else if (m_clock_keys == (KEY_CHG_MASK | KEY_READ_MASK)) {
			tmp = get_clock_register(RTC_SECOND);
			tmp++;
			if (tmp >= 60) {
				tmp = 0;
			}
			set_clock_register(RTC_SECOND , tmp);
			log_current_time();
			//m_clock_1s_div = 0;
			regen = true;
		}
		break;

	case CLOCK_HH:
		if (m_clock_keys == KEY_SET_MASK) {
			m_clock_state = CLOCK_MIN;
			regen = true;
		} else if (m_clock_keys == (KEY_CHG_MASK | KEY_READ_MASK)) {
			tmp = get_clock_register(RTC_HOUR);
			tmp++;
			if (tmp >= 24) {
				tmp = 0;
			}
			set_clock_register(RTC_HOUR , tmp);
			log_current_time();
			regen = true;
		}
		break;

	case CLOCK_MIN:
		if (m_clock_keys == KEY_SET_MASK) {
			m_clock_state = CLOCK_MON;
			regen = true;
		} else if (m_clock_keys == (KEY_CHG_MASK | KEY_READ_MASK)) {
			tmp = get_clock_register(RTC_MINUTE);
			tmp++;
			if (tmp >= 60) {
				tmp = 0;
			}
			set_clock_register(RTC_MINUTE , tmp);
			set_clock_register(RTC_SECOND , 0);
			//m_clock_1s_div = 0;
			log_current_time();
			regen = true;
		}
		break;

	case CLOCK_MON:
		if (m_clock_keys == KEY_SET_MASK) {
			m_clock_state = CLOCK_DOM;
			regen = true;
		} else if (m_clock_keys == (KEY_CHG_MASK | KEY_READ_MASK)) {
			tmp = get_clock_register(RTC_MONTH);
			tmp++;
			if (tmp >= 13) {
				tmp = 1;
			}
			set_clock_register(RTC_MONTH , tmp);
			log_current_time();
			regen = true;
		}
		break;

	case CLOCK_DOM:
		if (m_clock_keys == KEY_SET_MASK) {
			m_clock_state = CLOCK_OFF;
			regen = true;
		} else if (m_clock_keys == (KEY_CHG_MASK | KEY_READ_MASK)) {
			tmp = get_clock_register(RTC_DAY);
			tmp++;
			if (tmp > gregorian_days_in_month(get_clock_register(RTC_MONTH) , 0)) {
				tmp = 1;
			}
			set_clock_register(RTC_DAY , tmp);
			log_current_time();
			regen = true;
		}
		break;

	default:
		break;
	}

	if (regen) {
		regen_clock_image();
	}
}

void hp98035_io_card_device::clock_long_press()
{
	LOG(("Long press:%u\n" , m_clock_keys));

	bool regen = false;

	switch (m_clock_state) {
	case CLOCK_HHMM:
		if (m_clock_keys == KEY_READ_MASK) {
			m_clock_state = CLOCK_SS;
			regen = true;
		}
		break;

	case CLOCK_SS:
	case CLOCK_HH:
	case CLOCK_MIN:
	case CLOCK_MON:
	case CLOCK_DOM:
		if (m_clock_keys == KEY_SET_MASK) {
			m_clock_state = CLOCK_OFF;
			regen = true;
		}
		break;

	default:
		break;
	}

	if (regen) {
		regen_clock_image();
	}
}

void hp98035_io_card_device::log_current_time()
{
	LOG(("Time = %d:%d:%d:%d:%d\n" , get_clock_register(RTC_MONTH) ,
		 get_clock_register(RTC_DAY) , get_clock_register(RTC_HOUR) ,
		 get_clock_register(RTC_MINUTE) , get_clock_register(RTC_SECOND)));
}

void hp98035_io_card_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	// Do nothing, time is kept in "device_rtc_interface" registers
}

ROM_START(hp98035)
	ROM_REGION(0x800 , "np" , 0)
	ROM_LOAD("1818-0469.bin" , 0 , 0x800 , CRC(e16ab3bc) SHA1(34e89a37a2822f27af21969941201317dbff615b))
ROM_END

void hp98035_io_card_device::np_program_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000, 0x7ff).rom().region("np", 0);
}

void hp98035_io_card_device::np_io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0, 0x0).w(FUNC(hp98035_io_card_device::ram_addr_w));
	map(0x1, 0x1).r(FUNC(hp98035_io_card_device::ram_data_r));
	map(0x2, 0x2).w(FUNC(hp98035_io_card_device::ram_addr_data_w));
	map(0x3, 0x3).w(FUNC(hp98035_io_card_device::ram_data_w));
	map(0x5, 0x5).w(FUNC(hp98035_io_card_device::clock_key_w));
	map(0x7, 0x7).r(FUNC(hp98035_io_card_device::clock_digit_r));
	map(0x8, 0x8).w(FUNC(hp98035_io_card_device::odr_w));
	map(0x9, 0x9).r(FUNC(hp98035_io_card_device::idr_r));
	map(0xa, 0xa).r(FUNC(hp98035_io_card_device::np_status_r));
	map(0xb, 0xb).w(FUNC(hp98035_io_card_device::clear_np_irq_w));
	map(0xc, 0xc).r(FUNC(hp98035_io_card_device::clock_mux_r));
	map(0xd, 0xd).w(FUNC(hp98035_io_card_device::set_irq_w));
	map(0xe, 0xe).rw(FUNC(hp98035_io_card_device::clr_inten_r), FUNC(hp98035_io_card_device::clr_inten_w));
}

const tiny_rom_entry *hp98035_io_card_device::device_rom_region() const
{
	return ROM_NAME(hp98035);
}

void hp98035_io_card_device::device_add_mconfig(machine_config &config)
{
	HP_NANOPROCESSOR(config, m_cpu, XTAL(1'000'000));
	m_cpu->set_addrmap(AS_PROGRAM, &hp98035_io_card_device::np_program_map);
	m_cpu->set_addrmap(AS_IO, &hp98035_io_card_device::np_io_map);
	m_cpu->dc_changed().set(FUNC(hp98035_io_card_device::dc_w));
}

// device type definition
DEFINE_DEVICE_TYPE(HP98035_IO_CARD, hp98035_io_card_device, "hp98035", "HP98035 card")

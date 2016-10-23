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

    This driver tries to reproduce in C++ the behaviour of the
    nanoprocessor because I couldn't find any dump of the firmware ROM
    (HP part-no 1818-0469).
    The following commands are recognized in my "imitation":
    A   Halt all timer units
    B   Warm reset
    E   Read & clear interface errors
    F   Activate all timer units
    R   Read real time clock
    S   Set real time clock
    T   Read trigger status
    U   Control timer units
    W   Read lost interrupts

    The main reference for this module is this manual:
    HP, 98035A Real Time Clock Installation and Operation Manual

*********************************************************************/

#include "98035.h"
#include "coreutil.h"

// Debugging
#define VERBOSE 0
#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)

#define BIT_MASK(n) (1U << (n))

// Macros to clear/set single bits
#define BIT_CLR(w , n)  ((w) &= ~BIT_MASK(n))
#define BIT_SET(w , n)  ((w) |= BIT_MASK(n))

// Possible delimiters
#define DELIM_CH_0      '/'
#define DELIM_CH_1      '\n'

// Error masks in m_error
#define ERR_MASK_INT_MISSED     BIT_MASK(0)
#define ERR_MASK_WRONG_INS      BIT_MASK(1)
#define ERR_MASK_WRONG_UNIT     BIT_MASK(2)
#define ERR_MASK_CANT_EXEC      BIT_MASK(3)

// Empty date/time fields
#define EMPTY_FIELD     0xff

// Timers
enum {
	MSEC_TMR_ID
};

hp98035_io_card::hp98035_io_card(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hp9845_io_card_device(mconfig , HP98035_IO_CARD , "HP98035 card" , tag , owner , clock , "hp98035" , __FILE__)
{
}

hp98035_io_card::~hp98035_io_card()
{
}

static INPUT_PORTS_START(hp98035_port)
	MCFG_HP9845_IO_SC
INPUT_PORTS_END

ioport_constructor hp98035_io_card::device_input_ports() const
{
	return INPUT_PORTS_NAME(hp98035_port);
}

void hp98035_io_card::device_start()
{
	m_msec_timer = timer_alloc(MSEC_TMR_ID);
}

void hp98035_io_card::device_reset()
{
	hp9845_io_card_device::device_reset();
	install_readwrite_handler(read16_delegate(FUNC(hp98035_io_card::reg_r) , this) , write16_delegate(FUNC(hp98035_io_card::reg_w) , this));

	m_idr_full = false;
	m_idr = 0;
	m_odr = 0;
	m_ibuffer_ptr = 0;
	m_obuffer_len = 0;
	m_obuffer_ptr = 0;
	sts_w(true);
	set_flg(true);

	// Set real time from the real world
	system_time systime;
	machine().base_datetime(systime);
	m_msec = 0;
	m_sec = systime.local_time.second;
	m_min = systime.local_time.minute;
	m_hrs = systime.local_time.hour;
	m_dom = systime.local_time.mday;
	m_mon = systime.local_time.month + 1;

	attotime period(attotime::from_msec(1));
	m_msec_timer->adjust(period , 0 , period);

	half_init();
}

void hp98035_io_card::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == MSEC_TMR_ID) {
		// Update real time
		m_msec++;
		if (m_msec >= 1000) {
			m_msec = 0;
			m_sec++;
			if (m_sec >= 60) {
				m_sec = 0;
				m_min++;
				if (m_min >= 60) {
					m_min = 0;
					m_hrs++;
					if (m_hrs >= 24) {
						m_hrs = 0;
						m_dom++;
						// Use year 1: this RTC doesn't know the year and so it never advances
						// the date to february 29th
						if (m_dom > gregorian_days_in_month(m_mon , 1)) {
							m_dom = 1;
							m_mon++;
							if (m_mon > 12) {
								m_mon = 1;
							}
						}
					}
				}
			}

			// Every second: check if new real time matches in any active output unit
			for (timer_unit_t& unit : m_units) {
				if (!unit.m_input && unit.m_state == UNIT_ACTIVE &&
					m_sec == unit.m_match_datetime[ 3 ] &&
					(unit.m_match_datetime[ 2 ] == EMPTY_FIELD ||
					 (m_min == unit.m_match_datetime[ 2 ] &&
					  (unit.m_match_datetime[ 1 ] == EMPTY_FIELD ||
					   (m_hrs == unit.m_match_datetime[ 1 ] &&
						(unit.m_match_datetime[ 0 ] == EMPTY_FIELD ||
						 m_dom == unit.m_match_datetime[ 0 ])))))) {
					// Matched
					unit.adv_state();
					LOG(("matched %02u:%03u %p %d %u\n" , m_sec , m_msec , &unit , unit.m_state , unit.m_value));
				}
			}
		}

		// Every ms: advance active units
		uint8_t triggered = 0;
		for (timer_unit_t& unit : m_units) {
			if (unit.m_state != UNIT_IDLE) {
				if (unit.m_input && unit.m_state == UNIT_ACTIVE) {
					// Input unit: it just counts msecs
					// In the real 98035 there is a limit value of 1E+10: not simulated here
					unit.m_value++;
				} else if (!unit.m_input && unit.m_state == UNIT_WAIT_FOR_TO &&
						   (unit.m_value == 0 || --unit.m_value == 0)) {
					// Output unit that is not waiting for real time match and that just reached timeout
					// Triggered!
					LOG(("triggered %02u:%03u %p\n" , m_sec , m_msec , &unit));
					BIT_SET(triggered, unit.m_port - 1);
					unit.adv_state();
					if (unit.m_value == 0) {
						LOG(("deact %p\n" , &unit));
						unit.deactivate();
					}
				}
			}
		}
		// Generate IRQ
		if (triggered) {
			// Store which output(s) triggered
			m_triggered = triggered;
			if (m_inten) {
				m_irq = true;
				update_irq();
				LOG(("IRQ %02x\n" , m_triggered));
			} else if (m_intflag) {
				set_error(ERR_MASK_INT_MISSED);
				// Record lost interrupts
				m_lost_irq = triggered;
				LOG(("lost %02x\n" , m_triggered));
			}
		}
	}
}

uint16_t hp98035_io_card::reg_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	uint16_t res;

	switch (offset) {
	case 0:
		// R4: ODR
		res = m_odr;
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
		if (m_error) {
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

void hp98035_io_card::reg_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	bool new_inten;

	switch (offset) {
	case 0:
		// R4: IDR
		m_idr = (uint8_t)data;
		m_idr_full = true;
		update_ibuffer();
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
		update_ibuffer();
		update_obuffer();
		break;
	}

	LOG(("write R%u=%04x\n" , offset + 4 , data));
}

void hp98035_io_card::half_init(void)
{
	m_inten = false;
	m_intflag = false;
	m_irq = false;
	m_error = 0;
	m_triggered = 0;
	m_lost_irq = 0;
	update_irq();

	for (timer_unit_t& unit : m_units) {
		unit.init();
	}
	// Unit 1 defaults to output 1
	// Unit 2 defaults to input 1
	m_units[ 0 ].m_input = false;
	m_units[ 0 ].m_port = 1;
	m_units[ 1 ].m_input = true;
	m_units[ 1 ].m_port = 1;

	set_obuffer('\n');
}

void hp98035_io_card::set_flg(bool value)
{
	m_flg = value;
	flg_w(m_flg);
}

void hp98035_io_card::update_irq(void)
{
	if (!m_inten) {
		m_irq = false;
	}
	irq_w(m_inten && m_irq);
}

void hp98035_io_card::update_ibuffer(void)
{
	if (m_idr_full && !m_flg) {
		// New input byte, put in ibuffer if it's a valid char (A-Z 0-9 = / \n)
		if (m_idr == DELIM_CH_0 || m_idr == DELIM_CH_1) {
			process_ibuffer();
		} else if (((m_idr >= 'A' && m_idr <= 'Z') || (m_idr >= '0' && m_idr <= '9') || m_idr == '=') &&
				   m_ibuffer_ptr < HP98035_IBUFFER_LEN) {
			m_ibuffer[ m_ibuffer_ptr++ ] = m_idr;
		}
		m_idr_full = false;
		set_flg(true);
	}
}

void hp98035_io_card::process_ibuffer(void)
{
	m_ibuffer[ m_ibuffer_ptr ] = '\0';
	const uint8_t *p = &m_ibuffer[ 0 ];

	clear_obuffer();

	bool get_out = false;

	while (*p != '\0' && !get_out) {
		std::ostringstream out;
		uint8_t datetime[ 5 ];
		unsigned unit_no;

		switch (*p++) {
		case 'A':
			// Halt all timer units
			for (timer_unit_t& unit : m_units) {
				unit.deactivate();
			}
			m_inten = false;
			m_intflag = false;
			update_irq();
			break;

		case 'B':
			// Warm reset
			half_init();
			get_out = true;
			break;

		case 'E':
			// Read and clear errors
			set_obuffer(m_error);
			m_error = 0;
			break;

		case 'F':
			// Activate all timer units
			for (timer_unit_t& unit : m_units) {
				if (unit.m_port) {
					unit.adv_state(true);
				}
			}
			break;

		case 'R':
			// Read time
			// Assume US format of dates
			util::stream_format(out , "%02u:%02u:%02u:%02u:%02u" , m_mon , m_dom , m_hrs , m_min , m_sec);
			set_obuffer(out.str().c_str());
			break;

		case 'S':
			// Set time
			if (parse_datetime(p , datetime)) {
				// Cannot set time when there's one or more active output units
				if (std::any_of(std::begin(m_units) , std::end(m_units) , [](const timer_unit_t& u) { return u.m_state != UNIT_IDLE && !u.m_input; })) {
					set_error(ERR_MASK_CANT_EXEC);
				} else {
					m_msec = 0;
					m_sec = datetime[ 4 ];
					if (datetime[ 3 ] != EMPTY_FIELD) {
						m_min = datetime[ 3 ];
						if (datetime[ 2 ] != EMPTY_FIELD) {
							m_hrs = datetime[ 2 ];
							if (datetime[ 1 ] != EMPTY_FIELD) {
								m_dom = datetime[ 1 ];
								if (datetime[ 0 ] != EMPTY_FIELD) {
									m_mon = datetime[ 0 ];
								}
							}
						}
					}
				}
			} else {
				set_error(ERR_MASK_WRONG_INS);
				get_out = true;
			}
			break;

		case 'T':
			// Read triggered outputs
			set_obuffer(m_triggered);
			m_triggered = 0;
			break;

		case 'U':
			// Control timer units
			if (parse_unit_no(p , unit_no)) {
				get_out = parse_unit_command(p , unit_no);
			} else {
				set_error(ERR_MASK_WRONG_INS);
				get_out = true;
			}
			break;

		case 'W':
			// Read unserviced interrupts
			set_obuffer(m_lost_irq);
			m_lost_irq = 0;
			break;

		default:
			set_error(ERR_MASK_WRONG_INS);
			get_out = true;
			break;
		}
	}

	m_ibuffer_ptr = 0;
}

bool hp98035_io_card::assign_unit(timer_unit_t& unit , const uint8_t*& p , bool input)
{
	unsigned port_no;

	if (parse_unit_no(p , port_no)) {
		if (unit.m_state == UNIT_IDLE) {
			unit.init();

			if (std::any_of(std::begin(m_units) , std::end(m_units) , [input , port_no](const timer_unit_t& u) { return u.m_input == input && u.m_port == port_no; })) {
				// I/O port already assigned
				set_error(ERR_MASK_WRONG_UNIT);
				return false;
			}

			unit.m_input = input;
			unit.m_port = port_no;
		} else {
			set_error(ERR_MASK_CANT_EXEC);
		}
		return false;
	} else {
		set_error(ERR_MASK_WRONG_INS);
		return true;
	}
}

bool hp98035_io_card::parse_unit_command(const uint8_t*& p, unsigned unit_no)
{
	unit_no--;
	timer_unit_t& unit = m_units[ unit_no ];
	bool get_out = false;
	unsigned msec;
	uint8_t to_match[ 5 ];
	std::ostringstream out;

	LOG(("U %c %u %p\n" , *p , unit_no , &unit));

	switch (*p++) {
	case '=':
		// Assign unit
		if (*p == 'I') {
			p++;
			get_out = assign_unit(unit , p , true);
		} else if (*p == 'O') {
			p++;
			get_out = assign_unit(unit , p , false);
		}
		break;

	case 'C':
		// Clear input unit
		if (unit.m_input && unit.m_port) {
			unit.m_value = 0;
		} else {
			set_error(ERR_MASK_WRONG_UNIT);
		}
		break;

	case 'D':
		// Set delay on output unit
		if (parse_msec(p , msec)) {
			if (!unit.m_input && unit.m_port) {
				if (unit.m_state == UNIT_IDLE) {
					unit.m_delay = msec;
				} else {
					set_error(ERR_MASK_CANT_EXEC);
				}
			} else {
				set_error(ERR_MASK_WRONG_UNIT);
			}
		} else {
			set_error(ERR_MASK_WRONG_INS);
		}
		get_out = true;
		break;

	case 'G':
		// Activate unit
		if (unit.m_port && unit.m_state == UNIT_IDLE) {
			unit.adv_state(true);

			LOG(("act %p %d %d %u %02u:%02u:%02u:%02u %u %u %u\n" , &unit , unit.m_state , unit.m_input , unit.m_port , unit.m_match_datetime[ 0 ] , unit.m_match_datetime[ 1 ] , unit.m_match_datetime[ 2 ] , unit.m_match_datetime[ 3 ] , unit.m_delay , unit.m_period , unit.m_value));
		} else {
			set_error(ERR_MASK_WRONG_UNIT);
		}
		break;

	case 'H':
		// Halt unit
		if (unit.m_port) {
			unit.deactivate();
		} else {
			set_error(ERR_MASK_WRONG_UNIT);
		}
		break;

	case 'M':
		// Set date/time to match on output unit
		if (!unit.m_input && unit.m_port) {
			if (unit.m_state == UNIT_IDLE) {
				if (*p == '\0') {
					unit.m_match_datetime[ 0 ] = EMPTY_FIELD;
					unit.m_match_datetime[ 1 ] = EMPTY_FIELD;
					unit.m_match_datetime[ 2 ] = EMPTY_FIELD;
					unit.m_match_datetime[ 3 ] = EMPTY_FIELD;
				} else if (parse_datetime(p , to_match) && *p == '\0') {
					unit.m_match_datetime[ 0 ] = to_match[ 1 ];
					unit.m_match_datetime[ 1 ] = to_match[ 2 ];
					unit.m_match_datetime[ 2 ] = to_match[ 3 ];
					unit.m_match_datetime[ 3 ] = to_match[ 4 ];
				} else {
					set_error(ERR_MASK_WRONG_INS);
				}
			} else {
				set_error(ERR_MASK_CANT_EXEC);
			}
		} else {
			set_error(ERR_MASK_WRONG_UNIT);
		}
		get_out = true;
		break;

	case 'P':
		// Set period on output unit
		if (parse_msec(p , msec)) {
			if (!unit.m_input && unit.m_port) {
				if (unit.m_state == UNIT_IDLE) {
					unit.m_period = msec;
				} else {
					set_error(ERR_MASK_CANT_EXEC);
				}
			} else {
				set_error(ERR_MASK_WRONG_UNIT);
			}
		} else {
			set_error(ERR_MASK_WRONG_INS);
		}
		get_out = true;
		break;

	case 'V':
		// Get value of input unit
		if (unit.m_input && unit.m_port) {
			util::stream_format(out , "%010u" , unit.m_value);
			set_obuffer(out.str().c_str());
		} else {
			set_error(ERR_MASK_WRONG_UNIT);
		}
		break;
	}

	return get_out;
}

void hp98035_io_card::clear_obuffer(void)
{
	m_obuffer_len = 0;
	m_obuffer_ptr = 0;
}

void hp98035_io_card::set_obuffer(uint8_t b)
{
	m_obuffer[ 0 ] = b;
	m_obuffer_len = 1;
	m_obuffer_ptr = 0;
}

void hp98035_io_card::set_obuffer(const char* s)
{
	unsigned s_len = std::min((unsigned)strlen(s) , (unsigned)(HP98035_OBUFFER_LEN - 1));

	memcpy(&m_obuffer[ 0 ] , s , s_len);
	m_obuffer[ s_len++ ] = '\n';
	m_obuffer_len = s_len;
	m_obuffer_ptr = 0;
}

void hp98035_io_card::update_obuffer(void)
{
	if (!m_idr_full && !m_flg && m_obuffer_ptr < m_obuffer_len) {
		m_odr = m_obuffer[ m_obuffer_ptr++ ];
		set_flg(true);
	}
}

void hp98035_io_card::set_error(uint8_t mask)
{
	m_error |= mask;
}

bool hp98035_io_card::parse_datetime(const uint8_t*& p, uint8_t *out) const
{
	unsigned n_fields = 0;

	// Fill all fields with EMPTY_FIELD
	for (unsigned i = 0; i < 5; i++) {
		out[ i ] = EMPTY_FIELD;
	}

	while (n_fields < 5 && *p >= '0' && *p <= '9') {
		unsigned tmp = *p - '0';
		p++;
		if (*p < '0' || *p > '9') {
			return false;
		}
		// Shift all fields one position to the left
		memmove(&out[ 0 ] , &out[ 1 ] , 4 * sizeof(out[ 0 ]));
		// Put the new field in the last position
		out[ 4 ] = (uint8_t)(tmp * 10 + *p - '0');
		n_fields++;
		p++;
	}

	if (n_fields == 0) {
		return false;
	}

	// Seconds
	if (out[ 4 ] >= 60) {
		return false;
	}
	if (n_fields == 1) {
		return true;
	}
	// Minutes
	if (out[ 3 ] >= 60) {
		return false;
	}
	if (n_fields == 2) {
		return true;
	}
	// Hours
	if (out[ 2 ] >= 24) {
		return false;
	}
	if (n_fields == 3) {
		return true;
	}
	// Month
	uint8_t month;
	if (n_fields == 4) {
		month = m_mon;
	} else {
		if (out[ 0 ] < 1 || out[ 0 ] > 12) {
			return false;
		}
		month = out[ 0 ];
	}
	// Day of month
	// Use year 0 here to allow for February 29th
	if (out[ 1 ] < 1 || out[ 1 ] > gregorian_days_in_month(month , 0)) {
		return false;
	}
	return true;
}

bool hp98035_io_card::parse_unit_no(const uint8_t*& p, unsigned& unit) const
{
	if (*p < '1' || *p > '0' + HP98035_UNIT_COUNT) {
		return false;
	}
	unit = *p - '0';

	do {
		p++;
	} while (*p >= '0' && *p <= '9');

	return true;
}

bool hp98035_io_card::parse_msec(const uint8_t*& p, unsigned& msec) const
{
	msec = 0;

	while (*p >= '0' && *p <= '9') {
		msec = msec * 10 + *p - '0';
		p++;
	}

	return *p == '\0';
}

void hp98035_io_card::timer_unit_t::init(void)
{
	m_state = UNIT_IDLE;
	m_port = 0;
	m_match_datetime[ 0 ] = EMPTY_FIELD;
	m_match_datetime[ 1 ] = EMPTY_FIELD;
	m_match_datetime[ 2 ] = EMPTY_FIELD;
	m_match_datetime[ 3 ] = EMPTY_FIELD;
	m_delay = 0;
	m_period = 0;
	m_value = 0;
}

void hp98035_io_card::timer_unit_t::deactivate(void)
{
	m_state = UNIT_IDLE;
}

void hp98035_io_card::timer_unit_t::adv_state(bool reset)
{
	if (reset) {
		m_state = UNIT_IDLE;
	}

	if (m_input) {
		m_state = UNIT_ACTIVE;
		m_value = 0;
	} else {
		if (m_state == UNIT_IDLE) {
			m_state = UNIT_ACTIVE;
			if (m_match_datetime[ 3 ] != EMPTY_FIELD) {
				// Seconds field is not empty -> there's a date/time to be matched first
				return;
			}
		}
		if (m_state == UNIT_ACTIVE) {
			m_value = m_delay;
		} else {
			m_value = m_period;
		}
		m_state = UNIT_WAIT_FOR_TO;
	}
}

// device type definition
const device_type HP98035_IO_CARD = &device_creator<hp98035_io_card>;

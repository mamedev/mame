// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Motorola MCCS1850 / MC68HC68T1 Serial Real-Time Clock emulation

*********************************************************************/

/*

    TODO:

    - auto restart
    - test mode
    - MC68HC68T1 alarm and periodic interrupts

*/

#include "emu.h"
#include "mccs1850.h"

#include "multibyte.h"

//#define VERBOSE 0
#include "logmacro.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define RAM_SIZE 0x80


// serial state
enum
{
	STATE_ADDRESS,
	STATE_DATA
};


// registers
enum
{
	REGISTER_COUNTER_LATCH = 0x20,
	REGISTER_ALARM_LATCH = 0x24,
	REGISTER_STATUS = 0x30,
	REGISTER_CONTROL = 0x31,
	REGISTER_TEST_1 = 0x3e,
	REGISTER_TEST_2 = 0x3f,
	REGISTER_TEST_KICK_START_COUNTER = 0x40,
	REGISTER_TEST_PRESCALE_COUNTER = 0x43,
	REGISTER_TEST_COUNTER_INCREMENT = 0x4f
};


// clock status/interrupt register
#define STATUS_TM       0x20    // test mode
#define STATUS_FTU      0x10    // first time up
#define STATUS_IT       0x08    // interrupt true
#define STATUS_LB       0x04    // low battery
#define STATUS_AI       0x02    // alarm
#define STATUS_RPD      0x01    // request to power down


// clock control register
#define CONTROL_STR_STP 0x80    // start/stop
#define CONTROL_PD      0x40    // power down
#define CONTROL_AR      0x20    // auto restart
#define CONTROL_AE      0x10    // alarm enable
#define CONTROL_AC      0x08    // alarm clear
#define CONTROL_FTUC    0x04    // first time up clear
#define CONTROL_LBE     0x02    // low battery enable
#define CONTROL_RPCD    0x01    // request to power down clear


// test register 1
#define TEST1_DIV1      0x80    // divide by 1
#define TEST1_VOVR      0x40    // Vdd override
#define TEST1_VDDUP     0x20    // Vdd up
#define TEST1_VDDON     0x10    // Vdd on
#define TEST1_VRT       0x08    // valid RAM and time
#define TEST1_LOW_BAT   0x08    // low battery
#define TEST1_PCC       0x04    // programmable capacitor C (10.0 pF)
#define TEST1_PCB       0x02    // programmable capacitor B (5.0 pF)
#define TEST1_PCA       0x01    // programmable capacitor A (2.5 pF)


// test register 2
#define TEST2_OSCBY     0x80    // oscillator bypass
#define TEST2_COMPOVR   0x40    // comparator override
#define TEST2_POR       0x20    // power on reset
#define TEST2_SELTCK    0x10    // select test clock
#define TEST2_FRZ       0x08    // freeze mode
#define TEST2_DV_MASK   0x07    // divider bits select


namespace {

// MC68HC68T1 registers
enum : offs_t
{
	T1_RAM_END = 0x1f,
	T1_SECONDS = 0x20,
	T1_MINUTES = 0x21,
	T1_HOURS = 0x22,
	T1_DAY_OF_WEEK = 0x23,
	T1_DATE = 0x24,
	T1_MONTH = 0x25,
	T1_YEAR = 0x26,
	T1_STATUS = 0x30,
	T1_CLOCK_CONTROL = 0x31,
	T1_INTERRUPT_CONTROL = 0x32
};

// MC68HC68T1 register bits
enum : uint8_t
{
	T1_HOURS_12 = 0x80,         // hours: 12-hour mode
	T1_HOURS_PM = 0x20,         // hours: PM in 12-hour mode
	T1_CLOCK_START = 0x80,      // clock control: start
	T1_POWER_DOWN = 0x40        // interrupt control: power down
};

} // anonymous namespace



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(MCCS1850, mccs1850_device, "mccs1850", "MCCS1850 RTC")
DEFINE_DEVICE_TYPE(MC68HC68T1, mc68hc68t1_device, "mc68hc68t1", "MC68HC68T1 RTC")



//**************************************************************************
//  DEVICE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  check_interrupt -
//-------------------------------------------------

inline void mccs1850_device::check_interrupt()
{
	uint8_t status = m_ram[REGISTER_STATUS];
	uint8_t control = m_ram[REGISTER_CONTROL];

	bool interrupt = (((status & STATUS_AI) && (control & CONTROL_AE))      // alarm interrupt
					|| ((status & STATUS_LB) && (control & CONTROL_LBE))    // low battery
					|| (status & STATUS_FTU)                                // first time up
					|| (status & STATUS_RPD));                              // request to power down

	if (interrupt)
	{
		m_ram[REGISTER_STATUS] |= STATUS_IT;
	}
	else
	{
		m_ram[REGISTER_STATUS] &= ~STATUS_IT;
	}

	int_cb(interrupt);
}


//-------------------------------------------------
//  set_pse_line -
//-------------------------------------------------

inline void motorola_serial_rtc_device_base::set_pse_line(bool state)
{
	m_pse = state;

	pse_cb(m_pse);
}


//-------------------------------------------------
//  next_address - address after a burst byte
//-------------------------------------------------

offs_t motorola_serial_rtc_device_base::next_address(offs_t offset) const
{
	return (offset + 1) & 0x7f;
}


//-------------------------------------------------
//  read_register -
//-------------------------------------------------

uint8_t mccs1850_device::read_register(offs_t offset)
{
	switch (offset)
	{
	case REGISTER_COUNTER_LATCH:
	case REGISTER_COUNTER_LATCH+3: // Required by the NeXT power on test
		// load counter value into latch
		put_u32be(&m_ram[REGISTER_COUNTER_LATCH], m_counter);
		break;

	case REGISTER_TEST_1:
	case REGISTER_TEST_2:
	case REGISTER_TEST_KICK_START_COUNTER:
	case REGISTER_TEST_PRESCALE_COUNTER:
	case REGISTER_TEST_COUNTER_INCREMENT:
		logerror("MCCS1850 Unsupported read from test register %02x!\n", offset);
		break;
	}

	return m_ram[offset];
}


//-------------------------------------------------
//  write_register -
//-------------------------------------------------

void mccs1850_device::write_register(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case REGISTER_STATUS:
		// read only
		break;

	case REGISTER_CONTROL:
		LOG("MCCS1850 Counter %s\n", (data & CONTROL_STR_STP) ? "Start" : "Stop");
		m_clock_timer->enable(data & CONTROL_STR_STP);

		if (data & CONTROL_PD)
		{
			LOG("MCCS1850 Power Down\n");
			set_pse_line(false);
		}

		if (data & CONTROL_AR)
		{
			LOG("MCCS1850 Auto Restart\n");
		}

		if (data & CONTROL_AC)
		{
			LOG("MCCS1850 Alarm Clear\n");
			m_ram[REGISTER_STATUS] &= ~STATUS_AI;
		}

		if (data & CONTROL_FTUC)
		{
			LOG("MCCS1850 First Time Up Clear\n");
			m_ram[REGISTER_STATUS] &= ~STATUS_FTU;
		}

		if (data & CONTROL_RPCD)
		{
			LOG("MCCS1850 Request to Power Down Clear\n");
			m_ram[REGISTER_STATUS] &= ~STATUS_RPD;
		}

		m_ram[REGISTER_CONTROL] = data & 0xb2;

		check_interrupt();
		break;

	case REGISTER_TEST_1:
	case REGISTER_TEST_2:
	case REGISTER_TEST_KICK_START_COUNTER:
	case REGISTER_TEST_PRESCALE_COUNTER:
	case REGISTER_TEST_COUNTER_INCREMENT:
		logerror("MCCS1850 Unsupported write to test register %02x!\n", offset);
		break;

	default:
		m_ram[offset] = data;
	}
}


//-------------------------------------------------
//  advance_seconds -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(mccs1850_device::advance_seconds)
{
	uint32_t alarm = get_u32be(&m_ram[REGISTER_ALARM_LATCH]);

	m_counter++;

	if (m_counter == alarm)
	{
		if (m_pse)
		{
			// trigger alarm
			m_ram[REGISTER_STATUS] |= STATUS_AI;

			check_interrupt();
		}
		else
		{
			// wake up
			set_pse_line(true);
		}
	}
}


//-------------------------------------------------
//  decode_calendar - copy one MC68HC68T1
//  calendar register into the RTC
//-------------------------------------------------

inline void mc68hc68t1_device::decode_calendar(offs_t offset)
{
	static constexpr int REGISTERS[] = { RTC_SECOND, RTC_MINUTE, RTC_HOUR, RTC_DAY_OF_WEEK, RTC_DAY, RTC_MONTH, RTC_YEAR };

	const uint8_t data = m_ram[offset];
	int value = bcd_to_integer(data);
	if ((offset == T1_HOURS) && (data & T1_HOURS_12))
		value = (bcd_to_integer(data & 0x1f) % 12) + ((data & T1_HOURS_PM) ? 12 : 0);
	set_clock_register(REGISTERS[offset - T1_SECONDS], value);
}


//-------------------------------------------------
//  registers_updated - set the RTC from the
//  MC68HC68T1 calendar registers
//-------------------------------------------------

void mc68hc68t1_device::registers_updated()
{
	for (offs_t offset = T1_SECONDS; offset <= T1_YEAR; offset++)
		decode_calendar(offset);
}


//-------------------------------------------------
//  read_register -
//-------------------------------------------------

uint8_t mc68hc68t1_device::read_register(offs_t offset)
{
	return m_ram[offset];
}


//-------------------------------------------------
//  write_register -
//-------------------------------------------------

void mc68hc68t1_device::write_register(offs_t offset, uint8_t data)
{
	if (offset == T1_STATUS)
		return;

	m_ram[offset] = data;
	if ((offset >= T1_SECONDS) && (offset <= T1_YEAR))
		decode_calendar(offset);
	else if (offset == T1_CLOCK_CONTROL)
		m_clock_timer->enable(data & T1_CLOCK_START);
	else if ((offset == T1_INTERRUPT_CONTROL) && (data & T1_POWER_DOWN))
		set_pse_line(false);
}


//-------------------------------------------------
//  next_address - bursts wrap within the RAM
//  and within the clock registers
//-------------------------------------------------

offs_t mc68hc68t1_device::next_address(offs_t offset) const
{
	switch (offset)
	{
	case T1_RAM_END:
		return 0x00;

	case T1_INTERRUPT_CONTROL:
		return T1_SECONDS;

	default:
		return motorola_serial_rtc_device_base::next_address(offset);
	}
}


//-------------------------------------------------
//  advance_seconds -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(mc68hc68t1_device::advance_seconds)
{
	device_rtc_interface::advance_seconds();
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  motorola_serial_rtc_device_base - constructor
//-------------------------------------------------

motorola_serial_rtc_device_base::motorola_serial_rtc_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_rtc_interface(mconfig, *this),
	device_nvram_interface(mconfig, *this),
	int_cb(*this),
	pse_cb(*this),
	m_ram{},
	m_pse(1),
	m_clock_timer(nullptr),
	m_ce(0),
	m_sck(0),
	m_sdo(1),
	m_sdi(0),
	m_state(STATE_ADDRESS),
	m_address(0),
	m_bits(0),
	m_shift(0)
{
}


//-------------------------------------------------
//  mccs1850_device - constructor
//-------------------------------------------------

mccs1850_device::mccs1850_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	motorola_serial_rtc_device_base(mconfig, MCCS1850, tag, owner, clock),
	nuc_cb(*this),
	m_counter(0)
{
}


//-------------------------------------------------
//  mc68hc68t1_device - constructor
//-------------------------------------------------

mc68hc68t1_device::mc68hc68t1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	motorola_serial_rtc_device_base(mconfig, MC68HC68T1, tag, owner, clock)
{
}


//-------------------------------------------------
//  rtc_clock_changed -
//-------------------------------------------------

void mccs1850_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	// FIXME: implement this properly
	system_time systime;
	machine().base_datetime(systime);
	m_counter = systime.time;
}

void mc68hc68t1_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	m_ram[T1_SECONDS] = convert_to_bcd(second);
	m_ram[T1_MINUTES] = convert_to_bcd(minute);
	if (m_ram[T1_HOURS] & T1_HOURS_12)
		m_ram[T1_HOURS] = T1_HOURS_12 | ((hour >= 12) ? T1_HOURS_PM : 0) | convert_to_bcd((hour % 12) ? (hour % 12) : 12);
	else
		m_ram[T1_HOURS] = convert_to_bcd(hour);
	m_ram[T1_DAY_OF_WEEK] = convert_to_bcd(day_of_week);
	m_ram[T1_DATE] = convert_to_bcd(day);
	m_ram[T1_MONTH] = convert_to_bcd(month);
	m_ram[T1_YEAR] = convert_to_bcd(year % 100);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void motorola_serial_rtc_device_base::device_start()
{
	// allocate timers
	m_clock_timer = timer_alloc(FUNC(motorola_serial_rtc_device_base::advance_seconds), this);
	m_clock_timer->adjust(attotime::from_hz(clock() / 32768), 0, attotime::from_hz(clock() / 32768));

	// state saving
	save_item(NAME(m_ram));
	save_item(NAME(m_pse));
	save_item(NAME(m_ce));
	save_item(NAME(m_sck));
	save_item(NAME(m_sdo));
	save_item(NAME(m_sdi));
	save_item(NAME(m_state));
	save_item(NAME(m_address));
	save_item(NAME(m_bits));
	save_item(NAME(m_shift));
}

void mccs1850_device::device_start()
{
	motorola_serial_rtc_device_base::device_start();

	save_item(NAME(m_counter));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mccs1850_device::device_reset()
{
	if(!m_counter)
		m_ram[REGISTER_STATUS] = 0x80 | STATUS_FTU;
	else
		m_ram[REGISTER_STATUS] = 0x80;
	m_ram[REGISTER_CONTROL] = 0x00;
}

void mc68hc68t1_device::device_reset()
{
	m_ram[T1_STATUS] = 0;
	m_ram[T1_INTERRUPT_CONTROL] = 0;
}


//-------------------------------------------------
//  device_post_load - device-specific post-load
//-------------------------------------------------

void mc68hc68t1_device::device_post_load()
{
	registers_updated();
}


//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void motorola_serial_rtc_device_base::nvram_default()
{
	memset(m_ram, 0xff, RAM_SIZE);

	if (machine().root_device().memregion(tag()) != nullptr)
	{
		uint8_t *nvram = machine().root_device().memregion(tag())->base();

		// initialize NVRAM
		memcpy(m_ram, nvram, 0x20);
	}
}

void mc68hc68t1_device::nvram_default()
{
	motorola_serial_rtc_device_base::nvram_default();

	m_ram[T1_HOURS] = 0; // 24-hour mode
	registers_updated();
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

bool motorola_serial_rtc_device_base::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = read(file, m_ram, RAM_SIZE);
	return !err && (actual == RAM_SIZE);
}

bool mc68hc68t1_device::nvram_read(util::read_stream &file)
{
	if (!motorola_serial_rtc_device_base::nvram_read(file))
		return false;

	registers_updated();
	return true;
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

bool motorola_serial_rtc_device_base::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = write(file, m_ram, RAM_SIZE);
	return !err;
}


//-------------------------------------------------
//  ce_w - chip enable write
//-------------------------------------------------

void motorola_serial_rtc_device_base::ce_w(int state)
{
	m_ce = state;

	if (!m_ce)
	{
		m_state = STATE_ADDRESS;
		m_bits = 0;
	}
}


//-------------------------------------------------
//  sck_w - serial clock write
//-------------------------------------------------

void motorola_serial_rtc_device_base::sck_w(int state)
{
	if (!m_ce) return;

	switch (m_state)
	{
	case STATE_ADDRESS:
		if (m_sck && !state)
		{
			m_address <<= 1;
			m_address |= m_sdi;
			m_bits++;

			if (m_bits == 8)
			{
				LOG("MCCS1850 %s Address %02x\n", BIT(m_address, 7) ? "Write" : "Read", m_address & 0x7f);

				m_bits = 0;
				m_state = STATE_DATA;

				if (!BIT(m_address, 7))
				{
					m_shift = read_register(m_address & 0x7f);

					LOG("MCCS1850 Data Out %02x\n", m_shift);
				}
			}
		}
		break;

	case STATE_DATA:
		if (BIT(m_address, 7) && m_sck && !state)
		{
			// shift data in
			m_shift <<= 1;
			m_shift |= m_sdi;
			m_bits++;

			if (m_bits == 8)
			{
				LOG("MCCS1850 Data In %02x\n", m_shift);

				write_register(m_address & 0x7f, m_shift);

				m_bits = 0;

				// increment address counter
				m_address = next_address(m_address & 0x7f) | 0x80;
			}
		}
		else if (!BIT(m_address, 7) && !m_sck && state)
		{
			// shift data out
			m_sdo = BIT(m_shift, 7);
			m_shift <<= 1;
			m_bits++;

			if (m_bits == 8)
			{
				m_bits = 0;

				// increment address counter
				m_address = next_address(m_address & 0x7f);
				m_shift = read_register(m_address);
				LOG("MCCS1850 Data Out %02x\n", m_shift);
			}
		}
		break;
	}

	m_sck = state;
}


//-------------------------------------------------
//  sdo_r - serial data out read
//-------------------------------------------------

int motorola_serial_rtc_device_base::sdo_r()
{
	if (!m_ce || BIT(m_address, 7))
	{
		// Hi-Z
		return 1;
	}
	else
	{
		return m_sdo;
	}
}


//-------------------------------------------------
//  sdi_w - serial data in write
//-------------------------------------------------

void motorola_serial_rtc_device_base::sdi_w(int state)
{
	m_sdi = state;
}


//-------------------------------------------------
//  pwrsw_w - power switch write
//-------------------------------------------------

void mccs1850_device::pwrsw_w(int state)
{
	if (!state)
	{
		if (m_pse)
		{
			// request to power down
			m_ram[REGISTER_STATUS] |= STATUS_RPD;
			check_interrupt();
		}

		set_pse_line(true);
	}
}


//-------------------------------------------------
//  por_w - power on reset write
//-------------------------------------------------

void mccs1850_device::por_w(int state)
{
	if (!state)
	{
		device_reset();
	}
}


//-------------------------------------------------
//  test_w - test mode write
//-------------------------------------------------

void mccs1850_device::test_w(int state)
{
	if (state)
	{
		LOG("MCCS1850 Test Mode\n");

		m_ram[REGISTER_STATUS] |= STATUS_TM;
	}
	else
	{
		LOG("MCCS1850 Normal Operation\n");

		m_ram[REGISTER_STATUS] &= ~STATUS_TM;
	}
}

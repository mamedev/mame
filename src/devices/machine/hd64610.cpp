// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**********************************************************************

    Hitachi HD64610 Real Time Clock

*********************************************************************/

/*

    TODO:
    - leap year
    - test mode

*/

#include "hd64610.h"
#include "coreutil.h"


// device type definition
const device_type HD64610 = &device_creator<hd64610_device>;


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 1

// internal registers
enum
{
	REG_64HZ = 0,
	REG_SECOND,
	REG_MINUTE,
	REG_HOUR,
	REG_DAY_OF_THE_WEEK,
	REG_DAY,
	REG_MONTH,
	REG_YEAR,
	REG_64HZ_ALARM,
	REG_SECOND_ALARM,
	REG_MINUTE_ALARM,
	REG_HOUR_ALARM,
	REG_DAY_OF_THE_WEEK_ALARM,
	REG_DAY_ALARM,
	REG_CRA,
	REG_CRB
};


// Control Register A
#define CRA_CF          0x80
#define CRA_CIE         0x10
#define CRA_AIE         0x08
#define CRA_AF          0x01

// Control Register B
#define CRB_TEST        0x08
#define CRB_ADJ         0x04
#define CRB_RESET       0x02
#define CRB_S           0x01

// alarm
#define ALARM_ENB       0x80

// register write mask
static const int REG_WRITE_MASK[0x10] =
{
	0x00, 0x7f, 0x7f, 0x3f, 0x07, 0x3f, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xbf, 0x83, 0xbf, 0x18, 0xff
};

//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  set_irq_line -
//-------------------------------------------------

inline void hd64610_device::set_irq_line()
{
	int irq_out = (((m_regs[REG_CRA] & CRA_CF) && (m_regs[REG_CRA] & CRA_CIE)) ||
					((m_regs[REG_CRA] & CRA_AF) && (m_regs[REG_CRA] & CRA_AIE))) ? 0 : 1;

	if (m_irq_out != irq_out)
	{
		if (LOG) logerror("HD64610 '%s' IRQ %u\n", tag().c_str(), irq_out);

		m_out_irq_cb(irq_out);
		m_irq_out = irq_out;
	}
}


//-------------------------------------------------
//  read_counter -
//-------------------------------------------------

inline UINT8 hd64610_device::read_counter(int counter)
{
	return bcd_2_dec(m_regs[counter]);
}


//-------------------------------------------------
//  write_counter -
//-------------------------------------------------

inline void hd64610_device::write_counter(int counter, UINT8 value)
{
	m_regs[counter] = dec_2_bcd(value);
}


//-------------------------------------------------
//  check_alarm -
//-------------------------------------------------

inline void hd64610_device::check_alarm()
{
	bool alarm_flag = true;

	// clear alarm flag
	m_regs[REG_CRA] &= ~CRA_AF;

	if (m_regs[REG_64HZ_ALARM] & ALARM_ENB || m_regs[REG_SECOND_ALARM] & ALARM_ENB || m_regs[REG_MINUTE_ALARM] & ALARM_ENB ||
		m_regs[REG_HOUR_ALARM] & ALARM_ENB || m_regs[REG_DAY_OF_THE_WEEK_ALARM] & ALARM_ENB || m_regs[REG_DAY_ALARM] & ALARM_ENB)
	{
		// at least one ENB is active

		for (int i = REG_64HZ; i <= REG_DAY; i++)
		{
			if ((m_regs[i] & REG_WRITE_MASK[i]) != (m_regs[i+8] & REG_WRITE_MASK[i]) && m_regs[i+8] & ALARM_ENB)
					alarm_flag = false;
		}

		m_regs[REG_CRA] |= (alarm_flag ? CRA_AF : 0);
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  hd64610_device - constructor
//-------------------------------------------------

hd64610_device::hd64610_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, HD64610, "HD64610", tag, owner, clock, "hd64610", __FILE__),
		device_rtc_interface(mconfig, *this),
		device_nvram_interface(mconfig, *this),
		m_out_irq_cb(*this),
		m_out_1hz_cb(*this),
		m_hline_state(1),
		m_irq_out(1)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hd64610_device::device_start()
{
	// resolve callbacks
	m_out_irq_cb.resolve_safe();
	m_out_1hz_cb.resolve_safe();

	// allocate timers
	m_counter_timer = timer_alloc(TIMER_UPDATE_COUNTER);
	m_counter_timer->adjust(attotime::from_hz(clock() / 256), 0, attotime::from_hz(clock() / 256));

	// state saving
	save_item(NAME(m_regs));
	save_item(NAME(m_irq_out));
	save_item(NAME(m_hline_state));
}


//-------------------------------------------------
//  device_start - device-specific reset
//-------------------------------------------------

void hd64610_device::device_reset()
{
	set_current_time(machine());
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void hd64610_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_UPDATE_COUNTER:
		if(m_hline_state || (m_regs[REG_CRB] & CRB_S))
		{
			m_regs[REG_64HZ]++;

			if (m_regs[REG_64HZ] & 0x80)
			{
				// update seconds
				advance_seconds();

				// set carry flag
				m_regs[REG_CRA] |= CRA_CF;

				m_regs[REG_64HZ] &= 0x7f;
			}

			// update 1Hz out
			m_out_1hz_cb(BIT(m_regs[REG_64HZ], 6));

			// update IRQ
			check_alarm();
			set_irq_line();
		}
		break;
	}
}


//-------------------------------------------------
//  rtc_clock_updated -
//-------------------------------------------------

void hd64610_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	write_counter(REG_SECOND, second);
	write_counter(REG_MINUTE, minute);
	write_counter(REG_HOUR, hour);
	write_counter(REG_DAY, day);
	write_counter(REG_MONTH, month);
	write_counter(REG_YEAR, year);
	m_regs[REG_DAY_OF_THE_WEEK] = day_of_week;

	check_alarm();
	set_irq_line();
}


//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void hd64610_device::nvram_default()
{
	memset(m_regs, 0, 0x10);
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void hd64610_device::nvram_read(emu_file &file)
{
	file.read(m_regs, 0x10);
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void hd64610_device::nvram_write(emu_file &file)
{
	file.write(m_regs, 0x10);
}


//-------------------------------------------------
//  hardware start/stop
//-------------------------------------------------

WRITE_LINE_MEMBER( hd64610_device::h_w )
{
	m_hline_state = state;
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( hd64610_device::read )
{
	UINT8 data =  m_regs[offset & 0x0f];

	if (LOG) logerror("HD64610 '%s' Register %u Read %02x\n", tag().c_str(), offset, data);

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( hd64610_device::write )
{
	switch (offset & 0x0f)
	{
	case REG_64HZ:
		// read only
		logerror("HD64610 '%s' Writing to read-only register 64Hz Counter\n", tag().c_str());
		break;

	case REG_CRA:
		m_regs[REG_CRA] = data & REG_WRITE_MASK[REG_CRA];

		if ((data & CRA_CF) == 0)
		{
			if (LOG) logerror("HD64610 '%s' clear carry flag\n", tag().c_str());
			m_regs[REG_CRA] &= 0x7f;
		}
		if ((data & CRA_AF) == 0)
		{
			if (LOG) logerror("HD64610 '%s' clear alarm flag\n", tag().c_str());
			m_regs[REG_CRA] &= 0xfe;
		}

		if (LOG) logerror("HD64610 '%s' set alarm IRQ %d\n", tag().c_str(), BIT(data, 3));
		if (LOG) logerror("HD64610 '%s' set carry IRQ %d\n", tag().c_str(), BIT(data, 4));
		break;

	case REG_CRB:
		m_regs[REG_CRB] = data & REG_WRITE_MASK[REG_CRB];

		if (data & CRB_ADJ)
		{
			if (LOG) logerror("HD64610 '%s' 30-sec adjustament\n", tag().c_str());
			adjust_seconds();
			m_regs[REG_64HZ] = 0;

			m_regs[REG_CRB] &= ~CRB_ADJ;
		}

		if (data & CRB_RESET)
		{
			if (LOG) logerror("HD64610 '%s' CRB reset\n", tag().c_str());
			m_regs[REG_64HZ] = 0;

			m_regs[REG_CRB] &= ~CRB_RESET;
		}

		if (LOG) logerror("HD64610 '%s' set timer %d\n", tag().c_str(), BIT(data, 0));
		break;

	default:
		m_regs[offset & 0x0f] = data & REG_WRITE_MASK[offset & 0x0f];
		if (LOG) logerror("HD64610 '%s' Register %u Write %02x\n", tag().c_str(), offset & 0x0f, data);
		break;
	}
}

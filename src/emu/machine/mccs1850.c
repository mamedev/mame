// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Motorola MCCS1850 Serial Real-Time Clock emulation

*********************************************************************/

/*

    TODO:

    - auto restart
    - test mode

*/

#include "mccs1850.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


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



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type MCCS1850 = &device_creator<mccs1850_device>;



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  check_interrupt -
//-------------------------------------------------

inline void mccs1850_device::check_interrupt()
{
	UINT8 status = m_ram[REGISTER_STATUS];
	UINT8 control = m_ram[REGISTER_CONTROL];

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

	if(!int_cb.isnull())
		int_cb(interrupt);
}


//-------------------------------------------------
//  set_pse_line -
//-------------------------------------------------

inline void mccs1850_device::set_pse_line(bool state)
{
	m_pse = state;

	if(!pse_cb.isnull())
		pse_cb(m_pse);
}


//-------------------------------------------------
//  read_register -
//-------------------------------------------------

inline UINT8 mccs1850_device::read_register(offs_t offset)
{
	switch (offset)
	{
	case REGISTER_COUNTER_LATCH:
	case REGISTER_COUNTER_LATCH+3: // Required by the NeXT power on test
		// load counter value into latch
		m_ram[REGISTER_COUNTER_LATCH] = m_counter >> 24;
		m_ram[REGISTER_COUNTER_LATCH + 1] = m_counter >> 16;
		m_ram[REGISTER_COUNTER_LATCH + 2] = m_counter >> 8;
		m_ram[REGISTER_COUNTER_LATCH + 3] = m_counter;
		break;

	case REGISTER_TEST_1:
	case REGISTER_TEST_2:
	case REGISTER_TEST_KICK_START_COUNTER:
	case REGISTER_TEST_PRESCALE_COUNTER:
	case REGISTER_TEST_COUNTER_INCREMENT:
		logerror("MCCS1850 '%s' Unsupported read from test register %02x!\n", tag(), offset);
		break;
	}

	return m_ram[offset];
}


//-------------------------------------------------
//  write_register -
//-------------------------------------------------

inline void mccs1850_device::write_register(offs_t offset, UINT8 data)
{
	switch (offset)
	{
	case REGISTER_STATUS:
		// read only
		break;

	case REGISTER_CONTROL:
		if (LOG) logerror("MCCS1850 '%s' Counter %s\n", tag(), (data & CONTROL_STR_STP) ? "Start" : "Stop");
		m_clock_timer->enable(data & CONTROL_STR_STP);

		if (data & CONTROL_PD)
		{
			if (LOG) logerror("MCCS1850 '%s' Power Down\n", tag());
			set_pse_line(false);
		}

		if (data & CONTROL_AR)
		{
			if (LOG) logerror("MCCS1850 '%s' Auto Restart\n", tag());
		}

		if (data & CONTROL_AC)
		{
			if (LOG) logerror("MCCS1850 '%s' Alarm Clear\n", tag());
			m_ram[REGISTER_STATUS] &= ~STATUS_AI;
		}

		if (data & CONTROL_FTUC)
		{
			if (LOG) logerror("MCCS1850 '%s' First Time Up Clear\n", tag());
			m_ram[REGISTER_STATUS] &= ~STATUS_FTU;
		}

		if (data & CONTROL_RPCD)
		{
			if (LOG) logerror("MCCS1850 '%s' Request to Power Down Clear\n", tag());
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
		logerror("MCCS1850 '%s' Unsupported write to test register %02x!\n", tag(), offset);
		break;

	default:
		m_ram[offset] = data;
	}
}


//-------------------------------------------------
//  advance_seconds -
//-------------------------------------------------

inline void mccs1850_device::advance_seconds()
{
	UINT32 alarm = (m_ram[REGISTER_ALARM_LATCH] << 24) | (m_ram[REGISTER_ALARM_LATCH + 1] << 16) | (m_ram[REGISTER_ALARM_LATCH + 2] << 8) | m_ram[REGISTER_ALARM_LATCH + 3];

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



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mccs1850_device - constructor
//-------------------------------------------------

mccs1850_device::mccs1850_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, MCCS1850, "MCCS1850", tag, owner, clock, "mccs1850", __FILE__),
	device_rtc_interface(mconfig, *this),
	device_nvram_interface(mconfig, *this),
	int_cb(*this),
	pse_cb(*this),
	nuc_cb(*this),
	m_pse(1),
	m_counter(0),
	m_ce(0),
	m_sck(0),
	m_sdo(1),
	m_sdi(0),
	m_state(STATE_ADDRESS),
	m_bits(0)
{
}


//-------------------------------------------------
//  set_counter - set the counter at startup time
//-------------------------------------------------

void mccs1850_device::set_counter(UINT32 value)
{
	m_counter = value;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mccs1850_device::device_start()
{
	// resolve callbacks
	int_cb.resolve();
	pse_cb.resolve();
	nuc_cb.resolve();

	// allocate timers
	m_clock_timer = timer_alloc(TIMER_CLOCK);
	m_clock_timer->adjust(attotime::from_hz(clock() / 32768), 0, attotime::from_hz(clock() / 32768));

	// state saving
	save_item(NAME(m_pse));
	save_item(NAME(m_counter));
	save_item(NAME(m_ce));
	save_item(NAME(m_sck));
	save_item(NAME(m_sdo));
	save_item(NAME(m_sdi));
	save_item(NAME(m_state));
	save_item(NAME(m_address));
	save_item(NAME(m_bits));
	save_item(NAME(m_shift));
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


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void mccs1850_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_CLOCK:
		advance_seconds();
		break;
	}
}


//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void mccs1850_device::nvram_default()
{
	memset(m_ram, 0xff, RAM_SIZE);

	if (machine().root_device().memregion(tag()) != NULL)
	{
		UINT8 *nvram = machine().root_device().memregion(tag())->base();

		// initialize NVRAM
		memcpy(m_ram, nvram, 0x20);
	}
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void mccs1850_device::nvram_read(emu_file &file)
{
	file.read(m_ram, RAM_SIZE);
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void mccs1850_device::nvram_write(emu_file &file)
{
	file.write(m_ram, RAM_SIZE);
}


//-------------------------------------------------
//  ce_w - chip enable write
//-------------------------------------------------

WRITE_LINE_MEMBER( mccs1850_device::ce_w )
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

WRITE_LINE_MEMBER( mccs1850_device::sck_w )
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
				if (LOG) logerror("MCCS1850 '%s' %s Address %02x\n", tag(), BIT(m_address, 7) ? "Write" : "Read", m_address & 0x7f);

				m_bits = 0;
				m_state = STATE_DATA;

				if (!BIT(m_address, 7))
				{
					m_shift = read_register(m_address & 0x7f);

					if (LOG) logerror("MCCS1850 '%s' Data Out %02x\n", tag(), m_shift);
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
				if (LOG) logerror("MCCS1850 '%s' Data In %02x\n", tag(), m_shift);

				write_register(m_address & 0x7f, m_shift);

				m_bits = 0;

				// increment address counter
				m_address++;
				m_address |= 0x80;
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
				m_address++;
				m_address &= 0x7f;
				m_shift = read_register(m_address & 0x7f);
				if (LOG) logerror("MCCS1850 '%s' Data Out %02x\n", tag(), m_shift);
			}
		}
		break;
	}

	m_sck = state;
}


//-------------------------------------------------
//  sdo_r - serial data out read
//-------------------------------------------------

READ_LINE_MEMBER( mccs1850_device::sdo_r )
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

WRITE_LINE_MEMBER( mccs1850_device::sdi_w )
{
	m_sdi = state;
}


//-------------------------------------------------
//  pwrsw_w - power switch write
//-------------------------------------------------

WRITE_LINE_MEMBER( mccs1850_device::pwrsw_w )
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

WRITE_LINE_MEMBER( mccs1850_device::por_w )
{
	if (!state)
	{
		device_reset();
	}
}


//-------------------------------------------------
//  test_w - test mode write
//-------------------------------------------------

WRITE_LINE_MEMBER( mccs1850_device::test_w )
{
	if (state)
	{
		if (LOG) logerror("MCCS1850 '%s' Test Mode\n", tag());

		m_ram[REGISTER_STATUS] |= STATUS_TM;
	}
	else
	{
		if (LOG) logerror("MCCS1850 '%s' Normal Operation\n", tag());

		m_ram[REGISTER_STATUS] &= ~STATUS_TM;
	}
}

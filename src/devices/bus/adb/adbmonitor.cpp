// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  adbmonitor.cpp - ADB bus monitor device

***************************************************************************/

#include "emu.h"
#include "adbmonitor.h"

#define LOG_RAW_LINESTATES  (1U << 1)
#define LOG_BIT_BY_BIT      (1U << 2)
#define LOG_START_STOP      (1U << 3)

#define VERBOSE (LOG_GENERAL|LOG_RAW_LINESTATES|LOG_START_STOP)
#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

// use 1 MHz base to get microseconds
static constexpr int adb_timebase = 1000000;

// ADB line states
enum
{
	// receive states
	LST_IDLE = 0,
	LST_ATTENTION,
	LST_BIT0,
	LST_BIT1,
	LST_BIT2,
	LST_BIT3,
	LST_BIT4,
	LST_BIT5,
	LST_BIT6,
	LST_BIT7,
	LST_TSTOP,
	LST_WAITT1T,
	LST_RCVSTARTBIT,
	LST_SRQNODATA,

	// send states
	LST_TSTOPSTART,
	LST_TSTOPSTARTa,
	LST_STARTBIT,
	LST_SENDBIT0,
	LST_SENDBIT0a,
	LST_SENDBIT1,
	LST_SENDBIT1a,
	LST_SENDBIT2,
	LST_SENDBIT2a,
	LST_SENDBIT3,
	LST_SENDBIT3a,
	LST_SENDBIT4,
	LST_SENDBIT4a,
	LST_SENDBIT5,
	LST_SENDBIT5a,
	LST_SENDBIT6,
	LST_SENDBIT6a,
	LST_SENDBIT7,
	LST_SENDBIT7a,
	LST_SENDSTOP,
	LST_SENDSTOPa
};

// device type definition
DEFINE_DEVICE_TYPE(ADB_MONITOR, adb_monitor_device, "adbmon", "ADB Monitor Device")

adb_monitor_device::adb_monitor_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
		adb_device_interface(mconfig, ADB_MONITOR, tag, owner, clock),
		adb_slot_card_interface(mconfig, *this, DEVICE_SELF),
		m_last_adb_time(0),
		m_linein(0),
		m_stream_ptr(0),
		m_linestate(LST_IDLE),
		m_command(0),
		m_to_host(false)
{
	std::fill(std::begin(m_buffer), std::end(m_buffer), 0);
}

void adb_monitor_device::device_start()
{
	save_item(NAME(m_buffer));
	save_item(NAME(m_last_adb_time));
	save_item(NAME(m_linein));
	save_item(NAME(m_stream_ptr));
	save_item(NAME(m_linestate));
	save_item(NAME(m_command));
	save_item(NAME(m_to_host));
}

void adb_monitor_device::device_reset()
{
	m_last_adb_time = 0;
}

void adb_monitor_device::adb_w(int state)
{
static char const *const states[] =
	{
		"idle",
		"attention",
		"bit0",
		"bit1",
		"bit2",
		"bit3",
		"bit4",
		"bit5",
		"bit6",
		"bit7",
		"tstop",
		"waitt1t",
		"rcvstartbit",
		"srqnodata"
	};


	if (state == m_linein)
	{
		return;
	}
	m_linein = state;

	int dtime = (int)(machine().time().as_ticks(adb_timebase) - m_last_adb_time);
	m_last_adb_time = machine().time().as_ticks(adb_timebase);

	if (m_linestate <= 12)
	{
		LOGMASKED(LOG_RAW_LINESTATES, "linechange: %d -> %d, duration %d μS (state %d = %s, to_host = %d)\n", state ^ 1, state, dtime, m_linestate, states[m_linestate], m_to_host);
	}
	else
	{
		LOGMASKED(LOG_RAW_LINESTATES, "linechange: %d -> %d, duration %d μS (state %d = %s, to_host = %d)\n", state ^ 1, state, dtime, m_linestate, states[m_linestate], m_to_host);
	}

	switch (m_linestate)
	{
		case LST_IDLE:
			if ((state) && (dtime >= 2250))     // reset
			{
				LOG("ADB RESET\n");
			}
			else if ((state) && (dtime >= 600))    // attention
			{
				LOGMASKED(LOG_START_STOP, "ADB ATTENTION\n");
				m_linestate++;
				m_to_host = false;
			}
			else if ((!state) && (dtime > 105) && (dtime <= 210)) // SRQ for response
			{
				LOGMASKED(LOG_START_STOP, "ADB SRQ\n");
				m_command = 0;
				m_linestate = LST_RCVSTARTBIT;
				m_to_host = true;
			}
			else if ((!state) && (dtime >= 70) && (dtime <= 105)) // T1T for response
			{
				LOGMASKED(LOG_START_STOP, "ADB T1T\n");
				m_command = 0;
				m_linestate = LST_RCVSTARTBIT;
				m_to_host = true;
			}
			break;

		case LST_ATTENTION:
			if ((!state) && (dtime >= 45))     // Tsync
			{
				LOGMASKED(LOG_START_STOP, "ADB Tsync\n");
				m_command = 0;
				m_linestate++;
			}
			break;

		case LST_BIT0:
		case LST_BIT1:
		case LST_BIT2:
		case LST_BIT3:
		case LST_BIT4:
		case LST_BIT5:
		case LST_BIT6:
		case LST_BIT7:
			if (!state)
			{
				// if bit cell time is near spec
				if (dtime < 75)
				{
					if (dtime >= 45)    // "1" bit
					{
						m_command |= 1;
					}
					LOGMASKED(LOG_BIT_BY_BIT, "ADB bit %d (dtime = %d)\n", m_command & 1, dtime);

					if (m_linestate != LST_BIT7)
					{
						m_command <<= 1;
					}
					m_linestate++;
					if (m_linestate == LST_TSTOP)
					{
						LOGMASKED(LOG_GENERAL, "ADB: %02x %s host\n", m_command, m_to_host ? "to" : "from");
					}
				}
				else
				{
					m_linestate = LST_IDLE;
				}
			}
			break;

		case LST_TSTOP:
			if (state)
			{
				LOGMASKED(LOG_START_STOP, "ADB TSTOP\n");
				if (m_to_host)
				{
					m_command = 0;
					m_linestate = LST_BIT0;
				}
				else
				{
					m_linestate = LST_IDLE;
				}
			}
			break;

		case LST_WAITT1T:
			if ((!state) && (dtime >= 150))     // T1t
			{
				LOGMASKED(LOG_START_STOP, "ADB T1t\n");
				m_linestate++;
			}
			break;

		case LST_RCVSTARTBIT:
			if ((!state) && (dtime >= 45))       // start
			{
				LOGMASKED(LOG_START_STOP, "ADB start\n");
				m_linestate = LST_BIT0;
				m_command = 0;
			}
			break;
	}
}

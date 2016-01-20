// license:BSD-3-Clause
// copyright-holders:smf
/*
 * DS2401
 *
 * Dallas Semiconductor
 * Silicon Serial Number
 *
 */

#include "emu.h"
#include "machine/ds2401.h"

#define VERBOSE_LEVEL 0

inline void ds2401_device::verboselog(int n_level, const char *s_fmt, ...)
{
	if(VERBOSE_LEVEL >= n_level)
	{
		va_list v;
		char buf[32768];
		va_start(v, s_fmt);
		vsprintf(buf, s_fmt, v);
		va_end(v);
		logerror("ds2401 %s %s: %s", tag().c_str(), machine().describe_context(), buf);
	}
}

// device type definition
const device_type DS2401 = &device_creator<ds2401_device>;

ds2401_device::ds2401_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DS2401, "DS2401", tag, owner, clock, "ds2401", __FILE__), m_state(0), m_bit(0), m_shift(0), m_byte(0), m_rx(false), m_tx(false), m_timer_main(nullptr), m_timer_reset(nullptr)
{
}

void ds2401_device::device_start()
{
	t_samp = attotime::from_usec( 30);
	t_rdv  = attotime::from_usec( 30);
	t_rstl = attotime::from_usec(480);
	t_pdh  = attotime::from_usec( 30);
	t_pdl  = attotime::from_usec(120);

	m_rx = true;
	m_tx = true;

	save_item(NAME(m_state));
	save_item(NAME(m_bit));
	save_item(NAME(m_byte));
	save_item(NAME(m_shift));
	save_item(NAME(m_rx));
	save_item(NAME(m_tx));

	m_timer_main  = timer_alloc(TIMER_MAIN);
	m_timer_reset = timer_alloc(TIMER_RESET);
}

void ds2401_device::device_reset()
{
	m_state = STATE_IDLE;
	m_bit = 0;
	m_byte = 0;
	m_shift = 0;
	m_rx = true;
	m_tx = true;

	if(m_region)
	{
		if(m_region->bytes() == SIZE_DATA)
		{
			memcpy(m_data, m_region->base(), SIZE_DATA);
			return;
		}

		logerror("ds2401 %s: Wrong region length for id data, expected 0x%x, got 0x%x\n", tag().c_str(), SIZE_DATA, m_region->bytes());
	}
	else
	{
		logerror("ds2401 %s: Warning, no id provided, answer will be all zeroes.\n", tag().c_str());
	}

	memset(m_data, 0, SIZE_DATA);
}

void ds2401_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case TIMER_RESET:
		verboselog(1, "timer_reset\n");
		m_state = STATE_RESET;
		m_timer_reset->adjust(attotime::never);
		break;

	case TIMER_MAIN:
		switch(m_state)
		{
		case STATE_RESET1:
			verboselog(2, "timer_main state_reset1 %d\n", m_rx);
			m_tx = false;
			m_state = STATE_RESET2;
			m_timer_main->adjust(t_pdl);
			break;

		case STATE_RESET2:
			verboselog(2, "timer_main state_reset2 %d\n", m_rx);
			m_tx = true;
			m_bit = 0;
			m_shift = 0;
			m_state = STATE_COMMAND;
			break;

		case STATE_COMMAND:
			verboselog(2, "timer_main state_command %d\n", m_rx);

			m_shift >>= 1;
			if(m_rx)
			{
				m_shift |= 0x80;
			}

			m_bit++;
			if(m_bit == 8)
			{
				switch(m_shift)
				{
				case COMMAND_READROM:
					verboselog(1, "timer_main readrom\n");
					m_bit = 0;
					m_byte = 0;
					m_state = STATE_READROM;
					break;

				default:
					verboselog(0, "timer_main command not handled %02x\n", m_shift);
					m_state = STATE_IDLE;
					break;
				}
			}
			break;

		case STATE_READROM:
			m_tx = true;

			if( m_byte == SIZE_DATA )
			{
				verboselog(1, "timer_main readrom finished\n");
				m_state = STATE_IDLE;
			}
			else
			{
				verboselog(2, "timer_main readrom window closed\n");
			}
			break;
		default:
			verboselog(0, "timer_main state not handled: %d\n", m_state);
			break;
		}
	}
}

WRITE_LINE_MEMBER( ds2401_device::write )
{
	verboselog(1, "write(%d)\n", state);

	if(!state && m_rx)
	{
		switch(m_state)
		{
		case STATE_IDLE:
			break;

		case STATE_COMMAND:
			verboselog(2, "state_command\n");
			m_timer_main->adjust(t_samp);
			break;

		case STATE_READROM:
			if(!m_bit)
			{
				m_shift = m_data[7 - m_byte];
				verboselog(1, "<- data %02x\n", m_shift);
			}

			m_tx = m_shift & 1;
			m_shift >>= 1;

			m_bit++;
			if(m_bit == 8)
			{
				m_bit = 0;
				m_byte++;
			}

			verboselog(2, "state_readrom %d\n", m_tx);
			m_timer_main->adjust(t_rdv);
			break;

		default:
			verboselog(0, "state not handled: %d\n", m_state );
			break;
		}

		m_timer_reset->adjust(t_rstl);
	}
	else if(state && !m_rx)
	{
		switch(m_state)
		{
		case STATE_RESET:
			m_state = STATE_RESET1;
			m_timer_main->adjust(t_pdh);
			break;
		}

		m_timer_reset->adjust(attotime::never);
	}

	m_rx = state;
}

READ_LINE_MEMBER( ds2401_device::read )
{
	verboselog(2, "read %d\n", m_tx && m_rx);
	return m_tx && m_rx;
}

UINT8 ds2401_device::direct_read(int index)
{
	return m_data[index];
}

/*

app74.pdf

Under normal circumstances an ibutton will sample the line 30us after the falling edge of the start condition.
The internal time base of ibutton may deviate from its nominal value. The allowed tollerance band ranges from 15us to 60us.
This means that the actual slave sampling may occur anywhere from 15 and 60us after the start condition, which is a ratio of 1 to 4.
During this time frame the voltage on the data line must stay below Vilmax or above Vihmin.

In the 1-Wire system, the logical values 1 and 0 are represented by certain voltages in special waveforms.
The waveforms needed to write commands or data to ibuttons are called write-1 and write-0 time slots.
The duration of a low pulse to write a 1 must be shorter than 15us.
To write a 0, the duration of the low pulse must be at least 60us to cope with worst-case conditions.

The duration of the active part of a time slot can be extended beyond 60us.
The maximum extension is limited by the fact that a low pulse of a duration of at least eight active time slots ( 480us ) is defined as a Reset Pulse.
Allowing the same worst-case tolerance ratio, a low pulse of 120us might be sufficient for a reset.
This limits the extension of the active part of a time slot to a maximum of 120us to prevent misinterpretation with reset.

Commands and data are sent to ibuttons by combining write-0 and write-1 time slots.
To read data, the master has to generate read-data time slots to define the start condition of each bit.
The read-data time slots looks essentially the same as a write-1 time slot from the masters point of view.
Starting at the high-to-low transition, the ibuttons sends 1 bit of its addressed contents.
If the data bit is a 1, the ibutton leaves the pulse unchanged.
If the data bit is a 0, the ibutton will pull the data line low for 15us.
In this time frame data is valid for reading by the master.
The duration of the low pulse sent by the master should be a minimum of 1us with a maximum value as short as possible to maximize the master sampling window.

The Reset Pulse provides a clear starting condition that supersedes any time slot synchronisation.
It is defined as single low pulse of minimum duration of eight time slots or 480us followed by a Reset-high time tRSTH of another 480us.
After a Reset Pulse has been sent, the ibutton will wait for the time tPDH and then generate a Pulse-Presence Pulse of duration tPDL.
No other communication on the 1-Wire bus is allowed during tRSTH.

There are 1,000 microseconds in a millisecond, and 1,000 milliseconds in a second.
Thus, there are 1,000,000 microseconds in a second. Why is it "usec"?
The "u" is supposed to look like the Greek letter Mu that we use for "micro". .
*/

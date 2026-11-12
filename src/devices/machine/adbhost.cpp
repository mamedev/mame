// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
	adbhost.cpp - Apple Desktop Bus host HLE mix-in
	by R. Belmont

	A mix-in to allow host devices to speak ADB without reimplementing
	the protocol themselves.

	Intended for two users: the M68HC05PGE microcontroller in the Duos,
	and the "ADB Cell" in Heathrow/Paddington.  And maybe also NeXT machines
	with ADB.

	Things you'll want to call:
	  adb_host_reset()            drive the reset pulse; adb_host_reset_done()
								  fires when the line is released again
	  adb_host_command(cmd)       start a transaction (only when idle):
								  attention, sync, command byte, stop bit, then
								  a Listen turnaround, a Talk response wait or
								  nothing, according to the command's op bits
	  adb_host_listen_data_w(b)   load the holding register with the next
								  Listen data byte; false = holding full
	  adb_host_abort()            release the line and go idle, no callbacks
	  adb_host_line_w(state)      the bus line changed

	Overrides you can/should implement:
	  adb_host_drive_line(state)  drive or release the bus
	  adb_host_tx_empty()         the holding register is free: after the
								  command byte is accepted and after every
								  holding -> shift register transfer
	  adb_host_listen_more()      asked at a Listen byte boundary when the
								  holding register is empty: true = wait for
								  more data, false = send the stop bit
	  adb_host_command_sent(srq)  the command's stop bit has been released;
								  srq = a device kept the line low
	  adb_host_rx_byte(b)         a Talk data byte arrived; false = the owner
								  cannot take it (the transaction ends OVERRUN)
	  adb_host_transaction_done(status)
	  adb_host_reset_done()

	Important:
	  - adb_host_drive_line() may re-enter adb_host_line_w(), so the phase
		and timer are always set before the line is driven.
*/

#include "emu.h"
#include "adbhost.h"

#define LOG_COMMAND (1U << 1)
#define LOG_DATA    (1U << 2)
#define LOG_ERROR   (1U << 3)

#define VERBOSE (0)
#define LOG_OUTPUT_FUNC device().logerror

#include "logmacro.h"

namespace
{

constexpr u32 BIT_CELL_US       = 100;
constexpr u32 BIT_ONE_LOW_US    = 35;
constexpr u32 BIT_ZERO_LOW_US   = 65;
constexpr u32 STOP_LOW_US       = 70;
constexpr u32 RESET_LOW_US      = 3'000;
constexpr u32 ATTENTION_LOW_US  = 800;
constexpr u32 SYNC_HIGH_US      = 70;
constexpr u32 TURNAROUND_US     = 200;
constexpr u32 BIT_LOW_MIN_US    = 20;
constexpr u32 BIT_LOW_MAX_US    = 90;
constexpr u32 BIT_ZERO_US       = 50;
constexpr u32 BIT_CELL_MIN_US   = 70;
constexpr u32 BIT_CELL_MAX_US   = 140;
constexpr u32 EDGE_TIMEOUT_US   = 120;
constexpr u32 TALK_TIMEOUT_US   = 350;
constexpr u32 DATA_WAIT_US      = 10;

} // anonymous namespace

adb_hle_host_interface::adb_hle_host_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "adbhost"),
	m_adb_timer(nullptr),
	m_adb_phase(adb_phase::IDLE),
	m_adb_line(true),
	m_adb_tx_data_valid(false),
	m_adb_transmitting_command(false),
	m_adb_listen_started(false),
	m_adb_command(0),
	m_adb_tx_data(0),
	m_adb_tx_shift(0),
	m_adb_tx_bit(0),
	m_adb_rx_shift(0),
	m_adb_rx_bit(0),
	m_adb_received_byte(false),
	m_adb_last_low(0),
	m_adb_last_edge(attotime::zero)
{
}

void adb_hle_host_interface::interface_pre_start()
{
	m_adb_timer = device().machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(adb_hle_host_interface::adb_tick), this));
}

void adb_hle_host_interface::interface_post_start()
{
	device().save_item(NAME(m_adb_phase));
	device().save_item(NAME(m_adb_line));
	device().save_item(NAME(m_adb_tx_data_valid));
	device().save_item(NAME(m_adb_transmitting_command));
	device().save_item(NAME(m_adb_listen_started));
	device().save_item(NAME(m_adb_command));
	device().save_item(NAME(m_adb_tx_data));
	device().save_item(NAME(m_adb_tx_shift));
	device().save_item(NAME(m_adb_tx_bit));
	device().save_item(NAME(m_adb_rx_shift));
	device().save_item(NAME(m_adb_rx_bit));
	device().save_item(NAME(m_adb_received_byte));
	device().save_item(NAME(m_adb_last_low));
	device().save_item(NAME(m_adb_last_edge));
}

void adb_hle_host_interface::interface_post_reset()
{
	m_adb_phase = adb_phase::IDLE;
	m_adb_line = true;
	m_adb_tx_data_valid = false;
	m_adb_transmitting_command = false;
	m_adb_listen_started = false;
	m_adb_command = 0;
	m_adb_tx_data = 0;
	m_adb_tx_shift = 0;
	m_adb_tx_bit = 0;
	m_adb_rx_shift = 0;
	m_adb_rx_bit = 0;
	m_adb_received_byte = false;
	m_adb_last_low = 0;
	m_adb_last_edge = device().machine().time();
	m_adb_timer->adjust(attotime::never);
	adb_host_drive_line(ASSERT_LINE);
}

//-------------------------------------------------
//  owner -> engine
//-------------------------------------------------

void adb_hle_host_interface::adb_host_reset()
{
	LOGMASKED(LOG_COMMAND, "ADB reset pulse\n");
	adb_host_abort();
	m_adb_phase = adb_phase::RESET_LOW;
	adb_host_drive_line(CLEAR_LINE);
	m_adb_timer->adjust(attotime::from_usec(RESET_LOW_US));
}

void adb_hle_host_interface::adb_host_command(u8 command)
{
	if (m_adb_phase != adb_phase::IDLE)
	{
		LOGMASKED(LOG_ERROR, "ADB command %02x while busy (phase %u)\n", command, u32(m_adb_phase));
	}
	LOGMASKED(LOG_COMMAND, "ADB command %02x\n", command);
	m_adb_command = command;
	m_adb_tx_shift = command;
	m_adb_tx_bit = 0;
	m_adb_tx_data_valid = false;
	m_adb_transmitting_command = true;
	m_adb_listen_started = false;
	m_adb_received_byte = false;
	m_adb_rx_shift = 0;
	m_adb_rx_bit = 0;
	m_adb_phase = adb_phase::ATTENTION_LOW;
	adb_host_drive_line(CLEAR_LINE);
	m_adb_timer->adjust(attotime::from_usec(ATTENTION_LOW_US));

	adb_host_tx_empty();
}

bool adb_hle_host_interface::adb_host_listen_data_w(u8 data)
{
	if (m_adb_tx_data_valid)
	{
		LOGMASKED(LOG_ERROR, "ADB transmit data overrun writing %02x\n", data);
		return false;
	}

	LOGMASKED(LOG_DATA, "ADB queued transmit byte %02x\n", data);
	m_adb_tx_data = data;
	m_adb_tx_data_valid = true;
	return true;
}

void adb_hle_host_interface::adb_host_abort()
{
	m_adb_timer->adjust(attotime::never);
	m_adb_phase = adb_phase::IDLE;
	m_adb_tx_data_valid = false;
	m_adb_transmitting_command = false;
	m_adb_listen_started = false;
	m_adb_rx_shift = 0;
	m_adb_rx_bit = 0;
	m_adb_received_byte = false;
	adb_host_drive_line(ASSERT_LINE);
}

void adb_hle_host_interface::adb_host_line_w(int state)
{
	bool const line = bool(state);
	if (line == m_adb_line)
	{
		return;
	}

	attotime const now = device().machine().time();
	u32 const duration = adb_microseconds(now - m_adb_last_edge);
	m_adb_last_edge = now;
	m_adb_line = line;

	if (line)
	{
		switch (m_adb_phase)
		{
			case adb_phase::COMMAND_STOP_WAIT:
				// a device holding the line past our stop bit is a service request
				adb_finish_command_stop(duration > BIT_LOW_MAX_US);
				break;

			case adb_phase::TALK_START_LOW:
				if ((duration < BIT_LOW_MIN_US) || (duration > BIT_LOW_MAX_US) || (duration >= BIT_ZERO_US))
				{
					adb_finish_talk(adb_host_status::DATA_LOST);
				}
				else
				{
					m_adb_last_low = duration;
					m_adb_phase = adb_phase::TALK_BIT_HIGH;
					m_adb_timer->adjust(attotime::from_usec(EDGE_TIMEOUT_US));
				}
				break;

			case adb_phase::TALK_BIT_LOW:
				if ((duration < BIT_LOW_MIN_US) || (duration > BIT_LOW_MAX_US))
				{
					adb_finish_talk(adb_host_status::DATA_LOST);
				}
				else
				{
					bool const bit = duration < BIT_ZERO_US;
					m_adb_last_low = duration;
					if (m_adb_received_byte && !m_adb_rx_bit && !bit)
					{
						// a 0 at a byte boundary is the stop bit unless another cell follows
						m_adb_phase = adb_phase::TALK_STOP_HIGH;
					}
					else
					{
						adb_receive_bit(bit);
						if (m_adb_phase != adb_phase::IDLE)
						{
							m_adb_phase = adb_phase::TALK_BIT_HIGH;
						}
					}
					if (m_adb_phase != adb_phase::IDLE)
					{
						m_adb_timer->adjust(attotime::from_usec(EDGE_TIMEOUT_US));
					}
				}
				break;

			default:
				break;
		}
	}
	else
	{
		switch (m_adb_phase)
		{
			case adb_phase::TALK_WAIT:
				m_adb_phase = adb_phase::TALK_START_LOW;
				m_adb_timer->adjust(attotime::from_usec(EDGE_TIMEOUT_US));
				break;

			case adb_phase::TALK_BIT_HIGH:
				if (((m_adb_last_low + duration) < BIT_CELL_MIN_US) || ((m_adb_last_low + duration) > BIT_CELL_MAX_US))
				{
					adb_finish_talk(adb_host_status::DATA_LOST);
				}
				else
				{
					m_adb_phase = adb_phase::TALK_BIT_LOW;
					m_adb_timer->adjust(attotime::from_usec(EDGE_TIMEOUT_US));
				}
				break;

			case adb_phase::TALK_STOP_HIGH:
				if (((m_adb_last_low + duration) < BIT_CELL_MIN_US) || ((m_adb_last_low + duration) > BIT_CELL_MAX_US))
				{
					adb_finish_talk(adb_host_status::DATA_LOST);
				}
				else
				{
					// another cell followed: that 0 was data bit 0 of the next byte
					adb_receive_bit(false);
					if (m_adb_phase != adb_phase::IDLE)
					{
						m_adb_phase = adb_phase::TALK_BIT_LOW;
						m_adb_timer->adjust(attotime::from_usec(EDGE_TIMEOUT_US));
					}
				}
				break;

			default:
				break;
		}
	}
}

//-------------------------------------------------
//  transmit side: command and Listen data
//-------------------------------------------------

void adb_hle_host_interface::adb_start_transmit_bit()
{
	bool const bit = BIT(m_adb_tx_shift, 7 - m_adb_tx_bit);
	m_adb_phase = adb_phase::TRANSMIT_LOW;
	adb_host_drive_line(CLEAR_LINE);
	m_adb_timer->adjust(attotime::from_usec(bit ? BIT_ONE_LOW_US : BIT_ZERO_LOW_US));
}

void adb_hle_host_interface::adb_finish_transmit_byte()
{
	if (m_adb_transmitting_command)
	{
		m_adb_phase = adb_phase::COMMAND_STOP_LOW;
		adb_host_drive_line(CLEAR_LINE);
		m_adb_timer->adjust(attotime::from_usec(STOP_LOW_US));
	}
	else if (adb_load_listen_byte())
	{
		adb_start_transmit_bit();
	}
	else if (!adb_host_listen_more())
	{
		m_adb_phase = adb_phase::LISTEN_STOP_LOW;
		adb_host_drive_line(CLEAR_LINE);
		m_adb_timer->adjust(attotime::from_usec(STOP_LOW_US));
	}
	else
	{
		m_adb_phase = adb_phase::LISTEN_WAIT_DATA;
		m_adb_timer->adjust(attotime::from_usec(DATA_WAIT_US));
	}
}

void adb_hle_host_interface::adb_finish_command_stop(bool srq)
{
	m_adb_timer->adjust(attotime::never);
	m_adb_transmitting_command = false;
	adb_host_command_sent(srq);

	switch ((m_adb_command >> 2) & 3)
	{
		case 2:
			m_adb_phase = adb_phase::LISTEN_TURNAROUND;
			m_adb_timer->adjust(attotime::from_usec(TURNAROUND_US));
			break;

		case 3:
			adb_start_talk();
			break;

		default:
			m_adb_phase = adb_phase::IDLE;
			adb_host_transaction_done(adb_host_status::OK);
			break;
	}
}

void adb_hle_host_interface::adb_start_listen()
{
	if (adb_load_listen_byte())
	{
		m_adb_listen_started = true;
		m_adb_phase = adb_phase::LISTEN_START_LOW;
		adb_host_drive_line(CLEAR_LINE);
		m_adb_timer->adjust(attotime::from_usec(BIT_ONE_LOW_US));
	}
	else if (!adb_host_listen_more())
	{
		LOGMASKED(LOG_ERROR, "ADB Listen command completed without data\n");
		m_adb_phase = adb_phase::IDLE;
		adb_host_transaction_done(adb_host_status::NO_DATA);
	}
	else
	{
		m_adb_phase = adb_phase::LISTEN_WAIT_DATA;
		m_adb_timer->adjust(attotime::from_usec(DATA_WAIT_US));
	}
}

bool adb_hle_host_interface::adb_load_listen_byte()
{
	if (!m_adb_tx_data_valid)
	{
		return false;
	}

	m_adb_tx_shift = m_adb_tx_data;
	m_adb_tx_bit = 0;
	m_adb_tx_data_valid = false;
	adb_host_tx_empty();
	return true;
}

//-------------------------------------------------
//  receive side: Talk responses
//-------------------------------------------------

void adb_hle_host_interface::adb_start_talk()
{
	m_adb_phase = adb_phase::TALK_WAIT;
	m_adb_rx_shift = 0;
	m_adb_rx_bit = 0;
	m_adb_received_byte = false;
	m_adb_timer->adjust(attotime::from_usec(TALK_TIMEOUT_US));
}

void adb_hle_host_interface::adb_receive_bit(bool bit)
{
	m_adb_rx_shift = (m_adb_rx_shift << 1) | u8(bit);
	m_adb_rx_bit++;
	if (m_adb_rx_bit == 8)
	{
		adb_receive_byte();
	}
}

void adb_hle_host_interface::adb_receive_byte()
{
	u8 const data = m_adb_rx_shift;
	if (!adb_host_rx_byte(data))
	{
		LOGMASKED(LOG_ERROR, "ADB receive data overrun\n");
		adb_finish_talk(adb_host_status::OVERRUN);
		return;
	}

	LOGMASKED(LOG_DATA, "ADB received %02x\n", data);
	m_adb_rx_shift = 0;
	m_adb_rx_bit = 0;
	m_adb_received_byte = true;
}

void adb_hle_host_interface::adb_finish_talk(adb_host_status status)
{
	m_adb_timer->adjust(attotime::never);
	m_adb_phase = adb_phase::IDLE;
	if (status != adb_host_status::OK)
	{
		LOGMASKED(LOG_ERROR, "ADB Talk response missing or malformed (%u)\n", u32(status));
	}
	adb_host_transaction_done(status);
}

u32 adb_hle_host_interface::adb_microseconds(attotime duration)
{
	return duration.as_ticks(1'000'000);
}

//-------------------------------------------------
//  timer: every host-driven interval and every
//  receive timeout
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(adb_hle_host_interface::adb_tick)
{
	switch (m_adb_phase)
	{
		case adb_phase::RESET_LOW:
			m_adb_phase = adb_phase::IDLE;
			adb_host_drive_line(ASSERT_LINE);
			adb_host_reset_done();
			break;

		case adb_phase::ATTENTION_LOW:
			m_adb_phase = adb_phase::SYNC_HIGH;
			adb_host_drive_line(ASSERT_LINE);
			m_adb_timer->adjust(attotime::from_usec(SYNC_HIGH_US));
			break;

		case adb_phase::SYNC_HIGH:
			adb_start_transmit_bit();
			break;

		case adb_phase::TRANSMIT_LOW:
		{
			bool const bit = BIT(m_adb_tx_shift, 7 - m_adb_tx_bit);
			m_adb_phase = adb_phase::TRANSMIT_HIGH;
			adb_host_drive_line(ASSERT_LINE);
			m_adb_timer->adjust(attotime::from_usec(BIT_CELL_US - (bit ? BIT_ONE_LOW_US : BIT_ZERO_LOW_US)));
			break;
		}

		case adb_phase::TRANSMIT_HIGH:
			m_adb_tx_bit++;
			if (m_adb_tx_bit == 8)
			{
				adb_finish_transmit_byte();
			}
			else
			{
				adb_start_transmit_bit();
			}
			break;

		case adb_phase::COMMAND_STOP_LOW:
			// releasing the line can re-enter adb_host_line_w() through the bus,
			// so the phase and timeout are set first and checked again after
			m_adb_phase = adb_phase::COMMAND_STOP_WAIT;
			m_adb_timer->adjust(attotime::from_usec(TALK_TIMEOUT_US));
			adb_host_drive_line(ASSERT_LINE);
			if ((m_adb_phase == adb_phase::COMMAND_STOP_WAIT) && m_adb_line)
			{
				adb_finish_command_stop(false);
			}
			break;

		case adb_phase::COMMAND_STOP_WAIT:
			// the line never came back up after our stop bit
			adb_finish_talk(adb_host_status::DATA_LOST);
			break;

		case adb_phase::LISTEN_TURNAROUND:
			adb_start_listen();
			break;

		case adb_phase::LISTEN_START_LOW:
			m_adb_phase = adb_phase::LISTEN_START_HIGH;
			adb_host_drive_line(ASSERT_LINE);
			m_adb_timer->adjust(attotime::from_usec(BIT_CELL_US - BIT_ONE_LOW_US));
			break;

		case adb_phase::LISTEN_START_HIGH:
			adb_start_transmit_bit();
			break;

		case adb_phase::LISTEN_WAIT_DATA:
			if (m_adb_tx_data_valid)
			{
				if (m_adb_listen_started)
				{
					adb_load_listen_byte();
					adb_start_transmit_bit();
				}
				else
				{
					adb_start_listen();
				}
			}
			else if (!adb_host_listen_more())
			{
				if (m_adb_listen_started)
				{
					m_adb_phase = adb_phase::LISTEN_STOP_LOW;
					adb_host_drive_line(CLEAR_LINE);
					m_adb_timer->adjust(attotime::from_usec(STOP_LOW_US));
				}
				else
				{
					LOGMASKED(LOG_ERROR, "ADB Listen command completed without data\n");
					m_adb_phase = adb_phase::IDLE;
					adb_host_transaction_done(adb_host_status::NO_DATA);
				}
			}
			else
			{
				m_adb_timer->adjust(attotime::from_usec(DATA_WAIT_US));
			}
			break;

		case adb_phase::LISTEN_STOP_LOW:
			m_adb_phase = adb_phase::IDLE;
			adb_host_drive_line(ASSERT_LINE);
			adb_host_transaction_done(adb_host_status::OK);
			break;

		case adb_phase::TALK_WAIT:
			adb_finish_talk(adb_host_status::NO_RESPONSE);
			break;

		case adb_phase::TALK_START_LOW:
		case adb_phase::TALK_BIT_LOW:
		case adb_phase::TALK_BIT_HIGH:
			adb_finish_talk(adb_host_status::DATA_LOST);
			break;

		case adb_phase::TALK_STOP_HIGH:
			// silence after the stop bit: the response is complete
			adb_finish_talk(adb_host_status::OK);
			break;

		case adb_phase::IDLE:
		default:
			break;
	}
}

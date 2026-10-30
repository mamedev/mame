// license:BSD-3-Clause
// copyright-holders: R. Belmont, Olivier Galibert

// ADB HLE base class, inspired by nscsi_hle.cpp and years of learning what not to do with macadb

#include "emu.h"
#include "adbhle.h"

#define LOG_COMMAND (1U << 1)
#define LOG_TIMING  (1U << 2)
#define LOG_STATE   (1U << 3)

#define VERBOSE (0)

#include "logmacro.h"

namespace
{

constexpr unsigned RESET_LOW_US = 3'000;
constexpr unsigned ATTENTION_LOW_MIN_US = 500;
constexpr unsigned SYNC_HIGH_MIN_US = 35;
constexpr unsigned SYNC_HIGH_MAX_US = 110;
constexpr unsigned BIT_LOW_MIN_US = 20;
constexpr unsigned BIT_LOW_MAX_US = 90;
constexpr unsigned BIT_ZERO_THRESHOLD_US = 50;
constexpr unsigned BIT_CELL_MIN_US = 70;
constexpr unsigned BIT_CELL_MAX_US = 140;
constexpr unsigned BIT_HIGH_TIMEOUT_US = 110;
constexpr unsigned TURNAROUND_MIN_US = 100;
constexpr unsigned TURNAROUND_MAX_US = 300;
constexpr unsigned BIT_CELL_US = 100;
constexpr unsigned BIT_ONE_LOW_US = 35;
constexpr unsigned BIT_ZERO_LOW_US = 65;
constexpr unsigned STOP_LOW_US = 70;
constexpr unsigned TALK_TURNAROUND_US = 200;

} // anonymous namespace

adb_hle_device::adb_hle_device(
		const machine_config &mconfig,
		device_type type,
		const char *tag,
		device_t *owner,
		u32 clock,
		u8 default_address,
		u8 default_handler,
		address_mode mode) :
	adb_device_interface(mconfig, type, tag, owner, clock),
	adb_slot_card_interface(mconfig, *this, DEVICE_SELF),
	m_default_address(default_address & 0x0f),
	m_default_handler(default_handler),
	m_address_mode(mode),
	m_reset_timer(nullptr),
	m_receive_timer(nullptr),
	m_service_request_timer(nullptr),
	m_talk_timer(nullptr),
	m_transmit_timer(nullptr),
	m_phase(phase::IDLE),
	m_action(action::NONE),
	m_line_state(true),
	m_service_request(false),
	m_service_request_enabled(true),
	m_service_request_asserted(false),
	m_exceptional_event(true),
	m_collision(false),
	m_self_test_failed(false),
	m_active(mode == address_mode::MOVABLE),
	m_talk_prepared(false),
	m_talk_common(false),
	m_transmit_low(false),
	m_driving_low(false),
	m_address(default_address & 0x0f),
	m_handler(default_handler),
	m_command(0),
	m_command_bits(0),
	m_action_register(0),
	m_receive_byte(0),
	m_receive_bits(0),
	m_receive_length(0),
	m_transmit_length(0),
	m_transmit_cell(0),
	m_extended_length(0),
	m_random(1),
	m_last_low_duration(0),
	m_last_edge_time(attotime::zero),
	m_command_bit_zero_time(attotime::zero)
{
	std::fill(std::begin(m_receive_buffer), std::end(m_receive_buffer), 0);
	std::fill(std::begin(m_transmit_buffer), std::end(m_transmit_buffer), 0);
	std::fill(std::begin(m_extended_buffer), std::end(m_extended_buffer), 0);
}

void adb_hle_device::device_start()
{
	adb_device_interface::device_start();

	m_reset_timer = timer_alloc(FUNC(adb_hle_device::reset_tick), this);
	m_receive_timer = timer_alloc(FUNC(adb_hle_device::receive_timeout), this);
	m_service_request_timer = timer_alloc(FUNC(adb_hle_device::service_request_done), this);
	m_talk_timer = timer_alloc(FUNC(adb_hle_device::talk_turnaround_done), this);
	m_transmit_timer = timer_alloc(FUNC(adb_hle_device::transmit_tick), this);

	u8 seed = 0xa5;
	for (char const *cursor = tag(); *cursor; ++cursor)
	{
		seed = (seed * 33) ^ u8(*cursor);
	}
	if (!seed)
	{
		seed = 1;
	}
	m_random = seed;

	save_item(NAME(m_phase));
	save_item(NAME(m_action));
	save_item(NAME(m_line_state));
	save_item(NAME(m_service_request));
	save_item(NAME(m_service_request_enabled));
	save_item(NAME(m_service_request_asserted));
	save_item(NAME(m_exceptional_event));
	save_item(NAME(m_collision));
	save_item(NAME(m_self_test_failed));
	save_item(NAME(m_active));
	save_item(NAME(m_talk_prepared));
	save_item(NAME(m_talk_common));
	save_item(NAME(m_transmit_low));
	save_item(NAME(m_driving_low));
	save_item(NAME(m_address));
	save_item(NAME(m_handler));
	save_item(NAME(m_command));
	save_item(NAME(m_command_bits));
	save_item(NAME(m_action_register));
	save_item(NAME(m_receive_byte));
	save_item(NAME(m_receive_bits));
	save_item(NAME(m_receive_length));
	save_item(NAME(m_receive_buffer));
	save_item(NAME(m_transmit_length));
	save_item(NAME(m_transmit_cell));
	save_item(NAME(m_transmit_buffer));
	save_item(NAME(m_extended_length));
	save_item(NAME(m_extended_buffer));
	save_item(NAME(m_random));
	save_item(NAME(m_last_low_duration));
	save_item(NAME(m_last_edge_time));
	save_item(NAME(m_command_bit_zero_time));
}

void adb_hle_device::device_reset()
{
	adb_device_interface::device_reset();
	reset_protocol(false, phase::IDLE);
	m_line_state = true;
	m_last_edge_time = machine().time();
}

void adb_hle_device::adb_poll_inputs()
{
	adb_poll();
}

void adb_hle_device::adb_w(int state)
{
	bool const new_state = bool(state);
	if (new_state == m_line_state)
	{
		return;
	}

	attotime const now = machine().time();
	attotime const duration = now - m_last_edge_time;
	m_last_edge_time = now;
	m_line_state = new_state;

	if (!new_state)
	{
		m_reset_timer->adjust(attotime::from_usec(RESET_LOW_US));
		line_fell(duration);
	}
	else
	{
		m_reset_timer->adjust(attotime::never);
		line_rose(duration);
	}
}

void adb_hle_device::line_fell(attotime duration)
{
	u32 const high_us = microseconds(duration);

	if (m_phase == phase::TRANSMIT)
	{
		if (!m_driving_low)
		{
			LOGMASKED(LOG_STATE, "%s: lost ADB arbitration during a high portion\n", tag());
			finish_talk(false, true);
		}
		return;
	}

	if (m_phase == phase::TALK_WAIT)
	{
		LOGMASKED(LOG_STATE, "%s: another device started its response first\n", tag());
		finish_talk(false, true);
		return;
	}

	switch (m_phase)
	{
	case phase::SYNC:
		m_receive_timer->adjust(attotime::never);
		if ((high_us < SYNC_HIGH_MIN_US) || (high_us > SYNC_HIGH_MAX_US))
		{
			LOGMASKED(LOG_TIMING, "%s: invalid sync high time %u us\n", tag(), high_us);
			abort_receive();
			return;
		}
		m_phase = phase::COMMAND;
		m_command = 0;
		m_command_bits = 0;
		begin_command_bit();
		break;

	case phase::COMMAND:
		m_receive_timer->adjust(attotime::never);
		if (m_command_bits && !valid_cell(m_last_low_duration, high_us))
		{
			LOGMASKED(LOG_TIMING, "%s: invalid command cell %u+%u us\n", tag(), m_last_low_duration, high_us);
			abort_receive();
			return;
		}
		if (m_command_bits == 8)
		{
			command_stop_started();
		}
		else
		{
			begin_command_bit();
		}
		break;

	case phase::LISTEN_WAIT:
		m_receive_timer->adjust(attotime::never);
		if ((high_us < TURNAROUND_MIN_US) || (high_us > TURNAROUND_MAX_US))
		{
			LOGMASKED(LOG_TIMING, "%s: invalid listen turnaround %u us\n", tag(), high_us);
			abort_receive();
			return;
		}
		m_phase = phase::LISTEN_START;
		break;

	case phase::LISTEN_DATA:
		m_receive_timer->adjust(attotime::never);
		if (!valid_cell(m_last_low_duration, high_us))
		{
			LOGMASKED(LOG_TIMING, "%s: invalid listen cell %u+%u us\n", tag(), m_last_low_duration, high_us);
			abort_receive();
			return;
		}
		if (m_receive_bits == (m_receive_length * 8))
		{
			m_phase = phase::LISTEN_STOP;
		}
		break;

	case phase::IDLE:
	case phase::COMMAND_STOP:
	case phase::LISTEN_START:
	case phase::LISTEN_STOP:
	case phase::RESET_HOLD:
		break;

	default:
		abort_receive();
		break;
	}
}

void adb_hle_device::line_rose(attotime duration)
{
	u32 const low_us = microseconds(duration);

	if (m_phase == phase::RESET_HOLD)
	{
		m_phase = phase::IDLE;
		return;
	}

	if ((m_phase != phase::TRANSMIT) && (low_us >= ATTENTION_LOW_MIN_US))
	{
		begin_attention();
		return;
	}

	switch (m_phase)
	{
	case phase::IDLE:
		break;

	case phase::COMMAND:
		if (!valid_bit_low(low_us))
		{
			LOGMASKED(LOG_TIMING, "%s: invalid command low time %u us\n", tag(), low_us);
			abort_receive();
			return;
		}
		m_command = (m_command << 1) | decode_bit(low_us);
		m_last_low_duration = low_us;
		m_command_bits++;
		m_receive_timer->adjust(attotime::from_usec(BIT_HIGH_TIMEOUT_US));
		break;

	case phase::COMMAND_STOP:
		command_stop_finished();
		break;

	case phase::LISTEN_START:
		if (!valid_bit_low(low_us) || !decode_bit(low_us))
		{
			LOGMASKED(LOG_TIMING, "%s: invalid listen start bit (%u us)\n", tag(), low_us);
			abort_receive();
			return;
		}
		m_last_low_duration = low_us;
		m_receive_bits = 0;
		m_receive_byte = 0;
		m_phase = phase::LISTEN_DATA;
		m_receive_timer->adjust(attotime::from_usec(BIT_HIGH_TIMEOUT_US));
		break;

	case phase::LISTEN_DATA:
		if (!valid_bit_low(low_us))
		{
			LOGMASKED(LOG_TIMING, "%s: invalid listen low time %u us\n", tag(), low_us);
			abort_receive();
			return;
		}
		m_receive_byte = (m_receive_byte << 1) | decode_bit(low_us);
		m_receive_bits++;
		if (!(m_receive_bits & 7))
		{
			m_receive_buffer[(m_receive_bits / 8) - 1] = m_receive_byte;
			m_receive_byte = 0;
		}
		m_last_low_duration = low_us;
		m_receive_timer->adjust(attotime::from_usec(BIT_HIGH_TIMEOUT_US));
		break;

	case phase::LISTEN_STOP:
		if (!valid_bit_low(low_us) || decode_bit(low_us))
		{
			LOGMASKED(LOG_TIMING, "%s: invalid listen stop bit (%u us)\n", tag(), low_us);
			abort_receive();
			return;
		}
		finish_listen();
		break;

	case phase::TRANSMIT:
		break;

	default:
		abort_receive();
		break;
	}
}

void adb_hle_device::begin_attention()
{
	if (m_talk_prepared)
	{
		finish_talk(false, false);
	}
	m_receive_timer->adjust(attotime::from_usec(SYNC_HIGH_MAX_US));
	m_talk_timer->adjust(attotime::never);
	m_transmit_timer->adjust(attotime::never);
	m_phase = phase::SYNC;
	m_action = action::NONE;
	m_command_bits = 0;
}

void adb_hle_device::begin_command_bit()
{
	if (m_command_bits == 7)
	{
		m_command_bit_zero_time = machine().time();
	}
}

void adb_hle_device::command_stop_started()
{
	u8 const address = m_command >> 4;
	u8 const operation = (m_command >> 2) & 3;
	u8 const reg = m_command & 3;
	bool const send_reset = (m_command & 0x0f) == 0;
	bool const addressed = address == m_address;

	m_phase = phase::COMMAND_STOP;
	m_action = action::NONE;
	m_action_register = reg;

	LOGMASKED(LOG_COMMAND, "%s: command %02x (address %u, operation %u, register %u)\n", tag(), m_command, address, operation, reg);

	if (send_reset)
	{
		m_action = action::SEND_RESET;
		return;
	}
	adb_poll();

	if (!addressed)
	{
		if (m_active && m_service_request && m_service_request_enabled)
		{
			assert_service_request();
		}
		return;
	}

	if ((m_command & 0x0f) == 1)
	{
		if (m_active)
		{
			m_action = action::FLUSH;
		}
		return;
	}

	if (operation == 2)
	{
		if (m_active || (m_address_mode == address_mode::EXTENDED && ((reg == 2) || (reg == 3))))
		{
			m_action = action::LISTEN;
		}
		return;
	}

	if (operation == 3)
	{
		if (m_active || (m_address_mode == address_mode::EXTENDED && (reg == 3)))
		{
			prepare_talk(reg);
		}
	}
}

void adb_hle_device::command_stop_finished()
{
	switch (m_action)
	{
	case action::SEND_RESET:
		reset_protocol(true, phase::IDLE);
		break;

	case action::FLUSH:
		adb_flush();
		m_phase = phase::IDLE;
		break;

	case action::LISTEN:
		begin_listen();
		break;

	case action::TALK:
		if (m_talk_prepared)
		{
			m_phase = phase::TALK_WAIT;
			m_talk_timer->adjust(attotime::from_usec(TALK_TURNAROUND_US));
		}
		else
		{
			m_phase = phase::IDLE;
		}
		break;

	default:
		m_phase = phase::IDLE;
		break;
	}
	m_action = action::NONE;
}

void adb_hle_device::begin_listen()
{
	unsigned length;
	if (m_action_register == 3)
	{
		length = 2;
	}
	else if ((m_address_mode == address_mode::EXTENDED) && !m_active && (m_action_register == 2))
	{
		length = adb_extended_address(m_extended_buffer);
	}
	else
	{
		length = adb_listen_length(m_action_register);
	}

	if ((length < 2) || (length > std::size(m_receive_buffer)))
	{
		LOGMASKED(LOG_STATE, "%s: invalid register %u listen length %u\n", tag(), m_action_register, length);
		m_phase = phase::IDLE;
		return;
	}
	m_receive_length = length;
	if ((m_address_mode == address_mode::EXTENDED) && !m_active && (m_action_register == 2))
	{
		m_extended_length = length;
	}

	m_receive_bits = 0;
	m_receive_byte = 0;
	m_phase = phase::LISTEN_WAIT;
	m_receive_timer->adjust(attotime::from_usec(TURNAROUND_MAX_US));
}

void adb_hle_device::finish_listen()
{
	m_receive_timer->adjust(attotime::never);
	if (m_action_register == 3)
	{
		listen_register_three();
	}
	else if ((m_address_mode == address_mode::EXTENDED) && !m_active && (m_action_register == 2))
	{
		listen_extended_address();
	}
	else
	{
		adb_listen(m_action_register, std::span<u8 const>(m_receive_buffer, m_receive_length));
	}
	m_phase = phase::IDLE;
}

void adb_hle_device::abort_receive()
{
	m_receive_timer->adjust(attotime::never);
	m_action = action::NONE;
	m_phase = phase::IDLE;
}

void adb_hle_device::prepare_talk(u8 reg)
{
	unsigned length;
	m_talk_common = reg == 3;
	if (m_talk_common)
	{
		prepare_register_three();
		length = m_transmit_length;
	}
	else
	{
		length = adb_talk(reg, m_transmit_buffer);
	}

	if ((length < 2) || (length > std::size(m_transmit_buffer)))
	{
		m_transmit_length = 0;
		m_talk_prepared = false;
		return;
	}
	m_transmit_length = length;

	m_action = action::TALK;
	m_action_register = reg;
	m_talk_prepared = true;
}

void adb_hle_device::begin_talk()
{
	if (!m_talk_prepared)
	{
		m_phase = phase::IDLE;
		return;
	}
	if (!adb_line_r())
	{
		finish_talk(false, true);
		return;
	}

	m_phase = phase::TRANSMIT;
	m_transmit_cell = 0;
	m_transmit_low = false;
	begin_transmit_cell();
}

void adb_hle_device::begin_transmit_cell()
{
	if (!adb_line_r())
	{
		finish_talk(false, true);
		return;
	}

	bool const bit = transmit_bit();
	unsigned const low_time = (m_transmit_cell == ((m_transmit_length * 8) + 1)) ? STOP_LOW_US : (bit ? BIT_ONE_LOW_US : BIT_ZERO_LOW_US);
	m_transmit_low = true;
	m_driving_low = true;
	adb_drive_low();
	m_transmit_timer->adjust(attotime::from_usec(low_time));
}

void adb_hle_device::finish_talk(bool success, bool collision)
{
	m_talk_timer->adjust(attotime::never);
	m_transmit_timer->adjust(attotime::never);
	m_driving_low = false;
	if (adb_is_connected())
	{
		adb_release();
	}

	if (success)
	{
		m_collision = false;
	}
	else if (collision)
	{
		m_collision = true;
	}

	if (m_talk_prepared && !m_talk_common)
	{
		adb_talk_complete(m_action_register, success);
	}
	m_talk_prepared = false;
	m_talk_common = false;
	m_phase = phase::IDLE;
}

bool adb_hle_device::transmit_bit() const
{
	if (!m_transmit_cell)
	{
		return true;
	}
	if (m_transmit_cell == ((m_transmit_length * 8) + 1))
	{
		return false;
	}

	unsigned const data_bit = m_transmit_cell - 1;
	return BIT(m_transmit_buffer[data_bit / 8], 7 - (data_bit & 7));
}

void adb_hle_device::assert_service_request()
{
	unsigned const request_time = adb_service_request_time();
	attotime const deadline = m_command_bit_zero_time + attotime::from_usec(request_time);
	attotime const now = machine().time();

	m_service_request_asserted = true;
	adb_drive_low();
	m_service_request_timer->adjust(deadline > now ? deadline - now : attotime::zero);
}

void adb_hle_device::reset_protocol(bool notify_device, u8 next_phase)
{
	if (m_talk_prepared && notify_device && !m_talk_common)
	{
		adb_talk_complete(m_action_register, false);
	}

	m_reset_timer->adjust(attotime::never);
	m_receive_timer->adjust(attotime::never);
	m_service_request_timer->adjust(attotime::never);
	m_talk_timer->adjust(attotime::never);
	m_transmit_timer->adjust(attotime::never);
	m_phase = next_phase;
	m_action = action::NONE;
	m_service_request = false;
	m_service_request_enabled = true;
	m_service_request_asserted = false;
	m_exceptional_event = true;
	m_collision = false;
	m_self_test_failed = false;
	m_active = m_address_mode == address_mode::MOVABLE;
	m_talk_prepared = false;
	m_talk_common = false;
	m_transmit_low = false;
	m_driving_low = false;
	m_address = m_default_address;
	m_handler = m_default_handler;
	m_command = 0;
	m_command_bits = 0;
	m_receive_bits = 0;
	m_receive_length = 0;
	m_transmit_length = 0;
	m_extended_length = 0;

	if (adb_is_connected())
	{
		adb_release();
	}
	if (notify_device)
	{
		adb_bus_reset();
	}
}

void adb_hle_device::prepare_register_three()
{
	u8 const reported_address = (m_address_mode == address_mode::MOVABLE) ? next_random_nibble() : m_address;
	m_transmit_buffer[0] = (m_exceptional_event ? 0x40 : 0x00) | (m_service_request_enabled ? 0x20 : 0x00) | reported_address;
	m_transmit_buffer[1] = m_self_test_failed ? 0x00 : m_handler;
	m_transmit_length = 2;
}

void adb_hle_device::listen_register_three()
{
	u8 const requested_address = m_receive_buffer[0] & 0x0f;
	u8 const requested_handler = m_receive_buffer[1];

	switch (requested_handler)
	{
	case 0x00:
		m_address = requested_address;
		m_service_request_enabled = BIT(m_receive_buffer[0], 5);
		break;

	case 0xfd:
		if (adb_activator_pressed())
		{
			m_address = requested_address;
		}
		break;

	case 0xfe:
		if (!m_collision)
		{
			m_address = requested_address;
		}
		break;

	case 0xff:
		m_self_test_failed = !adb_self_test();
		break;

	default:
		if (adb_set_handler(requested_handler))
		{
			m_handler = requested_handler;
		}
		break;
	}
}

void adb_hle_device::listen_extended_address()
{
	if ((m_extended_length == m_receive_length) && std::equal(m_receive_buffer, m_receive_buffer + m_receive_length, m_extended_buffer))
	{
		m_active = true;
	}
}

u8 adb_hle_device::next_random_nibble()
{
	bool const feedback = BIT(m_random, 0);
	m_random >>= 1;
	if (feedback)
	{
		m_random ^= 0xb8;
	}
	if (!m_random)
	{
		m_random = 1;
	}
	return m_random & 0x0f;
}

u32 adb_hle_device::microseconds(attotime duration)
{
	return duration.as_ticks(1'000'000);
}

bool adb_hle_device::valid_bit_low(u32 duration)
{
	return (duration >= BIT_LOW_MIN_US) && (duration <= BIT_LOW_MAX_US);
}

bool adb_hle_device::valid_cell(u32 low, u32 high)
{
	u32 const duration = low + high;
	return (duration >= BIT_CELL_MIN_US) && (duration <= BIT_CELL_MAX_US);
}

bool adb_hle_device::decode_bit(u32 duration)
{
	return duration < BIT_ZERO_THRESHOLD_US;
}

TIMER_CALLBACK_MEMBER(adb_hle_device::reset_tick)
{
	if (adb_is_connected() && !adb_line_r())
	{
		LOGMASKED(LOG_STATE, "%s: ADB bus reset\n", tag());
		reset_protocol(true, phase::RESET_HOLD);
	}
}

TIMER_CALLBACK_MEMBER(adb_hle_device::receive_timeout)
{
	LOGMASKED(LOG_TIMING, "%s: receive timeout in phase %u\n", tag(), unsigned(m_phase));
	abort_receive();
}

TIMER_CALLBACK_MEMBER(adb_hle_device::service_request_done)
{
	if (m_service_request_asserted)
	{
		m_service_request_asserted = false;
		adb_release();
	}
}

TIMER_CALLBACK_MEMBER(adb_hle_device::talk_turnaround_done)
{
	begin_talk();
}

TIMER_CALLBACK_MEMBER(adb_hle_device::transmit_tick)
{
	if (m_phase != phase::TRANSMIT)
	{
		return;
	}

	if (m_transmit_low)
	{
		m_transmit_low = false;
		m_driving_low = false;
		adb_release();
		if (!adb_line_r())
		{
			finish_talk(false, true);
			return;
		}

		if (m_transmit_cell == ((m_transmit_length * 8) + 1))
		{
			finish_talk(true, false);
			return;
		}

		unsigned const high_time = transmit_bit() ? (BIT_CELL_US - BIT_ONE_LOW_US) : (BIT_CELL_US - BIT_ZERO_LOW_US);
		m_transmit_timer->adjust(attotime::from_usec(high_time));
	}
	else
	{
		m_transmit_cell++;
		begin_transmit_cell();
	}
}

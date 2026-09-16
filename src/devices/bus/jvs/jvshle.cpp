// license:BSD-3-Clause
// copyright-holders:smf

#include "emu.h"
#include "jvshle.h"
#include <algorithm>

#define LOG_COMMAND (1U << 1)
#define LOG_ERROR (1U << 2)
#define LOG_OUTPUT (1U << 3)

//#define VERBOSE (/*LOG_COMMAND | */LOG_ERROR | LOG_OUTPUT)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGCOMMAND(...) LOGMASKED(LOG_COMMAND, __VA_ARGS__)
#define LOGERROR(...) LOGMASKED(LOG_ERROR, __VA_ARGS__)
#define LOGOUTPUT(...) LOGMASKED(LOG_OUTPUT, __VA_ARGS__)

namespace {

struct State { enum : uint8_t
{
	Sync,
	Escape,
	EscapedData,
	Data
}; };

static constexpr uint8_t Sync = 0xe0;
static constexpr uint8_t Escape = 0xd0;

} // anonymous namespace

jvs_hle_device::jvs_hle_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_jvs_interface(mconfig, *this),
	m_rxd(1),
	m_output(0),
	m_analog_output{},
	m_analog_output_channels(0)
{
}

void jvs_hle_device::device_start()
{
	m_rx_timer = timer_alloc(FUNC(jvs_hle_device::rx_timer_callback), this);
	m_tx_timer = timer_alloc(FUNC(jvs_hle_device::tx_timer_callback), this);

	m_coin_timer = timer_alloc(FUNC(jvs_hle_device::coin_timer_callback), this);

	m_mainid.resize(100);
	m_mainid_length = 0;
	m_retransmit_buffer.resize(0x102);
	m_retransmit_length = 3;
	m_rx_bit = -1;
	m_rx_buffer.resize(0x102);
	m_rx_length = 0;
	m_rx_state = State::Sync;
	m_tx_bit = -1;
	m_tx_buffer.resize(0x102);
	m_tx_index = 0;
	m_tx_length = 0;
	m_tx_state = State::Sync;

	save_item(NAME(m_address));
	save_item(NAME(m_mainid));
	save_item(NAME(m_mainid_length));
	save_item(NAME(m_retransmit_buffer));
	save_item(NAME(m_retransmit_length));
	save_item(NAME(m_rx_bit));
	save_item(NAME(m_rx_buffer));
	save_item(NAME(m_rx_index));
	save_item(NAME(m_rx_length));
	save_item(NAME(m_rx_state));
	save_item(NAME(m_rxd));
	save_item(NAME(m_tx_bit));
	save_item(NAME(m_tx_buffer));
	save_item(NAME(m_tx_index));
	save_item(NAME(m_tx_length));
	save_item(NAME(m_tx_safe_length));
	save_item(NAME(m_tx_state));

	save_item(NAME(m_player_count));
	save_item(NAME(m_switch_count));
	save_item(NAME(m_switch_mask));
	save_item(NAME(m_coin_counter));
	save_item(NAME(m_coin_offset));
	save_item(NAME(m_coin_prev));
	save_item(NAME(m_analog_input_channels));
	save_item(NAME(m_analog_input_bits));
	save_item(NAME(m_rotary_input_channels));
	save_item(NAME(m_screen_position_input_channels));
	save_item(NAME(m_screen_position_input_xbits));
	save_item(NAME(m_screen_position_input_ybits));
	save_item(NAME(m_output));
	save_item(NAME(m_output_mask));
	save_item(NAME(m_output_slots));
	save_item(NAME(m_analog_output));
	save_item(NAME(m_analog_output_channels));
}

void jvs_hle_device::device_reset()
{
	rts(0);
	sense(jvs_port_device::sense::Uninitialized);
	rx_stop();
	rx_clear();

	std::fill_n(m_mainid.begin(), m_mainid_length, 0);
	m_mainid_length = 0;
	m_address = 0xff;
	m_retransmit_buffer[0] = 0x00; // destination
	m_retransmit_buffer[2] = StatusCode::Normal;
	std::fill_n(m_retransmit_buffer.begin() + 3, m_retransmit_length - 3, 0);
	m_retransmit_length = 3;
	m_rx_index = 0;

	m_player_count = player_count();
	m_switch_count = switch_count();
	m_switch_mask = switch_mask();
	m_coin_slots = coin_slots();
	m_coin_offset = coin_offset();

	m_coin_counter.fill(0);
	attotime const coin_timer_rate = m_coin_slots ? attotime::from_hz(120) : attotime::never;
	m_coin_timer->adjust(coin_timer_rate, 0, coin_timer_rate);
	m_coin_prev.fill(0xff);

	m_analog_input_channels = analog_input_channels();
	m_analog_input_bits = analog_input_bits();
	m_rotary_input_channels = rotary_input_channels();
	m_screen_position_input_channels = screen_position_input_channels();
	m_screen_position_input_xbits = screen_position_input_xbits();
	m_screen_position_input_xbits = screen_position_input_ybits();

	m_output_slots = output_slots();
	m_output_mask = jvs_bitmask<uint64_t>(m_output_slots);
	update_output(0);

	for (int i = 0; i < m_analog_output_channels; i++)
		update_analog_output(i, 0);

	m_analog_output_channels = analog_output_channels();
}

const char *jvs_hle_device::device_id()
{
	return "";
}

uint8_t jvs_hle_device::command_revision()
{
	return 0x13;
}

uint8_t jvs_hle_device::jvs_revision()
{
	return 0x30;
}

uint8_t jvs_hle_device::communication_revision()
{
	return 0x10;
}

void jvs_hle_device::feature_check()
{
	uint8_t *response;
	if (m_player_count && m_switch_count && produce(4, response))
	{
		response[0] = FunctionCode::SwitchInput;
		response[1] = m_player_count;
		response[2] = m_switch_count;
		response[3] = 0;
	}

	if (m_coin_slots && produce(4, response))
	{
		response[0] = FunctionCode::CoinInput;
		response[1] = m_coin_slots;
		response[2] = 0;
		response[3] = 0;
	}

	if (m_analog_input_channels && produce(4, response))
	{
		response[0] = FunctionCode::AnalogInput;
		response[1] = m_analog_input_channels;
		response[2] = m_analog_input_bits;
		response[3] = 0;
	}

	if (m_rotary_input_channels && produce(4, response))
	{
		response[0] = FunctionCode::RotaryInput;
		response[1] = m_rotary_input_channels;
		response[2] = 0;
		response[3] = 0;
	}

	if (m_screen_position_input_channels && produce(4, response))
	{
		response[0] = FunctionCode::ScreenPositionInput;
		response[1] = m_screen_position_input_xbits;
		response[2] = m_screen_position_input_ybits;
		response[3] = m_screen_position_input_channels;
	}

	if (m_output_slots && produce(4, response))
	{
		response[0] = FunctionCode::GeneralPurposeOutput;
		response[1] = m_output_slots;
		response[2] = 0;
		response[3] = 0;
	}

	if (m_analog_output_channels && produce(4, response))
	{
		response[0] = FunctionCode::AnalogOutput;
		response[1] = m_analog_output_channels;
		response[2] = 0;
		response[3] = 0;
	}
}

void jvs_hle_device::execute(uint8_t command)
{
	uint8_t const *parameters;
	uint8_t *response;

	switch (command)
	{
	case CommandCode::Reset:
		if (consume(1, parameters) && parameters[0] == 0xd9)
			device_reset();
		break;

	case CommandCode::SetAddress:
		if (consume(1, parameters) && m_jvs_sense != jvs_port_device::sense::Uninitialized && m_address == 0xff &&
			produce(1, response))
		{
			m_address = parameters[0];
			response[0] = ReportCode::Normal;
		}
		break;

	case CommandCode::IOIDENT:
	{
		const char *id = device_id();
		int len = strlen(id) + 1;
		if (produce(1 + len, response))
		{
			response[0] = ReportCode::Normal;
			memcpy(&response[1], id, len);
		}
		break;
	}

	case CommandCode::CMDREV:
		if (produce(2, response))
		{
			response[0] = ReportCode::Normal;
			response[1] = command_revision();
		}
		break;

	case CommandCode::JVSREV:
		if (produce(2, response))
		{
			response[0] = ReportCode::Normal;
			response[1] = jvs_revision();
		}
		break;

	case CommandCode::COMMVER:
		if (produce(2, response))
		{
			response[0] = ReportCode::Normal;
			response[1] = communication_revision();
		}
		break;

	case CommandCode::FEATCHK:
		if (produce(1, response))
		{
			response[0] = ReportCode::Normal;
			feature_check();
			if (produce(1, response))
				response[0] = 0x00;
		}
		break;

	case CommandCode::MAINID:
		if (consume(1, parameters))
		{
			uint8_t const *mainid = parameters;
			size_t const size = m_rx_length - m_rx_index;
			size_t length = 0;
			while (mainid[length] && length < size)
				length++;

			if (consume(length, parameters))
			{
				if (length < size && length < m_mainid.size())
				{
					if (produce(1, response))
					{
						std::copy_n(mainid, length, m_mainid.begin());
						if (m_mainid_length > length)
							std::fill_n(&m_mainid[length], m_mainid_length - length, 0);
						m_mainid_length = length;

						//std::string s(m_mainid.begin(), m_mainid.begin() + length);
						//osd_printf_info("mainid = %s\n", s);

						response[0] = ReportCode::Normal;
					}
				}
				else if (produce(1, response))
					response[0] = ReportCode::InvalidParameter;
			}
		}
		break;

	case CommandCode::SWINP:
		if (consume(2, parameters))
		{
			uint8_t const players = parameters[0];
			uint8_t const bytes = parameters[1];

			if (players <= m_player_count && bytes <= (m_switch_count + 7) / 8)
			{
				if (produce((players * bytes) + 2, response))
				{
					response[0] = ReportCode::Normal;
					response[1] = system_r(0xff);

					uint32_t mask = m_switch_mask & jvs_bitmask<uint32_t>(bytes * 8);
					for (int i = 0; i < players; i++)
					{
						uint32_t val = player_r(i, mask);
						for (int j = 0; j < bytes; j++)
							response[(i * bytes) + j + 2] = val >> (j * 8);
					}
				}
			}
			else if (produce(1, response))
				response[0] = ReportCode::InvalidParameter;
		}
		break;

	case CommandCode::COININP:
		if (consume(1, parameters))
		{
			uint8_t const count = parameters[0];

			if (count <= m_coin_slots)
			{
				if (produce((count * 2) + 1, response))
				{
					response[0] = ReportCode::Normal;

					for (int i = 0; i < count; i++)
					{
						response[(i * 2) + 1] = (m_coin_counter[i] >> 8) & 0x3f;
						response[(i * 2) + 2] = m_coin_counter[i];
					}
				}
			}
			else if (produce(1, response))
				response[0] = ReportCode::InvalidParameter;
		}
		break;

	case CommandCode::ANLINP:
		if (consume(1, parameters))
		{
			uint8_t const count = parameters[0];

			if (count <= m_analog_input_channels)
			{
				if (produce((count * 2) + 1, response))
				{
					uint16_t const analog_input_mask = m_analog_input_bits > 0 && m_analog_input_bits < 16 ? make_bitmask<uint16_t>(m_analog_input_bits) << (16 - m_analog_input_bits) : 0xffff;

					response[0] = ReportCode::Normal;

					for (int i = 0; i < count; i++)
					{
						uint16_t const analog_input = analog_input_r(i, analog_input_mask);
						response[(i * 2) + 1] = analog_input >> 8;
						response[(i * 2) + 2] = analog_input;
					}
				}
			}
			else if (produce(1, response))
				response[0] = ReportCode::InvalidParameter;
		}
		break;

	case CommandCode::ROTINP:
		if (consume(1, parameters))
		{
			uint8_t const count = parameters[0];

			if (count <= m_rotary_input_channels)
			{
				if (produce((count * 2) + 1, response))
				{
					response[0] = ReportCode::Normal;

					for (int i = 0; i < count; i++)
					{
						uint16_t const rotary_input = rotary_input_r(i);
						response[(i * 2) + 1] = rotary_input >> 8;
						response[(i * 2) + 2] = rotary_input;
					}
				}
			}
			else if (produce(1, response))
				response[0] = ReportCode::InvalidParameter;
		}
		break;

	case CommandCode::SCRPOSINP:
		if (consume(1, parameters))
		{
			uint8_t const index = parameters[0] - 1;
			if (index < m_screen_position_input_channels)
			{
				if (produce(5, response))
				{
					uint16_t const enable = screen_position_enable_r(index);
					uint16_t const x = enable ? screen_position_x_r(index) : 0xffff;
					uint16_t const y = enable ? screen_position_y_r(index) : 0xffff;
					response[0] = ReportCode::Normal;
					response[1] = x >> 8;
					response[2] = x;
					response[3] = y >> 8;
					response[4] = y;
				}
			}
			else if (produce(1, response))
				response[0] = ReportCode::InvalidParameter;
		}
		break;

	case CommandCode::RETRANSMIT:
		std::copy_n(m_retransmit_buffer.begin(), m_retransmit_length, m_tx_buffer.begin());
		m_tx_length = m_retransmit_length;
		break;

	case CommandCode::COINDEC:
		if (consume(3, parameters))
		{
			uint8_t const index = parameters[0] - m_coin_offset;
			uint16_t const amount = (parameters[1] << 8) | parameters[2];

			if (index < m_coin_slots)
			{
				if (produce(1, response))
				{
					m_coin_counter[index] = (m_coin_counter[index] < amount) ? 0 : m_coin_counter[index] - amount;
					response[0] = ReportCode::Normal;
				}
			}
			else if (produce(1, response))
				response[0] = ReportCode::InvalidParameter;
		}
		break;

	case CommandCode::OUTPUT1:
		if (consume(1, parameters))
		{
			uint8_t const count = parameters[0];

			if (consume(count, parameters))
			{
				if (count <= (m_output_slots + 7) / 8)
				{
					if (produce(1, response))
					{
						uint64_t output = m_output & ~jvs_bitmask<uint64_t>(count * 8);
						for (int i = 0; i < count; i++)
							output |= (uint64_t)parameters[i] << (i * 8);

						update_output(output);
						response[0] = ReportCode::Normal;
					}
				}
				else if (produce(1, response))
					response[0] = ReportCode::InvalidParameter;
			}
		}
		break;

	case CommandCode::ANLOUT:
		if (consume(1, parameters))
		{
			uint8_t const count = parameters[0];

			if (consume(count * 2, parameters))
			{
				if (count <= m_analog_output_channels)
				{
					if (produce(1, response))
					{
						for (int i = 0; i < count; i++)
							update_analog_output(i, (parameters[i * 2] << 8) | parameters[(i * 2) + 1]);

						response[0] = ReportCode::Normal;
					}
				}
				else if (produce(1, response))
					response[0] = ReportCode::InvalidParameter;
			}
		}
		break;

	case CommandCode::COININC:
		if (consume(3, parameters))
		{
			uint8_t const index = parameters[0] - m_coin_offset;
			uint16_t const amount = (parameters[1] << 8) | parameters[2];

			if (index < m_coin_slots)
			{
				if (produce(1, response))
				{
					m_coin_counter[index] = (m_coin_counter[index] + amount) & 0x3fff;
					response[0] = ReportCode::Normal;
				}
			}
			else if (produce(1, response))
				response[0] = ReportCode::InvalidParameter;
		}
		break;

	case CommandCode::OUTPUT2:
		if (consume(2, parameters))
		{
			uint8_t const index = parameters[0] - 1;
			uint8_t const output = parameters[1];

			if (index < (m_output_slots + 7) / 8)
			{
				if (produce(1, response))
				{
					update_output((m_output & ~(0xffULL << (index * 8))) | ((uint64_t)output << (index * 8)));
					response[0] = ReportCode::Normal;
				}
			}
			else if (produce(1, response))
				response[0] = ReportCode::InvalidParameter;
		}
		break;

	case CommandCode::OUTPUT3:
		if (consume(2, parameters))
		{
			uint8_t const index = parameters[0];
			uint8_t const output = parameters[1];

			if (index < m_output_slots && output <= 2)
			{
				if (produce(1, response))
				{
					if (output == 2 || BIT(m_output, index ^ 7) != output)
						update_output(m_output ^ (1ULL << (index ^ 7)));

					response[0] = ReportCode::Normal;
				}
			}
			else if (produce(1, response))
				response[0] = ReportCode::InvalidParameter;
		}
		break;

	default:
		LOGERROR("unknown JVS command % 02x\n", command);
		status_code(StatusCode::UnknownCommand);
		break;
	}
}

void jvs_hle_device::status_code(uint8_t status_code)
{
	m_tx_buffer[2] = status_code;
}

bool jvs_hle_device::consume(int size, uint8_t const *&parameters)
{
	if (m_rx_index + size < m_rx_length)
	{
		parameters = &m_rx_buffer[m_rx_index];
		m_rx_index += size;
		return true;
	}

	uint8_t *response;
	if (produce(1, response))
		response[0] = ReportCode::IncorrectNumberOfParameters;

	m_rx_index = m_rx_length - 1;
	parameters = nullptr;
	return false;
}

bool jvs_hle_device::produce(int size, uint8_t *&response)
{
	if (m_tx_length + size >= std::size(m_tx_buffer))
	{
		status_code(StatusCode::AcknowledgeOverflow);
		m_tx_length = m_tx_safe_length;
	}

	if (m_tx_buffer[2] == StatusCode::Normal)
	{
		response = &m_tx_buffer[m_tx_length];
		m_tx_length += size;
		return true;
	}

	response = nullptr;
	return false;
}

void jvs_hle_device::packet_received()
{
	m_tx_bit = -1;
	std::fill_n(m_tx_buffer.begin(), m_tx_length, 0);
	m_tx_index = 0;
	m_tx_length = 0;

	if (m_rx_length >= 4 && (m_rx_buffer[0] == 0xff || m_rx_buffer[0] == m_address))
	{
		m_tx_buffer[0] = 0x00; // destination

		uint8_t checksum = std::accumulate(m_rx_buffer.begin(), m_rx_buffer.begin() + m_rx_length - 1, 0);
		if (m_rx_buffer[m_rx_length - 1] != checksum)
		{
			LOGERROR("jvs checksum %02x expected %02x\n", checksum, m_rx_buffer[m_rx_length - 1]);

			if (m_rx_buffer[0] != 0xff)
			{
				status_code(StatusCode::ChecksumError);
				m_tx_length = 3;
			}
		}
		else
		{
			if (VERBOSE & LOG_COMMAND)
			{
				std::string jvsin;
				for (int i = 0; i < m_rx_length; i++)
					jvsin += util::string_format(" %02x", m_rx_buffer[i]);

				LOGCOMMAND("jvs in  %s\n", jvsin);
			}

			status_code(StatusCode::Normal);

			m_rx_index = 2;
			m_tx_length = 3;

			while (m_tx_buffer[2] == StatusCode::Normal && m_rx_index < m_rx_length - 1)
			{
				m_tx_safe_length = m_tx_length;
				execute(m_rx_buffer[m_rx_index++]);
			}

			if (m_tx_length == 3 && m_tx_buffer[2] == StatusCode::Normal)
				m_tx_length = 0;
			else
			{
				std::copy_n(m_tx_buffer.begin(), m_tx_length, m_retransmit_buffer.begin());
				m_retransmit_length = m_tx_length;
			}
		}

		if (m_tx_length)
		{
			m_tx_buffer[1] = m_tx_length - 1;
			uint8_t checksum = std::accumulate(m_tx_buffer.begin(), m_tx_buffer.begin() + m_tx_length, 0);
			m_tx_buffer[m_tx_length++] = checksum;

			if (VERBOSE & LOG_COMMAND)
			{
				std::string jvsout;
				for (int i = 0; i < m_tx_length; i++)
					jvsout += util::string_format(" %02x", m_tx_buffer[i]);

				LOGCOMMAND("jvs out %s\n", jvsout);
			}

			m_tx_timer->adjust(attotime::from_hz(115200) * 10, 0, attotime::from_hz(115200));
		}
	}

	rx_clear();
}

void jvs_hle_device::rxd(int state)
{
	if (m_rxd && !state && m_rx_timer && m_rx_timer->expire().is_never())
		m_rx_timer->adjust(attotime::from_hz(115200) / 2, 0, attotime::from_hz(115200));

	m_rxd = state;
}

TIMER_CALLBACK_MEMBER(jvs_hle_device::rx_timer_callback)
{
	if (m_rx_bit < 0)
	{
		if (m_rxd)
			rx_stop();
		else
			m_rx_bit = 0;
	}
	else if (m_rx_bit < 8)
	{
		if (m_rxd)
			m_rx_buffer[m_rx_length] |= 1 << m_rx_bit;

		m_rx_bit++;
	}
	else
	{
		m_rx_length++;

		if (!m_rxd)
			rx_clear(); // frame error
		else if (m_rx_buffer[m_rx_length - 1] == Sync)
		{
			rx_clear();
			m_rx_state = State::Data;
		}
		else if (m_rx_state == State::Sync)
			rx_discard();
		else if (m_rx_buffer[m_rx_length - 1] == Escape)
		{
			rx_discard();
			m_rx_state = State::EscapedData;
		}
		else
		{
			if (m_rx_state == State::EscapedData)
			{
				m_rx_buffer[m_rx_length - 1]++;
				m_rx_state = State::Data;
			}

			if (m_rx_length >= 2 && m_rx_length == m_rx_buffer[1] + 2)
				packet_received();
			else if (m_rx_length == m_rx_buffer.size())
				rx_clear();
		}

		rx_stop();
	}
}

TIMER_CALLBACK_MEMBER(jvs_hle_device::tx_timer_callback)
{
	if (m_tx_bit < 0)
	{
		if (m_tx_index >= m_tx_length)
		{
			m_tx_timer->adjust(attotime::never);
			m_tx_state = State::Sync;
			rts(0);
			sense(jvs_port_device::sense::Initialized);
		}
		else
		{
			if (m_tx_index == 0)
				rts(1);
			txd(0);
			m_tx_bit = 0;
		}
	}
	else if (m_tx_bit < 8)
	{
		uint8_t data = (m_tx_state == State::Sync) ? Sync :
			(m_tx_state == State::Escape) ? Escape :
			(m_tx_state == State::EscapedData) ? m_tx_buffer[m_tx_index] - 1 :
			m_tx_buffer[m_tx_index];

		txd(BIT(data, m_tx_bit));
		m_tx_bit++;
	}
	else
	{
		txd(1);
		m_tx_bit = -1;

		if (m_tx_state == State::Escape)
			m_tx_state = State::EscapedData;
		else
		{
			if (m_tx_state != State::Sync)
				m_tx_index++;

			if (m_tx_index < m_tx_length && (m_tx_buffer[m_tx_index] == Sync || m_tx_buffer[m_tx_index] == Escape))
				m_tx_state = State::Escape;
			else
				m_tx_state = State::Data;
		}
	}
}

TIMER_CALLBACK_MEMBER(jvs_hle_device::coin_timer_callback)
{
	for (int i = 0; i < m_coin_slots; i++)
	{
		uint8_t coin = coin_r(i, 0x01);
		if (BIT(coin & ~m_coin_prev[i], 0))
			m_coin_counter[i] = (m_coin_counter[i] + 1) & 0x3fff;

		m_coin_prev[i] = coin;
	}
}

void jvs_hle_device::rx_clear()
{
	std::fill_n(m_rx_buffer.begin(), m_rx_length, 0);
	m_rx_length = 0;
	m_rx_state = State::Sync;
}

void jvs_hle_device::rx_discard()
{
	m_rx_buffer[--m_rx_length] = 0;
}

void jvs_hle_device::rx_stop()
{
	m_rx_timer->adjust(attotime::never);
	m_rx_bit = -1;
}

uint32_t jvs_hle_device::switch_mask()
{
	return jvs_bitmask<uint32_t>(switch_count());
}

void jvs_hle_device::update_output(uint64_t output)
{
	output &= m_output_mask;

	if (m_output != output)
	{
		LOGOUTPUT("update_output %0*llx\n", (m_output_slots + 3) / 4, output);
		m_output_cb(0, output, output ^ m_output);
		m_output = output;
	}
}

void jvs_hle_device::update_analog_output(int offset, uint16_t output)
{
	if (m_analog_output[offset] != output)
	{
		LOGOUTPUT("update_fanalog_output %d %04x\n", offset, output);
		m_analog_output_cb[offset](0, output, 0xffff);
		m_analog_output[offset] = output;
	}
}

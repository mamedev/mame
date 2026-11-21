// license:GPL-2.0+
// copyright-holders:Andrea Bogazzi
// Transaction-level UPP SIMIF and GSM T=0 test card from the NEM-4 FPGA study.
// The card serves fixed test files and has no subscription or authentication.

#include "emu.h"
#include "ngage_sim.h"

#include <algorithm>
#include <array>

namespace {

struct test_file
{
	u16 id;
	u8 size;
	std::array<u8, 16> data;
};

constexpr test_file FILES[] =
{
	{ 0x2fe2, 10, { 0x98, 0x10, 0x10, 0x32, 0x54, 0x76, 0x98, 0x10, 0x32, 0xf8 } },
	{ 0x6f05,  1, { 1 } },
	{ 0x6f07,  9, { 8, 9, 0x10, 0x10, 0x10, 0x32, 0x54, 0x76, 0x98 } },
	{ 0x6f20,  9, { 0, 0, 0, 0, 0, 0, 0, 0, 7 } }, // Kc unavailable
	{ 0x6f38, 16, { } }, // No optional services
	{ 0x6f74, 16, { } },
	{ 0x6f78,  2, { 0, 1 } },
	{ 0x6f7b, 12, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } },
	{ 0x6f7e, 11, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 1 } },
	{ 0x6fad,  4, { 0, 0, 0, 2 } },
	{ 0x6fae,  1, { 3 } },
	{ 0x6fb7,  3, { 0x11, 0xf2, 0xff } }
};

test_file const *find_file(u16 id)
{
	auto const match = std::ranges::find(FILES, id, &test_file::id);
	return match != std::end(FILES) ? match : nullptr;
}

} // anonymous namespace

DEFINE_DEVICE_TYPE(NGAGE_SIM, ngage_sim_device, "ngage_sim", "N-Gage SIM interface")

ngage_sim_device::ngage_sim_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NGAGE_SIM, tag, owner, clock)
{
}

void ngage_sim_device::device_start()
{
	save_item(NAME(m_present));
	save_item(NAME(m_started));
	save_item(NAME(m_status));
	save_item(NAME(m_phase));
	save_item(NAME(m_position));
	save_item(NAME(m_reply));
	save_item(NAME(m_length));
	save_item(NAME(m_directory));
	save_item(NAME(m_selected));
	save_item(NAME(m_file_id));
	save_item(NAME(m_header));
	save_item(NAME(m_header_count));
	save_item(NAME(m_select_left));
	save_item(NAME(m_terminal_left));
}

void ngage_sim_device::device_reset()
{
	m_started = false;
	m_status = 0;
	m_phase = ATR;
	m_position = m_length = 0;
	m_directory = m_selected = 0x3f00;
	m_file_id = 0;
	m_header_count = m_select_left = m_terminal_left = 0;
}

void ngage_sim_device::start(u16 command)
{
	if (command == 1 && m_present && !m_started)
	{
		m_started = true;
		m_status = 0x10;
	}
}

void ngage_sim_device::acknowledge(u16 bits)
{
	bool const transmit_complete = (m_status & bits & 8) && m_phase == TX_COMPLETE;
	m_status &= ~bits;
	if (transmit_complete)
	{
		m_phase = RX;
		if (m_position < m_length)
			m_status |= 0x10;
	}
}

u8 ngage_sim_device::read()
{
	if (!m_present)
		return 0;
	bool const peek = machine().side_effects_disabled();
	if (m_phase == ATR)
	{
		u8 const result = m_position ? 0 : 0x3b;
		if (!peek)
			m_position = 1;
		return result;
	}
	if (m_position == m_length)
		return 0;

	u8 const result = m_reply[m_position];
	if (!peek)
	{
		++m_position;
		if (m_position == m_length)
			m_status = 0;
		else if (!(m_position & 1))
			m_status = 0x10;
	}
	return result;
}

void ngage_sim_device::reply_status(u16 status)
{
	m_reply[0] = status >> 8;
	m_reply[1] = status;
	m_length = 2;
}

void ngage_sim_device::command(u8 length)
{
	if (m_header[0] != 0xa0)
	{
		reply_status(0x6e00);
		return;
	}

	u8 const ins = m_header[1];
	if (m_header[2] || m_header[3])
	{
		reply_status(0x6d00);
		return;
	}

	test_file const *const selected = find_file(m_selected);
	if ((ins == 0x10 && length) || (ins == 0xa4 && length == 2))
	{
		if (ins == 0x10)
			m_terminal_left = length;
		else
		{
			m_select_left = 2;
			m_file_id = 0;
		}
		m_reply[0] = ins;
		m_length = 1;
	}
	else if (ins == 0xc0 && length == 15 && selected)
	{
		u8 const response[] = { 0xc0, 0, 0, 0, selected->size, u8(m_selected >> 8),
			u8(m_selected), 4, 0, 0x0f, 0xff, 0xff, 1, 2, 0, 0, 0x90, 0 };
		std::copy(std::begin(response), std::end(response), m_reply);
		m_length = std::size(response);
	}
	else if (ins == 0xf2 && (length == 15 || length == 22))
	{
		u8 const response[] = { 0xf2, 0, 0, 0, 0, u8(m_directory >> 8), u8(m_directory),
			u8(m_directory == 0x3f00 ? 1 : 2), 0, 0, 0, 0, 0, 9, 0xb0, 0, 0, 2,
			0, 0x83, 0x8a, 0x83, 0x8a, 0, 0 };
		std::copy(std::begin(response), std::end(response), m_reply);
		m_length = length + 3;
		m_reply[m_length - 2] = 0x90;
		m_reply[m_length - 1] = 0;
	}
	else if (ins == 0xb0 && selected && length == selected->size)
	{
		m_reply[0] = ins;
		std::copy_n(selected->data.begin(), length, &m_reply[1]);
		m_reply[length + 1] = 0x90;
		m_reply[length + 2] = 0;
		m_length = length + 3;
		logerror("SIM read EF %04x (%u bytes)\n", m_selected, length);
	}
	else
		reply_status(0x6d00);
}

void ngage_sim_device::write(u8 data)
{
	if (!m_started || (m_phase != ATR && m_position != m_length))
		return;

	m_length = m_position = 0;
	if (m_terminal_left)
	{
		if (!--m_terminal_left)
			reply_status(0x9000);
	}
	else if (m_select_left)
	{
		m_file_id = (m_file_id << 8) | data;
		if (!--m_select_left)
		{
			bool const directory = m_file_id == 0x3f00 || m_file_id == 0x7f20;
			bool const valid_file = (m_file_id == 0x2fe2 && m_directory == 0x3f00)
				|| (m_file_id != 0x2fe2 && m_directory == 0x7f20 && find_file(m_file_id));
			if (directory || valid_file)
			{
				m_selected = m_file_id;
				if (directory)
					m_directory = m_file_id;
				reply_status(directory ? 0x9f16 : 0x9f0f);
			}
			else
				reply_status(0x9404);
		}
	}
	else if (m_header_count < std::size(m_header))
		m_header[m_header_count++] = data;
	else
	{
		m_header_count = 0;
		command(data);
	}

	if (m_length)
	{
		m_phase = TX_COMPLETE;
		m_status = 8;
	}
}

// license:GPL-2.0+
// copyright-holders:Andrea Bogazzi
/*
    NEM-4 D550 audio DSP startup link.

    The startup packets and XOR16 framing follow the original C55x startup
    trace and the MiSTer D550 peer. Diagnostic 0x42 models the nominal clock,
    D551 boot signature and N501 codec reset used by the original test. The
    audio processor, codec samples and unrelated application services are
    unimplemented.
*/

#include "emu.h"
#include "ngage_adsp.h"

#include <algorithm>

DEFINE_DEVICE_TYPE(NGAGE_ADSP, ngage_adsp_device, "ngage_adsp", "N-Gage D550 Audio DSP")

ngage_adsp_device::ngage_adsp_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NGAGE_ADSP, tag, owner, clock)
{
}

void ngage_adsp_device::device_start()
{
	save_item(NAME(m_enabled));
	save_item(NAME(m_state));
	save_item(NAME(m_frame));
	save_item(NAME(m_frame_size));
	save_item(NAME(m_output));
	save_item(NAME(m_output_head));
	save_item(NAME(m_output_count));
	save_item(NAME(m_device_sequence));
	save_item(NAME(m_request_sequence));
	save_item(NAME(m_request));
	save_item(NAME(m_request_size));
	save_item(NAME(m_reply));
	save_item(NAME(m_reply_size));
}

void ngage_adsp_device::device_reset()
{
	m_enabled = false;
	reset_link();
}

void ngage_adsp_device::reset_link()
{
	m_state = WAIT_LINK;
	m_frame_size = 0;
	m_output_head = m_output_count = 0;
	m_device_sequence = 0;
	m_request_sequence = 0;
	m_request_size = m_reply_size = 0;
}

void ngage_adsp_device::power_w(bool enabled)
{
	if (enabled == m_enabled)
		return;

	m_enabled = enabled;
	reset_link();
	if (enabled)
		send_startup(false);
}

u8 ngage_adsp_device::rx_r()
{
	if (!rx_pending())
		return 0;

	u8 const data = m_output[m_output_head];
	m_output_head = (m_output_head + 1) % std::size(m_output);
	--m_output_count;
	return data;
}

void ngage_adsp_device::send_frame(u8 resource, std::span<u8 const> payload)
{
	u8 const size = payload.size();
	if (m_output_count + 10 + size + (size & 1) > std::size(m_output))
	{
		logerror("Audio DSP transmit queue full\n");
		return;
	}

	auto const push = [this](u8 byte)
	{
		m_output[(m_output_head + m_output_count++) % std::size(m_output)] = byte;
	};
	push(0x55);
	push(0x55);

	u16 checksum = 0;
	bool high = true;
	auto const append = [&push, &checksum, &high](u8 byte)
	{
		checksum ^= u16(byte) << (high ? 8 : 0);
		high = !high;
		push(byte);
	};
	u8 const header[] = { 0x20, 0, 0x78, resource, 0, size };
	for (u8 const byte : header)
		append(byte);
	for (u8 const byte : payload)
		append(byte);
	if (size & 1)
		append(0);
	push(checksum >> 8);
	push(checksum);
}

void ngage_adsp_device::send_startup(bool reset_only)
{
	static constexpr u8 startup[] = { 0x01, 0x11, 0x01, 0x60 };
	static constexpr u8 link_reset[] = { 0x04, 0x01, 0x60 };
	if (!reset_only)
		send_frame(0xd0, startup);
	send_frame(0xd0, link_reset);
	logerror("Audio DSP sent D0 startup frames\n");
}

void ngage_adsp_device::tx_w(u8 data)
{
	if (!m_enabled)
		return;

	if (m_frame_size < 2 && data != 0x55)
	{
		m_frame_size = 0;
		return;
	}
	m_frame[m_frame_size++] = data;
	if (m_frame_size < 8)
		return;

	unsigned const length = m_frame[7];
	if (length < 2 || length > MAX_PAYLOAD)
	{
		m_frame_size = 0;
		return;
	}
	if (m_frame_size == 10 + length + (length & 1))
	{
		handle_frame();
		m_frame_size = 0;
	}
}

void ngage_adsp_device::handle_frame()
{
	u16 checksum = 0;
	for (unsigned i = 2; i < m_frame_size; i += 2)
		checksum ^= (u16(m_frame[i]) << 8) | m_frame[i + 1];
	if (checksum || m_frame[2] != 0x20 || m_frame[3] != 0x78 || m_frame[4] || m_frame[6])
		return;

	u8 const resource = m_frame[5];
	unsigned const length = m_frame[7];
	u8 const *const data = &m_frame[8];
	u8 const flags = data[length - 1];
	u8 const sequence = flags & 7;

	if (resource == 0xd0 && data[0] == 1)
	{
		reset_link();
		send_startup(true);
		return;
	}
	if (resource == 0xd0 && length == 3 && data[0] == 5 && flags == 0x40)
	{
		if (m_state == WAIT_LINK)
		{
			m_state = READY;
			logerror("Audio DSP accepted D0/05 link confirmation\n");
		}
		return;
	}
	if (resource == 0x7f && length == 2 && data[0] == 0x82 && m_state == WAIT_ACK
			&& sequence == m_device_sequence)
	{
		if (BIT(flags, 7))
			send_frame(0x82, std::span(m_reply).first(m_reply_size));
		else
		{
			m_device_sequence = (m_device_sequence + 1) & 7;
			m_state = READY;
		}
		return;
	}
	if (resource != 0x82 || length < 4 || data[length - 2] != 1 || (flags & 0xc0) != 0x40 || m_state == WAIT_LINK)
		return;

	u8 const ack[] = { 0x82, sequence };
	if (m_request_size && sequence == m_request_sequence)
	{
		if (length != m_request_size || !std::equal(data, data + length, m_request)
				|| m_output_count + 12 + (m_state == WAIT_ACK ? 10 + m_reply_size : 0) > std::size(m_output))
			return;
		send_frame(0x7f, ack);
		if (m_state == WAIT_ACK)
			send_frame(0x82, std::span(m_reply).first(m_reply_size));
		return;
	}
	if (m_state != READY)
		return;

	u8 const operation = data[3];
	bool const diagnostic = length == 12 && operation == 0x2b && data[4] == 0 && data[5] == 7;
	bool const gain = length == 12 && operation == 0x17 && data[4] == 0 && data[5] == 3;
	bool const control = length == 10 && operation == 0x4b && data[4] == 0 && data[5] == 3;
	unsigned const reply_size = diagnostic ? 12 : (gain || control) ? 10 : 0;
	if (m_output_count + 12 + (reply_size ? 10 + reply_size : 0) > std::size(m_output))
		return;

	m_request_sequence = sequence;
	m_request_size = length;
	std::copy_n(data, length, m_request);
	send_frame(0x7f, ack);

	m_reply_size = reply_size;
	if (!reply_size)
		return;

	// Test 0x42 covers the modeled boot prerequisites. Other tests and
	// unimplemented gain/control operations return the original error codes.
	u16 const test = (u16(data[6]) << 8) | data[7];
	u16 const result = diagnostic ? (test == 0x42 ? 0 : 0x0c) : 2;
	m_reply[0] = data[1];
	m_reply[1] = data[0];
	m_reply[2] = data[2];
	m_reply[3] = diagnostic ? 0x8b : gain ? 0x77 : 0xab;
	m_reply[4] = 0;
	m_reply[5] = diagnostic ? 7 : 3;
	if (diagnostic)
	{
		m_reply[6] = data[6];
		m_reply[7] = data[7];
	}
	m_reply[reply_size - 4] = result >> 8;
	m_reply[reply_size - 3] = result;
	m_reply[reply_size - 2] = 1;
	m_reply[reply_size - 1] = 0x60 | m_device_sequence;
	send_frame(0x82, std::span(m_reply).first(reply_size));
	m_state = WAIT_ACK;
	logerror("Audio DSP application %02x returned %02x status %04x\n", operation, m_reply[3], result);
}

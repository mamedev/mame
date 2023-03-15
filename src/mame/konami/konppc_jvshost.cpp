// license:BSD-3-Clause
// copyright-holders:windyfairy
//
// JVS Host implementation shared between multiple PowerPC-based Konami platforms
//

#include "emu.h"

#include "konppc_jvshost.h"

DEFINE_DEVICE_TYPE(KONPPC_JVS_HOST, konppc_jvs_host_device, "konppc_jvs_host", "Konami JVS Host (PowerPC Common)")

konppc_jvs_host_device::konppc_jvs_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: jvs_host(mconfig, KONPPC_JVS_HOST, tag, owner, clock),
	output_cb(*this)
{
}

void konppc_jvs_host_device::device_start()
{
	jvs_host::device_start();

	output_cb.resolve_safe();

	m_jvs_sdata = make_unique_clear<uint8_t[]>(JVS_BUFFER_SIZE);
	m_jvs_is_escape_byte = false;
	m_jvs_sdata_ptr = 0;

	save_pointer(NAME(m_jvs_sdata), JVS_BUFFER_SIZE);
	save_item(NAME(m_jvs_sdata_ptr));
	save_item(NAME(m_jvs_is_escape_byte));
}

void konppc_jvs_host_device::device_reset()
{
	jvs_host::device_reset();

	m_jvs_is_escape_byte = false;
	m_jvs_sdata_ptr = 0;
}

READ_LINE_MEMBER( konppc_jvs_host_device::sense )
{
	return !get_address_set_line();
}

void konppc_jvs_host_device::read()
{
	const uint8_t *data;
	uint32_t length;

	get_encoded_reply(data, length);

	for (int i = 0; i < length; i++)
		output_cb(data[i]);
}

bool konppc_jvs_host_device::write(uint8_t data)
{
	bool is_escape_byte = m_jvs_is_escape_byte;
	m_jvs_is_escape_byte = false;

	// Throw away the buffer and wait for the next sync marker instead of overflowing when
	// a invalid packet is filling the entire buffer.
	if (m_jvs_sdata_ptr >= JVS_BUFFER_SIZE)
		m_jvs_sdata_ptr = 0;

	if (m_jvs_sdata_ptr == 0 && data != 0xe0)
		return false;

	if (m_jvs_sdata_ptr > 0 && data == 0xd0)
	{
		m_jvs_is_escape_byte = true;
		return false;
	}

	m_jvs_sdata[m_jvs_sdata_ptr++] = is_escape_byte ? data + 1 : data;

	const bool is_complete_packet = m_jvs_sdata_ptr >= 5
		&& m_jvs_sdata_ptr == m_jvs_sdata[2] + 3
		&& m_jvs_sdata[0] == 0xe0
		&& m_jvs_sdata[1] != 0x00
		&& m_jvs_sdata[m_jvs_sdata_ptr - 1] == (std::accumulate(&m_jvs_sdata[1], &m_jvs_sdata[m_jvs_sdata_ptr - 1], 0) & 0xff);
	if (is_complete_packet)
	{
		for (int i = 1; i < m_jvs_sdata_ptr - 1; i++)
			push(m_jvs_sdata[i]);
		commit_raw();
		m_jvs_sdata_ptr = 0;
	}

	return is_complete_packet;
}

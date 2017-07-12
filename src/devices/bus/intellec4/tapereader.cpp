// license:BSD-3-Clause
// copyright-holders:Vas Crabb

#include "emu.h"
#include "tapereader.h"


DEFINE_DEVICE_TYPE_NS(INTELLEC4_TAPE_READER, bus::intellec4, tape_reader_device, "intlc4ptr", "INTELLEC 4 paper tape reader")


namespace bus { namespace intellec4 {

tape_reader_device::tape_reader_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, INTELLEC4_TAPE_READER, tag, owner, clock)
	, device_univ_card_interface(mconfig, *this)
	, device_image_interface(mconfig, *this)
	, m_data(0xffU)
	, m_ready(false)
	, m_advance(false)
{
}


image_init_result tape_reader_device::call_load()
{
	m_data = 0x00U;
	m_ready = false;
	return image_init_result::PASS;
}

void tape_reader_device::call_unload()
{
	m_data = 0xffU;
	m_ready = false;
}


void tape_reader_device::device_start()
{
	save_item(NAME(m_data));
	save_item(NAME(m_ready));
	save_item(NAME(m_advance));

	rom_ports_space().install_read_handler(0x0040U, 0x004fU, read8_delegate(FUNC(tape_reader_device::rom4_in), this));
	rom_ports_space().install_read_handler(0x0060U, 0x006fU, read8_delegate(FUNC(tape_reader_device::rom6_in), this));
	rom_ports_space().install_read_handler(0x0070U, 0x007fU, read8_delegate(FUNC(tape_reader_device::rom7_in), this));
	rom_ports_space().install_write_handler(0x0040U, 0x004fU, write8_delegate(FUNC(tape_reader_device::rom4_out), this));
}


DECLARE_WRITE_LINE_MEMBER(tape_reader_device::advance)
{
	// this is edge-sensitive - CPU sends the narrowest pulse it can
	if (!m_advance && !bool(state))
	{
		// FIXME: it probably shouldn't be quite this fast
		if (is_loaded() && fread(&m_data, 1U))
		{
			m_ready = true;
		}
		else
		{
			m_data = 0xffU;
			m_ready = false;
		}
	}
	m_advance = !bool(state);
}

} } // namespace bus::intellec4

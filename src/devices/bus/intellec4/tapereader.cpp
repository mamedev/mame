// license:BSD-3-Clause
// copyright-holders:Vas Crabb

#include "emu.h"
#include "tapereader.h"


DEFINE_DEVICE_TYPE_NS(INTELLEC4_TAPE_READER, bus::intellec4, imm4_90_device, "intlc4_imm4_90", "Intel imm4-90 High-Speed Paper Tape Reader")


namespace bus { namespace intellec4 {

imm4_90_device::imm4_90_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, INTELLEC4_TAPE_READER, tag, owner, clock)
	, device_univ_card_interface(mconfig, *this)
	, device_image_interface(mconfig, *this)
	, m_step_timer(nullptr)
	, m_data(0xffU)
	, m_ready(false)
	, m_advance(false)
	, m_stepping(false)
{
}


image_init_result imm4_90_device::call_load()
{
	m_step_timer->reset();
	m_data = 0x00U;
	m_ready = false;
	m_stepping = false;
	return image_init_result::PASS;
}

void imm4_90_device::call_unload()
{
	m_step_timer->reset();
	m_data = 0xffU;
	m_ready = false;
	m_stepping = false;
}


void imm4_90_device::device_start()
{
	m_step_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(imm4_90_device::step), this));

	save_item(NAME(m_data));
	save_item(NAME(m_ready));
	save_item(NAME(m_advance));
	save_item(NAME(m_stepping));

	rom_ports_space().install_read_handler(0x0040U, 0x004fU, 0x0000U, 0x1f00U, 0x0000, read8_delegate(*this, FUNC(imm4_90_device::rom4_in)));
	rom_ports_space().install_read_handler(0x0060U, 0x006fU, 0x0000U, 0x1f00U, 0x0000, read8_delegate(*this, FUNC(imm4_90_device::rom6_in)));
	rom_ports_space().install_read_handler(0x0070U, 0x007fU, 0x0000U, 0x1f00U, 0x0000, read8_delegate(*this, FUNC(imm4_90_device::rom7_in)));
	rom_ports_space().install_write_handler(0x0040U, 0x004fU, 0x0000U, 0x1f00U, 0x0000, write8_delegate(*this, FUNC(imm4_90_device::rom4_out)));
}


DECLARE_WRITE_LINE_MEMBER(imm4_90_device::advance)
{
	// this is edge-sensitive - CPU sends the narrowest pulse it can
	if (!m_advance && !bool(state) && !m_stepping)
	{
		m_ready = false;
		m_stepping = true;
		m_step_timer->adjust(attotime::from_msec(5)); // 200 characters/second
	}
	m_advance = !bool(state);
}

TIMER_CALLBACK_MEMBER(imm4_90_device::step)
{
	m_stepping = false;
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

} } // namespace bus::intellec4

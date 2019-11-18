// license:BSD-3-Clause
// copyright-holders:Vas Crabb

#include "emu.h"
#include "prommemory.h"


DEFINE_DEVICE_TYPE_NS(INTELLEC4_PROM_MEMORY, bus::intellec4, imm6_26_device, "intlc4_imm6_26", "Intel imm6-26 PROM Memory Module")


namespace bus { namespace intellec4 {

imm6_26_device::imm6_26_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, INTELLEC4_PROM_MEMORY, tag, owner, clock)
	, device_univ_card_interface(mconfig, *this)
	, device_image_interface(mconfig, *this)
	, m_data()
{
}


image_init_result imm6_26_device::call_load()
{
	if ((length() > 4096U) || (length() % 256U))
		return image_init_result::FAIL;

	unmap();
	allocate();
	if (fread(m_data.get(), length()) != length())
		return image_init_result::FAIL;

	// FIXME: gimme a cookie!
	rom_space().install_rom(0x1000U, offs_t(0x1000U + length()), m_data.get());

	return image_init_result::PASS;
}

void imm6_26_device::call_unload()
{
	unmap();
}


void imm6_26_device::device_start()
{
	allocate();

	save_pointer(NAME(m_data), 4096U);
}


void imm6_26_device::allocate()
{
	if (!m_data)
	{
		m_data = std::make_unique<u8 []>(4096U);
		std::fill(m_data.get(), m_data.get() + 4096U, 0U);
	}
}

void imm6_26_device::unmap()
{
	// FIXME: where's my magic cookie?
	rom_space().unmap_read(0x1000U, 0x1fffU);
}

} } // namespace bus::intellec4

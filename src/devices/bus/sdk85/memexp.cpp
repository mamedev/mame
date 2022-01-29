// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    SDK-85 memory expansion (ALE-multiplexed)

***************************************************************************/

#include "emu.h"
#include "memexp.h"

#include "i8755.h"

// device type definition
DEFINE_DEVICE_TYPE(SDK85_ROMEXP, sdk85_romexp_device, "sdk85_romexp", "SDK-85 expansion ROM socket")

sdk85_romexp_device::sdk85_romexp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SDK85_ROMEXP, tag, owner, clock)
	, device_rom_image_interface(mconfig, *this)
	, device_single_card_slot_interface<device_sdk85_romexp_card_interface>(mconfig, *this)
	, m_dev(nullptr)
{
}

image_init_result sdk85_romexp_device::call_load()
{
	if (get_card_device() != nullptr)
	{
		u32 size = loaded_through_softlist() ? get_software_region_length("rom") : length();
		u8 *base = get_card_device()->get_rom_base(size);
		if (base == nullptr)
			return image_init_result::FAIL;

		if (loaded_through_softlist())
			memcpy(base, get_software_region("rom"), size);
		else
			fread(base, size);
	}

	return image_init_result::PASS;
}

std::string sdk85_romexp_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("i8755");
}

void sdk85_romexp_device::device_start()
{
	m_dev = get_card_device();
}

u8 sdk85_romexp_device::memory_r(offs_t offset)
{
	if (m_dev != nullptr)
		return m_dev->read_memory(offset);
	else
		return 0xff;
}

void sdk85_romexp_device::memory_w(offs_t offset, u8 data)
{
	if (m_dev != nullptr)
		m_dev->write_memory(offset, data);
}

u8 sdk85_romexp_device::io_r(offs_t offset)
{
	if (m_dev != nullptr)
		return m_dev->read_io(offset);
	else
		return 0xff;
}

void sdk85_romexp_device::io_w(offs_t offset, u8 data)
{
	if (m_dev != nullptr)
		m_dev->write_io(offset, data);
}

void sdk85_romexp_device::rom_options(device_slot_interface &slot)
{
	slot.option_add_internal("i8755", SDK85_I8755);
}

device_sdk85_romexp_card_interface::device_sdk85_romexp_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "sdk85_romexp")
{
}

u8 *device_sdk85_romexp_card_interface::get_rom_base(u32 size)
{
	return nullptr;
}

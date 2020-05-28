// license:BSD-3-Clause
// copyright-holders:AJR

#include "emu.h"
#include "i8755.h"

// device type definition
DEFINE_DEVICE_TYPE(SDK85_I8755, sdk85exp_i8755_device, "sdk85exp_i8755", "SDK-85 PROM I/O Expansion (Intel 8755)")

sdk85exp_i8755_device::sdk85exp_i8755_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SDK85_I8755, tag, owner, clock)
	, device_sdk85_romexp_card_interface(mconfig, *this)
	, m_i8755(*this, "i8755")
{
}

void sdk85exp_i8755_device::device_start()
{
}

u8 sdk85exp_i8755_device::read_memory(offs_t offset)
{
	return m_i8755->memory_r(offset);
}

void sdk85exp_i8755_device::write_memory(offs_t offset, u8 data)
{
	logerror("Writing %02Xh to base + %03Xh\n", data, offset);
	m_i8755->io_w(offset & 3, data);
}

u8 sdk85exp_i8755_device::read_io(offs_t offset)
{
	return m_i8755->io_r(offset & 3);
}

void sdk85exp_i8755_device::write_io(offs_t offset, u8 data)
{
	m_i8755->io_w(offset & 3, data);
}

void sdk85exp_i8755_device::device_add_mconfig(machine_config &config)
{
	I8355(config, m_i8755, DERIVED_CLOCK(1, 1));
}

u8 *sdk85exp_i8755_device::get_rom_base(u32 size)
{
	if (size != 0x800)
	{
		osd_printf_error("sdk85exp_i8755: Data length must be 2,048 bytes\n");
		return nullptr;
	}

	return memregion("i8755")->base();
}

ROM_START(sdk85exp_i8755)
	ROM_REGION(0x800, "i8755", ROMREGION_ERASE00)
ROM_END

const tiny_rom_entry *sdk85exp_i8755_device::device_rom_region() const
{
	return ROM_NAME(sdk85exp_i8755);
}

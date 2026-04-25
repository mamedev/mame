// license:BSD-3-Clause
// copyright-holders:vklachkov

#include "gridrom.h"

#include <inttypes.h>

#include "bus/generic/rom.h"

DEFINE_DEVICE_TYPE(GRIDROM_SOCKET, gridrom_socket_device, "gridrom_socket", "GRiD's ROM Socket")

device_slot_interface &gridrom_slot(device_slot_interface &device)
{
	device.option_add_internal("rom", GENERIC_ROM_PLAIN);
	return device;
}

gridrom_socket_device::gridrom_socket_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	generic_slot_device(mconfig, GRIDROM_SOCKET, tag, owner, clock)
{
}

gridrom_socket_device::~gridrom_socket_device()
{
}

std::pair<std::error_condition, std::string> gridrom_socket_device::call_load()
{
	const uint32_t size = loaded_through_softlist() ? get_software_region_length("rom") : length();
	const std::vector<uint32_t> &sizes = m_acceptable_sizes;

	if (std::find(sizes.begin(), sizes.end(), size) == sizes.end())
	{
		std::string sizes_str;
		for (size_t i = 0; i < sizes.size(); ++i)
		{
			if (i != 0) sizes_str += ", ";
			sizes_str += util::string_format("%" PRIu32, sizes[i]);
		}

		return std::make_pair(
				image_error::INVALIDLENGTH,
				util::string_format("Invalid ROM size: expected one of [%s] bytes, got %" PRIu32, sizes_str.c_str(), size));
	}

	return generic_slot_device::call_load();
}

uint8_t gridrom_socket_device::read(offs_t offset)
{
	return this->read_rom(offset);
}

void gridrom_socket_device::set_image_names(std::string type_name, std::string brief_type_name)
{
	m_image_type_name = type_name;
	m_image_brief_type_name = brief_type_name;
}

void gridrom_socket_device::set_acceptable_sizes(std::vector<uint32_t> values)
{
	m_acceptable_sizes = values;
}

// license:BSD-3-Clause
// copyright-holders:vklachkov

#ifndef MAME_GRIDCOMP_GRIDROM_H
#define MAME_GRIDCOMP_GRIDROM_H

#pragma once

#include "emu.h"
#include "bus/generic/slot.h"

device_slot_interface &gridrom_slot(device_slot_interface &device);

class gridrom_socket_device : public generic_slot_device
{
public:
    template <typename T>
	gridrom_socket_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *intf, char const *exts = nullptr)
		: gridrom_socket_device(mconfig, tag, owner, u32(0))
	{
		opts(*this);
		set_fixed(false);
		set_interface(intf);
		if (exts)
			set_extensions(exts);
	}

	gridrom_socket_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);
	virtual ~gridrom_socket_device();

	std::pair<std::error_condition, std::string> call_load() override;
	
    uint8_t read(offs_t offset);

    void set_image_names(std::string type_name, std::string brief_type_name);
	void set_acceptable_sizes(std::vector<uint32_t> values);

    virtual const char *image_type_name() const noexcept override { return m_image_type_name.c_str(); }
	virtual const char *image_brief_type_name() const noexcept override { return m_image_brief_type_name.c_str(); }

private:
	std::string m_image_type_name = "imagerom";
	std::string m_image_brief_type_name = "rom";
	std::vector<uint32_t> m_acceptable_sizes = {32*1024, 64*1024, 128*1024};
};

DECLARE_DEVICE_TYPE(GRIDROM_SOCKET, gridrom_socket_device)

#endif

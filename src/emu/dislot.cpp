// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Slot device

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "zippath.h"


// -------------------------------------------------
// ctor
// -------------------------------------------------

device_slot_interface::device_slot_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "slot"),
	m_default_option(nullptr),
	m_fixed(false),
	m_card_device(nullptr)
{
}


// -------------------------------------------------
// dtor
// -------------------------------------------------

device_slot_interface::~device_slot_interface()
{
}


// -------------------------------------------------
// device_slot_option ctor
// -------------------------------------------------

device_slot_option::device_slot_option(const char *name, const device_type &devtype):
	m_name(name),
	m_devtype(devtype),
	m_selectable(true),
	m_default_bios(nullptr),
	m_machine_config(nullptr),
	m_input_device_defaults(nullptr),
	m_clock(0)
{
}


// -------------------------------------------------
// static_option_reset
// -------------------------------------------------

void device_slot_interface::static_option_reset(device_t &device)
{
	device_slot_interface &intf = dynamic_cast<device_slot_interface &>(device);

	intf.m_options.clear();
}


// -------------------------------------------------
// static_option_add
// -------------------------------------------------

void device_slot_interface::static_option_add(device_t &device, const char *name, const device_type &devtype)
{
	device_slot_interface &intf = dynamic_cast<device_slot_interface &>(device);
	device_slot_option *option = intf.option(name);

	if (option != nullptr)
		throw emu_fatalerror("slot '%s' duplicate option '%s'\n", device.tag(), name);
	if (intf.m_options.count(name) != 0) throw tag_add_exception(name);
	intf.m_options.emplace(std::make_pair(name, std::make_unique<device_slot_option>(name, devtype)));
}


// -------------------------------------------------
// static_option
// -------------------------------------------------

device_slot_option *device_slot_interface::static_option(device_t &device, const char *name)
{
	device_slot_interface &intf = dynamic_cast<device_slot_interface &>(device);
	device_slot_option *option = intf.option(name);

	if (option == nullptr)
		throw emu_fatalerror("slot '%s' has no option '%s'\n", device.tag(), name);

	return option;
}


// -------------------------------------------------
// has_selectable_options
// -------------------------------------------------

bool device_slot_interface::has_selectable_options() const
{
	if (!fixed())
	{
		for (auto &option : option_list())
			if (option.second->selectable())
				return true;
	}
	return false;
}


// -------------------------------------------------
// option
// -------------------------------------------------

device_slot_option *device_slot_interface::option(const char *name) const
{
	device_slot_option *result = nullptr;
	if (name)
	{
		auto search = m_options.find(name);
		if (search != m_options.end())
			result = search->second.get();
	}
	return result;
}


device_slot_card_interface::device_slot_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "slot")
{
}

device_slot_card_interface::~device_slot_card_interface()
{
}

get_default_card_software_hook::get_default_card_software_hook(const std::string &path, std::function<bool(util::core_file &, std::string&)> &&get_hashfile_extrainfo)
	: m_get_hashfile_extrainfo(std::move(get_hashfile_extrainfo))
	, m_called_get_hashfile_extrainfo(false)
	, m_has_hash_extrainfo(false)
{
	if (!path.empty())
	{
		std::string revised_path;
		util::zippath_fopen(path, OPEN_FLAG_READ, m_image_file, revised_path);
		if (m_image_file)
			m_file_type = core_filename_extract_extension(revised_path, true);
	}
}

bool get_default_card_software_hook::hashfile_extrainfo(std::string &extrainfo)
{
	if (!m_called_get_hashfile_extrainfo)
	{
		if (m_get_hashfile_extrainfo)
			m_has_hash_extrainfo = m_get_hashfile_extrainfo(*image_file(), m_hash_extrainfo);
		m_called_get_hashfile_extrainfo = true;
	}
	extrainfo = m_hash_extrainfo;
	return m_has_hash_extrainfo;
}

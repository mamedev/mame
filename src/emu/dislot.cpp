// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Slot device

***************************************************************************/

#include "emu.h"
#include "emuopts.h"

#include "corestr.h"
#include "path.h"
#include "zippath.h"



//**************************************************************************
//  device_slot_interface::slot_option
//**************************************************************************

device_slot_interface::slot_option::slot_option(std::string_view name, device_type devtype, bool selectable) :
	m_name(name),
	m_devtype(devtype),
	m_selectable(selectable),
	m_default_bios(nullptr),
	m_machine_config(nullptr),
	m_input_device_defaults(nullptr),
	m_clock(0)
{
}

device_slot_interface::slot_option &device_slot_interface::slot_option::clock(XTAL clock)
{
	clock.validate(util::string_format("Configuring slot option %s", m_name));
	m_clock = clock.value();
	return *this;
}



//**************************************************************************
//  device_slot_interface
//**************************************************************************

device_slot_interface::device_slot_interface(machine_config const &mconfig, device_t &device) :
	device_interface(device, "slot"),
	m_default_clock(DERIVED_CLOCK(1, 1)),
	m_default_option(nullptr),
	m_fixed(false),
	m_card_device(nullptr)
{
}

device_slot_interface::~device_slot_interface()
{
}


void device_slot_interface::interface_validity_check(validity_checker &valid) const
{
	// prohibit tags for driver-level slot devices from clashing with image instance names to prevent errors when adding options
	if (device().owner() != nullptr && device().owner()->owner() == nullptr)
	{
		device_image_interface const *image;
		if (device().interface(image) && (device().basetag() == image->instance_name() || device().basetag() == image->brief_instance_name()))
			osd_printf_error("Slot device tag conflicts with image instance name\n");
	}

	if (m_default_option && (m_options.find(m_default_option) == m_options.end()))
		osd_printf_error("Default option '%s' does not correspond to any configured option\n", m_default_option);
}


device_slot_interface::slot_option &device_slot_interface::do_add_option(std::string_view name, device_type devtype, bool selectable)
{
	if (name.empty())
		throw emu_fatalerror("slot '%s' attempt to add option without name\n", device().tag());

	slot_option const *const existing = option(name);
	if (existing)
		throw emu_fatalerror("slot '%s' duplicate option '%s'\n", device().tag(), name);

	auto const ins = m_options.emplace(name, slot_option_ptr());
	assert(ins.second);
	ins.first->second = std::make_unique<slot_option>(ins.first->first, devtype, selectable);
	return ins.first->second->clock(m_default_clock);
}


device_slot_interface::slot_option &device_slot_interface::do_replace_option(std::string_view name, device_type devtype, bool selectable)
{
	if (name.empty())
		throw emu_fatalerror("slot '%s' attempt to replace option without name\n", device().tag());

	auto const found = m_options.find(name);
	if (found == m_options.end())
		throw emu_fatalerror("slot '%s' attempt to replace nonexistent option '%s'\n", device().tag(), name);

	found->second = std::make_unique<slot_option>(found->first, devtype, selectable);
	return found->second->clock(m_default_clock);
}


void device_slot_interface::option_remove(std::string_view name)
{
	if (name.empty())
		throw emu_fatalerror("slot '%s' attempt to remove option without name\n", device().tag());

	auto const it = m_options.find(name);
	if (m_options.end() == it)
		throw emu_fatalerror("slot '%s' attempt to remove nonexistent option '%s'\n", device().tag(), name);

	m_options.erase(it);
}


device_slot_interface::slot_option &device_slot_interface::config_option(std::string_view name)
{
	auto const found = m_options.find(name);
	if (found != m_options.end())
		return *found->second;

	throw emu_fatalerror("slot '%s' has no option '%s'\n", device().tag(), name);
}


bool device_slot_interface::has_selectable_options() const
{
	if (!fixed())
	{
		for (auto &option : option_list())
		{
			if (option.second->selectable())
				return true;
		}
	}
	return false;
}


device_slot_interface::slot_option const *device_slot_interface::option(std::string_view name) const
{
	auto const found = m_options.find(name);
	if (found != m_options.end())
		return found->second.get();

	return nullptr;
}



//**************************************************************************
//  get_default_card_software_hook
//**************************************************************************

get_default_card_software_hook::get_default_card_software_hook(std::string const &path, get_hashfile_extrainfo_func &&get_hashfile_extrainfo)
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

bool get_default_card_software_hook::is_filetype(std::string_view candidate_filetype) const
{
	return util::streqlower(m_file_type, candidate_filetype);
}

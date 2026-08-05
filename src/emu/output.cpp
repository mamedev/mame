// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Vas Crabb
/***************************************************************************

    output.cpp

    General purpose output routines.

***************************************************************************/

#include "emu.h"
#include "output.h"

#include <algorithm>


#define OUTPUT_VERBOSE 0



//**************************************************************************
//  OUTPUT ITEM
//**************************************************************************

output_manager::item_impl::item_impl(
		output_manager &manager,
		device_t &device,
		std::string_view name)
	: m_manager(manager)
	, m_device(device)
	, m_data(name, device.tag())
	, m_notifylist()
{
}


void output_manager::item_impl::notify(s32 value) const
{
	if (OUTPUT_VERBOSE)
		m_manager.machine().logerror("Output %s = %d (was %d)\n", qualified_name(), value, get());
	m_data.set(value);

	auto const now = m_manager.machine().scheduler().time();

	// call the local notifiers first
	for (auto const &notify : m_notifylist)
		notify(now, m_data);

	// call the global notifiers next
	for (auto const &notify : m_manager.m_global_notifylist)
		notify(now, m_data);
}



//**************************************************************************
//  OUTPUT ITEM CREATOR PROXY
//**************************************************************************

bool output_manager::item_creator_proxy::resolve(device_t &device, std::string_view name)
{
	assert(!m_item);
	m_item = &device.machine().output().find_or_create_item(device, name);
	return bool(m_item);
}



//**************************************************************************
//  OUTPUT PROXY
//**************************************************************************

output_manager::output_proxy::output_proxy() noexcept
	: m_item(nullptr)
	, m_value(m_local_value)
	, m_local_value(0)
{
}

output_manager::output_proxy::output_proxy(output_proxy &&that) noexcept
	: output_proxy()
{
	operator=(std::move(that));
}

output_manager::output_proxy::output_proxy(device_t &device, std::string_view name)
	: output_proxy()
{
	m_item = device.machine().output().find_item(device, name);
	if (m_item)
		m_value = m_item->get();
}

output_manager::output_proxy &output_manager::output_proxy::operator=(output_proxy &&that) noexcept
{
	m_item = std::exchange(that.m_item, nullptr);
	m_local_value = std::exchange(that.m_local_value, that.m_value);
	that.m_value = that.m_local_value;
	m_value = m_item ? m_item->get() : m_local_value;
	return *this;
}




//**************************************************************************
//  OUTPUT MANAGER
//**************************************************************************

/*-------------------------------------------------
    output_manager - constructor
-------------------------------------------------*/

output_manager::output_manager(running_machine &machine)
	: m_machine(machine)
{
	// add callbacks
	machine.save().register_presave(save_prepost_delegate(FUNC(output_manager::presave), this));
	machine.save().register_postload(save_prepost_delegate(FUNC(output_manager::postload), this));
}


/*-------------------------------------------------
    register_save - register for save states
-------------------------------------------------*/

void output_manager::register_save()
{
	assert(m_save_order.empty());
	assert(!m_save_data);

	// make space for the data
	m_save_order.clear();
	m_save_order.reserve(m_itemtable.size());
	m_save_data = std::make_unique<s32 []>(m_itemtable.size());

	// sort existing outputs by name and register for save
	for (auto &item : m_itemtable)
		m_save_order.emplace_back(item);
	std::sort(
			m_save_order.begin(),
			m_save_order.end(),
			[] (item_impl const &l, item_impl const &r)
			{
				// assume no conflicting device tags
				if (&l.device() != &r.device())
					return l.device_tag() < r.device_tag();
				else
					return l.name() < r.name();
			});

	// register the reserved space for saving
	std::size_t first = 0;
	std::size_t last = 0;
	while (m_save_order.size() != first)
	{
		if ((last != first) && ((m_save_order.size() == last) || (&m_save_order[first].get().device() != &m_save_order[last].get().device())))
		{
			device_t &device = m_save_order[first].get().device();
			machine().save().save_pointer(&device, "output", device.tag(), 0, &m_save_data[first], "save_data", last - first);
			first = last;
		}
		else
		{
			++last;
		}
	}
	if (OUTPUT_VERBOSE)
		osd_printf_verbose("Registered %u outputs for save states\n", m_itemtable.size());

}


/*-------------------------------------------------
    validate_name - check for illegal names
-------------------------------------------------*/

bool output_manager::validate_name(device_t &device, std::string_view name)
{
	if (name.empty())
	{
		osd_printf_error("Output names must not be empty\n");
		return false;
	}
	else if ((name[0] == ':') || (name[0] == '^'))
	{
		osd_printf_error("Output names must not begin with a parent device or root device reference\n");
		return false;
	}
	else if ((name[0] == '.') && ((name.size() == 1U) || (name[1] == ':')))
	{
		osd_printf_error("Output names must not begin with a current device reference\n");
		return false;
	}
	else if ((name.front() == ' ') || (name.back() == ' ' ))
	{
		osd_printf_error("Output names must not begin or end with whitespace\n");
		return false;
	}
	else if (std::find_if(name.begin(), name.end(), [] (char ch) { return (' ' > ch) || ('~' < ch); }) != name.end())
	{
		osd_printf_error("Output name contains invalid characters\n");
		return false;
	}
	else
	{
		return true;
	}
}


/*-------------------------------------------------
    find_item - find an item based on a string
-------------------------------------------------*/

output_manager::item_impl const *output_manager::find_item(device_t &device, std::string_view name)
{
	// first look for an output of the device itself
	auto const exact = m_itemtable.find(std::make_pair(std::ref(device), name));
	if (m_itemtable.end() != exact)
		return &*exact;

	// try treating the string as a relative tag
	auto const qualified = m_qualified.find(std::string_view(device.subtag(name)).substr(1));
	if (m_qualified.end() != qualified)
		return &qualified->get();

	// for backwards compatibility, look up unqualified names for the root device
	// TODO: phase out support for this
	if (!device.owner())
	{
		auto const unqualified = m_unqualified.find(name);
		if (m_unqualified.end() != unqualified)
			return &unqualified->get();
	}

	return nullptr;
}


/*-------------------------------------------------
    find_or_create_item - set up item
-------------------------------------------------*/

output_manager::item_impl const &output_manager::find_or_create_item(device_t &device, std::string_view name)
{
	assert(!m_save_data);

	// only use a perfect match, don't use find_item
	auto const found = m_itemtable.find(std::make_pair(std::ref(device), name));
	if (m_itemtable.end() != found)
		return *found;

	if (!validate_name(device, name))
		throw emu_fatalerror("Invalid output name,");

	if (OUTPUT_VERBOSE)
		osd_printf_verbose("Creating output %s:%s\n", device.tag() + 1, name);

	auto const ins(m_itemtable.emplace(*this, device, name));
	assert(ins.second);
	auto const qualified(m_qualified.emplace(*ins.first));
	if (!qualified.second)
	{
		osd_printf_warning(
				"Warning: qualified output names conflict for %s %s and %s %s\n",
				ins.first->device_tag(), ins.first->name(),
				qualified.first->get().device_tag(), qualified.first->get().name());
	}
	m_unqualified.emplace(*ins.first);

	return *ins.first;
}


/*-------------------------------------------------
    presave - prepare data for save state
-------------------------------------------------*/

void output_manager::presave()
{
	for (size_t i = 0; m_save_order.size() > i; ++i)
		m_save_data[i] = m_save_order[i].get().get();
}


/*-------------------------------------------------
    postload - restore loaded data
-------------------------------------------------*/

void output_manager::postload()
{
	for (size_t i = 0; m_save_order.size() > i; ++i)
		 m_save_order[i].get().set(m_save_data[i]);
}


//-------------------------------------------------
//  set_notifier - sets a notifier callback for a
//  particular output
//-------------------------------------------------

void output_manager::add_notifier(device_t &device, std::string_view name, notifier_func callback, void *param)
{
	// only use a perfect match, don't use find_item
	auto const found = m_itemtable.find(std::make_pair(std::ref(device), name));
	if (m_itemtable.end() == found)
		throw emu_fatalerror("Attempt to add notifier to non-existent output %s:%s", device.tag() + 1, name);

	found->add_notifier(callback, param);
}


//-------------------------------------------------
//  set_global_notifier - sets a notifier callback
//  for all outputs
//-------------------------------------------------

void output_manager::add_global_notifier(notifier_func callback, void *param)
{
	m_global_notifylist.emplace_back(callback, param);
}

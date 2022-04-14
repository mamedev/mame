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

output_manager::output_item::output_item(
		output_manager &manager,
		std::string &&name,
		u32 id,
		s32 value)
	: m_manager(manager)
	, m_name(std::move(name))
	, m_id(id)
	, m_value(value)
	, m_notifylist()
{
}


void output_manager::output_item::notify(s32 value)
{
	if (OUTPUT_VERBOSE)
		m_manager.machine().logerror("Output %s = %d (was %d)\n", m_name, value, m_value);
	m_value = value;

	// call the local notifiers first
	for (auto const &notify : m_notifylist)
		notify(m_name.c_str(), value);

	// call the global notifiers next
	for (auto const &notify : m_manager.m_global_notifylist)
		notify(m_name.c_str(), value);
}



//**************************************************************************
//  OUTPUT ITEM PROXY
//**************************************************************************

void output_manager::item_proxy::resolve(device_t &device, std::string_view name)
{
	assert(!m_item);
	m_item = &device.machine().output().find_or_create_item(name, 0);
}



//**************************************************************************
//  OUTPUT MANAGER
//**************************************************************************

/*-------------------------------------------------
    output_manager - constructor
-------------------------------------------------*/

output_manager::output_manager(running_machine &machine)
	: m_machine(machine)
	, m_uniqueid(12345)
{
	// add callbacks
	machine.add_notifier(MACHINE_NOTIFY_PAUSE, machine_notify_delegate(&output_manager::pause, this));
	machine.add_notifier(MACHINE_NOTIFY_RESUME, machine_notify_delegate(&output_manager::resume, this));
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
		m_save_order.emplace_back(item.second);
	std::sort(m_save_order.begin(), m_save_order.end(), [] (auto const &l, auto const &r) { return l.get().name() < r.get().name(); });

	// register the reserved space for saving
	machine().save().save_pointer(nullptr, "output", nullptr, 0, NAME(m_save_data), m_itemtable.size());
	if (OUTPUT_VERBOSE)
		osd_printf_verbose("Registered %u outputs for save states\n", m_itemtable.size());

}


/*-------------------------------------------------
    find_item - find an item based on a string
-------------------------------------------------*/

output_manager::output_item *output_manager::find_item(std::string_view string)
{
	auto item = m_itemtable.find(std::string(string));
	if (item != m_itemtable.end())
		return &item->second;

	return nullptr;
}


/*-------------------------------------------------
    create_new_item - create a new item
-------------------------------------------------*/

output_manager::output_item &output_manager::create_new_item(std::string_view outname, s32 value)
{
	if (OUTPUT_VERBOSE)
		osd_printf_verbose("Creating output %s = %d%s\n", outname, value, m_save_data ? " (will not be saved)" : "");

	auto const ins(m_itemtable.emplace(
			std::piecewise_construct,
			std::forward_as_tuple(outname),
			std::forward_as_tuple(*this, std::string(outname), m_uniqueid++, value)));
	assert(ins.second);
	return ins.first->second;
}

output_manager::output_item &output_manager::find_or_create_item(std::string_view outname, s32 value)
{
	output_item *const item = find_item(outname);
	return item ? *item : create_new_item(outname, value);
}


/*-------------------------------------------------
    output_pause - send pause message
-------------------------------------------------*/

void output_manager::pause()
{
	set_value("pause", 1);
}

void output_manager::resume()
{
	set_value("pause", 0);
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


/*-------------------------------------------------
    output_set_value - set the value of an output
-------------------------------------------------*/

void output_manager::set_value(std::string_view outname, s32 value)
{
	output_item *const item = find_item(outname);

	// if no item of that name, create a new one and force notification
	if (!item)
		create_new_item(outname, value).notify(value);
	else
		item->set(value); // set the new value (notifies on change)
}


/*-------------------------------------------------
    output_get_value - return the value of an
    output
-------------------------------------------------*/

s32 output_manager::get_value(std::string_view outname)
{
	output_item const *const item = find_item(outname);

	// if no item, value is 0
	return item ? item->get() : 0;
}


//-------------------------------------------------
//  set_notifier - sets a notifier callback for a
//  particular output
//-------------------------------------------------

void output_manager::set_notifier(std::string_view outname, notifier_func callback, void *param)
{
	// if an item is specified, find/create it
	output_item *const item = find_item(outname);
	(item ? *item : create_new_item(outname, 0)).set_notifier(callback, param);
}


//-------------------------------------------------
//  set_global_notifier - sets a notifier callback
//  for all outputs
//-------------------------------------------------

void output_manager::set_global_notifier(notifier_func callback, void *param)
{
	m_global_notifylist.emplace_back(callback, param);
}


/*-------------------------------------------------
    output_name_to_id - returns a unique ID for
    a given name
-------------------------------------------------*/

u32 output_manager::name_to_id(std::string_view outname)
{
	// if no item, ID is 0
	output_item const *const item = find_item(outname);
	return item ? item->id() : 0;
}


/*-------------------------------------------------
    output_id_to_name - returns a name that maps
    to a given unique ID
-------------------------------------------------*/

char const *output_manager::id_to_name(u32 id)
{
	for (auto &item : m_itemtable)
		if (item.second.id() == id)
			return item.second.name().c_str();

	// nothing found, return nullptr
	return nullptr;
}

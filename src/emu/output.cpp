// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Vas Crabb
/***************************************************************************

    output.c

    General purpose output routines.
***************************************************************************/

#include "emu.h"
#include "coreutil.h"
#include "modules/output/output_module.h"


#define OUTPUT_VERBOSE  0



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

void output_manager::item_proxy::resolve(device_t &device, std::string const &name)
{
	assert(!m_item);
	m_item = &device.machine().output().find_or_create_item(name.c_str(), 0);
}



//**************************************************************************
//  OUTPUT MANAGER
//**************************************************************************

//-------------------------------------------------
//  output_manager - constructor
//-------------------------------------------------

output_manager::output_manager(running_machine &machine)
	: m_machine(machine),
		m_uniqueid(12345)
{
	/* add pause callback */
	machine.add_notifier(MACHINE_NOTIFY_PAUSE, machine_notify_delegate(&output_manager::pause, this));
	machine.add_notifier(MACHINE_NOTIFY_RESUME, machine_notify_delegate(&output_manager::resume, this));
}

/*-------------------------------------------------
    find_item - find an item based on a string
-------------------------------------------------*/

output_manager::output_item* output_manager::find_item(const char *string)
{
	auto item = m_itemtable.find(std::string(string));
	if (item != m_itemtable.end())
		return &item->second;

	return nullptr;
}


/*-------------------------------------------------
    create_new_item - create a new item
-------------------------------------------------*/

output_manager::output_item &output_manager::create_new_item(const char *outname, s32 value)
{
	auto const ins(m_itemtable.emplace(
			std::piecewise_construct,
			std::forward_as_tuple(outname),
			std::forward_as_tuple(*this, outname, m_uniqueid++, value)));
	assert(ins.second);
	return ins.first->second;
}

output_manager::output_item &output_manager::find_or_create_item(const char *outname, s32 value)
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
    output_set_value - set the value of an output
-------------------------------------------------*/

void output_manager::set_value(const char *outname, s32 value)
{
	output_item *const item = find_item(outname);

	// if no item of that name, create a new one and force notification
	if (!item)
		create_new_item(outname, value).notify(value);
	else
		item->set(value); // set the new value (notifies on change)
}


/*-------------------------------------------------
    output_set_indexed_value - set the value of an
    indexed output
-------------------------------------------------*/

void output_manager::set_indexed_value(const char *basename, int index, int value)
{
	char buffer[100];
	char *dest = buffer;

	/* copy the string */
	while (*basename != 0)
		*dest++ = *basename++;

	/* append the index */
	if (index >= 1000) *dest++ = '0' + ((index / 1000) % 10);
	if (index >= 100) *dest++ = '0' + ((index / 100) % 10);
	if (index >= 10) *dest++ = '0' + ((index / 10) % 10);
	*dest++ = '0' + (index % 10);
	*dest++ = 0;

	/* set the value */
	set_value(buffer, value);
}


/*-------------------------------------------------
    output_get_value - return the value of an
    output
-------------------------------------------------*/

s32 output_manager::get_value(const char *outname)
{
	output_item const *const item = find_item(outname);

	// if no item, value is 0
	return item ? item->get() : 0;
}


/*-------------------------------------------------
    output_set_notifier - sets a notifier callback
    for a particular output, or for all outputs
    if nullptr is specified
-------------------------------------------------*/

void output_manager::set_notifier(const char *outname, output_notifier_func callback, void *param)
{
	// if an item is specified, find/create it
	if (outname)
	{
		output_item *const item = find_item(outname);
		(item ? *item : create_new_item(outname, 0)).set_notifier(callback, param);
	}
	else
	{
		m_global_notifylist.emplace_back(callback, param);
	}
}


/*-------------------------------------------------
    output_notify_all - immediately call the given
    notifier for all outputs
-------------------------------------------------*/

void output_manager::notify_all(output_module *module)
{
	for (auto &item : m_itemtable)
		module->notify(item.second.name().c_str(), item.second.get());
}


/*-------------------------------------------------
    output_name_to_id - returns a unique ID for
    a given name
-------------------------------------------------*/

u32 output_manager::name_to_id(const char *outname)
{
	// if no item, ID is 0
	output_item const *const item = find_item(outname);
	return item ? item->id() : 0;
}


/*-------------------------------------------------
    output_id_to_name - returns a name that maps
    to a given unique ID
-------------------------------------------------*/

const char *output_manager::id_to_name(u32 id)
{
	for (auto &item : m_itemtable)
		if (item.second.id() == id)
			return item.second.name().c_str();

	/* nothing found, return nullptr */
	return nullptr;
}

// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/***************************************************************************

    output.c

    General purpose output routines.
***************************************************************************/

#include "emu.h"
#include "coreutil.h"

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
	machine.add_notifier(MACHINE_NOTIFY_PAUSE, machine_notify_delegate(FUNC(output_manager::pause), this));
	machine.add_notifier(MACHINE_NOTIFY_RESUME, machine_notify_delegate(FUNC(output_manager::resume), this));
}

/*-------------------------------------------------
    find_item - find an item based on a string
-------------------------------------------------*/

output_manager::output_item* output_manager::find_item(const char *string)
{
	/* use the hash as a starting point and find an entry */
	for (auto &item : m_itemtable)
		if (strcmp(string, item.second.name.c_str()) == 0)
			return &item.second;

	return nullptr;
}


/*-------------------------------------------------
    create_new_item - create a new item
-------------------------------------------------*/

output_manager::output_item *output_manager::create_new_item(const char *outname, INT32 value)
{
	output_item item;
	
	/* fill in the data */
	item.name = outname;
	item.id = m_uniqueid++;
	item.value = value;

	/* add us to the hash table */
	m_itemtable.insert(std::pair<std::string, output_item>(outname, item));
	return &m_itemtable.find(outname)->second;
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

void output_manager::set_value(const char *outname, INT32 value)
{
	output_item *item = find_item(outname);
	INT32 oldval;

	/* if no item of that name, create a new one and send the item's state */
	if (item == nullptr)
	{
		item = create_new_item(outname, value);
		oldval = value + 1;
	}

	else
	{
		/* set the new value */
		oldval = item->value;
		item->value = value;
	}

	/* if the value is different, signal the notifier */
	if (oldval != value)
	{
		/* call the local notifiers first */
		for (auto notify : item->notifylist)
			(*notify.m_notifier)(outname, value, notify.m_param);

		/* call the global notifiers next */
		for (auto notify : m_global_notifylist)
			(*notify.m_notifier)(outname, value, notify.m_param);
	}
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

INT32 output_manager::get_value(const char *outname)
{
	output_item *item = find_item(outname);

	/* if no item, value is 0 */
	if (item == nullptr)
		return 0;
	return item->value;
}


/*-------------------------------------------------
    output_get_indexed_value - get the value of an
    indexed output
-------------------------------------------------*/

INT32 output_manager::get_indexed_value(const char *basename, int index)
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
	return get_value(buffer);
}


/*-------------------------------------------------
    output_set_notifier - sets a notifier callback
    for a particular output, or for all outputs
    if NULL is specified
-------------------------------------------------*/

void output_manager::set_notifier(const char *outname, output_notifier_func callback, void *param)
{
	output_notify notify(callback, param);
	/* if an item is specified, find it */
	if (outname != nullptr)
	{
		output_item *item = find_item(outname);

		/* if no item of that name, create a new one */
		if (item == nullptr)
			item = create_new_item(outname, 0);
		
		item->notifylist.push_back(notify);
	}
	else
		m_global_notifylist.push_back(notify);
}


/*-------------------------------------------------
    output_notify_all - immediately call the given
    notifier for all outputs
-------------------------------------------------*/

void output_manager::notify_all(output_notifier_func callback, void *param)
{
	for (auto &item : m_itemtable)
		(*callback)(item.second.name.c_str(), item.second.value, param);
}


/*-------------------------------------------------
    output_name_to_id - returns a unique ID for
    a given name
-------------------------------------------------*/

UINT32 output_manager::name_to_id(const char *outname)
{
	output_item *item = find_item(outname);

	/* if no item, ID is 0 */
	if (item == nullptr)
		return 0;
	return item->id;
}


/*-------------------------------------------------
    output_id_to_name - returns a name that maps
    to a given unique ID
-------------------------------------------------*/

const char *output_manager::id_to_name(UINT32 id)
{
	for (auto &item : m_itemtable)
		if (item.second.id == id)
			return item.second.name.c_str();

	/* nothing found, return nullptr */
	return nullptr;
}

// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    dvsave.cpp

    Save debugger view.

***************************************************************************/

#include "emu.h"
#include "dvsave.h"

#include "debugvw.h"

#include "screen.h"


//**************************************************************************
//  DEBUG VIEW STATE
//**************************************************************************

//-------------------------------------------------
//  debug_view_save - constructor
//-------------------------------------------------

debug_view_save::debug_view_save(running_machine &machine, debug_view_osd_update_func osdupdate, void *osdprivate)
	: debug_view(machine, DVT_SAVE, osdupdate, osdprivate)
	, m_divider(40)
{
}


//-------------------------------------------------
//  ~debug_view_save - destructor
//-------------------------------------------------

debug_view_save::~debug_view_save()
{
	reset();
}


//-------------------------------------------------
//  reset - delete all of our state items
//-------------------------------------------------

void debug_view_save::reset()
{
	// free all items in the state list
	m_save_list.clear();
}


//-------------------------------------------------
//  recompute - recompute all info for the
//  registers view
//-------------------------------------------------

void debug_view_save::build_list_recursive(save_registered_item &item, uintptr_t objbase, int depth, int count)
{
	// update the base pointer and forward if a trivial unwrap
	if (item.unwrap_and_update_objbase(objbase))
		return build_list_recursive(item.subitems().front(), objbase, depth, count);

	// switch off the type
	switch (item.type())
	{
		// boolean types save as a single byte
		case save_registered_item::TYPE_BOOL:
		case save_registered_item::TYPE_INT:
		case save_registered_item::TYPE_UINT:
		case save_registered_item::TYPE_FLOAT:
			m_save_list.push_back(save_item(item, objbase, depth, count));
			break;

		// structs and containers iterate over owned items
		case save_registered_item::TYPE_CONTAINER:
		case save_registered_item::TYPE_STRUCT:
			if (item.subitems().size() == 0)
				break;
			m_save_list.push_back(save_item(item, objbase, depth, count));
			for (auto &subitem : item.subitems())
				build_list_recursive(subitem, objbase, depth + 1);
			break;

		// arrays are multiples of a single item
		default:
			if (m_type < save_registered_item::TYPE_ARRAY)
			{
				m_save_list.push_back(save_item(item, objbase, depth, count));
				auto &subitem = item.subitems().front();
				int items_per_row = 1;
				if (subitem.type() == save_registered_item::TYPE_BOOL)
					items_per_row = 32;
				else if (subitem.type() == save_registered_item::TYPE_INT || subitem.type() == save_registered_item::TYPE_UINT)
				{
					if (subitem.native_size() <= 2)
						items_per_row = 16 / subitem.native_size();
					else
						items_per_row = 32 / subitem.native_size();
				}
				else if (subitem.type() == save_registered_item::TYPE_FLOAT)
					items_per_row = 4;
				for (uint32_t rep = 0; rep < item.count(); rep += items_per_row)
					build_list_recursive(subitem, objbase + rep * item.native_size(), depth + 1, std::min<int>(items_per_row, item.count() - rep));
			}
			break;
	}
}

void debug_view_save::recompute()
{
	// start with a blank list
	reset();

	// build the list of items
	build_list_recursive(machine().save().root_registrar().parent_item(), 0, 0);
	m_save_list[0].set_collapse(false);
	for (auto &item : m_save_list)
		item.update_value();

	// no longer need to recompute
	m_recompute = false;
}


//-------------------------------------------------
//  view_notify - handle notification of updates
//  to cursor changes
//-------------------------------------------------

void debug_view_save::view_notify(debug_view_notification type)
{
	if (type == VIEW_NOTIFY_SOURCE_CHANGED)
		m_recompute = true;
}


//-------------------------------------------------
//  view_update - update the contents of the
//  register view
//-------------------------------------------------

void debug_view_save::view_update()
{
	// if our assumptions changed, revisit them
	if (m_recompute)
		recompute();

	// update the console info
	m_total.x = m_divider + 90;
	m_total.y = m_save_list.size();

	// loop over rows
	m_total.y = 0;
	int collapse_depth = 100;
	int depth_index[100];
	debug_view_char *dest(&m_viewdata[0]);
	std::string tempstr;
	for (auto &item : m_save_list)
	{
		if (item.depth() > collapse_depth)
			continue;
		if (item.depth() == collapse_depth)
			collapse_depth = 100;
		if (item.depth() < collapse_depth)
		{
			if (m_total.y >= m_topleft.y && m_total.y < m_topleft.y + m_visible.y)
			{
				// get the name
				char const *name = item.name();
				if (name[0] == 0)
				{
					if (item.depth() == 0)
						tempstr = "(root)";
					else if (item.count() > 1)
						tempstr = string_format("[%d-%d]", depth_index[item.depth()], depth_index[item.depth()] + item.count() - 1);
					else
						tempstr = string_format("[%d]", depth_index[item.depth()]);
					name = tempstr.c_str();
				}
				std::string value = item.value();

				// see if we changed
				const u8 attrib(item.changed() ? DCA_CHANGED : DCA_NORMAL);

				// build up a string
				char temp[256];
				int len = 0;
				for (int index = 0; index < item.depth(); index++)
					temp[len++] = temp[len++] = ' ';
				temp[len++] = item.collapsible() ? (item.collapsed() ? '+' : '-') : ' ';
				temp[len++] = ' ';

				int namelen = std::min<int>(strlen(name), m_divider - 1 - len);
				memcpy(&temp[len], name, namelen);
				len += namelen;
				while (len < m_divider)
					temp[len++] = ' ';
				temp[len++] = ' ';

				int valuelen = std::min(value.length(), sizeof(temp) - m_divider - 4);
				memcpy(&temp[len], value.c_str(), valuelen);
				len += valuelen;

				if (len < sizeof(temp) - 1)
					memset(&temp[len], ' ', sizeof(temp) - 1 - len);
				temp[sizeof(temp) - 1] = 0;

				// copy data
				int col = 0;
				for (u32 effcol = m_topleft.x; (col < m_visible.x) && (effcol < len); ++dest, ++col)
				{
					dest->byte = temp[effcol++];
					dest->attrib = attrib | ((effcol <= m_divider) ? DCA_ANCILLARY : DCA_NORMAL);
				}

				// fill the rest with blanks
				while (col < m_visible.x)
				{
					dest->byte = ' ';
					dest->attrib = DCA_NORMAL;
					dest++;
					col++;
				}
			}
			m_total.y++;
			if (item.collapsed())
				collapse_depth = item.depth();

			// reset the depth index if this is an array
			depth_index[item.depth() + 1] = 0;
			depth_index[item.depth()] += item.count();
		}
	}
	for (int y = m_total.y; y < m_topleft.y + m_visible.y; y++)
	{
		for (int col = 0; col < m_visible.x; col++)
		{
			dest->byte = ' ';
			dest->attrib = DCA_NORMAL;
			dest++;
		}
	}
}


//-------------------------------------------------
//  view_click - handle a mouse click within the
//  current view
//-------------------------------------------------

void debug_view_save::view_click(const int button, const debug_view_xy& pos)
{
	int cury = 0;
	int collapse_depth = 100;
	for (auto &item : m_save_list)
	{
		if (item.depth() > collapse_depth)
			continue;
		if (item.depth() == collapse_depth)
			collapse_depth = 100;
		if (item.depth() < collapse_depth)
		{
			if (pos.y == cury)
			{
				if (!item.collapsible())
					break;
				item.set_collapse(!item.collapsed());
				begin_update();
				view_notify(VIEW_NOTIFY_VISIBLE_CHANGED);
				m_update_pending = true;
				end_update();
				break;
			}
			cury++;
			if (item.collapsed())
				collapse_depth = item.depth();
		}
	}
}



std::string debug_view_save::save_item::value()
{
	// pre-subtract the item's ptr_offset as it will get re-applied at unwrap time
	return value(m_item, m_count, m_objbase - m_item.ptr_offset(), m_collapsed);
}


std::string debug_view_save::save_item::value(save_registered_item &item, int count, uintptr_t objbase, bool collapsed)
{
	// update the base pointer and forward if a trivial unwrap
	if (item.unwrap_and_update_objbase(objbase))
		return value(item.subitems().front(), 0, objbase, collapsed);

	char tempbuf[256];
	tempbuf[0] = 0;
	int pos = 0;

	switch (item.type())
	{
		// boolean types
		case save_registered_item::TYPE_BOOL:
			if (count == 0)
				return *reinterpret_cast<bool const *>(objbase) ? "true" : "false";
			else
			{
				bool const *ptr = reinterpret_cast<bool const *>(objbase);
				for (int index = 0; index < count; index++)
					catprintf(tempbuf, pos, "%c ", *ptr++ ? 'T' : 'F');
				return tempbuf;
			}

		// signed integral types
		case save_registered_item::TYPE_INT:
			if (count == 0)
				catprintf(tempbuf, pos, "%lld", item.read_int_signed(objbase, item.native_size()));
			else
			{
				static u8 const s_width[] = { 4,4,6,6,11,11,11,11,20,20,20,20,20,20,20,20 };
				int size = item.native_size() & 15;
				for (int index = 0; index < count; index++)
					catprintf(tempbuf, pos, "%*lld ", collapsed ? 0 : s_width[size], item.read_int_signed(objbase + size * index, size));
			}
			return tempbuf;

		// unsigned integral types
		case save_registered_item::TYPE_UINT:
			if (count == 0)
				catprintf(tempbuf, pos, "0x%0*llX", collapsed ? 0 : 2 * item.native_size(), item.read_int_unsigned(objbase, item.native_size()));
			else
			{
				int size = item.native_size() & 15;
				for (int index = 0; index < count; index++)
					catprintf(tempbuf, pos, "0x%0*llX ", collapsed ? 0 : 2 * size, item.read_int_unsigned(objbase + size * index, size));
			}
			return tempbuf;

		// float types
		case save_registered_item::TYPE_FLOAT:
			if (count == 0)
				catprintf(tempbuf, pos, "%g", item.read_float(objbase, item.native_size()));
			else
			{
				int size = item.native_size() & 15;
				for (int index = 0; index < count; index++)
					catprintf(tempbuf, pos, "%*g ", collapsed ? 0 : 20, item.read_float(objbase + size * index, size));
			}
			return tempbuf;

		// structs and containers iterate over owned items
		case save_registered_item::TYPE_CONTAINER:
		case save_registered_item::TYPE_STRUCT:
			if (!collapsed || item.subitems().size() == 0)
				return "";
			else
			{
				catprintf(tempbuf, pos, "{");
				bool first = true;
				for (auto &subitem : item.subitems())
				{
					std::string val = value(subitem, 0, objbase, true);
					if (val[0] == 0)
						continue;
					char const *name = subitem.name();
					if (name[0] == 0)
						name = subitem.subitems().front().name();
					if (!catprintf(tempbuf, pos, "%s%s:%s", first ? "" : ",", name, val.c_str()))
						break;
					first = false;
				}
				catprintf(tempbuf, pos, "}");
				return tempbuf;
			}

		// arrays are multiples of a single item
		default:
			if (item.type() < save_registered_item::TYPE_ARRAY)
			{
				if (!collapsed)
					catprintf(tempbuf, pos, "[%d]", item.count());
				else
				{
					catprintf(tempbuf, pos, "[");
					auto &subitem = item.subitems().front();
					for (int num = 0; num < item.count(); num++)
					{
						std::string val = value(subitem, 0, objbase + num * item.native_size(), true);
						if (!catprintf(tempbuf, pos, "%s%s", (num == 0) ? "" : ",", val.c_str()))
							break;
					}
					catprintf(tempbuf, pos, "]");
				}
				return tempbuf;
			}
			return "(unknown)";
	}
}


bool debug_view_save::save_item::changed() const
{
	switch (m_item.type())
	{
		case save_registered_item::TYPE_BOOL:
		case save_registered_item::TYPE_INT:
		case save_registered_item::TYPE_UINT:
		case save_registered_item::TYPE_FLOAT:
			return (memcmp(reinterpret_cast<void const *>(m_objbase), m_prev_value, m_item.native_size()) != 0);

		// structs and containers iterate over owned items
		case save_registered_item::TYPE_CONTAINER:
		case save_registered_item::TYPE_STRUCT:
			return false;

		// arrays are multiples of a single item
		default:
			return false;
	}
}


void debug_view_save::save_item::update_value()
{
	switch (m_item.type())
	{
		case save_registered_item::TYPE_BOOL:
		case save_registered_item::TYPE_INT:
		case save_registered_item::TYPE_UINT:
		case save_registered_item::TYPE_FLOAT:
			memcpy(m_prev_value, reinterpret_cast<void const *>(m_objbase), m_item.native_size());
			break;

		// structs and containers iterate over owned items
		case save_registered_item::TYPE_CONTAINER:
		case save_registered_item::TYPE_STRUCT:
			return;

		// arrays are multiples of a single item
		default:
			return;
	}
}

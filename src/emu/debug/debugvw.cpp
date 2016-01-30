// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    debugvw.c

    Debugger view engine.

***************************************************************************/

#include "emu.h"
#include "express.h"
#include "debugvw.h"
#include "dvtext.h"
#include "dvstate.h"
#include "dvdisasm.h"
#include "dvmemory.h"
#include "dvbpoints.h"
#include "dvwpoints.h"
#include "debugcpu.h"
#include <ctype.h>



//**************************************************************************
//  DEBUG VIEW SOURCE
//**************************************************************************

//-------------------------------------------------
//  debug_view_source - constructor
//-------------------------------------------------

debug_view_source::debug_view_source(const char *name, device_t *device)
	: m_next(nullptr),
		m_name(name),
		m_device(device),
		m_is_octal(false)
{
	device_execute_interface *intf;
	if (device && device->interface(intf))
		m_is_octal = intf->is_octal();

}


//-------------------------------------------------
//  ~debug_view_source - destructor
//-------------------------------------------------

debug_view_source::~debug_view_source()
{
}



//**************************************************************************
//  DEBUG VIEW SOURCE LIST
//**************************************************************************


//**************************************************************************
//  DEBUG VIEW
//**************************************************************************

//-------------------------------------------------
//  debug_view - constructor
//-------------------------------------------------

debug_view::debug_view(running_machine &machine, debug_view_type type, debug_view_osd_update_func osdupdate, void *osdprivate)
	: m_next(nullptr),
		m_type(type),
		m_source(nullptr),
		m_osdupdate(osdupdate),
		m_osdprivate(osdprivate),
		m_visible(80,10),
		m_total(80,10),
		m_topleft(0,0),
		m_cursor(0,0),
		m_supports_cursor(false),
		m_cursor_visible(false),
		m_recompute(true),
		m_update_level(0),
		m_update_pending(true),
		m_osd_update_pending(true),
		m_viewdata(m_visible.y * m_visible.x),
		m_machine(machine)
{
}


//-------------------------------------------------
//  ~debug_view - destructor
//-------------------------------------------------

debug_view::~debug_view()
{
}


//-------------------------------------------------
//  end_update - bracket a sequence of changes so
//  that only one update occurs
//-------------------------------------------------

void debug_view::end_update()
{
	/* if we hit zero, call the update function */
	if (m_update_level == 1)
	{
		while (m_update_pending)
		{
			// no longer pending, but flag for the OSD
			m_update_pending = false;
			m_osd_update_pending = true;

			// resize the viewdata if needed
			m_viewdata.resize(m_visible.x * m_visible.y);

			// update the view
			view_update();
		}
	}

	// decrement the level
	m_update_level--;
}


//-------------------------------------------------
//  flush_osd_updates - notify the OSD of any
//  pending updates
//-------------------------------------------------

void debug_view::flush_osd_updates()
{
	if (m_osd_update_pending && m_osdupdate != nullptr)
		(*m_osdupdate)(*this, m_osdprivate);
	m_osd_update_pending = false;
}


//-------------------------------------------------
//  set_visible_size - set the visible size in
//  rows and columns
//-------------------------------------------------*/

void debug_view::set_visible_size(debug_view_xy size)
{
	if (size.x != m_visible.x || size.y != m_visible.y)
	{
		begin_update();
		m_visible = size;
		m_update_pending = true;
		view_notify(VIEW_NOTIFY_VISIBLE_CHANGED);
		end_update();
	}
}


//-------------------------------------------------
//  set_visible_position - set the top left
//  position of the visible area in rows and
//  columns
//-------------------------------------------------

void debug_view::set_visible_position(debug_view_xy pos)
{
	if (pos.x != m_topleft.x || pos.y != m_topleft.y)
	{
		begin_update();
		m_topleft = pos;
		m_update_pending = true;
		view_notify(VIEW_NOTIFY_VISIBLE_CHANGED);
		end_update();
	}
}


//-------------------------------------------------
//  set_cursor_position - set the current cursor
//  position as a row and column
//-------------------------------------------------

void debug_view::set_cursor_position(debug_view_xy pos)
{
	if (pos.x != m_cursor.x || pos.y != m_cursor.y)
	{
		begin_update();
		m_cursor = pos;
		m_update_pending = true;
		view_notify(VIEW_NOTIFY_CURSOR_CHANGED);
		end_update();
	}
}


//-------------------------------------------------
//  set_cursor_visible - set the visible state of
//  the cursor
//-------------------------------------------------

void debug_view::set_cursor_visible(bool visible)
{
	if (visible != m_cursor_visible)
	{
		begin_update();
		m_cursor_visible = visible;
		m_update_pending = true;
		view_notify(VIEW_NOTIFY_CURSOR_CHANGED);
		end_update();
	}
}


//-------------------------------------------------
//  set_subview - set the current subview
//-------------------------------------------------

void debug_view::set_source(const debug_view_source &source)
{
	if (&source != m_source)
	{
		begin_update();
		m_source = &source;
		m_update_pending = true;
		view_notify(VIEW_NOTIFY_SOURCE_CHANGED);
		end_update();
	}
}


//-------------------------------------------------
//  source_for_device - find the first source that
//  matches the given device
//-------------------------------------------------

const debug_view_source *debug_view::source_for_device(device_t *device) const
{
	for (debug_view_source *source = m_source_list.first(); source != nullptr; source = source->next())
		if (device == source->device())
			return source;
	return m_source_list.first();
}


//-------------------------------------------------
//  adjust_visible_x_for_cursor - adjust a view's
//  visible X position to ensure the cursor is
//  visible
//-------------------------------------------------

void debug_view::adjust_visible_x_for_cursor()
{
	if (m_cursor.x < m_topleft.x)
		m_topleft.x = m_cursor.x;
	else if (m_cursor.x >= m_topleft.x + m_visible.x - 1)
		m_topleft.x = m_cursor.x - m_visible.x + 2;
}


//-------------------------------------------------
//  adjust_visible_y_for_cursor - adjust a view's
//  visible Y position to ensure the cursor is
//  visible
//-------------------------------------------------

void debug_view::adjust_visible_y_for_cursor()
{
	if (m_cursor.y < m_topleft.y)
		m_topleft.y = m_cursor.y;
	else if (m_cursor.y >= m_topleft.y + m_visible.y - 1)
		m_topleft.y = m_cursor.y - m_visible.y + 2;
}


//-------------------------------------------------
//  view_notify - handle notification of updates
//-------------------------------------------------

void debug_view::view_notify(debug_view_notification type)
{
	// default does nothing
}


//-------------------------------------------------
//  view_char - handle a character typed within
//  the current view
//-------------------------------------------------

void debug_view::view_char(int chval)
{
	// default does nothing
}


//-------------------------------------------------
//  view_click - handle a mouse click within the
//  current view
//-------------------------------------------------

void debug_view::view_click(const int button, const debug_view_xy& pos)
{
	// default does nothing
}



//**************************************************************************
//  DEBUG VIEW MANAGER
//**************************************************************************

//-------------------------------------------------
//  debug_view_manager - constructor
//-------------------------------------------------

debug_view_manager::debug_view_manager(running_machine &machine)
	: m_machine(machine),
		m_viewlist(nullptr)
{
}


//-------------------------------------------------
//  ~debug_view_manager - destructor
//-------------------------------------------------

debug_view_manager::~debug_view_manager()
{
	while (m_viewlist != nullptr)
	{
		debug_view *oldhead = m_viewlist;
		m_viewlist = oldhead->m_next;
		global_free(oldhead);
	}
}


//-------------------------------------------------
//  alloc_view - create a new view
//-------------------------------------------------

debug_view *debug_view_manager::alloc_view(debug_view_type type, debug_view_osd_update_func osdupdate, void *osdprivate)
{
	switch (type)
	{
		case DVT_CONSOLE:
			return append(global_alloc(debug_view_console(machine(), osdupdate, osdprivate)));

		case DVT_STATE:
			return append(global_alloc(debug_view_state(machine(), osdupdate, osdprivate)));

		case DVT_DISASSEMBLY:
			return append(global_alloc(debug_view_disasm(machine(), osdupdate, osdprivate)));

		case DVT_MEMORY:
			return append(global_alloc(debug_view_memory(machine(), osdupdate, osdprivate)));

		case DVT_LOG:
			return append(global_alloc(debug_view_log(machine(), osdupdate, osdprivate)));

		case DVT_TIMERS:
//          return append(global_alloc(debug_view_timers(machine(), osdupdate, osdprivate)));

		case DVT_ALLOCS:
//          return append(global_alloc(debug_view_allocs(machine(), osdupdate, osdprivate)));

		case DVT_BREAK_POINTS:
			return append(global_alloc(debug_view_breakpoints(machine(), osdupdate, osdprivate)));

		case DVT_WATCH_POINTS:
			return append(global_alloc(debug_view_watchpoints(machine(), osdupdate, osdprivate)));

		default:
			fatalerror("Attempt to create invalid debug view type %d\n", type);
	}
	return nullptr;
}


//-------------------------------------------------
//  free_view - free a view
//-------------------------------------------------

void debug_view_manager::free_view(debug_view &view)
{
	// free us but only if we're in the list
	for (debug_view **viewptr = &m_viewlist; *viewptr != nullptr; viewptr = &(*viewptr)->m_next)
		if (*viewptr == &view)
		{
			*viewptr = view.m_next;
			global_free(&view);
			break;
		}
}


//-------------------------------------------------
//  update_all_except - force all views to refresh
//  except one
//-------------------------------------------------

void debug_view_manager::update_all_except(debug_view_type type)
{
	// loop over each view and force an update
	for (debug_view *view = m_viewlist; view != nullptr; view = view->next())
		if (type == DVT_NONE || type != view->type())
			view->force_update();
}


//-------------------------------------------------
//  update_all - force all views to refresh
//-------------------------------------------------

void debug_view_manager::update_all(debug_view_type type)
{
	// loop over each view and force an update
	for (debug_view *view = m_viewlist; view != nullptr; view = view->next())
		if (type == DVT_NONE || type == view->type())
			view->force_update();
}


//-------------------------------------------------
//  flush_osd_updates - flush all pending OSD
//  updates
//-------------------------------------------------

void debug_view_manager::flush_osd_updates()
{
	for (debug_view *view = m_viewlist; view != nullptr; view = view->m_next)
		view->flush_osd_updates();
}


//-------------------------------------------------
//  append - append a view to the end of our list
//-------------------------------------------------

debug_view *debug_view_manager::append(debug_view *view)
{
	debug_view **viewptr;
	for (viewptr = &m_viewlist; *viewptr != nullptr; viewptr = &(*viewptr)->m_next) { }
	*viewptr = view;
	return view;
}



//**************************************************************************
//  DEBUG VIEW EXPRESSION
//**************************************************************************

//-------------------------------------------------
//  debug_view_expression - constructor
//-------------------------------------------------

debug_view_expression::debug_view_expression(running_machine &machine)
	: m_machine(machine),
		m_dirty(true),
		m_result(0),
		m_parsed(debug_cpu_get_global_symtable(machine)),
		m_string("0")
{
}


//-------------------------------------------------
//  ~debug_view_expression - destructor
//-------------------------------------------------

debug_view_expression::~debug_view_expression()
{
}


//-------------------------------------------------
//  set_context - set the context for the
//  expression
//-------------------------------------------------

void debug_view_expression::set_context(symbol_table *context)
{
	m_parsed.set_symbols((context != nullptr) ? context : debug_cpu_get_global_symtable(machine()));
	m_dirty = true;
}


//-------------------------------------------------
//  recompute - recompute the value of an
//  expression
//-------------------------------------------------

bool debug_view_expression::recompute()
{
	bool changed = m_dirty;

	// if dirty, re-evaluate
	if (m_dirty)
	{
		std::string oldstring(m_parsed.original_string());
		try
		{
			m_parsed.parse(m_string.c_str());
		}
		catch (expression_error &)
		{
			m_parsed.parse(oldstring.c_str());
		}
	}

	// if we have a parsed expression, evalute it
	if (!m_parsed.is_empty())
	{
		// recompute the value of the expression
		try
		{
			UINT64 newresult = m_parsed.execute();
			if (newresult != m_result)
			{
				m_result = newresult;
				changed = true;
			}
		}
		catch (expression_error &)
		{
		}
	}

	// expression no longer dirty by definition
	m_dirty = false;
	return changed;
}

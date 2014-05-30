/*
 * pstate.c
 *
 */

#include "pstate.h"

ATTR_COLD pstate_manager_t::~pstate_manager_t()
{
	m_save.clear_and_free();
}



ATTR_COLD void pstate_manager_t::save_state_ptr(const pstring &stname, const pstate_data_type_e dt, const void *owner, const int size, const int count, void *ptr, bool is_ptr)
{
	pstring fullname = stname;
	ATTR_UNUSED  pstring ts[] = {
			"NOT_SUPPORTED",
			"DT_CUSTOM",
			"DT_DOUBLE",
			"DT_INT64",
			"DT_INT16",
			"DT_INT8",
			"DT_INT",
			"DT_BOOLEAN"
	};

	NL_VERBOSE_OUT(("SAVE: <%s> %s(%d) %p\n", fullname.cstr(), ts[dt].cstr(), size, ptr));
	pstate_entry_t *p = new pstate_entry_t(stname, dt, owner, size, count, ptr, is_ptr);
	m_save.add(p);
}

ATTR_COLD void pstate_manager_t::remove_save_items(const void *owner)
{
	pstate_entry_t::list_t todelete;

	for (int i=0; i < m_save.count(); i++)
	{
		if (m_save[i]->m_owner == owner)
			todelete.add(m_save[i]);
	}
	for (int i=0; i < todelete.count(); i++)
	{
		m_save.remove(todelete[i]);
	}
	todelete.clear_and_free();
}

ATTR_COLD void pstate_manager_t::pre_save()
{
	for (int i=0; i < m_save.count(); i++)
		if (m_save[i]->m_dt == DT_CUSTOM)
			m_save[i]->m_callback->on_pre_save();
}

ATTR_COLD void pstate_manager_t::post_load()
{
	for (int i=0; i < m_save.count(); i++)
		if (m_save[i]->m_dt == DT_CUSTOM)
			m_save[i]->m_callback->on_post_load();
}

/*
 * pstate.c
 *
 */

#include "pstate.h"

ATTR_COLD pstate_manager_t::~pstate_manager_t()
{
	m_save.reset_and_free();
}



ATTR_COLD void pstate_manager_t::save_state_ptr(const pstring &stname, const pstate_data_type_e dt, const int size, const int count, void *ptr)
{
	pstring fullname = stname;
	ATTR_UNUSED  pstring ts[] = {
			"NOT_SUPPORTED",
			"DT_DOUBLE",
			"DT_INT64",
			"DT_INT8",
			"DT_INT",
			"DT_BOOLEAN"
	};

	NL_VERBOSE_OUT(("SAVE: <%s> %s(%d) %p\n", fullname.cstr(), ts[dt].cstr(), size, ptr));
	pstate_entry_t *p = new pstate_entry_t(stname, dt, size, count, ptr);
	m_save.add(p);
}


ATTR_COLD void pstate_manager_t::pre_save()
{
	for (int i=0; i < m_callback.count(); i++)
		m_callback[i]->on_pre_save();
}

ATTR_COLD void pstate_manager_t::post_load()
{
	for (int i=0; i < m_callback.count(); i++)
		m_callback[i]->on_post_load();
}

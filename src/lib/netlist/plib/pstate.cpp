// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pstate.c
 *
 */

#include "pstate.h"

namespace plib {

pstate_manager_t::pstate_manager_t()
{
}

pstate_manager_t::~pstate_manager_t()
{
	m_save.clear();
}



void pstate_manager_t::save_state_ptr(const void *owner, const pstring &stname, const pstate_data_type_e dt, const int size, const int count, void *ptr, bool is_ptr)
{
	pstring fullname = stname;
	ATTR_UNUSED  pstring ts[] = {
			"NOT_SUPPORTED",
			"DT_CUSTOM",
			"DT_DOUBLE",
#if (PHAS_INT128)
			"DT_INT128",
#endif
			"DT_INT64",
			"DT_INT16",
			"DT_INT8",
			"DT_INT",
			"DT_BOOLEAN",
			"DT_FLOAT"
	};

	auto p = plib::make_unique<pstate_entry_t>(stname, dt, owner, size, count, ptr, is_ptr);
	m_save.push_back(std::move(p));
}

void pstate_manager_t::remove_save_items(const void *owner)
{
	for (auto i = m_save.begin(); i != m_save.end(); )
	{
		if (i->get()->m_owner == owner)
			i = m_save.erase(i);
		else
			i++;
	}
}

void pstate_manager_t::pre_save()
{
	for (auto & s : m_save)
		if (s->m_dt == DT_CUSTOM)
			s->m_callback->on_pre_save();
}

void pstate_manager_t::post_load()
{
	for (auto & s : m_save)
		if (s->m_dt == DT_CUSTOM)
			s->m_callback->on_post_load();
}

template<> void pstate_manager_t::save_item(const void *owner, pstate_callback_t &state, const pstring &stname)
{
	//save_state_ptr(stname, DT_CUSTOM, 0, 1, &state);
	pstate_callback_t *state_p = &state;
	auto p = plib::make_unique<pstate_entry_t>(stname, owner, state_p);
	m_save.push_back(std::move(p));
	state.register_state(*this, stname);
}

}

// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pstate.c
 *
 */

#include "pstate.h"
#include "palloc.h"

namespace plib {
state_manager_t::state_manager_t()
{
}

state_manager_t::~state_manager_t()
{
	m_save.clear();
}



void state_manager_t::save_state_ptr(const void *owner, const pstring &stname, const datatype_t dt, const std::size_t count, void *ptr)
{
	auto p = plib::make_unique<entry_t>(stname, dt, owner, count, ptr);
	m_save.push_back(std::move(p));
}

void state_manager_t::remove_save_items(const void *owner)
{
	for (auto i = m_save.begin(); i != m_save.end(); )
	{
		if (i->get()->m_owner == owner)
			i = m_save.erase(i);
		else
			i++;
	}
}

void state_manager_t::pre_save()
{
	for (auto & s : m_save)
		if (s->m_dt.is_custom)
			s->m_callback->on_pre_save();
}

void state_manager_t::post_load()
{
	for (auto & s : m_save)
		if (s->m_dt.is_custom)
			s->m_callback->on_post_load();
}

template<> void state_manager_t::save_item(const void *owner, callback_t &state, const pstring &stname)
{
	//save_state_ptr(stname, DT_CUSTOM, 0, 1, &state);
	callback_t *state_p = &state;
	auto p = plib::make_unique<entry_t>(stname, owner, state_p);
	m_save.push_back(std::move(p));
	state.register_state(*this, stname);
}

}

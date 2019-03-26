// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pstate.c
 *
 */

#include "pstate.h"
#include "palloc.h"

namespace plib {

void state_manager_t::save_state_ptr(const void *owner, const pstring &stname, const datatype_t &dt, const std::size_t count, void *ptr)
{
	auto p = plib::make_unique<entry_t>(stname, dt, owner, count, ptr);
	m_save.push_back(std::move(p));
}

void state_manager_t::remove_save_items(const void *owner)
{
	auto i = m_save.end();
	while (i != m_save.begin())
	{
		i--;
		if (i->get()->m_owner == owner)
			i = m_save.erase(i);
	}
	i = m_custom.end();
	while (i > m_custom.begin())
	{
		i--;
		if (i->get()->m_owner == owner)
			i = m_custom.erase(i);
	}
}

void state_manager_t::pre_save()
{
	for (auto & s : m_custom)
		s->m_callback->on_pre_save(*this);
}

void state_manager_t::post_load()
{
	for (auto & s : m_custom)
		s->m_callback->on_post_load(*this);
}

template<> void state_manager_t::save_item(const void *owner, callback_t &state, const pstring &stname)
{
	callback_t *state_p = &state;
	auto p = plib::make_unique<entry_t>(stname, owner, state_p);
	m_custom.push_back(std::move(p));
	state.register_state(*this, stname);
}

} // namespace plib

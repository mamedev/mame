// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * osdmodule.c
 *
 */

#include "modules/osdmodule.h"
#include "osdcore.h"

#include <algorithm>


osd_module_manager::osd_module_manager()
{
}

osd_module_manager::~osd_module_manager()
{
}

void osd_module_manager::register_module(const module_type &mod_type)
{
	std::unique_ptr<osd_module> module = mod_type();
	if (module->probe())
	{
		osd_printf_verbose("===> registered module %s %s\n", module->name(), module->type());
		m_modules.emplace_back(std::move(module));
	}
	else
	{
		osd_printf_verbose("===> not supported %s %s\n", module->name(), module->type());
	}
}

bool osd_module_manager::type_has_name(const char *type, const char *name) const
{
	return (get_module_index(type, name) >= 0);
}

osd_module *osd_module_manager::get_module_generic(const char *type, const char *name)
{
	int const i = get_module_index(type, name);
	if (i >= 0)
		return m_modules[i].get();
	else
		return nullptr;
}

osd_module *osd_module_manager::select_module(const char *type, const char *name)
{
	osd_module *m = get_module_generic(type, name);

	// FIXME: check if already exists!
	if (m)
		m_selected.emplace_back(*m);
	return m;
}

void osd_module_manager::init(const osd_options &options)
{
	for (osd_module &m : m_selected)
		m.init(options);
}

void osd_module_manager::exit()
{
	// Find count
	while (!m_selected.empty())
	{
		m_selected.back().get().exit();
		m_selected.pop_back();
	}
}

int osd_module_manager::get_module_index(const char *type, const char *name) const
{
	for (int i = 0; m_modules.size() > i; i++)
	{
		if ((m_modules[i]->type() == type) && (!name[0] || (m_modules[i]->name() == name)))
			return i;
	}
	return -1;
}

void osd_module_manager::get_module_names(const char *type, const int max, int &num, const char *names[]) const
{
	num = 0;
	for (int i = 0; (m_modules.size() > i) && (max > num); i++)
	{
		if (m_modules[i]->type() == type)
			names[num++] = m_modules[i]->name().c_str();
	}
}

// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * osdmodule.cpp
 *
 */

#include "modules/osdmodule.h"

#include "emucore.h"

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

osd_module &osd_module_manager::select_module(osd_interface &osd, const osd_options &options, const char *type, const char *name)
{
	// see if a module of this type has already been already selected
	for (osd_module &existing : m_selected)
	{
		if (existing.type() == type)
		{
			osd_printf_warning(
					"Attempt to select %s module %s after selecting module %s\n",
					type,
					(name && *name) ? name : "<default>",
					existing.name());
			return existing;
		}
	}

	// try to start the selected module
	osd_module *const m = get_module_generic(type, name);
	if (m)
	{
		//osd_printf_verbose("Attempting to initialize %s module %s\n", type, m->name());
		if (!m->init(osd, options))
		{
			m_selected.emplace_back(*m);
			return *m;
		}
		osd_printf_verbose("Initializing %s module %s failed\n", type, m->name());
	}
	else
	{
		osd_printf_verbose("Could not find %s module %s\n", type, name ? name : "<default>");
	}

	// walk down the list until something works
	for (auto const &fallback : m_modules)
	{
		if ((fallback->type() == type) && (fallback.get() != m))
		{
			osd_printf_verbose("Attempting to initialize %s module %s\n", type, fallback->name());
			if (!fallback->init(osd, options))
			{
				osd_printf_warning(
						m
							?  "Initializing %s module %s failed, falling back to %s\n"
							: "Could not find %s module %s, falling back to %s\n",
						type,
						m ? m->name() : (name && *name) ? name : "<default>",
						fallback->name());
				m_selected.emplace_back(*fallback);
				return *fallback;
			}
		}
	}

	// can't deal with absence of a module, and at least the "none" module should have worked
	throw emu_fatalerror("All %s modules failed to initialize", type);
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

std::vector<std::string_view> osd_module_manager::get_module_names(const char *type) const
{
	std::vector<std::string_view> result;
	for (auto &m : m_modules)
	{
		if (m->type() == type)
			result.emplace_back(m->name());
	}
	return result;
}

// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * osdmodule.c
 *
 */

#include "modules/osdmodule.h"

osd_module_manager::osd_module_manager()
{
	for (int i=0; i<MAX_MODULES; i++)
	{
		m_modules[i]  = NULL;
		m_selected[i] = NULL;
	}
}
osd_module_manager::~osd_module_manager()
{
	for (int i = 0; m_modules[i] != NULL; i++)
	{
		m_modules[i]->~osd_module();
		osd_free(m_modules[i]);
	}
}

void osd_module_manager::register_module(const module_type &mod_type)
{
	osd_module *module = mod_type();
	if (module->probe())
	{
		osd_printf_verbose("===> registered module %s %s\n", module->name(), module->type());

		int i;
		for (i = 0; m_modules[i] != NULL; i++)
			;
		m_modules[i] = module;
	}
	else
	{
		osd_printf_verbose("===> not supported %s %s\n", module->name(), module->type());
		module->~osd_module();
		osd_free(module);
	}
}

bool osd_module_manager::type_has_name(const char *type, const char *name)
{
	return (get_module_index(type, name) >= 0);
}

osd_module *osd_module_manager::get_module_generic(const char *type, const char *name)
{
	int i = get_module_index(type, name);
	if (i>=0)
		return m_modules[i];
	else
		return NULL;
}

osd_module *osd_module_manager::select_module(const char *type, const char *name)
{
	osd_module *m = get_module_generic(type, name);

	// FIXME: check if already exists!
	int i;
	for (i = 0; m_selected[i] != NULL; i++)
		;
	m_selected[i] = m;
	return m;
}

void osd_module_manager::init(const osd_options &options)
{
	for (int i = 0; m_selected[i] != NULL; i++)
	{
		m_selected[i]->init(options);
	}
}

void osd_module_manager::exit()
{
	// Find count
	int cnt;
	for (cnt = 0; m_selected[cnt] != NULL; cnt++)
		;
	for (int i = cnt - 1; i >= 0; i--)
	{
		m_selected[i]->exit();
		m_selected[i] = NULL;
	}
}

int osd_module_manager::get_module_index(const char *type, const char *name)
{
	for (int i = 0; m_modules[i] != NULL; i++)
	{
		if (strcmp(m_modules[i]->type(), type) == 0 && ((name[0] == 0) || (strcmp(name, m_modules[i]->name())==0)))
			return i;
	}
	return -1;
}

void osd_module_manager::get_module_names(const char *type, const int max, int *num, const char *names[])
{
	*num = 0;
	for (int i = 0; m_modules[i] != NULL; i++)
	{
		if ((strcmp(m_modules[i]->type(), type) == 0) && (*num < max))
		{
			names[*num] = m_modules[i]->name();
			*num = *num + 1;
		}

	}
}

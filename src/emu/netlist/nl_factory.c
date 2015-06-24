// license:GPL-2.0+
// copyright-holders:Couriersud
/***************************************************************************

    nl_factory.c

    Discrete netlist implementation.

****************************************************************************/

#include "nl_factory.h"
#include "nl_setup.h"

namespace netlist
{
// ----------------------------------------------------------------------------------------
// net_device_t_base_factory
// ----------------------------------------------------------------------------------------

ATTR_COLD const pstring_list_t base_factory_t::term_param_list()
{
	if (m_def_param.startsWith("+"))
		return pstring_list_t(m_def_param.substr(1), ",");
	else
		return pstring_list_t();
}

ATTR_COLD const pstring_list_t base_factory_t::def_params()
{
	if (m_def_param.startsWith("+") || m_def_param.equals("-"))
		return pstring_list_t();
	else
		return pstring_list_t(m_def_param, ",");
}


factory_list_t::factory_list_t()
{
}

factory_list_t::~factory_list_t()
{
	for (std::size_t i=0; i < m_list.size(); i++)
	{
		base_factory_t *p = m_list[i];
		pfree(p);
	}
	m_list.clear();
}

device_t *factory_list_t::new_device_by_classname(const pstring &classname) const
{
	for (std::size_t i=0; i < m_list.size(); i++)
	{
		base_factory_t *p = m_list[i];
		if (p->classname() == classname)
		{
			device_t *ret = p->Create();
			return ret;
		}
		p++;
	}
	return NULL; // appease code analysis
}

device_t *factory_list_t::new_device_by_name(const pstring &name, setup_t &setup) const
{
	base_factory_t *f = factory_by_name(name, setup);
	return f->Create();
}

base_factory_t * factory_list_t::factory_by_name(const pstring &name, setup_t &setup) const
{
	for (std::size_t i=0; i < m_list.size(); i++)
	{
		base_factory_t *p = m_list[i];
		if (p->name() == name)
		{
			return p;
		}
		p++;
	}
	setup.netlist().error("Class %s not found!\n", name.cstr());
	return NULL; // appease code analysis
}

}

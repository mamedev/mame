// license:GPL-2.0+
// copyright-holders:Couriersud
/***************************************************************************

    nl_factory.c

    Discrete netlist implementation.

****************************************************************************/

#include "nl_factory.h"
#include "nl_setup.h"

// ----------------------------------------------------------------------------------------
// net_device_t_base_factory
// ----------------------------------------------------------------------------------------

ATTR_COLD const nl_util::pstring_list net_device_t_base_factory::term_param_list()
{
	if (m_def_param.startsWith("+"))
		return nl_util::split(m_def_param.substr(1), ",");
	else
		return nl_util::pstring_list();
}

ATTR_COLD const nl_util::pstring_list net_device_t_base_factory::def_params()
{
	if (m_def_param.startsWith("+") || m_def_param.equals("-"))
		return nl_util::pstring_list();
	else
		return nl_util::split(m_def_param, ",");
}


netlist_factory_t::netlist_factory_t()
{
}

netlist_factory_t::~netlist_factory_t()
{
	for (std::size_t i=0; i < m_list.size(); i++)
	{
		net_device_t_base_factory *p = m_list[i];
		pfree(p);
	}
	m_list.clear();
}

netlist_device_t *netlist_factory_t::new_device_by_classname(const pstring &classname) const
{
	for (std::size_t i=0; i < m_list.size(); i++)
	{
		net_device_t_base_factory *p = m_list[i];
		if (p->classname() == classname)
		{
			netlist_device_t *ret = p->Create();
			return ret;
		}
		p++;
	}
	return NULL; // appease code analysis
}

netlist_device_t *netlist_factory_t::new_device_by_name(const pstring &name, netlist_setup_t &setup) const
{
	net_device_t_base_factory *f = factory_by_name(name, setup);
	return f->Create();
}

net_device_t_base_factory * netlist_factory_t::factory_by_name(const pstring &name, netlist_setup_t &setup) const
{
	for (std::size_t i=0; i < m_list.size(); i++)
	{
		net_device_t_base_factory *p = m_list[i];
		if (p->name() == name)
		{
			return p;
		}
		p++;
	}
	setup.netlist().error("Class %s not found!\n", name.cstr());
	return NULL; // appease code analysis
}

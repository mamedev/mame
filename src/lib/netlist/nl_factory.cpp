// license:GPL-2.0+
// copyright-holders:Couriersud
/***************************************************************************

    nl_factory.c

    Discrete netlist implementation.

****************************************************************************/

#include "nl_factory.h"
#include "nl_setup.h"
#include "plib/putil.h"

namespace netlist
{

// ----------------------------------------------------------------------------------------
// net_device_t_base_factory
// ----------------------------------------------------------------------------------------

const plib::pstring_vector_t base_factory_t::term_param_list()
{
	if (m_def_param.startsWith("+"))
		return plib::pstring_vector_t(m_def_param.substr(1), ",");
	else
		return plib::pstring_vector_t();
}

const plib::pstring_vector_t base_factory_t::def_params()
{
	if (m_def_param.startsWith("+") || m_def_param.equals("-"))
		return plib::pstring_vector_t();
	else
		return plib::pstring_vector_t(m_def_param, ",");
}


factory_list_t::factory_list_t( setup_t &setup)
: m_setup(setup)
{
}

factory_list_t::~factory_list_t()
{
	clear();
}

void factory_list_t::error(const pstring &s)
{
	m_setup.log().fatal("{1}", s);
}

base_factory_t * factory_list_t::factory_by_name(const pstring &devname)
{
	for (auto & e : *this)
		if (e->name() == devname)
			return e.get();

	m_setup.log().fatal("Class {1} not found!\n", devname);
	return nullptr; // appease code analysis
}

}

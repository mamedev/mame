// license:GPL-2.0+
// copyright-holders:Couriersud
/***************************************************************************

    nl_factory.c

    Discrete netlist implementation.

****************************************************************************/

#include "nl_factory.h"
#include "nl_setup.h"
#include "plib/putil.h"

namespace netlist { namespace factory
{

element_t::element_t(const pstring &name, const pstring &classname,
		const pstring &def_param)
	: m_name(name), m_classname(classname), m_def_param(def_param)
{
}

element_t::~element_t()
{
}


// ----------------------------------------------------------------------------------------
// net_device_t_base_factory
// ----------------------------------------------------------------------------------------

list_t::list_t( setup_t &setup)
: m_setup(setup)
{
}

list_t::~list_t()
{
	clear();
}

void list_t::error(const pstring &s)
{
	m_setup.log().fatal("{1}", s);
}

factory::element_t * list_t::factory_by_name(const pstring &devname)
{
	for (auto & e : *this)
	{
		if (e->name() == devname)
			return e.get();
	}

	m_setup.log().fatal("Class <{1}> not found!\n", devname);
	return nullptr; // appease code analysis
}

// -----------------------------------------------------------------------------
// factory_lib_entry_t: factory class to wrap macro based chips/elements
// -----------------------------------------------------------------------------

plib::owned_ptr<device_t> library_element_t::Create(netlist_t &anetlist, const pstring &name)
{
	return plib::owned_ptr<device_t>::Create<NETLIB_NAME(wrapper)>(anetlist, name);
}

void library_element_t::macro_actions(netlist_t &anetlist, const pstring &name)
{
	anetlist.setup().namespace_push(name);
	anetlist.setup().include(this->name());
	anetlist.setup().namespace_pop();
}

NETLIB_RESET(wrapper)
{
}

NETLIB_UPDATE(wrapper)
{
}


} }
